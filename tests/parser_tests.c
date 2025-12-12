//
// Created by vincent on 08/12/2025.
//

#include <stdlib.h>
#include <string.h>

#include "tests.h"
#include "../parser/parser.h"

TEST_CASE(parser)

static struct {
    size_t count;
    struct {
        const char* function_name;
        void* parameter;
    } entries[128];
} test_call_stack;

static json_parser_result_t on_null(json_parser_handler_t* self) {
    test_call_stack.entries[test_call_stack.count].function_name = "on_null";
    test_call_stack.entries[test_call_stack.count].parameter = nullptr;

    ++test_call_stack.count;

    return json_create_success_result();
}

static json_parser_result_t on_bool(json_parser_handler_t* self, bool value) {
    test_call_stack.entries[test_call_stack.count].function_name = "on_bool";
    test_call_stack.entries[test_call_stack.count].parameter = malloc(sizeof(bool));
    *((bool*)test_call_stack.entries[test_call_stack.count].parameter) = value;

    ++test_call_stack.count;

    return json_create_success_result();
}

static json_parser_result_t on_number(json_parser_handler_t* self, double value) {
    test_call_stack.entries[test_call_stack.count].function_name = "on_number";
    test_call_stack.entries[test_call_stack.count].parameter = malloc(sizeof(double));
    *((double*)test_call_stack.entries[test_call_stack.count].parameter) = value;

    ++test_call_stack.count;

    return json_create_success_result();
}

static json_parser_result_t on_string(json_parser_handler_t* self, json_raw_string_t value) {
    test_call_stack.entries[test_call_stack.count].function_name = "on_string";
    test_call_stack.entries[test_call_stack.count].parameter = malloc(sizeof(json_raw_string_t));
    memcpy(test_call_stack.entries[test_call_stack.count].parameter, &value, sizeof(json_raw_string_t));

    ++test_call_stack.count;

    return json_create_success_result();
}

static json_parser_result_t on_array_start(json_parser_handler_t* self) {
    test_call_stack.entries[test_call_stack.count].function_name = "on_array_start";
    test_call_stack.entries[test_call_stack.count].parameter = nullptr;

    ++test_call_stack.count;

    return json_create_success_result();
}

static json_parser_result_t on_array_end(json_parser_handler_t* self) {
    test_call_stack.entries[test_call_stack.count].function_name = "on_array_end";
    test_call_stack.entries[test_call_stack.count].parameter = nullptr;

    ++test_call_stack.count;

    return json_create_success_result();
}

static json_parser_result_t on_object_start(json_parser_handler_t* self) {
    test_call_stack.entries[test_call_stack.count].function_name = "on_object_start";
    test_call_stack.entries[test_call_stack.count].parameter = nullptr;

    ++test_call_stack.count;

    return json_create_success_result();
}

static json_parser_result_t on_object_property(json_parser_handler_t* self, json_raw_string_t key) {
    test_call_stack.entries[test_call_stack.count].function_name = "on_object_property";
    test_call_stack.entries[test_call_stack.count].parameter = malloc(sizeof(json_raw_string_t));
    memcpy(test_call_stack.entries[test_call_stack.count].parameter, &key, sizeof(json_raw_string_t));

    ++test_call_stack.count;

    return json_create_success_result();
}

static json_parser_result_t on_object_end(json_parser_handler_t* self) {
    test_call_stack.entries[test_call_stack.count].function_name = "on_object_end";
    test_call_stack.entries[test_call_stack.count].parameter = nullptr;

    ++test_call_stack.count;

    return json_create_success_result();
}

static void reset_test_call_stack() {
    for (size_t i = 0; i < test_call_stack.count; ++i) {
        if (test_call_stack.entries[i].parameter != nullptr) {
            free(test_call_stack.entries[i].parameter);
        }

        test_call_stack.entries[i].function_name = "\0";
        test_call_stack.entries[i].parameter = nullptr;
    }

    test_call_stack.count = 0;
}

static json_parser_handler_t init_handler() {
    reset_test_call_stack();

    return (json_parser_handler_t) {
        .on_null = on_null,
        .on_bool = on_bool,
        .on_number = on_number,
        .on_string = on_string,
        .on_array_start = on_array_start,
        .on_array_end = on_array_end,
        .on_object_start = on_object_start,
        .on_object_property = on_object_property,
        .on_object_end = on_object_end,
    };
}

