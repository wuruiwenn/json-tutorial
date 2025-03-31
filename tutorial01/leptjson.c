#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
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



//c指向了传入json值的第一个非空字符，v是存储c指向内容的 解析完毕的 结果type
/* 解析：value = null / false / true */
/* 提示：下面代码没处理 false / true，将会是练习之一 */
static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) 
    {
        case 'n':  return lept_parse_null(c, v);//null
        case 'f': return lept_parse_false(c,v);
        case 't': return lept_parse_true(c,v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;//全空白
        default:   return LEPT_PARSE_INVALID_VALUE;//非法的json值
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
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
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}
