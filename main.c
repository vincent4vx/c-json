#include <stdio.h>
#include "parser/stream_parser.h"

json_stream_result_t my_on_bool(json_stream_handler_t* self, bool value) {
    printf("Got boolean: %s\n", value ? "true" : "false");
    return json_create_success_result();
}

json_stream_result_t my_on_number(json_stream_handler_t* self, double value) {
    printf("Got number: %f\n", value);
    return json_create_success_result();
}

json_stream_result_t my_on_null(json_stream_handler_t* self) {
    printf("Got null\n");
    return json_create_success_result();
}

json_stream_result_t my_on_string(json_stream_handler_t* self, json_raw_string_t value) {
    printf("Got string: %.*s\n", (int) value.length, value.value);
    return json_create_success_result();
}

json_stream_result_t my_on_object_property(json_stream_handler_t* self, json_raw_string_t value) {
    printf("Got property: %.*s\n", (int) value.length, value.value);
    return json_create_success_result();
}

void handle_json_result(json_stream_result_t result) {
    if (result.result != JSON_PARSE_SUCCESS) {
        printf("JSON Error: %s\n", result.message);
    }
}

int main(void) {
    printf("Hello, World!\n");

    json_stream_handler_t handler = {
        .on_bool = my_on_bool,
        .on_number = my_on_number,
        .on_null = my_on_null,
        .on_string = my_on_string,
        .on_object_property = my_on_object_property,
    };

    handle_json_result(json_stream_parse("true", 4, &handler, 32, 1024, 1024));
    handle_json_result(json_stream_parse("false", 5, &handler, 32, 1024, 1024));
    handle_json_result(json_stream_parse("42", 2, &handler, 32, 1024, 1024));
    handle_json_result(json_stream_parse("123.456789", 10, &handler, 32, 1024, 1024));
    handle_json_result(json_stream_parse("null", 4, &handler, 32, 1024, 1024));
    handle_json_result(json_stream_parse("\"Hello, World!\"", 15, &handler, 32, 1024, 1024));
    handle_json_result(json_stream_parse("[12, 85, null, true, 12.36]", 27, &handler, 32, 1024, 1024));
    handle_json_result(json_stream_parse("[    12       ,    \"foo\"     ,    ]", 35, &handler, 32, 1024, 1024));
    handle_json_result(json_stream_parse("{\"key\": 42,     \"bar\"   :   true    }", 37, &handler, 32, 1024, 1024));

    return 0;
}
