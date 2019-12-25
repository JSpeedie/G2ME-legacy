#ifndef G2ME_OPP_FILES
#define G2ME_OPP_FILES

#include <stdio.h>


/* Opp file operations */
int opp_file_contains_opponent(char *);
int opp_file_open_contains_opponent(FILE *, char *);
int opp_file_add_new_opponent(struct entry *);


/* Opp file conversion functions */
int opp_file_get_name_from_id(struct entry *);
int opp_file_get_id_from_name(struct entry *);

#endif