static json_parser_result_t parse_json(const char* json) {
    json_parser_handler_t handler = init_handler();

    return json_parse(strlen(json), json, &handler, (json_parser_options_t) {32, 1024, 1024 });
}

static json_parser_result_t parse_json_no_handler(const char* json) {
    json_parser_handler_t handler = {};

    return json_parse(strlen(json), json, &handler, (json_parser_options_t) {32, 1024, 1024 });
}

TEST(parse_number) {
    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("123").code);
    ASSERT_INT(1, test_call_stack.count);
    ASSERT_STR("on_number", test_call_stack.entries[0].function_name);
    ASSERT_DOUBLE(123.0, *(double*) test_call_stack.entries[0].parameter, 0.0);

    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("123.456").code);
    ASSERT_INT(1, test_call_stack.count);
    ASSERT_STR("on_number", test_call_stack.entries[0].function_name);
    ASSERT_DOUBLE(123.456, *(double*) test_call_stack.entries[0].parameter, 0.0);

    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("-42").code);
    ASSERT_INT(1, test_call_stack.count);
    ASSERT_STR("on_number", test_call_stack.entries[0].function_name);
    ASSERT_DOUBLE(-42.0, *(double*) test_call_stack.entries[0].parameter, 0.0);
}

TEST(parse_null) {
    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("null").code);
    ASSERT_INT(1, test_call_stack.count);
    ASSERT_STR("on_null", test_call_stack.entries[0].function_name);
}

TEST(parse_boolean) {
    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("true").code);
    ASSERT_INT(1, test_call_stack.count);
    ASSERT_STR("on_bool", test_call_stack.entries[0].function_name);
    ASSERT_TRUE(*(bool*) test_call_stack.entries[0].parameter == true);

    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("false").code);
    ASSERT_INT(1, test_call_stack.count);
    ASSERT_STR("on_bool", test_call_stack.entries[0].function_name);
    ASSERT_TRUE(*(bool*) test_call_stack.entries[0].parameter == false);
}

TEST(parse_string) {
    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("\"Hello, World!\"").code);
    ASSERT_INT(1, test_call_stack.count);
    ASSERT_STR("on_string", test_call_stack.entries[0].function_name);

    json_raw_string_t str = *(json_raw_string_t*) test_call_stack.entries[0].parameter;
    ASSERT_INT(15, str.length);
    ASSERT_STRN("\"Hello, World!\"", str.value, 15);
}

TEST(parse_array_simple) {
    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("[]").code);
    ASSERT_INT(2, test_call_stack.count);
    ASSERT_STR("on_array_start", test_call_stack.entries[0].function_name);
    ASSERT_STR("on_array_end", test_call_stack.entries[1].function_name);

    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("[123, true]").code);
    ASSERT_INT(4, test_call_stack.count);
    ASSERT_STR("on_array_start", test_call_stack.entries[0].function_name);
    ASSERT_STR("on_number", test_call_stack.entries[1].function_name);
    ASSERT_DOUBLE(123.0, *(double*) test_call_stack.entries[1].parameter, 0.0);
    ASSERT_STR("on_bool", test_call_stack.entries[2].function_name);
    ASSERT_TRUE(*(bool*) test_call_stack.entries[2].parameter == true);
    ASSERT_STR("on_array_end", test_call_stack.entries[3].function_name);
}

TEST(parse_object_simple) {
    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("{}").code);
    ASSERT_INT(2, test_call_stack.count);
    ASSERT_STR("on_object_start", test_call_stack.entries[0].function_name);
    ASSERT_STR("on_object_end", test_call_stack.entries[1].function_name);

    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json("{\"foo\": 42}").code);
    ASSERT_INT(4, test_call_stack.count);
    ASSERT_STR("on_object_start", test_call_stack.entries[0].function_name);
    ASSERT_STR("on_object_property", test_call_stack.entries[1].function_name);
    json_raw_string_t key = *(json_raw_string_t*) test_call_stack.entries[1].parameter;
    ASSERT_INT(5, key.length);
    ASSERT_STRN("\"foo\"", key.value, 5);
    ASSERT_STR("on_number", test_call_stack.entries[2].function_name);
    ASSERT_DOUBLE(42.0, *(double*) test_call_stack.entries[2].parameter, 0.0);
    ASSERT_STR("on_object_end", test_call_stack.entries[3].function_name);
}

