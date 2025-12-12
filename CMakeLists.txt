cmake_minimum_required(VERSION 3.31)
project(json_parser C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_library(json
        parser/parser.h
        parser/parser.c
        type/types.h
        parser/value_parser.h
        parser/value_parser.c
        type/factory.c
        type/factory.h
        type/factory.c
        formater/formater.h
        formater/formater.c)

add_executable(app main.c)
target_link_libraries(app PRIVATE json)

add_executable(tests tests/tests.c
        tests/tests.h
        tests/parser_tests.c
        tests/value_parser_tests.c)
target_include_directories(tests PRIVATE tests)
target_link_libraries(tests PRIVATE json)

enable_testing()
add_test(NAME tests COMMAND tests)