
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "rooms.h"
#include "util.h"
#include "vocab.h"

#define ONE_OF(a, loc) a[lrng(loc, #a) % DIM(a)]
#define ANOTHER_OF(a, loc) a[lrng(loc, #a "another") % DIM(a)]

struct Description {
    struct Exits exits;  // the exits we have not already described in the text
    bool is_corner;
    bool is_edge;
    bool is_dead_end;  // "Dead end."
    const char *adj1;
    const char *adj2;  // may be NULL
    const char *noun;
    const char *prepositional_phrase; // may be NULL
    const char *above;  // may be NULL
    const char *below;  // may be NULL
};

static int count_unique_exits(const struct Exits *exits)
{
    int result = 0;
    for (int i = MIN_MOTION; i <= MAX_MOTION; ++i) {
        if (exits->go[i] == NOWHERE) continue;
        result += 1;
        for (int j = MIN_MOTION; j < i; ++j) {
            if (exits->go[i] == exits->go[j]) {
                result -= 1;  // for example, a staircase that goes "down to the north"
                break;
            }
        }
    }
    return result;
}

static MotionWord get_nth_exit(const struct Exits *exits, int n)
{
    int result = 0;
    for (int i = MIN_MOTION; i <= MAX_MOTION; ++i) {
        if (exits->go[i] == NOWHERE) continue;
        if (result++ == n) {
            return i;
        }
    }
    assert(false);
    return 0;
}

static const char *get_raw_adj1(Location loc)
{
    static const char *raw_adjs[] = {
        "impressive", "ornate", "arched", "imposing", "cramped",
        "gigantic", "tiny", "enormous", "low", "high", "narrow",
        "wide", "little", "big", "small", "large", "damp", "dry",
        "magnificent",
    };
    return ONE_OF(raw_adjs, loc);
}

static const char *get_raw_noun(Location loc)
{
    static const char *raw_nouns[] = {
        "room", "chamber", "hall",
        "passage", "passageway", "corridor", "tunnel",
        "alcove",
        "cavern", "cave", "grotto",
    };
    return ONE_OF(raw_nouns, loc);
}

static int degrees_between(MotionWord mot, MotionWord mot2)
{
    if (mot2 < mot) {
        MotionWord temp = mot; mot = mot2; mot2 = temp;
    }
    const int search_term = ((mot << 8) | mot2);
    const int fortyfive[] = {
        (N << 8) | NW, (N << 8) | NE,
        (S << 8) | SW, (S << 8) | SE,
        (E << 8) | NE, (E << 8) | SE,
        (W << 8) | NW, (W << 8) | SW
    };
    const int ninety[] = {
        (N << 8) | E, (N << 8) | W,
        (S << 8) | E, (S << 8) | W,
        (NE << 8) | NW, (NE << 8) | SE,
        (SW << 8) | SE, (SW << 8) | NW
    };
    const int onethirtyfive[] = {
        (N << 8) | SE, (N << 8) | SW,
        (S << 8) | NE, (S << 8) | NW,
        (E << 8) | NW, (E << 8) | SW,
        (W << 8) | NE, (W << 8) | SE
    };
    for (int i=0; i < (int)DIM(fortyfive); ++i) {
        if (fortyfive[i] == search_term) return 45;
    }
    for (int i=0; i < (int)DIM(ninety); ++i) {
        if (ninety[i] == search_term) return 90;
    }
    for (int i=0; i < (int)DIM(onethirtyfive); ++i) {
        if (onethirtyfive[i] == search_term) return 135;
    }
    return 180;
}

static const char *an(const char *next_word)
{
    return strchr("aeiou", next_word[0]) ? "an" : "a";
}

static const char *Cap(const char *word)
{
    static char buffer[100];  // This works because we only ever need one thing capitalized at a time.
    assert(strlen(word) < sizeof buffer);
    strcpy(buffer, word);
    buffer[0] = toupper(buffer[0]);
    return buffer;
}

static bool should_create_hallway(Location loc, struct Description *desc)
{
    const int num_exits = count_unique_exits(&desc->exits);
    const int num_semicardinal_exits = num_exits - (desc->exits.go[U] != NOWHERE) - (desc->exits.go[D] != NOWHERE);

    if (num_semicardinal_exits < 2) {
        return false;  // can't possibly make a hallway without two endpoints
    } else if (!strcmp(desc->noun, "tunnel") || !strncmp(desc->noun, "passage", 7)) {
        return true;
    } else if (num_semicardinal_exits == 2) {
        return lrng_two_in(3, loc, "is_hallway");
    } else if (num_semicardinal_exits == 3 || num_semicardinal_exits == 4) {
        return lrng_one_in(10, loc, "is_hallway");
    }
    return false;
}

