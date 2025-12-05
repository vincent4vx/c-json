cmake_minimum_required(VERSION 4.0)
project(json C)

set(CMAKE_C_STANDARD 23)

add_executable(json main.c
        parser/stream_parser.h
        parser/stream_parser.c)
