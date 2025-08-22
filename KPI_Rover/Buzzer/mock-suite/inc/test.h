#ifndef __TEST_H
#define __TEST_H


#include <stdio.h>

#define TEST(test_func) \
	do { \
		int return_code = test_func(); \
		if (return_code != 0) { \
			printf("TEST FAILED: %s, return code %08x\n", #test_func, (unsigned int) return_code); \
		} else { \
			printf("TEST PASSED: %s\n", #test_func); \
		} \
	} while (0)


#endif /* __TEST_H */
