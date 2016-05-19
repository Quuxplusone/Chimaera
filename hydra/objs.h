#pragma once
#ifndef H_OBJS
#define H_OBJS

#include "map.h"
#include "vocab.h"

struct Object {
    const char *name;
    const char *desc[4];
    int prop;
    Location loc;
};

struct Object objs_[(MAX_OBJ + 1) - MIN_OBJ];

#define objs(o) objs_[(o) - MIN_OBJ]

bool toting(ObjectWord obj);
bool here(ObjectWord obj, Location loc);
bool there(ObjectWord obj, Location loc);
int holding_count(void);

void materialize_objects_if_necessary(Location loc);

void initialize_objects(void);

#endif // H_OBJS
