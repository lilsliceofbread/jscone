#include "test.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
    #define PASSED_COLOUR FOREGROUND_GREEN
    #define FAILED_COLOUR FOREGROUND_RED 
    #define LOG_COLOUR FOREGROUND_RED | FOREGROUND_BLUE
#else
#ifdef __linux__
    #include <unistd.h>
    #include <fcntl.h>
    #define PASSED_COLOUR 92
    #define FAILED_COLOUR 91
    #define LOG_COLOUR 95
#endif
#endif

void test_print(u8 num, const char* msg)
{
    int colour = 0;
    const char* prefix = "";

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
        case TEST_ASSERT_FAIL:
            colour = FAILED_COLOUR;
            prefix = "[ASSERT FAILED]";
            break;
    }

    /* set colour for prefix */
    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO cb_info;
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(console_handle, &cb_info);
        int original_colour = cb_info.wAttributes;

        SetConsoleTextAttribute(console_handle, (WORD)colour);
            printf("%s: ", prefix);
        SetConsoleTextAttribute(console_handle, (WORD)original_colour);
    #else
        printf("\x1B[0m\x1B[%dm%s:\x1B[0m ", colour, prefix);
    #endif

    printf("%s\n", msg);
}

void test_print_var(u8 num, const char* msg, ...)
{
    va_list args;

    char buffer[1024];
    va_start(args, msg);
        vsnprintf(buffer, 1024, msg, args);
    va_end(args);

    test_print(num, buffer);
}

void test_allocate_string(char** ptr, char* string)
{
    unsigned int length = (unsigned int)strlen(string);
    *ptr = malloc((length + 1) * sizeof(char));
    strncpy(*ptr, string, length + 1);
}

/* NOTE: stdout = 1, stderr = 2 */

void test_suppress_output(int* stdout_fd, int* stderr_fd)
{
    fflush(stdout);
    fflush(stderr);

    *stdout_fd = dup(1); 
    *stderr_fd = dup(2);

    int null_fd = open("/dev/null", O_WRONLY); 
    dup2(null_fd, 1);
    dup2(null_fd, 2);

    close(null_fd);
}

void test_resume_output(int stdout_fd, int stderr_fd)
{
    fflush(stdout);
    fflush(stderr);

    dup2(stdout_fd, 1);
    dup2(stderr_fd, 2);

    close(stdout_fd);
    close(stderr_fd);
}

int main(void)
{
    /* tests run automatically before and after main with insane preprocessor manipulation */
    return 0;
}
