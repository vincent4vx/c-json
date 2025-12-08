//
// Created by vincent on 05/12/2025.
//

#include "formater.h"

#include <stdio.h>

typedef struct {
    char* buffer;
    const size_t buffer_size;
    size_t position;
} json_string_builder_t;

typedef struct {
    json_type_enum_t type;
    json_member_entry_t* current_entry;
} json_formater_stack_entry_t;

typedef struct {
    const size_t size;
    size_t used;
    json_formater_stack_entry_t entries[64];
} json_formater_stack_t;

static bool json_string_builder_append_cstring(json_string_builder_t* builder, const char* str) {
    for (size_t index = 0; builder->position < builder->buffer_size; ++index, ++builder->position) {
        const char current_char = str[index];

        if (current_char == '\0') {
            return true;
        }

        builder->buffer[builder->position] = current_char;
    }


    return false;
}

static bool json_string_builder_append_number(json_string_builder_t* builder, const double number) {
    if (builder->position >= builder->buffer_size) {
        return false;
    }

    const size_t available_size = builder->buffer_size - builder->position;
    const int written = snprintf(
        builder->buffer + builder->position,
        available_size,
        "%f",
        number
    );

    // Write failed, make the builder full to prevent further writes
    if (written < 0 || written > available_size) {
        builder->position = builder->buffer_size;
        return false;
    }

    builder->position += (size_t) written;
    return true;
}

/**
 * Get the escaped value for a given character
 * If the character does not need to be escaped, return 0
 *
 * @param c The character to escape
 * @return The escaped value, or 0 if the character does not need to be escaped
 */
static inline char json_char_escaped_value(const char c) {
    switch (c) {
        case '"':
            return '"';
        case '\\':
            return '\\';
        case '/':
            return '/';
        case '\b':
            return 'b';
        case '\f':
            return 'f';
        case '\n':
            return 'n';
        case '\r':
            return 'r';
        case '\t':
            return 't';
        case '\0':
            return '0';
        default:
            return 0;
    }
}

static bool json_format_string(json_string_builder_t* builder, const char* str, const size_t length) {
    if (builder->position >= builder->buffer_size) {
        return false;
    }

    builder->buffer[builder->position++] = '"';

    for (size_t i = 0; i < length; ++i) {
        if (builder->position >= builder->buffer_size) {
            return false;
        }

        const char current_char = str[i];
        const char escaped_value = json_char_escaped_value(current_char);

        if (escaped_value != 0) {
            builder->buffer[builder->position++] = '\\';

            if (builder->position >= builder->buffer_size) {
                return false;
            }

            builder->buffer[builder->position++] = escaped_value;
        } else {
            builder->buffer[builder->position++] = current_char;
        }
    }

    if (builder->position >= builder->buffer_size) {
        return false;
    }

    builder->buffer[builder->position++] = '"';
    return true;
}

