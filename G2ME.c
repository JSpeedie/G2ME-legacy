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

#include "glicko2.h"
#include "entry_file.h"

char *NORMAL = "\x1B[0m";
char *RED = "\x1B[31m";
char *GREEN = "\x1B[32m";
char *YELLOW = "\x1B[33m";
char *BLUE = "\x1B[34m";
char *MAGENTA = "\x1B[35m";
char *CYAN = "\x1B[36m";
char *WHITE = "\x1B[37m";
char *PLAYER_DIR = ".players/";

char verbose = 0;
char use_games = 0;
char keep_players = 0;
int pr_minimum_events = 0;
char colour_output = 1;
char print_ties = 1;
char player_list_file[MAX_FILE_PATH_LEN];
char calc_absent_players = 1;
char calc_absent_players_with_file = 0;
double outcome_weight = 1;
/* TODO: make it dynamically realloc */
char tournament_names[128][128];
unsigned char tournament_names_len = 0;
char pr_list_file_path[MAX_FILE_PATH_LEN];
char o_generate_pr = 0;
char player_dir[MAX_FILE_PATH_LEN];

struct entry temp;

char *file_path_with_player_dir(char *s) {
	int new_path_size = sizeof(char) * (MAX_FILE_PATH_LEN - MAX_NAME_LEN);
	char *new_path = malloc(new_path_size);
	/* 0 all elements in the new string */
	memset(new_path, 0, new_path_size);

	/* Copy the player directory file path into the return string */
	strncpy(new_path, player_dir, new_path_size - 1);
	/* If the last character in the player directory path is not a '/' */
	if (new_path[strlen(new_path) - 1] != '/') {
		/* Append a '/' */
		new_path[strlen(new_path)] = '/';
	}

	/* Append the player file to the player dir file path */
	strncat(new_path, s, new_path_size - strlen(new_path) - 1);

	return new_path;
}

/** Writes nothing to a file, clearing it.
 *
 * \param '*file' the file path for the file to be wiped
 * \return void
 */
void clear_file(char* file) {

	FILE* victim = fopen(file, "w");
	if (victim == NULL) {
		perror("fopen (clear_file)");
		return;
	}

	fprintf(victim, "");
	fclose(victim);
	return;
}

/** Appends an entry created from user input to a file.
 *
 * \param '*file_path' the file path that you want to append the entry to
 * \return void
 */
void write_entry_from_input(char* file_path) {
	printf("[name] [opp_name] [rating] [RD] [vol] [gc] [opp_gc] [day] [month] [year] [t_name]: ");
	char *full_path = file_path_with_player_dir(file_path);

	struct entry input_entry;
	scanf("%s %s %lf %lf %lf %hhd %hhd %hhd %hhd %hd %s",
		input_entry.name, input_entry.opp_name, &input_entry.rating,
		&input_entry.RD, &input_entry.vol, &input_entry.gc,
		&input_entry.opp_gc, &input_entry.day, &input_entry.month,
		&input_entry.year, input_entry.t_name);
	input_entry.len_name = strlen(input_entry.name);
	input_entry.len_opp_name = strlen(input_entry.opp_name);
	input_entry.len_t_name = strlen(input_entry.t_name);
	entry_file_append_entry_to_file(&input_entry, file_path);
	free(full_path);
}

/** Prints a string representation of a struct player to stdout
 *
 * \param '*P' the struct player to print
 */
void print_player(struct player *P) {
	printf("%16.14lf %16.14lf %16.14lf\n", getRating(P), getRd(P), P->vol);
}

/** Prints a string representation of a struct entry to stdout
 *
 * \param 'E' the struct entry to print
 */
void print_entry(struct entry E) {
	/* Process date data into one string */
	char date[32];
	sprintf(date, "%d/%d/%d", E.day, E.month, E.year);

	printf("%d  %d  %-10s  %-10s  %16.14lf  %16.14lf  %16.14lf" \
		"%d-%d  %s  %d  %-10s\n",  \
		E.len_name, E.len_opp_name, E.name, E.opp_name, E.rating, E.RD, \
		E.vol, E.gc, E.opp_gc, date, E.len_t_name, E.t_name);
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
	int longest_rating, int longest_RD, int longest_vol, int longest_date) {

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
	char *output_colour = NORMAL;
	if (colour_output == 1) {
		/* If this player won against their opponent,
		 * Set their opponent's name colour to green.
		 * If they lost, to red, and if they tied, to yellow */
		output_colour = YELLOW;
		if (E.gc > E.opp_gc) output_colour = GREEN;
		else if (E.gc < E.opp_gc) output_colour = RED;
	}

	printf("%*d  %*d  %-*s  %*d  %s%-*s%s  %*s%.4lf  %*s%.4lf  %*s%.8lf  %d-%d" \
		"  %-*s  %d  %s\n", \
		longest_nl, E.len_name, longest_opp_nl, E.len_opp_name, \
		E.len_name, E.name, longest_opp_id, E.opp_id, output_colour, \
		longest_name, E.opp_name, NORMAL, longest_rating-rating_length, "", \
		E.rating, longest_RD-rd_length, "", E.RD, longest_vol-vol_length, "", \
		E.vol, E.gc, E.opp_gc, longest_date, date, E.tournament_id, E.t_name);
}
/** Prints a string representation of a struct entry to stdout
 *
 * \param 'E' the struct entry to print
 * \param 'longest_name' the length in characters to print the opponent
 *     name in/with.
 */

void print_entry_name(struct entry E, int longest_name, int longest_rating, \
	int longest_RD, int longest_vol, int longest_date) {

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
	char *output_colour = NORMAL;
	if (colour_output == 1) {
		/* If this player won against their opponent,
		 * Set their opponent's name colour to green.
		 * If they lost, to red, and if they tied, to yellow */
		output_colour = YELLOW;
		if (E.gc > E.opp_gc) output_colour = GREEN;
		else if (E.gc < E.opp_gc) output_colour = RED;
	}

	printf("%s  %s%-*s%s  %*s%.1lf  %*s%.1lf  %*s%.6lf  %d-%d  %-*s  %s\n", \
		E.name, output_colour, longest_name, E.opp_name, NORMAL, \
		longest_rating-rating_length, "", E.rating, longest_RD-rd_length, \
		"", E.RD, longest_vol-vol_length, "", E.vol, E.gc, E.opp_gc, \
		longest_date, date, E.t_name);
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
	unsigned long int longest_date = 0;

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
	}

	fseek(p_file, 0, SEEK_SET);
	entry_file_get_to_entries(p_file);

	while (entry_file_read_entry(p_file, &line) == 0) {
		print_entry_name_verbose(line, longest_nl, longest_opp_nl, \
		longest_opp_id, longest_name, longest_rating, longest_RD, longest_vol, \
		longest_date);
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
	}

	fseek(p_file, 0, SEEK_SET);
	entry_file_get_to_entries(p_file);

	while (entry_file_read_entry(p_file, &line) == 0) {
		print_entry_name(line, longest_name, longest_rating, longest_RD, \
			longest_vol, longest_date);
	}

	fclose(p_file);
	return 0;
}

