
#include <assert.h>

#include "map.h"
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
    unsigned char *p;
    for (p = (unsigned char*)s; *p != '\0'; p++) {
        seed *= 37;
        seed += *p;
    }
    return seed;
}

int lrng(Location loc, const char *salt)
{
    char xyz[] = { x_of(loc) + 1, y_of(loc) + 1, z_of(loc) + 1, '\0' };
    return hash(hash(0, xyz), salt) & 0x7fff;
}

int lrng_one_in(int chance, Location loc, const char *salt)
{
    return lrng(loc, salt) % chance == 0;
}

static unsigned int get_direction_mask(int x, int y, int z)
{
    const Location loc = xyz(x, y, z);
    return lrng(loc, "get_direction_mask");
}

static struct Exits get_raw_directions(Location loc)
{
    const int x = x_of(loc);
    const int y = y_of(loc);
    const int z = z_of(loc);
    const int mymask = get_direction_mask(x, y, z);
    struct Exits result;
    for (int m=MIN_MOTION; m <= MAX_MOTION; ++m) {
        result.go[m] = NOWHERE;
    }
    if (x+1 < MAP_MAX && (mymask & 0x01) && (get_direction_mask(x+1, y, z) & 0x02)) { result.go[N] = xyz(x+1, y, z); }
    if (x-1 >= 0      && (mymask & 0x02) && (get_direction_mask(x-1, y, z) & 0x01)) { result.go[S] = xyz(x-1, y, z); }
    if (y+1 < MAP_MAX && (mymask & 0x04) && (get_direction_mask(x, y+1, z) & 0x08)) { result.go[E] = xyz(x, y+1, z); }
    if (y-1 >= 0      && (mymask & 0x08) && (get_direction_mask(x, y-1, z) & 0x04)) { result.go[W] = xyz(x, y-1, z); }
    if (z+1 < MAP_MAX && (mymask & 0x10) && (get_direction_mask(x, y, z+1) & 0x20)) { result.go[D] = xyz(x, y, z+1); }
    if (z-1 >= 0      && (mymask & 0x20) && (get_direction_mask(x, y, z-1) & 0x10)) { result.go[U] = xyz(x, y, z-1); }

    if (x+1 < MAP_MAX && y+1 < MAP_MAX && (mymask & 0x40) && (get_direction_mask(x+1, y+1, z) & 0x80)) { result.go[NE]  = xyz(x+1, y+1, z); }
    if (x-1 >= 0      && y-1 >= 0      && (mymask & 0x80) && (get_direction_mask(x-1, y-1, z) & 0x40)) { result.go[SW]  = xyz(x-1, y-1, z); }
    if (x-1 >= 0      && y+1 < MAP_MAX && (mymask & 0x100) && (get_direction_mask(x-1, y+1, z) & 0x200)) { result.go[SE]  = xyz(x-1, y+1, z); }
    if (x+1 < MAP_MAX && y-1 >= 0      && (mymask & 0x200) && (get_direction_mask(x+1, y-1, z) & 0x100)) { result.go[NW]  = xyz(x+1, y-1, z); }

    return result;
}

static struct Exits get_wiggled_directions(Location loc)
{
    struct Exits result = get_raw_directions(loc);

#define WIGGLE(n, nw, ne) do { \
        if (result.go[n] != NOWHERE) { \
            if (result.go[nw] == NOWHERE && lrng_one_in(10, loc, "wiggle" #n #nw)) { \
                result.go[nw] = result.go[n]; \
                result.go[n] = NOWHERE; \
            } else if (result.go[ne] == NOWHERE && lrng_one_in(9, loc, "wiggle" #n #ne)) { \
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
    struct Exits result = get_wiggled_directions(loc);
    return result;
}

bool has_light(Location loc)
{
    return z_of(loc) == 0;
}
