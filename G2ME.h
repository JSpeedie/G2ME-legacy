#ifndef __G2ME_H_
#define __G2ME_H_

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


typedef struct g2me_flags {
	/* If true, the program will output to `stdout` rather than to a file */
	bool output_to_stdout;
	bool silent;
	bool silent_all;
	bool verbose;
	bool use_games;
	bool keep_players;
	/* A filter that removes players who have not participated in at least
	 * `min_events` events. */
	int min_events;
	bool colour_output;
	bool print_ties;
	bool calc_absent_players;
	double outcome_weight;
	/* A filter that removes players whose name does NOT appear as a one-line
	 * entry in the filter file */
	bool filter_by_filter_file;
	char filter_file_path[MAX_FILE_PATH_LEN + 1];
}g2me_flags_t;

/* This struct keeps track of all the stored data the program is to make use
 * of. */
typedef struct g2me_data {
	char player_dir[MAX_FILE_PATH_LEN + 1];
	char data_dir[MAX_FILE_PATH_LEN + 1];
}g2me_data_t;

typedef struct g2me_state {
	g2me_flags_t flags;
	g2me_data_t data;
}g2me_state_t;

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

void update_player_on_outcome(g2me_flags_t *flags, g2me_data_t *data, \
	short p1_id, char* p1_name, short p2_id, char* p2_name, \
	struct player* p1, struct player* p2, char* p1_gc, char* p2_gc, char day, \
	char month, short year, short t_id, char* t_name, short season_id);

/* Adjustments */
void adjust_absent_player(g2me_data_t *data, char *player_file, char day, \
	char month, short year, short t_id, char *t_name);
void adjust_absent_players_no_file(g2me_data_t *data, char day, char month, \
	short year, short t_id, char* t_name, int available_cores);

/* Glicko2 number crunching functions */
int update_players(g2me_flags_t *flags, g2me_data_t *data, \
	char *bracket_file_path, short season_id);
int run_single_bracket(g2me_flags_t *flags, g2me_data_t *data, \
	char *bracket_file_path);
int run_brackets(g2me_flags_t *flags, g2me_data_t *data, char *);

/* Generate Ratings */
int generate_ratings_file(g2me_flags_t *flags, g2me_data_t *data, \
	char *output_file_path);
int generate_ratings_file_full(g2me_flags_t *flags, g2me_data_t *data, \
	char *output_file_path);

/* Records */
int get_record(g2me_data_t *data, char *player1, char *player2, \
	struct record *ret);
struct record *get_all_records(g2me_data_t *data, const char *file_path, \
	long *num_of_records);

/* Random helper functions */
long longest_name(char *, int);
int filter_player_list(char **, short *, const char *);
int filter_player_list_min_events(g2me_data_t *data, char **, short *, \
	int min_events);

#endif
