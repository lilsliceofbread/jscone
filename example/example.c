#define JSCONE_IMPLEMENTATION
#include "jscone.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/*
int main(void)
{
    char* json;
    FILE* file;
    u32 file_size;

    file = fopen("example.json", "rb");

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);

    json = (char*)malloc(file_size + 1);
    
    fseek(file, 0, SEEK_SET);
    fread(json, sizeof(char), (u32)file_size, file);
    json[file_size] = '\0';

    fclose(file);



    JsconeNode* result = jscone_parse(json, file_size);

    jscone_print(result);

    jscone_free(result);
    free(json);

    return 0;
}
*/

char* dump_file_contents(const char* filename, u32* file_size_out)
{
    char* text;
    FILE* file;
    u32 file_size;

    file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);

    text = (char*)malloc(file_size + 1);
    
    fseek(file, 0, SEEK_SET);
    fread(text, sizeof(char), (u32)file_size, file);
    text[file_size] = '\0';

    fclose(file);

    *file_size_out = file_size;
    return text;
}

int main(void)
{
    char filename_buf[32] = {0};
    char* json = NULL;
    u32 file_size;
    JsconeNode* result;
    
    printf("parsing incorrect json");
    for(u32 i = 1; i <= 33; i++)
    {
        snprintf(filename_buf, 32, "suite/fail%u.json", i);
        json = dump_file_contents(filename_buf, &file_size);
        printf("\nparsing %s. json string:\n%s\n", filename_buf, json);

        result = jscone_parse(json, file_size);
        jscone_print(result);
        jscone_free(result);
        if(json != NULL)
        {
            free(json);
        }
    }

    printf("\nparsing correct json");
    for(u32 i = 1; i <= 3; i++)
    {
        snprintf(filename_buf, 32, "suite/pass%u.json", i);
        json = dump_file_contents(filename_buf, &file_size);
        printf("\nparsing %s:\n", filename_buf);

        result = jscone_parse(json, file_size);
        jscone_print(result);
        jscone_free(result);
        if(json != NULL)
        {
            free(json);
        }
    }

    return 0;
}
