#pragma once

#define NOTHING 0

typedef enum {
    WordClass_None, WordClass_Motion, WordClass_Object,
    WordClass_Action, WordClass_Message
} WordClass;

typedef enum {
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

void build_vocabulary(void);
WordClass word_class(int word);
int lookup(const char *w);
