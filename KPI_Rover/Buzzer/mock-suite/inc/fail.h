#ifndef __FAIL_H
#define __FAIL_H


#define FAIL_ON(expression, return_on_fail, prefix, counter) \
	do { \
		if (!(expression)) { \
			printf("[%s %s:%d] %s: generic check \"%s\" fails\n", \
					(prefix), __FILE__, __LINE__, __func__, #expression); \
			(counter)++; \
			return (return_on_fail); \
		} \
	} while (0)

#define FAIL_ON_EQ(exp1, exp2, return_on_fail, prefix, counter) \
	do { \
		if ((exp1) == (exp2)) { \
			printf("[%s %s:%d] %s: check %s (0x%08x) != %s (0x%08x) fails\n", \
					(prefix), __FILE__, __LINE__, __func__, #exp1, (unsigned int) (exp1), #exp2, (unsigned int) (exp2)); \
			(counter)++; \
			return (return_on_fail); \
		} \
	} while (0)

#define FAIL_ON_LE(exp1, exp2, return_on_fail, prefix, counter) \
	do { \
		if ((exp1) <= (exp2)) { \
			printf("[%s %s:%d] %s: check %s (0x%08x) > %s (0x%08x) fails\n", \
					(prefix), __FILE__, __LINE__, __func__, #exp1, (unsigned int) (exp1), #exp2, (unsigned int) (exp2)); \
			(counter)++; \
			return (return_on_fail); \
		} \
	} while (0)

#define WARN_ON_EQ(exp1, exp2, prefix, counter) \
	do { \
		if ((exp1) == (exp2)) { \
			printf("[%s %s:%d] %s: check %s (0x%08x) != %s (0x%08x) fails, continuing\n", \
					(prefix), __FILE__, __LINE__, __func__, #exp1, (unsigned int) (exp1), #exp2, (unsigned int) (exp2)); \
			(counter)++; \
		} \
	} while (0)

#define REPORT_COUNTER(counter) \
	do { \
		issues += (counter); \
		if ((counter) == 1) \
			printf("[ISSUE] %s has been triggered %d time\n", \
					#counter, (counter)); \
		else if ((counter) >= 1) \
			printf("[ISSUE] %s has been triggered %d times\n", #counter, (counter)); \
	} while (0)


#endif /* __FAIL_H */
