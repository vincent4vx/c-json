//
// Created by vincent on 05/12/2025.
//

#ifndef JSON_FORMATER_H
#define JSON_FORMATER_H

#include "../type/types.h"

typedef enum {
    JSON_FORMATER_SUCCESS = 0,
    JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
    JSON_FORMATER_ERROR_INVALID_VALUE,
} json_formater_result_code_t;

typedef struct {
    json_formater_result_code_t code;
    union {
        struct {
            char message[32];
        } error;

        struct {
            size_t length;
            char* buffer;
        } result;
    };
} json_formater_result_t;

// @todo max depth, etc.
json_formater_result_t json_format_value(const json_value_t* value, char* buffer, size_t buffer_size);

#endif //JSON_FORMATER_H