/** Initializes a struct player based off of the information found in a
 * struct entry.
 *
 * \param '*P' the struct player to be initialized
 * \param '*E' the struct entry from which to get the data to initialize the
 *     struct player
 * \return void
 */
void init_player_from_entry(struct player* P, struct entry* E) {
	setRating(P, E->rating);
	setRd(P, E->RD);
	P->vol = E->vol;
	return;
}

/** Creates a struct entry which contains all the important data about a set
 *
 * \param '*P' a struct player that represents player-1
 * \param '*name' the name of player-1
 * \param '*opp_name' the name of player-2
 * \param 'gc' the number of games won by player-1 in the set
 * \param 'opp_gc' the number of games won by player-2 in the set
 * \param 'day' a char representing the day of the month the set was played on
 * \param 'month' a char representing the month the set was played in
 * \param 'year' a short representing the year the set was played in
 * \param 't_name' a string containing the name of the tournament this
 *     outcome took place at.
 * \return a struct entry containing all that information
 */
struct entry create_entry(struct player* P, char* name, char* opp_name,
	char gc, char opp_gc, char day, char month, short year, char* t_name) {

	struct entry ret;
	ret.len_name = strlen(name);
	ret.len_opp_name = strlen(opp_name);
	strncpy(ret.name, name, sizeof(ret.name));
	strncpy(ret.opp_name, opp_name, sizeof(ret.opp_name));
	ret.rating = getRating(P);
	ret.RD = getRd(P);
	ret.vol = P->vol;
	ret.gc = gc;
	ret.opp_gc = opp_gc;
	ret.day = day;
	ret.month = month;
	ret.year = year;
	ret.len_t_name = strlen(t_name);
	strncpy(ret.t_name, t_name, sizeof(ret.t_name));

	return ret;
}

/** Updates a struct player's glicko data (and corresponding file) assuming
 * the player went through a given set on a given date. If the player does
 * not have a corresponding file, one is created and initialized to the
 * default glicko2 values.
 *
 * \param '*p1_name' string representing player-1's name
 * \param '*p2_name' string representing player-2's name
 * \param '*p1' a struct player representing player-1
 * \param '*p2' a struct player representing player-2
 * \param '*p1_gc' a double representing the number of games player-1 won in
 *     the set
 * \param '*p2_gc' a double representing the number of games player-2 won in
 *     the set
 * \param 'day' a char representing the day of the month the set was played on
 * \param 'month' a char representing the month the set was played in
 * \param 'year' a short representing the year the set was played in
 * \param 't_name' a string containing the name of the tournament this outcome
 *     took place at.
 * \return void
 */
void update_player_on_outcome(char* p1_name, char* p2_name,
	struct player* p1, struct player* p2, double* p1_gc, double* p2_gc,
	char day, char month, short year, char* t_name) {

	char *full_p1_path = file_path_with_player_dir(p1_name);
	char *full_p2_path = file_path_with_player_dir(p2_name);
	/* If the file does not exist, init the player struct to defaults */
	if (access(full_p1_path, R_OK | W_OK) == -1) {
		setRating(p1, DEF_RATING);
		setRd(p1, DEF_RD);
		p1->vol = DEF_VOL;
	} else {
		/* Read latest entries into usable data */
		struct entry p1_latest;
		int t;
		if (0 == (t = entry_file_read_last_entry(full_p1_path, &p1_latest))) {
			init_player_from_entry(p1, &p1_latest);
		} else {
			perror("entry_file_read_last_entry (update_player_on_outcome)");
		}
	}
	/* If the file does not exist, init the player struct to defaults */
	if (access(full_p2_path, R_OK | W_OK) == -1) {
		setRating(p2, DEF_RATING);
		setRd(p2, DEF_RD);
		p2->vol = DEF_VOL;
	} else {
		/* Read latest entries into usable data */
		struct entry p2_latest;
		if (0 == entry_file_read_last_entry(full_p2_path, &p2_latest)) {
			init_player_from_entry(p2, &p2_latest);
		} else {
			perror("entry_file_read_last_entry (update_player_on_outcome)");
		}
	}

	p1->_tau = DEF_TAU;
	p2->_tau = DEF_TAU;

	struct player new_p1 = *p1;
	struct player new_p2 = *p2;

	update_player(&new_p1, &p2->__rating, 1, &p2->__rd, p1_gc);
	update_player(&new_p2, &p1->__rating, 1, &p1->__rd, p2_gc);
	/* Adjust changes in glicko data based on weight of given game/set */
	new_p1.__rating = p1->__rating + ((new_p1.__rating - p1->__rating) * outcome_weight);
	new_p1.__rd = p1->__rd + ((new_p1.__rd - p1->__rd) * outcome_weight);
	new_p1.vol = p1->vol + ((new_p1.vol - p1->vol) * outcome_weight);
	struct entry p1_new_entry =
		create_entry(&new_p1, p1_name, p2_name, *p1_gc, *p2_gc, day, month, year, t_name);
	entry_file_append_entry_to_file(&p1_new_entry, full_p1_path);

	free(full_p1_path);
	free(full_p2_path);

	return;
}

/** All players whose last entry is not for the event of 't_name'
 * get their Glicko2 data adjusted. Unless their last RD adjustment
 * was within the same day.
 *
 * \param 'day' a char representing the day of the tournament
 * \param 'month' a char representing the month of the tournament
 * \param 'year' a char representing the year of the tournament
 * \param '*t_name' a string containing the name of the tournament.
 * \return void.
 */
