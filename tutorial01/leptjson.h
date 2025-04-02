#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include<stddef.h> // size_t

// JSON 中有 6 种数据类型
typedef enum { 
    LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT 
} lept_type;

//用于存储解析结果
    // 包括：解析的结果类型，比如number类型、FALSE类型...
          // 解析的结果，number类型的数据解析要存储本身解析的结果数据
// Json是一个树形结构，由节点组成，最终需要实现一个树的数据结构，每个节点使用 lept_value 结构体表示
// 每个节点就是一个：key-value 中的 key
//用于存储json数据解析结果的 结构体
// 例如：lept_type 用于存储false/false/null类型json数据的类型
// typedef struct {
//     lept_type type;
//     double n;//用于存储数值型json数据的本身的数据，也就是说，仅用于 type==LEPT_NUMBER时的数据存储

//     char* str;//用于存储 字符串类型 的Json数据
//     size_t len;
// }lept_value;

typedef struct {
    lept_type type;
    union
    {
        double n;//存储number数据
        struct //存储字符串数据
        {
            char* str;
            size_t len;
        }s;
    }uion;
}lept_value;



// lept_parse 返回值
enum {
    LEPT_PARSE_OK = 0,//解析成功
    LEPT_PARSE_EXPECT_VALUE, // 错误码：若一个 Json 只含有空白，传回 LEPT_PARSE_EXPECT_VALUE
    LEPT_PARSE_INVALID_VALUE, // 错误码：若值不是那三种字面值，传回 LEPT_PARSE_INVALID_VALUE
    LEPT_PARSE_ROOT_NOT_SINGULAR, // 错误码：若一个值之后，在空白之后还有其他字符，传回 LEPT_PARSE_ROOT_NOT_SINGULAR
    LEPT_PARSE_NUMBER_TOO_BIG ,//错误码，用于number类型数据解析，解析的结果数字数值溢出
};

//解析Json，解析为一个树状数据结构(原始Json字符串解析为一个Json对象的数据结构)
//参数v: 存储json对象结果的指针
//参数json: 传入的Json文本内容，，例如 const char json[] = "true ";
int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(const lept_value* v);// 获取Json对象的数据类型
double lept_get_number(const lept_value* v);//获取json数值，当且仅当json是nuber类型

void lept_set_string(lept_value* v, const char* s, size_t len);



#endif /* LEPTJSON_H__ */

