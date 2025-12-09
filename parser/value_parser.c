#include "value_parser.h"

#include <stdio.h>

#include "../type/factory.h"

typedef struct {
    json_parser_handler_t callbacks;
    json_arena_t* arena;
    json_value_t* root;
    size_t stack_size;
    size_t stack_used;
    json_value_t* stack[];
} json_value_parser_handler;

static json_parser_result_t json_value_parser_populate_top(json_value_parser_handler* handler, json_value_t* top, json_value_t* value) {
    if (top == nullptr) {
        return (json_parser_result_t) { JSON_PARSE_CONFIG_ERROR, JSON_CONTEXT_UNKNOWN, JSON_ERROR_NULL_POINTER };
    }

    switch (top->type) {
        case JSON_ARRAY: {
            json_member_entry_t* member = json_create_array_member(handler->arena, (int) top->array_value.length, value);

            if (member == nullptr) {
                return (json_parser_result_t) { JSON_PARSE_HANDLER_ERROR, JSON_CONTEXT_ARRAY, JSON_ERROR_OUT_OF_MEMORY };
            }

            ++top->array_value.length;
            json_member_entry_t* previous_tail = top->array_value.tail;

            if (previous_tail == nullptr) {
                top->array_value.head = member;
                top->array_value.tail = member;
            } else {
                previous_tail->next = member;
                top->array_value.tail = member;

                if (previous_tail->key_int + 1 != member->key_int) {
                    return (json_parser_result_t) { JSON_PARSE_CONFIG_ERROR, JSON_CONTEXT_ARRAY, JSON_ERROR_NOT_SEQUENTIAL_KEYS };
                }
            }

            break;
        }

        case JSON_OBJECT: {
            if (top->object_value.tail->value != nullptr) {
                return (json_parser_result_t) { JSON_PARSE_CONFIG_ERROR, JSON_CONTEXT_OBJECT, JSON_ERROR_PROPERTY_ALREADY_SET };
            }

            top->object_value.tail->value = value;
            break;
        }

        default:
            return (json_parser_result_t) { JSON_PARSE_CONFIG_ERROR, JSON_CONTEXT_UNKNOWN, JSON_ERROR_INVALID_TYPE };
    }

    return json_create_success_result();
}

static json_parser_result_t json_value_parser_push(json_value_parser_handler* handler, json_value_t* value, json_parse_context_t context) {
    if (value == nullptr) {
        return (json_parser_result_t) { JSON_PARSE_CONFIG_ERROR, context, JSON_ERROR_NULL_POINTER };
    }

    if (handler->root == nullptr) {
        handler->root = value;
    }

    if (handler->stack_used > 0) {
        json_value_t* top = handler->stack[handler->stack_used - 1];
        const json_parser_result_t populate_result = json_value_parser_populate_top(handler, top, value);

        if (populate_result.code != JSON_PARSE_SUCCESS) {
            return populate_result;
        }
    }

    if (value->type == JSON_ARRAY || value->type == JSON_OBJECT) {
        if (handler->stack_used >= handler->stack_size) {
            return (json_parser_result_t) { JSON_PARSE_HANDLER_ERROR, context, JSON_ERROR_STACK_OVERFLOW };
        }

        handler->stack[handler->stack_used++] = value;
    }

    return json_create_success_result();
}

static json_parser_result_t json_value_parser_pop(json_value_parser_handler* handler, const json_parse_context_t context) {
    if (handler->stack_used < 1) {
        return (json_parser_result_t) { JSON_PARSE_HANDLER_ERROR, context, JSON_ERROR_STACK_EMPTY };
    }

    // @todo check type

    --handler->stack_used;
    handler->stack[handler->stack_used] = nullptr;

    return json_create_success_result();
}

static json_parser_result_t json_value_parser_handler_on_null(json_parser_handler_t* self) {
    json_value_parser_handler* handler = (json_value_parser_handler*) self;
    json_value_t* null_value = json_create_null_value(handler->arena);

    if (null_value == nullptr) {
        return (json_parser_result_t) { JSON_PARSE_HANDLER_ERROR, JSON_CONTEXT_NULL, JSON_ERROR_OUT_OF_MEMORY };
    }

    return json_value_parser_push(handler, null_value, JSON_CONTEXT_NULL);
}

static json_parser_result_t json_value_parser_handler_on_bool(json_parser_handler_t* self, const bool value) {
    json_value_parser_handler* handler = (json_value_parser_handler*) self;
    json_value_t* bool_value = json_create_bool_value(handler->arena, value);

    if (bool_value == nullptr) {
        return (json_parser_result_t) { JSON_PARSE_HANDLER_ERROR, JSON_CONTEXT_BOOL, JSON_ERROR_OUT_OF_MEMORY };
    }

    return json_value_parser_push(handler, bool_value, JSON_CONTEXT_BOOL);
}

static json_parser_result_t json_value_parser_handler_on_number(json_parser_handler_t* self, const double value) {
    json_value_parser_handler* handler = (json_value_parser_handler*) self;
    json_value_t* number_value = json_create_number_value(handler->arena, value);

    if (number_value == nullptr) {
        return (json_parser_result_t) { JSON_PARSE_HANDLER_ERROR, JSON_CONTEXT_NUMBER, JSON_ERROR_OUT_OF_MEMORY };
    }

    return json_value_parser_push(handler, number_value, JSON_CONTEXT_NUMBER);
}

