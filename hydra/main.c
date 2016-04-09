
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "vocab.h"
#include "rooms.h"

#ifdef Z_MACHINE
#ifdef SAVE_AND_RESTORE
extern int attempt_save(void);
extern int attempt_restore(void);
#endif /* SAVE_AND_RESTORE */

#define SOFT_NL " "
#else
#define SOFT_NL "\n"
#endif /* Z_MACHINE */

static int turns;  // how many times we've read your commands
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

void look_around(Location loc, bool force_verbose)
{
    static Location last_few_locations[10] = {
        NOWHERE, NOWHERE, NOWHERE, NOWHERE, NOWHERE,
        NOWHERE, NOWHERE, NOWHERE, NOWHERE, NOWHERE,
    };

    if (false) {
        puts("It is now pitch dark.  If you proceed you will most likely fall into a pit.");
        return;
    }

    bool be_terse = false;
    if (!force_verbose) {
        for (int i=0; i < 10; ++i) {
            if (last_few_locations[i] == loc) {
                be_terse = true;
                break;
            }
        }
    }

    // Update the list of locations we've recently passed through.
    memmove(last_few_locations+1, last_few_locations, 9 * sizeof last_few_locations[0]);
    last_few_locations[0] = loc;

    if (be_terse) {
        print_short_description(loc);
    } else {
        print_long_description(loc);
    }

    // Here is where we'd describe the scenery and objects at this location.
}

void attempt_inventory()
{
    puts("You are carrying nothing.");
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

MotionWord try_going_back_to(Location to, Location from)
{
    if (to == from) {
        puts("Sorry, but I no longer seem to remember how you got here.");
        return NOTHING;
    }
    const struct Exits exits = get_exits(from);
    for (int mot = MIN_MOTION; mot <= MAX_MOTION; ++mot) {
        if (exits.go[mot] == to) {
            return mot;
        }
    }

    puts("You can't get there from here.");
    return NOTHING;
}

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
        report_inapplicable_motion(mot);
        return loc;
    }
    return exits.go[mot];
}

void print_message(MessageWord msg)
{
    switch (msg) {
        case ABRA:
            puts("Good try, but that is an old worn-out magic word.");
            break;
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
        look_around(loc, verbose_mode);

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
                    goto cycle;
                case WordClass_Motion:
                    mot = k;
                    goto try_move;
                case WordClass_Object:
                    obj = k;

                    // Here is where we'd check noun validity.

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
                case BACK:
                    mot = try_going_back_to(oldloc, loc);
                    if (mot != NOTHING) {
                        goto try_move;
                    }
                    continue;
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
                    word1[0] = toupper(word1[0]);
                    printf("%s what?\n", word1);
                    goto cycle;
            }

        transitive:
            switch (verb) {
                case GO:
                    puts("Where?");
                    continue;
                case QUIT:
#ifdef SAVE_AND_RESTORE
                case SAVE:
                case RESTORE:
#endif /* SAVE_AND_RESTORE */
                    puts("Eh?");
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

int main()
{
#ifdef Z_MACHINE
    puts("\n\n\n\n\n\n\n\n");
#endif /* Z_MACHINE */

    build_vocabulary();
    simulate_an_adventure(xyz(10,10,0));
    return 0;
}
