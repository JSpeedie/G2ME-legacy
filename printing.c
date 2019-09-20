/* Non-windows includes */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
/* Windows includes */
#ifdef _WIN32
#include <windows.h>
#endif

#include "player_dir.h"
#include "sorting.h"

char NOTHING[] = { "" };
char NORMAL[] = { "\x1B[0m" };
char RED[] = { "\x1B[31m" };
char GREEN[] = { "\x1B[32m" };
char YELLOW[] = { "\x1B[33m" };
char BLUE[] = { "\x1B[34m" };
char MAGENTA[] = { "\x1B[35m" };
char CYAN[] = { "\x1B[36m" };
char WHITE[] = { "\x1B[37m" };

/** Prints a string representation of a struct player to stdout
 *
 * \param '*P' the struct player to print
 */
void print_player(struct player *P) {
	fprintf(stdout, "%16.14lf %16.14lf %16.14lf\n", \
		getRating(P), getRd(P), P->vol);
}

/** Prints a string representation of a struct entry to stdout
 *
 * Difference with verbosity: Now outputs all variables of the
 * 'struct entry' including 'len_name', 'opp_name', and 't_name'

 * \param 'E' the struct entry to print
 */
void print_entry_verbose(struct entry E) {
	/* Process date data into one string */
	char date[32];
	sprintf(date, "%d/%d/%d", E.day, E.month, E.year);
	// TODO: fix magic number thing
	char *output_colour = NOTHING;
	char *reset_colour = NOTHING;
	if (colour_output == 1) {
		/* If this player won against their opponent,
		 * Set their opponent's name colour to green.
		 * If they lost, to red, and if they tied, to yellow */
		reset_colour = NORMAL;
		output_colour = YELLOW;
		if (E.gc > E.opp_gc) output_colour = GREEN;
		else if (E.gc < E.opp_gc) output_colour = RED;
	}

	fprintf(stdout, "%d  %d  %s  %d  %s%s%s  %d  %.4lf  %.4lf  " \
		"%.8lf  %d-%d  %s  %d  %s\n", \
		E.len_name, E.len_opp_name, E.name, E.opp_id, output_colour, \
		E.opp_name, reset_colour, E.is_competitor, E.rating, E.RD, E.vol, \
		E.gc, E.opp_gc, date, E.tournament_id, E.t_name);
}

/** Prints a string representation of a struct entry to stdout
 *
 * \param 'E' the struct entry to print
 */
void print_entry(struct entry E) {
	/* Process date data into one string */
	char date[32];
	sprintf(date, "%d/%d/%d", E.day, E.month, E.year);
	// TODO: fix magic number thing
	char *output_colour = NOTHING;
	char *reset_colour = NOTHING;
	if (colour_output == 1) {
		/* If this player won against their opponent,
		 * Set their opponent's name colour to green.
		 * If they lost, to red, and if they tied, to yellow */
		reset_colour = NORMAL;
		output_colour = YELLOW;
		if (E.gc > E.opp_gc) output_colour = GREEN;
		else if (E.gc < E.opp_gc) output_colour = RED;
	}

	fprintf(stdout, "%s  %s%s%s  %.1lf  %.1lf  " \
		"%.6lf  %d-%d  %s  %s\n", \
		E.name, output_colour, E.opp_name, reset_colour, E.rating,
		E.RD, E.vol, E.gc, E.opp_gc, date, E.t_name);
}

/** Prints a string representation of a struct entry to stdout
 *
 * Difference with verbosity: Now outputs all variables of the
 * 'struct entry' including 'len_name', 'opp_name', and 't_name'

 * \param 'E' the struct entry to print
 * \param 'longest_name' the length in characters to print the opponent
 *     name in/with.
 */
