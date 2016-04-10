
#include <stdlib.h>
#include "distance.h"
#include "map.h"
#include "util.h"

struct AStarNode {
    Location loc;
    int distance_from_source;
    int estimated_result;
};

int get_crow_distance_between(Location from, Location to)
{
    int dx = abs(x_of(from) - x_of(to));
    int dy = abs(y_of(from) - y_of(to));
    int dz = abs(z_of(from) - z_of(to));
    return ((dy > dx) ? dy : dx) + dz;
}

static struct AStarNode pop_first(struct AStarNode *frontier, int n)
{
    struct AStarNode result = frontier[0];
    for (int i=0; i < n-1; ++i) {
        frontier[i] = frontier[i+1];
    }
    return result;
}

static int insert(struct AStarNode *frontier, int frontier_size, int frontier_capacity, struct AStarNode node)
{
    for (int i=0; i < frontier_size; ++i) {
        if (frontier[i].loc == node.loc) {
            if (frontier[i].estimated_result < node.estimated_result) {
                while (i > 0 && node.estimated_result < frontier[i-1].estimated_result) {
                    frontier[i] = frontier[i-1];
                    --i;
                }
                frontier[i] = node;
            }
            return frontier_size;
        }
    }
    // Insert the new node (possibly in place of an old one).
    if (frontier_size < frontier_capacity) {
        frontier_size += 1;
    }
    if (true) {
        int i = frontier_size - 1;
        while (i > 0 && node.estimated_result < frontier[i-1].estimated_result) {
            frontier[i] = frontier[i-1];
            --i;
        }
        frontier[i] = node;
    }
    return frontier_size;
}

int get_shortest_distance_between(Location from, Location to, int cutoff)
{
    struct AStarNode frontier[20];
    frontier[0] = (struct AStarNode){ from, 0, get_crow_distance_between(from, to) };
    int frontier_size = 1;

    while (frontier_size != 0) {
        struct AStarNode current = pop_first(frontier, frontier_size--);
        const int next_distance = current.distance_from_source + 1;
        if (next_distance < cutoff) {
            struct Exits exits = get_exits(current.loc);
            for (MotionWord m = MIN_MOTION; m <= MAX_MOTION; ++m) {
                Location nextloc = exits.go[m];
                if (nextloc == to) {
                    return next_distance;
                }
                if (nextloc != NOWHERE && nextloc != current.loc) {
                    struct AStarNode next = { nextloc, next_distance, next_distance + get_crow_distance_between(nextloc, to) };
                    frontier_size = insert(frontier, frontier_size, DIM(frontier), next);
                }
            }
        }
    }

    return cutoff;
}

MotionWord get_direction_from(Location from, Location to)
{
    if (from != to) {
        struct Exits exits = get_exits(from);
        for (MotionWord m = MIN_MOTION; m <= MAX_MOTION; ++m) {
            if (exits.go[m] == to) {
                return m;
            }
        }
    }
    return NOTHING;
}

bool is_adjacent_to(Location from, Location to)
{
    return get_direction_from(from, to) != NOTHING;
}
