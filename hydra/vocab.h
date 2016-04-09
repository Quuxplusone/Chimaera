#pragma once
#ifndef H_VOCAB
#define H_VOCAB

#define NOTHING 0

typedef enum {
    WordClass_None, WordClass_Motion, WordClass_Object,
    WordClass_Action, WordClass_Message
} WordClass;

typedef enum {
    // It is important for efficiency that MIN_MOTION be 1.
    // It is important for correctness that N,S,E,W,NW,NE,SW,SE be contiguous.
    MIN_MOTION=1,
    N=MIN_MOTION,S,E,W,NW,NE,SW,SE,U,D,
    MAX_MOTION=D
} MotionWord;

typedef enum {
    MIN_OBJ=100,
    LAMP=MIN_OBJ,
    MAX_OBJ=LAMP
} ObjectWord;

typedef enum {
    MIN_ACTION=200,
    GO=MIN_ACTION, BACK, INVENTORY, LOOK, VERBOSE,
#ifdef SAVE_AND_RESTORE
    SAVE, RESTORE,
#endif /* SAVE_AND_RESTORE */
    QUIT,
    MAX_ACTION=QUIT
} ActionWord;

typedef enum {
    MIN_MESSAGE=300,
    ABRA=MIN_MESSAGE, HELP,
    MAX_MESSAGE=HELP
} MessageWord;

int lookup(const char *w);
void build_vocabulary(void);
WordClass word_class(int word);
bool is_semicardinal(MotionWord mot);

#endif // H_VOCAB
