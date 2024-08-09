#ifndef JSCONE_H
#define JSCONE_H

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

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
    JSCONE_NAME, // name part of name: value pair
    JSCONE_STRING,
    JSCONE_NUM, 
    JSCONE_BOOL, 
    JSCONE_NULL,
} JsconeType;

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
 */
void jscone_free(JsconeNode* parsed_json);

/**
 * internal types and functions
 */

#define JSCONE_ERROR(msg, ...) fprintf(stderr,"JSCONE: " msg,##__VA_ARGS__)
//! change later so program can continue
#define JSCONE_EXPECT(char, expected) \
if(char != expected)                                  \
{                                                     \
    JSCONE_ERROR("expected char %c", expected);       \
    exit(-1);                                         \
}

typedef struct
{
    u32 first; // first character in json string (index)
    u32 end;  // 1 past last character
} JsconeToken;

typedef struct
{
    const char* json;
    u32 length;
    JsconeToken curr;
} JsconeLexer;

typedef struct
{
    JsconeLexer lexer;
    u32 tree_level; // how far into tree: 0 is root json object
    JsconeNode* curr_node;
} JsconeParser;

/**
 * @note if first == end then EOF
 */
bool jscone_lexer_next_token(JsconeLexer* lexer);
bool jscone_lexer_lex_string(JsconeLexer* lexer);
bool jscone_parse_object(JsconeParser* parser);

JsconeNode* jscone_parse(const char* json, u32 length)
{
    JsconeNode* root = NULL; // top object

    return root;
}

void jscone_free(JsconeNode* parsed_json)
{
    /* find top node (incase node passed is not top) */

    /* free strings */

    /* navigate to bottom nodes */

    /* free children */

    /* free top node */
}

/**
 * internal functions
 */

bool jscone_lexer_next_token(JsconeLexer* lexer)
{
    lexer->curr.first = lexer->curr.end;

    /* skip whitespace */
    while(true)
    {
        if(lexer->curr.first >= lexer->length - 1)
        {
            lexer->curr.end = lexer->curr.first;
            return true;
        }

        switch(lexer->json[lexer->curr.first])
        {
            case '\0':
                return true;

            case '\r': case '\n': case '\t': case ' ':
                lexer->curr.first++;
                break;

            default:
                goto begin_lexing;
        }
    }

    begin_lexing:

    lexer->curr.end = lexer->curr.first;

    while(true)
    {
        if(lexer->curr.end >= lexer->length - 1)
        {
            return true;
        }

        switch(lexer->json[lexer->curr.end])
        {
            case '\0':
                return true;

            case '{': case '}':
            case '[': case ']':
            case ':': case ',':
                /* only if not coming from contiguous characters */
                if(lexer->curr.first == lexer->curr.end)
                {
                    lexer->curr.end++;
                }
                return true;

            case '\"':
                /* check if coming from contiguous characters */
                if(lexer->curr.first < lexer->curr.end)
                {
                    JSCONE_ERROR("unexpected char \"\n");
                    return false;
                }
                jscone_lexer_lex_string(lexer);
                return true;

            case '\r': case '\n': case '\t': case ' ':
                /* have reached end of contiguous characters  */
                return true;
            
            default:
                /* contiguous characters (could be a number, true/false or invalid tokens) */
                lexer->curr.end++;
                break;
        }
    }

    return true;
}

bool jscone_lexer_lex_string(JsconeLexer* lexer)
{
    bool escaped = false;

    /* skip first " */
    lexer->curr.end++;
    while(true)
    {
        if(lexer->curr.end >= lexer->length - 1)
            return true;

        switch(lexer->json[lexer->curr.end])
        {
            case '\0':
                return true;

            case '\\':
                escaped = !escaped;
                lexer->curr.end++;
                break;

            case '\"':
                lexer->curr.end++;

                if(!escaped)
                    return true;

                escaped = false;
                break;

            case '\r': case '\n':
                /* if there is a newline or carriage return, end anyway */
                return true;
            
            default:
                /* characters of string */
                lexer->curr.end++;
                escaped = false;
                break;
        }
    }
}

#endif