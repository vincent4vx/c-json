//
// Created by vincent on 05/12/2025.
//

#ifndef JSON_TYPES_H
#define JSON_TYPES_H

#include <stddef.h>
#include <stdint.h>

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
    size_t length;
    char* value;
} json_string_t;

typedef struct json_member_entry_t {
    /**
     * The key as integer (for array index) or the key length (for object property name)
     */
    int32_t key_int;

    /**
     * The key as string (for object property name)
     * Must be null for array index keys
     */
    char* key_str;

    /**
     * The value associated to the key
     */
    struct json_value_t* value;

    /**
     * Pointer to the next member in the array or object
     * Null if this is the last member
     */
    struct json_member_entry_t* next;
} json_member_entry_t;

typedef struct {
    size_t length;
    json_member_entry_t* head;
    json_member_entry_t* tail;
} json_array_t;

typedef struct {
    size_t length;
    json_member_entry_t* head;
    json_member_entry_t* tail;
} json_object_t;

typedef struct json_value_t {
    json_type_enum_t type;
    union {
        bool bool_value;
        double number_value;
        json_string_t string_value;
        json_array_t array_value;
        json_object_t object_value;
    };
} json_value_t;

#endif //JSON_TYPES_H