void print_entry_name_verbose(struct entry E, int longest_nl, \
	int longest_opp_nl, int longest_opp_id, int longest_name, \
	int longest_rating, int longest_RD, int longest_vol, \
	int longest_gc, int longest_opp_gc, int longest_t_id, \
	int longest_date, int longest_t_name, int longest_s_id) {

	/* Process date data into one string */
	char date[32];
	char temp[64];
	sprintf(date, "%d/%d/%d", E.day, E.month, E.year);
	// TODO: fix magic number thing
	sprintf(temp, "%.4lf", E.rating);
	unsigned int rating_length = strlen(temp);
	sprintf(temp, "%.4lf", E.RD);
	unsigned int rd_length = strlen(temp);
	sprintf(temp, "%.8lf", E.vol);
	unsigned int vol_length = strlen(temp);
	sprintf(temp, "%d", E.opp_gc);
	unsigned int opp_gc_length = strlen(temp);
	char *output_colour = NOTHING;
	char *tournament_colour = NOTHING;
	char *reset_colour = NOTHING;
	if (colour_output == 1) {
		/* If this player won against their opponent,
		 * Set their opponent's name colour to green.
		 * If they lost, to red, and if they tied, to yellow */
		reset_colour = NORMAL;
		output_colour = YELLOW;
		if (E.gc > E.opp_gc) output_colour = GREEN;
		else if (E.gc < E.opp_gc) output_colour = RED;
		tournament_colour = RED;
		if (E.is_competitor == 1) tournament_colour = GREEN;
		else output_colour = NORMAL;
	}
	fprintf(stdout, "%*d  %*d  %-*s  %*d  %s%-*s%s  %d  %*s%.4lf  %*s%.4lf  " \
		"%*s%.8lf  %*d-%d%*s  %-*s  %*d  %s%-*s%s %*d\n", \
		longest_nl, E.len_name, longest_opp_nl, E.len_opp_name, \
		E.len_name, E.name, longest_opp_id, E.opp_id, output_colour, \
		longest_name, E.opp_name, reset_colour, E.is_competitor, \
		longest_rating-rating_length, "", E.rating,
		longest_RD-rd_length, "", E.RD, longest_vol-vol_length, "", \
		E.vol, longest_gc, E.gc, E.opp_gc, \
		longest_opp_gc-opp_gc_length, "", longest_date, date, \
		longest_t_id, E.tournament_id, tournament_colour, longest_t_name, \
		E.t_name, reset_colour, longest_s_id, E.season_id);
}

/** Prints a string representation of a struct entry to stdout
 *
 * \param 'E' the struct entry to print
 * \param 'longest_name' the length in characters to print the opponent
 *     name in/with.
 */

void print_entry_name(struct entry E, int longest_name, int longest_rating, \
	int longest_RD, int longest_vol, int longest_gc, int longest_opp_gc, int longest_date) {

	/* Process date data into one string */
	char date[32];
	char temp[64];
	sprintf(date, "%d/%d/%d", E.day, E.month, E.year);
	// TODO: fix magic number thing
	sprintf(temp, "%.1lf", E.rating);
	unsigned int rating_length = strlen(temp);
	sprintf(temp, "%.1lf", E.RD);
	unsigned int rd_length = strlen(temp);
	sprintf(temp, "%.6lf", E.vol);
	unsigned int vol_length = strlen(temp);
	sprintf(temp, "%d", E.opp_gc);
	unsigned int opp_gc_length = strlen(temp);
	char *output_colour = NOTHING;
	char *tournament_colour = NOTHING;
	char *reset_colour = NOTHING;
	if (colour_output == 1) {
		/* If this player won against their opponent,
		 * Set their opponent's name colour to green.
		 * If they lost, to red, and if they tied, to yellow */
		reset_colour = NORMAL;
		output_colour = YELLOW;
		if (E.gc > E.opp_gc) output_colour = GREEN;
		else if (E.gc < E.opp_gc) output_colour = RED;
		tournament_colour = RED;
		if (E.is_competitor == 1) tournament_colour = GREEN;
		else output_colour = NORMAL;
	}

	fprintf(stdout, "%s  %s%-*s%s  %*s%.1lf  %*s%.1lf  %*s%.6lf  %*d-%d%*s  " \
		"%-*s  %s%s%s\n", \
		E.name, output_colour, longest_name, E.opp_name, reset_colour, \
		longest_rating-rating_length, "", E.rating, longest_RD-rd_length, \
		"", E.RD, longest_vol-vol_length, "", E.vol, \
		longest_gc, E.gc, E.opp_gc, longest_opp_gc-opp_gc_length, "", \
		longest_date, date, tournament_colour, E.t_name, reset_colour);
}

