//
// Created by vincent on 05/12/2025.
//

#ifndef JSON_TYPES_H
#define JSON_TYPES_H

#include <stddef.h>

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
} json_type_enum_t;

struct json_value_t;

typedef struct {
    const size_t length;
    const char value[];
} json_string_t;

typedef struct {
    const size_t length;
    const struct json_value_t *items;
} json_array_t;

typedef struct {
    const size_t length;
    const json_string_t *keys;
    const struct json_value_t *values;
} json_object_t;

typedef struct {
    const json_type_enum_t type;
    union {
        const bool bool_value;
        const double number_value;
        const json_string_t string_value;
        const json_array_t array_value;
        const json_object_t object_value;
    };
} json_value_t;

#endif //JSON_TYPES_H