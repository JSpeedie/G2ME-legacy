#ifndef G2ME_G2ME
#define G2ME_G2ME

#define MAX_FILE_PATH_LEN 512
#define REALLOC_PR_ENTRIES_INC 4
#define SIZE_PR_ENTRY 128
#define REALLOC_BRACKET_PATHS_INC 8
#define SIZE_BRACKET_PATHS 128
#define REALLOC_OUTCOMES_INC 16
#define SIZE_OUTCOMES 128
#define LEXIO 1

extern char flag_output_to_stdout;
extern char colour_output;
extern char f_flag_used;
extern char p_flag_used;
extern char verbose;
extern char print_ties;
extern char player_dir[MAX_FILE_PATH_LEN];
extern char data_dir[MAX_FILE_PATH_LEN];
extern int pr_minimum_events;
extern char filter_file_path[MAX_FILE_PATH_LEN];
extern char DIR_TERMINATOR;

#include "entry_file.h"
#include "glicko2.h"

typedef struct record {
	char name[MAX_NAME_LEN];
	char opp_name[MAX_NAME_LEN];
	unsigned short wins;
	unsigned short ties;
	unsigned short losses;
	char last_outcomes[MAX_NAME_LEN];
}Record;


/* Records */
int get_record(char *, char *, struct record *);
struct record *get_all_records(char *, int *);


/* Adjustments */
void adjust_absent_player(char *, char, char, short, char *);
void adjust_absent_players_no_file(char, char, short, char *);
void adjust_absent_players(char *, char, char, short, char *);


void init_player_from_entry(struct player *, struct entry *);
struct entry create_entry(struct player *, char *, char *, \
	char, char, char, char, short, char *, short);



void write_entry_from_input(char *);
void update_player_on_outcome(char *, char *, struct player *, \
	struct player *, char *, char *, char, char, short, char *, short);
int update_players(char *, short);
int run_single_bracket(char *);
int run_brackets(char *);
int generate_ratings_file(char *, char *);
int generate_ratings_file_full(char *);
void num_players_in_player_dir(int *);
unsigned long int longest_name(char *, int);
int filter_player_list(char **, int *, char *);

#endif
