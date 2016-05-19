#pragma once
#ifndef H_NPCS
#define H_NPCS

#include "map.h"

struct Rabbit {
    const char *color;
    Location loc;
    Location oldloc;
};

extern int number_of_rabbits;
extern struct Rabbit rabbits[10];

struct Rabbit create_rabbit(Location loc);
int rabbits_at(Location loc);

#endif // H_NPCS
