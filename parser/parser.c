#include "parser.h"

#include <stdio.h>

typedef struct {
    const char* json;
    const size_t length;
    json_parser_handler_t* handler;
    const size_t max_depth;
    const size_t max_string_size;
    const size_t max_struct_size;
    size_t position;
} json_stream_parser_state_t;

json_parser_result_t json_create_success_result() {
    return (json_parser_result_t) {
        .result = JSON_PARSE_SUCCESS,
        .message = {0},
    };
}

json_parser_result_t json_create_error_result(const json_parse_code_t result, const char* message) {
    if (result < 0 || result > JSON_PARSE_CONFIG_ERROR) {
        json_parser_result_t result_out = { .result = JSON_PARSE_CONFIG_ERROR };
        snprintf(result_out.message, JSON_ERROR_MESSAGE_SIZE, "Unknown error code: %d", result);

        return result_out;
    }

    json_parser_result_t result_out = { .result = result };
    snprintf(result_out.message, JSON_ERROR_MESSAGE_SIZE, "%s", message);

    return result_out;
}

static json_parser_result_t json_parse_value(json_stream_parser_state_t* state, size_t depth);
static json_parser_result_t json_parse_string(json_stream_parser_state_t* state, size_t depth);
static json_parser_result_t json_parse_object(json_stream_parser_state_t* state, size_t depth);
static json_parser_result_t json_parse_array(json_stream_parser_state_t* state, size_t depth);
static json_parser_result_t json_parse_number(json_stream_parser_state_t* state, size_t depth);
static json_parser_result_t json_parse_boolean(json_stream_parser_state_t* state, bool expected_value, size_t depth);
static json_parser_result_t json_parse_null(json_stream_parser_state_t* state, size_t depth);

json_parser_result_t json_parse(const char* json, size_t length, json_parser_handler_t* handler, const size_t max_depth, const size_t max_string_size, const size_t max_struct_size) {
    if (json == nullptr || length == 0 || handler == nullptr) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Invalid configuration: null pointer or zero length");
    }

    if (max_depth < 1 || max_depth > 1000000) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Invalid configuration: max_depth parameter out of bounds");
    }

    if (max_string_size < 1 || max_struct_size < 1) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Invalid configuration: max_string_size and max_struct_size parameters must be greater than zero");
    }

    json_stream_parser_state_t state = {
        .json = json,
        .length = length,
        .handler = handler,
        .max_depth = max_depth,
        .max_string_size = max_string_size,
        .max_struct_size = max_struct_size,
        .position = 0,
    };

    return json_parse_value(&state, 0);
}

static bool skip_whitespace(json_stream_parser_state_t* state) {
    while (state->position < state->length) {
        const char current_char = state->json[state->position];

        if (current_char == ' ' || current_char == '\n' || current_char == '\r' || current_char == '\t') {
            ++state->position;
            continue;
        }

        break;
    }

    return state->position < state->length;
}

static json_parser_result_t json_parse_value_inner_switch(json_stream_parser_state_t* state, const char current_char, const size_t depth) {
    switch (current_char) {
        case 'n':
            return json_parse_null(state, depth + 1);

        case 't':
            return json_parse_boolean(state, true, depth + 1);

        case 'f':
            return json_parse_boolean(state, false, depth + 1);

        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return json_parse_number(state, depth + 1);

        case '"':
            return json_parse_string(state, depth + 1);

        case '{':
            return json_parse_object(state, depth + 1);

        case '[':
            return json_parse_array(state, depth + 1);

        default:
            json_parser_result_t result = { .result = JSON_PARSE_ERROR_INVALID_SYNTAX };
            snprintf(result.message, JSON_ERROR_MESSAGE_SIZE, "Syntax error: unexpected character '%c' at position %zu", current_char, state->position);
            return result;
    }
}