TEST(parse_complexe_json) {
    const char* json =
        "{"
            "\"name\": \"Alice\","
            "\"age\": 30,"
            "\"married\": false,"
            "\"children\": [\"Bob\", null],"
            "\"address\": {\"city\": \"Paris\", \"zip\": 75000},"
            "\"scores\": [12.5, -314]"
        "}";

    ASSERT_INT(JSON_PARSE_SUCCESS, parse_json(json).code);

    // Nombre d'événements attendus
    ASSERT_INT(25, test_call_stack.count);

    // 0
    ASSERT_STR("on_object_start", test_call_stack.entries[0].function_name);

    // 1 name prop -> 2 string "Alice"
    ASSERT_STR("on_object_property", test_call_stack.entries[1].function_name);
    {
        json_raw_string_t key = *(json_raw_string_t*) test_call_stack.entries[1].parameter;
        ASSERT_INT(6, key.length); // "\"name\""
        ASSERT_STRN("\"name\"", key.value, 6);
    }
    ASSERT_STR("on_string", test_call_stack.entries[2].function_name);
    {
        json_raw_string_t val = *(json_raw_string_t*) test_call_stack.entries[2].parameter;
        ASSERT_INT(7, val.length); // "\"Alice\""
        ASSERT_STRN("\"Alice\"", val.value, 7);
    }

    // 3 age prop -> 4 number 30
    ASSERT_STR("on_object_property", test_call_stack.entries[3].function_name);
    {
        json_raw_string_t key = *(json_raw_string_t*) test_call_stack.entries[3].parameter;
        ASSERT_INT(5, key.length); // "\"age\""
        ASSERT_STRN("\"age\"", key.value, 5);
    }
    ASSERT_STR("on_number", test_call_stack.entries[4].function_name);
    ASSERT_DOUBLE(30.0, *(double*) test_call_stack.entries[4].parameter, 0.0);

    // 5 married prop -> 6 bool false
    ASSERT_STR("on_object_property", test_call_stack.entries[5].function_name);
    {
        json_raw_string_t key = *(json_raw_string_t*) test_call_stack.entries[5].parameter;
        ASSERT_INT(9, key.length); // "\"married\""
        ASSERT_STRN("\"married\"", key.value, 9);
    }
    ASSERT_STR("on_bool", test_call_stack.entries[6].function_name);
    ASSERT_TRUE(*(bool*) test_call_stack.entries[6].parameter == false);

    // 7 children prop -> array start, "Bob", null, array end
    ASSERT_STR("on_object_property", test_call_stack.entries[7].function_name);
    {
        json_raw_string_t key = *(json_raw_string_t*) test_call_stack.entries[7].parameter;
        ASSERT_INT(10, key.length); // "\"children\""
        ASSERT_STRN("\"children\"", key.value, 10);
    }
    ASSERT_STR("on_array_start", test_call_stack.entries[8].function_name);
    ASSERT_STR("on_string", test_call_stack.entries[9].function_name);
    {
        json_raw_string_t val = *(json_raw_string_t*) test_call_stack.entries[9].parameter;
        ASSERT_INT(5, val.length); // "\"Bob\""
        ASSERT_STRN("\"Bob\"", val.value, 5);
    }
    ASSERT_STR("on_null", test_call_stack.entries[10].function_name);
    ASSERT_STR("on_array_end", test_call_stack.entries[11].function_name);

    // 12 address prop -> object start, city/"Paris", zip/75000, object end
    ASSERT_STR("on_object_property", test_call_stack.entries[12].function_name);
    {
        json_raw_string_t key = *(json_raw_string_t*) test_call_stack.entries[12].parameter;
        ASSERT_INT(9, key.length); // "\"address\""
        ASSERT_STRN("\"address\"", key.value, 9);
    }
    ASSERT_STR("on_object_start", test_call_stack.entries[13].function_name);

    ASSERT_STR("on_object_property", test_call_stack.entries[14].function_name);
    {
        json_raw_string_t key = *(json_raw_string_t*) test_call_stack.entries[14].parameter;
        ASSERT_INT(6, key.length); // "\"city\""
        ASSERT_STRN("\"city\"", key.value, 6);
    }
    ASSERT_STR("on_string", test_call_stack.entries[15].function_name);
    {
        json_raw_string_t val = *(json_raw_string_t*) test_call_stack.entries[15].parameter;
        ASSERT_INT(7, val.length); // "\"Paris\""
        ASSERT_STRN("\"Paris\"", val.value, 7);
    }

    ASSERT_STR("on_object_property", test_call_stack.entries[16].function_name);
    {
        json_raw_string_t key = *(json_raw_string_t*) test_call_stack.entries[16].parameter;
        ASSERT_INT(5, key.length); // "\"zip\""
        ASSERT_STRN("\"zip\"", key.value, 5);
    }
    ASSERT_STR("on_number", test_call_stack.entries[17].function_name);
    ASSERT_DOUBLE(75000.0, *(double*) test_call_stack.entries[17].parameter, 0.0);

    ASSERT_STR("on_object_end", test_call_stack.entries[18].function_name);

    // 19 scores prop -> array start, 12.5, -314.0, array end
    ASSERT_STR("on_object_property", test_call_stack.entries[19].function_name);
    {
        json_raw_string_t key = *(json_raw_string_t*) test_call_stack.entries[19].parameter;
        ASSERT_INT(8, key.length); // "\"scores\""
        ASSERT_STRN("\"scores\"", key.value, 8);
    }
    ASSERT_STR("on_array_start", test_call_stack.entries[20].function_name);
    ASSERT_STR("on_number", test_call_stack.entries[21].function_name);
    ASSERT_DOUBLE(12.5, *(double*) test_call_stack.entries[21].parameter, 0.0);
    ASSERT_STR("on_number", test_call_stack.entries[22].function_name);
    ASSERT_DOUBLE(-314.0, *(double*) test_call_stack.entries[22].parameter, 0.0);
    ASSERT_STR("on_array_end", test_call_stack.entries[23].function_name);

    // final object end
    ASSERT_STR("on_object_end", test_call_stack.entries[24].function_name);
}

