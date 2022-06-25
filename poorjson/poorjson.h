#ifndef POORJSON_H__
#define POORJSON_H__

#include <stddef.h> //size_t
#include <assert.h>
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, strtod() */
#include <memory>    /*shared_point*/
#include <string>
#include <vector>
#include <iostream>

namespace poorjson {
/**JSON 中有 6 种数据类型，如果把 true 和 false 当作两个类型就是 7 种
 * 枚举类型不是类型安全
 * 枚举类型被视为整数，这使得两种不同的枚举类型之间可以进行比较
 * 枚举的名称全数暴露于一般范围中，因此C++03两个不同的枚举，不可以有相同的枚举名  
*/
enum class json_type {
    NULL1,
    FALSE,
    TRUE,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT
}; //枚举类替换枚举
   
// JSON 是一个树形结构，节点用 `json_value` 类表示
class json_value {
public:     //FIXME
    double n;
    std::string s;
    json_type type;
    // union ? TODO一个值不可能同时为数字和字符串
};
   
// API 函数:解析 JSON 传入 const char* json
int json_parse(json_value* v, const char* json);

// API 函数:生成 JSON
const char* json_stringify(const json_value* v);

enum class return_json {
    OK = 0,
    EXPECT_VALUE,       //只有空白
    INVALID_VALUE,      //不是合法字面值
    ROOT_NOT_SINGULAR,  //空白只有还有字面值
    NUMBER_TOO_BIG,
    MISS_QUOTATION_MARK,
    INVALID_STRING_ESCAPE,
    INVALID_STRING_CHAR
};

//获取类型的函数 FIXME
json_type json_get_type(const json_value* v);

double json_get_number(const json_value* v);

const char* json_get_string(const json_value* v);

} //namespace poorjson

#endif  /* POORJSON_H__ */
