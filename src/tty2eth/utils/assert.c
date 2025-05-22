#include "utils/assert.h"
#include <cmsis_compiler.h>

bool assert_func(char* file, int line, bool cond, char* msg, ...)
{
    if(cond)
        return false;

    va_list argptr;
    va_start(argptr, msg);
    printf("[%s:%d]: ", file, line);
    vfprintf(stdout, msg, argptr);
    printf("\n");
    va_end(argptr);
    return true;
}

void panic_func(char* file, int line, char* msg, ...)
{
    va_list argptr;
    va_start(argptr, msg);
    printf("[%s:%d]: ", file, line);
    vfprintf(stdout, msg, argptr);
    printf("\n");
    va_end(argptr);
    __BKPT(0xBE);
}