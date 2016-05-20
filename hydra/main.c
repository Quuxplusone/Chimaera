
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "distance.h"
#include "map.h"
#include "objs.h"
#include "npcs.h"
#include "rooms.h"
#include "util.h"
#include "vocab.h"

#ifdef Z_MACHINE
#ifdef SAVE_AND_RESTORE
extern int attempt_save(void);
extern int attempt_restore(void);
#endif /* SAVE_AND_RESTORE */

#define SOFT_NL " "
#else
#define SOFT_NL "\n"
#endif /* Z_MACHINE */

static bool verbose_mode = false;


/*========== Input routines. ==============================================
 */

#define BUF_SIZE 72
char buffer[BUF_SIZE]; /* your input goes here */
char word1[BUF_SIZE], word2[BUF_SIZE]; /* and then we snarf it to here */

bool yes(const char *q, const char *y, const char *n)
{
    while (true) {
        printf("%s\n** ", q); fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        if (tolower(*buffer) == 'y') {
            if (y) puts(y);
            return true;
        } else if (tolower(*buffer) == 'n') {
            if (n) puts(n);
            return false;
        } else {
            puts(" Please answer Yes or No.");
        }
    }
}

void listen(void)
{
    char *p, *q;
    while (true) {
        printf("* "); fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        for (p = buffer; isspace(*p); ++p) ;
        if (*p == '\0') {
            puts(" Tell me to do something."); continue;
        }
        /* Notice that this algorithm depends on the buffer's being
         * terminated by "\n\0", or at least some whitespace character. */
        for (q = word1; !isspace(*p); ++p, ++q) {
            *q = tolower(*p);
        }
        *q = '\0';
        for (++p; isspace(*p); ++p) ;
        if (*p == '\0') {
            *word2 = '\0'; return;
        }
        for (q = word2; !isspace(*p); ++p, ++q) {
            *q = tolower(*p);
        }
        *q = '\0';
        for (++p; isspace(*p); ++p) ;
        if (*p == '\0') return;
        puts(" Please stick to 1- and 2-word commands.");
    }
}

void shift_words(void)
{
    strcpy(word1, word2);
    *word2 = '\0';
}


/*========== Verbs. ===============================================
 */

void describe_rabbits(Location loc)
{
    int rabbits_in_room = 0;
    struct Rabbit *rabbit = NULL;
    for (int i=0; i < number_of_rabbits; ++i) {
        rabbits_in_room += (rabbits[i].loc == loc);
        rabbit = &rabbits[i];
    }
    if (rabbits_in_room != 0) {
        puts("");
        if (rabbits_in_room == 1) {
            printf("There is a fuzzy %s rabbit", rabbit->color);
        } else {
            printf("There are %d fuzzy little rabbits", rabbits_in_room);
        }
        if (is_overworld(loc)) {
            printf(" munching grass nearby.\n");
        } else {
            printf(" in the room with you.\n");
        }
    }
}

void look_around(Location loc, bool force_verbose)
{
    static Location last_few_locations[10] = {
        NOWHERE, NOWHERE, NOWHERE, NOWHERE, NOWHERE,
        NOWHERE, NOWHERE, NOWHERE, NOWHERE, NOWHERE,
    };

    const bool lighted = (here(LAMP, loc) && objs(LAMP).prop == 1) || has_light(loc);

    if (!lighted) {
        puts("It is now pitch dark.  If you proceed you will most likely fall into a pit.");
        return;
    }

    // Update the list of locations we've recently passed through.
    bool be_terse = false;
    Location to_swap = loc;
    if (!force_verbose) {
        for (int i=0; i < 10; ++i) {
            Location temp = last_few_locations[i];
            last_few_locations[i] = to_swap;
            to_swap = temp;
            if (to_swap == loc) {
                be_terse = true;
                break;
            }
        }
    }

    if (be_terse) {
        print_short_description(loc);
    } else {
        print_long_description(loc);
    }

    /* Describe the objects at this location. */
    for (ObjectWord t = MIN_OBJ; t <= MAX_OBJ; ++t) {
        if (there(t, loc)) {
            const char *obj_description = objs(t).desc[objs(t).prop];
            if (obj_description != NULL) {
                puts(obj_description);
            }
        }
    }

    describe_rabbits(loc);
}

