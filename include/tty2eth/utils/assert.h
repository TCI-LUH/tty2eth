#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#define assert(cond, msg, ...) assert_func(__FILE__, __LINE__, cond, msg, ##__VA_ARGS__)
bool assert_func(char* file, int line, bool cond, char* msg, ...);

#define panic(msg, ...) panic_func(__FILE__, __LINE__, msg, ##__VA_ARGS__)
void panic_func(char* file, int line, char* msg, ...);