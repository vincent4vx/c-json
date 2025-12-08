//
// Created by vincent on 05/12/2025.
//

#ifndef JSON_TYPES_FACTORY_H
#define JSON_TYPES_FACTORY_H

#include "types.h"

typedef struct {
    char* string_pool;
    size_t string_pool_size;
    size_t string_pool_used;

    json_value_t* value_pool;
    size_t value_pool_size;
    size_t value_pool_used;

    json_member_entry_t* key_pool;
    size_t key_pool_size;
    size_t key_pool_used;
} json_arena_t;

/**
 * Compute the expected size of a json arena given the sizes of its pools.
 */
size_t json_arena_size(size_t string_pool_size, size_t value_pool_size, size_t key_pool_size);

// @todo error for incohérent sizes
bool json_arena_init(json_arena_t* arena, size_t arena_size, size_t string_pool_size, size_t value_pool_size, size_t key_pool_size);

json_value_t* json_create_null_value(json_arena_t* arena);
json_value_t* json_create_bool_value(json_arena_t* arena, bool value);
json_value_t* json_create_number_value(json_arena_t* arena, double value);
json_value_t* json_create_string_value(json_arena_t* arena, const char* str, size_t length);
json_value_t* json_create_empty_array(json_arena_t* arena);
json_value_t* json_create_empty_object(json_arena_t* arena);
json_member_entry_t* json_create_array_member(json_arena_t* arena, int key, json_value_t* value);
json_member_entry_t* json_create_object_member(json_arena_t* arena, const char* key, size_t key_length);
// json_value_t json_create_object_value(const json_string_t* keys, const json_value_t* values, size_t length);

#endif //JSON_TYPES_FACTORY_H