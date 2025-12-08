//
// Created by vincent on 05/12/2025.
//

#include "factory.h"

#include <stdio.h>

size_t json_arena_size(const size_t string_pool_size, const size_t value_pool_size, const size_t key_pool_size) {
    return sizeof(json_arena_t)
        + string_pool_size * sizeof(char)
        + value_pool_size * sizeof(json_value_t)
        + key_pool_size * sizeof(json_member_entry_t)
    ;
}

bool json_arena_init(json_arena_t* arena, const size_t arena_size, const size_t string_pool_size, const size_t value_pool_size, const size_t key_pool_size) {
    if (arena_size != json_arena_size(string_pool_size, value_pool_size, key_pool_size)) {
        return false;
    }

    char* buffer = (char*) arena + sizeof(json_arena_t);

    arena->string_pool = buffer;
    arena->string_pool_size = string_pool_size;
    arena->string_pool_used = 0;

    buffer += string_pool_size * sizeof(char);
    arena->value_pool = (json_value_t*) buffer;
    arena->value_pool_size = value_pool_size;
    arena->value_pool_used = 0;

    buffer += value_pool_size * sizeof(json_value_t);
    arena->key_pool = (json_member_entry_t*) buffer;
    arena->key_pool_size = key_pool_size;
    arena->key_pool_used = 0;

    return true;
}

static json_value_t* json_arena_push_value(json_arena_t* arena, const json_value_t value) {
    if (arena->value_pool_used >= arena->value_pool_size) {
        return nullptr;
    }

    const size_t index = arena->value_pool_used++;
    arena->value_pool[index] = value;

    return &arena->value_pool[index];
}

json_value_t* json_create_null_value(json_arena_t* arena) {
    return json_arena_push_value(arena, (json_value_t) { .type = JSON_NULL });
}

json_value_t* json_create_bool_value(json_arena_t* arena, const bool value) {
    return json_arena_push_value(arena, (json_value_t) {
        .type = JSON_BOOL,
        .bool_value = value,
    });
}

json_value_t* json_create_number_value(json_arena_t* arena, const double value) {
    return json_arena_push_value(arena, (json_value_t) {
        .type = JSON_NUMBER,
        .number_value = value,
    });
}

typedef struct {
    const ssize_t length;
    char* value;
} json_internal_parsed_string_t;

static json_internal_parsed_string_t json_arena_parse_raw_string(json_arena_t* arena, const char* str, size_t str_length) {
    // @todo assert for surounding quotes
    bool is_escaped = false;
    const size_t start_index = arena->string_pool_used;
    ssize_t length = 0;

    // Skip the surrounding quotes
    for (size_t i = 1; i < str_length - 1; ++i) {
        if (arena->string_pool_used >= arena->string_pool_size) {
            return (json_internal_parsed_string_t) { .length = -1, .value = nullptr };
        }

        char current_char = str[i];

        if (!is_escaped && current_char == '\\') {
            is_escaped = true;
            continue;
        }

        if (is_escaped) {
            // @todo handle unicode escape sequences
            switch (current_char) {
                case 'n':
                    current_char = '\n';
                    break;
                case 'r':
                    current_char = '\r';
                    break;
                case 't':
                    current_char = '\t';
                    break;
                case 'b':
                    current_char = '\b';
                    break;
                case 'f':
                    current_char = '\f';
                    break;
                case '0':
                    current_char = '\0';
                    break;
                default:
                    // Keep the character as is for other escape sequences
                    break;
            }

            is_escaped = false;
        }

        arena->string_pool[arena->string_pool_used++] = current_char;
        ++length;
    }

    return (json_internal_parsed_string_t) {
        .length = length,
        .value = &arena->string_pool[start_index],
    };
}

json_value_t* json_create_string_value(json_arena_t* arena, const char* str, const size_t length) {
    const json_internal_parsed_string_t parsed = json_arena_parse_raw_string(arena, str, length);

    if (parsed.length < 0 || parsed.value == nullptr) {
        return nullptr;
    }

    return json_arena_push_value(arena, (json_value_t) {
        .type = JSON_STRING,
        .string_value = {
            .length = (size_t) parsed.length,
            .value = parsed.value,
        },
    });
}

json_value_t* json_create_empty_array(json_arena_t* arena) {
    return json_arena_push_value(arena, (json_value_t) {
        .type = JSON_ARRAY,
        .array_value = {
            .length = 0,
            .head = nullptr,
            .tail = nullptr,
        },
    });
}

json_value_t* json_create_empty_object(json_arena_t* arena) {
    return json_arena_push_value(arena, (json_value_t) {
        .type = JSON_OBJECT,
        .array_value = {
            .length = 0,
            .head = nullptr,
            .tail = nullptr,
        },
    });
}

json_member_entry_t* json_create_array_member(json_arena_t* arena, const int key, json_value_t* value) {
    // @todo assert value is in arena
    if (arena->key_pool_used >= arena->key_pool_size) {
        return nullptr;
    }

    const size_t index = arena->key_pool_used++;
    arena->key_pool[index] = (json_member_entry_t) {
        .key_int = key,
        .key_str = nullptr,
        .value = value,
        .next = nullptr,
    };

    return &arena->key_pool[index];
}

json_member_entry_t* json_create_object_member(json_arena_t* arena, const char* key, size_t key_length) {
    if (key == nullptr || arena->string_pool_used >= arena->string_pool_size) {
        return nullptr;
    }

    if (arena->key_pool_used >= arena->key_pool_size) {
        return nullptr;
    }

    const json_internal_parsed_string_t keystr = json_arena_parse_raw_string(arena, key, key_length);

    if (keystr.length < 0 || keystr.value == nullptr) {
        return nullptr;
    }

    const size_t index = arena->key_pool_used++;
    arena->key_pool[index] = (json_member_entry_t) {
        .key_int = (int) keystr.length,
        .key_str = keystr.value,
        .value = nullptr,
        .next = nullptr,
    };

    return &arena->key_pool[index];
}
