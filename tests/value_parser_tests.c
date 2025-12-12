//
// Created by vincent on 08/12/2025.
//

#include <stdlib.h>
#include <string.h>

#include "tests.h"
#include "../parser/value_parser.h"

TEST_CASE(value_parser)

static json_arena_t* test_arena = nullptr;

static json_value_parser_result_t parse_json(const char* json) {
    if (test_arena == nullptr) {
        const size_t arena_size = json_arena_size(1000000, 1024, 1024);
        test_arena = (json_arena_t*) malloc(arena_size);
        json_arena_init(test_arena, arena_size, 1000000, 1024, 1024);
    }

    return json_parse_value_defaults(strlen(json), json, test_arena);
}

TEST_TEARDOWN {
    if (test_arena != nullptr) {
        free(test_arena);
        test_arena = nullptr;
    }
}

TEST(parse_number) {
    {
        json_value_parser_result_t result = parse_json("123");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_NUMBER, result.value->type);
        ASSERT_DOUBLE(123.0, result.value->number_value, 0.0);
    }

    {
        json_value_parser_result_t result = parse_json("12.3");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_NUMBER, result.value->type);
        ASSERT_DOUBLE(12.3, result.value->number_value, 0.0);
    }

    {
        json_value_parser_result_t result = parse_json("-42");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_NUMBER, result.value->type);
        ASSERT_DOUBLE(-42, result.value->number_value, 0.0);
    }
}

TEST(parse_null) {
    json_value_parser_result_t result = parse_json("null");
    ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
    ASSERT_INT(JSON_NULL, result.value->type);
}

TEST(parse_boolean) {
    {
        json_value_parser_result_t result = parse_json("true");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_BOOL, result.value->type);
        ASSERT_TRUE(result.value->bool_value == true);
    }

    {
        json_value_parser_result_t result = parse_json("false");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_BOOL, result.value->type);
        ASSERT_TRUE(result.value->bool_value == false);
    }
}

TEST(parse_string) {
    {
        json_value_parser_result_t result = parse_json("\"Hello, World!\"");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_STRING, result.value->type);
        ASSERT_INT(13, result.value->string_value.length);
        ASSERT_STRN("Hello, World!", result.value->string_value.value, 13);
    }

    {
        json_value_parser_result_t result = parse_json("\"\\n\\\\\\0\"");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_STRING, result.value->type);
        ASSERT_INT(3, result.value->string_value.length);
        ASSERT_STRN("\n\\\0", result.value->string_value.value, 3);
    }
}

TEST(parse_array_simple) {
    {
        json_value_parser_result_t result = parse_json("[]");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_ARRAY, result.value->type);
        ASSERT_INT(0, result.value->array_value.length);
        ASSERT_NULL(result.value->array_value.head);
        ASSERT_NULL(result.value->array_value.tail);
    }

    {
        json_value_parser_result_t result = parse_json("[123, true]");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_ARRAY, result.value->type);
        ASSERT_INT(2, result.value->array_value.length);

        json_member_entry_t* first = result.value->array_value.head;
        ASSERT_TRUE(first != nullptr);
        ASSERT_INT(0, first->key_int);
        ASSERT_NULL(first->key_str);
        ASSERT_INT(JSON_NUMBER, first->value->type);
        ASSERT_DOUBLE(123.0, first->value->number_value, 0.0);

        json_member_entry_t* second = first->next;
        ASSERT_TRUE(second != nullptr);
        ASSERT_INT(1, second->key_int);
        ASSERT_NULL(second->key_str);
        ASSERT_INT(JSON_BOOL, second->value->type);
        ASSERT_TRUE(second->value->bool_value == true);

        ASSERT_NULL(second->next);
        ASSERT_TRUE(second == result.value->array_value.tail);
    }
}

TEST(parse_object_simple) {
    {
        json_value_parser_result_t result = parse_json("{}");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_OBJECT, result.value->type);
        ASSERT_INT(0, result.value->object_value.length);
        ASSERT_NULL(result.value->object_value.head);
        ASSERT_NULL(result.value->object_value.tail);
    }

    {
        json_value_parser_result_t result = parse_json("{\"foo\": 42}");
        ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
        ASSERT_INT(JSON_OBJECT, result.value->type);
        ASSERT_INT(1, result.value->object_value.length);
        json_member_entry_t* property = result.value->object_value.head;
        ASSERT_TRUE(property != nullptr);
        ASSERT_INT(3, property->key_int);
        ASSERT_STRN("foo", property->key_str, property->key_int);
        ASSERT_INT(JSON_NUMBER, property->value->type);
        ASSERT_DOUBLE(42.0, property->value->number_value, 0.0);
        ASSERT_NULL(property->next);
        ASSERT_TRUE(property == result.value->object_value.tail);
    }
}