static struct Description get_raw_cave_description(Location loc, bool verbose)
{
    static char buffer[100];
    struct Description result;
    memset(&result, '\0', sizeof result);

    struct Exits exits = get_exits(loc);
    result.exits = exits;

    if (count_unique_exits(&exits) == 1) {
        result.is_dead_end = true;
        result.adj1 = "dead";
        result.noun = "end";
        result.exits.go[get_nth_exit(&exits, 0)] = NOWHERE;
        return result;
    }

    result.adj1 = get_raw_adj1(loc);
    result.noun = get_raw_noun(loc);

    if (should_create_hallway(loc, &result)) {
        MotionWord dir1 = get_nth_exit(&exits, 0);
        MotionWord dir2 = get_nth_exit(&exits, 1);
        static const char *hall_nouns[] = {
            "hall", "hallway", "corridor", "passage", "tunnel", "crawl"
        };
        result.noun = ONE_OF(hall_nouns, loc);

        const int deg = degrees_between(dir1, dir2);
        if (dir1 == E && dir2 == W) {
            result.adj1 = "east-west";
            if (lrng_one_in(3, loc, "sloping")) {
                result.adj2 = result.adj1;
                result.adj1 = "gently sloping";
            }
        } else if (dir1 == N && dir2 == S) {
            result.adj1 = "north-south";
            if (lrng_one_in(3, loc, "sloping")) {
                result.adj2 = result.adj1;
                result.adj1 = "gently sloping";
            }
        } else if (verbose) {
            const bool can_omit_description_of_bend = (deg >= 135);
            if (can_omit_description_of_bend && lrng_two_in(3, loc, "leading")) {
                static const char *leading_verbs[] = {
                    "leading", "sloping", "sloping downwards", "sloping upwards",
                    "sloping gently downwards", "sloping gently upwards",
                };
                const char *leading = ONE_OF(leading_verbs, loc);
                sprintf(buffer, "%s from %s to %s", leading, dir_to_text(dir1), dir_to_text(dir2));
            } else {
                if (lrng_one_in(2, loc, "swaphall")) {
                    // With the following construction, it's okay to swap the directions sometimes.
                    MotionWord temp = dir1;
                    dir1 = dir2;
                    dir2 = temp;
                }
                if (deg == 90 && lrng_one_in(3, loc, "corner")) {
                    result.is_corner = true;
                }
                sprintf(buffer, "that enters from the %s and bends ", dir_to_text(dir1));
                if (deg == 45) {
                    strcat(buffer, "sharply ");
                    strcat(buffer, dir_to_text(dir2));
                } else if (deg == 90) {
                    if (lrng_one_in(2, loc, "sharply")) {
                        strcat(buffer, "sharply ");
                    }
                    strcat(buffer, dir_to_text(dir2));
                } else if (deg == 135) {
                    if (lrng_two_in(3, loc, "sharply")) {
                        strcat(buffer, lrng_one_in(2, loc, "gently") ? "gently " : "gradually ");
                    }
                    strcat(buffer, dir_to_text(dir2));
                } else {
                    assert(deg == 180);
                    sprintf(buffer, "exiting to %s and %s", dir_to_text(dir1), dir_to_text(dir2));
                }
            }
            result.prepositional_phrase = buffer;
        }
        result.exits.go[dir1] = NOWHERE;
        result.exits.go[dir2] = NOWHERE;
    } else if (verbose && exits.go[U] != NOWHERE && z_of(loc) != 1 && lrng_one_in(3, loc, "below") && !lrng_one_in(3, exits.go[U], "above")) {
        struct Description dest_desc = get_raw_cave_description(exits.go[U], false);
        sprintf(buffer, "%s %s", dest_desc.adj1, dest_desc.noun);
        result.below = buffer;
        result.exits.go[U] = NOWHERE;
    } else if (verbose && exits.go[D] != NOWHERE && lrng_one_in(3, loc, "above") && !lrng_one_in(3, exits.go[D], "below")) {
        struct Description dest_desc = get_raw_cave_description(exits.go[D], false);
        sprintf(buffer, "%s %s", dest_desc.adj1, dest_desc.noun);
        result.above = buffer;
        result.exits.go[D] = NOWHERE;
    }

    return result;
}

static struct Description get_raw_overworld_description(Location loc)
{
    assert(is_overworld(loc));

    struct Description result;
    memset(&result, '\0', sizeof result);

    const struct Exits exits = get_exits(loc);
    result.exits = exits;

