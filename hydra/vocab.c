
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "vocab.h"

struct HashEntry {
    char text[6];
    int meaning;
};

#define HASH_PRIME 1009
struct HashEntry hash_table[HASH_PRIME];

static void new_word(const char *w, int m)
{
    int h = 0;
    for (const char *p = w; *p != '\0'; ++p) {
        h = ((h << 1) + *p) % HASH_PRIME;
    }
    while (hash_table[h].meaning != 0)
        h = (h+1) % HASH_PRIME;
    strcpy(hash_table[h].text, w);
    hash_table[h].meaning = m;
}

int lookup(const char *w)
{
    int h = 0;
    for (const char *p = w; *p != '\0' && p != w+5; ++p) {
        h = ((h << 1) + *p) % HASH_PRIME;
    }
    while (hash_table[h].meaning != 0) {
        if (strncmp(w, hash_table[h].text, 5) == 0) return hash_table[h].meaning;
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
    new_word("nw", NW);
    new_word("ne", NE);
    new_word("sw", SW);
    new_word("se", SE);
    new_word("upwar", U); new_word("up", U);
    new_word("u", U); new_word("above", U);
    new_word("ascen", U);
    new_word("d", D); new_word("downw", D);
    new_word("down", D); new_word("desce", D);

    new_word("walk", GO); new_word("run", GO);
    new_word("trave", GO); new_word("go", GO);
    new_word("proce", GO); new_word("explo", GO);
    new_word("goto", GO); new_word("follo", GO);
    new_word("turn", GO);
    new_word("back", BACK); new_word("retur", BACK);
    new_word("retre", BACK);
    new_word("inven", INVENTORY);
    new_word("look", LOOK); new_word("exami", LOOK);
    new_word("touch", LOOK); new_word("descr", LOOK);
    new_word("verbo", VERBOSE);
#ifdef SAVE_AND_RESTORE
    new_word("save", SAVE);
    new_word("resto", RESTORE);
#endif /* SAVE_AND_RESTORE */
    new_word("quit", QUIT);

    /* Finally, our vocabulary is rounded out by words like HELP, which
     * trigger the printing of fixed messages. */
    new_word("abra", ABRA);
    new_word("abrac", ABRA);
    new_word("opens", ABRA);
    new_word("sesam", ABRA);
    new_word("shaza", ABRA);
    new_word("hocus", ABRA);
    new_word("pocus", ABRA);
    new_word("xyzzy", ABRA);
    new_word("plugh", ABRA);
    new_word("plove", ABRA);
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