TEST(parse_complex_json) {
    const char* json =
        "{"
            "\"name\": \"Alice\","
            "\"age\": 30,"
            "\"married\": false,"
            "\"children\": [\"Bob\", null],"
            "\"address\": {\"city\": \"Paris\", \"zip\": 75000},"
            "\"scores\": [12.5, -314]"
        "}";

    json_value_parser_result_t result = parse_json(json);
    ASSERT_INT(JSON_PARSE_SUCCESS, result.result.code);
    ASSERT_INT(JSON_OBJECT, result.value->type);
    ASSERT_INT(6, result.value->object_value.length);

    json_member_entry_t* name_prop = result.value->object_value.head;
    ASSERT_TRUE(name_prop != nullptr);
    ASSERT_INT(4, name_prop->key_int);
    ASSERT_STRN("name", name_prop->key_str, name_prop->key_int);
    ASSERT_INT(JSON_STRING, name_prop->value->type);
    ASSERT_INT(5, name_prop->value->string_value.length);
    ASSERT_STRN("Alice", name_prop->value->string_value.value, name_prop->value->string_value.length);

    json_member_entry_t* age_prop = name_prop->next;
    ASSERT_TRUE(age_prop != nullptr);
    ASSERT_INT(3, age_prop->key_int);
    ASSERT_STRN("age", age_prop->key_str, age_prop->key_int);
    ASSERT_INT(JSON_NUMBER, age_prop->value->type);
    ASSERT_DOUBLE(30.0, age_prop->value->number_value, 0.0);

    json_member_entry_t* married_prop = age_prop->next;
    ASSERT_TRUE(married_prop != nullptr);
    ASSERT_INT(7, married_prop->key_int);
    ASSERT_STRN("married", married_prop->key_str, married_prop->key_int);
    ASSERT_INT(JSON_BOOL, married_prop->value->type);
    ASSERT_TRUE(married_prop->value->bool_value == false);

    json_member_entry_t* children_prop = married_prop->next;
    ASSERT_TRUE(children_prop != nullptr);
    ASSERT_INT(8, children_prop->key_int);
    ASSERT_STRN("children", children_prop->key_str, children_prop->key_int);
    ASSERT_INT(JSON_ARRAY, children_prop->value->type);
    ASSERT_INT(2, children_prop->value->array_value.length);
    json_member_entry_t* first_child = children_prop->value->array_value.head;
    ASSERT_TRUE(first_child != nullptr);
    ASSERT_INT(0, first_child->key_int);
    ASSERT_NULL(first_child->key_str);
    ASSERT_INT(JSON_STRING, first_child->value->type);
    ASSERT_INT(3, first_child->value->string_value.length);
    ASSERT_STRN("Bob", first_child->value->string_value.value, first_child->value->string_value.length);
    json_member_entry_t* second_child = first_child->next;
    ASSERT_TRUE(second_child != nullptr);
    ASSERT_INT(1, second_child->key_int);
    ASSERT_NULL(second_child->key_str);
    ASSERT_INT(JSON_NULL, second_child->value->type);
    ASSERT_NULL(second_child->next);
    ASSERT_TRUE(second_child == children_prop->value->array_value.tail);

    json_member_entry_t* address_prop = children_prop->next;
    ASSERT_TRUE(address_prop != nullptr);
    ASSERT_INT(7, address_prop->key_int);
    ASSERT_STRN("address", address_prop->key_str, address_prop->key_int);
    ASSERT_INT(JSON_OBJECT, address_prop->value->type);
    ASSERT_INT(2, address_prop->value->object_value.length);
    json_member_entry_t* city_prop = address_prop->value->object_value.head;
    ASSERT_TRUE(city_prop != nullptr);
    ASSERT_INT(4, city_prop->key_int);
    ASSERT_STRN("city", city_prop->key_str, city_prop->key_int);
    ASSERT_INT(JSON_STRING, city_prop->value->type);
    ASSERT_INT(5, city_prop->value->string_value.length);
    ASSERT_STRN("Paris", city_prop->value->string_value.value, city_prop->value->string_value.length);
    json_member_entry_t* zip_prop = city_prop->next;
    ASSERT_TRUE(zip_prop != nullptr);
    ASSERT_INT(3, zip_prop->key_int);
    ASSERT_STRN("zip", zip_prop->key_str, zip_prop->key_int);
    ASSERT_INT(JSON_NUMBER, zip_prop->value->type);
    ASSERT_DOUBLE(75000.0, zip_prop->value->number_value, 0.0);
    ASSERT_NULL(zip_prop->next);
    ASSERT_TRUE(zip_prop == address_prop->value->object_value.tail);

    json_member_entry_t* scores_prop = address_prop->next;
    ASSERT_TRUE(scores_prop != nullptr);
    ASSERT_INT(6, scores_prop->key_int);
    ASSERT_STRN("scores", scores_prop->key_str, scores_prop->key_int);
    ASSERT_INT(JSON_ARRAY, scores_prop->value->type);
    ASSERT_INT(2, scores_prop->value->array_value.length);
    json_member_entry_t* first_score = scores_prop->value->array_value.head;
    ASSERT_TRUE(first_score != nullptr);
    ASSERT_INT(0, first_score->key_int);
    ASSERT_NULL(first_score->key_str);
    ASSERT_INT(JSON_NUMBER, first_score->value->type);
    ASSERT_DOUBLE(12.5, first_score->value->number_value, 0.0);
    json_member_entry_t* second_score = first_score->next;
    ASSERT_TRUE(second_score != nullptr);
    ASSERT_INT(1, second_score->key_int);
    ASSERT_NULL(second_score->key_str);
    ASSERT_INT(JSON_NUMBER, second_score->value->type);
    ASSERT_DOUBLE(-314.0, second_score->value->number_value, 0.0);
    ASSERT_NULL(second_score->next);
    ASSERT_TRUE(second_score == scores_prop->value->array_value.tail);

    ASSERT_TRUE(result.value->object_value.tail == scores_prop);
    ASSERT_NULL(scores_prop->next);
}
//
// TEST(parse_error) {
//     {
//         const json_parser_result_t result = parse_json("{]sdfdsfoi");
//         ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
//         ASSERT_INT(JSON_CONTEXT_OBJECT_PROPERTY, result.context);
//         ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
//         ASSERT_CHAR('"', result.extra);
//         ASSERT_INT(1, result.position);
//         ASSERT_STR("Syntax error: Unexpected character expected '\"' at position 1 while parsing object property", json_parse_error_message(result));
//     }
//
//     {
//         const json_parser_result_t result = parse_json("tr");
//         ASSERT_INT(JSON_PARSE_ERROR_UNEXPECTED_END, result.code);
//         ASSERT_INT(JSON_CONTEXT_BOOL, result.context);
//         ASSERT_INT(JSON_ERROR_TOO_SMALL, result.error);
//         ASSERT_INT(4, result.extra);
//         ASSERT_INT(0, result.position);
//         ASSERT_STR("Unexpected end of input: Remaining input is too small (expected length 4) at position 0 while parsing boolean value", json_parse_error_message(result));
//     }
//
//     {
//         const json_parser_result_t result = parse_json("trsssssssssss");
//         ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
//         ASSERT_INT(JSON_CONTEXT_BOOL, result.context);
//         ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
//         ASSERT_CHAR('u', result.extra);
//         ASSERT_INT(2, result.position);
//         ASSERT_STR("Syntax error: Unexpected character expected 'u' at position 2 while parsing boolean value", json_parse_error_message(result));
//     }
//
//     {
//         const json_parser_result_t result = parse_json("[,]");
//         ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
//         ASSERT_INT(JSON_CONTEXT_ARRAY, result.context);
//         ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
//         ASSERT_CHAR(0, result.extra);
//         ASSERT_INT(1, result.position);
//         ASSERT_STR("Syntax error: Unexpected character at position 1 while parsing array value", json_parse_error_message(result));
//     }
//
//     {
//         const json_parser_result_t result = parse_json("{test}");
//         ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
//         ASSERT_INT(JSON_CONTEXT_OBJECT_PROPERTY, result.context);
//         ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
//         ASSERT_CHAR('"', result.extra);
//         ASSERT_INT(1, result.position);
//         ASSERT_STR("Syntax error: Unexpected character expected '\"' at position 1 while parsing object property", json_parse_error_message(result));
//     }
//
//     {
//         const json_parser_result_t result = parse_json("{\"test\"}");
//         ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
//         ASSERT_INT(JSON_CONTEXT_OBJECT, result.context);
//         ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
//         ASSERT_CHAR(':', result.extra);
//         ASSERT_INT(7, result.position);
//         ASSERT_STR("Syntax error: Unexpected character expected ':' at position 7 while parsing object value", json_parse_error_message(result));
//     }
//
//     {
//         const json_parser_result_t result = parse_json("{\"test\":}");
//         ASSERT_INT(JSON_PARSE_ERROR_INVALID_SYNTAX, result.code);
//         ASSERT_INT(JSON_CONTEXT_UNKNOWN, result.context);
//         ASSERT_INT(JSON_ERROR_UNEXPECTED_CHARACTER, result.error);
//         ASSERT_CHAR(0, result.extra);
//         ASSERT_INT(8, result.position);
//         ASSERT_STR("Syntax error: Unexpected character at position 8 ", json_parse_error_message(result));
//     }
//
//     {
//         const json_parser_result_t result = parse_json("  ");
//         ASSERT_INT(JSON_PARSE_ERROR_UNEXPECTED_END, result.code);
//         ASSERT_INT(JSON_CONTEXT_UNKNOWN, result.context);
//         ASSERT_INT(JSON_ERROR_EMPTY_VALUE, result.error);
//         ASSERT_CHAR(0, result.extra);
//         ASSERT_INT(2, result.position);
//         ASSERT_STR("Unexpected end of input: Value is empty or contains only whitespace at position 2 ", json_parse_error_message(result));
//     }
// }
//
// TEST(max_depth_exceeded) {
//     const json_parser_result_t result = parse_json("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]");
//     ASSERT_INT(JSON_PARSE_ERROR_MAX_DEPTH, result.code);
//     ASSERT_INT(JSON_CONTEXT_ARRAY, result.context);
//     ASSERT_INT(JSON_ERROR_UNKNOWN, result.error);
//     ASSERT_CHAR(0, result.extra);
//     ASSERT_INT(16, result.position);
//     ASSERT_STR("Maximum depth exceeded:  at position 16 while parsing array value", json_parse_error_message(result));
// }
//
// TEST(max_string_size_exceeded) {
//     char long_string[2049];
//     memset(long_string, 'a', 2047);
//     long_string[0] = '"';
//     long_string[2047] = '"';
//     long_string[2048] = '\0';
//
//     const json_parser_result_t result = parse_json(long_string);
//     ASSERT_INT(JSON_PARSE_ERROR_MAX_STRING_SIZE, result.code);
//     ASSERT_INT(JSON_CONTEXT_STRING, result.context);
//     ASSERT_INT(JSON_ERROR_UNKNOWN, result.error);
//     ASSERT_CHAR(0, result.extra);
//     ASSERT_INT(1024, result.position);
//     ASSERT_STR("Maximum string size exceeded:  at position 1024 while parsing string value", json_parse_error_message(result));
// }
//
// TEST(max_array_length_exceeded) {
//     char large_array[2053];
//     large_array[0] = '[';
//     for (size_t i = 1; i < 2050; i += 2) {
//         large_array[i] = '0';
//         large_array[i + 1] = ',';
//     }
//     large_array[2051] = ']';
//     large_array[2052] = '\0';
//
//     const json_parser_result_t result = parse_json_no_handler(large_array);
//     ASSERT_INT(JSON_PARSE_ERROR_MAX_STRUCT_SIZE, result.code);
//     ASSERT_INT(JSON_CONTEXT_ARRAY, result.context);
//     ASSERT_INT(JSON_ERROR_UNKNOWN, result.error);
//     ASSERT_CHAR(0, result.extra);
//     ASSERT_INT(1026, result.position);
//     ASSERT_STR("Maximum structure size exceeded:  at position 1026 while parsing array value", json_parse_error_message(result));
// }
//
// TEST(max_struct_length_exceeded) {
//     char large_obj[5128];
//     large_obj[0] = '{';
//     for (size_t i = 1; i < 5125; i += 5) {
//         large_obj[i] = '"';
//         large_obj[i + 1] = '"';
//         large_obj[i + 2] = ':';
//         large_obj[i + 3] = '0';
//         large_obj[i + 4] = ',';
//     }
//     large_obj[5126] = '}';
//     large_obj[5127] = '\0';
//
//     const json_parser_result_t result = parse_json_no_handler(large_obj);
//     ASSERT_INT(JSON_PARSE_ERROR_MAX_STRUCT_SIZE, result.code);
//     ASSERT_INT(JSON_CONTEXT_OBJECT, result.context);
//     ASSERT_INT(JSON_ERROR_UNKNOWN, result.error);
//     ASSERT_CHAR(0, result.extra);
//     ASSERT_INT(2565, result.position);
//     ASSERT_STR("Maximum structure size exceeded:  at position 2565 while parsing object value", json_parse_error_message(result));
// }
