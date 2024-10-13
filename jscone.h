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

/* for jscone_print() */
#define JSCONE_MAX_INDENT 8
#define JSCONE_INDENT_SIZE 4

enum
{
    JSCONE_SUCCESS = 0,
    JSCONE_FAILURE = -1,
};

typedef enum
{
    JSCONE_NULL,
    JSCONE_BOOL, 
    JSCONE_NUM, 
    JSCONE_STRING,
    JSCONE_OBJECT,
    JSCONE_ARRAY,
    JSCONE_TYPE_COUNT, // amount of types
} JsconeType;

typedef union
{
    char* str; // malloced
    f64 num;
    u8 bool;
} JsconeVal;

typedef struct JsconeNode
{
    struct JsconeNode* parent;
    struct JsconeNode* child;

    /* other children of parent node */
    struct JsconeNode* next;
    struct JsconeNode* prev;

    const char* name; // malloced
    JsconeType type;
    JsconeVal value;
} JsconeNode;

/**
 * exposed functions
 */

JsconeNode* jscone_parse(const char* json, u32 length);

/**
 * @brief    finds node at specified path e.g "world/player_data"
 * @returns  pointer to node at path
 */
JsconeNode* jscone_find(JsconeNode* parsed_json, const char* path);

/**
 * @brief  frees output from jscone_parse. call when you are done with it.
 * @note   pass any node from the output, the function will free them all
 */
void jscone_free(JsconeNode* parsed_json);

/**
 * @brief  prints out tree of parsed_json
 * @note   slow, should be used for debugging purposes only
 */
void jscone_print(JsconeNode* parsed_json);


/**
 * internal types and functions
 */

/* shouldn't cause leaked memory as the output can still be freed */
#define JSCONE_PARSER_NEXT_TOKEN(parser) if(jscone_lexer_next_token(&((parser)->lexer)) == JSCONE_FAILURE) return JSCONE_FAILURE; 
#define JSCONE_PARSER_TOKEN_LENGTH(parser) ((parser)->lexer.curr.end - (parser)->lexer.curr.first)
#define JSCONE_PARSER_GET_FIRST_CHAR(parser) ((parser)->lexer.json[(parser)->lexer.curr.first])
#define JSCONE_PARSER_GET_LAST_CHAR(parser) ((parser)->lexer.json[(parser)->lexer.curr.end - 1])

#define JSCONE_ERROR(msg, ...) fprintf(stderr,"[JSCONE]: " msg,##__VA_ARGS__)
#define JSCONE_EXPECT_CHAR(char, expected) \
if(char != expected)                                  \
{                                                     \
    JSCONE_ERROR("expected char %c\n", expected);       \
    return JSCONE_FAILURE;                            \
}

typedef struct
{
    u32 first; // first character in json string (index)
    u32 end;   // 1 past last character
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
    JsconeNode* curr_node; // current parent i.e. only the current object/array
} JsconeParser;

i32 jscone_parser_parse_value(JsconeParser* parser, const char* name);

i32 jscone_parser_parse_object(JsconeParser* parser, const char* name);
i32 jscone_parser_parse_array(JsconeParser* parser, const char* name);
i32 jscone_parser_parse_string(JsconeParser* parser, const char* name);
i32 jscone_parser_parse_number(JsconeParser* parser, const char* name);
i32 jscone_parser_parse_enum(JsconeParser* parser, const char* name);

/**
 * @note if first == end then EOF
 */
i32 jscone_lexer_next_token(JsconeLexer* lexer);
i32 jscone_lexer_lex_string(JsconeLexer* lexer);

JsconeNode* jscone_node_create(JsconeNode* parent, JsconeType type, JsconeVal value);
void jscone_node_free(JsconeNode* node);
void jscone_node_print(JsconeNode* node, u32 indent);



#ifdef JSCONE_IMPLEMENTATION

static const char* jscone_get_type_name(JsconeType type);
char* jscone_remove_string_quotes(const char* first_char, u32 length);

/**
 * exposed functions
 */

JsconeNode* jscone_parse(const char* json, u32 length)
{
    /* top object */
    JsconeParser parser = {
        .lexer = {
            .json = json,
            .length = length,
            .curr = {.first = 0, .end = 0}
        },
        .curr_node = NULL,
    };

    // can't use macro because JSCONE_FAILURE != NULL
    if(jscone_lexer_next_token(&parser.lexer) == JSCONE_FAILURE)
    {
        return NULL;
    }
    jscone_parser_parse_object(&parser, NULL); // will return the root node

    return parser.curr_node;
}

// TODO
/* 
JsconeNode* jscone_find(JsconeNode* root, const char* path)
{

}
*/

void jscone_free(JsconeNode* parsed_json)
{
    /* find top node */
    JsconeNode* head = parsed_json;
    while(head->parent != NULL)
    {
        head = head->parent;
    }

    jscone_node_free(head);
}

void jscone_print(JsconeNode* parsed_json)
{
    jscone_node_print(parsed_json, 0);
}



/**
 * internal functions
 */

/* parsing */