static json_parser_result_t json_parse_value(json_stream_parser_state_t* state, const size_t depth) {
    if (state == nullptr) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Internal error: null parser state");
    }

    if (depth > state->max_depth) {
        return json_create_error_result(JSON_PARSE_ERROR_MAX_DEPTH, "Maximum parsing depth exceeded");
    }

    if (!skip_whitespace(state)) {
        return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input");
    }

    const size_t position = state->position;
    const char current_char = state->json[position];
    const json_parser_result_t result = json_parse_value_inner_switch(state, current_char, depth);

    if (result.result < 0 || result.result > JSON_PARSE_CONFIG_ERROR) {
        json_parser_result_t result_out = { .result = JSON_PARSE_CONFIG_ERROR };
        snprintf(result_out.message, JSON_ERROR_MESSAGE_SIZE, "Unknown error code: %d", result.result);

        return result_out;
    }

    if (result.result != JSON_PARSE_SUCCESS) {
        return result;
    }

    if (state->position <= position) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Parser did not advance position");
    }

    if (state->position > state->length) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Parser position exceeded input length");
    }

    return result;
}

static json_parser_result_t json_parse_constant(json_stream_parser_state_t* state, const char* expected_value, const size_t expected_value_length, const size_t depth) {
    if (state == nullptr) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Internal error: null parser state");
    }

    if (expected_value_length > 100) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Internal error: expected_value_length too large");
    }

    if (depth > state->max_depth) {
        return json_create_error_result(JSON_PARSE_ERROR_MAX_DEPTH, "Maximum parsing depth exceeded");
    }

    if (state->length - state->position < expected_value_length) {
        return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input");
    }

    for (int i = 0; i < expected_value_length; ++i) {
        const char current_char = state->json[state->position + i];

        if (current_char != expected_value[i]) {
            json_parser_result_t result = { .result = JSON_PARSE_ERROR_INVALID_SYNTAX };
            snprintf(result.message, JSON_ERROR_MESSAGE_SIZE, "Syntax error: expected '%s' but get '%c' at position %zu", expected_value, current_char, state->position + i);
            return result;
        }
    }

    state->position += expected_value_length;
    return json_create_success_result();
}

static json_parser_result_t json_parse_null(json_stream_parser_state_t* state, const size_t depth) {
    const json_parser_result_t result = json_parse_constant(state, "null", 4, depth);

    if (
        result.result != JSON_PARSE_SUCCESS
        || state->handler->on_null == nullptr
    ) {
        return result;
    }

    return state->handler->on_null(state->handler);
}

static json_parser_result_t json_parse_boolean(json_stream_parser_state_t* state, const bool expected_value, const size_t depth) {
    const json_parser_result_t result = expected_value == true
        ? json_parse_constant(state, "true", 4, depth)
        : json_parse_constant(state, "false", 5, depth)
    ;

    if (
        result.result != JSON_PARSE_SUCCESS
        || state->handler->on_bool == nullptr
    ) {
        return result;
    }

    return state->handler->on_bool(state->handler, expected_value);
}

static json_parser_result_t json_parse_number(json_stream_parser_state_t* state, const size_t depth) {
    if (state == nullptr) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Internal error: null parser state");
    }

    if (depth > state->max_depth) {
        return json_create_error_result(JSON_PARSE_ERROR_MAX_DEPTH, "Maximum parsing depth exceeded");
    }

    if (state->position >= state->length) {
        return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input");
    }

    const size_t start_position = state->position;
    const size_t max_position = start_position + 328 > state->length ? state->length : start_position + 328; // 328 is the maximum length of a float64 using decimal notation
    bool has_decimal_point = false;

    // @todo better parser
    double integer_part = 0.0;
    double fractional_part = 0.0;
    double sign = 1.0;

    for (; state->position < max_position; ++state->position) {
        const char current_char = state->json[state->position];

        if (current_char >= '0' && current_char <= '9') {
            const double digit_value = (double)(current_char - '0');

            if (!has_decimal_point) {
                integer_part = integer_part * 10.0 + digit_value;
            } else {
                fractional_part = fractional_part * 10.0 + digit_value;
            }
            continue;
        }

        if (current_char == '-') {
            if (state->position != start_position) {
                break;
            }

            sign = -1.0;
            continue;
        }

        if (current_char == '.') {
            if (has_decimal_point) {
                break;
            }

            has_decimal_point = true;
            continue;
        }

        break; // Invalid character for number
    }

    while (fractional_part >= 1.0) {
        fractional_part /= 10.0;
    }

    if (state->handler->on_number == nullptr) {
        return json_create_success_result();
    }

    const double number_value = sign * (integer_part + fractional_part);
    return state->handler->on_number(state->handler, number_value);
}

