extern char NOTHING[];
extern char NORMAL[];
extern char RED[];
extern char GREEN[];
extern char YELLOW[];
extern char BLUE[];
extern char MAGENTA[];
extern char CYAN[];
extern char WHITE[];

#include "G2ME.h"


/* Space-needed for printing record functions */
int chars_needed_to_print_record(struct record *);
int chars_needed_to_print_record_no_ties(struct record *);


void print_player(struct player *);
void print_entry_verbose(struct entry);
void print_entry(struct entry);
void print_entry_name_verbose(struct entry, int, \
	int, int, int, int, int, int, int, int, int, int, int, int);
void print_entry_name(struct entry, int, int, int, int, int, int, int);
int print_player_file_verbose(char *);
int print_player_file(char *);
int print_player_records(char *);
void print_player_attended(char *, int);
int print_matchup_table(void);
int print_matchup_table_csv(void);
