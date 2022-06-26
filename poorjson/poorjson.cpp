#include "poorjson.h"

using namespace std;

namespace poorjson{

//解析字符串（数组、对象）时，需要把解析的结果先储存在一个临时的缓冲区
//最后再用 poor_set_string()把缓冲区的结果设进值之中
class json_context {
public:
    const char* json;
    string str; //缓冲区
};

template<typename T>
inline bool ISDIGIT(const T& ch) {
   return ((ch) >= '0' && (ch) <= '9');
}

template<typename T>
inline bool ISDIGIT1TO9(const T& ch) {
   return ((ch) >= '1' && (ch) <= '9');
}

inline void EXPECT(json_context* c, const char& ch) {   //不加const 会报错：非常量引用必须是左值
    assert(*c->json == (ch)); //fixme
    c->json++;
}

//JSON-text = ws value ws
//ws = *(%x20 / %x09 / %x0A / %x0D)
//value = null / false / true
//null  = "null"
//false = "false"
//true  = "true"
//ws 零或多个空格符（space U+0020）、制表符（tab U+0009）、换行符（LF U+000A）、回车符（CR U+000D）

//判断空格
static void poor_parse_whitespace(json_context* c) {
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        ++p;
    c->json = p;
}

//判断true
static int poor_parse_true(json_context* c, json_value* v) {
    EXPECT(c,'t');
    //注意此时 c->json 已经 +1
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return static_cast<int>(return_json::INVALID_VALUE);      //不可能将强类型的枚举隐式转换为整数，必须使用static_cast明确显示转换
    c->json += 3;
    v->type = json_type::TRUE;
    return static_cast<int>(return_json::OK);
}

//判断false
static int poor_parse_false(json_context* c, json_value* v) {
    EXPECT(c , 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return static_cast<int>(return_json::INVALID_VALUE);
    c->json += 4;
    v->type = json_type::FALSE;
    return static_cast<int>(return_json::OK);
}

//判断null
static int poor_parse_null(json_context* c, json_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return static_cast<int>(return_json::INVALID_VALUE);
    c->json += 3;
    v->type = json_type::NULL1;
    return static_cast<int>(return_json::OK);
}
//判断number
//number = [ "-" ] int [ frac ] [ exp ]
//int = "0" / digit1-9 *digit
//frac = "." 1*digit
//exp = ("e" / "E") ["-" / "+"] 1*digit
static int poor_parse_number(json_context* c, json_value* v) {
    const char* p = c->json;
    if (*p == '-') ++p;
    if (*p == '0') ++p;
    else {
        if (!ISDIGIT1TO9(*p)) 
            return static_cast<int>(return_json::INVALID_VALUE);
        for (p++; ISDIGIT(*p); p++);  //跳过数字
    }
    if (*p == '.') {
        ++p;
        if (!ISDIGIT(*p)) 
            return static_cast<int>(return_json::INVALID_VALUE);
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        ++p;
        if (*p == '+' || *p == '-') 
            ++p;
        if (!ISDIGIT(*p)) 
            return static_cast<int>(return_json::INVALID_VALUE);
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
       return static_cast<int>(return_json::NUMBER_TOO_BIG);
    v->type = json_type::NUMBER;
    c->json = p; //校验成功，把 p 赋值回 c->json
    return static_cast<int>(return_json::OK);
}

// 解析 4 位十六进数字，存储为码点 `u`
static const char* parse_hex4(const char* p, unsigned* u) {
    int i;
    *u = 0;
    for (i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        if      (ch >= '0' && ch <= '9')  *u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
        else return NULL;
    }
    return p;
}

//把码点编码成 UTF-8，写进缓冲区
static void encode_utf8(json_context* c, unsigned u) {
    if (u <= 0x7F) 
        c->str += (u & 0xFF);
    else if (u <= 0x7FF) {
        c->str += (0xC0 | ((u >> 6) & 0xFF));
        c->str += (0x80 | ( u       & 0x3F));
    }
    else if (u <= 0xFFFF) {
        c->str += (0xE0 | ((u >> 12) & 0xFF));
        c->str += (0x80 | ((u >>  6) & 0x3F));
        c->str += (0x80 | ( u        & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);
        c->str += (0xF0 | ((u >> 18) & 0xFF));
        c->str += (0x80 | ((u >> 12) & 0x3F));
        c->str += (0x80 | ((u >>  6) & 0x3F));
        c->str += (0x80 | ( u        & 0x3F));
    }
}

//判断 string  FIXME 
//TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
static int poor_parse_string(json_context* c, json_value* v) {
    EXPECT(c, '\"');//已经跳过 \" 了
    ///cout << *(c->json)<<endl;
    unsigned u, u2;
    const char* p = c->json;
    while(1) {
        char ch = *p++;
        //cout << ch <<endl;
        switch (ch) {
            case '\"':
                v->s = c->str;
                v->type = json_type::STRING;
                c->json = p;
                return static_cast<int>(return_json::OK);
            case '\\':// \\就是一个字符
                switch (*p++) {
                    case '\"': c->str += '\"'; break;
                    case '\\': c->str += '\\'; break;
                    case '/':  c->str += '/';  break;
                    case 'b':  c->str += '\b'; break;
                    case 'f':  c->str += '\f'; break;
                    case 'n':  c->str += '\n'; break;
                    case 'r':  c->str += '\r'; break;
                    case 't':  c->str += '\t'; break;
                    case 'u': 
                        if (!(p = parse_hex4(p, &u)))
                            return static_cast<int>(return_json::INVALID_UNICODE_HEX);
                        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
                            if (*p++ != '\\')
                                return static_cast<int>(return_json::INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                return static_cast<int>(return_json::INVALID_UNICODE_SURROGATE);
                            if (!(p = parse_hex4(p, &u2)))
                                return static_cast<int>(return_json::INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                return static_cast<int>(return_json::INVALID_UNICODE_SURROGATE);
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        encode_utf8(c, u);
                        break;
                    default:
                        return static_cast<int>(return_json::INVALID_STRING_ESCAPE);
                }
            //case '\0':// 为什么会进入这里
                //return static_cast<int>(return_json::MISS_QUOTATION_MARK);
            default:
                if ((unsigned char)ch < 0x20)
                    return static_cast<int>(return_json::INVALID_STRING_CHAR);
                c->str += ch;
        }
    }
}

static int poor_parse_value(json_context* c, json_value* v);

//TODO
static int poor_parse_array(json_context* c, json_value* v) {
    size_t i, size = 0;
    int ret;
    EXPECT(c, '[');
    poor_parse_whitespace(c);
    if (*c->json == ']') {
        c->json++;
        v->type = json_type::ARRAY;

        return static_cast<int>(return_json::OK);
    }
    while(1) {

    }
}
int poor_parse_value(json_context* c, json_value* v) {
    switch (*c->json) {
        case 't':  return poor_parse_true(c, v);
        case 'f':  return poor_parse_false(c, v);
        case 'n':  return poor_parse_null(c, v);
        case '"':  return poor_parse_string(c, v);
        case '\0': return static_cast<int>(return_json::EXPECT_VALUE);
        case '[]': return poor_parse_array(c, v);
        default:   return poor_parse_number(c, v);
    }
}

//解析json   传入的根节点指针 v  TODO！
int json_parse(json_value* v, const char* json) {  //"null"
    json_context j;
    //当程序以 release 配置编译时（定义了 `NDEBUG` 宏），`assert()` 不会做检测；
    //而当在 debug 配置时（没定义 `NDEBUG` 宏），则会在运行时检测 `assert(cond)` 中的条件是否为真（非 0），断言失败会直接令程序崩溃
    assert(v != nullptr); 
    j.json = json;
    j.str ="";
    v->type = json_type::NULL1;
    poor_parse_whitespace(&j);
    int rt = static_cast<int>(poor_parse_value(&j, v)); 
    if( rt == static_cast<int>(return_json::OK)) {
        poor_parse_whitespace(&j);  
        if (*j.json != '\0')
           rt = static_cast<int>(return_json::ROOT_NOT_SINGULAR);
    }
    return rt;
}

//string是可变长的，用于临时储存生成的结果 TODO 为什么第一个字符被删
const char* json_stringify(const json_value* v ){
    json_context c;
    string s = " "; 
    switch (v->type) {
        case (json_type::NULL1):  s += "null"; break;
        case (json_type::FALSE):  s += "false"; break;
        case (json_type::TRUE):   s += "true"; break;
        case (json_type::NUMBER): s += to_string(v->n); break; //C++11，标准库提供了std::to_string辅助函数转化各类型为一个字符串
        case (json_type::STRING): s += v->s; break;
    }
    //c.json = const_cast<char*>(c.json); //const_cast 将对象的常量性移除
    c.json = s.c_str();
    return c.json;
    // c.json = strTochar(s);
    //return const_cast<char*>(c.json);
}

json_type json_value::json_get_type() {
    return type;
}

//仅当 json_type == NUMBER 时，n 才表示 JSON 数字的数值
double json_value::json_get_number() {
    assert(type == json_type::NUMBER);
    return n;
}

const char* json_value::json_get_string() {
    assert(type == json_type::STRING);
    return s.c_str();
}

} //namespace poorjson

