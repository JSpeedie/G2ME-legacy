#define MAX_NAME_LEN 128
#define MAX_FILE_PATH_LEN 512
#define REALLOC_PR_ENTRIES_INC 4
#define SIZE_PR_ENTRY 128
#define LEXIO 1

extern char flag_output_to_stdout;
extern char colour_output;
extern char f_flag_used;
extern char p_flag_used;
extern char verbose;
extern char print_ties;
extern char player_dir[MAX_FILE_PATH_LEN];
extern int pr_minimum_events;
extern char filter_file_path[MAX_FILE_PATH_LEN];

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

typedef struct record {
	char name[MAX_NAME_LEN];
	char opp_name[MAX_NAME_LEN];
	unsigned short wins;
	unsigned short ties;
	unsigned short losses;
	char last_outcomes[MAX_NAME_LEN];
}Record;




void write_entry_from_input(char *);

/* Player dir stuff */
char *file_path_with_player_dir(char *);
int reset_players(void);
int check_and_create_player_dir(void);
char *players_in_player_dir(char *, int *, char);


/* Records */
int get_record(char *, char *, struct record *);
struct record *get_all_records(char *, int *);


/* Adjustments */
void adjust_absent_player(char *, char, char, short, char *);
void adjust_absent_players_no_file(char, char, short, char *);
void adjust_absent_players(char *, char, char, short, char *);


/* Basic file ops */
int check_if_dir(char *, char *);
void clear_file(char *);


void init_player_from_entry(struct player *, struct entry *);
struct entry create_entry(struct player *, char *, char *, \
	char, char, char, char, short, char *, short);
void update_player_on_outcome(char *, char *, struct player *, \
	struct player *, char *, char *, char, char, short, char *, short);
int update_players(char *, short);
int run_single_bracket(char *);
int run_brackets(char *);
void merge(struct entry *, int, struct entry *, int, struct entry *);
void merge_sort_pr_entry_array(struct entry *, int);
void merge_player_records(struct record *, int, \
	struct record *, int, struct record *);
int generate_ratings_file(char *, char *);
int generate_ratings_file_full(char *);
void merge_sort_player_records(struct record *, int);
void num_players_in_player_dir(int *);
unsigned long int longest_name(char *, int);
int filter_player_list(char **, int *, char *);
