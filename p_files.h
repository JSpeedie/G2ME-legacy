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


int p_file_initialize(Entry *, const char *);


/* Read next entry functions */
int p_file_open_read_entry(const char *, FILE *, struct entry *);
int p_file_open_read_next_opp_entry(const char *, FILE *, struct entry *, short);


/* Read partial entry functions */
int p_file_open_read_entry_minimal(FILE *, struct entry *);
int p_file_open_read_entry_absent(const char *, FILE *, struct entry *);
int p_file_open_read_entry_tournament_id(FILE *, struct entry *);


/* Seeking functions */
int p_file_open_position_at_start_of_entries(FILE *);


/* Number of functions */
long p_file_number_of_opponents(const char *, const char *, short **);
long p_file_number_of_events(const char *);
int p_file_get_number_of_entries(const char *, const char *);


/* Number of outcomes functions */
long p_file_get_number_of_outcomes_against(const char *, const char *, char *);
long *p_file_get_all_number_of_outcomes_against(const char *, const char *, long, short *);


long int p_file_get_last_entry_offset(const char *);


/* Read [partial] last entry functions */
int p_file_read_last_entry(const char *, const char *, struct entry *);
int p_file_read_last_entry_minimal(const char *, struct entry *);
int p_file_read_last_entry_absent(const char *, const char *, struct entry *);
int p_file_read_last_entry_tournament_id(const char *, struct entry *);
int p_file_open_read_last_entry_tournament_id(FILE *, struct entry *);


/* Append entry to file functions */
int p_file_append_adjustment_to_file_id(struct entry *, const char *);
int p_file_append_entry_to_file_id(struct entry *, const char *);
int p_file_append_entry_to_file(struct entry *, const char *, const char *);


/* PR output functions */
int p_file_append_pr_entry_to_file(struct entry *, char *, int, bool);
int p_file_append_pr_entry_to_file_verbose(struct entry *, char *, \
	int, int, int, bool);


/* Read start of file functions */
int p_file_read_start_from_file(const char *, struct entry *);
int p_file_open_read_start_from_file(FILE *, struct entry *);


/* Count functions */
int p_file_get_outcome_count(const char *);
int p_file_open_get_outcome_count(FILE *);
int p_file_get_events_attended_count(const char *);


/* List and count functions */
char *p_file_get_events_attended(const char *, const char *, int *);


double p_file_get_glicko_change_since_last_event(const char *, const char *);

#endif
