#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdlib.h>
#include <string.h>

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