/* Non-windows includes */
#include <dirent.h>
#include <math.h>
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
#include "opp_files.h"
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


/** Takes a pointer to a struct record and returns an integer representing
 * the number of characters needed to print the record.
 *
 * \param '*r' the pointer to the struct record to be examined.
 * \return an integer representing the number of characters needed to print
 *     the record.
 */
int chars_needed_to_print_record(struct record *r) {
	int chars_for_wins;
	if (r->wins > 0) {
		chars_for_wins = ((int) log10((int) r->wins)) + 1;
	} else {
		chars_for_wins = 1;
	}

	int chars_for_ties;
	if (r->ties > 0) {
		chars_for_ties = ((int) log10((int) r->ties)) + 1;
	} else {
		chars_for_ties = 1;
	}

	int chars_for_losses;
	if (r->losses > 0) {
		chars_for_losses = ((int) log10((int) r->losses)) + 1;
	} else {
		chars_for_losses = 1;
	}

	/* 2 chars for the dashes "-", then the number of characters
	 * needed to print the wins, ties and losses (log base 10 of each
	 * value, "ceiled" (+1)) */
	return 2 + chars_for_wins + chars_for_ties + chars_for_losses;
}


/** Takes a pointer to a struct record and returns an integer representing
 * the number of characters needed to print the record, excluding the number
 * of ties.
 *
 * \param '*r' the pointer to the struct record to be examined.
 * \return an integer representing the number of characters needed to print
 *     the record excluding the number of ties.
 */
int chars_needed_to_print_record_no_ties(struct record *r) {
	int chars_for_wins;
	if (r->wins > 0) {
		chars_for_wins = ((int) (log10((int) r->wins))) + 1;
	} else {
		chars_for_wins = 1;
	}

	int chars_for_losses;
	if (r->losses > 0) {
		chars_for_losses = ((int) (log10((int) r->losses))) + 1;
	} else {
		chars_for_losses = 1;
	}

	/* 2 chars for the dashes "-", then the number of characters
	 * needed to print the wins, and losses (log base 10 of each
	 * value, "ceiled" (+1)) */
	return 2 + chars_for_wins + chars_for_losses;
}


/** Takes 3 variables representing the day, month and year of a date,
 * and returns the number of characters needed to print the date, including
 * 2 separator characters inbetween the 3.
 *
 * \param 'day' a char representing the day of the date.
 * \param 'month' a char representing the month of the date.
 * \param 'year' a char representing the year of the date.
 * \return an integer representing the number of characters needed to print
 *     the date including 2 separator characters.
 */
int chars_needed_to_print_date(char day, char month, short year) {
	int chars_for_day;
	if (day > 0) {
		chars_for_day = ((int) (log10((int) day))) + 1;
	} else {
		chars_for_day = 1;
	}

	int chars_for_month;
	if (month > 0) {
		chars_for_month = ((int) (log10((int) month))) + 1;
	} else {
		chars_for_month = 1;
	}

	int chars_for_year;
	if (year > 0) {
		chars_for_year = ((int) (log10((int) year))) + 1;
	} else {
		chars_for_year = 1;
	}

	/* 2 chars for the slashes "/", then the number of characters
	 * needed to print the day, month, and year (log base 10 of each
	 * value, "ceiled" (+1)) */
	return 2 + chars_for_day + chars_for_month + chars_for_year;
}


/** Prints a string representation of a struct player to stdout.
 *
 * \param '*P' the struct player to print
 */
void print_player(struct player *P) {
	fprintf(stdout, "%16.14lf %16.14lf %16.14lf\n", \
		getRating(P), getRd(P), P->vol);
}


/** Prints a string representation of a struct entry to stdout.
 *
 * Difference with verbosity: Now outputs all variables of the
 * 'struct entry' including 'len_name', 'opp_name', and 't_name'.

 * \param 'E' the struct entry to print
 */
