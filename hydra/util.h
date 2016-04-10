#pragma once
#ifndef H_UTIL
#define H_UTIL

#define DIM(a) (sizeof a / sizeof *a)

extern int global_seed;
extern int turns;

int ran(int modulus);

#endif // H_UTIL
