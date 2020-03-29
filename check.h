#ifndef CHECK_H
#define CHECK_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#define CHECK(comparison_operator, fail_condition) comparison_operator fail_condition ? print_error_message_and_exit(__FILE__,  __LINE__, "", errno) : 0;
#define CHECK_IS_NEGATIVE_ONE CHECK(==, -1);
#define CHECK_IS_NULL CHECK(==, NULL);
#define INTERPRET_AS_ERRNUM_IF_NOT_ZERO(func) interpret_as_errnum_if_not_zero(__FILE__,  __LINE__, #func, func);
#define PARTIAL_IF partial_if(__FILE__,  __LINE__);

int strerror_r(int errnum, char *buf, size_t buflen);

void interpret_as_errnum_if_not_zero(char* filename, int line, char* func, int status);
void print_error_message_and_exit(char* filename, int line, char* func, int errnum);
void partial_if(char* filename, int line);

#endif //CHECK_H
