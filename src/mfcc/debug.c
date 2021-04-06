#include <stdarg.h>
#include <stdio.h>

void debug_printf(FILE *fp, char* file, int line, char *fmt, ...)
{
	fprintf(fp, "[%s]:%d\t", file, line);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);
}
