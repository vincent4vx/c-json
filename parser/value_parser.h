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
    json_value_t* value;
} json_value_parser_result_t;

json_value_parser_result_t json_parse_value(const char* json, size_t length, json_arena_t* arena, json_parser_options_t options);

#endif //JSON_VALUE_PARSER_H
