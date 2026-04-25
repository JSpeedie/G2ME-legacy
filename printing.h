#ifndef __PRINTING_H_
#define __PRINTING_H_

extern char NOTHING[];
extern char NORMAL[];
extern char RED[];
extern char GREEN[];
extern char YELLOW[];
extern char BLUE[];
extern char MAGENTA[];
extern char CYAN[];
extern char WHITE[];

#include <stdbool.h>

#include "G2ME.h"


/* Space-needed for printing record functions */
int chars_needed_to_print_record(struct record *);
int chars_needed_to_print_record_no_ties(struct record *);

void print_player(struct player *);

void print_entry(struct entry E, bool colour_output, bool verbose);

void print_entry_column_spaced_verbose(struct entry, int, \
	int, int, int, int, int, int, int, int, int, int, int, int, bool);
void print_entry_column_spaced(struct entry, int, int, int, int, int, int, int, \
	bool);

int print_player_file_verbose(g2me_flags_t *flags, g2me_data_t *data, \
	const char *file_path);
int print_player_file(g2me_flags_t *flags, g2me_data_t *data, \
	const char *file_path);

int print_player_records(g2me_flags_t *flags, g2me_data_t *data, \
	const char *file_path);

void print_player_attended(char *, int);

int print_matchup_table(g2me_flags_t *flags, g2me_data_t *data);
int print_matchup_table_csv(g2me_flags_t *flags, g2me_data_t *data);

#endif