void print_entry_verbose(struct entry E) {
	/* Process date data into one string */
	char date[chars_needed_to_print_date(E.day, E.month, E.year) + 1];
	sprintf(date, "%d/%d/%d", E.day, E.month, E.year);
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

	// TODO: fix magic number thing
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
	char date[chars_needed_to_print_date(E.day, E.month, E.year) + 1];
	sprintf(date, "%d/%d/%d", E.day, E.month, E.year);
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

	// TODO: fix magic number thing
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
	char date[chars_needed_to_print_date(E.day, E.month, E.year) + 1];
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
	int longest_RD, int longest_vol, int longest_gc, int longest_opp_gc, \
	int longest_date) {

	/* Process date data into one string */
	char date[chars_needed_to_print_date(E.day, E.month, E.year) + 1];
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
	entry_file_open_get_to_entries(p_file);
	/* Get the longest lengths of the parts of an entry in string form */
	while (entry_file_open_read_entry(p_file, &line) == 0) {
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
	entry_file_open_get_to_entries(p_file);

	while (entry_file_open_read_entry(p_file, &line) == 0) {
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
	entry_file_open_get_to_entries(p_file);
	/* Get the longest lengths of the parts of an entry in string form */
	while (entry_file_open_read_entry(p_file, &line) == 0) {
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
	entry_file_open_get_to_entries(p_file);

	while (entry_file_open_read_entry(p_file, &line) == 0) {
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
	// TODO: add filter to get_all_records, array indexed by opp_id, \
	contains ret array index
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
						player_dir_file_path_with_player_dir( \
							records[i].opp_name);
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
			/* If the user wants ties to be printed */
			if (print_ties == 1) {
				fprintf(stdout, "-%d", records[i].ties);
			}
			fprintf(stdout, "-%d", records[i].losses);
			if (verbose == 1) {
				if (colour_output == 1) {
					fprintf(stdout, " -> ");
					for (int j = 0; records[i].last_outcomes[j] != '\0'; j++) {
						if (records[i].last_outcomes[j] == 'W') {
							fprintf(stdout, "%s%c", GREEN, \
								records[i].last_outcomes[j]);
						} else if (records[i].last_outcomes[j] == 'T') {
							fprintf(stdout, "%s%c", YELLOW, \
								records[i].last_outcomes[j]);
						} else if (records[i].last_outcomes[j] == 'L') {
							fprintf(stdout, "%s%c", RED, \
								records[i].last_outcomes[j]);
						} else if (records[i].last_outcomes[j] == '|') {
							fprintf(stdout, "%s%c", NORMAL, \
								records[i].last_outcomes[j]);
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

	// TODO: free records[i].last_outcomes, free records?
	return 0;
}


/* Prints to stdout all the names in a player name list.
 *
 * \param '*attended' a list of 'count' names, each 'MAX_NAME_LEN + 1'
 *     bytes long, including the null terminator.
 * \param 'count' the number of elements in the array.
 * \return void
 */
void print_player_attended(char *attended, int count) {
	for (int i = 0; i < count; i++) {
		fprintf(stdout, "%s\n", &attended[i * (MAX_NAME_LEN + 1)]);
	}
}


/** Takes no arguments, prints a table to stdout with all the head-to-heads of
 * all players in the system, and returns an integer representing whether it
 * succeeded.
 *
 * \return a negative integer upon failure, and 0 upon success.
 */
int print_matchup_table(void) {
	int space_between_columns = 3;
	/* Get the number of players and the names of all players in the system */
	short num_players;
	char *players = \
		opp_file_get_all_opponent_names(EXCLUDE_RD_ADJ, &num_players);
	if (players == NULL) {
		fprintf(stderr, \
			"opp_file_get_all_opponent_names (print_matchup_table)");
		return -1;
	}

	/* Filter players to be the ones in the given '-f' flag file */
	if (f_flag_used == 1) {
		int ret = 0;
		if (0 != (ret = \
			filter_player_list(&players, &num_players, filter_file_path))) {

			fprintf(stderr, \
				"filter_player_list (%d) (print_matchup_table)", \
				ret);
			return -2;
		}
	}

	/* Filter players so that it keeps only those that pass the  '-m' flag */
	if (pr_minimum_events > 0) {
		int ret = 0;
		if (0 != (ret = \
			filter_player_list_min_events(&players, &num_players))) {

			fprintf(stderr, \
				"filter_player_list_min_events (%d) (print_matchup_table)", \
				ret);
			return -2;
		}
	}

	long longest_n = longest_name(players, num_players);
	int longest_rec[num_players];
	/* Initialize all values to 0, or else they could be anything */
	for (int j = 0; j < num_players; j++) {
		longest_rec[j] = 0;
	}
	struct record records[num_players][num_players];

	/* Fill in record data array. Only does half of the array, since the
	 * record array is always mirrored down the diagonal */
	for (int i = 0; i < num_players; i++) {
		for (int j = i; j < num_players; j++) {

			if (i == j) {
				records[j][i].wins = 0;
				records[j][i].ties = 0;
				records[j][i].losses = 0;
			} else {
				get_record( \
					&players[i * (MAX_NAME_LEN + 1)], \
					&players[j * (MAX_NAME_LEN + 1)], \
					&records[i][j]);
				/* Copy inverse of record to inverse spot */
				records[j][i].wins = records[i][j].losses;
				records[j][i].ties = records[i][j].ties;
				records[j][i].losses = records[i][j].wins;
			}
		}
	}

	for (int j = 0; j < num_players; j++) {
		/* By default set col width to name length */
		int col_width = strlen(&players[(MAX_NAME_LEN + 1) * j]);

		for (int i = 0; i < num_players; i++) {
			int record_length;

			if (print_ties == 1) {
				record_length = chars_needed_to_print_record(&records[i][j]);
			} else {
				record_length = \
					chars_needed_to_print_record_no_ties(&records[i][j]);
			}

			if (col_width < record_length) {
				col_width = record_length;
			}

			/* Save the longest-to-print record length for printing later */
			if (col_width > longest_rec[j]) {
				longest_rec[j] = col_width;
			}
		}
	}

	long width_of_longest_line = longest_n \
		+ (space_between_columns * (num_players + 1));

	/* Calculate width of the longest line */
	for (int j = 0; j < num_players; j++) {
		width_of_longest_line += longest_rec[j];
	}
	/* 'num_players + 1' to accomodate one player per row and an extra row
	 * for the column titles.
	 * width_of_longest_line + 1 to accomodate null term */
	char output[num_players + 1][width_of_longest_line + 1];
	/* Empty the first line of output */
	memset(output[0], 0, width_of_longest_line);

	/* Fill output array with printed data */
	for (int i = 0; i < num_players; i++) {
		/* Add row title. Note that final "" must be empty since it is
		 * not to be printed, the * in %*s is the focus of the statement */
		snprintf(output[i + 1], longest_n + space_between_columns + 1, \
			"%*s%*s", (int) longest_n, &players[i * (MAX_NAME_LEN + 1)], \
			space_between_columns, "");

		/* Get row content */
		for (int j = 0; j < num_players; j++) {

			/* Create the column of minimum length to fit all the characters,
			 * + the gap between columns. +1 to accomodate null term */
			char col[longest_rec[j] + space_between_columns + 1];

			/* If the user wants ties to be printed */
			if (print_ties == 1) {
				 snprintf(col, sizeof(col), "%d-%d-%-*d", \
				 	records[i][j].wins, \
					records[i][j].ties, \
					MAX_NAME_LEN, \
					records[i][j].losses);
			} else {
				 snprintf(col, sizeof(col), "%d-%-*d", \
				 	records[i][j].wins, \
					MAX_NAME_LEN, \
					records[i][j].losses);
			}

			/* If the player has no data against a given opponent, print "-" */
			if (records[i][j].wins == 0 && records[i][j].ties == 0 \
				&& records[i][j].losses == 0) {

				snprintf(col, sizeof(col), "-%*s", \
					longest_rec[j] + space_between_columns - 1, "");
			}
			strcat(output[i + 1], col);
		}
	}

	/* Create buffer in first row/line, first column */
	snprintf(output[0], (int) longest_n + space_between_columns + 1, \
		"%*s", (int) longest_n + space_between_columns, "");

	/* Format column titles (first row) for output */
	for (int j = 0; j < num_players; j++) {
		/* Make column width to be the width of its widest entry. +1 to
		 * accomodate null term. */
		char col[longest_rec[j] + space_between_columns + 1];

		snprintf(col, sizeof(col), "%-*s", \
			longest_rec[j] + space_between_columns, \
			&players[(MAX_NAME_LEN + 1) * j]);
		strcat(output[0], col);
	}

	/* Print all the lines of output */
	for (int i = 0; i < num_players + 1; i++) {
		fprintf(stdout, "%s\n", output[i]);
	}

	return 0;
}


/** Takes no arguments, prints a comma delimited csv of all the head-to-heads
 * of all players in the system, and returns an integer representing whether it
 * succeeded.
 *
 * \return a negative integer upon failure, and 0 upon success.
 */
int print_matchup_table_csv(void) {
	/* Get the number of players and the names of all players in the system */
	short num_players;
	char *players = \
		opp_file_get_all_opponent_names(EXCLUDE_RD_ADJ, &num_players);
	if (players == NULL) {
		fprintf(stderr, \
			"Error: print_matchup_table_csv(): " \
			"(NULL) opp_file_get_all_opponent_names()");
		return -1;
	}

	/* Filter players to be the ones in the given '-f' flag file */
	if (f_flag_used == 1) {
		int ret = 0;
		if (0 != (ret = \
			filter_player_list(&players, &num_players, filter_file_path))) {

			fprintf(stderr, \
				"Error: print_matchup_table_csv(): " \
				"(%d) filter_player_list(): ", \
				ret);
			return -2;
		}
	}

	/* Filter players so that it keeps only those that pass the  '-m' flag */
	if (pr_minimum_events > 0) {
		int ret = 0;
		if (0 != (ret = \
			filter_player_list_min_events(&players, &num_players))) {

			fprintf(stderr, \
				"filter_player_list_min_events (%d) (print_matchup_table)", \
				ret);
			return -2;
		}
	}

	/* num_players + 1 to accomodate one player per row and an extra row
	 * for the column titles */
	char *output[num_players + 1];
	long line_sizes[num_players + 1];
	long line_length[num_players + 1];
	for (int i = 0; i < num_players + 1; i++) {
		line_sizes[i] = 256;
		line_length[i] = 0;
		output[i] = malloc(sizeof(char) * line_sizes[i]);
		output[i][0] = '\0';
	}
	/* Topmost, leftmost cell should be empty */
	sprintf(output[0], ",");
	line_length[0] = 1;

	/* Helper function to add strings to a line of output {{{ */
	int add_to_line(char *s, int i) {
		/* Realloc output line, if necessary */
		long len_of_s = strlen(s);
		if (len_of_s + line_length[i] >= line_sizes[i]) {
			line_sizes[i] *= 2;
			output[i] = (char *)\
				realloc(output[i], sizeof(char) * line_sizes[i]);
			if (output[i] == NULL) {
				perror("realloc (print_matchup_table_csv)");
				return -1;
			}
		}

		line_length[i] += len_of_s;
		strcat(output[i], s);
		return 0;
	}
	/* }}} */

	/* Fill in column titles with player names + a comma delimiter */
	for (int i = 0; i < num_players; i++) {
		add_to_line(&players[i * (MAX_NAME_LEN + 1)], 0);
		add_to_line(",", 0);
	}
	fprintf(stdout, "%s\n", output[0]);

	for (int i = 0; i < num_players; i++) {
		/* Add row title */
		add_to_line(&players[i * (MAX_NAME_LEN + 1)], i + 1);
		add_to_line(",", i + 1);

		/* Get row content */
		for (int j = 0; j < num_players; j++) {
			struct record temp_rec;
			get_record( \
				&players[i * (MAX_NAME_LEN + 1)], \
				&players[j * (MAX_NAME_LEN + 1)], \
				&temp_rec);
			/* If the player has no data against a given opponent, print "-" */
			if (temp_rec.wins == 0 && temp_rec.ties == 0 \
				&& temp_rec.losses == 0) {

				add_to_line("-,", i + 1);
			} else {
				int record_length;

				if (print_ties == 1) {
					record_length = chars_needed_to_print_record(&temp_rec);
				} else {
					record_length = \
						chars_needed_to_print_record_no_ties(&temp_rec);
				}
				/* +1 for the comma, +1 for the null term*/
				char col[record_length + 1 + 1];

				/* If the user wants ties to be printed */
				if (print_ties == 1) {
					 snprintf(col, sizeof(col), "%d-%d-%d,", \
						temp_rec.wins, temp_rec.ties, temp_rec.losses);
				} else {
					 snprintf(col, sizeof(col), "%d-%d,", \
						temp_rec.wins, temp_rec.losses);
				}
				add_to_line(col, i + 1);
			}
		}
		fprintf(stdout, "%s\n", output[i + 1]);
	}

	for (int i = 0; i < num_players + 1; i++) {
		free(output[i]);
	}

	return 0;
}