static json_parser_result_t json_parse_string_internal(json_stream_parser_state_t* state, const size_t depth, const bool is_property_key) {
    if (state == nullptr) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Internal error: null parser state");
    }

    if (depth > state->max_depth) {
        return json_create_error_result(JSON_PARSE_ERROR_MAX_DEPTH, "Maximum parsing depth exceeded");
    }

    if (state->position + 1 >= state->length) {
        return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input");
    }

    if (state->json[state->position] != '"') {
        return json_create_error_result(JSON_PARSE_ERROR_INVALID_SYNTAX, "Syntax error: expected '\"' at the beginning of string");
    }

    // Skip the opening quote
    const size_t start_position = state->position++;
    bool is_escaped = false;
    bool end = false;

    for (; state->position < state->length; ++state->position) {
        if (state->position - start_position >= state->max_string_size) {
            return json_create_error_result(JSON_PARSE_ERROR_MAX_STRING_SIZE, "Maximum string size exceeded");
        }

        const char current_char = state->json[state->position];

        if (is_escaped) {
            is_escaped = false;
            continue;
        }

        if (current_char == '\\') {
            is_escaped = true;
            continue;
        }

        if (current_char == '"') {
            end = true;
            break;
        }
    }

    if (!end) {
        return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input while parsing string");
    }

    // Move to the next character after the closing quote
    ++state->position;

    json_parser_result_t (*string_handler)(json_parser_handler_t*, json_raw_string_t) = is_property_key
        ? state->handler->on_object_property
        : state->handler->on_string
    ;

    if (string_handler == nullptr) {
        return json_create_success_result();
    }

    const size_t string_length = state->position - start_position;
    const json_raw_string_t raw_string = {
        .length = string_length,
        .value = &state->json[start_position],
    };

    return string_handler(state->handler, raw_string);
}

static json_parser_result_t json_parse_string(json_stream_parser_state_t* state, const size_t depth) {
    return json_parse_string_internal(state, depth, false);
}

