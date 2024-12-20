#ifndef G2ME_G2ME
#define G2ME_G2ME

#include "p_files.h"
#include "fileops.h"
#include "glicko2.h"

#define MAX_BRACKET_FILE_LINE_LEN 512
/* The number of elements to increment the size of a pr entry array by */
#define REALLOC_PR_ENTRIES_INC 4
/* The starting size for a pr entry array */
#define SIZE_PR_ENTRY 128
#define REALLOC_BRACKET_PATHS_INC 8
#define SIZE_BRACKET_PATHS 128
#define REALLOC_OUTCOMES_INC 16
#define SIZE_OUTCOMES 128
#define LEXIO 1
#define MAX_OUTCOME_STRING_LEN 128
#define REALLOC_TOURNAMENT_NAMES_FACTOR 2
#define SIZE_TOURNAMENT_NAMES_LEN 64
#define MINIMUM_ADJ_BEFORE_FORK 24
#define SIZE_ATTEN_HASHTABLE 128

extern char colour_output;
extern char f_flag_used;
extern char verbose;
extern char print_ties;
extern char filter_file_path[MAX_FILE_PATH_LEN + 1];

typedef struct record {
	char name[MAX_NAME_LEN + 1];
	char opp_name[MAX_NAME_LEN + 1];
	unsigned short wins;
	unsigned short ties;
	unsigned short losses;
	int num_outcomes;
	char *last_outcomes;
}Record;

typedef struct tournament_attendee {
	char name[MAX_NAME_LEN + 1];
	unsigned short id;
}TournamentAttendee;

typedef struct linked_list_node {
	struct entry E;
	struct linked_list_node *next;
}LinkedListNode;


int hashtable_reset();

/* struct record helpers */
int init_record(struct record *);

void update_player_on_outcome(short, char *, short, char *, struct player *, \
	struct player *, char *, char *, char, char, short, short, char *, short);

/* Adjustments */
void adjust_absent_player(char *, char, char, short, short, char *);
void adjust_absent_players_no_file(char, char, short, short, char *, int);

/* Glicko2 number crunching functions */
int update_players(char *, short);
int run_single_bracket(char *);
int run_brackets(char *);

/* Generate Ratings */
int generate_ratings_file(char *, char *);
int generate_ratings_file_full(char *);

/* Records */
int get_record(char *, char *, struct record *);
struct record *get_all_records(char *, long *);

/* Random helper functions */
long longest_name(char *, int);
int filter_player_list(char **, short *, char *);
int filter_player_list_min_events(char **, short *);

#endif
