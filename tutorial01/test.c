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

static void test_parse_true(){
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
    lept_value_init_type(&v);//初始化type = LEPT_NULL，表示此时没有任何类型

    lept_set_string(&v,"",0);//内部会设置 type = LEPT_STRING
    EXPECT_EQ_STRING("",lept_get_string(&v),0);//测试str数据
    lept_set_string(&v,"abc",3);
    EXPECT_EQ_STRING("abc",lept_get_string(&v),3);//测试str数据

}

static void test_parse() {
    test_parse_null();//测试 null 的解析函数是否正确
    test_parse_false();
    test_parse_true();
    test_parse_expect_value();//测试空白
    test_parse_invalid_value();
    test_parse_root_not_singular();

    test_parse_number();
    test_access_string();//测试string数据类型
}



int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
