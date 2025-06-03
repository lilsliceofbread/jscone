#define JSCONE_IMPLEMENTATION
#include "jscone.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

char* dump_file_contents(const char* filename, unsigned int* file_size_out)
{
    char* text;
    FILE* file;
    unsigned int file_size;

    file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);

    text = (char*)malloc(file_size + 1);
    
    fseek(file, 0, SEEK_SET);
    fread(text, sizeof(char), (unsigned int)file_size, file);
    text[file_size] = '\0';

    fclose(file);

    *file_size_out = file_size;
    return text;
}

int main(void)
{
    char* json = NULL;
    unsigned int file_size;
    JsconeNode* result;
    
    /* from https://microsoftedge.github.io/Demos/json-dummy-data/ */
    json = dump_file_contents("5MB-min.json", &file_size);
    result = jscone_parse(json, file_size);

    JsconeNode* last_node = result->child;
    int i = 1;
    while(last_node->next != NULL)
    {
        last_node = last_node->next;
        i++;
    }
    printf("Person number %d:\n", i);
    jscone_print(last_node->child);
    jscone_free(result);

    free(json);

    return 0;
}
