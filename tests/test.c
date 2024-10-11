#include "test.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef __linux__
    #define PASSED_COLOUR 92
    #define FAILED_COLOUR 91
    #define LOG_COLOUR 95
#elif _WIN32
    #include <windows.h>
    #define PASSED_COLOUR FOREGROUND_GREEN
    #define FAILED_COLOUR FOREGROUND_RED 
    #define LOG_COLOUR FOREGROUND_RED | FOREGROUND_BLUE
#endif

void test_print(u8 num, const char* msg)
{
    int colour;
    const char* prefix;

    switch(num)
    {
        case TEST_SUCCESS:
            colour = PASSED_COLOUR;
            prefix = "[TEST PASSED]";
            break;
        case TEST_FAILURE:
            colour = FAILED_COLOUR;
            prefix = "[TEST FAILED]";
            break;
        case TEST_LOG:
            colour = LOG_COLOUR;
            prefix = "[TESTER]";
            break;
    }

    // set colour for prefix
    #ifdef __linux__
        printf("\x1B[%dm%s:\x1B[0m ", colour, prefix);
    #elif _WIN32
        CONSOLE_SCREEN_BUFFER_INFO cb_info;
        HANDLE console_handle = (output_stream == stderr)
                              ? GetStdHandle(STD_ERROR_HANDLE) : GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(console_handle, &cb_info);
        int original_colour = cb_info.wAttributes;

        SetConsoleTextAttribute(console_handle, (WORD)colour);
            fprintf(output_stream, "%s: ", prefix);
        SetConsoleTextAttribute(console_handle, (WORD)original_colour);
    #endif

    printf("%s\n", msg);
}

void test_allocate_string(char** ptr, char* string)
{
    *ptr = malloc((strlen(string) + 1) * sizeof(char));
    strcpy(*ptr, string);
}