static json_parser_result_t json_value_parser_handler_on_string(json_parser_handler_t* self, const json_raw_string_t value) {
    json_value_parser_handler* handler = (json_value_parser_handler*) self;
    json_value_t* string_value = json_create_string_value(handler->arena, value.value, value.length);

    if (string_value == nullptr) {
        return (json_parser_result_t) { JSON_PARSE_HANDLER_ERROR, JSON_CONTEXT_STRING, JSON_ERROR_OUT_OF_MEMORY };
    }

    return json_value_parser_push(handler, string_value, JSON_CONTEXT_STRING);
}

static json_parser_result_t json_value_parser_handler_on_array_start(json_parser_handler_t* self) {
    json_value_parser_handler* handler = (json_value_parser_handler*) self;
    json_value_t* new_array = json_create_empty_array(handler->arena);

    if (new_array == nullptr) {
        return (json_parser_result_t) { JSON_PARSE_HANDLER_ERROR, JSON_CONTEXT_ARRAY, JSON_ERROR_OUT_OF_MEMORY };
    }

    return json_value_parser_push(handler, new_array, JSON_CONTEXT_ARRAY);
}

static json_parser_result_t json_value_parser_handler_on_array_end(json_parser_handler_t* self) {
    json_value_parser_handler* handler = (json_value_parser_handler*) self;
    return json_value_parser_pop(handler, JSON_CONTEXT_ARRAY);
}

static json_parser_result_t json_value_parser_handler_on_object_start(json_parser_handler_t* self) {
    json_value_parser_handler* handler = (json_value_parser_handler*) self;
    json_value_t* new_object = json_create_empty_object(handler->arena);

    if (new_object == nullptr) {
        return (json_parser_result_t) { JSON_PARSE_HANDLER_ERROR, JSON_CONTEXT_OBJECT, JSON_ERROR_OUT_OF_MEMORY };
    }

    return json_value_parser_push(handler, new_object, JSON_CONTEXT_OBJECT);
}

static json_parser_result_t json_value_parser_handler_on_object_property(json_parser_handler_t* self, json_raw_string_t key) {
    json_value_parser_handler* handler = (json_value_parser_handler*) self;
    json_member_entry_t* new_property = json_create_object_member(handler->arena, key.value, key.length);

    if (new_property == nullptr) {
        return (json_parser_result_t) { JSON_PARSE_HANDLER_ERROR, JSON_CONTEXT_OBJECT_PROPERTY, JSON_ERROR_OUT_OF_MEMORY };
    }

    if (handler->stack_used < 1) {
        return (json_parser_result_t) { JSON_PARSE_CONFIG_ERROR, JSON_CONTEXT_OBJECT_PROPERTY, JSON_ERROR_STACK_EMPTY };
    }

    json_value_t* object = handler->stack[handler->stack_used - 1];

    if (object->type != JSON_OBJECT) {
        return (json_parser_result_t) { JSON_PARSE_CONFIG_ERROR, JSON_CONTEXT_OBJECT_PROPERTY, JSON_ERROR_INVALID_TYPE };
    }

    ++object->object_value.length;

    if (object->object_value.tail == nullptr) {
        object->object_value.head = new_property;
        object->object_value.tail = new_property;
    } else {
        object->object_value.tail->next = new_property;
        object->object_value.tail = new_property;
    }

    // @todo check if the previous property has been set

    return json_create_success_result();
}

static json_parser_result_t json_value_parser_handler_on_object_end(json_parser_handler_t* self) {
    json_value_parser_handler* handler = (json_value_parser_handler*) self;
    return json_value_parser_pop(handler, JSON_CONTEXT_OBJECT);
}

static void json_value_parser_handler_init(json_value_parser_handler* handler, json_arena_t* arena, const size_t stack_size) {
    handler->callbacks.on_null = json_value_parser_handler_on_null;
    handler->callbacks.on_bool = json_value_parser_handler_on_bool;
    handler->callbacks.on_number = json_value_parser_handler_on_number;
    handler->callbacks.on_string = json_value_parser_handler_on_string;
    handler->callbacks.on_array_start = json_value_parser_handler_on_array_start;
    handler->callbacks.on_array_end = json_value_parser_handler_on_array_end;
    handler->callbacks.on_object_start = json_value_parser_handler_on_object_start;
    handler->callbacks.on_object_property = json_value_parser_handler_on_object_property;
    handler->callbacks.on_object_end = json_value_parser_handler_on_object_end;

    handler->arena = arena;
    handler->stack_size = stack_size;
    handler->stack_used = 0;
    handler->root = nullptr;
}

json_value_parser_result_t json_parse_value(const char* json, size_t length, json_arena_t* arena, json_parser_options_t options) {
    options = json_default_parser_options(options);

    // @todo faire mieux
    char buffer[sizeof(json_value_parser_handler) + sizeof(json_value_t*) * options.max_depth];
    json_value_parser_handler* handler = (json_value_parser_handler*) buffer;
    json_value_parser_handler_init(handler, arena, options.max_depth);

    const json_parser_result_t result = json_parse(json, length, &handler->callbacks, options);

    if (handler->root != nullptr) {
        return (json_value_parser_result_t) {
            .result = result,
            .value = handler->root,
        };
    }

    return (json_value_parser_result_t) {
        .result = (json_parser_result_t) {
            .code = JSON_PARSE_CONFIG_ERROR,
            .context = JSON_CONTEXT_UNKNOWN,
            .error = JSON_ERROR_EMPTY_VALUE,
        },
        .value = nullptr,
    };
}
