
#include <assert.h>

#include "map.h"
#include "objs.h"
#include "util.h"
#include "vocab.h"

Location xyz(int x, int y, int z)
{
    assert(0 <= x && x <= MAP_MAX);
    assert(0 <= y && y <= MAP_MAX);
    assert(0 <= z && z <= MAP_MAX);
    return (x << 10) | (y << 5) | z;
}

int x_of(Location loc) { return (loc / (MAP_MAX * MAP_MAX)) % MAP_MAX; }
int y_of(Location loc) { return (loc / MAP_MAX) % MAP_MAX; }
int z_of(Location loc) { return loc % MAP_MAX; }

static unsigned int hash(unsigned int seed, const char *s)
{
    assert(s);
    unsigned char *p;
    for (p = (unsigned char*)s; *p != '\0'; p++) {
        seed *= 37;
        seed ^= *p;
    }
    return seed;
}

int lrng(Location loc, const char *salt)
{
    // Get a random number based on the hash of a single location.
    char xyz[] = { x_of(loc) + 1, y_of(loc) + 1, z_of(loc) + 1, '\0' };
    return hash(hash(global_seed, xyz), salt) >> 4;
}

int object_can_be_found_at(ObjectWord obj, Location loc)
{
    char xyz[] = { x_of(loc) + 1, y_of(loc) + 1, z_of(loc) + 1, '\0' };
    return (int)(hash(hash(global_seed, xyz), objs(obj).name) >> 4) % 100 < objs(obj).pct;
}

int llrng(Location loc, Location loc2, const char *salt)
{
    // Get a random number based on the hash of an UNORDERED pair of locations.
    // This function must remain symmetrical: llrng(a,b,s) == llrng(b,a,s).
    if (loc2 < loc) {
        Location temp = loc;
        loc = loc2;
        loc2 = temp;
    }
    char xyz[] = { x_of(loc) + 1, y_of(loc) + 1, z_of(loc) + 1, x_of(loc2) + 1, y_of(loc2) + 1, z_of(loc2) + 1, '\0' };
    return hash(hash(global_seed, xyz), salt) >> 4;
}

int lrng_one_in(int chance, Location loc, const char *salt)
{
    return lrng(loc, salt) % chance == 0;
}

int lrng_two_in(int chance, Location loc, const char *salt)
{
    return lrng(loc, salt) % chance <= 1;
}

int llrng_one_in(int chance, Location loc, Location loc2, const char *salt)
{
    return llrng(loc, loc2, salt) % chance == 0;
}

static Location get_raw_neighbor(Location loc, MotionWord mot)
{
    const int x = x_of(loc);
    const int y = y_of(loc);
    const int z = z_of(loc);
    switch (mot) {
        case N: return (x+1 > MAP_MAX) ? NOWHERE : xyz(x+1, y, z);
        case S: return (x-1 < 0)       ? NOWHERE : xyz(x-1, y, z);
        case E: return (y+1 > MAP_MAX) ? NOWHERE : xyz(x, y+1, z);
        case W: return (y-1 < 0)       ? NOWHERE : xyz(x, y-1, z);
        case D: return (z+1 > MAP_MAX) ? NOWHERE : xyz(x, y, z+1);
        case U: return (z-1 < 0)       ? NOWHERE : xyz(x, y, z-1);
        case NW: return (x+1 > MAP_MAX || y-1 < 0)       ? NOWHERE : xyz(x+1, y-1, z);
        case NE: return (x+1 > MAP_MAX || y+1 > MAP_MAX) ? NOWHERE : xyz(x+1, y+1, z);
        case SW: return (x-1 < 0       || y-1 < 0)       ? NOWHERE : xyz(x-1, y-1, z);
        case SE: return (x-1 < 0       || y+1 > MAP_MAX) ? NOWHERE : xyz(x-1, y+1, z);
        default: return NOWHERE;
    }
}

static struct Exits get_raw_exits(Location loc)
{
    struct Exits result;
    for (MotionWord mot = MIN_MOTION; mot <= MAX_MOTION; ++mot) {
        result.go[mot] = get_raw_neighbor(loc, mot);
    }
    return result;
}

