#include "test.h"
#include "jscone.h"
#include <stdlib.h>
#include "test_utils.h"

BEGIN_TESTS()

TEST(multiple_child_nodes)
{
    JsconeNode* parent = jscone_node_create(NULL, JSCONE_NULL, (JsconeVal){0});

    const char* string = "string";

    JsconeVal values[6] = {
        (JsconeVal){0},
        (JsconeVal){.bool = JSCONE_TRUE},
        (JsconeVal){.num = 3.0f},
        (JsconeVal){.str = test_parser_allocate_string(string)},
        (JsconeVal){0},
        (JsconeVal){0},
    };

    for(int i = 0; i < JSCONE_TYPE_COUNT; i++)
    {
        jscone_node_create(parent, (JsconeType)(JSCONE_NULL + i), values[i]);

        JsconeNode* child = parent->child;
        JsconeNode* prev = NULL;
        TEST_ASSERT(child->prev == NULL);

        for(int j = 0; j < i; j++)
        {
            prev = child;
            child = child->next;
        }

        TEST_ASSERT(child->type == (JsconeType)(JSCONE_NULL + i));
        TEST_ASSERT(child->name == NULL);
        TEST_ASSERT(child->next == NULL);
        TEST_ASSERT(child->prev == prev);
        TEST_ASSERT(child->parent == parent);

        TEST_ASSERT_VALUES_EQUAL(child, values[i]);
    }
    jscone_node_free(parent);

    return TEST_SUCCESS;
}

END_TESTS()