void attempt_inventory()
{
    bool holding_anything = false;
    for (ObjectWord t = MIN_OBJ; t <= MAX_OBJ; ++t) {
        if (toting(t)) {
            if (!holding_anything) {
                holding_anything = true;
                puts("You are currently holding the following:");
            }
            printf(" %s\n", objs(t).name);
        }
    }
    if (!holding_anything) {
        puts("You're not carrying anything.");
    }
}

void attempt_take(ObjectWord obj, Location loc)
{
    if (toting(obj)) {
        puts("You are already carrying it!");
    } else if (obj == BIRD) {
        if (toting(ROD)) {
            puts("The bird was unafraid at first, but as you approach it becomes disturbed and you cannot catch it.");
        } else {
            puts("You can catch the bird, but you cannot carry it.");
        }
    } else if (holding_count() >= 7) {
        puts("You can't carry anything more.  You'll have to drop something first.");
    } else if (obj == RABBIT) {
        puts("The rabbit evades your clumsy attempt at capture.");
    } else {
        assert(there(obj, loc));
        objs(obj).loc = INHAND;
        puts("OK.");
    }
}

void attempt_drop(ObjectWord obj, Location loc)
{
    if (!toting(obj)) {
        puts("You aren't carrying it!");
    } else {
        objs(obj).loc = loc;
        puts("OK.");
    }
}

void attempt_on(Location loc)
{
    if (!here(LAMP, loc)) {
        puts("You have no source of light.");
    } else if (objs(LAMP).prop == 1) {
        puts("Your lamp is already on!");
    } else {
        objs(LAMP).prop = 1;
        puts("Your lamp is now on.");
        if (!has_light(loc)) {
            look_around(loc, true);
        }
    }
}

void attempt_off(Location loc)
{
    if (!here(LAMP, loc)) {
        puts("You have no source of light.");
    } else if (objs(LAMP).prop == 1) {
        puts("Your lamp is already off!");
    } else {
        objs(LAMP).prop = 0;
        puts("Your lamp is now off.");
        if (!has_light(loc)) {
            look_around(loc, true);
        }
    }
}

void attempt_wave(ObjectWord obj, Location loc)
{
    if (!toting(obj)) {
        puts("You aren't carrying it!");
    } else if (obj == LAMP && !has_light(loc)) {
        puts("Shadows dance and sway on the walls.");
    } else {
        puts("Nothing happens.");
        if (obj == ROD) {
            const int r = rabbits_at(loc);
            if (r == 1) {
                puts("The rabbit looks at you expectantly.");
            } else if (r >= 2) {
                puts("The rabbits look at you expectantly.");
            }
        }
    }
}

void attempt_abra(Location loc)
{
    if (number_of_rabbits < (int)DIM(rabbits)) {
        struct Rabbit *rabbit = &rabbits[number_of_rabbits++];
        *rabbit = create_rabbit(loc);
        printf("Upon your utterance of the magic word, a fuzzy %s rabbit poofs into existence", rabbit->color);
        if (ran(2)) {
            struct Exits exits = get_exits(loc);
            MotionWord m = (N + ran(8));  // one of the 8 semicardinal directions
            assert(is_semicardinal(m));
            if (exits.go[m] != NOWHERE) {
                printf(" and hops away to the %s", dir_to_text(m));
                rabbit->oldloc = loc;
                rabbit->loc = exits.go[m];
            } else {
                printf(". Its nose twitches");
            }
        }
        printf(".\n");
    } else {
        printf("Nothing happens.\n");
    }
}

void attempt_abra_rabbit(Location loc)
{
    int rabbits_in_room = 0;
    struct Rabbit *rabbit = NULL;
    for (int i=0; i < number_of_rabbits; ++i) {
        if (rabbits[i].loc == loc) {
            rabbits_in_room += 1;
            rabbit = &rabbits[i];
        }
    }
    assert(1 <= rabbits_in_room && rabbits_in_room <= number_of_rabbits);
    assert(rabbit != NULL);

    const char *incantations[] = {
        "wiggle your fingers at", "point dramatically at", "point toward",
        "gesture vaguely in the direction of", "glare at"
    };
    printf("You %s the rabbit%s and intone the magic word.\n",
        incantations[ran(DIM(incantations))], rabbits_in_room == 1 ? "" : "s");
    printf("The%s%s rabbit disappears in a puff of smoke!\n",
           rabbits_in_room == 1 ? "" : " ",
           rabbits_in_room == 1 ? "" : rabbit->color);

    *rabbit = rabbits[--number_of_rabbits];
}