    if (is_forested(loc)) {
        static const char *raw_adjs[] = {
            "open", "dense", "patchy",
        };
        result.adj1 = ONE_OF(raw_adjs, loc);
        result.noun = "forest";

        int count_of_forested_neighbors = 0;
        int count_of_deforested_neighbors = 0;
        for (MotionWord m = MIN_MOTION; m <= MAX_MOTION; ++m) {
            if (exits.go[m] != NOWHERE) {
                if (is_forested(exits.go[m])) {
                    count_of_forested_neighbors += 1;
                } else {
                    count_of_deforested_neighbors += 1;
                }
            }
        }
        if (count_of_deforested_neighbors == 0) {
            result.adj1 = "deep";
        } else if (count_of_forested_neighbors >= 3) {
            result.is_edge = true;
        }
    } else {
        static const char *raw_adjs[] = {
            "sunny", "dappled", "rolling", "grassy", "open"
        };
        static const char *raw_nouns[] = {
            "meadow", "field", "plain"
        };
        result.adj1 = ONE_OF(raw_adjs, loc);
        result.noun = ONE_OF(raw_nouns, loc);

        int count_of_forested_neighbors = 0;
        MotionWord forest_dirs[MAX_MOTION - MIN_MOTION + 1];
        for (MotionWord m = MIN_MOTION; m <= MAX_MOTION; ++m) {
            if (exits.go[m] != NOWHERE) {
                if (is_forested(exits.go[m])) {
                    forest_dirs[count_of_forested_neighbors++] = m;
                }
            }
        }
        if (count_of_forested_neighbors == 0) {
            // do nothing
        } else if (count_of_forested_neighbors == 1) {
            static char buffer[] = "with forest to the northwest";
            sprintf(buffer, "with forest to the %s", dir_to_text(forest_dirs[0]));
            result.prepositional_phrase = buffer;
        } else if (count_of_forested_neighbors == 2) {
            static char buffer[] = "with forest to the northwest and northeast";
            sprintf(buffer, "with forest to the %s and %s", dir_to_text(forest_dirs[0]), dir_to_text(forest_dirs[1]));
            result.prepositional_phrase = buffer;
        } else {
            result.prepositional_phrase = "at the edge of the forest";
        }
    }
    return result;
}

static struct Description get_raw_description(Location loc, bool verbose)
{
    if (is_overworld(loc)) {
        return get_raw_overworld_description(loc);
    } else {
        return get_raw_cave_description(loc, verbose);
    }
}

const char *an_exit_description(Location loc, const struct Exits *exits, MotionWord dir)
{
    static char buffer[100];
    const Location destination = exits->go[dir];
    assert(destination != NOWHERE);
    const struct Description dest_desc = get_raw_description(destination, false);
    static const char *exit_nouns[] = {
        "hall", "hallway", "passage", "corridor", "tunnel", "crawl", "squeeze", "crack"
    };
    for (int i=0; i < (int)DIM(exit_nouns); ++i) {
        if (strcmp(dest_desc.noun, exit_nouns[i]) == 0) {
            if (lrng_one_in(3, loc, "exit_adj")) {
                sprintf(buffer, "%s %s", an(dest_desc.noun), dest_desc.noun);
            } else {
                sprintf(buffer, "%s %s %s", an(dest_desc.adj1), dest_desc.adj1, dest_desc.noun);
            }
            return buffer;
        }
    }
    // Otherwise, pick a "tunnel" word at random.
    sprintf(buffer, "a %s", ONE_OF(exit_nouns, loc));
    return buffer;
}