/** Prints all the contents of a player file to stdout with each entry
 * on a new line.
 *
 * Difference with verbosity: Now outputs all variables of the
 * 'struct entry' including 'len_name', 'opp_name', and 't_name'
 *
 * \param '*file_path' the file path of the player file to be read
 * \return 0 if the function succeeded and a negative number if there was
 *     a failure.
 */
int print_player_file_verbose(char* file_path) {
	struct entry line;
	entry_file_read_start_from_file(file_path, &line);

	/* Open file for reading */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (print_player_file)");
		return -1;
	}

	unsigned long int longest_name = 0;
	char temp[64];
	memset(temp, 0, sizeof(temp));
	unsigned long int longest_nl = 0;
	unsigned long int longest_opp_nl = 0;
	unsigned long int longest_opp_id = 0;
	unsigned long int longest_rating = 0;
	unsigned long int longest_RD = 0;
	unsigned long int longest_vol = 0;
	unsigned long int longest_gc = 0;
	unsigned long int longest_opp_gc = 0;
	unsigned long int longest_date = 0;
	unsigned long int longest_t_id = 0;
	unsigned long int longest_t_name = 0;
	unsigned long int longest_s_id = 0;

	fseek(p_file, 0, SEEK_SET);
	entry_file_get_to_entries(p_file);
	/* Get the longest lengths of the parts of an entry in string form */
	while (entry_file_read_entry(p_file, &line) == 0) {
		if (strlen(line.opp_name) > longest_name) {
			longest_name = strlen(line.opp_name);
		}

		sprintf(temp, "%d", line.len_name);
		if (strlen(temp) > longest_nl) longest_nl = strlen(temp);
		sprintf(temp, "%d", line.len_opp_name);
		if (strlen(temp) > longest_opp_nl) longest_opp_nl = strlen(temp);
		sprintf(temp, "%d", line.opp_id);
		if (strlen(temp) > longest_opp_id) longest_opp_id = strlen(temp);
		sprintf(temp, "%.4lf", line.rating);
		if (strlen(temp) > longest_rating) longest_rating = strlen(temp);
		sprintf(temp, "%.4lf", line.RD);
		if (strlen(temp) > longest_RD) longest_RD = strlen(temp);
		sprintf(temp, "%.8lf", line.vol);
		if (strlen(temp) > longest_vol) longest_vol = strlen(temp);
		sprintf(temp, "%d/%d/%d", line.day, line.month, line.year);
		if (strlen(temp) > longest_date) longest_date = strlen(temp);
		sprintf(temp, "%d", line.gc);
		if (strlen(temp) > longest_gc) longest_gc = strlen(temp);
		sprintf(temp, "%d", line.opp_gc);
		if (strlen(temp) > longest_opp_gc) longest_opp_gc = strlen(temp);
		sprintf(temp, "%d", line.tournament_id);
		if (strlen(temp) > longest_t_id) longest_t_id = strlen(temp);

		if (strlen(line.t_name) > longest_t_name) {
			longest_t_name = strlen(line.t_name);
		}
		sprintf(temp, "%d", line.season_id);
		if (strlen(temp) > longest_s_id) longest_s_id = strlen(temp);
	}

	fseek(p_file, 0, SEEK_SET);
	entry_file_get_to_entries(p_file);

	while (entry_file_read_entry(p_file, &line) == 0) {
		print_entry_name_verbose(line, longest_nl, longest_opp_nl, \
			longest_opp_id, longest_name, longest_rating, longest_RD, \
			longest_vol, longest_gc, longest_opp_gc, longest_t_id, \
			longest_date, longest_t_name, longest_s_id);
	}

	fclose(p_file);
	return 0;
}

/** Prints all the contents of a player file to stdout with each entry
 * on a new line.
 *
 * \param '*file_path' the file path of the player file to be read
 * \return 0 if the function succeeded and a negative number if there was
 *     a failure.
 */
