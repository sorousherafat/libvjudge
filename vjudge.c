#include "vjudge.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_NAME_SIZE 128
#define MAX_ASSERTIONS_NO 1024
#define MAX_TESTS_NO 64

static const char *out_file_name = ".tmp.o";
static const char *vcd_file_name = ".tmp.vcd";
static DIR *tests_dir;
static char *tests_dir_path;
static test_t tests[MAX_TESTS_NO];
static size_t tests_count = 0;
static judge_result_t *judge_result;

void run_judge(judge_input_t *judge_input_arg, judge_result_t *judge_result_arg) {
    judge_result = judge_result_arg;
    judge_result->error = NO_ERROR;
    judge_result->tests_count = tests_count;

    tests_dir_path = judge_input_arg->test_dir_path;
    int src_files_count = judge_input_arg->src_files_count;
    char **src_file_paths = judge_input_arg->src_file_paths;

    check_files_existence(src_files_count, src_file_paths);
    load_tests();

    test_src_files(src_files_count, src_file_paths);
    judge_result->passed = true ? judge_result->passed_tests_count == judge_result->tests_count : false;
}

void check_files_existence(int src_files_count, char *const *src_file_paths) {
    bool files_exist = true;
    if ((tests_dir = opendir(tests_dir_path)) == NULL) {
        judge_result->error = ERROR_OPENING_TESTS_DIRECTORY;
        judge_result->passed = false;
        judge_result->passed_tests_count = 0;
        judge_result->tests_count = -1;
        fprintf(stderr, "Could not open tests directory: %s\n", tests_dir_path);
        files_exist = false;
    }

    for (int i = 0; i < src_files_count; i++)
        if (access(src_file_paths[i], R_OK) == -1) {
            judge_result->error = ERROR_OPENING_SOURCE_FILE;
            judge_result->passed = false;
            judge_result->passed_tests_count = 0;
            judge_result->tests_count = -1;
            fprintf(stderr, "Could not open source file: %s\n", src_file_paths[i]);
            files_exist = false;
        }

    if (!files_exist) {
        fprintf(stderr, "\nError!");
        exit(EXIT_FAILURE);
    }
}

void load_tests() {
    struct dirent *tests_dirent;
    while ((tests_dirent = readdir(tests_dir)) != NULL) {
        if (tests_dirent->d_type != DT_REG)
            continue;

        char *test_name;
        if (!try_get_test_name(tests_dirent->d_name, &test_name))
            continue;

        if (test_exists(test_name))
            continue;

        test_t *test = read_test(test_name);

        read_test_assertions(tests_dirent, test_name, test);
    }
}

bool try_get_test_name(char *file_name, char **test_name) {
    int result = sscanf(file_name, "%m[^-]-test.v", test_name);
    if (result == 1 && *test_name[0] != '\0' && file_name[result + strlen("-test.v")] != '\0')
        return true;
    return false;
}

bool test_exists(const char *test_name) {
    for (int test_index = 0; test_index < tests_count; ++test_index) {
        test_t *test = &tests[test_index];
        if (strcmp(test->name, test_name) == 0)
            return true;
    }

    return false;
}

test_t *read_test(char *test_name) {
    test_t *test = &tests[tests_count];
    test->name = test_name;
    test->assertions_count = 0;

    judge_result->tests[tests_count] = *test;
    tests_count += 1;
    judge_result->tests_count = tests_count;
    
    return test;
}

void read_test_assertions(const struct dirent *tests_dirent, const char *test_name, test_t *test) {
    char *assertion_file_path = (char *)malloc(PATH_MAX * sizeof(char));
    write_assertion_file_path(assertion_file_path, test_name);

    FILE *assertion_file = open_assertion_file(tests_dirent, assertion_file_path);
    read_assertions(test, assertion_file);
}

void read_assertions(test_t *test, FILE *assertion_file) {
    timestamp_t timestamp;
    char *expected_value;
    char *signal_name;
    int result;
    while ((result = fscanf(assertion_file, "%d %m[^=]=%m[^\n]\n", &timestamp, &signal_name, &expected_value)) != EOF) {
        if (result != 3) {
            judge_result->error = ERROR_ASSERTIONS_FILE_WRONG_FORMAT;
            judge_result->passed = false;
            judge_result->passed_tests_count = 0;
            judge_result->tests_count = -1;
            fprintf(stderr, "An error occurred when trying to read assertion files");
            exit(EXIT_FAILURE);
        }

        write_assertion(test, timestamp, expected_value, signal_name);
    }
}

