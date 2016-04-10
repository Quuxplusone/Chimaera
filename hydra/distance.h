#pragma once
#ifndef H_DISTANCE
#define H_DISTANCE

#include "map.h"

int get_crow_distance_between(Location from, Location to);
int get_shortest_distance_between(Location from, Location to, int cutoff);
MotionWord get_direction_from(Location from, Location to);
bool is_adjacent_to(Location from, Location to);

#endif // H_DISTANCE
