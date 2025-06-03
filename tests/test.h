#ifndef TEST_H
#define TEST_H

/**
 ** my test system that automatically runs tests using macros (edit: doesn't work on MSVC :/)
 */

#include <string.h>
#include <stdlib.h>

#define TEST_SUCCESS 1
#define TEST_FAILURE 0

typedef unsigned char u8;
typedef u8 (*TestFunc)(void);

#define MAX_TESTS 32 

#define TEST_ASSERT(cond) TEST_ASSERT_MSG(cond, #cond);
#define TEST_ASSERT_MSG(cond, msg) if(!(cond)) {test_print(TEST_ASSERT_FAIL, msg); return TEST_FAILURE;}
#define TEST_ASSERT_STREQUAL(str1, str2) if(strcmp(str1, str2) != 0) {test_print_var(TEST_ASSERT_FAIL, "%s != %s", str1, str2); return TEST_FAILURE;}

#ifdef _MSC_VER
#define BEGIN_TESTS()
#define TEST(func_name) static u8 func_name(void)
#define END_TESTS()
#else

/* begin region of test funcs */
#define BEGIN_TESTS() static TestFunc test_funcs[MAX_TESTS]; static char* test_names[MAX_TESTS]; static int test_count = 0;

/* define test function */
#define TEST(func_name) static u8 test_##func_name(void); \
static void __attribute__((constructor)) add_test_##func_name(void)         \
{                                                                           \
    if(test_count >= MAX_TESTS)                                             \
    {                                                                       \
        test_print(TEST_LOG, "EXCEEDED MAX TESTS\n");                       \
        return;                                                             \
    }                                                                       \
    test_funcs[test_count] = test_##func_name;                              \
    test_allocate_string(&test_names[test_count], "test_" #func_name "()"); \
    test_count++;                                                           \
}                                                                           \
static u8 test_##func_name(void)                                 

/* end region of test funcs */
/* could move this into a do_tests_impl to do more stuff like sum total failed/succeeded */
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
#endif

/**
 * internal stuff
 */

#define TEST_ASSERT_FAIL 3
#define TEST_LOG 2

void test_print(u8 num, const char* msg);

void test_print_var(u8 num, const char* msg, ...);

void test_allocate_string(char** ptr, char* string);

#endif