void adjust_absent_players_no_file(char day, char month, \
	short year, char* t_name) {

	char did_not_comp = 1;
	DIR *p_dir;
	struct dirent *entry;
	// TODO: maybe not here but create .players directory if it doesn't exist?
	/* If the directory could not be accessed, print error and return */
	if ((p_dir = opendir(player_dir)) == NULL) {
		perror("opendir (adjust_absent_players_no_file)");
		return;
	}

	/* Get list of player names that did not compete
	 * apply step 6 to them and append to player file */
	while ((entry = readdir(p_dir)) != NULL) {
		/* If the directory item is a directory, skip */
		if (entry->d_type == DT_DIR) continue;

		/* Reset variable to assume player did not compete */
		did_not_comp = 1;
		for (int i = 0; i < tournament_names_len; i++) {
			/* If the one of the player who the system manager wants to track
			 * is found in the list of competitors at the tourney */
			if (0 == strcmp(entry->d_name, tournament_names[i])) {
				did_not_comp = 0;
				break;
			}
		}

		if (did_not_comp) {
			/* If the player who did not compete has a player file */
			if (access(file_path_with_player_dir(entry->d_name), \
				R_OK | W_OK) != -1) {

				struct player P;
				struct entry latest_ent;
				if (0 == \
					entry_file_read_last_entry(file_path_with_player_dir(entry->d_name), \
					&latest_ent)) {

					/* If this adjustment is taking place on a different
					 * day from their last entry */
					if (latest_ent.day != day || latest_ent.month != month \
						|| latest_ent.year != year) {

						init_player_from_entry(&P, &latest_ent);
						did_not_compete(&P);
						/* Only need to change entry RD since that's all
						 * Step 6 changes */
						latest_ent.RD = getRd(&P);
						/* Change qualities of the entry to reflect
						 * that it was not a real set, but a
						 * did_not_compete */
						strcpy(latest_ent.opp_name, "-");
						latest_ent.len_opp_name = strlen(latest_ent.opp_name);
						latest_ent.gc = 0;
						latest_ent.opp_gc = 0;
						latest_ent.day = day;
						latest_ent.month = month;
						latest_ent.year = year;
						strncpy(latest_ent.t_name, t_name, MAX_NAME_LEN - 1);
						latest_ent.t_name[strlen(latest_ent.t_name)] = '\0';
						latest_ent.len_t_name = strlen(latest_ent.t_name);
						entry_file_append_entry_to_file(&latest_ent, \
							file_path_with_player_dir(entry->d_name));
					}
				}
			}
			/* If they do not then they have never competed, so skip them */
		}
	}
	closedir(p_dir);
}
/** Takes a file path representing a file containing a list of file paths
 * to player files. All players who did not compete but are in the list,
 * get their Glicko2 data adjusted. Unless their last RD adjustment
 * was within the same day.
 *
 * \param '*player_list' the file path of the player list file.
 * \param '*t_name' a string containing the name of the tournament.
 * \return void.
 */
void adjust_absent_players(char* player_list, char day, char month, \
	short year, char* t_name) {

	FILE *player_file = fopen(player_list, "r");
	if (player_file == NULL) {
		perror("fopen (adjust_absent_players)");
		return;
	}

	char line[256];
	char did_not_comp = 1;
	/* get list of player names that did not compete
	 * apply step 6 to them and append to player file */

	/* Iterate through list of all the players the system manager wants
	 * to track */
	while (fgets(line, sizeof(line), player_file)) {
		/* Reset variable to assume player did not compete */
		did_not_comp = 1;
		/* Replace newline with null terminator */
		*strchr(line, '\n') = '\0';
		for (int i = 0; i < tournament_names_len; i++) {
			/* If the one of the player who the system manager wants to track
			 * is found in the list of competitors at the tourney */
			if (0 == strcmp(line, tournament_names[i])) {
				did_not_comp = 0;
				break;
			}
		}

		if (did_not_comp) {
			/* If the player who did not compete has a player file */
			if (access(file_path_with_player_dir(line), R_OK | W_OK) != -1) {
				struct player P;
				struct entry latest_ent;
				if (0 == \
					entry_file_read_last_entry(file_path_with_player_dir(line), \
					&latest_ent)) {

					/* If this adjustment is taking place on a different
					 * day from their last entry */
					if (latest_ent.day != day || latest_ent.month != month \
						|| latest_ent.year != year) {

						init_player_from_entry(&P, &latest_ent);
						did_not_compete(&P);
						/* Only need to change entry RD since that's all
						 * Step 6 changes */
						latest_ent.RD = getRd(&P);
						/* Change qualities of the entry to reflect that it was
						 * not a real set, but a did_not_compete */
						strcpy(latest_ent.opp_name, "-");
						latest_ent.len_opp_name = strlen(latest_ent.opp_name);
						latest_ent.gc = 0;
						latest_ent.opp_gc = 0;
						latest_ent.day = day;
						latest_ent.month = month;
						latest_ent.year = year;
						strncpy(latest_ent.t_name, t_name, MAX_NAME_LEN - 1);
						latest_ent.t_name[strlen(latest_ent.t_name)] = '\0';
						latest_ent.len_t_name = strlen(latest_ent.t_name);
						entry_file_append_entry_to_file(&latest_ent, \
							file_path_with_player_dir(line));
					}
				}
			}
			/* If they do not then they have never competed, so skip them */
		}
	}

	fclose(player_file);
}

/** Takes a bracket file and updates the ratings of all the players
 * mentioned in the bracket file as well as those specified in TODO another
 * file to account for inactivity.
 *
 * \param '*bracket_file_path' the file path of a bracket file which, on
 *     each line has the format of "[p1_name] [p2_name] [p1_game_count]
 *     [p2_game_count] [day] [month] [year]"
 * \return void
 */
