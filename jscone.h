#ifndef JSCONE_H
#define JSCONE_H

#include <string.h>
#include <stdbool.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef float f32;
typedef double f64;

typedef enum
{
    JSCONE_NONE,
    JSCONE_OBJECT,
    JSCONE_ARRAY,
    JSCONE_STRING,
    JSCONE_NUM, 
    JSCONE_BOOL, 
    JSCONE_NULL,
} JsconeType;

typedef struct
{
    JsconeType type;
    u32 str;  // index into json file string (must keep around)
    u32 length; // length of string
} JsconeToken;

typedef struct
{
    u32 token_last; // token for this level in tree?
    u32 token;
} JsconeParser;

typedef union
{
    char* str; // malloced
    f64 num;
    bool boolean;
} JsconeVal;

typedef struct JsconeNode // generate tree structure?
{
    struct JsconeNode* child;
    struct JsconeNode* next; // for arrays and objects
    struct JsconeNode* prev;

    JsconeType type;
    JsconeVal value;
} JsconeNode;

JsconeNode* jscone_parse(const char* json, u32 length);

/**
 * @brief  frees output from jscone_parse. call when you are done with it.
 * @note   
 * @param  node: output from json parser
 * @returns None
 */
void jscone_free(JsconeNode* parsed_json);

JsconeNode* jscone_parse(const char* json, u32 length)
{
    return NULL;
}

void jscone_free(JsconeNode* parsed_json)
{
    /* find top node (incase node passed is not top) */

    /* free strings */

    /* navigate to bottom nodes */

    /* free children */

    /* free top node */
}

#endif