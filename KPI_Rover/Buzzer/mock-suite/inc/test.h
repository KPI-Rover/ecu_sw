#ifndef __TEST_H
#define __TEST_H


#include <stdio.h>

#include "colors.h"

#define INIT(...) int failed_tests = 0, passed_tests = 0;

#define TEST(test_func) \
	do { \
		int return_code = test_func(); \
		if (return_code != 0) { \
			printf(COLOR_RED "TEST FAILED" COLOR_RESET ": %s, return code %08x\n", #test_func, (unsigned int) return_code); \
			failed_tests++; \
		} else { \
			printf(COLOR_GREEN "TEST PASSED" COLOR_RESET ": %s\n", #test_func); \
			passed_tests++; \
		} \
	} while (0)

#define SUMMARY(...) \
	do { \
		if (failed_tests) { \
			printf("--- " COLOR_GREEN "%d" COLOR_RESET " passed / " COLOR_RED "%d" COLOR_RESET " failed ---\n", passed_tests, failed_tests); \
		} else { \
			printf("--- " COLOR_GREEN "%d" COLOR_RESET " passed ---\n", passed_tests); \
		} \
		return !!failed_tests; \
	} while (0)


#endif /* __TEST_H */