i32 jscone_parser_parse_value(JsconeParser* parser, const char* name)
{
    /**
     * potential errors with this if user forgets initial { or [ in object/array
     * but (hopefully) should be caught by EXPECT_CHAR in object/array parse funcs
     * edit: technically a lot of issues are required to be caught by EXPECT_CHAR
     * such as if the file ends early
     */
    /* determine type */
    switch(JSCONE_PARSER_GET_FIRST_CHAR(parser))
    {
        case '{':
            return jscone_parser_parse_object(parser, name);
        case '[':
            return jscone_parser_parse_array(parser, name);
        case '\"':
            return jscone_parser_parse_string(parser, name);
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '-': case '.':
            return jscone_parser_parse_number(parser, name);
        default:
            return jscone_parser_parse_enum(parser, name);
    }

    return JSCONE_FAILURE; // how did we get here?
}

i32 jscone_parser_parse_object(JsconeParser* parser, const char* name)
{
    JsconeNode* node_before = parser->curr_node;
    parser->curr_node = jscone_node_create(node_before, JSCONE_OBJECT, (JsconeVal){0});
    parser->curr_node->name = name;
    char* curr_name = NULL;

    /* caller should have already gone to next token */
    JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_FIRST_CHAR(parser), '{');
    JSCONE_PARSER_NEXT_TOKEN(parser);
    while(JSCONE_PARSER_GET_FIRST_CHAR(parser) != '}')
    {
        JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_FIRST_CHAR(parser), '\"');
        JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_LAST_CHAR(parser), '\"');

        curr_name = jscone_remove_string_quotes(&JSCONE_PARSER_GET_FIRST_CHAR(parser), JSCONE_PARSER_TOKEN_LENGTH(parser));

        JSCONE_PARSER_NEXT_TOKEN(parser);
        JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_FIRST_CHAR(parser), ':');

        JSCONE_PARSER_NEXT_TOKEN(parser);
        if(jscone_parser_parse_value(parser, curr_name) == JSCONE_FAILURE)
        {
            free((void*)curr_name);
        }

        JSCONE_PARSER_NEXT_TOKEN(parser);
        if(JSCONE_PARSER_GET_FIRST_CHAR(parser) == ',')
        {
            JSCONE_PARSER_NEXT_TOKEN(parser);
        }
        else
        {
            JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_FIRST_CHAR(parser), '}');
        }
    }
    
    /* special case for root node */
    if(node_before != NULL)
    {
        /* reset curr_node to go back up the tree so next calls work */
        parser->curr_node = node_before;
    }
    return JSCONE_SUCCESS;
}

i32 jscone_parser_parse_array(JsconeParser* parser, const char* name)
{
    JsconeNode* node_before = parser->curr_node;
    parser->curr_node = jscone_node_create(node_before, JSCONE_ARRAY, (JsconeVal){0});
    parser->curr_node->name = name;

    /* caller should have already gone to next token */
    JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_FIRST_CHAR(parser), '[');
    JSCONE_PARSER_NEXT_TOKEN(parser);
    while(JSCONE_PARSER_GET_FIRST_CHAR(parser) != ']')
    {
        jscone_parser_parse_value(parser, name); // keep all names same in array

        JSCONE_PARSER_NEXT_TOKEN(parser);
        if(JSCONE_PARSER_GET_FIRST_CHAR(parser) == ',')
        {
            JSCONE_PARSER_NEXT_TOKEN(parser);
        }
        else
        {
            JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_FIRST_CHAR(parser), ']');
        }
    }

    /* reset curr_node to go back up the tree so next calls work */
    parser->curr_node = node_before;
    return JSCONE_SUCCESS;
}

i32 jscone_parser_parse_string(JsconeParser* parser, const char* name)
{
    /**
     * json strings allow escaping forward slashes "\/" and c doesn't
     * and I can't be bothered to handle that, lets hope it never comes up
     */
    char* string = jscone_remove_string_quotes(&JSCONE_PARSER_GET_FIRST_CHAR(parser), JSCONE_PARSER_TOKEN_LENGTH(parser));

    JsconeNode* node = jscone_node_create(parser->curr_node, JSCONE_STRING, (JsconeVal){.str = string});
    node->name = name;
    
    return JSCONE_SUCCESS;
}

i32 jscone_parser_parse_number(JsconeParser* parser, const char* name)
{
    f64 num = 0.0f;

    u32 length = parser->lexer.curr.end - parser->lexer.curr.first;
    char* num_str = calloc(length + 1, sizeof(char));
    strncpy(num_str, parser->lexer.json + parser->lexer.curr.first, length);

    num = strtod(num_str, NULL);

    free(num_str);

    JsconeNode* node = jscone_node_create(parser->curr_node, JSCONE_NUM, (JsconeVal){.num = num});
    node->name = name;
    return JSCONE_SUCCESS;
}

