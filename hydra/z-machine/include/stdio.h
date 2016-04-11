
#ifndef H_STDIO
 #define H_STDIO

#ifndef NULL
 #define NULL 0
#endif /* NULL */

typedef struct _Neverdefined FILE;
extern FILE *stdin;
extern FILE *stdout;

void printf(const char *fmt, ...);
void puts(const char *);
void putc(int, FILE *);
char *fgets(char *buffer, int len, FILE *in);

void sprintf(char *buf, const char *fmt, ...);

void fflush(FILE *) = "";

#endif /* H_STDIO */
