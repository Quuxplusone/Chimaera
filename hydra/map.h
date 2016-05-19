#pragma once
#ifndef H_MAP
#define H_MAP

#include <stdbool.h>
#include "vocab.h"

// positive x is NORTH
// positive y is EAST
// positive z is DOWN
// valid x,y,z values range within [0, MAP_MAX)
//
typedef int Location;

#define MAP_MAX 32
#define NOWHERE -1
#define INHAND -2

struct Exits {
    Location go[MAX_MOTION + 1];
};

Location xyz(int x, int y, int z);

int x_of(Location loc);
int y_of(Location loc);
int z_of(Location loc);

int lrng(Location loc, const char *salt);
int lrng_one_in(int chance, Location loc, const char *salt);
int lrng_two_in(int chance, Location loc, const char *salt);

int object_can_be_found_at(ObjectWord obj, Location loc, int pct);

int llrng(Location loc, Location loc2, const char *salt);
int llrng_one_in(int chance, Location loc, Location loc2, const char *salt);

struct Exits get_exits(Location loc);

bool has_light(Location loc);
bool is_overworld(Location loc);
bool is_forested(Location loc);
bool has_up_stairs(Location loc);
bool has_down_stairs(Location loc);

#endif // H_MAP