void attempt_accio(ObjectWord obj, Location loc)
{
    if (toting(obj)) {
        puts("You are already carrying it!");
    } else if (here(obj, loc) || (obj == RABBIT && rabbits_at(loc))) {
        puts("I believe what you want is right here with you.");
    } else {
        apport(obj, INHAND);
        puts("OK.");
    }
}


/*========== Scoring and quitting. =========================================
 */

void quit(void)
{
    puts("Goodbye.");
#ifdef Z_MACHINE
    puts("\n");
#endif /* Z_MACHINE */
    exit(0);
}


/*========== NPC movement. ================================================
 */

static MotionWord choose_random_exit(struct Exits *exits, Location to_avoid)
{
    MotionWord result = NOTHING;
    MotionWord fallback_result = NOTHING;
    int count = 0;
    for (MotionWord mot = MIN_MOTION; mot <= MAX_MOTION; ++mot) {
        if (exits->go[mot] == NOWHERE) {
            // do nothing
        } else if (exits->go[mot] == to_avoid) {
            fallback_result = mot;
        } else {
            if (ran(++count) == 0) {
                result = mot;
            }
        }
    }
    return (result != NOTHING) ? result : fallback_result;
}

static void attempt_moving_rabbit(Location loc, struct Rabbit *rabbit, int rabbits_in_room)
{
    struct Exits exits = get_exits(rabbit->loc);
    MotionWord mot = choose_random_exit(&exits, rabbit->oldloc);
    rabbit->oldloc = rabbit->loc;
    rabbit->loc = exits.go[mot];
    if (rabbit->oldloc == loc && rabbit->loc != loc) {
        // Describe a rabbit leaving the player's current location.
        printf("The ");
        if (rabbits_in_room > 1 || ran(3) == 0) {
            printf("%s ", rabbit->color);
        }
        printf("rabbit ");
        if (mot == U) {
            if (is_overworld(rabbit->loc)) {
                printf("hops up the stairs");
                if (!is_forested(rabbit->loc)) {
                    printf(", its fur catching the sunlight as it disappears");
                }
                printf(".\n");
            } else if (has_up_stairs(loc)) {
                printf("hops upstairs.\n");
            } else {
                printf("hops away upward.\n");
            }
        } else if (mot == D) {
            if (has_down_stairs(loc)) {
                printf("hops downstairs.\n");
            } else {
                printf("hops away downward.\n");
            }
        } else {
            printf("hops away to the %s.\n", dir_to_text(mot));
        }
    } else if (rabbit->oldloc != loc && rabbit->loc == loc) {
        // Describe a rabbit arriving in the player's current location.
        printf("A fuzzy %s rabbit ", rabbit->color);
        MotionWord mot2 = get_direction_from(loc, rabbit->oldloc);
        assert(mot2 != NOTHING);
        if (mot2 == U) {
            if (has_up_stairs(loc)) {
                printf("hops in from upstairs.\n");
            } else {
                printf("hops in from above.\n");
            }
        } else if (mot2 == D) {
            if (is_overworld(loc) || has_down_stairs(loc)) {
                printf("hops up the stairs");
            } else {
                printf("hops in from below.\n");
            }
        } else {
            printf("hops in from the %s.\n", dir_to_text(mot2));
        }
    }
}

void move_npcs(Location loc)
{
    int rabbits_in_room = 0;
    for (int i=0; i < number_of_rabbits; ++i) {
        rabbits_in_room += (rabbits[i].loc == loc);
    }

    for (int i=0; i < number_of_rabbits; ++i) {
        struct Rabbit *rabbit = &rabbits[i];
        bool should_move = false;
        if (rabbit->oldloc == rabbit->loc) {
            // A rabbit at rest tends to remain at rest.
            should_move = (ran(6) == 0);
        } else {
            should_move = (ran(3) != 0);
        }
        if (should_move) {
            attempt_moving_rabbit(loc, rabbit, rabbits_in_room);
        }
    }
}

/*========== Hints. =======================================================
 */

void advise_about_going_west(const char *word1)
{
    /* Here's a freely offered hint that may save you typing. */
    static int west_count = 0;
    if (strcmp(word1, "west") == 0) {
        ++west_count;
        if (west_count == 10) {
            puts(" If you prefer, simply type W rather than WEST.");
        }
    }
}

/*========== Main loop. ===================================================
 */

