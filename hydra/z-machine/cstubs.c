/*
! This file contains only those (pieces of) standard C library functions
! which are required in order to build "advent.c" for the Z-machine, and
! are not more easily written in Inform. These functions are:
!
!     int atoi(s)
!     void memset(dest, fill, n)
!     void puts(buf)
!     void printf(fmt, ...)
!     void sprintf(buf, fmt, ...)
!     int sqrt(x)
!     int strlen(s)
!     void strcat(dest, src)
!     char *strchr(h, n)
!     int strcmp(a, b)
!     void strcpy(dest, src)
!     int strncmp(a, b, n)
!     char *strstr(h, n)
!     char *strtok(s, delim)
!
! The following functions are defined in Inform, in the file stubs.inf:
!
!     void fgets(buf, size, stdin)
!     void putc(ch, stdout)
!     void exit(0)
*/

#include <math.h>    /* sqrt */
#include <stdio.h>   /* puts, printf */
#include <string.h>  /* strcat, strcmp, strcpy, strlen, strncmp */
#include <stdarg.h>  /* va_list */

/* These should have unique values, but that's about it. */
FILE *stdin = (FILE *)&stdin;
FILE *stdout = (FILE *)&stdout;

void puts(const char *s)
{
    while (*s) {
        putc(*s, stdout);
        ++s;
    }
    putc('\n', stdout);
}

void memset(void *dstv, int fill, int n)
{
    char *dst = dstv;
    for (int i = 0; i < n; ++i) {
        *dst++ = fill;
    }
}

int sqrt(int x)
{
    if (x <= 0) return 0;
    for (int i = 1; i < 10000; ++i) {
        if (x < i*i) return i-1;
    }
    return 10000;  // doesn't need to be very accurate
}

void strcat(char *dst, const char *src)
{
    dst += strlen(dst);
    strcpy(dst, src);
}

char *strchr(const char *h, char n)
{
    while (*h != '\0') {
        if (*h == n) return (char *)h;
        ++h;
    }
    if (n == '\0') return (char *)h;
    return NULL;
}

int strcmp(const char *a, const char *b)
{
    unsigned char ca, cb;
    while (*a == *b) {
        if (*a == '\0')
            return 0;
        ++a, ++b;
    }
    ca = *a;
    cb = *b;
    return (ca < cb) ? -1 : (ca > cb);
}

void strcpy(char *dst, const char *src)
{
    while (*src) {
        *dst++ = *src++;
    }
    *dst = '\0';
}

int strlen(const char *s)
{
    int n = 0;
    while (s[n]) ++n;
    return n;
}

int strncmp(const char *a, const char *b, int n)
{
    for (int i=0; i < n; ++i) {
        unsigned char ca = *a++, cb = *b++;
        if (ca != cb) return (ca < cb) ? -1 : (ca > cb);
        if (ca == '\0') return 0;
    }
    return 0;
}

char *strstr(const char *h, const char *n)
{
    int hlen = strlen(h);
    int nlen = strlen(n);
    for (int i=0; i + nlen <= hlen; ++i) {
        if (strncmp(h+i, n, nlen) == 0) return (char *)h+i;
    }
    return NULL;
}

char *strtok(char *s, const char *delim)
{
    static char *saved = NULL;
    if (s == NULL) {
        if (saved == NULL) {
            return NULL;
        }
        s = saved;
    }
    while (*s && strchr(delim, *s)) ++s;
    if (*s == '\0') {
        saved = NULL;
        return NULL;
    }
    char *result = s;
    while (!strchr(delim, *s)) ++s;
    if (*s == '\0') {
        saved = NULL;
    } else {
        *s = '\0';
        saved = s + 1;
    }
    return result;
}

void _print_int(int i) = "\t@print_num r0;\n";

void printf(const char *format, ...)
{
    char c;
    va_list ap;
    va_start(ap, format);
    while (c = *format++) {
        if (c == '%') {
            switch (c = *format++) {
                case 'd': {
                    int i = va_arg(ap, int);
                    _print_int(i);
                    break;
                }
                case 's': {
                    char *s = va_arg(ap, char*);
                    while (c = *s++)
                        putc(c, stdout);
                    break;
                }
                default: {
                    putc(c, stdout);
                    break;
                }
            }
        } else {
            putc(c, stdout);
        }
    }
    va_end(ap);
}

void sprintf(char *buf, const char *format, ...)
{
    char c;
    va_list ap;
    va_start(ap, format);
    while (c = *format++) {
        if (c == '%') {
            switch (c = *format++) {
                case 's': {
                    char *s = va_arg(ap, char*);
                    while (c = *s++)
                        *buf++ = c;
                    break;
                }
                default: {
                    *buf++ = c;
                    break;
                }
            }
        } else {
            *buf++ = c;
        }
    }
    va_end(ap);
}

/* We don't handle signed inputs, but that's fine for our purpose. */
int atoi(const char *str)
{
    int result = 0;
    while ('0' <= *str && *str <= '9') {
        result *= 10;
        result += (*str - '0');
        ++str;
    }
    return result;
}
