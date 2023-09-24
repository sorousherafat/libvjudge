#ifndef VJUDGE_VJUDGE_H
#define VJUDGE_VJUDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "libvcd.h"

#define VJUDGE_MAX_NAME_SIZE 128
#define VJUDGE_MAX_ASSERTIONS_NO 1024
#define VJUDGE_MAX_TESTS_NO 64

typedef enum {
    NO_ERROR = 0,
    ERROR_OPENING_VCD_FILE,
    ERROR_COMPILING_VERILOG_FILE,
    ERROR_ASSERTIONS_FILE_WRONG_FORMAT,
    ERROR_ASSERTIONS_FILE_NOT_EXISTS,
    ERROR_OPENING_TESTS_DIRECTORY,
    ERROR_OPENING_SOURCE_FILE
} error_t;

typedef struct {
    char *signal_name;
    char *expected_value;
    timestamp_t timestamp;
} assertion_t;

typedef struct {
    assertion_t assertion;
    bool passed;
    char *actual_value;
} assertion_result_t;

typedef struct {
    char *name;
    bool passed;
    size_t passed_assertions_count;
    size_t assertions_count;
    assertion_result_t assertion_results[VJUDGE_MAX_ASSERTIONS_NO];
} test_t;

typedef struct {
    bool passed;
    size_t passed_tests_count;
    size_t tests_count;
    test_t tests[VJUDGE_MAX_TESTS_NO];
    error_t error;
} judge_result_t;

#endif // VJUDGE_VJUDGE_H