static void print_exit_descriptions(Location loc, struct Description desc)
{
    assert(!is_overworld(loc));
    assert(!desc.is_dead_end);

    struct Exits exits = desc.exits;
    const bool upstairs = (exits.go[U] != NOWHERE) && has_up_stairs(loc);
    const bool downstairs = (exits.go[D] != NOWHERE) && has_down_stairs(loc);

    if (upstairs) exits.go[U] = NOWHERE;
    if (downstairs) exits.go[D] = NOWHERE;

    const int num_tunnels = count_unique_exits(&exits);

    const char *tunnel_word = "tunnel";
    if (!strcmp(desc.noun, "tunnel") || !strcmp(desc.noun, "corridor") || !strncmp(desc.noun, "hall", 4)) {
        tunnel_word = "side passage";
    } else if (!strcmp(desc.noun, "tunnel") || lrng_one_in(2, loc, "tunnelword")) {
        tunnel_word = "passage";
    }

    switch (num_tunnels) {
        case 0: {
            break;
        }
        case 1: {
            MotionWord dir1 = get_nth_exit(&exits, 0);
            printf("A %s leads %s.\n", tunnel_word, dir_to_text(dir1));
            break;
        }
        case 2: {
            MotionWord dir1 = get_nth_exit(&exits, 0);
            MotionWord dir2 = get_nth_exit(&exits, 1);
            if (is_semicardinal(dir1) && is_semicardinal(dir2)) {
                printf("There is %s to the %s and ",
                    an_exit_description(loc, &exits, dir1), dir_to_text(dir1));
                printf("%s to the %s.\n",
                    an_exit_description(loc, &exits, dir2), dir_to_text(dir2));
            } else {
                printf("%ss lead %s and %s.\n", Cap(tunnel_word), dir_to_text(dir1), dir_to_text(dir2));
            }
            break;
        }
        default: {
            assert(num_tunnels >= 3);
            int index_of_exit_to_omit = 100;
            int num_nonomitted_exits = num_tunnels;
            if (num_tunnels >= 4 && lrng_one_in(2, loc, "omit")) {
                index_of_exit_to_omit = lrng(loc, "omit2") % num_tunnels;
                num_nonomitted_exits -= 1;
            }
            printf("%ss lead ", Cap(tunnel_word));
            for (int i=0; i < num_nonomitted_exits; ++i) {
                const int adjusted_i = i + (i >= index_of_exit_to_omit);
                if (i == num_nonomitted_exits-1) {
                    printf(", and ");
                } else if (i != 0) {
                    printf(", ");
                }
                MotionWord m = get_nth_exit(&exits, adjusted_i);
                printf("%s", dir_to_text(m));
            }
            printf(".\n");
            if (index_of_exit_to_omit != 100) {
                const MotionWord m = get_nth_exit(&exits, index_of_exit_to_omit);
                const struct Description dest_desc = get_raw_description(exits.go[m], false);
                if (m == U) {
                    printf("Above you is %s %s %s.\n",
                           an(dest_desc.adj1), dest_desc.adj1, desc.noun);
                } else if (m == D) {
                    printf("Below you is %s %s %s.\n",
                           an(dest_desc.adj1), dest_desc.adj1, dest_desc.noun);
                } else if (lrng_one_in(2, loc, "omit3")) {
                    printf("There is %s %s %s to the %s.\n",
                           an(dest_desc.adj1), dest_desc.adj1, dest_desc.noun,
                           dir_to_text(m));
                } else {
                    printf("To the %s is %s %s %s.\n",
                           dir_to_text(m),
                           an(dest_desc.adj1), dest_desc.adj1, dest_desc.noun);
                }
            }
            break;
        }
    }

    if (upstairs) {
        if (z_of(loc) == 1) {
            printf("In the center of the room a marble staircase leads upward.\n");
            if (downstairs) {
                printf("Another set of stairs leads downward from the far end of the room.\n");
            }
        } else {
            if (downstairs) {
                printf("Stairs lead both up and down from here.\n");
            } else {
                printf("Stairs lead up.\n");
            }
        }
    } else if (downstairs) {
        printf("Stairs lead down.\n");
    }
}

void print_short_description(Location loc)
{
    const struct Description desc = get_raw_description(loc, true);

    if (desc.is_dead_end) {
        puts("Dead end.");
    } else {
        if (desc.is_corner) {
            printf("You're at corner of ");
        } else if (desc.is_edge) {
            printf("You're at edge of ");
        } else {
            printf("You're in ");
        }
        if (desc.above != NULL) {
            assert(desc.below == NULL);
            printf("%s above %s", desc.noun, desc.above);
        } else if (desc.below != NULL) {
            printf("%s below %s", desc.noun, desc.below);
        } else {
            printf("%s%s%s %s",
                desc.adj1, desc.adj2 ? " " : "", desc.adj2 ? desc.adj2 : "",
                desc.noun);
        }
        printf(".\n");
    }
}

void print_long_description(Location loc)
{
    const struct Description desc = get_raw_description(loc, true);

    if (desc.is_dead_end) {
        puts("Dead end.");
        return;
    }

    if (desc.is_corner) {
        printf("You're at the corner of ");
    } else if (desc.is_edge) {
        printf("You're at the edge of ");
    } else {
        printf("You're in ");
    }
    printf("%s %s%s%s %s",
        an(desc.adj1), desc.adj1, desc.adj2 ? " " : "", desc.adj2 ? desc.adj2 : "",
        desc.noun);
    if (desc.prepositional_phrase != NULL) {
        assert(desc.above == NULL && desc.below == NULL);
        printf(" %s", desc.prepositional_phrase);
    } else if (desc.above != NULL) {
        assert(desc.below == NULL);
        printf(" above %s %s", an(desc.above), desc.above);
    } else if (desc.below != NULL) {
        printf(" below %s %s", an(desc.below), desc.below);
    }
    printf(".\n");

    if (is_overworld(loc)) {
        if (desc.exits.go[D] != NOWHERE) {
            puts("Before you is a marble staircase set into the earth, leading down into darkness.");
        } else if (is_overworld(loc) && !is_forested(loc)) {
            puts("The sun is shining brightly above.");
        }
    } else {
        print_exit_descriptions(loc, desc);
    }
}
