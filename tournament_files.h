#ifndef G2ME_TOURNAMENT_FILES
#define G2ME_TOURNAMENT_FILES

#include <stdio.h>

/* Tournament file operations */
int t_file_contains_tournament(char *);
int t_file_add_new_tournament(struct entry *);


/* Tournament file conversion functions */
int t_file_get_tournament_name_from_id(struct entry *);
int t_file_get_tournament_id_from_name(struct entry *);

#endif
