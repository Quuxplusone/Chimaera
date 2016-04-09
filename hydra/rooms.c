
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "rooms.h"

#define DIM(a) (sizeof a / sizeof *a)
#define ONE_OF(a, loc) a[lrng(loc, #a) % DIM(a)]
#define ANOTHER_OF(a, loc) a[lrng(loc, #a "another") % DIM(a)]

struct Description {
    struct Exits exits;
    bool is_corner;
    bool is_edge;
    bool is_dead_end;  // "Dead end."
    const char *adj1;
    const char *adj2;  // may be NULL
    const char *noun;
    const char *leading_x_and_y; // may be NULL
    bool describe_exits;
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

static MotionWord get_nth_unique_exit(const struct Exits *exits, int n)
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
        if (result == n) {
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
        "wide", "little", "big", "small", "large",
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

static const char *dir_to_text(MotionWord mot)
{
    switch (mot) {
        case N: return "north";
        case S: return "south";
        case E: return "east";
        case W: return "west";
        case NE: return "northeast";
        case NW: return "northwest";
        case SE: return "southeast";
        case SW: return "southwest";
        case U: return "up";
        case D: return "down";
    }
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
    for (int i=0; i < DIM(fortyfive); ++i) {
        if (fortyfive[i] == search_term) return 45;
    }
    for (int i=0; i < DIM(ninety); ++i) {
        if (ninety[i] == search_term) return 90;
    }
    for (int i=0; i < DIM(onethirtyfive); ++i) {
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

static struct Description get_raw_cave_description(Location loc)
{
    struct Description result;
    memset(&result, '\0', sizeof result);

    result.adj1 = get_raw_adj1(loc);
    result.noun = get_raw_noun(loc);

    const struct Exits exits = get_exits(loc);
    const int num_exits = count_unique_exits(&exits);

    result.exits = exits;

    if (num_exits == 1) {
        result.is_dead_end = true;
        result.adj1 = "dead";
        result.noun = "end";
    } else if (num_exits == 2 && exits.go[U] == NOWHERE && exits.go[D] == NOWHERE && lrng_one_in(2, loc, "is_hallway")) {
        const MotionWord dir1 = get_nth_unique_exit(&exits, 1);
        const MotionWord dir2 = get_nth_unique_exit(&exits, 2);
        static const char *hall_nouns[] = {
            "hall", "hallway", "corridor", "passage", "tunnel", "crawl"
        };
        result.noun = ONE_OF(hall_nouns, loc);

        const int deg = degrees_between(dir1, dir2);
        if (dir1 == E && dir2 == W) {
            result.adj2 = "east-west";
            if (lrng_one_in(3, loc, "sloping")) {
                result.adj1 = "gently sloping";
            }
        } else if (dir1 == N && dir2 == S) {
            result.adj2 = "north-south";
            if (lrng_one_in(3, loc, "sloping")) {
                result.adj1 = "gently sloping";
            }
        } else {
            static char buffer[] = "that enters from the northwest and bends sharply southeast";
            const bool can_omit_description_of_bend = (deg >= 135);
            if (can_omit_description_of_bend && lrng_two_in(3, loc, "leading_x_and_y")) {
                static const char *leading_verbs[] = {
                    "leading", "sloping", "sloping downwards", "sloping upwards",
                    "sloping gently downwards", "sloping gently upwards",
                };
                const char *leading = ONE_OF(leading_verbs, loc);
                sprintf(buffer, "%s from %s to %s", leading, dir_to_text(dir1), dir_to_text(dir2));
            } else {
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
            result.leading_x_and_y = buffer;
        }
    } else {
        result.describe_exits = true;
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
        MotionWord forest_dirs[8];
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
            result.leading_x_and_y = buffer;
        } else if (count_of_forested_neighbors == 2) {
            static char buffer[] = "with forest to the northwest and northeast";
            sprintf(buffer, "with forest to the %s and %s", dir_to_text(forest_dirs[0]), dir_to_text(forest_dirs[1]));
            result.leading_x_and_y = buffer;
        } else {
            result.leading_x_and_y = "at the edge of the forest";
        }
    }
    return result;
}

static struct Description get_raw_description(Location loc)
{
    if (is_overworld(loc)) {
        return get_raw_overworld_description(loc);
    } else {
        return get_raw_cave_description(loc);
    }
}

const char *an_exit_description(Location loc, const struct Exits *exits, MotionWord dir)
{
    static char buffer[100];
    const Location destination = exits->go[dir];
    assert(destination != NOWHERE);
    const struct Description dest_desc = get_raw_description(destination);
    static const char *exit_nouns[] = {
        "hall", "hallway", "passage", "corridor", "tunnel", "crawl", "squeeze"
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
    const struct Exits *exits = &desc.exits;
    const int num_exits = count_unique_exits(exits);

    switch (num_exits) {
        case 0:
        case 1:
            assert(false);
            break;
        case 2: {
            MotionWord dir1 = get_nth_unique_exit(exits, 1);
            MotionWord dir2 = get_nth_unique_exit(exits, 2);
            if (dir1 == U && dir2 == D) {
                printf("Stairs lead up and down%s.\n", lrng(loc, "fromhere") ? "" : " from here");
            } else if (dir1 != U && dir1 != D && dir2 != U && dir2 != D) {
                printf("There is %s to the %s and ",
                    an_exit_description(loc, exits, dir1), dir_to_text(dir1));
                printf("%s to the %s.\n",
                    an_exit_description(loc, exits, dir2), dir_to_text(dir2));
            } else {
                if (dir1 == U || dir1 == D) {
                    MotionWord temp = dir1; dir1 = dir2; dir2 = temp;
                }
                assert(dir1 != U && dir1 != D);
                assert(dir2 == U || dir2 == D);
                printf("%s leads %s. ", Cap(an_exit_description(loc, exits, dir1)), dir_to_text(dir1));
                printf("Stairs lead %s.\n", dir_to_text(dir2));
            }
            break;
        }
        default: {
            assert(num_exits >= 3);
            if (strcmp(desc.noun, "tunnel") == 0 || strcmp(desc.noun, "corridor") == 0) {
                printf("Side passages lead ");
            } else if (strncmp(desc.noun, "passage", 7) == 0 || lrng_one_in(2, loc, "tunpas")) {
                printf("Tunnels lead ");
            } else {
                printf("Passages lead ");
            }
            for (int i=0; i < num_exits; ++i) {
                if (i == num_exits-1) {
                    printf(", and ");
                } else if (i != 0) {
                    printf(", ");
                }
                printf("%s", dir_to_text(get_nth_unique_exit(exits, i+1)));
            }
            printf(".\n");
        }
    }
}

void print_short_description(Location loc)
{
    const struct Description desc = get_raw_description(loc);

    if (desc.is_dead_end) {
        puts("Dead end.");
    } else {
        printf("You're %s %s%s%s %s.\n",
            desc.is_corner ? "at corner of" : desc.is_edge ? "at edge of" : "in",
            desc.adj1, desc.adj2 ? " " : "", desc.adj2 ? desc.adj2 : "",
            desc.noun);
    }
}

void print_long_description(Location loc)
{
    const struct Description desc = get_raw_description(loc);

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
    printf("%s %s%s%s %s%s%s.\n",
        an(desc.adj1), desc.adj1, desc.adj2 ? " " : "", desc.adj2 ? desc.adj2 : "",
        desc.noun, desc.leading_x_and_y ? " " : "", desc.leading_x_and_y ? desc.leading_x_and_y : "");

    if (desc.describe_exits) {
        print_exit_descriptions(loc, desc);
    } else if (is_overworld(loc) && desc.exits.go[D] != NOWHERE) {
        puts("There is a large hole at your feet, leading down into darkness.");
    } else if (is_overworld(loc) && !is_forested(loc)) {
        puts("The sun is shining brightly above.");
    }
}
