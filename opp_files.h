#ifndef G2ME_OPP_FILES
#define G2ME_OPP_FILES

#define EXCLUDE_RD_ADJ 1

#include <stdio.h>
/* Local Includes */
#include "entry.h"


/* Opp file operations */
int opp_file_contains_opponent(const char *, const char *);
int opp_file_open_contains_opponent(FILE *, char *);
int opp_file_add_new_opponent(const char *, struct entry *);


/* Opp file conversion functions */
int opp_file_get_name_from_id(const char *, struct entry *);
int opp_file_get_id_from_name(const char *, struct entry *);


/* Opp file statistic functions */
int opp_file_num_opponents(const char *, char);


/* Opp file aggregate data functions */
char *opp_file_get_all_opponent_names(const char *, char, short *);

#endif
