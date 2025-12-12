//
// Created by vincent on 05/12/2025.
//

#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stddef.h>
#include <stdint.h>

#define JSON_ERROR_MESSAGE_SIZE 128
#define JSON_DEFAULT_MAX_DEPTH 128
#define JSON_DEFAULT_MAX_STRING_SIZE 16384
#define JSON_DEFAULT_MAX_STRUCT_SIZE 256

/**
 * Represents a raw JSON string with its length.
 * The string is not parsed or null-terminated, and contains the exact characters as found in the JSON input
 * including quotes.
 */
typedef struct {
    /**
     * The length of the raw JSON string, including quotes.
     * So, an empty string ("") will have a length of 2.
     */
    const size_t length;

    /**
     * The pointer to the start of the raw JSON string in the input.
     */
    const char* value;
} json_raw_string_t;

typedef enum: uint8_t {
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

typedef enum: uint8_t {
    JSON_CONTEXT_UNKNOWN,
    JSON_CONTEXT_NULL,
    JSON_CONTEXT_BOOL,
    JSON_CONTEXT_NUMBER,
    JSON_CONTEXT_STRING,
    JSON_CONTEXT_ARRAY,
    JSON_CONTEXT_OBJECT,
    JSON_CONTEXT_OBJECT_PROPERTY,
} json_parse_context_t;

typedef enum: uint8_t {
    JSON_ERROR_UNKNOWN,
    JSON_ERROR_INVALID_CODE,
    JSON_ERROR_NULL_POINTER,
    JSON_ERROR_INVALID_MAX_DEPTH,
    JSON_ERROR_INVALID_MAX_STRING_SIZE,
    JSON_ERROR_INVALID_MAX_STRUCT_SIZE,
    JSON_ERROR_UNEXPECTED_CHARACTER,
    JSON_ERROR_CURSOR_NOT_ADVANCE,
    JSON_ERROR_CURSOR_EXCEED_INPUT,
    JSON_ERROR_EXPECTED_CONSTANT_LENGTH_TOO_LONG,
    JSON_ERROR_EMPTY_VALUE,
    JSON_ERROR_TOO_SMALL,
    JSON_ERROR_MISSING_CLOSING_CHARACTER,
    JSON_ERROR_OUT_OF_MEMORY,
    JSON_ERROR_NOT_SEQUENTIAL_KEYS,
    JSON_ERROR_PROPERTY_ALREADY_SET,
    JSON_ERROR_INVALID_TYPE,
    JSON_ERROR_STACK_OVERFLOW,
    JSON_ERROR_STACK_EMPTY,
} json_parse_error_t;

typedef struct {
    /**
     * The result code.
     * If not `JSON_PARSE_SUCCESS`, indicates an error occurred during parsing.
     *
     * The value of the following fields is only valid if code is not `JSON_PARSE_SUCCESS`.
     */
    const json_parse_code_t code;

    /**
     * The parser context which led to this result.
     */
    const json_parse_context_t context;

    /**
     * A more specific error code, if applicable.
     */
    const json_parse_error_t error;

    /**
     * Extra information about the error, if applicable.
     * Can be a character code, a length, or other small data.
     */
    const uint8_t extra;

    /**
     * The cursor position in the input JSON string where the error occurred.
     */
    const size_t position;
} json_parser_result_t;

/**
 * Define callbacks for JSON parsing nodes.
 * Set to nullptr any callback you do not wish to handle.
 *
 * All callbacks receive a pointer to the handler itself as first argument,
 * allowing to maintain state between calls if needed.
 *
 * Note: for string parameters, the input buffer will be used directly without copying.
 * So, you must copy the data if you need to keep it after the parsing is complete.
 */
typedef struct json_parser_handler_t {
    json_parser_result_t (*on_null)(struct json_parser_handler_t* self);
    json_parser_result_t (*on_bool)(struct json_parser_handler_t* self, bool value);
    json_parser_result_t (*on_number)(struct json_parser_handler_t* self, double value);
    json_parser_result_t (*on_string)(struct json_parser_handler_t* self, json_raw_string_t value);
    json_parser_result_t (*on_array_start)(struct json_parser_handler_t* self);
    json_parser_result_t (*on_array_end)(struct json_parser_handler_t* self);
    json_parser_result_t (*on_object_start)(struct json_parser_handler_t* self);
    json_parser_result_t (*on_object_property)(struct json_parser_handler_t* self, json_raw_string_t key);
    json_parser_result_t (*on_object_end)(struct json_parser_handler_t* self);
} json_parser_handler_t;

typedef struct {
    /**
     * The maximum allowed depth for nested JSON structures.
     * With a depth of 0, only top-level values are allowed (no arrays or objects).
     *
     * If this depth is exceeded, `JSON_PARSE_ERROR_MAX_DEPTH` error will be returned.
     */
    size_t max_depth;

    /**
     * The maximum allowed size for JSON strings, in characters.
     * Note: this size is for raw string, i.e. includes the surrounding quotes and escape characters.
     *
     * If this size is exceeded, `JSON_PARSE_ERROR_MAX_STRING_SIZE` error will be returned.
     */
    size_t max_string_size;

    /**
     * The maximum number of elements / properties allowed in JSON structures (arrays or objects).
     *
     * If this size is exceeded, `JSON_PARSE_ERROR_MAX_STRUCT_SIZE` error will be returned.
     */
    size_t max_struct_size;
} json_parser_options_t;

/**
 * Create a success result for JSON parsing.
 */
json_parser_result_t json_create_success_result();

/**
 * Fill the given json_parser_options_t structure with default values for any field set to zero.
 */
json_parser_options_t json_default_parser_options(json_parser_options_t options);

/**
 * Parse the JSON input string with the given length, invoking the provided handler callbacks.
 *
 * @param length The length of the JSON input string.
 * @param json The JSON input string to parse. Null-terminated is not required.
 */
json_parser_result_t json_parse(size_t length, const char json[length], json_parser_handler_t* handler, json_parser_options_t options);

/**
 * Get a human-readable error message for the given parser result.
 * The result will be a static null-terminated string, do not free it.
 */
char* json_parse_error_message(json_parser_result_t result);

#endif //JSON_PARSER_H
