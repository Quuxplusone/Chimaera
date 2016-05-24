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

bool create_rabbit(Location loc);
int rabbits_at(Location loc);

void capture_a_rabbit(Location loc);
void release_a_rabbit(Location loc);

#endif // H_NPCS
