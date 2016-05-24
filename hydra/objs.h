#pragma once
#ifndef H_OBJS
#define H_OBJS

#include "map.h"
#include "vocab.h"

struct Object {
    const char *name;  // const
    const char *desc[4];  // const
    int pct;  // const
    int min_level;  // const

    int prop;
    Location loc;
    bool worn;
};

struct Object objs_[(MAX_OBJ + 1) - MIN_OBJ];

#define objs(o) objs_[(o) - MIN_OBJ]

bool toting(ObjectWord obj);
bool here(ObjectWord obj, Location loc);
bool there(ObjectWord obj, Location loc);
int holding_count(void);
bool is_immobile(ObjectWord obj);
bool is_treasure(ObjectWord obj);
bool is_wearable(ObjectWord obj);
void apport(ObjectWord t, Location loc);

void materialize_objects_if_necessary(Location loc);

void initialize_objects(void);

#endif // H_OBJS