int print_player_file(char* file_path) {
	struct entry line;
	entry_file_read_start_from_file(file_path, &line);

	/* Open file for reading */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (print_player_file)");
		return -1;
	}

	unsigned long int longest_name = 0;
	char temp[64];
	memset(temp, 0, sizeof(temp));
	unsigned long int longest_nl = 0;
	unsigned long int longest_opp_nl = 0;
	unsigned long int longest_rating = 0;
	unsigned long int longest_RD = 0;
	unsigned long int longest_vol = 0;
	unsigned long int longest_gc = 0;
	unsigned long int longest_opp_gc = 0;
	unsigned long int longest_date = 0;

	fseek(p_file, 0, SEEK_SET);
	entry_file_get_to_entries(p_file);
	/* Get the longest lengths of the parts of an entry in string form */
	while (entry_file_read_entry(p_file, &line) == 0) {
		sprintf(temp, "%d", line.len_name);
		if (strlen(line.opp_name) > longest_name) {
			longest_name = strlen(line.opp_name);
		}
		if (strlen(temp) > longest_nl) longest_nl = strlen(temp);
		sprintf(temp, "%d", line.len_opp_name);
		if (strlen(temp) > longest_opp_nl) longest_opp_nl = strlen(temp);
		sprintf(temp, "%.1lf", line.rating);
		if (strlen(temp) > longest_rating) longest_rating = strlen(temp);
		sprintf(temp, "%.1lf", line.RD);
		if (strlen(temp) > longest_RD) longest_RD = strlen(temp);
		sprintf(temp, "%.6lf", line.vol);
		if (strlen(temp) > longest_vol) longest_vol = strlen(temp);
		sprintf(temp, "%d/%d/%d", line.day, line.month, line.year);
		if (strlen(temp) > longest_date) longest_date = strlen(temp);
		sprintf(temp, "%d", line.gc);
		if (strlen(temp) > longest_gc) longest_gc = strlen(temp);
		sprintf(temp, "%d", line.opp_gc);
		if (strlen(temp) > longest_opp_gc) longest_opp_gc = strlen(temp);
	}

	fseek(p_file, 0, SEEK_SET);
	entry_file_get_to_entries(p_file);

	while (entry_file_read_entry(p_file, &line) == 0) {
		print_entry_name(line, longest_name, longest_rating, longest_RD, \
			longest_vol, longest_gc, longest_opp_gc, longest_date);
	}

	fclose(p_file);
	return 0;
}

/* Takes a file path to an entry-file and prints to 'stdout' that
 * players records against every player they have played, filtered
 * by any applicable filters.
 *
 * \param '*file_path' a file path to a player entry-file.
 * \return an int representing if this function succeeded or failed.
 *     Negative upon failure. 0 upon success.
 */