void update_players(char* bracket_file_path) {

	/* Set to 0 since the bracket is beginning and no names are stored */
	tournament_names_len = 0;
	FILE *bracket_file = fopen(bracket_file_path, "r");
	if (bracket_file == NULL) {
		perror("fopen (bracket_file)");
		return;
	}

	char line[256];
	char p1_name[MAX_NAME_LEN];
	char p2_name[MAX_NAME_LEN];
	char p1_gc;
	char p2_gc;
	char day;
	char month;
	short year;
	char t_name[MAX_NAME_LEN];
	memset(t_name, 0, sizeof(t_name));
	strncpy(t_name, basename(bracket_file_path), sizeof(t_name));

	while (fgets(line, sizeof(line), bracket_file)) {
		/* Read data from one line of bracket file into all the variables */
		sscanf(line, "%s %s %hhd %hhd %hhd %hhd %hd",
			p1_name, p2_name, &p1_gc, &p2_gc, &day, &month, &year);

		if (calc_absent_players_with_file || calc_absent_players == 1) {
			char already_in = 0;
			char already_in2 = 0;
			for (int i = 0; i < tournament_names_len; i++) {
				/* If the name already exists in the list of entrants,
				 * don't add */
				if (0 == strcmp(p1_name, tournament_names[i])) {
					already_in = 1;
					break;
				}
				if (0 == strcmp(p2_name, tournament_names[i])) {
					already_in2 = 1;
					break;
				}
			}
			if (!already_in) {
				strncpy(tournament_names[tournament_names_len], \
					p1_name, MAX_NAME_LEN);
				tournament_names_len++;
			}
			if (!already_in2) {
				strncpy(tournament_names[tournament_names_len], \
					p2_name, MAX_NAME_LEN);
				tournament_names_len++;
			}
		}

		struct player p1;
		struct player p2;
		double p1_out;
		double p2_out;
		if (use_games == 1) {
			p1_out = 1;
			p2_out = 0;
			for (int i = 0; i < p1_gc; i++) {
				update_player_on_outcome(p1_name, p2_name, &p1, &p2, \
					&p1_out, &p2_out, day, month, year, t_name);
				update_player_on_outcome(p2_name, p1_name, &p2, &p1, \
					&p2_out, &p1_out, day, month, year, t_name);
			}
			p1_out = 0;
			p2_out = 1;
			for (int i = 0; i < p2_gc; i++) {
				update_player_on_outcome(p1_name, p2_name, &p1, &p2, \
					&p1_out, &p2_out, day, month, year, t_name);
				update_player_on_outcome(p2_name, p1_name, &p2, &p1, \
					&p2_out, &p1_out, day, month, year, t_name);
			}
		} else {
			p1_out = p1_gc > p2_gc;
			p2_out = p1_gc < p2_gc;
			update_player_on_outcome(p1_name, p2_name, &p1, &p2, \
				&p1_out, &p2_out, day, month, year, t_name);
			update_player_on_outcome(p2_name, p1_name, &p2, &p1, \
				&p2_out, &p1_out, day, month, year, t_name);
		}
	}

	// TODO: maybe: Print out everyones before and after with a (+/- change here)
	fclose(bracket_file);

	if (calc_absent_players == 1) {
		adjust_absent_players_no_file(day, month, year, t_name);
	}
	else if (calc_absent_players_with_file) {
		adjust_absent_players(player_list_file, day, month, year, t_name);
	}
}

void run_brackets(char *bracket_list_file_path) {
	FILE *bracket_list_file = fopen(bracket_list_file_path, "r");
	if (bracket_list_file == NULL) {
		perror("fopen (run_brackets)");
		return;
	}

	char line[256];

	while (fgets(line, sizeof(line), bracket_list_file)) {
		*strchr(line, '\n') = '\0';
		if (use_games == 1) {
			printf("running %s using games\n", line);
		} else {
			printf("running %s\n", line);
		}
		update_players(line);
	}

	fclose(bracket_list_file);
}

void merge(struct entry *first_array, int first_length, \
	struct entry *second_array, int second_length, struct entry *output_array) {

	int first_index = 0;
	int second_index = 0;
	int final_index = 0;

	while (first_index < first_length && second_index < second_length) {
		/* If the next element in the first array is greater than the second... */
		if (first_array[first_index].rating >= second_array[second_index].rating) {
			/* Add the first array element to the final array */
			output_array[final_index] = first_array[first_index];
			first_index++;
		} else {
			/* Add the second array element to the final array */
			output_array[final_index] = second_array[second_index];
			second_index++;
		}
		final_index++;
	}
	int elements_to_add = first_length - first_index;
	/* When one side array has been added to the output array before the
	 * other has been fully added */
	for (int i = 0; i < elements_to_add; i++) {
		output_array[final_index] = first_array[first_index];
		first_index++;
		final_index++;
	}
	elements_to_add = second_length - second_index;
	for (int i = 0; i < elements_to_add; i++) {
		output_array[final_index] = second_array[second_index];
		second_index++;
		final_index++;
	}
}

void merge_sort_pr_entry_array(struct entry *pr_entries, int array_size) {
	if (array_size <= 1) {
		return;
	} else if (array_size == 2) {
		if (pr_entries[0].rating < pr_entries[1].rating) {
			struct entry swap = pr_entries[0];
			pr_entries[0] = pr_entries[1];
			pr_entries[1] = swap;
		} else {
			return;
		}
	} else {
		/* split into 2 calls and recurse */
		int middle_index = (int) floor(array_size / 2.00);
		int len_sec_half = (int) ceil(array_size / 2.00);
		merge_sort_pr_entry_array(pr_entries, middle_index);
		merge_sort_pr_entry_array(&pr_entries[middle_index], len_sec_half);
		/* merge 2 resulting arrays */
		struct entry ret[array_size];
		merge(pr_entries, middle_index, &pr_entries[middle_index], len_sec_half, &ret[0]);
		/* Copy merged array contents into original array */
		for (int i = 0; i < array_size; i++) {
			pr_entries[i] = ret[i];
		}
		return;
	}
}

/** Creates a file listing the player's glicko details at the location
 * 'output_file_path'.
 *
 * \param '*file_path' the file path for a file containing, on each line,
 *     the file path of a player file generated by this program.
 * \param '*output_file_path' the file path at which to create the pr
 *     file.
 * \return void
 */
int generate_ratings_file(char* file_path, char* output_file_path) {
	FILE *players = fopen(file_path, "r");
	if (players == NULL) {
		perror("fopen (generate_ratings_file)");
		return -1;
	}

	clear_file(output_file_path);

	/* Player list file is a series of names followed by newlines '\n'
	 * therefore the longest line that should be supported should be the
	 * longest name */
	char line[MAX_NAME_LEN];
	int pr_entries_num = 0;
	int longest_name_length = 0;
	int longest_attended = 0;
	int longest_outcomes = 0;
	double longest_glicko_change = 0;
	/* Create a starting point pr entry array */
	int pr_entries_size = SIZE_PR_ENTRY;
	struct entry *players_pr_entries = \
		malloc(sizeof(struct entry) * pr_entries_size);
	struct entry temp;

	while (fgets(line, sizeof(line), players)) {
		/* Replace newline with null terminator */
		*strchr(line, '\n') = '\0';
		char *full_player_path = file_path_with_player_dir(line);
		/* If the player file was able to be read properly... */
		if (0 == entry_file_read_last_entry(full_player_path, &temp)) {
			int num_events;
			entry_file_get_events_attended(full_player_path, &num_events);
			if (longest_attended < num_events) longest_attended = num_events;
			int num_outcomes = entry_file_get_outcome_count(full_player_path);
			if (longest_outcomes < num_outcomes) {
				longest_outcomes = num_outcomes;
			}
			double num_glicko_change = \
				entry_file_get_glicko_change_since_last_event(full_player_path);
			if (fabs(longest_glicko_change) < fabs(num_glicko_change)) {
				longest_glicko_change = num_glicko_change;
			}
			// If the player attended the minimum number of events
			if (num_events >= pr_minimum_events) {
				/* If there is no space to add this pr entry, reallocate */
				if (pr_entries_num + 1 > pr_entries_size) {
					pr_entries_size += REALLOC_PR_ENTRIES_INC;
					players_pr_entries = realloc(players_pr_entries, \
						sizeof(struct entry) * pr_entries_size);
					if (players_pr_entries == NULL) {
						perror("realloc (generate_ratings_file)");
						return -2;
					}
				}
				/* ...add the player data to the player pr entry array*/
				players_pr_entries[pr_entries_num] = temp;
				pr_entries_num++;
			}
		}
		free(full_player_path);
	}

	/* Sort entries in the list by rating into non-increasing order */
	merge_sort_pr_entry_array(players_pr_entries, pr_entries_num);
	/* Get the longest name on the pr */
	for (int i = 0; i < pr_entries_num; i++) {
		if (longest_name_length < players_pr_entries[i].len_name) {
			longest_name_length = players_pr_entries[i].len_name;
		}
	}
	/* Store how long in characters the longest_attended count would take
	 * in longest_attended */
	char string_rep[128];
	sprintf(string_rep, "%d", longest_attended);
	longest_attended = strlen(string_rep);
	/* Store how long in characters the longest_attended count would take
	 * in longest_attended */
	sprintf(string_rep, "%d", longest_outcomes);
	longest_outcomes = strlen(string_rep);
	/* Append each entry pr file */
	for (int i = 0; i < pr_entries_num; i++) {
		if (verbose == 1) {
			entry_file_append_pr_entry_to_file_verbose(&players_pr_entries[i], \
				output_file_path, longest_name_length, longest_attended, \
				longest_outcomes);
		} else {
			entry_file_append_pr_entry_to_file(&players_pr_entries[i], output_file_path, \
				longest_name_length);
		}
	}

	fclose(players);
	return 0;
}

