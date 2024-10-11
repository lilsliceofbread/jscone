#ifndef TEST_H
#define TEST_H

/**
 ** my test system that automatically runs tests using macros
 */

#include <string.h>
#include <stdlib.h>

#define TEST_SUCCESS 1
#define TEST_FAILURE 0

typedef unsigned char u8;
typedef u8 (*TestFunc)(void);

#define MAX_TESTS 32 

/* note: can't believe i got this to work */

/* begin region of test funcs */
#define BEGIN_TESTS() static TestFunc test_funcs[MAX_TESTS]; static char* test_names[MAX_TESTS]; static int test_count = 0;

/* define test function */
#define TEST(func_name) static u8 test_##func_name(void); \
static void __attribute__((constructor)) add_test_##func_name(void)    \
{                                                                      \
    if(test_count == MAX_TESTS)                                        \
    {                                                                  \
        test_print(TEST_LOG, "EXCEEDED MAX TESTS\n");                  \
        return;                                                        \
    }                                                                  \
    test_funcs[test_count] = test_##func_name;                         \
    test_allocate_string(&test_names[test_count], "test_" #func_name); \
    test_count++;                                                      \
}                                                                      \
static u8 test_##func_name(void)                                 

/* end region of test funcs */
#define END_TESTS() static void __attribute__((destructor)) do_tests(void) \
{                                          \
    TestFunc func = NULL;                  \
    printf("\n");                          \
    test_print(TEST_LOG, __FILE__);        \
    for(int i = 0; i < test_count; i++)    \
    {                                      \
        func = test_funcs[i];              \
        test_print(func(), test_names[i]); \
        free(test_names[i]);               \
    }                                      \
}

/**
 * internal functions
 */

#define TEST_LOG 2

void test_print(u8 num, const char* msg);

void test_allocate_string(char** ptr, char* string);

#endif