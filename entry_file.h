#ifndef G2ME_ENTRY_FILE
#define G2ME_ENTRY_FILE

#include <stdio.h>

#define MAX_NAME_LEN 256

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

double entry_file_get_glicko_change_since_last_event(char *);
int entry_file_contains_opponent(char *, char *);
int entry_file_add_new_opponent(struct entry *, char *);
int entry_file_contains_tournament(char *, char *);
int entry_file_add_new_tournament(struct entry *, char *);
int entry_get_name_from_id(FILE *, struct entry *);
int entry_file_get_tournament_name_from_id(FILE *, struct entry *);
int entry_file_read_entry(FILE *, struct entry *);
int entry_file_get_to_entries(FILE *);
int entry_file_get_number_of_entries(char *);
int entry_file_number_of_opponents(char *);
int entry_file_number_of_events(char *);
int entry_file_get_number_of_outcomes_against(char *, char *);
long *entry_file_get_all_number_of_outcomes_against(char *);
long int entry_file_get_last_entry_offset(char *);
int entry_file_read_last_entry(char *, struct entry *);
int entry_file_append_entry_to_file(struct entry *, char *);
int entry_file_append_pr_entry_to_file(struct entry *, char *, int);
int entry_file_append_pr_entry_to_file_verbose(\
	struct entry *, char *, int, int, int);
int entry_file_read_start_from_file(char *, struct entry *);
int entry_file_refactor_name(char *);
int entry_file_remove_entry(char *);
int entry_file_get_outcome_count(char *);
char *entry_file_get_events_attended(char *, int *);
double entry_file_get_glicko_change_since_last_event(char*);

#endif
