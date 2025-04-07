

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

//用于存放 待解析的原始数据、解析完毕的数据 的结构
typedef struct {
    const char* json;//表示原始的待解析的json数据

    char* stack;//解析 数组、字符串、对象 数据类型数据时，先把解析完毕的字符，存入缓冲区，最后执行各数据类型的set方法时，再从缓冲区读取数据，写入lept_value
    size_t capacity, top;//堆栈容量，栈顶位置(栈当前存储了的字节数)（为什么要用栈的先进后出结构？）
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


lept_type lept_get_type(const lept_value* v)
{
    assert(v != NULL);
    return v->type;
}




//释放string数据类型的内存，以及初始化 type为LEPT_NULL，表示没有任何类型
// void lept_value_free_string(lept_value* v)
// {
//     assert(v != NULL);
//     if(v->type == LEPT_STRING)
//     {
//         free(v->uion.s.str);
//     }
//     // v->type = LEPT_NULL;
//     // lept_value_init_type(v);
// }

// 设置一个值为字符串
// "abcde",len = 5,但要开辟6个字节空间，最后一位放结束符
void lept_set_string(lept_value* v, const char* s, size_t len)
{
    assert(v != NULL && (s != NULL || len == 0));
    lept_value_free(v);//释放之前的内存，并重置type=NULL

    //字符串拷贝
    char* newStr = (char*)malloc(len+1);
    v->uion.s.str = newStr;
    memcpy(v->uion.s.str,s,len);//memcpy直接逐字节拷贝字符进去
    v->uion.s.str[len] = '\0';//字符串要以结束符 '\0'结尾
    v->uion.s.len = len;
    v->type = LEPT_STRING;
}
const char* lept_get_string(const lept_value* v)
{
    assert(v != NULL);
    assert(v->type == LEPT_STRING);
    return v->uion.s.str;
}
size_t lept_get_string_length(const lept_value* v)
{
    assert(v != NULL);
    assert(v->type == LEPT_STRING);
    return v->uion.s.len;
}


void lept_value_init(lept_value* v)
{
    assert(v != NULL);
    v->type = LEPT_NULL;//置为默认LEPT_NULL类型
}

void lept_value_free(lept_value* v)
{
    assert(v != NULL);
    if(v->type == LEPT_STRING && v->uion.s.str != NULL)//若是字符串类型，还要释放内存
    {
        free(v->uion.s.str);
    }
    v->type = LEPT_NULL;//置为默认LEPT_NULL类型
}

//缓冲栈区，push，pop
//push:把解析完毕的字符，压入栈缓冲区
//pop:从栈缓冲区中，取出解析完的数据，存入lept_value

//内存申请，stack内存空间的初始化、扩容
//一次性存储size字节数据 (对string来说是字符，对数组来说，是数组元素)
//这里栈实现是以<字节为单位>存储的。每次可要求压入任意大小size的数据，返回数据起始的指针
static void* lept_context_stack_realloc(lept_context* c,size_t size)
{
    assert(size > 0);

    if(c->top + size >= c->capacity)
    {
        //缓冲区根本没有初始化
        if(c->capacity == 0)
            c->capacity = LEPT_PARSE_STACK_INIT_SIZE;

        //缓冲区需要扩容
        while(c->top + size >= c->capacity)
            c->capacity = (c->capacity) * 1.5;

        c->stack = (char*)realloc(c->stack,c->capacity);
    }
    void* ret;
    //返回所插入的新数据的内存起始位置，后续真正push元素，从ret位置开始插入内存即可
    //c->stack + c->top： 指针运算，会按指针指向数据类型所占字节数进行批量偏移，
    //c->stach为char*，char-1字节，则c->stack+c->top表示自从c->stack偏移 1*top 个字节
    ret = c->stack + c->top;
    c->top = c->top + size;
    return ret;
}
//向栈压入一个char
void lept_context_stack_push(lept_context* c, char ch) {
    // *(char*)lept_context_push(c, sizeof(char)) = ch;
    void* ret = lept_context_stack_realloc(c,sizeof(char));//先为该ch申请内存（除非导致扩容，否则不需要申请，高效）
    *((char*)ret) = ch;//再 把字符实际存入栈
}

//弹出元素
//更新栈顶位置
static void* lept_context_stack_pop(lept_context* c, size_t size) {
    assert(c->top >= size);
    c->top = c->top - size;
    return c->stack + c->top;//返回新的栈顶位置
}




int lept_get_bool(const lept_value* v)
{
    assert(v != NULL);
    assert(v->type == LEPT_FALSE || v->type == LEPT_TRUE);
    return v->type == LEPT_TRUE; //或者 return !(type == LEPT_FALSE);
}

void lept_set_bool(lept_value* v,int boolval)
{
    assert(v != NULL);
    lept_value_free(v);
    v->type = boolval ? LEPT_TRUE : LEPT_FALSE;
}

void lept_set_number(lept_value* v, double n)
{
    assert(v != NULL);
    lept_value_free(v);
    v->uion.n = n;
    v->type = LEPT_NUMBER;
}

double lept_get_number(const lept_value* v)
{
    assert(v != NULL);
    assert(v->type == LEPT_NUMBER);
    return v->uion.n;
}




static int lept_parse_literal(lept_context* c, lept_value* v)
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

//解析字符串
//输入若是一个字符串，必然是左右带双引号的：
//   "abc"————字符串
//    abc ————不是字符串
// 而且实际 test.c中，用宏来测试的，传入字符串
static int lept_parse_string(lept_context* c, lept_value* v) { // "abc"
    
    //期望字符串的第一个字符是 引号，EXPECT会跳过这个字符，所以实际 双引号，被解析后没有存储起来
    //所以，输入的json字符串数据是包含两边双引号的，但是解析结果中只存储下来了双引号中间的所有字符
    EXPECT(c,'\"');
    size_t preTop = c->top;//此刻(还未加入解析完毕的字符)的栈顶
    size_t len;//记录栈内压入了len字节数据

    //c->josn原始输入的 const char* 字符串，const是修饰p指向内容，p本身可变
    //p指向当前遍历的字符
    const char* p = c->json;
    while(1)
    {
        char ch = *p++;//当前字符。后置++，先使用旧值p，执行*p，然后指针偏移p++
        switch(ch)
        {
            //遇到最右边的字符串结束引号，解析成功结束，将栈缓冲区内全部字符取出存入lept_value
            //注意，这个结束符 双引号，并没有被存储下来，只是作为标记，存储此前放入缓冲区的字符
            case '\"':
                len = c->top - preTop;//当前栈内已经push了len个字节(字符)
                void* ret = lept_context_stack_pop(c,len);//一次性取出len字节的字符数据，会更新c->top，下降的
                lept_set_string(v,(const char*)(ret),len);//把这len字节字符数据存入lept_value
                c->json = p;//更新指针到 待解析的位置
                return LEPT_PARSE_OK;
            case '\\'://转义字符，是中间部分的，不是起始位置，以及结束位置的，因为起始，结束位置的字符情况在其他case都处理过了
                switch (*p++)
                {
                    case '\"': lept_context_stack_push(c, '\"'); break;
                    case '\\': lept_context_stack_push(c, '\\'); break;
                    case '/':  lept_context_stack_push(c, '/' ); break;
                    case 'b':  lept_context_stack_push(c, '\b'); break;
                    case 'f':  lept_context_stack_push(c, '\f'); break;
                    case 'n':  lept_context_stack_push(c, '\n'); break;
                    case 'r':  lept_context_stack_push(c, '\r'); break;
                    case 't':  lept_context_stack_push(c, '\t'); break;
                    default:
                        // 解析错误处理，如果遇到非法的，则要回退c->top，因为此前可能有合法的字符已经被解析而缓存到栈中，
                        // 那么此时遇到非法字符没法解析，则此前的数据都要抛弃才合理
                        c->top = preTop;
                        return LEPT_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            case '\0'://能够执行至此，说明该输入字符串没有 以引号结束
                c->top = preTop;//解析错误处理
                return LEPT_PARSE_MISS_QUOTATION_MARK;
            default://正常字符
                if ((unsigned char)ch < 0x20) { //非法字符： ASCII 码小于 0x20 的控制字符
                    c->top = preTop;//解析错误处理
                    return LEPT_PARSE_INVALID_STRING_CHAR;
                }
                lept_context_stack_push(c,ch);//解析完毕的数据（此处为string的各字符）存入 栈缓冲区，内部会更新c->top栈顶
        }
    }
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
            return lept_parse_literal(c,v);//null,false,true类型的Json文本数据解析，整合为一个函数
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;//全空白
        case '"'://双引号，说明输入数据是字符串，需要解析字符串
            return lept_parse_string(c,v);
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