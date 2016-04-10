
#include "util.h"

int global_seed = 1;
int turns;

int ran(int modulus)
{
    // Get a random number based on the state of the game so far.
    static unsigned int seed = 42;
    seed = (seed * 37) + global_seed;
    seed ^= turns;
    return (seed >> 4) % modulus;
}