TEST(parse_error) {
    {
        const json_parser_result_t result = parse_json("{]sdfdsfoi");
        ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
        ASSERT_INT(JSON_CONTEXT_OBJECT_PROPERTY, result.context);
        ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
        ASSERT_CHAR('"', result.extra);
        ASSERT_INT(1, result.position);
        ASSERT_STR("Syntax error: Unexpected character expected '\"' at position 1 while parsing object property", json_parse_error_message(result));
    }

    {
        const json_parser_result_t result = parse_json("tr");
        ASSERT_INT(JSON_PARSE_ERROR_UNEXPECTED_END, result.code);
        ASSERT_INT(JSON_CONTEXT_BOOL, result.context);
        ASSERT_INT(JSON_ERROR_TOO_SMALL, result.error);
        ASSERT_INT(4, result.extra);
        ASSERT_INT(0, result.position);
        ASSERT_STR("Unexpected end of input: Remaining input is too small (expected length 4) at position 0 while parsing boolean value", json_parse_error_message(result));
    }

    {
        const json_parser_result_t result = parse_json("trsssssssssss");
        ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
        ASSERT_INT(JSON_CONTEXT_BOOL, result.context);
        ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
        ASSERT_CHAR('u', result.extra);
        ASSERT_INT(2, result.position);
        ASSERT_STR("Syntax error: Unexpected character expected 'u' at position 2 while parsing boolean value", json_parse_error_message(result));
    }

    {
        const json_parser_result_t result = parse_json("[,]");
        ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
        ASSERT_INT(JSON_CONTEXT_ARRAY, result.context);
        ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
        ASSERT_CHAR(0, result.extra);
        ASSERT_INT(1, result.position);
        ASSERT_STR("Syntax error: Unexpected character at position 1 while parsing array value", json_parse_error_message(result));
    }

    {
        const json_parser_result_t result = parse_json("{test}");
        ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
        ASSERT_INT(JSON_CONTEXT_OBJECT_PROPERTY, result.context);
        ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
        ASSERT_CHAR('"', result.extra);
        ASSERT_INT(1, result.position);
        ASSERT_STR("Syntax error: Unexpected character expected '\"' at position 1 while parsing object property", json_parse_error_message(result));
    }

    {
        const json_parser_result_t result = parse_json("{\"test\"}");
        ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
        ASSERT_INT(JSON_CONTEXT_OBJECT, result.context);
        ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
        ASSERT_CHAR(':', result.extra);
        ASSERT_INT(7, result.position);
        ASSERT_STR("Syntax error: Unexpected character expected ':' at position 7 while parsing object value", json_parse_error_message(result));
    }

    {
        const json_parser_result_t result = parse_json("{\"test\":}");
        ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
        ASSERT_INT(JSON_CONTEXT_UNKNOWN, result.context);
        ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
        ASSERT_CHAR(0, result.extra);
        ASSERT_INT(8, result.position);
        ASSERT_STR("Syntax error: Unexpected character at position 8 ", json_parse_error_message(result));
    }

    {
        const json_parser_result_t result = parse_json("  ");
        ASSERT_INT(JSON_PARSE_ERROR_UNEXPECTED_END, result.code);
        ASSERT_INT(JSON_CONTEXT_UNKNOWN, result.context);
        ASSERT_INT(JSON_ERROR_EMPTY_VALUE, result.error);
        ASSERT_CHAR(0, result.extra);
        ASSERT_INT(2, result.position);
        ASSERT_STR("Unexpected end of input: Value is empty or contains only whitespace at position 2 ", json_parse_error_message(result));
    }
}

