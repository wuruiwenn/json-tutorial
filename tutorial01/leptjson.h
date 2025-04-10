#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include<stddef.h> // size_t

// JSON 中有 6 种数据类型
// LEPT_NULL表示默认类型，即，当前json数据，没有设置任何类型
typedef enum {
    LEPT_NULL, 
    LEPT_FALSE, LEPT_TRUE, //bool类型
    LEPT_NUMBER, //number类型
    LEPT_STRING,  //字符串类型
    LEPT_ARRAY,  //数组 
    LEPT_OBJECT //对象
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


typedef struct lept_value lept_value;
typedef struct lept_member lept_member;

struct lept_value
{
    lept_type type;
    union//对于所有Json的数据类型(数组，对象，字符串，bool...)，用union结构表示，因为，任意时刻lept_value只会是其中一种数据类型
    {
        double number;//存储number数据
        struct //存储字符串数据
        {
            char* str;
            size_t len;
        }s;
        struct //存储Json数组 数据，json数组是由多个数组元素组成的，而数组元素可以是Json任何类型的数据
        {
            lept_value* arr;//Json数组用动态数组结构实现，本质就是一个指针，指向内容是 lept_value(因为数组内元素可以是任何类型数据)
            size_t arr_size;//数组元素个数
        }array;
        struct//存储json 对象类型的数据
        {
            lept_member* memb;//对象类型是很多个lept_member，所以肯定要用数组结构来表达，且是C语言中的动态数组,即一个指针指向每个元素的类型
            size_t object_size;//对象内部kmember个数，即k-v对的数量
        }object;
    }uion;
};

/*
    Json对象 数据结构 设计：
    Json对象，整体上和Json数组相似
    数组，由数组元素组成，数组元素本质是Json各种值(true/false，string，null...)
    对象，由member组成，
        每个member就是一个key-value对;
        而key只能是字符串;
        value可以是Json任何值，比如 字符串，number，true/false，null,这一点和json数组一样
*/
struct lept_member//Json对象的成员
{
    char* key; size_t key_length;  //member的key，因为key是string类型，所以，以及key的长度
    lept_value value;     //member的value
};


// lept_parse 返回值
enum {
    LEPT_PARSE_OK = 0,//解析成功
    LEPT_PARSE_EXPECT_VALUE, // 错误码：若一个 Json 只含有空白，传回 LEPT_PARSE_EXPECT_VALUE
    LEPT_PARSE_INVALID_VALUE, // 错误码：若值不是那三种字面值，传回 LEPT_PARSE_INVALID_VALUE
    LEPT_PARSE_ROOT_NOT_SINGULAR, // 错误码：若一个值之后，在空白之后还有其他字符，传回 LEPT_PARSE_ROOT_NOT_SINGULAR
    LEPT_PARSE_NUMBER_TOO_BIG ,//错误码，用于number类型数据解析，解析的结果数字数值溢出
    LEPT_PARSE_MISS_QUOTATION_MARK,//错误码，用于string类型数据解析，字符串没有正常以双引号结束 的场景
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    LEPT_PARSE_MISS_KEY,
    LEPT_PARSE_MISS_COLON,////colon:冒号，标记，缺少冒号 的解析错误
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

//解析Json，解析为一个树状数据结构(原始Json字符串解析为一个Json对象的数据结构)
//参数v: 存储json对象结果的指针
//参数json: 传入的Json文本内容，，例如 const char json[] = "true ";
int lept_parse(lept_value* v, const char* json);

//所有数据类型的json数据的 写入、读取 （set、get）
//数据存入lept_value，lept_value相当于一个针对所有数据类型共用的一个结构体，用于存储数据
//它包括所存储数据本身(double n、char* str...)、以及数据的类型标识(type)


// =================================get set ======================================
//set: 新数据，存入lept_value
//get: 从当前lept_value中读取数据即可，lept_value对象任意时刻只会是某一种数据类型的存储器
lept_type lept_get_type(const lept_value* v);// 获取Json对象的数据类型

const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);

double lept_get_number(const lept_value* v);//获取json数值，当且仅当json是nuber类型
void lept_set_number(lept_value* v, double n);//把数据n，写入lept_value

int lept_get_bool(const lept_value* v);
void lept_set_bool(lept_value* v,int boolval);

int lept_get_array_length(const lept_value* v);//获取数组长度
lept_value* lept_get_array_element(const lept_value* v,size_t indx);//获取数组元素，返回指针，指向元素的指针，返回元素本身不太好，因为你不知道该数组的该元素是什么类型
void lept_set_array(lept_value* v,lept_value* e,size_t size);

//对象类型数据 get/set
size_t lept_get_object_size(const lept_value* v);
const char* lept_get_object_key(const lept_value* v, size_t index);//获取json对象的第index个member的key
size_t lept_get_object_key_length(const lept_value* v, size_t index);
lept_value* lept_get_object_value(const lept_value* v, size_t index);//获取json对象的第index个member的value
// =================================get set ======================================

//重置 用于存储 当前json数据 的 结构体
//由于lept_value是所有Json数据类型共用的存器，所以每次用于存储新类型，要把内部初始化。
// 包括type、数据本身...是字符串类型，还得free释放动态申请的内存空间
void lept_value_init(lept_value* v);

void lept_value_free(lept_value* v);

// void lept_value_free_string(lept_value* v);//释放string类型的内存空间

#endif

