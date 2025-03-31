#ifndef LEPTJSON_H__
#define LEPTJSON_H__

// JSON 中有 6 种数据类型
typedef enum { 
    LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT 
} lept_type;

// Json是一个树形结构，由节点组成，最终需要实现一个树的数据结构，每个节点使用 lept_value 结构体表示
// 每个节点就是一个：key-value 中的 key
typedef struct {
    lept_type type;
}lept_value;

// lept_parse 返回值
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE, // 错误码：若一个 Json 只含有空白，传回 LEPT_PARSE_EXPECT_VALUE
    LEPT_PARSE_INVALID_VALUE, // 错误码：若值不是那三种字面值，传回 LEPT_PARSE_INVALID_VALUE
    LEPT_PARSE_ROOT_NOT_SINGULAR // 错误码：若一个值之后，在空白之后还有其他字符，传回 LEPT_PARSE_ROOT_NOT_SINGULAR
};

//解析Json，解析为一个树状数据结构(原始Json字符串解析为一个Json对象的数据结构)
//参数v: 存储json对象结果的指针
//参数json: 传入的Json文本内容，，例如 const char json[] = "true ";
int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(const lept_value* v);// 访问结果的函数，获取Json对象的数据类型

#endif /* LEPTJSON_H__ */

