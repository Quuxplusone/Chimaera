
#include "map.h"
#include "objs.h"

static bool materialized(ObjectWord obj)
{
    return objs(obj).prop != -1;
}

bool toting(ObjectWord obj)
{
    return materialized(obj) && objs(obj).loc == INHAND;
}

bool here(ObjectWord obj, Location loc)
{
    return toting(obj) || there(obj, loc);
}

bool there(ObjectWord obj, Location loc)
{
    return materialized(obj) && objs(obj).loc == loc;
}

int holding_count(void)
{
    int result = 0;
    for (ObjectWord obj = MIN_OBJ; obj <= MAX_OBJ; ++obj) {
        result += toting(obj);
    }
    return result;
}

void materialize_objects_if_necessary(Location loc)
{
    for (ObjectWord obj = MIN_OBJ; obj <= MAX_OBJ; ++obj) {
        if (materialized(obj)) continue;
        if (obj == BIRD && !is_forested(loc)) continue;
        if (obj == ROD && z_of(loc) <= 1) continue;
        if (obj == CLUB && z_of(loc) <= 2) continue;
        if (object_can_be_found_at(loc, obj, 10)) {
            objs(obj).prop = 0;
            objs(obj).loc = loc;
        }
    }
}

void initialize_objects(void)
{
    for (ObjectWord obj = MIN_OBJ; obj <= MAX_OBJ; ++obj) {
        objs(obj).prop = -1;
        objs(obj).loc = NOWHERE;
    }
    objs(RABBIT).prop = 0;
    objs(RABBIT).name = "Fuzzy rabbit in cage";
    objs(BIRD).name = "Little bird in cage";
    objs(BIRD).desc[0] = "A cheerful little bird is sitting here singing.";
    objs(BIRD).desc[1] = "There is a little bird in the cage.";
    objs(LAMP).name = "Brass lantern";
    objs(LAMP).desc[0] = "There is a shiny brass lamp nearby.";
    objs(LAMP).desc[1] = "There is a lamp shining nearby.";
    objs(ROD).name = "Black rod";
    objs(ROD).desc[0] = "A three-foot black rod with a rusty star on an end lies nearby.";
    objs(CLUB).name = "Club with white stripe";
    objs(CLUB).desc[0] = "A two-foot club with a white stripe on the end lies nearby.";
}