// TODO: divide function into 3 subfunctions, player_passes_filters,
//       one that gets the array of records, and one that prints the records
// TODO: improve efficiency for checking players pass filters
int print_player_records(char *file_path) {
	/* Get all records and sort them alphabetically */
	long num_rec = 0;
	// TODO: add filter to get_all_records, array indexed by opp_id, contains ret array index
	struct record *records = get_all_records(file_path, &num_rec);
	merge_sort_player_records(records, num_rec);

	char passes_filter = 0;
	char line[MAX_NAME_LEN];

	for (int i = 0; i < num_rec; i++) {
		/* If a minimum event requirement is being used, assume
		 * the player does not pass the filter initially */
		passes_filter = 0;
		/* Filter players to be ones who have attended the minimum number of
		 * events specified by the '-m' flag */
		int attended_count = 0;
		/* If the entry is for a player and not an RD adjustment since
		 * RD adjustments don't have player files */
		// TODO: add is_competitor to records struct
		if (0 != strcmp(records[i].opp_name, "-")) {
			/* If there is no record data */
			if (records[i].wins == 0
				&& records[i].ties == 0
				&& records[i].losses == 0) {
				continue;
			} else {
				/* No need to retrieve attended_count if the minimum
				 * criteria is zero */
				if (pr_minimum_events != 0) {
					char *full_player_path = \
						player_dir_file_path_with_player_dir(records[i].opp_name);
					attended_count = \
						entry_file_get_events_attended_count(full_player_path);
					free(full_player_path);
				}
			}
		} else {
			continue;
		}

		if (attended_count >= pr_minimum_events) {
			passes_filter = 1;
			/* Filter players to be the ones in the given '-f' flag file */
			if (f_flag_used == 1) {
				passes_filter = 0;
				FILE *filter_file = fopen(filter_file_path, "r");
				if (filter_file == NULL) {
					perror("fopen (filter_player_list)");
					return -1;
				}

				while (fgets(line, sizeof(line), filter_file)) {
					// TODO: dangerous line replacement. Fix
					char *end_of_line = strchr(line, '\n');
					if (end_of_line == NULL) {
						perror("strchr (print_player_records)");
						return -2;
					}
					*end_of_line = '\0';
					/* If the name is in the filter, mark it accordingly */
					if (0 == strcmp(records[i].opp_name, line)) {
						passes_filter = 1;
						break;
					}
				}
				fclose(filter_file);
			}
		}

		if (passes_filter == 1) {
			char* output_colour_player = NOTHING;
			char* reset_colour_player = NOTHING;
			if (colour_output == 1) {
				reset_colour_player = NORMAL;
				// If the player has a winning record
				if (records[i].wins > records[i].losses) {
					/* If the player has a "perfect" record, use a
					 * different colour to print it */
					if (records[i].losses == 0) {
						output_colour_player = BLUE;
					} else {
						output_colour_player = GREEN;
					}
				// If the player has a losing record
				} else if (records[i].wins < records[i].losses) {
					/* If the player has a "reverse perfect" record, use a
					 * different colour to print it */
					if (records[i].wins == 0) {
						output_colour_player = MAGENTA;
					} else {
						output_colour_player = RED;
					}
				// If the player has a tied record
				} else {
					output_colour_player = YELLOW;
				}
			}

			fprintf(stdout, "%s vs %s%s%s = %d",
				records[i].name, output_colour_player, records[i].opp_name, \
				reset_colour_player, records[i].wins);
			// If the user wants ties to be printed
			if (print_ties == 1) {
				fprintf(stdout, "-%d", records[i].ties);
			}
			fprintf(stdout, "-%d", records[i].losses);
			if (verbose == 1) {
				if (colour_output == 1) {
					fprintf(stdout, " -> ");
					for (int j = 0; records[i].last_outcomes[j] != '\0'; j++) {
						if (records[i].last_outcomes[j] == 'W') {
							fprintf(stdout, "%s%c", GREEN, records[i].last_outcomes[j]);
						} else if (records[i].last_outcomes[j] == 'T') {
							fprintf(stdout, "%s%c", YELLOW, records[i].last_outcomes[j]);
						} else if (records[i].last_outcomes[j] == 'L') {
							fprintf(stdout, "%s%c", RED, records[i].last_outcomes[j]);
						} else if (records[i].last_outcomes[j] == '|') {
							fprintf(stdout, "%s%c", NORMAL, records[i].last_outcomes[j]);
						} else {
							fprintf(stdout, "%sc", NORMAL);
						}
					}
					fprintf(stdout, "%s", NORMAL);
				} else {
					fprintf(stdout, " -> %s", records[i].last_outcomes);
				}
			}
			fprintf(stdout, "\n");
		}
	}

	return 0;
}

void print_player_attended(char *attended, int count) {
	// Print names of all tournaments attended by the player
	for (int i = 0; i < count; i++) {
		fprintf(stdout, "%s\n", attended + i * MAX_NAME_LEN);
	}
}

