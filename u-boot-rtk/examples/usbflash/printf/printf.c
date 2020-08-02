#define TINYPRINTF_DEFINE_TFP_PRINTF 0
#define TINYPRINTF_OVERRIDE_LIBC 0

#include "TinyStdio/tinystdio.c"
#include "printf.h"

extern void serial_write(unsigned char  *p_param);

static void ts_putc(void *p, char c) {
    serial_write(&c);
    if (p) {
        ++(*((int *)p));
    }
}

int printf(const char *fmt, ...)
{
    int count = 0;
    va_list va;
    va_start(va, fmt);
    tfp_format(&count, ts_putc, fmt, va);
    va_end(va);
    return count;
}

int vprintf(const char *fmt, va_list args) 
{
    int count = 0;
    tfp_format(&count, ts_putc, fmt, args);
    return count;
}
/*
void panic(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tfp_format(NULL, ts_putc, fmt, args);
	va_end(args);
	while(1);
}
*/