void report_inapplicable_motion(MotionWord mot)
{
    switch (mot) {
        case N: case S: case E: case W: case NE: case SE: case NW: case SW:
        case U: case D:
            puts("There is no way to go in that direction.");
            break;
        default:
            puts("I don't know how to apply that word here.");
            break;
    }
}

Location determine_next_newloc(Location loc, MotionWord mot)
{
    const struct Exits exits = get_exits(loc);
    if (exits.go[mot] == NOWHERE) {
        if (is_forested(loc) && is_semicardinal(mot) && lrng_two_in(3, loc, "newloc")) {
            puts("A tangle of branches blocks your way.");
        } else {
            report_inapplicable_motion(mot);
        }
        return loc;
    }
    return exits.go[mot];
}

void print_message(MessageWord msg)
{
    switch (msg) {
        case HELP:
            puts("I know of places, actions, and things. Most of my vocabulary" SOFT_NL
                 "describes places and is used to move you there. To move, try words" SOFT_NL
                 "like forest, building, downstream, enter, east, west, north, south," SOFT_NL
                 "up, or down. I know about a few special objects, like a black rod" SOFT_NL
                 "hidden in the cave. These objects can be manipulated using some of" SOFT_NL
                 "the action words that I know. Usually you will need to give both the" SOFT_NL
                 "object and action words (in either order), but sometimes I can infer" SOFT_NL
                 "the object from the verb alone. Some objects also imply verbs; in" SOFT_NL
                 "particular, \"inventory\" implies \"take inventory\", which causes me to" SOFT_NL
                 "give you a list of what you're carrying. The objects have side" SOFT_NL
                 "effects; for instance, the rod scares the bird. Usually people having" SOFT_NL
                 "trouble moving just need to try a few more words. Usually people" SOFT_NL
                 "trying unsuccessfully to manipulate an object are attempting something" SOFT_NL
                 "beyond their (or my!) capabilities and should try a completely" SOFT_NL
                 "different tack. To speed the game you can sometimes move long" SOFT_NL
                 "distances with a single word. For example, \"building\" usually gets" SOFT_NL
                 "you to the building from anywhere above ground except when lost in the" SOFT_NL
                 "forest. Also, note that cave passages turn a lot, and that leaving a" SOFT_NL
                 "room to the north does not guarantee entering the next from the south." SOFT_NL
                 "Good luck!");
            break;
    }
}

static bool noun_is_valid(Location loc, ObjectWord obj, ActionWord verb)
{
    if (verb == ACCIO) {
        return true;
    } else if (obj == RABBIT) {
        return rabbits_at(loc) != 0;
    } else {
        return here(obj, loc);
    }
}

static ObjectWord single_object_at(Location loc)
{
    ObjectWord result = NOTHING;
    for (ObjectWord obj = MIN_OBJ; obj <= MAX_OBJ; ++obj) {
        if (obj == RABBIT) {
            int r = rabbits_at(loc);
            if (r == 0) continue;
            if (r >= 2 || result != NOTHING) return NOTHING;
            obj = RABBIT;
        } else if (there(obj, loc)) {
            if (result != NOTHING) {
                return NOTHING;
            }
            result = obj;
        }
    }
    return result;
}

