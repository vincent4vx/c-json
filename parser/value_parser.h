//
// Created by vincent on 05/12/2025.
//

#ifndef JSON_VALUE_PARSER_H
#define JSON_VALUE_PARSER_H

#include "../type/types.h"
#include "../type/factory.h"
#include "parser.h"

typedef struct {
    json_parser_result_t result;

    /**
     * The parsed JSON value.
     * In case of JSON object or array, this is the root value.
     *
     * This value can be nullptr if an error occurred during parsing.
     */
    json_value_t* value;
} json_value_parser_result_t;

/**
 * Parse the JSON string as a value, returning the resulting JSON value.
 *
 * @param length The length of the JSON input string.
 * @param json The JSON input string to parse. Null-terminated is not required.
 * @param arena The arena to use for allocating JSON values and strings.
 * @param stack_size The size of the internal stack used for parsing nested structures. This value should be equals to `options.max_depth`.
 * @param stack The stack to use for parsing nested structures.
 * @param options The parser options to use.
 */
json_value_parser_result_t json_parse_value(size_t length, const char json[length], json_arena_t* arena, size_t stack_size, json_value_t* stack[stack_size], json_parser_options_t options);

/**
 * Parse the JSON string as a value, using default options.
 * The maximum depth is set to 32.
 *
 * @param length The length of the JSON input string.
 * @param json The JSON input string to parse. Null-terminated is not required.
 * @param arena The arena to use for allocating JSON values and strings.
 */
json_value_parser_result_t json_parse_value_defaults(size_t length, const char json[length], json_arena_t* arena);

#endif //JSON_VALUE_PARSER_H
