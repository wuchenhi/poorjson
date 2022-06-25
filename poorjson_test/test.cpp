//直接编译 g++ test.cpp ../poorjson/poorjson.cpp -std=c++11 -o test
#include "../poorjson/poorjson.h" //相对路径
#include <stdio.h>//FIXME
#include <stdlib.h>//FIXME
#include <string>
#include <iostream>
using namespace  std;

using namespace poorjson;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

//这个测试框架使用了 `__LINE__` 这个编译器提供的宏，代表编译时该行的行号。如果用函数或内联函数，每次的行号便都会相同。
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do { \
        ++test_count; \
        if (equality) \
            ++test_pass; \
        else { \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1; \
        } \
    } while(0)

template<typename T>
inline void EXPECT_EQ_INT(const T& expect, const T& actual) {
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d");
}

template<typename T>
inline void EXPECT_EQ_DOUBLE(const T &expect, const T& actual) {
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g");
}

static void test_parse_null() {
    json_value v;
    v.type = json_type::FALSE;

    EXPECT_EQ_INT(static_cast<int>(return_json::OK), json_parse(&v, "null"));
    EXPECT_EQ_INT(static_cast<int>(json_type::NULL1),static_cast<int>(json_get_type(&v)));
}

static void test_parse_true() {
    json_value v;
    v.type = json_type::FALSE;

    EXPECT_EQ_INT(static_cast<int>(return_json::OK), json_parse(&v, "true"));
    EXPECT_EQ_INT(static_cast<int>(json_type::TRUE),static_cast<int>(json_get_type(&v)));
}

static void test_parse_false() {
    json_value v;
    v.type = json_type::TRUE;

    EXPECT_EQ_INT(static_cast<int>(return_json::OK), json_parse(&v, "false"));
    EXPECT_EQ_INT(static_cast<int>(json_type::FALSE),static_cast<int>(json_get_type(&v)));
}

static void test_parse_expect_value() {
    json_value v;
    v.type = json_type::FALSE;

    EXPECT_EQ_INT(static_cast<int>(return_json::EXPECT_VALUE), json_parse(&v, " "));
    EXPECT_EQ_INT(static_cast<int>(json_type::NULL1),static_cast<int>(json_get_type(&v)));

    v.type = json_type::FALSE;

    EXPECT_EQ_INT(static_cast<int>(return_json::EXPECT_VALUE), json_parse(&v, ""));
    EXPECT_EQ_INT(static_cast<int>(json_type::NULL1),static_cast<int>(json_get_type(&v)));
}

static void test_parse_invalid_value() {
    json_value v;
    v.type = json_type::FALSE;

    EXPECT_EQ_INT(static_cast<int>(return_json::INVALID_VALUE), json_parse(&v, "nul"));
    EXPECT_EQ_INT(static_cast<int>(json_type::NULL1),static_cast<int>(json_get_type(&v)));

    v.type = json_type::FALSE;

    EXPECT_EQ_INT(static_cast<int>(return_json::INVALID_VALUE), json_parse(&v, "?"));  
    EXPECT_EQ_INT(static_cast<int>(json_type::NULL1),static_cast<int>(json_get_type(&v)));
}

static void test_parse_root_not_singular() {
    json_value v;
    v.type = json_type::FALSE;

    EXPECT_EQ_INT(static_cast<int>(return_json::ROOT_NOT_SINGULAR), json_parse(&v, "null x"));  
    EXPECT_EQ_INT(static_cast<int>(json_type::NULL1),static_cast<int>(json_get_type(&v)));
}

template<typename M,typename N>
inline void TEST_NUMBER(const M& expect, const N* json){
    json_value v;
    EXPECT_EQ_INT(static_cast<int>(return_json::OK), json_parse(&v, json));
    EXPECT_EQ_INT(static_cast<int>(json_type::NUMBER), static_cast<int>(json_get_type(&v)));
    EXPECT_EQ_DOUBLE(expect, static_cast<double>(json_get_number(&v)));
}

static void test_parse_string() {
    json_value v;
    v.type = json_type::FALSE;

    EXPECT_EQ_INT(static_cast<int>(return_json::OK), json_parse(&v, "\"strstr\""));
    EXPECT_EQ_INT(static_cast<int>(json_type::STRING),static_cast<int>(json_get_type(&v)));
    const char* ch = json_get_string(&v);
    cout << ch << endl;
}
static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number();
    
    test_parse_string();
}

static void test_stringify_null() {
    json_value v;
    v.type = json_type::NULL1;
    const char* x = json_stringify(&v);
    cout<< x <<endl;
}

static void test_stringify_true() {
    json_value v;
    v.type = json_type::TRUE;
    const char* x = json_stringify(&v);
    cout<< x <<endl;
}

static void test_stringify_false() {
    json_value v;
    v.type = json_type::FALSE;
    const char* x = json_stringify(&v);
    cout<< x <<endl;
 }

static void test_stringify_number() {
    json_value v;
    v.type = json_type::NUMBER;
    v.n = 1.21;
    const char* x = json_stringify(&v);
    cout<< x <<endl;
 }

static void test_stringify_string() {
    json_value v;
    v.type = json_type::STRING;
    v.s = "string";
    const char* x = json_stringify(&v);
    cout<< x <<endl;
}

static void test_stringify() {
    test_stringify_null();
    test_stringify_true();
    test_stringify_false();
    test_stringify_number();
    test_stringify_string();
}

int main() {
    json_value v;
    v.type = json_type::FALSE;
    test_parse();
    test_stringify();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}

