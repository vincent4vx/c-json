#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formater/formater.h"
#include "parser/value_parser.h"
#include "type/types.h"

json_value_t* json_parse_cstr(char* cstr, json_arena_t* arena) {
    json_value_parser_result_t result = json_parse_value(cstr, strlen(cstr), arena, 32, 1024, 1024);

    if (result.result.code != JSON_PARSE_SUCCESS) {
        printf("Error parsing JSON: %s\n", result.result.message);
    }

    return result.value;
}

void json_print(json_value_t* value) {
    char buffer[512] = {0};

    if (value == nullptr) {
        printf("Error: Null value\n");
        return;
    }

    json_formater_result_t result = json_format_value(value, buffer, 512);

    if (result.code == JSON_FORMATER_SUCCESS) {
        printf("Formatted JSON: %*s\n", (int) result.result.length, result.result.buffer);
    } else {
        printf("Error formatting JSON: %s\n", result.error.message);
    }
}

int main(void) {
    const size_t string_pool_size = 1000000;
    const size_t value_pool_size = 1000;
    const size_t key_pool_size = 1000;
    const size_t arena_size = json_arena_size(string_pool_size, value_pool_size, key_pool_size);
    json_arena_t* arena = malloc(arena_size);
    json_arena_init(arena, arena_size, string_pool_size, value_pool_size, key_pool_size);

    json_print(json_parse_cstr("123.456", arena));
    json_print(json_parse_cstr("true", arena));
    json_print(json_parse_cstr("false", arena));
    json_print(json_parse_cstr("null", arena));
    json_print(json_parse_cstr("\"Hello, World!\"", arena));
    json_print(json_parse_cstr("[1, 2, 3]", arena));
    json_print(json_parse_cstr("{\"foo\": 456, \"bar\": [true, null, \"abcd\n\"]}", arena));

    return 0;
}
