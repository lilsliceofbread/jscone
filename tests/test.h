#ifndef TEST_H
#define TEST_H

/**
 ** my test system that automatically runs tests using functions which run before/after main
 ** (should) work on gcc and clang, doesn't work on msvc (yet?)
 */

#include <string.h>
#include <stdlib.h>

#define TEST_SUCCESS 1
#define TEST_FAILURE 0

typedef unsigned char u8;
typedef u8 (*TestFunc)(void);

#define MAX_TESTS 32 

#define TEST_SUPPRESS_OUTPUT() test_suppress_output(&stdout_fd, &stderr_fd)
#define TEST_RESUME_OUTPUT() test_resume_output(stdout_fd, stderr_fd)

#define TEST_PRINT(type, msg) TEST_RESUME_OUTPUT(); test_print(type, msg); TEST_SUPPRESS_OUTPUT()
#define TEST_PRINT_VAR(type, ...) TEST_RESUME_OUTPUT(); test_print_var(type, __VA_ARGS__); TEST_SUPPRESS_OUTPUT()

#define TEST_ASSERT(cond) TEST_ASSERT_MSG(cond, #cond);
#define TEST_ASSERT_MSG(cond, msg) if(!(cond)) {TEST_PRINT(TEST_ASSERT_FAIL, msg); return TEST_FAILURE;}
#define TEST_ASSERT_STREQUAL(str1, str2) if(strcmp(str1, str2) != 0) {TEST_PRINT_VAR(TEST_ASSERT_FAIL, "%s != %s", str1, str2); return TEST_FAILURE;}

/* no implementation for MSVC */
#if defined(_MSC_VER)
/* should prevent compiler errors */
#define BEGIN_TESTS() static TestFunc test_funcs[MAX_TESTS]; static char* test_names[MAX_TESTS]; static int test_count = 0; static int stdout_fd = 0; static int stderr_fd = 0;
#define TEST(func_name) static u8 func_name(void)
#define END_TESTS()
#else

/* begin region of test funcs */
#define BEGIN_TESTS() static TestFunc test_funcs[MAX_TESTS]; static char* test_names[MAX_TESTS]; static int test_count = 0; static int stdout_fd = 0; static int stderr_fd = 0;

/* define test function and add to list */
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

/* run test funcs and print results */
#define END_TESTS() static void __attribute__((destructor)) do_tests(void) \
{                                          \
    TestFunc func = NULL;                  \
    u8 ret = TEST_FAILURE;                 \
    printf("\n");                          \
    test_print(TEST_LOG, __FILE__);        \
    for(int i = 0; i < test_count; i++)    \
    {                                      \
        func = test_funcs[i];              \
        TEST_SUPPRESS_OUTPUT();            \
        ret = func();                      \
        TEST_RESUME_OUTPUT();              \
        test_print(ret, test_names[i]);    \
        free(test_names[i]);               \
    }                                      \
}
#endif

#define TEST_ASSERT_FAIL 3
#define TEST_LOG 2

void test_suppress_output(int* stdout_fd, int* stderr_fd);

void test_resume_output(int stdout_fd, int stderr_fd);

void test_print(u8 num, const char* msg);

void test_print_var(u8 num, const char* msg, ...);

void test_allocate_string(char** ptr, char* string);

#endif
