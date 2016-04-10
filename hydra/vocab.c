
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "vocab.h"

struct HashEntry {
    char text[7];
    int meaning;
};

#define HASH_PRIME 1009
struct HashEntry hash_table[HASH_PRIME];

static void new_word(const char *w, int m)
{
    int h = 0;
    for (const char *p = w; *p != '\0' && p != w+5; ++p) {
        h = ((h << 1) + *p) % HASH_PRIME;
    }
    while (hash_table[h].meaning != 0)
        h = (h+1) % HASH_PRIME;
    strcpy(hash_table[h].text, w);
    hash_table[h].meaning = m;
}

static inline bool streq(const char *input, const char *pattern)
{
    if (strlen(input) <= 5) {
        return !strncmp(input, pattern, 5);
    } else {
        return !strncmp(input, pattern, 6);  // handle NORTHWEST
    }
}

int lookup(const char *w)
{
    int h = 0;
    for (const char *p = w; *p != '\0' && p != w+5; ++p) {
        h = ((h << 1) + *p) % HASH_PRIME;
    }
    while (hash_table[h].meaning != 0) {
        if (streq(w, hash_table[h].text)) return hash_table[h].meaning;
        h = (h+1) % HASH_PRIME;
    }
    return 0;
}
#undef HASH_PRIME

void build_vocabulary(void)
{
    new_word("north", N); new_word("n", N);
    new_word("south", S); new_word("s", S);
    new_word("east", E); new_word("e", E);
    new_word("west", W); new_word("w", W);
    new_word("northw", NW); new_word("nw", NW);
    new_word("northe", NE); new_word("ne", NE);
    new_word("southw", SW); new_word("sw", SW);
    new_word("southe", SE); new_word("se", SE);
    new_word("upward", U); new_word("up", U);
    new_word("u", U); new_word("above", U);
    new_word("ascend", U);
    new_word("d", D); new_word("downwa", D);
    new_word("down", D); new_word("descen", D);

    new_word("walk", GO); new_word("run", GO);
    new_word("travel", GO); new_word("go", GO);
    new_word("procee", GO); new_word("explor", GO);
    new_word("follow", GO); new_word("turn", GO);
    new_word("back", BACK); new_word("return", BACK);
    new_word("retrea", BACK);
    new_word("invent", INVENTORY);
    new_word("look", LOOK); new_word("examin", LOOK);
    new_word("touch", LOOK); new_word("descri", LOOK);
    new_word("abra", ABRA);
    new_word("abraca", ABRA);
    new_word("opense", ABRA);
    new_word("sesame", ABRA);
    new_word("shazam", ABRA);
    new_word("hocus", ABRA);
    new_word("pocus", ABRA);
    new_word("xyzzy", ABRA);
    new_word("plugh", ABRA);
    new_word("plover", ABRA);
    new_word("verbos", VERBOSE);
#ifdef SAVE_AND_RESTORE
    new_word("save", SAVE);
    new_word("restor", RESTORE);
#endif /* SAVE_AND_RESTORE */
    new_word("quit", QUIT);

    new_word("rabbit", RABBIT); new_word("fuzzy", RABBIT);
    new_word("bunny", RABBIT); new_word("bunnie", RABBIT);

    /* Finally, our vocabulary is rounded out by words like HELP, which
     * trigger the printing of fixed messages. */
    new_word("help", HELP);
    new_word("?", HELP);
}

WordClass word_class(int word)
{
    if (word == NOTHING) {
        return WordClass_None;
    } else if (MIN_MOTION <= word && word <= MAX_MOTION) {
        return WordClass_Motion;
    } else if (MIN_OBJ <= word && word <= MAX_OBJ) {
        return WordClass_Object;
    } else if (MIN_ACTION <= word && word <= MAX_ACTION) {
        return WordClass_Action;
    } else if (MIN_MESSAGE <= word && word <= MAX_MESSAGE) {
        return WordClass_Message;
    } else {
        assert(false);
        return WordClass_None;
    }
}

bool is_semicardinal(MotionWord mot)
{
    return N <= mot && mot <= SE;
}

const char *dir_to_text(MotionWord mot)
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
