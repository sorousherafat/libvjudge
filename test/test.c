#include "../vjudge.h"

int main(int argc, char *argv[]) {
    judge_input_t judge_input;
    judge_result_t judge_result;

    judge_input.test_dir_path = argv[1];
    judge_input.src_files_count = argc - 2;
    for (int i = 0; i < judge_input.src_files_count; i++)
    {
        judge_input.src_file_paths[i] = argv[i + 2];
    }
    
    run_judge(&judge_input, &judge_result);

    printf("Passed: %d/%d\n", judge_result.passed_tests_count, judge_result.tests_count);
    printf("Error: %d\n", judge_result.error);

    return 0;
}