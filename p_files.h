#ifndef G2ME_P_FILES
#define G2ME_P_FILES

#include <stdio.h>
#include <stdbool.h>
/* Local Includes */
#include "entry.h"


#define OUTPUT_TEMP_LEN 24
/* The starting size for a tournament list */
#define TOURN_LIST_START_SIZE 128
/* The number of elements to increment the size of a pr entry array by */
#define TOURN_LIST_REALLOC_INC 4


/* Season file functions */
short s_file_get_latest_season_id(void);
int s_file_set_latest_season_id(int);


/* Read next entry functions */
int p_file_open_read_entry(FILE *, struct entry *);
int p_file_open_read_next_opp_entry(FILE *, struct entry *, short);


/* Read partial entry functions */
int p_file_open_read_entry_minimal(FILE *, struct entry *);
int p_file_open_read_entry_absent(FILE *, struct entry *);
int p_file_open_read_entry_tournament_id(FILE *, struct entry *);


/* Seeking functions */
int p_file_open_position_at_start_of_entries(FILE *);


/* Number of functions */
long p_file_number_of_opponents(char *, short **);
long p_file_number_of_events(char *);
int p_file_get_number_of_entries(char *);


/* Number of outcomes functions */
long p_file_get_number_of_outcomes_against(char *, char *);
long *p_file_get_all_number_of_outcomes_against(char *, long, short *);


long int p_file_get_last_entry_offset(char *);


/* Read [partial] last entry functions */
int p_file_read_last_entry(char *, struct entry *);
int p_file_read_last_entry_minimal(char *, struct entry *);
int p_file_read_last_entry_absent(char *, struct entry *);
int p_file_read_last_entry_tournament_id(char *, struct entry *);
int p_file_open_read_last_entry_tournament_id(FILE *, struct entry *);


/* Append entry to file functions */
int p_file_append_adjustment_to_file_id(struct entry *, char *);
int p_file_append_entry_to_file_id(struct entry *, char *);
int p_file_append_entry_to_file(struct entry *, char *);


/* PR output functions */
int p_file_append_pr_entry_to_file(struct entry *, char *, int, bool);
int p_file_append_pr_entry_to_file_verbose(struct entry *, char *, \
	int, int, int, bool);


/* Read start of file functions */
int p_file_read_start_from_file(char *, struct entry *);
int p_file_open_read_start_from_file(FILE *, struct entry *);


/* Count functions */
int p_file_get_outcome_count(char *);
int p_file_open_get_outcome_count(FILE *);
int p_file_get_events_attended_count(char *);


/* List and count functions */
char *p_file_get_events_attended(char *, int *);


double p_file_get_glicko_change_since_last_event(char *);

#endif
