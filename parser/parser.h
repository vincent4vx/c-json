//
// Created by vincent on 05/12/2025.
//

#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stddef.h>
#define JSON_ERROR_MESSAGE_SIZE 128

typedef struct {
    const size_t length;
    const char* value;
} json_raw_string_t;

typedef enum {
    /**
     * Parsing completed successfully
     */
    JSON_PARSE_SUCCESS = 0,

    /**
     * A syntax error was encountered in the JSON input, and the parser could not continue.
     */
    JSON_PARSE_ERROR_INVALID_SYNTAX,

    /**
     * The end of the JSON input was reached unexpectedly, indicating incomplete or malformed data.
     * This error can be ignored as it do not lead to invalid state.
     */
    JSON_PARSE_ERROR_UNEXPECTED_END,

    /**
     * The maximum allowed depth for nested JSON structures was exceeded during parsing.
     * Increase the max_depth parameter if necessary.
     */
    JSON_PARSE_ERROR_MAX_DEPTH,

    /**
     * The maximum allowed size for JSON strings was exceeded during parsing.
     * Increase the max_string_size parameter if necessary.
     */
    JSON_PARSE_ERROR_MAX_STRING_SIZE,

    /**
     * The maximum allowed size (or length) for JSON structures (arrays or objects) was exceeded during parsing.
     * Increase the max_struct_size parameter if necessary.
     */
    JSON_PARSE_ERROR_MAX_STRUCT_SIZE,

    /**
     * User defined handler function returned an error, aborting parsing.
     */
    JSON_PARSE_HANDLER_ERROR,

    /**
     * Error occurred in the parser configuration or internal state.
     * This type of error must not happen in normal conditions, indicating a bug in the code.
     */
    JSON_PARSE_CONFIG_ERROR,
} json_parse_code_t;

typedef struct {
    const json_parse_code_t result;
    char message[JSON_ERROR_MESSAGE_SIZE];
} json_parser_result_t;

typedef struct json_stream_handler_t {
    json_parser_result_t (*on_null)(struct json_stream_handler_t* self);
    json_parser_result_t (*on_bool)(struct json_stream_handler_t* self, bool value);
    json_parser_result_t (*on_number)(struct json_stream_handler_t* self, double value);
    json_parser_result_t (*on_string)(struct json_stream_handler_t* self, json_raw_string_t value);
    json_parser_result_t (*on_start_array)(struct json_stream_handler_t* self);
    json_parser_result_t (*on_end_array)(struct json_stream_handler_t* self);
    json_parser_result_t (*on_start_object)(struct json_stream_handler_t* self);
    json_parser_result_t (*on_object_property)(struct json_stream_handler_t* self, json_raw_string_t key);
    json_parser_result_t (*on_end_object)(struct json_stream_handler_t* self);
} json_stream_handler_t;

json_parser_result_t json_create_success_result();
json_parser_result_t json_create_error_result(json_parse_code_t result, const char* message);
json_parser_result_t json_parse(const char* json, size_t length, json_stream_handler_t* handler, size_t max_depth, size_t max_string_size, size_t max_struct_size);

#endif //JSON_PARSER_H