int generate_ratings_file_full(char* output_file_path) {
	DIR *p_dir;
	struct dirent *entry;
	if ((p_dir = opendir(player_dir)) != NULL) {
		clear_file(output_file_path);

		/* Player list file is a series of names followed by newlines '\n'
		 * therefore the longest line that should be supported should be the
		 * longest name */
		int pr_entries_num = 0;
		int longest_name_length = 0;
		int longest_attended = 0;
		int longest_outcomes = 0;
		double longest_glicko_change = 0;
		/* Create a starting point pr entry array */
		int pr_entries_size = SIZE_PR_ENTRY;
		struct entry *players_pr_entries = \
			malloc(sizeof(struct entry) * pr_entries_size);
		struct entry temp;

		while ((entry = readdir(p_dir)) != NULL) {
			// Make sure it doesn't count directories
			if (entry->d_type != DT_DIR) {
				char *full_player_path = file_path_with_player_dir(entry->d_name);
				/* If the player file was able to be read properly... */
				if (0 == entry_file_read_last_entry(full_player_path, &temp)) {
					int num_events;
					entry_file_get_events_attended(full_player_path, &num_events);
					if (longest_attended < num_events) longest_attended = num_events;
					int num_outcomes = entry_file_get_outcome_count(full_player_path);
					if (longest_outcomes < num_outcomes) {
						longest_outcomes = num_outcomes;
					}
					double num_glicko_change = \
						entry_file_get_glicko_change_since_last_event(full_player_path);
					if (fabs(longest_glicko_change) < fabs(num_glicko_change)) {
						longest_glicko_change = num_glicko_change;
					}
					// If the player attended the minimum number of events
					if (num_events >= pr_minimum_events) {
						/* If there is no space to add this pr entry,
						 * reallocate */
						if (pr_entries_num + 1 > pr_entries_size) {
							pr_entries_size += REALLOC_PR_ENTRIES_INC;
							players_pr_entries = realloc(players_pr_entries, \
								sizeof(struct entry) * pr_entries_size);
							if (players_pr_entries == NULL) {
								perror("realloc (generate_ratings_file)");
								return -2;
							}
						}
						/* ...add the player data to the player
						 * pr entry array */
						players_pr_entries[pr_entries_num] = temp;
						pr_entries_num++;
					}
				} else {
					return -3;
				}
				free(full_player_path);
			}
		}
		closedir(p_dir);
		/* Sort entries in the list by rating into non-increasing order */
		merge_sort_pr_entry_array(players_pr_entries, pr_entries_num);
		/* Get the longest name on the pr */
		for (int i = 0; i < pr_entries_num; i++) {
			if (longest_name_length < players_pr_entries[i].len_name) {
				longest_name_length = players_pr_entries[i].len_name;
			}
		}
		/* Store how long in characters the longest_attended count would take
		 * in longest_attended */
		char string_rep[128];
		sprintf(string_rep, "%d", longest_attended);
		longest_attended = strlen(string_rep);
		/* Store how long in characters the longest_attended count would take
		 * in longest_attended */
		sprintf(string_rep, "%d", longest_outcomes);
		longest_outcomes = strlen(string_rep);
		/* Store how long in characters the longest_glicko_change count would take
		 * in longest_glicko_change */
		sprintf(string_rep, "%5.1f", longest_glicko_change);
		/* Append each entry pr file */
		for (int i = 0; i < pr_entries_num; i++) {
			if (verbose == 1) {
				entry_file_append_pr_entry_to_file_verbose(&players_pr_entries[i], \
					output_file_path, longest_name_length, longest_attended, \
					longest_outcomes);
			} else {
				entry_file_append_pr_entry_to_file(&players_pr_entries[i], output_file_path, \
					longest_name_length);
			}
		}
		return 0;
	} else {
		perror("opendir (generate_ratings_file_full)");
		return -1;
	}
}

void merge_player_records(struct record *first_array, int first_length, \
	struct record *second_array, int second_length, struct record *output_array) {

	int first_index = 0;
	int second_index = 0;
	int final_index = 0;

	while (first_index < first_length && second_index < second_length) {
		/* If the second name is < the first name */
		if (strcmp(second_array[second_index].opp_name, first_array[first_index].opp_name) < 0) {
			output_array[final_index] = second_array[second_index];
			second_index++;
		/* If the first name is < the second name */
		} else {
			output_array[final_index] = first_array[first_index];
			first_index++;
		}
		final_index++;
	}
	int elements_to_add = first_length - first_index;
	/* When one side array has been added to the output array before the
	 * other has been fully added */
	for (int i = 0; i < elements_to_add; i++) {
		/* Add the first array element to the final array */
		output_array[final_index] = first_array[first_index];
		first_index++;
		final_index++;
	}
	elements_to_add = second_length - second_index;
	for (int i = 0; i < elements_to_add; i++) {
		/* Add the second array element to the final array */
		output_array[final_index] = second_array[second_index];
		second_index++;
		final_index++;
	}
}

