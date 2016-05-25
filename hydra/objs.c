
#include "map.h"
#include "npcs.h"
#include "objs.h"

bool materialized(ObjectWord obj)
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

bool is_immobile(ObjectWord obj)
{
    switch (obj) {
        case DOG:
        case ANVIL:
            return true;
        default:
            return false;
    }
}

bool is_treasure(ObjectWord obj)
{
    switch (obj) {
        case GOLD: case DIAMONDS: case SILVER: case JEWELS:
        case COINS: case CHEST: case EGGS: case TRIDENT:
        case VASE: case EMERALD: case PYRAMID: case PEARL:
        case RUG: case SPICES: case CHAIN:
        case SWORD: case ERING:
            return true;
        default:
            return false;
    }
}

bool is_wearable(ObjectWord obj)
{
    switch (obj) {
        case JEWELS: case ERING:
            return true;
        default:
            return false;
    }
}

bool gives_light(ObjectWord obj)
{
    switch (obj) {
        case LAMP:
            return (objs(LAMP).prop == 1);
        case MOSS:
            return true;
        default:
            return false;
    }
}

void apport(ObjectWord t, Location loc)
{
    if (!materialized(t)) {
        objs(t).prop = 0;  // materialize it
    }
    objs(t).loc = loc;
    objs(t).worn = false;
}

void dematerialize(ObjectWord t)
{
    objs(t).prop = -1;
    objs(t).loc = NOWHERE;
}

void materialize_objects_if_necessary(Location loc)
{
    for (ObjectWord t = MIN_OBJ; t <= MAX_OBJ; ++t) {
        if (z_of(loc) < objs(t).min_level) continue;
        if (t == BIRD && !is_forested(loc)) continue;
        if (t == LAMP && !has_light(loc)) continue;  // that wouldn't be fair
        if (object_can_be_found_at(t, loc)) {
            // Only one object should be found in any given room.
            // Objects should not suddenly appear in a room that's already been visited!
            if (!materialized(t)) {
                apport(t, loc);
                if (t == ANVIL) {
                    apport(SWORD, loc);
                }
            }
            break;
        }
    }
    if (is_overworld(loc) && lrng_one_in(10, loc, "rabbit")) {
        create_rabbit(loc);
    }
}

void initialize_objects(void)
{
#define new_obj(Lev, Pct, t, Name) objs(t).name = Name; objs(t).min_level = Lev; objs(t).pct = Pct

    for (ObjectWord obj = MIN_OBJ; obj <= MAX_OBJ; ++obj) {
        objs(obj).prop = -1;
        objs(obj).loc = NOWHERE;
        objs(obj).worn = false;
    }
    objs(RABBIT).prop = 0;

    new_obj(5, 10, DOG, "(Three-headed dog)");
    new_obj(0, 10, LAMP, "Brass lantern");
    new_obj(0, 5, CAGE, "Wicker cage");
    new_obj(0, 0, RABBIT, "Fuzzy rabbit in cage");
    new_obj(0, 10, BIRD, "Little bird in cage");
    new_obj(1, 5, ROD, "Black rod");
    new_obj(2, 5, CLUB, "Club with white stripe");
    new_obj(6, 10, ANVIL, "(Anvil)");
    new_obj(0, 0, MOSS, "Handful of glowing moss");

    new_obj(2, 5, GOLD, "Large gold nugget");
    new_obj(2, 5, DIAMONDS, "Several diamonds");
    new_obj(2, 5, SILVER, "Bars of silver");
    new_obj(2, 5, JEWELS, "Precious jewelry");
    new_obj(2, 5, COINS, "Rare coins");
    new_obj(3, 5, CHEST, "Treasure chest");
    new_obj(3, 5, EGGS, "Golden eggs");
    new_obj(3, 5, TRIDENT, "Jeweled trident");
    new_obj(3, 5, VASE, "Ming vase");
    new_obj(3, 5, EMERALD, "Egg-sized emerald");
    new_obj(4, 5, PYRAMID, "Platinum pyramid");
    new_obj(4, 5, PEARL, "Glistening pearl");
    new_obj(4, 5, RUG, "Persian rug");
    new_obj(4, 5, SPICES, "Rare spices");
    new_obj(4, 5, CHAIN, "Golden chain");
    new_obj(6, 0, SWORD, "Magic sword");
    new_obj(10, 20, ERING, "Engagement ring");

    // Rabbit descriptions are handled elsewhere.
    objs(DOG).desc[0] = "There is a ferocious three-headed dog slavering and snarling in the center of the room!";
    objs(DOG).desc[1] = "You are being followed by a tame three-headed dog.";
    objs(LAMP).desc[0] = "There is a shiny brass lamp nearby.";
    objs(LAMP).desc[1] = "There is a lamp shining nearby.";
    objs(CAGE).desc[0] = "There is a small wicker cage discarded nearby.";
    objs(RABBIT).desc[0] = "(Rabbit)";
    objs(RABBIT).desc[1] = "There is a fuzzy little rabbit in the cage.";
    objs(BIRD).desc[0] = "A cheerful little bird is sitting here singing.";
    objs(BIRD).desc[1] = "There is a little bird in the cage.";
    objs(ROD).desc[0] = "A three-foot black rod with a rusty star on an end lies nearby.";
    objs(CLUB).desc[0] = "A two-foot club with a white stripe on the end lies nearby.";
    objs(ANVIL).desc[0] = "In the center of the room is a great stone, to which is affixed an enormous rusty iron anvil.\n"
                          "In golden letters are written the words: \"Whoso Pulleth Out This Sword of This\n"
                          "Stone and Anvil, is Rightwise King-Born of All Adventuredom.\"";

    objs(GOLD).desc[0] = "There is a large sparkling nugget of gold here!";
    objs(DIAMONDS).desc[0] = "There are diamonds here!";
    objs(SILVER).desc[0] = "There are bars of silver here!";
    objs(JEWELS).desc[0] = "There is precious jewelry here!";
    objs(COINS).desc[0] = "There are many coins here!";
    objs(CHEST).desc[0] = "A pirate's treasure chest is here!";
    objs(EGGS).desc[0] = "There is a large nest here, full of golden eggs!";
    objs(TRIDENT).desc[0] = "There is a jewel-encrusted trident here!";
    objs(VASE).desc[0] = "There is a delicate, precious Ming vase here!";
    objs(EMERALD).desc[0] = "There is an emerald here the size of a plover's egg!";
    objs(PYRAMID).desc[0] = "There is a platinum pyramid here, 8 inches on a side!";
    objs(PEARL).desc[0] = "Off to one side lies a glistening pearl!";
    objs(RUG).desc[0] = "There is a Persian rug spread out on the floor!";
    objs(SPICES).desc[0] = "There are rare spices here!";
    objs(CHAIN).desc[0] = "There is a golden chain lying in a heap on the floor!";
    objs(SWORD).desc[0] = "A gleaming elven sword is stuck into the anvil!";
    objs(SWORD).desc[1] = "A gleaming elven sword is stuck into the anvil!";
    objs(SWORD).desc[2] = "There is a gleaming elven sword here!";
    objs(ERING).desc[0] = "There is a diamond-studded engagement ring here!";

#undef new_obj
}
