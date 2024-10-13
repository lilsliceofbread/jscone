#include "test.h"
#include "jscone.h"
#include "test_utils.h"

/**
 * helper functions
 */

BEGIN_TESTS()

TEST(parse_string)
{
    JsconeNode* node = jscone_node_create(NULL, JSCONE_NULL, (JsconeVal){0});
    const char* test_string = "\"test123\" \"missing quote";
    JsconeParser parser = {
        .lexer = {
            .json = test_string,
            .length = (u32)strlen(test_string),
            .curr = {.first = 0, .end = 0},
        },
        .curr_node = node,
    };

    TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);
    TEST_ASSERT(parser.lexer.curr.first == 0 && parser.lexer.curr.end == 9);

    jscone_parser_parse_string(&parser, NULL); // always returns SUCCESS
    TEST_ASSERT(node->child->type == JSCONE_STRING);
    TEST_ASSERT_STREQUAL(node->child->value.str, "test123");

    jscone_node_free(node);

    TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_FAILURE);
    TEST_ASSERT(parser.lexer.curr.first == 10 && parser.lexer.curr.end == strlen(test_string));

    return TEST_SUCCESS;
}

TEST(parse_number)
{
    JsconeNode* node = jscone_node_create(NULL, JSCONE_NULL, (JsconeVal){0});
    const char* test_string = "123 1.01 1e-3 1E+4 -5e3";
    f64 values[5] = {123.0, 1.01, 0.001, 10000.0, -5000.0};

    JsconeParser parser = {
        .lexer = {
            .json = test_string,
            .length = (u32)strlen(test_string),
            .curr = {.first = 0, .end = 0},
        },
        .curr_node = node,
    };

    TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);
    
    for(int i = 0; i < 5; i++)
    {
        TEST_ASSERT_MSG(jscone_parser_parse_number(&parser, NULL) == JSCONE_SUCCESS, "jscone_parser_parse_number failed");
        TEST_ASSERT(node->child->type == JSCONE_NUM);
        TEST_ASSERT(node->child->value.num == values[i]);

        jscone_node_free(node->child);
        node->child = NULL;
        TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);
    }

    jscone_node_free(node);

    return TEST_SUCCESS;
}

TEST(parse_enum)
{
    JsconeNode* node = jscone_node_create(NULL, JSCONE_NULL, (JsconeVal){0});
    const char* test_string = "true false null toolong sho nope";
    JsconeType expected_types[3] = {JSCONE_BOOL, JSCONE_BOOL, JSCONE_NULL};

    JsconeParser parser = {
        .lexer = {
            .json = test_string,
            .length = (u32)strlen(test_string),
            .curr = {.first = 0, .end = 0},
        },
        .curr_node = node,
    };

    TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);
    
    for(int i = 0; i < 3; i++)
    {
        TEST_ASSERT_MSG(jscone_parser_parse_enum(&parser, NULL) == JSCONE_SUCCESS, "parsing failed on good token");
        TEST_ASSERT(node->child->type == expected_types[i]);

        jscone_node_free(node->child);
        node->child = NULL;
        TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);
    }

    while(parser.lexer.curr.first != parser.lexer.curr.end)
    {
        // use const name since not going to be freed
        TEST_ASSERT_MSG(jscone_parser_parse_enum(&parser, NULL) == JSCONE_FAILURE, "parsing did not fail on bad token");
        TEST_ASSERT(node->child == NULL);

        TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);
    }

    jscone_node_free(node);

    return TEST_SUCCESS;
}

TEST(parse_empty_array)
{
    JsconeNode* node = jscone_node_create(NULL, JSCONE_NULL, (JsconeVal){0});
    const char* test_string = "[]";
    JsconeParser parser = {
        .lexer = {
            .json = test_string,
            .length = (u32)strlen(test_string),
            .curr = {.first = 0, .end = 0},
        },
        .curr_node = node,
    };

    const char* name = "array";

    TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);

    jscone_parser_parse_array(&parser, test_parser_allocate_string(name));

    TEST_ASSERT(parser.curr_node == node); // should be reset by jscone_parser_parse_array

    JsconeNode* array = node->child;
    TEST_ASSERT(array->type == JSCONE_ARRAY);
    TEST_ASSERT_STREQUAL(array->name, name);
    TEST_ASSERT(array->child == NULL);

    jscone_node_free(node);

    return TEST_SUCCESS;
}

