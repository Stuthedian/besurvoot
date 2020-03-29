#include "check.h"

void interpret_as_errnum_if_not_zero(char* filename, int line, char* func, int status)
{
	if(status != 0)
		print_error_message_and_exit(filename, line, func, status);
}

void print_error_message_and_exit(char* filename, int line, char* func, int errnum)
{
	char buf[256];
	int status = strerror_r(errnum, buf, 256);
	if(status != 0)
	{
		if(status > 0)
			fprintf(stderr, "strerror_r failed. Error number is: %d\n", status);
		else if(status == -1)
			fprintf(stderr, "strerror_r failed. Error number is: %d\n", errno);
		else
			fputs("strerror_r failed. Unknown failure", stderr);
        fprintf(stderr, "--%s,  line %d: %s \n", filename, line, func);
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "--%s,  line %d: %s %s\n", filename, line, func, buf);
	exit(EXIT_FAILURE);
}

void partial_if(char* filename, int line)
{
	fprintf(stderr, "--%s,  line %d: Hit else clause, exiting...\n", filename, line);
	exit(EXIT_FAILURE);
}
