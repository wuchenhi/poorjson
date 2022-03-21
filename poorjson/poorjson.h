#ifndef POORJSON_H__
#define POORJSON_H__
/*宏的名字唯一，习惯以 `_H__` 作为后缀*/

#include <stddef.h> //size_t

namespace poorjson{
    /**JSON 中有 6 种数据类型，如果把 true 和 false 当作两个类型就是 7 种
     * 枚举类型不是类型安全
     * 枚举类型被视为整数，这使得两种不同的枚举类型之间可以进行比较
     * 枚举的名称全数暴露于一般范围中，因此C++03两个不同的枚举，不可以有相同的枚举名  
    */
    enum class json_type
    {
        NULL1,
        FALSE,
        TRUE,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT
    }; //枚举类替换枚举
   
    /*JSON 是一个树形结构，我们最终需要实现一个树的数据结构，每个节点使用 `poor_value` 类表示，我们会称它为一个 JSON 值（JSON value）*/
    class json_value
     {public:  //FIXME
        double n;
        json_type type;
     };
   
    
    /* API 函数:解析 JSON*/
    int json_parse(json_value* v, const char* json);

   
    /* API 函数:生成 JSON*/
    char* json_stringify(const json_value* v);

    enum class return_json{
        OK = 0,
        EXPECT_VALUE,       //只有空白
        INVALID_VALUE,      //不是合法字面值
        ROOT_NOT_SINGULAR,  //空白只有还有字面值
        NUMBER_TOO_BIG
    };


    /*获取类型的函数 FIXME*/
    json_type json_get_type(const json_value* v);


    double json_get_number(const json_value* v);

    

}




#endif  /* POORJSON_H__ */