static json_parser_result_t json_parse_object(json_stream_parser_state_t* state, const size_t depth) {
    if (state == nullptr) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Internal error: null parser state");
    }

    if (depth > state->max_depth) {
        return json_create_error_result(JSON_PARSE_ERROR_MAX_DEPTH, "Maximum parsing depth exceeded");
    }

    if (state->position + 1 >= state->length) {
        return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input");
    }

    if (state->json[state->position] != '{') {
        return json_create_error_result(JSON_PARSE_ERROR_INVALID_SYNTAX, "Syntax error: expected '{' at the beginning of an object");
    }

    // Skip the opening bracket
    ++state->position;

    if (state->handler->on_object_start != nullptr) {
        const json_parser_result_t start_array_result = state->handler->on_object_start(state->handler);

        if (start_array_result.result != JSON_PARSE_SUCCESS) {
            return start_array_result;
        }
    }

    bool end = false;
    bool property_expected = true;

    for (size_t len = 0; state->position < state->length; ++len) {
        if (len > state->max_struct_size) {
            return json_create_error_result(JSON_PARSE_ERROR_MAX_STRUCT_SIZE, "Maximum object size exceeded");
        }

        if (!skip_whitespace(state)) {
            return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input while parsing object");
        }

        const char current_char = state->json[state->position];

        if (current_char == '}') {
            ++state->position;
            end = true;
            break;
        }

        if (current_char == ',') {
            if (property_expected == true) {
                return json_create_error_result(JSON_PARSE_ERROR_INVALID_SYNTAX, "Syntax error: unexpected ',' in object");
            }

            property_expected = true;
            ++state->position;
            continue;
        }

        if (!property_expected) {
            return json_create_error_result(JSON_PARSE_ERROR_INVALID_SYNTAX, "Syntax error: expected ',' between object properties");
        }

        property_expected = false;

        const json_parser_result_t key_result = json_parse_string_internal(state, depth + 1, true);
        if (key_result.result != JSON_PARSE_SUCCESS) {
            return key_result;
        }

        if (!skip_whitespace(state)) {
            return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input while parsing object");
        }

        if (state->position >= state->length || state->json[state->position] != ':') {
            return json_create_error_result(JSON_PARSE_ERROR_INVALID_SYNTAX, "Syntax error: expected ':' after object property key");
        }

        ++state->position;

        const json_parser_result_t value_result = json_parse_value(state, depth + 1);
        if (value_result.result != JSON_PARSE_SUCCESS) {
            return value_result;
        }
    }

    if (!end) {
        return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input while parsing object");
    }

    if (state->handler->on_object_end == nullptr) {
        return json_create_success_result();
    }

    return state->handler->on_object_end(state->handler);
}

static json_parser_result_t json_parse_array(json_stream_parser_state_t* state, const size_t depth) {
    if (state == nullptr) {
        return json_create_error_result(JSON_PARSE_CONFIG_ERROR, "Internal error: null parser state");
    }

    if (depth > state->max_depth) {
        return json_create_error_result(JSON_PARSE_ERROR_MAX_DEPTH, "Maximum parsing depth exceeded");
    }

    if (state->position + 1 >= state->length) {
        return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input");
    }

    if (state->json[state->position] != '[') {
        return json_create_error_result(JSON_PARSE_ERROR_INVALID_SYNTAX, "Syntax error: expected '[' at the beginning of an array");
    }

    // Skip the opening bracket
    ++state->position;

    if (state->handler->on_array_start != nullptr) {
        const json_parser_result_t start_array_result = state->handler->on_array_start(state->handler);

        if (start_array_result.result != JSON_PARSE_SUCCESS) {
            return start_array_result;
        }
    }

    bool end = false;
    bool value_expected = true;

    for (size_t len = 0; state->position < state->length; ++len) {
        if (len > state->max_struct_size) {
            return json_create_error_result(JSON_PARSE_ERROR_MAX_STRUCT_SIZE, "Maximum array size exceeded");
        }

        if (!skip_whitespace(state)) {
            return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input while parsing array");
        }

        const char current_char = state->json[state->position];

        if (current_char == ']') {
            ++state->position;
            end = true;
            break;
        }

        if (current_char == ',') {
            if (value_expected == true) {
                return json_create_error_result(JSON_PARSE_ERROR_INVALID_SYNTAX, "Syntax error: unexpected ',' in array");
            }

            value_expected = true;
            ++state->position;
            continue;
        }

        if (!value_expected) {
            return json_create_error_result(JSON_PARSE_ERROR_INVALID_SYNTAX, "Syntax error: expected ',' between array values");
        }

        const json_parser_result_t value_result = json_parse_value(state, depth + 1);
        value_expected = false;

        if (value_result.result != JSON_PARSE_SUCCESS) {
            return value_result;
        }
    }

    if (!end) {
        return json_create_error_result(JSON_PARSE_ERROR_UNEXPECTED_END, "Unexpected end of JSON input while parsing array");
    }

    if (state->handler->on_array_end == nullptr) {
        return json_create_success_result();
    }

    return state->handler->on_array_end(state->handler);
}
