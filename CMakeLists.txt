cmake_minimum_required(VERSION 4.0)
project(json C)

set(CMAKE_C_STANDARD 23)

add_executable(json main.c
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