void merge_sort_player_records(struct record *records, int array_size) {
	if (array_size <= 1) {
		return;
	} else if (array_size == 2) {
		/* If there is less data on the first player or if there is equal
		 * data, but the second name < first name */
		if (strcmp(records[1].opp_name, records[0].opp_name) < 0) {
			struct record swap;
			/* Save data from first player to swap variables */
			swap = records[0];
			/* Put second player data in first player spot */
			records[0] = records[1];
			/* Put first player (swap) data in second player spot */
			records[1] = swap;
		} else {
			return;
		}
	} else {
		/* split into 2 calls and recurse */
		int middle_index = (int) floor(array_size / 2.00);
		int len_sec_half = (int) ceil(array_size / 2.00);
		merge_sort_player_records(records, middle_index);
		merge_sort_player_records(&records[middle_index], len_sec_half);
		/* merge 2 resulting arrays */
		struct record ret[array_size];
		merge_player_records(records, middle_index, \
			&records[middle_index], len_sec_half, ret);
		/* Copy merged array contents into original array */
		for (int i = 0; i < array_size; i++) {
			records[i] = ret[i];
		}
		return;
	}
}

int print_player_records(char *file_path) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (print_player_records)");
		return -1;
	}

	struct record records[128];
	int found_name = 0;
	int num_rec = 0;
	char passes_filter = 0;
	char line[MAX_NAME_LEN];
	struct entry ent;
	/* Read the starter data in the file */
	entry_file_read_start_from_file(file_path, &ent);
	/* Go back to beginning of entries for reading */
	fseek(p_file, 0, SEEK_SET);
	entry_file_get_to_entries(p_file);

	while (entry_file_read_entry(p_file, &ent) == 0) {
		/* If a minimum event requirement is being used, assume
		 * the player does not pass the filter initially */
		passes_filter = 0;
		/* Filter players to be ones who have attended the minimum number of
		 * events specified by the '-m' flag */
		int attended_count = 0;
		/* If the entry is for a player and not an RD adjustment since
		 * RD adjustments don't have player files */
		if (0 != strcmp(ent.opp_name, "-")) {
			char *full_player_path = \
				file_path_with_player_dir(ent.opp_name);
			entry_file_get_events_attended(full_player_path, &attended_count);
			free(full_player_path);
		}

		if (attended_count >= pr_minimum_events) {
			passes_filter = 1;
			/* Filter players to be the ones in the given '-p' flag file */
			if (o_generate_pr == 1) {
				passes_filter = 0;
				FILE *filter_file = fopen(pr_list_file_path, "r");
				if (filter_file == NULL) {
					perror("fopen (filter_player_list)");
					return -1;
				}

				while (fgets(line, sizeof(line), filter_file)) {
					*strchr(line, '\n') = '\0';
					/* If the name is in the filter, mark it accordingly */
					if (0 == strcmp(ent.opp_name, line)) {
						passes_filter = 1;
						break;
					}
				}
				fclose(filter_file);
			}
		}
		/* If a filter was not given, use every entry */
		if (passes_filter == 1) {
			found_name = 0;
			for (int i = 0; i < num_rec; i++) {
				if (0 == strcmp(ent.opp_name, records[i].opp_name)) {
					found_name = 1;
					if (ent.gc > ent.opp_gc) records[i].wins += 1;
					else if (ent.gc == ent.opp_gc) records[i].ties += 1;
					else if (ent.gc < ent.opp_gc) records[i].losses += 1;
				}
			}
			if (found_name == 0) {
				strncpy(records[num_rec].name, ent.name, MAX_NAME_LEN);
				strncpy(records[num_rec].opp_name, ent.opp_name, MAX_NAME_LEN);
				if (ent.gc > ent.opp_gc) records[num_rec].wins += 1;
				else if (ent.gc == ent.opp_gc) records[num_rec].ties += 1;
				else if (ent.gc < ent.opp_gc) records[num_rec].losses += 1;
				num_rec++;
			}
		}
	}

	merge_sort_player_records(&(records[0]), num_rec);

	for (int i = 0; i < num_rec; i++) {
		char* output_colour_player = NORMAL;
		if (colour_output == 1) {
			// If the player has a winning record
			if (records[i].wins > records[i].losses) {
				output_colour_player = GREEN;
			// If the player has a losing record
			} else if (records[i].wins < records[i].losses) {
				output_colour_player = RED;
			// If the player has a tied record
			} else {
				output_colour_player = YELLOW;
			}
		}

		// If the user wants ties to be printed
		if (print_ties == 1) {
			printf("%s vs %s%s%s = %d-%d-%d\n", \
				records[i].name, output_colour_player, records[i].opp_name, \
				NORMAL, records[i].wins, records[i].ties, records[i].losses);
		} else {
			printf("%s vs %s%s%s = %d-%d\n", \
				records[i].name, output_colour_player, records[i].opp_name, \
				NORMAL, records[i].wins, records[i].losses);
		}
	}

	fclose(p_file);
	return 0;
}

void print_player_attended(char *attended, int count) {
	// Print names of all tournaments attended by the player
	for (int i = 0; i < count; i++) {
		printf("%s\n", attended + i * MAX_NAME_LEN);
	}
}

/* Takes a char pointer created by malloc or calloc and a pointer
 * to an integer. Gets the name of every player in the player directory
 * 'player_dir' into the array in lexiographical order. Modifies '*num'
 * to point to the numer of elements in the array upon completion.
 *
 * \param '*players' a char pointer created by calloc or malloc which
 *     will be modified to contain all the player names,
 *     'MAX_NAME_LEN' apart and in lexiographical order.
 * \param '*num' a int pointer that will be modified to contain
 *     the number of player names in the array when this function
 *     has completed.
 * \param 'type' a char representing the type of sort the returned
 *     array should be of. Options are 'LEXIO' for a lexiograpically
 *     sorted array. Any value that is not 'LEXIO' will make
 *     this function return an orderless array.
 * \return a pointer to the return array.
 */
// TODO: change to check syscalls and to return int
char *players_in_player_dir(char *players, int *num, char type) {
	DIR *p_dir;
	struct dirent *entry;
	// TODO reallocate '*players' if necessary make it required that
	// '*players' is a pointer made by a calloc or malloc call

	if ((p_dir = opendir(player_dir)) != NULL) {
		*num = 0;
		while ((entry = readdir(p_dir)) != NULL) {
			// Make sure it doesn't count directories
			if (entry->d_type != DT_DIR) {
				int num_events;
				char *full_player_path = \
					file_path_with_player_dir(entry->d_name);
				entry_file_get_events_attended(full_player_path, &num_events);
				// If the player attended the minimum number of events
				if (num_events >= pr_minimum_events) {
					if (type == LEXIO) {
						int i = MAX_NAME_LEN * (*(num) - 1);
						// Find the right index to insert the name at
						while (strcmp(&players[i], entry->d_name) > 0 \
							&& i >= 0) {

							// Move later-occuring name further in the array
							strncpy(&players[i + MAX_NAME_LEN], &players[i], \
								MAX_NAME_LEN);
							i -= MAX_NAME_LEN;
						}
						strncpy(&players[i + MAX_NAME_LEN], entry->d_name, \
							MAX_NAME_LEN);
					} else {
						strncpy(&players[MAX_NAME_LEN * *(num)], \
							entry->d_name, MAX_NAME_LEN);
					}
					// Add null terminator to each name
					players[MAX_NAME_LEN * (*(num) + 1)] = '\0';
					*num = *(num) + 1;
				}
				free(full_player_path);
			}
		}
		closedir(p_dir);
	}
	return players;
}

