
#include "npcs.h"
#include "util.h"

#define ONE_OF(a) a[ran(DIM(a))]

int number_of_rabbits = 0;
struct Rabbit rabbits[10];

struct Rabbit create_rabbit(Location loc)
{
    const char *colors[] = {
        "white", "brown", "sandy-colored", "spotted", "black", "black and white", "yellow", "pink"
    };
    struct Rabbit result;
    result.oldloc = NOWHERE;
    result.loc = loc;
    result.color = ONE_OF(colors);
    return result;
}

int rabbits_at(Location loc)
{
    int result = 0;
    for (int i=0; i < number_of_rabbits; ++i) {
        result += (rabbits[i].loc == loc);
    }
    return result;
}
