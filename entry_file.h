#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "G2ME.h"

char *entry_file_get_events_attended(char *, int *);
int entry_file_get_outcome_count(char *);
double entry_file_get_glicko_change_since_last_event(char *);
int entry_file_read_start_from_file(char *, struct entry *);
int entry_file_contains_opponent(char *, char *);
int entry_file_add_new_opponent(struct entry *, char *);
int entry_file_contains_tournament(char *, char *);
int entry_file_add_new_tournament(struct entry *, char *);
int entry_get_name_from_id(FILE *, struct entry *);
int entry_file_get_tournament_name_from_id(FILE *, struct entry *);
int entry_file_read_entry(FILE *, struct entry *);
int entry_file_get_to_entries(FILE *);
int entry_file_get_number_of_entries(char *);
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