TEST(max_depth_exceeded) {
    const json_parser_result_t result = parse_json("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]");
    ASSERT_INT(JSON_PARSE_ERROR_MAX_DEPTH, result.code);
    ASSERT_INT(JSON_CONTEXT_ARRAY, result.context);
    ASSERT_INT(JSON_ERROR_UNKNOWN, result.error);
    ASSERT_CHAR(0, result.extra);
    ASSERT_INT(16, result.position);
    ASSERT_STR("Maximum depth exceeded:  at position 16 while parsing array value", json_parse_error_message(result));
}

TEST(max_string_size_exceeded) {
    char long_string[2049];
    memset(long_string, 'a', 2047);
    long_string[0] = '"';
    long_string[2047] = '"';
    long_string[2048] = '\0';

    const json_parser_result_t result = parse_json(long_string);
    ASSERT_INT(JSON_PARSE_ERROR_MAX_STRING_SIZE, result.code);
    ASSERT_INT(JSON_CONTEXT_STRING, result.context);
    ASSERT_INT(JSON_ERROR_UNKNOWN, result.error);
    ASSERT_CHAR(0, result.extra);
    ASSERT_INT(1024, result.position);
    ASSERT_STR("Maximum string size exceeded:  at position 1024 while parsing string value", json_parse_error_message(result));
}

TEST(max_array_length_exceeded) {
    char large_array[2053];
    large_array[0] = '[';
    for (size_t i = 1; i < 2050; i += 2) {
        large_array[i] = '0';
        large_array[i + 1] = ',';
    }
    large_array[2051] = ']';
    large_array[2052] = '\0';

    const json_parser_result_t result = parse_json_no_handler(large_array);
    ASSERT_INT(JSON_PARSE_ERROR_MAX_STRUCT_SIZE, result.code);
    ASSERT_INT(JSON_CONTEXT_ARRAY, result.context);
    ASSERT_INT(JSON_ERROR_UNKNOWN, result.error);
    ASSERT_CHAR(0, result.extra);
    ASSERT_INT(1026, result.position);
    ASSERT_STR("Maximum structure size exceeded:  at position 1026 while parsing array value", json_parse_error_message(result));
}

TEST(max_struct_length_exceeded) {
    char large_obj[5128];
    large_obj[0] = '{';
    for (size_t i = 1; i < 5125; i += 5) {
        large_obj[i] = '"';
        large_obj[i + 1] = '"';
        large_obj[i + 2] = ':';
        large_obj[i + 3] = '0';
        large_obj[i + 4] = ',';
    }
    large_obj[5126] = '}';
    large_obj[5127] = '\0';

    const json_parser_result_t result = parse_json_no_handler(large_obj);
    ASSERT_INT(JSON_PARSE_ERROR_MAX_STRUCT_SIZE, result.code);
    ASSERT_INT(JSON_CONTEXT_OBJECT, result.context);
    ASSERT_INT(JSON_ERROR_UNKNOWN, result.error);
    ASSERT_CHAR(0, result.extra);
    ASSERT_INT(2565, result.position);
    ASSERT_STR("Maximum structure size exceeded:  at position 2565 while parsing object value", json_parse_error_message(result));
}