TEST(parse_one_item_array)
{
    JsconeNode* node = jscone_node_create(NULL, JSCONE_NULL, (JsconeVal){0});
    const char* test_string = "[\"this is an array!\"]";
    JsconeParser parser = {
        .lexer = {
            .json = test_string,
            .length = (u32)strlen(test_string),
            .curr = {.first = 0, .end = 0},
        },
        .curr_node = node,
    };

    const char* name = "array";

    TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);

    TEST_ASSERT(jscone_parser_parse_array(&parser, test_parser_allocate_string(name)) == JSCONE_SUCCESS);

    JsconeNode* array = node->child;
    TEST_ASSERT(array->child != NULL);

    TEST_ASSERT_STREQUAL(array->child->name, name);
    TEST_ASSERT(array->child->type == JSCONE_STRING);
    TEST_ASSERT_STREQUAL(array->child->value.str, "this is an array!");
    TEST_ASSERT(array->child->next == NULL);

    jscone_node_free(node);

    return TEST_SUCCESS;
}

TEST(parse_complex_array)
{
    JsconeNode* node = jscone_node_create(NULL, JSCONE_NULL, (JsconeVal){0});
    const char* test_string = "[42, \"element 2\", true, [\"yes\", 3.50]]";
    JsconeType expected_types[5] = {JSCONE_NUM, JSCONE_STRING, JSCONE_BOOL, JSCONE_STRING, JSCONE_NUM};
    JsconeVal values[5] = {
        (JsconeVal){.num = 42.0},
        (JsconeVal){.str = "element 2"},
        (JsconeVal){.bool = JSCONE_TRUE},
        (JsconeVal){.str = "yes"},
        (JsconeVal){.num = 3.5},
    };
    JsconeParser parser = {
        .lexer = {
            .json = test_string,
            .length = (u32)strlen(test_string),
            .curr = {.first = 0, .end = 0},
        },
        .curr_node = node,
    };

    const char* name = "array";

    TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);

    TEST_ASSERT(jscone_parser_parse_array(&parser, test_parser_allocate_string(name)) == JSCONE_SUCCESS);

    JsconeNode* array = node->child;
    TEST_ASSERT(array->child != NULL);
    JsconeNode* child = array->child;

    for(int i = 0; i < 3; i++)
    {
        TEST_ASSERT_STREQUAL(child->name, name);
        TEST_ASSERT(child->type == expected_types[i]);
        TEST_ASSERT_VALUES_EQUAL(child, values[i]);

        TEST_ASSERT(child->next != NULL);
        child = child->next;
    }

    // now on 4th element, which should be nested array
    array = child;
    TEST_ASSERT(array->type == JSCONE_ARRAY);
    TEST_ASSERT_STREQUAL(array->name, name);
    TEST_ASSERT(array->child != NULL);
    child = array->child;

    for(int i = 3; i < 5; i++)
    {
        TEST_ASSERT_STREQUAL(child->name, name);
        TEST_ASSERT(child->type == expected_types[i]);
        TEST_ASSERT_VALUES_EQUAL(child, values[i]);

        child = child->next;
    }

    jscone_node_free(node);

    return TEST_SUCCESS;
}

