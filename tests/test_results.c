#include "test.h"

/* im choosing to put implementation in this test file */
#define JSCONE_IMPLEMENTATION
#include "jscone.h"

BEGIN_TESTS()

TEST(find_simple)
{
    const char* json = "{\"name1\": {\"name2\": {\"name3\": true}}}";

    JsconeNode* result = jscone_parse(json, (u32)strlen(json));
    TEST_ASSERT(result != NULL);

    JsconeNode* found_node = jscone_find(result, "/name1/name2/name3");
    TEST_ASSERT(found_node != NULL);
    TEST_ASSERT(found_node->name != NULL);
    TEST_ASSERT_STREQUAL(found_node->name, "name3");
    TEST_ASSERT(found_node->type == JSCONE_BOOL);
    TEST_ASSERT(found_node->value.bool == JSCONE_TRUE);

    jscone_free(result);

    return TEST_SUCCESS;
}

TEST(find_complex)
{
    const char* json =
        "{"
            "\"name1\\/\": {"
                "\"not1\": 0,"
                "\"not2\": 1,"
                "\"\\u0022name2\": {"
                        "\"name3\": true"
                "}"
            "},"
            "\"name1\": false"
        "}";

    JsconeNode* result = jscone_parse(json, (u32)strlen(json));
    TEST_ASSERT(result != NULL);

    JsconeNode* found_node = jscone_find(result, "/name1\\//\\u0022name2/name3");
    TEST_ASSERT(found_node != NULL);
    TEST_ASSERT(found_node->name != NULL);
    TEST_ASSERT_STREQUAL(found_node->name, "name3");
    TEST_ASSERT(found_node->type == JSCONE_BOOL);
    TEST_ASSERT(found_node->value.bool == JSCONE_TRUE);

    result = result->child;
    found_node = jscone_find(result, "name1\\/"); // test search starting from current node
    TEST_ASSERT(found_node != NULL);
    TEST_ASSERT(found_node->name != NULL);
    TEST_ASSERT_STREQUAL(found_node->name, "name1/");
    TEST_ASSERT(found_node->type == JSCONE_OBJECT);

    found_node = jscone_find(result, "name1"); 
    TEST_ASSERT(found_node != NULL);
    TEST_ASSERT(found_node->name != NULL);
    TEST_ASSERT_STREQUAL(found_node->name, "name1");
    TEST_ASSERT(found_node->type == JSCONE_BOOL);
    TEST_ASSERT(found_node->value.bool == JSCONE_FALSE);

    jscone_free(result);

    return TEST_SUCCESS;
}

END_TESTS()