i32 jscone_parser_parse_enum(JsconeParser* parser, const char* name)
{
    char str[5] = {0}; // 5 since false is max length enum name
    u32 length = JSCONE_PARSER_TOKEN_LENGTH(parser);

    if(length > 5 || length < 4)
    {
        return JSCONE_FAILURE;
    }

    JsconeType type;
    JsconeVal value;
    strncpy(str, &JSCONE_PARSER_GET_FIRST_CHAR(parser), length);
    if(strcmp(str, "true") == 0)
    {
        type = JSCONE_BOOL;
        value = (JsconeVal){.bool = JSCONE_TRUE};
    }
    else if(strcmp(str, "false") == 0)
    {
        type = JSCONE_BOOL;
        value = (JsconeVal){.bool = JSCONE_FALSE};
    }
    else if(strcmp(str, "null") == 0)
    {
        type = JSCONE_NULL;
        value = (JsconeVal){0};
    }
    else
    {
        return JSCONE_FAILURE;
    }

    JsconeNode* node = jscone_node_create(parser->curr_node, type, value);
    node->name = name;

    return JSCONE_SUCCESS;
}

/* lexing */

i32 jscone_lexer_next_token(JsconeLexer* lexer)
{
    lexer->curr.first = lexer->curr.end;

    /* skip whitespace */
    while(JSCONE_TRUE)
    {
        if(lexer->curr.first >= lexer->length - 1)
        {
            lexer->curr.end = lexer->curr.first;
            return JSCONE_SUCCESS; // still thinking about this one. should not matter because root object should end before EOF anyway
        }

        switch(lexer->json[lexer->curr.first])
        {
            case '\0':
                return JSCONE_FAILURE; // if no tokens left, fail

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
        if(lexer->curr.end > lexer->length - 1) // can be one past end
        {
            return JSCONE_SUCCESS;
        }

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
        if(lexer->curr.end > lexer->length - 1) // one past end
        {
            return JSCONE_FAILURE; // reached EOF before end of string
        }

        switch(lexer->json[lexer->curr.end])
        {
            case '\0':
                return JSCONE_FAILURE; // reached EOF before end of string

            case '\\':
                /* toggle escaped incase it is escaping a backslash */
                escaped = !escaped;
                lexer->curr.end++;
                break;

            case '\"':
                lexer->curr.end++;

                /* only end string if quote is not escaped */
                if(!escaped)
                {
                    return JSCONE_SUCCESS;
                }

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

/* nodes */

JsconeNode* jscone_node_create(JsconeNode* parent, JsconeType type, JsconeVal value)
{
    JsconeNode* node = (JsconeNode*)malloc(sizeof(JsconeNode));

    node->parent = parent;
    node->type = type;
    node->value = value;
    node->child = NULL;
    node->prev = NULL;
    node->next = NULL;
    node->name = NULL;

    // automatically insert child correctly
    if(parent != NULL)
    {
        if(parent->child == NULL)
        {
            parent->child = node;
            return node;
        }

        // other children already exist
        JsconeNode* last_child = parent->child;
        while(last_child->next != NULL)
        {
            last_child = last_child->next;
        }
        last_child->next = node;
        node->prev = last_child;
    }

    return node;
}

void jscone_node_free(JsconeNode* node)
{
    if(node->name != NULL && node->parent->type != JSCONE_ARRAY) // array sub-nodes have same name ptr as parent array node
    {
        free((void*)node->name);
    }
    if(node->value.str != NULL && node->type == JSCONE_STRING)
    {
        free(node->value.str);
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

void jscone_node_print(JsconeNode* node, u32 indent)
{
    char indent_str[JSCONE_MAX_INDENT * JSCONE_INDENT_SIZE] = {0};
    if(indent > JSCONE_MAX_INDENT)
    {
        JSCONE_ERROR("cannot print json output, beyond max indentation\n");
        return;
    }

    if(node->name == NULL && indent == 0)
    {
        printf("root node/object\n");
    }

    memset(indent_str, ' ', indent * JSCONE_INDENT_SIZE);

    if(node->prev != NULL && node->prev->child == NULL)
    {
        printf("%s============\n", indent_str);
    }

    printf("%sname: %s\n", indent_str, node->name);

    printf("%stype: %s\n", indent_str, jscone_get_type_name(node->type));

    switch(node->type)
    {
        case JSCONE_STRING:
            printf("%svalue: %s\n", indent_str, node->value.str);
            break;
        case JSCONE_NUM:
            printf("%svalue: %lf\n", indent_str, node->value.num);
            break;
        case JSCONE_BOOL:
            printf("%svalue: %s\n", indent_str, node->value.bool ? "true" : "false");
            break;
        default: break;
    }

    if(node->child != NULL)
    {
        jscone_node_print(node->child, indent + 1);
    }

    if(node->next != NULL)
    {
        jscone_node_print(node->next, indent);
    }
}

static const char* jscone_get_type_name(JsconeType type)
{
    static const char* type_names[JSCONE_TYPE_COUNT] = {"NULL", "BOOL", "NUM", "STRING", "OBJECT", "ARRAY"};
    return type_names[type];
}

char* jscone_remove_string_quotes(const char* first_char, u32 length)
{
    char* string = calloc(length, sizeof(char));

    strncpy(string, first_char + 1, length - 2);
    return string;
}

#endif

#endif