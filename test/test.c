#include <vjudge.h>

void print_judge_result(judge_result_t *result) {
    printf("{\n"
           "    Passed: %d\n"
           "    Passed tests: %ld/%ld\n"
           "    Error: %d\n"
           "    Tests: [\n",
           result->passed, result->passed_tests_count, result->tests_count, result->error);
    for (size_t i = 0; i < result->tests_count; i++) {
        test_t *test = &result->tests[i];
        printf("        {\n"
               "            Name: %s\n"
               "            Passed: %d\n"
               "            Passed assertions: %ld/%ld\n"
               "            Assertions: [\n",
               test->name, test->passed, test->passed_assertions_count, test->assertions_count);
        for (size_t j = 0; j < test->assertions_count; j++) {
            assertion_result_t *assertion_result = &test->assertion_results[j];
            printf("                {\n"
                   "                    Signal name: %s\n"
                   "                    Expected value: %s\n"
                   "                    Actual value: %s\n"
                   "                    Passed: %d\n"
                   "                }\n",
                   assertion_result->assertion.signal_name, assertion_result->assertion.expected_value,
                   assertion_result->actual_value, assertion_result->passed);
        }
        printf("            ]\n"
               "        }\n");
    }
    printf("    ]\n"
           "}\n");
}

int main(int argc, char *argv[]) {
    judge_input_t judge_input;
    judge_result_t judge_result;

    judge_input.test_dir_path = argv[1];
    judge_input.src_dir_path = argv[2];

    run_judge(&judge_input, &judge_result);

    print_judge_result(&judge_result);

    return 0;
}