static struct Exits get_constrained_cave_exits(Location loc)
{
    struct Exits result = get_raw_exits(loc);
    for (MotionWord mot = MIN_MOTION; mot <= MAX_MOTION; ++mot) {
        if (result.go[mot] == NOWHERE) {
            // ok, it's on the edge of the map already
        } else if (llrng_one_in(4, loc, result.go[mot], "allow_exit")) {
            // ok, allow this exit 25% of the time
        } else {
            result.go[mot] = NOWHERE;  // block 75% of the exits
        }
    }
    return result;
}

static struct Exits get_overworld_exits(Location loc)
{
    assert(is_overworld(loc));

    const bool forested = is_forested(loc);
    struct Exits result = get_raw_exits(loc);
    for (MotionWord mot = MIN_MOTION; mot <= MAX_MOTION; ++mot) {
        if (result.go[mot] == NOWHERE) {
            // ok, it's on the edge of the map already
        } else {
            if (forested && is_forested(result.go[mot])) {
                if (llrng_one_in(2, loc, result.go[mot], "block_exit")) {
                    result.go[mot] = NOWHERE;
                }
            }
        }
    }
    result.go[D] = get_constrained_cave_exits(loc).go[D];  // match up the tops and bottoms of overworld stairs
    return result;
}

static struct Exits get_wiggled_cave_exits(Location loc)
{
    assert(!is_overworld(loc));
    struct Exits result = get_constrained_cave_exits(loc);

#define WIGGLE(n, nw, ne) do { \
        if (result.go[n] != NOWHERE) { \
            if (result.go[nw] == NOWHERE && lrng_one_in(6, loc, "wiggle" #n #nw)) { \
                result.go[nw] = result.go[n]; \
                result.go[n] = NOWHERE; \
            } else if (result.go[ne] == NOWHERE && lrng_one_in(5, loc, "wiggle" #n #ne)) { \
                result.go[ne] = result.go[n]; \
                result.go[n] = NOWHERE; \
            } \
        } \
    } while (0)

    WIGGLE(N, NW, NE);
    WIGGLE(W, NW, SW);
    WIGGLE(S, SE, SW);
    WIGGLE(E, NE, SE);
    WIGGLE(NW, N, W);
    WIGGLE(SW, S, W);
    WIGGLE(NE, N, E);
    WIGGLE(SE, S, E);

    return result;
}

struct Exits get_exits(Location loc)
{
    if (is_overworld(loc)) {
        return get_overworld_exits(loc);
    } else {
        return get_wiggled_cave_exits(loc);
    }
}

bool has_light(Location loc)
{
    if (loc == NOWHERE) {
        return false;
    } else if (is_overworld(loc)) {
        return true;
    } else if (has_glowing_moss(loc)) {
        return true;
    } else {
        struct Exits exits = get_exits(loc);
        return has_light(exits.go[U]);
    }
}

bool is_overworld(Location loc)
{
    return z_of(loc) == 0;
}

bool is_forested(Location loc)
{
    if (!is_overworld(loc)) {
        return false;
    }

    const int x = x_of(loc);
    const int y = y_of(loc);
    const int z = z_of(loc);

    int wood = 0;
    for (int i = 2; i <= 8; i *= 2) {
        const Location temp = xyz(x/i, y/i, z/i);
        wood += lrng_one_in(2, temp, "forest");
    }
    return (wood >= 2);
}

bool has_glowing_moss(Location loc)
{
    if (is_overworld(loc)) {
        return false;
    }

    const int x = x_of(loc);
    const int y = y_of(loc);
    const int z = z_of(loc);

    int moss = 0;
    for (int i = 2; i <= 8; i *= 2) {
        const Location temp = xyz(x/i, y/i, z/i);
        moss += lrng_one_in(2, temp, "forest");
    }
    return (moss >= 2);
}

bool has_up_stairs(Location loc)
{
    if (is_overworld(loc)) {
        return false;
    }
    const struct Exits exits = get_exits(loc);
    if (exits.go[U] == NOWHERE) {
        return false;
    } else if (z_of(loc) == 1) {
        return true;
    } else {
        return llrng_one_in(3, loc, exits.go[U], "stairs");
    }
}

bool has_down_stairs(Location loc)
{
    const struct Exits exits = get_exits(loc);
    if (exits.go[D] == NOWHERE) {
        return false;
    } else if (z_of(loc) == 0) {
        return true;
    } else {
        return llrng_one_in(3, loc, exits.go[D], "stairs");
    }
}
