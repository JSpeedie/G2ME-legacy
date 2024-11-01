/* General Includes */
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Windows includes */
#ifdef _WIN32
#include <io.h>
//#include <windows.h>
//#include <fcntl.h>
#endif
/* Local Includes */
#include "player_dir.h"
#include "p_files.h"

/** Appends a pr entry (the name and glicko2 data for a player) to a given
 * file. Returns an int representing success.
 *
 * \param '*E' the struct entry to append to the pr file.
 * \param '*file_path' the file path for the pr file.
 * \return 0 upon success, negative number on failure.
 */
int append_pr_entry_to_file(struct entry *E, char *file_path, \
	int longest_name_length, bool output_to_stdout) {

	FILE *p_file;

	if (!output_to_stdout) {
		p_file = fopen(file_path, "a+");
		if (p_file == NULL) {
			perror("fopen (p_file_append_pr_entry_to_file)");
			return -1;
		}
	} else {
		p_file = stdout;
	}

	if (fprintf(p_file, "%*s  %6.1lf  %5.1lf  %10.8lf\n", \
		longest_name_length, E->name, E->rating, E->RD, E->vol) < 0) {

		perror("fprintf");
		return -3;
	}

	if (!output_to_stdout) {
		fclose(p_file);
	}
	return 0;
}


/** Appends a verboe pr entry (the name and glicko2 data for a player) to a
 * given file. Returns an int representing success.
 *
 * Difference with verbosity: Adds 3 columns: Events attended,
 * outcomes gone through, and glicko rating change since last event.
 * Changes with verbosity: Adds more decimals to output of glicko variables.
 *
 * \param '*E' the struct entry to append to the pr file.
 * \param '*file_path' the file path for the pr file.
 * \return 0 upon success, negative number on failure.
 */
int append_pr_entry_to_file_verbose(const char *player_dir, \
	const char *data_dir_file_path, struct entry *E, char *file_path, \
	int longest_name_length, int longest_attended_count, \
	int longest_outcome_count, bool output_to_stdout) {

	FILE *p_file;

	if (!output_to_stdout) {
		p_file = fopen(file_path, "a+");
		if (p_file == NULL) {
			perror("fopen (p_file_append_pr_entry_to_file_verbose)");
			return -1;
		}
	} else {
		p_file = stdout;
	}

	char *full_player_path = player_dir_file_path_with_player_dir(player_dir, E->name);
	int attended_count = p_file_get_events_attended_count(full_player_path);
	int outcome_count = p_file_get_outcome_count(full_player_path);
	double glicko_change = \
		p_file_get_glicko_change_since_last_event(data_dir_file_path, \
			full_player_path);
	free(full_player_path);
	char temp[OUTPUT_TEMP_LEN];
	// TODO: fix magic number thing
	sprintf(temp, "%4.3lf", E->rating);
	unsigned int rating_length = strlen(temp);
	sprintf(temp, "%3.3lf", E->RD);
	unsigned int rd_length = strlen(temp);

	/* <= 0.0001 to handle double rounding errors */
	if (fabs(glicko_change) <= 0.0001) {
		/* If unable to write to the file */
		// TODO: remove magic numbers 7 and 6
		if (fprintf(p_file, "%*s  %*s%4.3lf  %*s%3.3lf  %10.8lf  %*d  %*d\n", \
			longest_name_length, E->name, 8-rating_length, "", \
			E->rating, 7-rd_length, "", E->RD, E->vol, \
			longest_attended_count, attended_count, \
			longest_outcome_count, outcome_count) < 0) {

			perror("fprintf (append_pr_entry_to_file_verbose)");
			return -2;
		}
	} else {
		/* If unable to write to the file */
		if (fprintf(p_file, "%*s  %*s%4.3lf  %*s%3.3lf  %10.8lf  %*d  %*d  %+5.1lf\n", \
			longest_name_length, E->name, 8-rating_length, "",
			E->rating, 7-rd_length, "", E->RD, E->vol, \
			longest_attended_count, attended_count, \
			longest_outcome_count, outcome_count, \
			glicko_change) < 0) {

			perror("fprintf (p_file_append_pr_entry_to_file_verbose)");
			return -3;
		}
	}

	if (!output_to_stdout) {
		fclose(p_file);
	}
	return 0;
}