void print_matchup_table(void) {
	// Print a table showing the matchup data for all players stored in the
	// system (aka the player directory)
	DIR *p_dir;
	// Get a list of all players tracked by the system to allow for proper
	// column and row titles
	int num_players = 0;
	int space_between_columns = 3;
	/* Get the number of players to allocate the array of player names */
	int max_num_players;
	player_dir_num_players(&max_num_players);
	char *players = (char *)malloc(MAX_NAME_LEN * max_num_players);
	/* Get the names of eligible players to print */
	player_dir_players_list(players, &num_players, LEXIO);

	/* Filter players to be the ones in the given '-f' flag file */
	if (f_flag_used == 1) {
		filter_player_list(&players, &num_players, filter_file_path);
	}

	int longest_n = longest_name(players, num_players);
	// 'num_players + 1' to accomodate one player per row and an extra row
	// for the column titles
	char output[num_players + 1][1024];
	// Empty the first line of output
	memset(output[0], 0, 1024);
	/* Create columns line */
	snprintf(output[0], longest_n + space_between_columns, "%*s", \
		longest_n + space_between_columns, "");
	// Format column titles for output
	for (int i = 0; i < num_players; i++) {
		// Make column width to be the length of the column title (the name)
		int col_width = strlen(&players[i * MAX_NAME_LEN]) \
			+ space_between_columns;
		char col[col_width];
		snprintf(col, col_width, "%-*s", col_width, &players[i * MAX_NAME_LEN]);
		strcat(output[0], col);
	}
	fprintf(stdout, "%s\n", output[0]);

	if ((p_dir = opendir(player_dir)) != NULL) {
		for (int i = 0; i < num_players; i++) {
			// Add row title
			snprintf(output[i + 1], longest_n + space_between_columns, \
				"%*s%*s", longest_n, &players[i * MAX_NAME_LEN], \
				space_between_columns, "");
			for (int j = 0; j < num_players; j++) {
				struct record temp_rec;
				get_record(&players[i * MAX_NAME_LEN], \
					&players[j * MAX_NAME_LEN], &temp_rec);
				// Make column width to be the length of the column title
				// plus a space character on each side
				char col[strlen(&players[j * MAX_NAME_LEN]) \
					+ space_between_columns];
				// If the user wants ties to be printed
				if (print_ties == 1) {
					 snprintf(col, sizeof(col), "%d-%d-%-20d", \
					 	temp_rec.wins, temp_rec.ties, temp_rec.losses);
				} else {
					 snprintf(col, sizeof(col), "%d-%-20d", \
					 	temp_rec.wins, temp_rec.losses);
				}
				// If the player has no data against a given opponent,
				// print "-"
				if (temp_rec.wins == 0 && temp_rec.ties == 0 \
					&& temp_rec.losses == 0) {
					snprintf(col, sizeof(col), "-%-24s", "");
				}
				strcat(output[i + 1], col);
			}
			fprintf(stdout, "%s\n", output[i + 1]);
		}
		closedir(p_dir);
	} else {
		perror("opendir (print_matchup_table)");
		return;
	}
}

// TODO clean up these functions. Less hard numbers and shorter code
void print_matchup_table_csv(void) {
	// Print a table showing the matchup data for all players stored in the
	// system (aka the player directory)
	DIR *p_dir;
	// Get a list of all players tracked by the system to allow for proper
	// column and row titles
	int num_players = 0;
	// TODO: better size allocation
	char *players = (char *)malloc(MAX_NAME_LEN * 128);
	player_dir_players_list(players, &num_players, LEXIO);

	/* Filter players to be the ones in the given '-f' flag file */
	if (f_flag_used == 1) {
		filter_player_list(&players, &num_players, filter_file_path);
	}

	// 'num_players + 1' to accomodate one player per row and an extra row
	// for the column titles
	char output[num_players + 1][1024];
	// Empty the first line of output
	memset(output[0], 0, 1024);
	// Topmost, leftmost cell should be empty
	sprintf(output[0], ",");
	// Fill in column titles with player names + a comma delimiter
	for (int i = 0; i < num_players; i++) {
		strncat(output[0], &players[i * MAX_NAME_LEN], \
			1024 - 1 - strlen(output[0]));
		strncat(output[0], ",", 1024 - 1 - strlen(output[0]));
	}
	fprintf(stdout, "%s\n", output[0]);

	if ((p_dir = opendir(player_dir)) != NULL) {
		for (int i = 0; i < num_players; i++) {
			// Add row title
			sprintf(output[i + 1], "%s,", &players[i * MAX_NAME_LEN]);
			for (int j = 0; j < num_players; j++) {
				struct record temp_rec;
				get_record(&players[i * MAX_NAME_LEN], \
					&players[j * MAX_NAME_LEN], &temp_rec);
				// Make column width to be the length of the column title
				// plus a space character on each side
				// TODO:change to accomodate large records
				char col[30];
				// If the user wants ties to be printed
				if (print_ties == 1) {
					 snprintf(col, sizeof(col), "%d-%d-%d,", \
					 	temp_rec.wins, temp_rec.ties, temp_rec.losses);
				} else {
					 snprintf(col, sizeof(col), "%d-%d,", \
					 	temp_rec.wins, temp_rec.losses);
				}
				// If the player has no data against a given opponent,
				// print "-"
				if (temp_rec.wins == 0 && temp_rec.ties == 0 \
					&& temp_rec.losses == 0) {
					snprintf(col, sizeof(col), "-,");
				}
				strcat(output[i + 1], col);
			}
			fprintf(stdout, "%s\n", output[i + 1]);
		}
		closedir(p_dir);
	} else {
		perror("opendir (print_matchup_table_csv)");
		return;
	}
}
