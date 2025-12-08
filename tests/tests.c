//
// Created by vincent on 08/12/2025.
//

#include <stdio.h>
#include "tests.h"

test_case_list_t g_test_cases = {
    .count = 0,
};

void test_register_case(test_case_entry_t* test_case) {
    if (g_test_cases.count >= MAX_TESTS_CASES) {
        fprintf(stderr, "[ERROR] Cannot add test case %s: Maximum number of test cases reached (%d)\n", test_case->name, MAX_TESTS_CASES);
        return;
    }

    g_test_cases.cases[g_test_cases.count++] = test_case;
}

int test_run_all() {
    size_t success_count = 0;
    size_t failure_count = 0;

    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    for (size_t i = 0; i < g_test_cases.count; ++i) {
        test_case_entry_t* test_case = g_test_cases.cases[i];
        printf("[INFO] Running test case: %s\n", test_case->name);

        for (size_t j = 0; j < test_case->count; ++j) {
            const test_entry_t test = test_case->tests[j];

            if (test.name == nullptr || test.func == nullptr) {
                break; // No more tests in this case
            }

            printf("[RUN ] test %s...\n", test.name);

            bool current_success = true;
            test.func(&current_success);

            if (current_success) {
                printf("[OK  ] Test %s completed.\n", test.name);
                ++success_count;
            } else {
                ++failure_count;
            }
        }
    }

    printf("\n[INFO] Test run completed: %zu succeeded, %zu failed.\n", success_count, failure_count);

    return failure_count > 0 ? 1 : 0;
}

int main() {
    return test_run_all();
}
