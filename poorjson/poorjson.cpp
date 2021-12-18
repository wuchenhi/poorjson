#include "poorjson.h"


#include <assert.h>


#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, strtod() */


#include <iostream>


using namespace std;


namespace poorjson{


class json_context {
public:
    const char* json;
};


template<typename T>
inline bool ISDIGIT(const T& ch)        
{
   return ((ch) >= '0' && (ch) <= '9');
}


template<typename T>
inline bool ISDIGIT1TO9(const T& ch)        
{
   return ((ch) >= '1' && (ch) <= '9');
}




inline void EXPECT(json_context* c, const char& ch)   //不加const 会报错：非常量引用必须是左值
{
    assert(*c->json == (ch)); //fixme
    c->json++;
}


//判断空格
static void poor_parse_whitespace(json_context* c)
{
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}


static int poor_parse_true(json_context* c, json_value* v)
{
    EXPECT(c,'t');//c不是指针吗     已解决，比较的不是c和‘t’ 是断言里的内容傻子
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return static_cast<int>(return_json::INVALID_VALUE);      //不可能将强类型的枚举隐式转换为整数，必须使用static_cast明确显示转换
    c->json += 3;
    v->type = json_type::TRUE;
    return static_cast<int>(return_json::OK);
}


static int poor_parse_false(json_context* c, json_value* v)
{
    EXPECT(c , 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return static_cast<int>(return_json::INVALID_VALUE);
    c->json += 4;
    v->type = json_type::FALSE;
    return static_cast<int>(return_json::OK);
}


static int poor_parse_null(json_context* c, json_value* v)
{
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return static_cast<int>(return_json::INVALID_VALUE);
    c->json += 3;
    v->type = json_type::NULL1;
    return static_cast<int>(return_json::OK);
}


static int poor_parse_number(json_context* c, json_value* v)
{
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return static_cast<int>(return_json::INVALID_VALUE);
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return static_cast<int>(return_json::INVALID_VALUE);
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return static_cast<int>(return_json::INVALID_VALUE);
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
       return static_cast<int>(return_json::NUMBER_TOO_BIG);
    v->type = json_type::NUMBER;
    c->json = p;
    return static_cast<int>(return_json::OK);
}


int poor_parse_value(json_context* c, json_value* v) {
    switch (*c->json) {
        case 't':  return poor_parse_true(c, v);
        case 'f':  return poor_parse_false(c, v);
        case 'n':  return poor_parse_null(c, v);
        default:   return poor_parse_number(c, v);
        case '\0': return static_cast<int>(return_json::EXPECT_VALUE);
       
    }


}


//解析json   传入的根节点指针 v  TODO！
int json_parse(json_value* v, const char* json)//"null"
{
    json_context j;
    /*当程序以 release 配置编译时（定义了 `NDEBUG` 宏），`assert()` 不会做检测；而当在 debug 配置时（没定义 `NDEBUG` 宏），则会在运行时检测 `assert(cond)` 中的条件是否为真（非 0），断言失败会直接令程序崩溃*/
    assert(v != nullptr);   //C++11：—— 引入关键字 nullptr
    j.json = json;
    v->type = json_type::NULL1;
    poor_parse_whitespace(&j);          //FIXME
    int rt = static_cast<int>(poor_parse_value(&j,v));  //分行来写，不然报错
    if( rt == static_cast<int>(return_json::OK)){
        poor_parse_whitespace(&j);  
        if (*j.json != '\0')
           rt = static_cast<int>(return_json::ROOT_NOT_SINGULAR);
    }
    return rt;
}


json_type json_get_type(const json_value* v)
{
    assert(v != nullptr);
    return v->type;
}


double json_get_number(const json_value* v)
{
    assert(v != nullptr);
    return v->n;
}




}
