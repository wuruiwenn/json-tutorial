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
    LEPT_PARSE_MISS_QUOTATION_MARK,//错误码，用于string类型数据解析，字符串没有正常以引号结束
};

//解析Json，解析为一个树状数据结构(原始Json字符串解析为一个Json对象的数据结构)
//参数v: 存储json对象结果的指针
//参数json: 传入的Json文本内容，，例如 const char json[] = "true ";
int lept_parse(lept_value* v, const char* json);

//所有数据类型的json数据的 写入、读取 （set、get）
//数据存入lept_value，lept_value相当于一个针对所有数据类型共用的一个结构体，用于存储数据
//它包括所存储数据本身(double n、char* str...)、以及数据的类型标识(type)

//set: 新数据，存入lept_value
//get: 从当前lept_value中读取数据即可，lept_value对象任意时刻只会是某一种数据类型的存储器

lept_type lept_get_type(const lept_value* v);// 获取Json对象的数据类型

double lept_get_number(const lept_value* v);//获取json数值，当且仅当json是nuber类型
void lept_set_number(lept_value* v, double n);//把数据n，写入lept_value

const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);

int lept_get_bool(const lept_value* v);
void lept_set_bool(lept_value* v,int boolval);

void lept_value_init_type(lept_value* v);//由于lept_value是所有Json数据类型共用的存器，所以每次用于存储新类型，要把内部初始化。包括type、数据本身...

void lept_value_free_string(lept_value* v);//释放string类型的内存空间

#endif

