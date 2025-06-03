#ifndef JSCONE_H
#define JSCONE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JSCONE_TRUE 1
#define JSCONE_FALSE 0 

/* for jscone_print() */
#define JSCONE_MAX_INDENT 20
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
    double num;
    unsigned char bool;
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

/**
 * @param  length:  length of json string
 * @returns         root node/object
 */
JsconeNode* jscone_parse(const char* json, unsigned int length);

/**
 * @brief    finds node at specified path from the current node e.g "/world/player_data"
 * @note     use backslash \ to escape forward slashes / if they are contained within names
 * @note     if you also want to search in the current node and it's siblings first omit the first slash e.g. "world/player_data"
 * @returns  pointer to node at path
 */
JsconeNode* jscone_find(JsconeNode* node, const char* path);

/**
 * @brief  frees output from jscone_parse. call when you are done with it.
 * @note   pass any node from the output, the function will free them all
 */
void jscone_free(JsconeNode* node);

/**
 * @brief  prints out tree of specific node
 * @note   slow, should be used for debugging purposes only
 */
void jscone_print(JsconeNode* node);


/**
 * internal types and functions
 */

#define JSCONE_PARSER_NEXT_TOKEN(parser) if(jscone_lexer_next_token(&((parser)->lexer)) == JSCONE_FAILURE) { return JSCONE_FAILURE; }
#define JSCONE_PARSER_TOKEN_LENGTH(parser) ((parser)->lexer.curr.end - (parser)->lexer.curr.first)
#define JSCONE_PARSER_GET_FIRST_CHAR(parser) ((parser)->lexer.json[(parser)->lexer.curr.first])
#define JSCONE_PARSER_GET_LAST_CHAR(parser) ((parser)->lexer.json[(parser)->lexer.curr.end - 1])

#define JSCONE_ERROR(...) do { fprintf(stderr,"[JSCONE]: "); fprintf(stderr, __VA_ARGS__); } while(0)
#define JSCONE_EXPECT_CHAR(char, expected) \
if(char != expected)                              \
{                                                 \
    JSCONE_ERROR("expected char %c\n", expected); \
    return JSCONE_FAILURE;                        \
}

typedef struct
{
    unsigned int first; // first character of token in json string (index)
    unsigned int end;   // 1 past last character
} JsconeToken;

typedef struct
{
    const char* json;
    unsigned int length;
    JsconeToken curr;
} JsconeLexer;

typedef struct
{
    JsconeLexer lexer;
    JsconeNode* curr_node;
} JsconeParser;

int jscone_parser_parse_value(JsconeParser* parser, const char* name);
int jscone_parser_parse_object(JsconeParser* parser, const char* name);
int jscone_parser_parse_array(JsconeParser* parser, const char* name);
int jscone_parser_parse_number(JsconeParser* parser, const char* name);
int jscone_parser_parse_enum(JsconeParser* parser, const char* name);
int jscone_parser_parse_string(JsconeParser* parser, const char* name);
char* jscone_parser_parse_name(JsconeParser* parser);
char* jscone_parser_get_string(JsconeParser* parser);

/**
 * @note if first == end then EOF
 */
int jscone_lexer_next_token(JsconeLexer* lexer);
int jscone_lexer_lex_string(JsconeLexer* lexer);


JsconeNode* jscone_find_name_in_siblings(JsconeParser* parser, const char* name);

JsconeNode* jscone_node_create(JsconeNode* parent, JsconeType type, JsconeVal value);
void jscone_node_free(JsconeNode* node);
void jscone_node_print(JsconeNode* node, unsigned int indent);



#ifdef JSCONE_IMPLEMENTATION

static const char* jscone_get_type_name(JsconeType type);
unsigned char jscone_parse_escape_sequence(JsconeParser* parser, unsigned int offset, char* bytes);
unsigned char jscone_codepoint_to_utf8(char* bytes, const char* codepoint_str);

/**
 * exposed functions
 */