TEST(parse_empty_object)
{
    JsconeNode* node = jscone_node_create(NULL, JSCONE_NULL, (JsconeVal){0});
    const char* test_string = "{}";
    JsconeParser parser = {
        .lexer = {
            .json = test_string,
            .length = (u32)strlen(test_string),
            .curr = {.first = 0, .end = 0},
        },
        .curr_node = node,
    };

    const char* name = "object";

    TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);

    jscone_parser_parse_object(&parser, test_parser_allocate_string(name));

    TEST_ASSERT(parser.curr_node == node); // should be reset by jscone_parser_parse_object

    JsconeNode* object = node->child;
    TEST_ASSERT(object->type == JSCONE_OBJECT);
    TEST_ASSERT_STREQUAL(object->name, name);
    TEST_ASSERT(object->child == NULL);

    jscone_node_free(node);

    return TEST_SUCCESS;
}

TEST(parse_one_item_object)
{
    JsconeNode* node = jscone_node_create(NULL, JSCONE_NULL, (JsconeVal){0});
    const char* test_string = "{\"element\": 9}";
    JsconeParser parser = {
        .lexer = {
            .json = test_string,
            .length = (u32)strlen(test_string),
            .curr = {.first = 0, .end = 0},
        },
        .curr_node = node,
    };

    TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);

    TEST_ASSERT(jscone_parser_parse_object(&parser, NULL) == JSCONE_SUCCESS);

    JsconeNode* object = node->child;
    TEST_ASSERT(object->child != NULL);

    TEST_ASSERT(object->child->type == JSCONE_NUM);
    TEST_ASSERT_STREQUAL(object->child->name, "element");
    TEST_ASSERT(object->child->value.num == 9.0);
    TEST_ASSERT(object->child->next == NULL);

    jscone_node_free(node);

    return TEST_SUCCESS;
}

TEST(parse_complex_object)
{
    JsconeNode* node = jscone_node_create(NULL, JSCONE_NULL, (JsconeVal){0});
    const char* test_string = "{\"name1\": 42, \"name2\": \"element 2\", \"name3\": true, \"name4\": {\"name5\": \"yes\", \"name6\": 3.50}}";
    JsconeType expected_types[5] = {JSCONE_NUM, JSCONE_STRING, JSCONE_BOOL, JSCONE_STRING, JSCONE_NUM};
    JsconeVal values[5] = {
        (JsconeVal){.num = 42.0},
        (JsconeVal){.str = "element 2"},
        (JsconeVal){.bool = JSCONE_TRUE},
        (JsconeVal){.str = "yes"},
        (JsconeVal){.num = 3.5},
    };
    JsconeParser parser = {
        .lexer = {
            .json = test_string,
            .length = (u32)strlen(test_string),
            .curr = {.first = 0, .end = 0},
        },
        .curr_node = node,
    };

    TEST_ASSERT(jscone_lexer_next_token(&parser.lexer) == JSCONE_SUCCESS);

    TEST_ASSERT(jscone_parser_parse_object(&parser, NULL) == JSCONE_SUCCESS);

    JsconeNode* object = node->child;
    TEST_ASSERT(object->child != NULL);
    JsconeNode* child = object->child;

    char expected_name[6] = "name1";
    for(int i = 0; i < 3; i++)
    {
        TEST_ASSERT_STREQUAL(child->name, expected_name);
        TEST_ASSERT(child->type == expected_types[i]);
        TEST_ASSERT_VALUES_EQUAL(child, values[i]);

        TEST_ASSERT(child->next != NULL);
        child = child->next;
        expected_name[4]++;
    }

    // now on 4th element, which should be nested object
    object = child;
    TEST_ASSERT(object->type == JSCONE_OBJECT);
    TEST_ASSERT_STREQUAL(object->name, expected_name);
    TEST_ASSERT(object->child != NULL);
    child = object->child;
    expected_name[4]++;

    for(int i = 3; i < 5; i++)
    {
        TEST_ASSERT_STREQUAL(child->name, expected_name);
        TEST_ASSERT(child->type == expected_types[i]);
        TEST_ASSERT_VALUES_EQUAL(child, values[i]);

        child = child->next;
        expected_name[4]++;
    }

    jscone_node_free(node);

    return TEST_SUCCESS;
}

END_TESTS()