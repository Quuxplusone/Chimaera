
#ifndef H_STRING
 #define H_STRING

void memset(void *dst, int fill, int n);
void strcat(char *dst, const char *src);
char *strchr(const char *h, char n);
int strcmp(const char *a, const char *b);
void strcpy(char *dst, const char *src);
int strlen(const char *s);
int strncmp(const char *a, const char *b, int n);
char *strstr(const char *h, const char *n);
char *strtok(char *s, const char *delim);

#endif /* H_STRING */