JsconeNode* jscone_parse(const char* json, unsigned int length)
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

    /* go to first token */
    if(jscone_lexer_next_token(&parser.lexer) == JSCONE_FAILURE)
    {
        JSCONE_ERROR("could not lex first token\n");
        return NULL;
    }
    
    if(JSCONE_PARSER_GET_FIRST_CHAR(&parser) == '{')
    {
        if(jscone_parser_parse_object(&parser, NULL) == JSCONE_FAILURE) // will return the root node
        {
            JSCONE_ERROR("parsing failed\n");
            jscone_free(parser.curr_node);
            return NULL;
        }
    }
    else if(JSCONE_PARSER_GET_FIRST_CHAR(&parser) == '[')
    {
        if(jscone_parser_parse_array(&parser, NULL) == JSCONE_FAILURE) // will return the root node
        {
            JSCONE_ERROR("parsing failed\n");
            jscone_free(parser.curr_node);
            return NULL;
        }
    }
    else
    {
        JSCONE_ERROR("first character not { or [\n");
        return NULL;
    }

    /* check for extra characters after json end */
    if(parser.lexer.curr.first != parser.lexer.curr.end)
    {
        jscone_lexer_next_token(&parser.lexer);

        /* if multiple characters beyond end of json */
        if(parser.lexer.curr.first != parser.lexer.curr.end)
        {
            jscone_free(parser.curr_node);
            JSCONE_ERROR("extra characters after JSON end\n");
            return NULL;
        }

        /* only 1 character beyond end of token or just whitespace */
        switch(parser.lexer.json[parser.lexer.curr.first])
        {
            case '\r': case '\n': case '\t': case ' ':
                return parser.curr_node; // allow whitespace
            default:
                JSCONE_ERROR("extra characters after JSON end\n");
                jscone_free(parser.curr_node);
                return NULL;
        }
    }

    return parser.curr_node;
}

JsconeNode* jscone_find(JsconeNode* node, const char* path)
{
    if(node == NULL || path == NULL)
    {
        return NULL;
    }

    JsconeParser parser = {
        .lexer = {
            .curr = {.first = (unsigned int)0, .end = (unsigned int)0},
            .json = path,
            .length = (unsigned int)strlen(path),
        },
        .curr_node = node,
    };
    
    /* skip current node and start search with children */
    if(path[0] == '/')
    {
        if(node->child == NULL)
        {
            return NULL;
        }

        parser.curr_node = node->child;

        /* skip past first / */
        path++;
        parser.lexer.json++;
    }

    char* curr_name = NULL;

    char c;
    unsigned char escaped = JSCONE_FALSE;
    for(unsigned int i = 0;; i++)
    {
        c = path[i];

        switch(c)
        {
            case '\\':
                escaped = !escaped;
                break;
            case '/': case '\0':
                if(escaped)
                {
                    parser.lexer.curr.end++;
                    escaped = JSCONE_FALSE;
                    continue;
                }

                /* we have a full name */

                curr_name = jscone_parser_get_string(&parser);
                if(jscone_find_name_in_siblings(&parser, curr_name) == NULL)
                {
                    return NULL;
                }
                free(curr_name);

                parser.lexer.curr.first = parser.lexer.curr.end + 1; // move ahead of /

                if(c == '\0') // bit scuffed
                {
                    return parser.curr_node;
                }
                else if(parser.curr_node->child == NULL)
                {
                    JSCONE_ERROR("reached terminating node before it was expected\n");
                    return NULL;
                }
                parser.curr_node = parser.curr_node->child;

                break;
            default:
                escaped = JSCONE_FALSE;
                break;
        }
        parser.lexer.curr.end++;
    }

    return NULL; // should not reach this
}

void jscone_free(JsconeNode* node)
{
    if(node == NULL)
    {
        return;
    }
    
    /* find top node */
    JsconeNode* head = node;
    while(head->parent != NULL)
    {
        head = head->parent;
    }

    jscone_node_free(head);
}

void jscone_print(JsconeNode* node)
{
    if(node == NULL)
    {
        return;
    }

    jscone_node_print(node, 0);
}



