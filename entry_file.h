#ifndef G2ME_ENTRY_FILE
#define G2ME_ENTRY_FILE

#include <stdio.h>

#define MAX_NAME_LEN 64
#define OUTPUT_TEMP_LEN 24

typedef struct entry {
	unsigned short opp_id;
	unsigned short tournament_id;
	unsigned short season_id;
	unsigned char len_name;
	unsigned char len_opp_name;
	char name[MAX_NAME_LEN];
	char opp_name[MAX_NAME_LEN];
	double rating;
	double RD;
	double vol;
	signed char gc;
	signed char opp_gc;
	unsigned char is_competitor;
	unsigned char day;
	unsigned char month;
	unsigned short year;
	unsigned char len_t_name;
	char t_name[MAX_NAME_LEN];
}Entry;


/* Opp file operations */
int opp_file_contains_opponent(char *);
int opp_file_add_new_opponent(struct entry *);


/* Tournament file operations */
int t_file_contains_tournament(char *);
int t_file_add_new_tournament(struct entry *);


/* Opp file conversion functions */
int opp_file_get_name_from_id(struct entry *);
int opp_file_get_id_from_name(struct entry *);


/* Tournament file conversion functions */
int t_file_get_tournament_name_from_id(struct entry *);
int t_file_get_tournament_id_from_name(struct entry *);


/* Season file functions */
int s_file_get_latest_season_id(void);
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
