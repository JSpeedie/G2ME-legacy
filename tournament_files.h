#ifndef G2ME_TOURNAMENT_FILES
#define G2ME_TOURNAMENT_FILES

#include <stdio.h>

/* Tournament file operations */
int t_file_contains_tournament(const char *, char *);
int t_file_add_new_tournament(const char *, struct entry *);


/* Tournament file conversion functions */
int t_file_get_tournament_name_from_id(const char *, struct entry *);
int t_file_get_tournament_id_from_name(const char *, struct entry *);

/* Season file functions */
short s_file_get_latest_season_id(const char *);
int s_file_set_latest_season_id(const char *, int);

#endif