/**
 * internal functions
 */

JsconeNode* jscone_find_name_in_siblings(JsconeParser* parser, const char* name)
{
    while(JSCONE_TRUE)
    {
        if(strcmp(name, parser->curr_node->name) == 0)
        {
            return parser->curr_node;
        }

        if(parser->curr_node->next == NULL)
        {
            JSCONE_ERROR("could not find name %s\n", name);
            return NULL;
        }

        parser->curr_node = parser->curr_node->next;
    }

    return NULL; // should not be reached
}

/* parsing */

int jscone_parser_parse_value(JsconeParser* parser, const char* name)
{
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

    return JSCONE_FAILURE; // should not be reached
}

int jscone_parser_parse_object(JsconeParser* parser, const char* name)
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

        curr_name = jscone_parser_parse_name(parser);
        if(curr_name == NULL)
        {
            return JSCONE_FAILURE;
        }

        JSCONE_PARSER_NEXT_TOKEN(parser);
        if(JSCONE_PARSER_GET_FIRST_CHAR(parser) != ':') // expect char macro but free name as well
        {
            JSCONE_ERROR("expected char :\n");
            free((void*)curr_name);
            return JSCONE_FAILURE;
        }


        JSCONE_PARSER_NEXT_TOKEN(parser);
        if(jscone_parser_parse_value(parser, curr_name) == JSCONE_FAILURE)
        {
            free((void*)curr_name);
            return JSCONE_FAILURE;
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

int jscone_parser_parse_array(JsconeParser* parser, const char* name)
{
    JsconeNode* node_before = parser->curr_node;
    parser->curr_node = jscone_node_create(node_before, JSCONE_ARRAY, (JsconeVal){0});
    parser->curr_node->name = name;

    /* caller should have already gone to next token */
    JSCONE_EXPECT_CHAR(JSCONE_PARSER_GET_FIRST_CHAR(parser), '[');
    JSCONE_PARSER_NEXT_TOKEN(parser);
    while(JSCONE_PARSER_GET_FIRST_CHAR(parser) != ']')
    {
        if(jscone_parser_parse_value(parser, name) == JSCONE_FAILURE) // keep all names same in array
        {
            return JSCONE_FAILURE;
        }

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

    /* special case for root node */
    if(node_before != NULL)
    {
        /* reset curr_node to go back up the tree so next calls work */
        parser->curr_node = node_before;
    }
    return JSCONE_SUCCESS;
}

char* jscone_parser_parse_name(JsconeParser* parser)
{
    parser->lexer.curr.first++; // move past first "
    return jscone_parser_get_string(parser);
}

int jscone_parser_parse_string(JsconeParser* parser, const char* name)
{
    parser->lexer.curr.first++; // move past first "
    char* string = jscone_parser_get_string(parser);
    if(string == NULL)
    {
        return JSCONE_FAILURE;
    }

    JsconeNode* node = jscone_node_create(parser->curr_node, JSCONE_STRING, (JsconeVal){.str = string});
    node->name = name;
    
    return JSCONE_SUCCESS;
}

int jscone_parser_parse_number(JsconeParser* parser, const char* name)
{
    double num = 0.0f;

    unsigned int length = JSCONE_PARSER_TOKEN_LENGTH(parser);
    char* num_str = calloc(length + 1, sizeof(char));
    strncpy(num_str, parser->lexer.json + parser->lexer.curr.first, length);

    errno = 0;
    num = strtod(num_str, NULL);
    free(num_str);
    if(errno != 0)
    {
        JSCONE_ERROR("could not convert number; errno = %d: %s", errno, strerror(errno));
        return JSCONE_FAILURE;
    }


    JsconeNode* node = jscone_node_create(parser->curr_node, JSCONE_NUM, (JsconeVal){.num = num});
    node->name = name;
    return JSCONE_SUCCESS;
}

int jscone_parser_parse_enum(JsconeParser* parser, const char* name)
{
    const char* token_start = parser->lexer.json + parser->lexer.curr.first;
    unsigned int length = JSCONE_PARSER_TOKEN_LENGTH(parser);

    if(length > 5 || length < 4)
    {
        JSCONE_ERROR("characters wrong size to be true/false/null enum\n");
        return JSCONE_FAILURE;
    }

    JsconeType type;
    JsconeVal value;
    if(strncmp(token_start, "true", length) == 0)
    {
        type = JSCONE_BOOL;
        value = (JsconeVal){.bool = JSCONE_TRUE};
    }
    else if(strncmp(token_start, "false", length) == 0)
    {
        type = JSCONE_BOOL;
        value = (JsconeVal){.bool = JSCONE_FALSE};
    }
    else if(strncmp(token_start, "null", length) == 0)
    {
        type = JSCONE_NULL;
        value = (JsconeVal){0};
    }
    else
    {
        JSCONE_ERROR("characters do not match true/false/null enums\n");
        return JSCONE_FAILURE;
    }

    JsconeNode* node = jscone_node_create(parser->curr_node, type, value);
    node->name = name;

    return JSCONE_SUCCESS;
}

char* jscone_parser_get_string(JsconeParser* parser)
{
    unsigned int length = JSCONE_PARSER_TOKEN_LENGTH(parser);
    char* string = (char*)calloc(length, sizeof(char));

    char c;
    unsigned int str_i = 0; 
    unsigned char escaped = JSCONE_FALSE;
    for(unsigned int i = 0; i < length; i++) // assume starting after first "
    {
        c = parser->lexer.json[parser->lexer.curr.first + i];
        if(c == '\\')
        {
            if(escaped)
            {
                string[str_i++] = c;
            }
            escaped = !escaped;
            continue;
        }
        else if(c == '\"')
        {
            if(!escaped)
            {
                goto end_loop;
            }
            string[str_i++] = c;
            escaped = !escaped;
            continue;
        }

        /* handle escape sequence */
        if(escaped)
        {
            char bytes[5];
            unsigned char length = jscone_parse_escape_sequence(parser, i, bytes);
            if(length == 0)
            {
                free(string);
                return NULL;
            }

            for(unsigned char j = 0; j < length; j++)
            {
                string[str_i++] = bytes[j];
            }

            if(parser->lexer.json[parser->lexer.curr.first + i] == 'u') // skip 4 unicode hex characters
            {
                i += 4;
            }
            escaped = JSCONE_FALSE;
            continue;
        }
        string[str_i++] = c;
    }
    end_loop:

    /* set size to minimum */
    string = realloc(string, (str_i + 1) * sizeof(char));
    string[str_i] = '\0';

    return string;
}



/* lexing */

int jscone_lexer_next_token(JsconeLexer* lexer)
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
            case '\0': // if length is incorrect
                lexer->curr.end = --lexer->curr.first;
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
        if(lexer->curr.end > lexer->length - 1) // can be one past end (on \0)
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

int jscone_lexer_lex_string(JsconeLexer* lexer)
{
    unsigned char escaped = JSCONE_FALSE;

    /* skip first " */
    lexer->curr.end++;

    while(JSCONE_TRUE)
    {
        if(lexer->curr.end > lexer->length - 1) // one past end
        {
            JSCONE_ERROR("expected char \" before end of file\n");
            return JSCONE_FAILURE; // reached EOF before end of string
        }

        switch(lexer->json[lexer->curr.end])
        {
            case '\0':
                JSCONE_ERROR("expected char \" before end of file\n");
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

            case '\r': case '\n': case '\t':
                JSCONE_ERROR("invalid whitespace in string (newline, tab)\n");
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

    /* automatically insert child correctly */
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

void jscone_node_print(JsconeNode* node, unsigned int indent)
{
    static char indent_str[JSCONE_MAX_INDENT * JSCONE_INDENT_SIZE + 1];
    memset(indent_str, 0, JSCONE_MAX_INDENT * JSCONE_INDENT_SIZE + 1);
    if(indent > JSCONE_MAX_INDENT)
    {
        JSCONE_ERROR("cannot print json output, beyond max indentation\n");
        return;
    }

    if(node->name == NULL && node->parent == NULL)
    {
        printf("(root node) ");
    }

    memset(indent_str, ' ', indent * JSCONE_INDENT_SIZE);

    printf("%sname: %s, ", indent_str, node->name);

    printf("type: %s, ", jscone_get_type_name(node->type));

    switch(node->type)
    {
        case JSCONE_STRING:
            printf("value: %s\n", node->value.str);
            break;
        case JSCONE_NUM:
            printf("value: %lf\n", node->value.num);
            break;
        case JSCONE_BOOL:
            printf("value: %s\n", node->value.bool ? "true" : "false");
            break;
        default: 
            printf("\b\b \n"); // back twice to overwrite comma with space so no trailing comma
            break;
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

unsigned char jscone_parse_escape_sequence(JsconeParser* parser, unsigned int offset, char* bytes)
{
    const char* cp = &parser->lexer.json[parser->lexer.curr.first + offset];
    switch(*cp)
    {
        case 'u':
            return jscone_codepoint_to_utf8(bytes, ++cp); // only case that size can be other than 1
        case '/':
            bytes[0] = '/';
            break;
        case 'b':
            bytes[0] = '\b';
            break;
        case 'f':
            bytes[0] = '\f';
            break;
        case 'n':
            bytes[0] = '\n';
            break;
        case 'r':
            bytes[0] = '\r';
            break;
        case 't':
            bytes[0] = '\t';
            break;
        default:
            JSCONE_ERROR("invalid escape sequence char %c\n", *cp);
            return 0; // 0 length (error)
    }

    return 1;
}

/* thank you https://gist.github.com/MightyPork/52eda3e5677b4b03524e40c9f0ab1da5 */
unsigned char jscone_codepoint_to_utf8(char* bytes, const char* codepoint_str)
{
    unsigned int codepoint = 0;

    for(unsigned int i = 0; i < 4; i++)
    {
        char c = *(codepoint_str++);
        if(c == '\0')
        {
            JSCONE_ERROR("not enough hex characters in unicode escape sequence\n");
            return 0; // 0 length (error)
        }
        codepoint = (codepoint << 4);

        if('0' <= c && c <= '9')
        {
            codepoint += (unsigned int)(c - '0');
        }
        else if('a' <= c && c <= 'f')
        {
            codepoint += (unsigned int)(c - 'a' + 10);
        }
        else if('A' <= c && c <= 'F')
        {
            codepoint += (unsigned int)(c - 'A' + 10);
        }
        else
        {
            JSCONE_ERROR("nonvalid hex character for unicode escape sequence\n");
            return 0; // 0 length (error)
        }
    }

    if(codepoint <= 0x7F)
    {
        /* plain ascii */
        bytes[0] = (char)codepoint;
        bytes[1] = 0;
        return 1;
    }
    else if(codepoint <= 0x07FF) 
    {
        /* 2-byte unicode */
        bytes[0] = (char)(((codepoint >> 6) & 0x1F) | 0xC0);
        bytes[1] = (char)(((codepoint >> 0) & 0x3F) | 0x80);
        bytes[2] = 0;
        return 2;
    }
    else if(codepoint <= 0xFFFF)
    {
        /* 3-byte unicode */
        bytes[0] = (char)(((codepoint >> 12) & 0x0F) | 0xE0);
        bytes[1] = (char)(((codepoint >>  6) & 0x3F) | 0x80);
        bytes[2] = (char)(((codepoint >>  0) & 0x3F) | 0x80);
        bytes[3] = 0;
        return 3;
    }
    else
    { 
        /* error - use replacement character */
        bytes[0] = (char)0xEF;  
        bytes[1] = (char)0xBF;
        bytes[2] = (char)0xBD;
        bytes[3] = 0;
        return 0;
    }
}

#endif /* JSCONE_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* JSCONE_H */
