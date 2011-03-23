#ifndef STABH_
#define STABH_

typedef struct {
    char ** strs;
    int n;
}stab_t;


/** PROTOTYPES from stab.c **/
char * stab_SaveString(stab_t *stab, char *s);
size_t stab_NextEntryNum(stab_t *stab);
void stab_CleanUp(stab_t *stab, size_t bot);
void stab_Mark(stab_t *stab, char mark);
void stab_ShowStrings(stab_t *stab);
int stab_FindString(stab_t *stab, char *s);
int stab_RemoveString(stab_t *stab, char *s);

#endif
