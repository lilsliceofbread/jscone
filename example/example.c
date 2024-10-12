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

    /* parser WIP: testing lexing */

    JsconeLexer lexer = {
        .json = json,
        .length = file_size,
        .curr = {.first = 0, .end = 0},
    };

    char buffer[128];
    while(jscone_lexer_next_token(&lexer) != JSCONE_FAILURE)
    {
        memset(buffer, 0, 128);
        memcpy(buffer, lexer.json + lexer.curr.first, lexer.curr.end - lexer.curr.first);

        printf("%s\n", buffer);
    }

    free(json);

    return 0;
}