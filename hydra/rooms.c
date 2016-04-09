
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
    bool is_dead_end;  // "Dead end."
    const char *adj1;
    const char *adj2;  // may be NULL
    const char *noun;
    const char *leading_x_and_y; // may be NULL
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

static struct Description get_raw_description(Location loc)
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

        if (dir1 == E && dir2 == W) {
            result.adj2 = "east-west";
        } else if (dir1 == N && dir2 == S) {
            result.adj2 = "north-south";
        } else {
            static char buffer[] = "that enters from the northwest and bends sharply southeast";
            switch (lrng(loc, "leading_x_and_y") % 2) {
                case 0: {
                    static const char *leading_verbs[] = {
                        "leading", "sloping", "sloping downwards", "sloping upwards",
                        "sloping gently downwards", "sloping gently upwards",
                    };
                    const char *leading = ONE_OF(leading_verbs, loc);
                    sprintf(buffer, "%s %s and %s", leading, dir_to_text(dir1), dir_to_text(dir2));
                    break;
                }
                case 1: {
                    sprintf(buffer, "that enters from the %s and bends %s", dir_to_text(dir1), dir_to_text(dir2));
                    break;
                }
            }
            result.leading_x_and_y = buffer;
        }
    }

    return result;
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

void print_short_description(Location loc)
{
    const struct Description desc = get_raw_description(loc);

    if (desc.is_dead_end) {
        puts("Dead end.");
    } else {
        printf("You're in %s%s%s %s.\n",
            desc.adj1, desc.adj2 ? " " : "", desc.adj2 ? desc.adj2 : "",
            desc.noun);
    }
}

void print_long_description(Location loc)
{
    const struct Description desc = get_raw_description(loc);
    const struct Exits exits = desc.exits;  // for convenience

    printf("FYI, you're at (%d,%d,%d).\n", x_of(loc), y_of(loc), z_of(loc));

    if (desc.is_dead_end) {
        puts("Dead end.");
    } else {
        printf("You're in %s %s%s%s %s%s%s.\n",
            an(desc.adj1), desc.adj1, desc.adj2 ? " " : "", desc.adj2 ? desc.adj2 : "",
            desc.noun, desc.leading_x_and_y ? " " : "", desc.leading_x_and_y ? desc.leading_x_and_y : "");

        if (desc.leading_x_and_y == NULL) {
            const int num_exits = count_unique_exits(&exits);

            switch (num_exits) {
                case 0:
                case 1:
                    assert(false);
                    break;
                case 2: {
                    MotionWord dir1 = get_nth_unique_exit(&exits, 1);
                    MotionWord dir2 = get_nth_unique_exit(&exits, 2);
                    if (dir1 == U && dir2 == D) {
                        printf("Stairs lead up and down%s.\n", lrng(loc, "fromhere") ? "" : " from here");
                    } else if (dir1 != U && dir1 != D && dir2 != U && dir2 != D) {
                        printf("There is %s to the %s and ",
                            an_exit_description(loc, &exits, dir1), dir_to_text(dir1));
                        printf("%s to the %s.\n",
                            an_exit_description(loc, &exits, dir2), dir_to_text(dir2));
                    } else {
                        if (dir1 == U || dir1 == D) {
                            MotionWord temp = dir1; dir1 = dir2; dir2 = temp;
                        }
                        assert(dir1 != U && dir1 != D);
                        assert(dir2 == U || dir2 == D);
                        printf("%s leads %s. ", Cap(an_exit_description(loc, &exits, dir1)), dir_to_text(dir1));
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
                        printf("%s", dir_to_text(get_nth_unique_exit(&exits, i+1)));
                    }
                    printf(".\n");
                }
            }
        }
    }

    if (has_light(loc)) {
        puts("The sun is shining brightly above.");
    }
}