void simulate_an_adventure(Location xyz)
{
    Location oldloc = NOWHERE;
    Location loc = NOWHERE;
    Location newloc = xyz;
    MotionWord mot = NOTHING;  /* currently specified motion */
    ActionWord verb = NOTHING;  /* currently specified action */
    ObjectWord obj = NOTHING;  /* currently specified object, if any */

    while (true) {
        // Report the player's environs after movement.
        loc = newloc;
        materialize_objects_if_necessary(loc);
        look_around(loc, verbose_mode);

        move_npcs(loc);

        while (true) {
            int k;
            verb = NOTHING;
            obj = NOTHING;
        cycle:
            listen();  // get one or two words
            ++turns;
        parse:
            advise_about_going_west(word1);
            k = lookup(word1);
            switch (word_class(k)) {
                case WordClass_None:
                    printf("Sorry, I don't know the word \"%s\".\n", word1);
                    verb = NOTHING;
                    obj = NOTHING;
                    goto cycle;
                case WordClass_Motion:
                    mot = k;
                    goto try_move;
                case WordClass_Object:
                    obj = k;
                    if (!noun_is_valid(loc, obj, verb)) {
                        printf("I see no %s here.\n", word1);
                        continue;
                    }

                    if (*word2 != '\0') break;
                    if (verb != NOTHING) {
                        goto transitive;
                    } else {
                        printf("What do you want to do with the %s?\n", word1);
                        goto cycle;
                    }
                case WordClass_Action:
                    verb = k;
                    if (*word2 != '\0') break;
                    if (obj != NOTHING) {
                        goto transitive;
                    } else {
                        goto intransitive;
                    }
                case WordClass_Message:
                    print_message(k);
                    continue;
            }

            shift_words();
            goto parse;

        intransitive:
            switch (verb) {
                case GO:
                    puts("Where?");
                    continue;
                case BACK: {
                    if (oldloc == loc) {
                        puts("Sorry, but I no longer seem to remember how you got here.");
                    } else {
                        mot = get_direction_from(loc, oldloc);
                        if (mot != NOTHING) {
                            goto try_move;
                        } else {
                            puts("You can't get there from here.");
                        }
                    }
                    continue;
                }
                case INVENTORY:
                    attempt_inventory();
                    continue;
                case LOOK: {
                    static int look_count = 0;
                    if (look_count <= 3) {
                        puts("Sorry, but I am not allowed to give more detail.  I will repeat the long description of your location.");
                        ++look_count;
                    }
                    look_around(loc, true);
                    continue;
                }
                case TAKE: {
                    /* TAKE makes sense by itself if there's only one possible thing to take. */
                    obj = single_object_at(loc);
                    if (obj == NOTHING) {
                        goto get_object;
                    } else {
                        goto transitive;
                    }
                }
                case ON: {
                    attempt_on(loc);
                    continue;
                }
                case OFF: {
                    attempt_off(loc);
                    continue;
                }
                case ABRA: {
                    attempt_abra(loc);
                    continue;
                }
                case VERBOSE:
                    verbose_mode = true;
                    puts("Okay, from now on I'll describe places in full every time.");
                    continue;
#ifdef SAVE_AND_RESTORE
                case SAVE:
                    switch (attempt_save()) {
                        case 0: puts("Save failed!"); break;
                        case 1: puts("Saved."); break;
                        case 2: puts("Restored."); break;
                    }
                    continue;
                case RESTORE:
                    /* On the fizmo interpreter, @restore yields 2
                     * when the save file doesn't exist, or when it
                     * has the wrong serial number for this game.
                     * I don't know what return value 0 would mean. */
                    attempt_restore();
                    puts("Restore failed!");
                    continue;
#endif /* SAVE_AND_RESTORE */
                case QUIT:
                    if (yes("Do you really want to quit now?", "OK.", "OK.")) quit();
                    continue;
                default:
                get_object:
                    word1[0] = toupper(word1[0]);
                    printf("%s what?\n", word1);
                    goto cycle;
            }

        transitive:
            switch (verb) {
                case GO:
                    puts("Where?");
                    continue;
                case TAKE:
                    attempt_take(obj, loc);
                    continue;
                case DROP:
                    attempt_drop(obj, loc);
                    continue;
                case ON: case OFF:
                    goto intransitive;
                case WAVE:
                    attempt_wave(obj, loc);
                    continue;
                case ABRA:
                    if (obj == RABBIT) {
                        attempt_abra_rabbit(loc);
                    } else {
                        puts("Eh?");
                    }
                    continue;
                case ACCIO:
                    attempt_accio(obj, loc);
                    continue;
                case LOOK:
                    goto intransitive;
                case INVENTORY:
                case BACK:
                case VERBOSE:
                case QUIT:
#ifdef SAVE_AND_RESTORE
                case SAVE:
                case RESTORE:
#endif /* SAVE_AND_RESTORE */
                    puts("Eh?");
                    continue;
                default:
                    puts("Sorry, I don't know how to do that.");
                    continue;
            }
        }

    try_move:
        /* A major cycle comes to an end when a motion verb mot has been
         * given and we have computed the appropriate newloc accordingly. */
        oldloc = loc;
        newloc = determine_next_newloc(loc, mot);
    }
}

int main(int argc, char **argv)
{
    if (argc == 2) {
        global_seed = atoi(argv[1]);
    }

#ifdef Z_MACHINE
    puts("\n\n\n\n\n\n\n\n");
#endif /* Z_MACHINE */

    build_vocabulary();
    initialize_objects();
    simulate_an_adventure(xyz(10,10,0));
    return 0;
}
