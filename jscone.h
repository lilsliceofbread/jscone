#ifndef JSCONE_H
#define JSCONE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

#define JSCONE_TRUE 1
#define JSCONE_FALSE 0 

enum
{
    JSCONE_SUCCESS = 0,
    JSCONE_FAILURE = -1,
};

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
    u8 boolean;
} JsconeVal;

typedef struct JsconeNode
{
    struct JsconeNode* parent;
    struct JsconeNode* child;

    /* other children of parent node */
    struct JsconeNode* next;
    struct JsconeNode* prev;

    JsconeType type;
    JsconeVal value;
} JsconeNode;

JsconeNode* jscone_parse(const char* json, u32 length);

/**
 * @brief  frees output from jscone_parse. call when you are done with it.
 * @note   pass any node from the output, the function will free them all
 */
void jscone_free(JsconeNode* parsed_json);

/**
 * internal types and functions
 */

#define JSCONE_PARSER_NEXT_TOKEN(parser) jscone_lexer_next_token(&((parser)->lexer))
#define JSCONE_PARSER_GET_FIRST_CHAR(parser) ((parser)->lexer.json[(parser)->lexer.curr.first])
#define JSCONE_PARSER_GET_LAST_CHAR(parser) ((parser)->lexer.json[(parser)->lexer.curr.end - 1])
#define JSCONE_ERROR(msg, ...) fprintf(stderr,"JSCONE: " msg,##__VA_ARGS__)
//! change later so program can continue
#define JSCONE_EXPECT_CHAR(char, expected) \
if(char != expected)                                  \
{                                                     \
    JSCONE_ERROR("expected char %c", expected);       \
    return JSCONE_FAILURE;                            \
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

i32 jscone_parser_parse_object(JsconeParser* parser);
i32 jscone_parser_parse_array(JsconeParser* parser);
i32 jscone_parser_parse_string(JsconeParser* parser);
i32 jscone_parser_parse_number(JsconeParser* parser);
i32 jscone_parser_parse_enum(JsconeParser* parser);

/**
 * @note if first == end then EOF
 */
i32 jscone_lexer_next_token(JsconeLexer* lexer);
i32 jscone_lexer_lex_string(JsconeLexer* lexer);

JsconeNode* jscone_node_create(void);
void jscone_node_free(JsconeNode* node);

/**
 * user functions
 */

JsconeNode* jscone_parse(const char* json, u32 length)
{
    JsconeNode* root = jscone_node_create(); // top object

    JsconeParser parser = {
        .lexer = {
            .json = json,
            .length = length,
            .curr = {.first = 0, .end = 0}
        },
        .tree_level = 0,
        .curr_node = root
    };

    ////JSCONE_PARSER_NEXT_TOKEN(&parser);
    ////jscone_parser_parse_object(&parser);

    return root;
}

void jscone_free(JsconeNode* parsed_json)
{
    /* find top node */
    JsconeNode* head = parsed_json;
    while(head->parent != NULL)
        head = head->parent;

    jscone_node_free(head);
}

/**
 * internal functions
 */

i32 jscone_parser_parse_object(JsconeParser* parser)
{
    JsconeNode* last_node = parser->curr_node;
    /* caller should have already gone to next token */
    JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_FIRST_CHAR(parser), '{');
    while(JSCONE_PARSER_GET_FIRST_CHAR(parser) != '}')
    {
        JSCONE_PARSER_NEXT_TOKEN(parser);
        JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_FIRST_CHAR(parser), '"');
        JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_LAST_CHAR(parser), '"');
        /* parse string */

        /* parse value */
    }
    
    /* return to root node of object? */
    return JSCONE_SUCCESS;
}

i32 jscone_lexer_next_token(JsconeLexer* lexer)
{
    lexer->curr.first = lexer->curr.end;

    /* skip whitespace */
    while(JSCONE_TRUE)
    {
        if(lexer->curr.first >= lexer->length - 1)
        {
            lexer->curr.end = lexer->curr.first;
            return JSCONE_SUCCESS;
        }

        switch(lexer->json[lexer->curr.first])
        {
            case '\0':
                return JSCONE_SUCCESS;

            case '\r': case '\n': case '\t': case ' ':
                lexer->curr.first++;
                break;

            default:
                goto begin_lexing;
        }
    }

    begin_lexing:

    lexer->curr.end = lexer->curr.first;

    while(JSCONE_TRUE)
    {
        if(lexer->curr.end >= lexer->length - 1)
            return JSCONE_SUCCESS;

        switch(lexer->json[lexer->curr.end])
        {
            case '\0':
                return JSCONE_SUCCESS;

            case '{': case '}':
            case '[': case ']':
            case ':': case ',':
                /* only if not coming from contiguous characters */
                if(lexer->curr.first == lexer->curr.end)
                {
                    lexer->curr.end++;
                }

                return JSCONE_SUCCESS;

            case '\"':
                /* check if coming from contiguous characters */
                if(lexer->curr.first < lexer->curr.end)
                {
                    JSCONE_ERROR("unexpected char \"\n");
                    return JSCONE_FAILURE;
                }

                return jscone_lexer_lex_string(lexer);

            case '\r': case '\n': case '\t': case ' ':
                /* have reached end of contiguous characters  */
                return JSCONE_SUCCESS;
            
            default:
                /* contiguous characters (could be a number, true/false, null or invalid token) */
                lexer->curr.end++;
                break;
        }
    }

    return JSCONE_SUCCESS;
}

i32 jscone_lexer_lex_string(JsconeLexer* lexer)
{
    u8 escaped = JSCONE_FALSE;

    /* skip first " */
    lexer->curr.end++;

    while(JSCONE_TRUE)
    {
        if(lexer->curr.end >= lexer->length - 1)
            return JSCONE_SUCCESS;

        switch(lexer->json[lexer->curr.end])
        {
            case '\0':
                return JSCONE_SUCCESS;

            case '\\':
                /* toggle escaped incase it is escaping a backslash */
                escaped = !escaped;
                lexer->curr.end++;
                break;

            case '\"':
                lexer->curr.end++;

                /* only end string if quote is not escaped */
                if(!escaped)
                    return JSCONE_SUCCESS;

                escaped = JSCONE_FALSE;
                break;

            case '\r': case '\n':
                JSCONE_ERROR("newline before end of string\n");
                return JSCONE_FAILURE;

            
            default:
                /* characters of string */
                lexer->curr.end++;
                escaped = JSCONE_FALSE;
                break;
        }
    }
}

JsconeNode* jscone_node_create(void)
{
    JsconeNode* node = (JsconeNode*)malloc(sizeof(JsconeNode));

    node->parent = NULL;
    node->child = NULL;
    node->prev = NULL;
    node->next = NULL;
    node->type = JSCONE_NONE;
    /* don't need to zero value since type is NONE */

    return node;
}

void jscone_node_free(JsconeNode* node)
{
    if(node->type == JSCONE_STRING && node->value.str != NULL)
    {
        free(node->value.str);
    }

    if(node->prev != NULL)
    {
        jscone_node_free(node->prev);
    }
    if(node->next != NULL)
    {
        jscone_node_free(node->next);
    }
    if(node->child != NULL)
    {
        jscone_node_free(node->child);
    }

    free(node);
}

#endif