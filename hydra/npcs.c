
#include <assert.h>
#include "npcs.h"
#include "util.h"

#define ONE_OF(a) a[ran(DIM(a))]

int number_of_rabbits = 0;
struct Rabbit rabbits[10];

bool create_rabbit(Location loc)
{
    static const char *colors[] = {
        "white", "brown", "sandy-colored", "spotted", "black", "black and white", "yellow", "pink"
    };
    if (number_of_rabbits < 10) {
        struct Rabbit *rabbit = &rabbits[number_of_rabbits++];
        rabbit->oldloc = NOWHERE;
        rabbit->loc = loc;
        rabbit->color = ONE_OF(colors);
        return true;
    } else {
        return false;
    }
}

int rabbits_at(Location loc)
{
    int result = 0;
    for (int i=0; i < number_of_rabbits; ++i) {
        result += (rabbits[i].loc == loc);
    }
    return result;
}

void capture_a_rabbit(Location loc)
{
    for (int i=0; i < number_of_rabbits; ++i) {
        if (rabbits[i].loc == loc) {
            rabbits[i].loc = INHAND;
            return;
        }
    }
    assert(false);
}

void release_a_rabbit(Location loc)
{
    for (int i=0; i < number_of_rabbits; ++i) {
        if (rabbits[i].loc == INHAND) {
            rabbits[i].loc = loc;
            return;
        }
    }
    assert(false);
}