/* Takes 2 entry-file file paths and a struct record, modifies the
 * given struct record to be the first players
 * record/head-to-head/matchup on the other player.
 *
 * \param '*player1' A file path to an entry-file for the first player
 * \param '*player2' A file path to an entry-file for the second player
 * \param '*ret' A struct record which upon the successful completion of this
 *     function, will contain the first players record against
 *     the second player.
 * \return an int representing if this function succeeded or failed.
       Negative upon failure, 0 upon success.
 */
int get_record(char *player1, char *player2, struct record *ret) {

	char *full_player1_path = file_path_with_player_dir(player1);
	/* Read the starter data in the file */
	struct entry ent;
	entry_file_read_start_from_file(full_player1_path, &ent);
	//printf("full_player1_path: \"%s\"\n", full_player1_path);

	FILE *p_file = fopen(full_player1_path, "rb");
	if (p_file == NULL) {
		perror("fopen (get_record)");
		return -1;
	}
	strncpy(ret->name, ent.name, MAX_NAME_LEN);
	// TODO: actually get player2 name from their file.
	// '*player2' is just a file name
	strncpy(ret->opp_name, player2, MAX_NAME_LEN);
	ret->wins = 0;
	ret->losses = 0;
	ret->ties = 0;

	/* Get to the entries in the player file */
	int r = entry_file_get_to_entries(p_file);
	if (r != 0) {
		perror("get_record (entry_file_get_to_entries)");
		return -2;
	}

	while (entry_file_read_entry(p_file, &ent) == 0) {
		// If the opponent for the given entry is the player of interest
		if (0 == strcmp(ent.opp_name, player2)) {
			if (ent.gc > ent.opp_gc) ret->wins += 1;
			else if (ent.gc == ent.opp_gc) ret->ties += 1;
			else if (ent.gc < ent.opp_gc) ret->losses += 1;
		}
	}
	free(full_player1_path);
	fclose(p_file);

	return 0;
}

/* Takes an array of player names, and the length of the array.
 * Returns the maximum 'strlen' result of an element in the array.
 *
 * \param '*players' pointer to an array of 'MAX_NAME_LEN' * '*(num_players)'
 *     chars.
 * \param 'array_len' the length of the '*players' array.
 * \return an integer representing the longest name in the array.
 *     The longest possible return value is 'MAX_NAME_LEN'.
 */
unsigned long int longest_name(char *players, int array_len) {
	unsigned long int ret = 0;
	for (int i = 0; i < array_len; i++) {
		if (strlen(&players[MAX_NAME_LEN * i]) > ret) {
			ret = strlen(&players[MAX_NAME_LEN * i]);
		}
	}

	return ret;
}

/* Takes an array of player names created with a malloc or calloc call,
 * the length of the array, and a file path. Modifies the array
 * to contain only player names that exist as lines in the file.
 * changes '*num_players' accordingly.
 *
 * \param '*players' pointer to an array of 'MAX_NAME_LEN' * '*(num_players)'
 *     chars.
 * \param '*num_players' the length of the '*players' array.
 * \param '*pr_list_file_path' the file path of a pr list file.
 * \return an integer representing the success or failure of
 *     this function. 0 Means sucess, negative numbers mean failure.
 */
int filter_player_list(char **players_pointer, int *num_players, \
	char *pr_list_file_path) {

	FILE *filter_file = fopen(pr_list_file_path, "r");
	if (filter_file == NULL) {
		perror("fopen (filter_player_list)");
		return -1;
	}

	int app_ind = 0;
	char line[MAX_NAME_LEN];
	char *players = *(players_pointer);
	char *filtered_players = \
		malloc(sizeof(char) * MAX_NAME_LEN * (*num_players));

	while (fgets(line, sizeof(line), filter_file)) {
		/* Replace newline with null terminator */
		*strchr(line, '\n') = '\0';

		for (int i = 0; i < *num_players; i++) {
			/* If the player name exists in the player list, add
			 * it to the filtered list of players */
			if (0 == strcmp(line, players + (i * MAX_NAME_LEN))) {
				strncpy(filtered_players + (app_ind * MAX_NAME_LEN), line, \
					MAX_NAME_LEN - 1);
				app_ind++;
			}
		}
	}

	fclose(filter_file);
	free(players);
	*num_players = app_ind;
	*players_pointer = filtered_players;

	return 0;
}

void print_matchup_table(void) {
	// Print a table showing the matchup data for all players stored in the
	// system (aka the player directory)
	DIR *p_dir;
	// Get a list of all players tracked by the system to allow for proper
	// column and row titles
	int num_players = 0;
	int space_between_columns = 3;
	// TODO: better size allocation
	char *players = malloc(MAX_NAME_LEN * 128);
	players_in_player_dir(players, &num_players, LEXIO);

	/* Filter players to be the ones in the given '-p' flag file */
	if (o_generate_pr == 1) {
		filter_player_list(&players, &num_players, pr_list_file_path);
	}

	int longest_n = longest_name(players, num_players);
	// 'num_players + 1' to accomodate one player per row and an extra row
	// for the column titles
	char output[num_players + 1][1024];
	// Empty the first line of output
	memset(output[0], 0, 1024);
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
	printf("%s\n", output[0]);

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
			printf("%s\n", output[i + 1]);
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
	char *players = malloc(MAX_NAME_LEN * 128);
	players_in_player_dir(players, &num_players, LEXIO);

	/* Filter players to be the ones in the given '-p' flag file */
	if (o_generate_pr == 1) {
		filter_player_list(&players, &num_players, pr_list_file_path);
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
	printf("%s\n", output[0]);

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
			printf("%s\n", output[i + 1]);
		}
		closedir(p_dir);
	} else {
		perror("opendir (print_matchup_table_csv)");
		return;
	}
}

/* Deletes every player file in the player directory 'player_dir'.
 *
 * \return an int representing if the function succeeded or not.
 *     Negative if there was an error, 0 on success.
 */
