#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */
#include <stdbool.h> // bool type
#include <errno.h> //ERANGE、errno
#include <math.h>    /* HUGE_VAL，-HUGE_VAL，正无穷，负无穷*/
#include <string.h> // strlen

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)  ((ch) >= '0' && (ch) <= '9')

//栈容量
#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

//用于存放 待解析的 原始数据 的结构
typedef struct {
    const char* json;

    char* stack;//解析 数组、字符串、对象 数据类型数据时，先把解析完毕的字符，存入缓冲区，最后执行各数据类型的set方法时，再从缓冲区读取数据，写入lept_value
    size_t capacity, top;//堆栈容量，栈顶位置（为什么要用栈的先进后出结构？）
}lept_context;



//略过空格、Tab、换行，找到第一个非空格字符
static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
    {
        p++;
    }
    c->json = p;
}

// //解析数值型json
// static double lept_parse_number(lept_context* c, lept_value* v)
// {
//     // assert(v != NULL && c)
// }

#if 0
//解析 为null的json值（指 "=" 右边的数据）
/* null  = "null" */
static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
    {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

//解析 false/true 的json值
static int lept_parse_false(lept_context* c, lept_value* v)
{
    EXPECT(c,'f');
    if(c->json[0]!='a' || c->json[1]!='l' || c->json[2] != 's' || c->json[3] != 'e')
    {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}
static int lept_parse_true(lept_context* c, lept_value* v)
{
    EXPECT(c,'t');
    if(c->json[0]!='r' || c->json[1]!='u' || c->json[2] != 'e')
    {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}
#endif

static int check_in_order(lept_context* c, lept_value* v,size_t n,const char* pattern,lept_type expectType)
{
    // EXPECT(c,str[0]);
    for(size_t i=1;i<n;i++)//第0个已经判断过了
    {
        if(c->json[i] != pattern[i])
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json += n;
    v->type = expectType;
    return LEPT_PARSE_OK;
}

static int lept_parse_type(lept_context* c, lept_value* v)
{
    char ch = *c->json;
    // assert(ch == 'n' || ch == 'f' || ch == 't');外部已经判断过了
    switch(ch)
    {
        case 'n':
            return check_in_order(c,v,4,"null",LEPT_NULL);
        case 'f':
            return check_in_order(c,v,5,"false",LEPT_FALSE);
        case 't':
            return check_in_order(c,v,4,"true",LEPT_TRUE);
    }
}



//解析number类型的Json文本
static int lept_parse_number(lept_context* c, lept_value* v) 
{
    errno = 0; // C 语言标准库中的一个全局变量，strtod函数会影响到这个值
    char* end;
    v->uion.n = strtod(c->json, &end);//解析的结果数值存储进lept_value

    //解析失败的一些场景
    if((!ISDIGIT(*c->json) && *c->json != '-') || !ISDIGIT(c->json[strlen(c->json)-1]))//首部出现非数字，除了是符号，其他都不合法。末尾出现非数字，全部不合法
    {
        return LEPT_PARSE_INVALID_VALUE;
    }
    if(*c->json == '+' || end == c->json || *end != '\0')//输入的字符串无法解析，部分解析成功(解析到某位置后面还有不能解析的字符)
    {
        return LEPT_PARSE_INVALID_VALUE;
    }
    // 解析成功的场景
    // 解析成功，但结果溢出范围，若同时是正无穷，负无穷，导致的溢出，则返回TOO_BIG。
    // 否则其他溢出情况，对于strtod来说，应该表达为 解析正常，溢出情况都解析为0
    if(errno == ERANGE && (v->uion.n == HUGE_VAL || v->uion.n == -HUGE_VAL))
    {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    // v->n = tmp;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}



//c指向了传入json值的第一个非空字符，v是存储c指向内容的 解析完毕的 结果type
/* 解析：value = null / false / true */
/* 提示：下面代码没处理 false / true，将会是练习之一 */
static int lept_parse_value(lept_context* c, lept_value* v) {
    //判断输入数据的类型，调用不同的具体解析方法
    switch (*c->json) 
    {
        case 'n':
        case 'f':
        case 't':
            return lept_parse_type(c,v);//null,false,true类型的Json文本数据解析，整合为一个函数
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;//全空白
        default:
            return lept_parse_number(c,v);
    }
}

//统一解析任何类型数据的函数
int lept_parse(lept_value* v, const char* json) {

    assert(v != NULL);

    //定义并初始化lept_context，lept_context用于存放原始输入的数据（char* json）
    //因为后续需要不断对输入数据进行解析动作，所以定义一个结构lept_context，方便传输
    lept_context c;
    c.json = json;
    v->type = LEPT_NULL;
    // lept_value_init_type(v);

    c.stack = NULL;//初始化用于缓冲的栈区
    c.capacity = c.top = 0;

    lept_parse_whitespace(&c);//略过空格,"space TTT space"

    //若字符之后还有其他字符，则应该返回LEPT_PARSE_ROOT_NOT_SINGULAR
    int ret;
    if((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK)
    {
        lept_parse_whitespace(&c);
        if(*(c.json) != '\0')
        {
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    // return lept_parse_value(&c, v);//正式解析所有可能的json值

    assert(c.top == 0);//在释放时，加入了assert确保所有数据都被弹出
    free(c.stack);

    return ret;
}

lept_type lept_get_type(const lept_value* v) 
{
    assert(v != NULL);
    return v->type;
}


double lept_get_number(const lept_value* v)
{
    assert(v != NULL);
    assert(v->type == LEPT_NUMBER);
    return v->uion.n;
}

//释放string数据类型的内存，以及初始化 type为LEPT_NULL，表示没有任何类型
void lept_value_free_string(lept_value* v)
{
    assert(v != NULL);
    if(v->type == LEPT_STRING)
    {
        free(v->uion.s.str);
    }
    // v->type = LEPT_NULL;
    lept_value_init_type(v);
}

// 设置一个值为字符串
// "abcde",len = 5,但要开辟6个字节空间，最后一位放结束符
void lept_set_string(lept_value* v, const char* s, size_t len)
{
    assert(v != NULL && (s != NULL || len == 0));

    lept_value_free_string(v);
    //字符串拷贝
    char* newStr = (char*)malloc(len+1);
    v->uion.s.str = newStr;
    memcpy(v->uion.s.str,s,len);
    v->uion.s.str[len] = '\0';//字符串要以结束符 '\0'结尾
    v->uion.s.len = len;
    v->type = LEPT_STRING;
}
const char* lept_get_string(const lept_value* v)
{
    assert(v->type == LEPT_STRING);
    return v->uion.s.str;
}


void lept_value_init_type(lept_value* v)
{
    assert(v != NULL);
    v->type = LEPT_NULL;
}

//缓冲栈区，push，pop
static void push(lept_context* c,size_t size)
{
    
}

static void pop(lept_context* c)
{

}