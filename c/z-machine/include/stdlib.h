
#ifndef H_STDLIB
 #define H_STDLIB

void exit(int status) = "\t@quit;\n";

inline int abs(int x) { return (x < 0) ? -x : x; }

int atoi(const char *str);

#endif /* H_STDLIB */
