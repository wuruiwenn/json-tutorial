cmake_minimum_required (VERSION 2.6)
project (leptjson_test C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
    set(CMAKE_C_STANDARD 99)  # 或者 C11, C17，使用C99标准，才能使用//作为注释符号
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra") 
endif()

add_library(leptjson leptjson.c)
add_executable(leptjson_test test.c)
target_link_libraries(leptjson_test leptjson)


set(CMAKE_C_STANDARD 99)  # 或者 11, 17, 23

set(CMAKE_BUILD_TYPE Debug) 
# 告诉cmake——想要用 Debug 模式 编译项目，指定cmake构建模式的一种方式，主要用来控制编译参数，比如是否 包含调试信息、是否优化等
# 指定为Debug模式，则会生成调试信息（用于 GDB、Valgrind 等）