json_formater_result_t json_format_value(const json_value_t* value, char* buffer, const size_t buffer_size) {
    if (buffer == nullptr || buffer_size == 0) {
        return (json_formater_result_t) {
            .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
            .error = { "Buffer is null or size is zero" },
        };
    }

    json_string_builder_t builder = {
        .buffer = buffer,
        .buffer_size = buffer_size,
        .position = 0,
    };
    json_formater_stack_t stack = {
        .size = 64, // @todo configurable stack size
        .used = 0,
    };

    json_member_entry_t root_entry = {
        .key_int = 0,
        .key_str = nullptr,
        .value = (struct json_value_t *) value,
        .next = nullptr,
    };
    stack.entries[stack.used++] = (typeof(stack.entries[0])) {
        .type = JSON_NULL, // Use arbitrary type for root
        .current_entry = &root_entry,
    };

    while (stack.used > 0) {
        json_formater_stack_entry_t* stack_entry = &stack.entries[stack.used - 1];
        const json_member_entry_t* current_entry = stack_entry->current_entry;

        if (current_entry == nullptr) {
            --stack.used;

            // Root entry: the "closing" character is already written
            if (stack.used == 0) {
                continue;
            }

            switch (stack_entry->type) {
                case JSON_ARRAY:
                    if (!json_string_builder_append_cstring(&builder, "]")) {
                        return (json_formater_result_t) {
                            .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                            .error = { "Buffer too small for ]" },
                        };
                    }
                    break;

                case JSON_OBJECT:
                    if (!json_string_builder_append_cstring(&builder, "}")) {
                        return (json_formater_result_t) {
                            .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                            .error = { "Buffer too small for }" },
                        };
                    }
                    break;

                default:
                    return (json_formater_result_t) {
                        .code = JSON_FORMATER_ERROR_INVALID_VALUE,
                        .error = { "Invalid type on stack" },
                    };
            }

            // @todo handle end of array/object
            continue;
        }

        const json_value_t* current_value = current_entry->value; // @todo check for null!!!
        stack_entry->current_entry = current_entry->next;

        if (stack_entry->type == JSON_OBJECT) {
            if (current_entry->key_str == nullptr) {
                return (json_formater_result_t) {
                    .code = JSON_FORMATER_ERROR_INVALID_VALUE,
                    .error = { "Object property must be a string" },
                };
            }

            if (!json_format_string(&builder, current_entry->key_str, (size_t) current_entry->key_int)) {
                return (json_formater_result_t) {
                    .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                    .error = { "Buffer too small for object key" },
                };
            }

            if (!json_string_builder_append_cstring(&builder, ": ")) {
                return (json_formater_result_t) {
                    .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                    .error = { "Buffer too small for colon" },
                };
            }
        }

        switch (current_value->type) {
            case JSON_NULL:
                if (!json_string_builder_append_cstring(&builder, "null")) {
                    return (json_formater_result_t) {
                        .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                        .error = { "Buffer too small for null" },
                    };
                }
                break;

            case JSON_BOOL:
                if (!json_string_builder_append_cstring(&builder, current_value->bool_value ? "true" : "false")) {
                    return (json_formater_result_t) {
                        .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                        .error = { "Buffer too small for boolean" },
                    };
                }
                break;

            case JSON_NUMBER:
                if (!json_string_builder_append_number(&builder, current_value->number_value)) {
                    return (json_formater_result_t) {
                        .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                        .error = { "Buffer too small for number" },
                    };
                }
                break;

            case JSON_STRING:
                if (!json_format_string(&builder, current_value->string_value.value, current_value->string_value.length)) {
                    return (json_formater_result_t) {
                        .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                        .error = { "Buffer too small for string" },
                    };
                }
                break;

            case JSON_ARRAY:
                if (!json_string_builder_append_cstring(&builder, "[")) {
                    return (json_formater_result_t) {
                        .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                        .error = { "Buffer too small for array" },
                    };
                }

                stack.entries[stack.used].current_entry = current_value->array_value.head;
                stack.entries[stack.used].type = JSON_ARRAY;
                ++stack.used;
                break;

            case JSON_OBJECT:
                if (!json_string_builder_append_cstring(&builder, "{")) {
                    return (json_formater_result_t) {
                        .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                        .error = { "Buffer too small for object" },
                    };
                }

                stack.entries[stack.used].current_entry = current_value->array_value.head;
                stack.entries[stack.used].type = JSON_OBJECT;
                ++stack.used;
                break;

            default:
                return (json_formater_result_t) {
                    .code = JSON_FORMATER_ERROR_INVALID_VALUE,
                    .error = { "Invalid type" },
                };
        }

        if (current_entry->next != nullptr) {
            if (!json_string_builder_append_cstring(&builder, ", ")) {
                return (json_formater_result_t) {
                    .code = JSON_FORMATER_ERROR_BUFFER_TOO_SMALL,
                    .error = { "Buffer too small for comma" },
                };
            }
        }
    }

    return (json_formater_result_t) {
        .code = JSON_FORMATER_SUCCESS,
        .result = {
            .length = builder.position,
            .buffer = builder.buffer,
        },
    };
}
