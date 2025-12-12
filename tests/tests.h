//
// Created by vincent on 08/12/2025.
//

#ifndef JSON_TESTS_H
#define JSON_TESTS_H

#define MAX_TESTS_PER_CASE 128
#define MAX_TESTS_CASES 10

#define TEST_CASE(test_case_name) \
    static test_case_entry_t current_test_case = { \
        .name = #test_case_name, \
        .tests = {0}, \
        .count = 0, \
    }; \
    __attribute__((constructor)) static void register_test_case_##test_case_name() { \
        test_register_case(&current_test_case); \
    }

#define TEST(test_name) \
    static void test_func_##test_name(bool* success); \
    __attribute__((constructor)) static void register_test_##test_name() { \
        if (current_test_case.count >= MAX_TESTS_PER_CASE) { \
            fprintf(stderr, "[ERROR] Cannot add test %s: Maximum number of tests per case reached (%d)\n", #test_name, MAX_TESTS_PER_CASE); \
            return; \
        } \
        current_test_case.tests[current_test_case.count++] = (test_entry_t){ \
            .name = #test_name, \
            .func = test_func_##test_name, \
        }; \
    } \
    static void test_func_##test_name(bool* success)

#define TEST_SETUP \
    static void test_setup(); \
    __attribute__((constructor)) static void register_test_setup() { \
        current_test_case.setup = test_setup; \
    } \
    static void test_setup()

#define TEST_TEARDOWN \
    static void test_teardown(); \
    __attribute__((constructor)) static void register_test_teardown() { \
        current_test_case.teardown = test_teardown; \
    } \
    static void test_teardown()

#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "[FAIL] Assertion failed: %s, function %s, file %s, line %d.\n", #expr, __func__, __FILE__, __LINE__); \
            *success = false; \
            return; \
        } \
    } while (0)

#define ASSERT_INT(expected, actual) \
    do { \
        const int expected_int = (int)(expected); \
        const int actual_int = (int)(actual); \
        if (expected_int != actual_int) { \
            fprintf(stderr, "[FAIL] Assertion failed: expected %d, but get %d, function %s, file %s, line %d.\n", expected_int, actual_int, __func__, __FILE__, __LINE__); \
            *success = false; \
            return; \
        } \
    } while (0)


#define ASSERT_NULL(expr) \
    do { \
        if (expr != nullptr) { \
            fprintf(stderr, "[FAIL] Assertion failed: expecting null for %s, function %s, file %s, line %d.\n", #expr, __func__, __FILE__, __LINE__); \
            *success = false; \
            return; \
        } \
    } while (0)

#define ASSERT_CHAR(expected, actual) \
    do { \
        const char expected_char = (char)(expected); \
        const char actual_char = (char)(actual); \
        if (expected != actual) { \
            fprintf(stderr, "[FAIL] Assertion failed: expected %c, but get %c, function %s, file %s, line %d.\n", expected_char, actual_char, __func__, __FILE__, __LINE__); \
            *success = false; \
            return; \
        } \
    } while (0)

#define ASSERT_DOUBLE(expected, actual, delta) \
    do { \
        const double expected_double = (double)(expected); \
        const double actual_double = (double)(actual); \
        const double diff = expected_double - actual_double; \
        if (diff > delta || -diff > delta) { \
            fprintf(stderr, "[FAIL] Assertion failed: expected %f, but get %f, function %s, file %s, line %d.\n", expected_double, actual_double, __func__, __FILE__, __LINE__); \
            *success = false; \
            return; \
        } \
    } while (0)

#define ASSERT_STR(expected, actual) \
    do { \
        if (strcmp(expected, actual) != 0) { \
            fprintf(stderr, "[FAIL] Assertion failed: expected %s, but get %s, function %s, file %s, line %d.\n", expected, actual, __func__, __FILE__, __LINE__); \
            *success = false; \
            return; \
        } \
    } while (0)

#define ASSERT_STRN(expected, actual, len) \
    do { \
        if (strncmp(expected, actual, len) != 0) { \
            fprintf(stderr, "[FAIL] Assertion failed: expected %*s, but get %*s, function %s, file %s, line %d.\n", (int) len, expected, (int) len, actual, __func__, __FILE__, __LINE__); \
            *success = false; \
            return; \
        } \
    } while (0)

#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef void (*test_func_t)(bool* success);

typedef struct test_entry {
    const char* name;
    test_func_t func;
} test_entry_t;

typedef struct {
    const char* name;
    test_entry_t tests[MAX_TESTS_PER_CASE];
    size_t count;
    void (*setup)(void);
    void (*teardown)(void);
} test_case_entry_t;

typedef struct test_case_list_t {
    test_case_entry_t* cases[MAX_TESTS_CASES];
    size_t count;
} test_case_list_t;

extern test_case_list_t g_test_cases;
void test_register_case(test_case_entry_t* test_case);
int test_run_all();

#endif //JSON_TESTS_H
