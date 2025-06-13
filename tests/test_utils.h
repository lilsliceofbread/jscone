#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdlib.h>
#include <string.h>

//typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef float f32;
typedef double f64;

#define TEST_ASSERT_VALUES_EQUAL(node, val) \
if(node->type == JSCONE_STRING)                     \
{                                                   \
    TEST_ASSERT_STREQUAL(node->value.str, val.str); \
}                                                   \
else if(node->type == JSCONE_BOOL)                  \
{                                                   \
    TEST_ASSERT(node->value.bool == val.bool);      \
}                                                   \
else                                                \
{                                                   \
    TEST_ASSERT(node->value.num == val.num);        \
}

static inline char* test_parser_allocate_string(const char* name)
{
    char* name_ptr = (char*)calloc((strlen(name) + 1), sizeof(char));
    strcpy(name_ptr, name);
    return name_ptr;
}

#endif
