#include "check.h"

void interpret_as_errnum_if_not_zero(char* filename, int line, char* func, int status)
{
	if(status != 0)
		print_error_message_and_exit(filename, line, func, status);
}

void print_error_message_and_exit(char* filename, int line, char* func, int errnum)
{
	char buf[256];

	fprintf(stderr, "--%s,  line %d: %s %s\n", filename, line, func, strerror_r(errnum, buf, 256));
	exit(EXIT_FAILURE);
}
