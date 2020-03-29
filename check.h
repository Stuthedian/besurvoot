#ifndef CHECK_H
#define CHECK_H

#define _GNU_SOURCE 1
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define CHECK(comparison_operator, fail_condition) comparison_operator fail_condition ? print_error_message_and_exit(__FILE__,  __LINE__, "", errno) : 0;
#define CHECK_IS_NEGATIVE_ONE CHECK(==, -1);
#define CHECK_IS_NULL CHECK(==, NULL);
#define INTERPRET_AS_ERRNUM_IF_NOT_ZERO(func) interpret_as_errnum_if_not_zero(__FILE__,  __LINE__, #func, func);

void interpret_as_errnum_if_not_zero(char* filename, int line, char* func, int status);
void print_error_message_and_exit(char* filename, int line, char* func, int errnum);

#endif //CHECK_H