int reset_players(void) {
	DIR *p_dir;
	struct dirent *entry;
	if ((p_dir = opendir(player_dir)) != NULL) {
		while ((entry = readdir(p_dir)) != NULL) {
			// Make sure it doesn't count directories
			if (entry->d_type != DT_DIR) {
				char *full_player_path = \
					file_path_with_player_dir(entry->d_name);
				remove(full_player_path);
				free(full_player_path);
			}
		}
		closedir(p_dir);
		return 0;
	} else {
		perror("opendir (reset_players)");
		return -1;
	}
}

int main(int argc, char **argv) {
	/* INIT SECTION:
	 * 1. Initialize player_dir to the file path for the player directory
	 * 2. Check if player_dir exists
	 */

	/* 1. Initialize player_dir to the file path for the player directory */
	memset(player_dir, 0, sizeof(player_dir));
	strncpy(player_dir, PLAYER_DIR, sizeof(player_dir) - 1);

	/* 2. Check if player_dir exists */
	DIR *d = opendir(player_dir);
	if (d) {
		closedir(d);
	}
	/* If 'player_dir' does not exist */
	else if (errno == ENOENT) {
		fprintf(stderr, \
			"G2ME: Warning: 'player_dir' did not exist, creating...\n");
		/* If there was an error making the player_directory */
		if (0 != mkdir(player_dir, 0700)) {
			perror("mkdir (main)");
		}
	} else {
		perror("opendir (main)");
		return -1;
	}

	int opt;
	struct option opt_table[] = {
		/* Don't make RD adjustments for players absent
		 * from some tournaments */
		{ "no-adjustment",	no_argument,		NULL,	'0' },
		/* Add (or create if necessary) a player entry/player entry file
		 * from user input */
		{ "add-entry",		required_argument,	NULL,	'a' },
		{ "events-attended",required_argument,	NULL,	'A' },
		/* Run through a given bracket file making the necessary updates
		 * to the glicko2 scores */
		{ "bracket",		required_argument,	NULL,	'b' },
		{ "brackets",		required_argument,	NULL,	'B' },
		{ "count-outcomes",	required_argument,	NULL,	'c' },
		{ "matchup-csv",	required_argument,	NULL,	'C' },
		{ "player-dir",		required_argument,	NULL,	'd' },
		{ "use-games",		no_argument,		NULL,	'g' },
		/* Output given player file in human readable form */
		{ "human",			required_argument,	NULL,	'h' },
		/* Don't delete the player files when running a new bracket */
		{ "keep-players",	no_argument,		NULL,	'k' },
		/* Output last entry in given player file in human readable form */
		{ "last-entry",		required_argument,	NULL,	'l' },
		{ "min-events",		required_argument,	NULL,	'm' },
		{ "matchup-table",	required_argument,	NULL,	'M' },
		{ "no-colour",		required_argument,	NULL,	'n' },
		{ "no-ties",		required_argument,	NULL,	'N' },
		{ "output",			required_argument,	NULL,	'o' },
		/* Output a file with a sorted list of players and their ratings */
		{ "power-rating",	required_argument,	NULL,	'p' },
		{ "P",				required_argument,	NULL,	'P' },
		{ "refactor",		required_argument,	NULL,	'r' },
		{ "records",		required_argument,	NULL,	'R' },
		{ "verbose",		no_argument,		NULL,	'v' },
		{ "weight",			required_argument,	NULL,	'w' },
		{ "remove-entries",	required_argument,	NULL,	'x' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, \
		"0a:A:b:B:c:Cd:gh:kl:m:MnNo:p:P:r:R:vw:x:", opt_table, NULL)) != -1) {
		if (opt == 'A') {
			int count;
			char *full_player_path = file_path_with_player_dir(optarg);
			char *attended = \
				entry_file_get_events_attended(full_player_path, &count);
			print_player_attended(attended, count);
			free(full_player_path);
			free(attended);
		} else if (opt == 'c') {
			char *full_player_path = file_path_with_player_dir(optarg);
			printf("%d\n", entry_file_get_outcome_count(full_player_path));
			free(full_player_path);
		} else if (opt == 'd') {
			memset(player_dir, 0, sizeof(player_dir));
			strncpy(player_dir, optarg, sizeof(player_dir) - 1);
		} else if (opt == 'h') {
			char *full_player_path = file_path_with_player_dir(optarg);
			if (verbose == 1) print_player_file_verbose(full_player_path);
			else print_player_file(full_player_path);
			free(full_player_path);
		} else if (opt == 'l') {
			char *full_player_path = file_path_with_player_dir(optarg);
			if (0 == entry_file_read_last_entry(full_player_path, &temp)) {
				print_entry(temp);
			}
			free(full_player_path);
		} else if (opt == 'r') {
			char *full_player_path = file_path_with_player_dir(optarg);
			entry_file_refactor_name(full_player_path);
			free(full_player_path);
		} else if (opt == 'R') {
			char *full_player_path = file_path_with_player_dir(optarg);
			print_player_records(full_player_path);
			free(full_player_path);
		} else if (opt == 'x') {
			char *full_player_path = file_path_with_player_dir(optarg);
			entry_file_remove_entry(full_player_path);
			free(full_player_path);
		}

		switch (opt) {
			case '0': calc_absent_players = 0; break;
			case 'a':
				write_entry_from_input(file_path_with_player_dir(optarg));
				break;
			case 'b':
				if (keep_players == 0) reset_players();
				update_players(optarg);
				break;
			case 'C': print_matchup_table_csv(); break;
			case 'B':
				if (keep_players == 0) reset_players();
				run_brackets(optarg);
				break;
			case 'g': use_games = 1; break;
			case 'k': keep_players = 1; break;
			case 'm': pr_minimum_events = atoi(optarg); break;
			case 'M': print_matchup_table(); break;
			case 'n': colour_output = 0; break;
			case 'N': print_ties = 0; break;
			case 'p':
				o_generate_pr = 1;
				strncpy(pr_list_file_path, optarg, \
					sizeof(pr_list_file_path) - 1);
				break;
			case 'w': outcome_weight = strtod(optarg, NULL); break;
			case 'P':
				calc_absent_players_with_file = 1;
				strncpy(player_list_file, optarg, sizeof(player_list_file) - 1);
				break;
			case 'o':
				if (o_generate_pr) {
					generate_ratings_file(pr_list_file_path, optarg);
					/* The pr has been generated,
					 * o is no longer set to make one */
					o_generate_pr = 0;
				} else {
					generate_ratings_file_full(optarg);
				}
				break;
			case 'v': verbose = 1; break;
		}
	}
}
