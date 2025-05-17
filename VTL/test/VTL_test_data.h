#ifndef _VTL_TEST_DATA_H
#define _VTL_TEST_DATA_H

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdio.h>

extern unsigned int VTL_global_test_fails;

#define VTL_TEST_FAIL_TEXT "Fail:"
#define VTL_TEST_STR_FORMAT  VTL_TEST_FAIL_TEXT"%s (%s:%d)\n"

#define VTL_ASSERT(condition, message) \
    if (!(condition)) { \
        printf(VTL_TEST_STR_FORMAT, message, __FILE__, __LINE__); \
        ++VTL_global_test_fails; \
    }


#define TEST(func_name) void func_name() 

#define VTL_RUN_TEST_TEXT "Test starts:"
#define VTL_RUN_TEST_TEXt_FORMAT  VTL_RUN_TEST_TEXT"%s\n"
#define VTL_RUN_TEST(func_name) \
    printf("Running test: %s\n", #func_name); \
    func_name(); \


#define VTL_RUN_TESTS_PASSED "All tests passed."
#define VTL_RUN_TESTS_FAILS_FORMAT "Failed tests count: %d.\n"

int VTL_test_result();

// VTL_StandartString


#ifdef __cplusplus
}
#endif


#endif