void write_assertion(test_t *test, timestamp_t timestamp, char *expected_value, char *signal_name) {
    assertion_result_t *assertion_result = &test->assertion_results[test->assertions_count];
    assertion_result->passed = false;    
    assertion_t *assertion = &assertion_result->assertion;
    assertion->timestamp = timestamp;
    assertion->expected_value = expected_value;
    assertion->signal_name = signal_name;
    test->assertions_count += 1;
}

void write_assertion_file_path(char *assertion_file_path, const char *test_name) {
    snprintf(assertion_file_path, MAX_NAME_SIZE, "%s/%s-assertion.txt", tests_dir_path, test_name);
}

FILE *open_assertion_file(const struct dirent *tests_dirent, const char *assertion_file_path) {
    FILE *assertion_file;
    if ((assertion_file = fopen(assertion_file_path, "r")) == NULL) {
        judge_result->error = ERROR_ASSERTIONS_FILE_NOT_EXISTS;
        judge_result->passed = false;
        judge_result->passed_tests_count = 0;
        judge_result->tests_count = -1;
        fprintf(stderr, "Found '%s/%s' test file but could not open '%s' assertion file\n", tests_dir_path,
                tests_dirent->d_name, assertion_file_path);
        exit(EXIT_FAILURE);
    }
    return assertion_file;
}

void test_src_files(int src_files_count, char *const *src_file_paths) {
    for (int src_file_index = 0; src_file_index < src_files_count; ++src_file_index) {
        char *src_file_path = src_file_paths[src_file_index];
        test_src_file(src_file_path);
    }
}

void test_src_file(const char *src_file_path) {
    size_t passed_tests_count = 0;
    judge_result->passed_tests_count = passed_tests_count;
    for (int test_index = 0; test_index < tests_count; ++test_index) {
        test_t *test = &tests[test_index];
        passed_tests_count += run_test(src_file_path, test);
        judge_result->passed_tests_count = passed_tests_count;
        test_clean_up();
    }
    print_src_report(src_file_path, passed_tests_count);
}

bool run_test(const char *src_file_path, test_t *test) {
    create_vcd_file(src_file_path, test);

    vcd_t *vcd = open_vcd((char *)vcd_file_name);
    if (vcd == NULL) {
        judge_result->error = ERROR_OPENING_VCD_FILE;
        judge_result->passed = false;
        judge_result->passed_tests_count = 0;
        judge_result->tests_count = -1;
        fprintf(stderr, "An error occurred when opening .vcd file '%s'", vcd_file_name);
        exit(EXIT_FAILURE);
    }

    bool test_passed = check_assertions(test, vcd);

    if (test_passed) {
        // print_test_passed(src_file_path, test);
    } else {
        // print_test_failed(src_file_path, test);
        // print_failed_assertions_if_verbose(src_file_path, test);
    }

    return test_passed;
}

void create_vcd_file(const char *src_file_path, const test_t *test) {
    char command[PATH_MAX];
    sprintf(command, "%s %s/%s-test.v %s -o %s && ./%s > /dev/null", "iverilog", tests_dir_path, test->name,
            src_file_path, out_file_name, out_file_name);
    system(command);
}

bool check_assertion(vcd_t *vcd, assertion_result_t *assertion_result, assertion_t *assertion) {
    assertion_result->actual_value = get_value_from_vcd(vcd, assertion->signal_name, assertion->timestamp);
    if (strcmp(assertion_result->actual_value, assertion->expected_value) == 0) {
        assertion_result->passed = true;
        return true;
    }

    assertion_result->passed = false;
    return false;
}

bool check_assertions(const test_t *test, vcd_t *vcd) {
    bool test_passed = true;
    for (int assertion_index = 0; assertion_index < test->assertions_count; ++assertion_index) {
        assertion_result_t *assertion_result = (assertion_result_t *)&test->assertion_results[assertion_index];
        assertion_t *assertion = &assertion_result->assertion;
        test_passed &= check_assertion(vcd, assertion_result, assertion);
    }
    return test_passed;
}

void test_clean_up() {
    remove(out_file_name);
    remove(vcd_file_name);
}