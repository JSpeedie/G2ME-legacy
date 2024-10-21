#ifndef G2ME_ENTRY_FILE
#define G2ME_ENTRY_FILE

#include <stdio.h>
/* Local Includes */
#include "entry.h"


#define OUTPUT_TEMP_LEN 24


/* Season file functions */
short s_file_get_latest_season_id(void);
int s_file_set_latest_season_id(int);


/* Read next entry functions */
int entry_file_open_read_entry(FILE *, struct entry *);
int entry_file_open_read_next_opp_entry(FILE *, struct entry *, short);


/* Read partial entry functions */
int entry_file_open_read_entry_minimal(FILE *, struct entry *);
int entry_file_open_read_entry_absent(FILE *, struct entry *);
int entry_file_open_read_entry_tournament_id(FILE *, struct entry *);


/* Seeking functions */
int entry_file_open_get_to_entries(FILE *);


/* Number of functions */
long entry_file_number_of_opponents(char *, short **);
long entry_file_number_of_events(char *);
int entry_file_get_number_of_entries(char *);


/* Number of outcomes functions */
long entry_file_get_number_of_outcomes_against(char *, char *);
long *entry_file_get_all_number_of_outcomes_against(char *, long, short *);


long int entry_file_get_last_entry_offset(char *);


/* Read [partial] last entry functions */
int entry_file_read_last_entry(char *, struct entry *);
int entry_file_read_last_entry_minimal(char *, struct entry *);
int entry_file_read_last_entry_absent(char *, struct entry *);
int entry_file_read_last_entry_tournament_id(char *, struct entry *);
int entry_file_open_read_last_entry_tournament_id(FILE *, struct entry *);


/* Append entry to file functions */
int entry_file_append_adjustment_to_file_id(struct entry *, char *);
int entry_file_append_entry_to_file_id(struct entry *, char *);
int entry_file_append_entry_to_file(struct entry *, char *);


/* PR output functions */
int entry_file_append_pr_entry_to_file(struct entry *, char *, int);
int entry_file_append_pr_entry_to_file_verbose(struct entry *, char *, \
	int, int, int);


/* Read start of file functions */
int entry_file_read_start_from_file(char *, struct entry *);
int entry_file_open_read_start_from_file(FILE *, struct entry *);


/* Count functions */
int entry_file_get_outcome_count(char *);
int entry_file_open_get_outcome_count(FILE *);
int entry_file_get_events_attended_count(char *);


/* List and count functions */
char *entry_file_get_events_attended(char *, int *);


double entry_file_get_glicko_change_since_last_event(char *);

#endif
