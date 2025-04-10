

#include <stdio.h>
#include <stdlib.h> /* NULL, strtod() */
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

//测试单元的核心：判断，期望值 和 解析结果数据(数据、类型) 是否一致
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_STRING(expect_str,actual_str,actual_len) \
        EXPECT_EQ_BASE((actual_len == sizeof(expect_str)-1) && (memcmp(expect_str,actual_str,actual_len) == 0),\
        expect_str,actual_str,"%s")

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define TEST_ERROR(error, json)\
    do {\
        lept_value v;\
        v.type = LEPT_FALSE;\
        EXPECT_EQ_INT(error, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));\
    } while(0)

#define TEST_NUMBER(number,json)\
    do {\
        lept_value v;\
        v.type = LEPT_NUMBER;\
        EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,json)); /*解析结果是否正确*/  \
        EXPECT_EQ_INT(LEPT_NUMBER,lept_get_type(&v));  /*解析结果的类型是否正确*/  \
        EXPECT_EQ_INT(number,lept_get_number(&v)); /*解析的number类型数据，解析得到的数值是否正确*/  \
    }while(0)

#define TEST_STRING(ExpetStr,InputStr) \
    do { \
        lept_value v; \
        v.type = LEPT_STRING; \
        EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,InputStr));\
        EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
        EXPECT_EQ_STRING(ExpetStr,lept_get_string(&v),lept_get_string_length(&v));\
        lept_value_free(&v);\
    }while(0)


#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#if defined(_MSC_VER) //_MSC_VER 是 MSVC 编译器预定义的宏
    #define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
    #define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif


static void test_parse_null() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));

    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nu ll"));
    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null null"));
}
static void test_parse_false() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"false"));
    EXPECT_EQ_INT(LEPT_FALSE,lept_get_type(&v));
}

static void test_parse_true() {
    lept_value v;
    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"true"));
    EXPECT_EQ_INT(LEPT_TRUE,lept_get_type(&v));
}

static void test_parse_expect_value() {//测试空白，expect == 空白...
    lept_value v;

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, ""));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, " "));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

//测试一些错误情况，是否如我们预期那样
static void test_parse_invalid_value() {
    // lept_value v;
    // v.type = LEPT_FALSE;
    // EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nul"));
    // EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));

    // v.type = LEPT_FALSE;
    // EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "?"));

    //关于 null，false，true类型的一些错误 用例
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"nul");//"nul"肯定是非法json文本，我们期望解析得到：LEPT_PARSE_INVALID_VALUE
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"nu ll");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"?");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"fals e");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"True");
    
    //关于 number类型 的Json数据 的一些错误场景 的测试用例
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null x"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}
static void test_parse_number()
{
    //测试number类型数据，解析结果，是否如我们预期，得到正确的解析数值
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

    TEST_NUMBER(0.0, "1e-10000"); 
    /* must underflow 这个应该是最特殊的一个，下溢出，很小的值，接近0，strtod会将其解析为0 */ 
    //所以，解析为0，属于 期望的解析结果

    //一些离谱的边界值测试
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

//测试 string数据类型的get、set
static void test_access_string()
{
    lept_value v;
    lept_value_init(&v);//初始化type = LEPT_NULL，表示此时没有任何类型

    lept_set_string(&v,"",0);//内部会设置 type = LEPT_STRING
    EXPECT_EQ_STRING("",lept_get_string(&v),0);//测试str数据
    lept_set_string(&v,"abcd",4);
    EXPECT_EQ_STRING("abcd",lept_get_string(&v),4);//测试str数据

    lept_value_free(&v);//注释，用来测试 valgrind的内存泄漏检测效果
    //对于 分配 "abc" 的字符串 内存来说，实际分配了4字节，因为还有结束符
}

//解析string，测试
static void test_parse_string()
{
    TEST_STRING("Hello", "\"Hello\"");//真正输入是 \"Hello\"，实际为 "Hello"

    TEST_STRING("", "\"\"");

    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_access_bool() {
    lept_value v;
    lept_value_init(&v);
    lept_set_string(&v, "a", 1);
    lept_set_bool(&v, 1);
    EXPECT_TRUE(lept_get_bool(&v));
    lept_set_bool(&v, 0);
    EXPECT_FALSE(lept_get_bool(&v));
    lept_value_free(&v);
}

static void test_access_number() {
    lept_value v;
    lept_value_init(&v);
    lept_set_string(&v, "a", 1);
    lept_set_number(&v, 1234.5);
    EXPECT_EQ_DOUBLE(1234.5, lept_get_number(&v));
    lept_value_free(&v);
}
static void test_parse_array() {
    size_t i, j;
    lept_value v;
    lept_value_init(&v);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
    EXPECT_EQ_SIZE_T(5, lept_get_array_length(&v));
    EXPECT_EQ_INT(LEPT_NULL,   lept_get_type(lept_get_array_element(&v, 0)));
    EXPECT_EQ_INT(LEPT_FALSE,  lept_get_type(lept_get_array_element(&v, 1)));
    EXPECT_EQ_INT(LEPT_TRUE,   lept_get_type(lept_get_array_element(&v, 2)));
    EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_array_element(&v, 3)));
    EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_array_element(&v, 4)));
    EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_array_element(&v, 3)));
    EXPECT_EQ_STRING("abc", lept_get_string(lept_get_array_element(&v, 4)), lept_get_string_length(lept_get_array_element(&v, 4)));
    lept_value_free(&v);

    lept_value_init(&v);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));//包含空白字符的测试
    EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
    EXPECT_EQ_SIZE_T(4, lept_get_array_length(&v));
    for (i = 0; i < 4; i++) {
        lept_value* a = lept_get_array_element(&v, i);
        EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(a));
        EXPECT_EQ_SIZE_T(i, lept_get_array_length(a));
        for (j = 0; j < i; j++) {
            lept_value* e = lept_get_array_element(a, j);
            EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
            EXPECT_EQ_DOUBLE((double)j, lept_get_number(e));
        }
    }
    lept_value_free(&v);
}

static void test_parse_miss_key() {
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{1:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{true:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{false:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{null:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{[]:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{{}:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {//colon:冒号
    TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\"}");
    TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}
static void test_parse_object()
{
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
}

static void test_parse() {
    // test_parse_null();//测试 null 的解析函数是否正确
    // test_parse_false();
    // test_parse_true();
    // test_parse_expect_value();//测试空白
    // test_parse_invalid_value();
    // test_parse_root_not_singular();
    // test_parse_number();

    // //以下可能产生内存泄漏
    // test_access_string();//测试string数据类型

    // test_parse_string();//测试string的解析

    // test_access_bool();
    // test_access_number();

    // test_parse_array();

    test_parse_object();
   
}



int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
