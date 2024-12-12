#define JSCONE_IMPLEMENTATION
#include "jscone.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// load example.json (from https://json.org/example.html)
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
