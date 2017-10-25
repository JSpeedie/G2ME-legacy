#include <getopt.h>
#include <libgen.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "glicko2.h"

#define MAX_NAME_LEN 128
#define MAX_FILE_PATH_LEN 256

char use_games = 0;
char player_list_file[256];
char calc_absent_players = 0;
double outcome_weight = 1;
char tournament_names[128][128];
unsigned char tournament_names_len = 0;
char pr_list_file_path[128];
char o_generate_pr = 0;
char *player_dir = ".players/";

typedef struct entry {
	unsigned char len_name;
	unsigned char len_opp_name;
	char name[MAX_NAME_LEN];
	char opp_name[MAX_NAME_LEN];
	double rating;
	double RD;
	double vol;
	unsigned char gc;
	unsigned char opp_gc;
	unsigned char day;
	unsigned char month;
	short year;
}Entry;

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

/** Reads contents of a player file to a struct entry. Returns 0 upon success,
 * and a negative number upon failure. Function expects that starter data
 * has already been passed and that the FILE is on an entry
 *
 * \param '*f' the file being read
 * \param '*E' the struct entry to store an entry found in the file too
 * \return 0 upon success, or a negative number upon failure.
 */
int read_entry(FILE *f, struct entry *E) {
	if (0 == fread(&E->len_opp_name, sizeof(char), 1, f)) { return -1; }
	/* Read player names */
	if (0 == fread(E->opp_name, sizeof(char), E->len_opp_name, f)) { return -2; }
	/* Add null terminators */
	E->name[E->len_name] = '\0';
	E->opp_name[E->len_opp_name] = '\0';
	if (0 == fread(&E->rating, sizeof(double), 1, f)) { return -3; }
	if (0 == fread(&E->RD, sizeof(double), 1, f)) { return -4; }
	if (0 == fread(&E->vol, sizeof(double), 1, f)) { return -5; }
	if (0 == fread(&E->gc, sizeof(char), 1, f)) { return -6; }
	if (0 == fread(&E->opp_gc, sizeof(char), 1, f)) { return -7; }
	if (0 == fread(&E->day, sizeof(char), 1, f)) { return -8; }
	if (0 == fread(&E->month, sizeof(char), 1, f)) { return -9; }
	if (0 == fread(&E->year, sizeof(short), 1, f)) { return -10; }

	return 0;
}

/** Reads a player file at the given file path and returns the number
 * of entries contained in that file.
 *
 * \param '*file_path' the file path of the file to be read.
 * \return 0 upon success, or a negative number upon failure.
 */
int get_entries_in_file(char *file_path) {
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (get_entries_in_file)");
		return -1;
	}

	int entries = 0;
	/* Read entry from old file */
	struct entry *cur_entry = malloc(sizeof(struct entry));
	/* Read the starter data in the file */
	if (1 != fread(&cur_entry->len_name, sizeof(char), 1, base_file)) { return -2; }
	if (cur_entry->len_name
		!= fread(cur_entry->name, sizeof(char), cur_entry->len_name, base_file)) { return -3; }

	/* While the function is still able to read entries from the old file */
	while (0 == read_entry(base_file, cur_entry)) {
		entries++;
	}
	free(cur_entry);
	fclose(base_file);

	return entries;
}

/** Returns the offset within a player file at which the last entry begins.
 *
 * \param '*file_path' a string of the file path of a player file for the
 *     function to find the offset of the last entry
 * \return a long representing the offset within the file at which the last
 *     entry begins
 */
long int get_last_entry_offset(char* file_path) {
	FILE *entry_file = fopen(file_path, "rb");
	if (entry_file == NULL) {
		perror("fopen (get_last_entry_offset)");
		return 0;
	}

	/* Read entry from old file */
	char len_of_name;
	char name[MAX_NAME_LEN];
	/* Read the starter data in the file */
	fread(&len_of_name, sizeof(char), 1, entry_file);
	fread(name, sizeof(char), len_of_name, entry_file);

	char len_of_opp_name;
	long int last_entry_offset = ftell(entry_file);

	while (0 != fread(&len_of_opp_name, sizeof(char), 1, entry_file)) {

		/* = chars for the current opponent name, the 3 glicko doubles, the 4
		 * chars for the 2 game counts, the day and month and the one short
		 * for the year */
		long int size_of_current_entry =
			len_of_opp_name + (3 * sizeof(double))
			+ (4 * sizeof(char)) + sizeof(short);
		fseek(entry_file, size_of_current_entry, SEEK_CUR);

		last_entry_offset =
			ftell(entry_file) - size_of_current_entry - (1 * sizeof(char));
	}

	fclose(entry_file);

	return last_entry_offset;
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

/** Appends an entry to a given player file and return an int representing
 * whether the function succeeded or not.
 *
 * \param '*E' the struct entry to be appended
 * \param '*file_path' the file path of the player file
 * \return int that is 0 upon the function succeeding and negative upon
 *     any sort of failure.
 */
int append_entry_to_file(struct entry* E, char* file_path) {

	char len_name = strlen(E->name);
	char len_opp_name = strlen(E->opp_name);
	/* If the file did not exist */
	char existed = access(file_path, R_OK) != -1;

	/* Open file for appending */
	FILE *entry_file = fopen(file_path, "ab+");
	if (entry_file == NULL) {
		perror("fopen (append_entry_to_file)");
		return -1;
	}

	if (!existed) {
		if (1 != fwrite(&len_name, sizeof(char), 1, entry_file)) { return -2; }
		if (strlen(E->name)
			!= fwrite(E->name, sizeof(char), strlen(E->name), entry_file)) {
			return -3;
		}
	}

	/* Write length of opp name and opp name */
	if (1 != fwrite(&len_opp_name, sizeof(char), 1, entry_file)) { return -4; }
	if (strlen(E->opp_name)
		!= fwrite(E->opp_name, sizeof(char), strlen(E->opp_name), entry_file)) {
			return -5;
	}
	/* Write glicko data */
	if (1 != fwrite(&E->rating, sizeof(double), 1, entry_file)) { return -6; }
	if (1 != fwrite(&E->RD, sizeof(double), 1, entry_file)) { return -7; }
	if (1 != fwrite(&E->vol, sizeof(double), 1, entry_file)) { return -8; }
	/* Write game counts */
	if (1 != fwrite(&E->gc, sizeof(char), 1, entry_file)) { return -9; }
	if (1 != fwrite(&E->opp_gc, sizeof(char), 1, entry_file)) { return -10; }
	/* Write date data */
	if (1 != fwrite(&E->day, sizeof(char), 1, entry_file)) { return -11; }
	if (1 != fwrite(&E->month, sizeof(char), 1, entry_file)) { return -12; }
	if (1 != fwrite(&E->year, sizeof(short), 1, entry_file)) { return -13; }

	fclose(entry_file);
	return 0;
}

/** Appends a pr entry (the name and glicko2 data for a player) to a given
 * file. Returns an int representing success.
 *
 * \param '*E' the struct entry to append to the pr file
 * \param '*file_path' the file path for the pr file
 * \return 0 upon success, negative number on failure.
 */
int append_pr_entry_to_file(struct entry* E, char* file_path, \
	int longest_name_length) {

	FILE *entry_file = fopen(file_path, "a+");
	if (entry_file == NULL) {
		perror("fopen (append_pr_entry_to_file)");
		return -1;
	}
	if (fprintf(entry_file, "%*s  %6.1lf  %5.1lf  %10.8lf\n", \
		longest_name_length, E->name, E->rating, E->RD, E->vol) < 0) {

		perror("fprintf");
		return -2;
	}

	fclose(entry_file);
	return 0;
}

/** Appends an entry created from user input to a file.
 *
 * \param '*file_path' the file path that you want to append the entry to
 * \return void
 */
void write_entry_from_input(char* file_path) {
	printf("[Name] [Opp] [Rating] [RD] [Vol] [gc] [opp gc] [day] [month] [year]: ");

	struct entry input_entry;
	scanf("%s %s %lf %lf %lf %hhd %hhd %hhd %hhd %hd",
		input_entry.name, input_entry.opp_name, &input_entry.rating,
		&input_entry.RD, &input_entry.vol, &input_entry.gc,
		&input_entry.opp_gc, &input_entry.day, &input_entry.month,
		&input_entry.year);
	// TODO: make this use player dir?
	append_entry_to_file(&input_entry, file_path);
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

	printf("%d %d %-10s %-10s %16.14lf %16.14lf %16.14lf %d-%d %s\n", \
		E.len_name, E.len_opp_name, E.name, E.opp_name, E.rating, E.RD, E.vol, E.gc, E.opp_gc, date);
}

/** Reads a player file at the given file path, reads the "Player 1"
 * data into the given entry parameter.
 *
 * \param '*file_path' the file path of the file to be read.
 * \return 0 upon success, or a negative number upon failure.
 */
int read_start_from_file(char *file_path, struct entry *E) {
	/* Open file for appending */
	FILE *entry_file = fopen(file_path, "rb");
	if (entry_file == NULL) {
		perror("fopen (read_start_from_file)");
		return -1;
	}

	/* Read the starter data in the file */
	if (1 != fread(&E->len_name, sizeof(char), 1, entry_file)) { return -2; }
	if (E->len_name != fread(E->name, sizeof(char), E->len_name, entry_file)) { return -3; }
	E->name[E->len_name] = '\0';

	fclose(entry_file);
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
	/* Open file for reading */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (print_player_file)");
		return -1;
	}

	char len_of_name;
	char name[MAX_NAME_LEN];
	/* Read the starter data in the file */
	if (1 != fread(&len_of_name, sizeof(char), 1, p_file)) { return -2; }
	if (len_of_name != fread(name, sizeof(char), len_of_name, p_file)) { return -3; }

	struct entry line;
	line.len_name = len_of_name;
	strncpy(line.name, name, MAX_NAME_LEN);

	while (read_entry(p_file, &line) == 0) {
		print_entry(line);
	}

	fclose(p_file);
	return 0;
}

/** Modifies a struct entry to be that of the last entry found in a player file.
 *
 * \param '*file_path' the file path of the player file to be read
 * \param '*ret' a struct entry pointer to have the data read into.
 * \return 0 upon success, a negative int upon failure.
 */
int read_last_entry(char* file_path, struct entry *ret) {
	/* Open files for reading contents */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		printf("read last %s\n", file_path);
		perror("fopen (read_last_entry)");
		/* If the file could not be read for any reason, return accordingly */
		return -1;
	}

	/* Read the player's name from the file */
	read_start_from_file(file_path, ret);
	/* Set file position to be at the latest entry for that player */
	long int offset = get_last_entry_offset(file_path);
	fseek(p_file, offset, SEEK_SET);
	read_entry(p_file, ret);
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
 * \return a struct entry containing all that information
 */
struct entry create_entry(struct player* P, char* name, char* opp_name,
	char gc, char opp_gc, char day, char month, short year) {

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
 * \return void
 */
void update_player_on_outcome(char* p1_name, char* p2_name,
	struct player* p1, struct player* p2, double* p1_gc, double* p2_gc,
	char day, char month, short year) {

	char *full_p1_path = file_path_with_player_dir(p1_name);
	char *full_p2_path = file_path_with_player_dir(p2_name);
	/* If the file does not exist, init the player struct to defaults */
	if (access(full_p1_path, R_OK | W_OK) == -1) {
		setRating(p1, 1500.0);
		setRd(p1, 350.0);
		p1->vol = 0.06;
	} else {
		/* Read latest entries into usable data */
		struct entry p1_latest;
		if (0 == read_last_entry(full_p1_path, &p1_latest)) {
			init_player_from_entry(p1, &p1_latest);
		} else {
			perror("read_last_entry (update_player_on_outcome)");
		}
	}
	/* If the file does not exist, init the player struct to defaults */
	if (access(full_p2_path, R_OK | W_OK) == -1) {
		setRating(p2, 1500.0);
		setRd(p2, 350.0);
		p2->vol = 0.06;
	} else {
		/* Read latest entries into usable data */
		struct entry p2_latest;
		if (0 == read_last_entry(full_p2_path, &p2_latest)) {
			init_player_from_entry(p2, &p2_latest);
		} else {
			perror("read_last_entry (update_player_on_outcome)");
		}
	}

	p1->_tau = 0.5;
	p2->_tau = 0.5;

	struct player new_p1 = *p1;
	struct player new_p2 = *p2;

	update_player(&new_p1, &p2->__rating, 1, &p2->__rd, p1_gc);
	update_player(&new_p2, &p1->__rating, 1, &p1->__rd, p2_gc);
	/* Adjust changes in glicko data based on weight of given game/set */
	new_p1.__rating = p1->__rating + ((new_p1.__rating - p1->__rating) * outcome_weight);
	new_p1.__rd = p1->__rd + ((new_p1.__rd - p1->__rd) * outcome_weight);
	new_p1.vol = p1->vol + ((new_p1.vol - p1->vol) * outcome_weight);
	struct entry p1_new_entry =
		create_entry(&new_p1, p1_name, p2_name, *p1_gc, *p2_gc, day, month, year);
	append_entry_to_file(&p1_new_entry, full_p1_path);

	free(full_p1_path);
	free(full_p2_path);

	return;
}

/** Takes a file path representing a file containing a list of file paths
 * to player files. All players who did not compete but are in the list,
 * and adjusts their Glicko2 data.
 *
 * \param '*player_list' the file path of the player list file.
 * \return void.
 */
void adjust_absent_players(char* player_list) {
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
				if (0 == read_last_entry(file_path_with_player_dir(line), &latest_ent)) {
					init_player_from_entry(&P, &latest_ent);
					did_not_compete(&P);
					/* Only need to change entry RD since that's all Step 6 changes */
					latest_ent.RD = getRd(&P);
					/* Change qualities of the entry to reflect that it was not a
					 * real set, but a did_not_compete */
					strcpy(latest_ent.opp_name, "-");
					latest_ent.len_opp_name = strlen(latest_ent.opp_name);
					latest_ent.gc = 0;
					latest_ent.opp_gc = 0;
					latest_ent.day = 0;
					latest_ent.month = 0;
					latest_ent.year = 0;
					append_entry_to_file(&latest_ent, file_path_with_player_dir(line));
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

	while (fgets(line, sizeof(line), bracket_file)) {
		/* Read data from one line of bracket file into all the variables */
		sscanf(line, "%s %s %hhd %hhd %hhd %hhd %hd",
			p1_name, p2_name, &p1_gc, &p2_gc, &day, &month, &year);

		if (calc_absent_players) {
			char already_in = 0;
			char already_in2 = 0;
			for (int i = 0; i < tournament_names_len; i++) {
				/* If the name already exists in the list of entrants, don't add */
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
				strncpy(tournament_names[tournament_names_len], p1_name, MAX_NAME_LEN);
				tournament_names_len++;
			}
			if (!already_in2) {
				strncpy(tournament_names[tournament_names_len], p2_name, MAX_NAME_LEN);
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
				update_player_on_outcome(p1_name, p2_name, &p1, &p2, &p1_out, &p2_out, day, month, year);
			}
			p1_out = 0;
			p2_out = 1;
			for (int i = 0; i < p2_gc; i++) {
				update_player_on_outcome(p2_name, p1_name, &p2, &p1, &p2_out, &p1_out, day, month, year);
			}
		} else {
			p1_out = p1_gc > p2_gc;
			p2_out = p1_gc < p2_gc;
			update_player_on_outcome(p1_name, p2_name, &p1, &p2, &p1_out, &p2_out, day, month, year);
			update_player_on_outcome(p2_name, p1_name, &p2, &p1, &p2_out, &p1_out, day, month, year);
		}
	}

	// TODO: maybe: Print out everyones before and after with a (+/- change here)
	fclose(bracket_file);

	if (calc_absent_players) {
		adjust_absent_players(player_list_file);
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
		printf("running %s", line);
		*strchr(line, '\n') = '\0';
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
void generate_ratings_file(char* file_path, char* output_file_path) {
	FILE *players = fopen(file_path, "r");
	if (players == NULL) {
		perror("fopen (generate_ratings_file)");
		return;
	}

	clear_file(output_file_path);

	char line[256];
	int pr_entries_size = 0;
	/* Create a starting point, 128 person, pr entry array */
	struct entry *players_pr_entries = malloc(sizeof(struct entry) * 128);
	struct entry temp;

	while (fgets(line, sizeof(line), players)) {
		// TODO: realloc if over 128 players
		/* Replace newline with null terminator */
		*strchr(line, '\n') = '\0';
		char *full_player_path = file_path_with_player_dir(line);
		/* If the player file was able to be read properly... */
		if (0 == read_last_entry(full_player_path, &temp)) {
			/* ...add the player data to the player pr entry array*/
			players_pr_entries[pr_entries_size] = temp;
			pr_entries_size++;
		}
		free(full_player_path);
	}

	/* Sort entries in the list by rating into non-increasing order */
	merge_sort_pr_entry_array(players_pr_entries, pr_entries_size);
	/* Get the longest name on the pr */
	int longest_name_length = 0;
	for (int i = 0; i < pr_entries_size; i++) {
		if (longest_name_length < players_pr_entries[i].len_name) {
			longest_name_length = players_pr_entries[i].len_name;
		}
	}
	/* Append each entry pr file */
	for (int i = 0; i < pr_entries_size; i++) {
		append_pr_entry_to_file(&players_pr_entries[i], output_file_path, \
			longest_name_length);
	}

	fclose(players);
	return;
}

/** Takes a file path of a player file, prompts the user for the new name,
 * and renames Player 1 to the new name.
 *
 * \param '*file_path' the file path of the player file.
 * \return 0 upon success, a negative number upon failure.
 */
int refactor_file(char *file_path) {
	char new_name[MAX_NAME_LEN];
	printf("New player name: ");
	scanf("%s", new_name);
	char *full_new_name_path = file_path_with_player_dir(new_name);

	FILE *base_file = fopen(file_path, "ab+");
	if (base_file == NULL) {
		perror("fopen (refactor_file)");
		return -1;
	}

	/* Read entry from old file */
	struct entry *cur_entry = malloc(sizeof(struct entry));
	/* Read the starter data in the file */
	if (1 != fread(&cur_entry->len_name, sizeof(char), 1, base_file)) { return -2; }
	if (cur_entry->len_name
		!= fread(cur_entry->name, sizeof(char), cur_entry->len_name, base_file)) { return -3; }
	/* While the function is still able to read entries from the old file */
	while (0 == read_entry(base_file, cur_entry)) {
		/* Update entry information to have the new name */
		strncpy(cur_entry->name, new_name, MAX_NAME_LEN - 1);
		cur_entry->len_name = strlen(full_new_name_path);

		// write new entry in new file as we get each old entry
		append_entry_to_file(cur_entry, full_new_name_path);
	}
	free(cur_entry);
	free(full_new_name_path);
	fclose(base_file);
	return 0;
}

/** Takes a file path of a player file, and removes the last entry in it.
 *
 * \param '*file_path' the file path of the player file.
 * \return 0 upon success, a negative number upon failure.
 */
int remove_line_from_file(char *file_path) {
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (remove_line_from_file)");
		return -1;
	}
	char dir[strlen(file_path) + 1];
	char base[strlen(file_path) + 1];
	memset(dir, 0, sizeof(dir));
	memset(base, 0, sizeof(base));
	strncpy(dir, file_path, sizeof(dir) - 1);
	strncpy(base, file_path, sizeof(base) - 1);

	int lines_to_remove = 1;
	int entries = get_entries_in_file(file_path);
	int entries_read = 0;
	/* Read entry from old file */
	struct entry *cur_entry = malloc(sizeof(struct entry));
	char new_file_name[MAX_NAME_LEN + 1];
	memset(new_file_name, 0, sizeof(new_file_name));
	/* Add the full path up to the file */
	strncat(new_file_name, dirname(dir), sizeof(new_file_name) - 1);
	strncat(new_file_name, "/", sizeof(new_file_name) - strlen(new_file_name) - 1);
	/* Add the temp file */
	strncat(new_file_name, ".", sizeof(new_file_name) - strlen(new_file_name) - 1);
	strncat(new_file_name, basename(base), sizeof(new_file_name) - strlen(new_file_name) - 1);
	/* Read the starter data in the file */
	if (1 != fread(&cur_entry->len_name, sizeof(char), 1, base_file)) { return -2; }
	if (cur_entry->len_name
		!= fread(cur_entry->name, sizeof(char), cur_entry->len_name, base_file)) { return -3; }
	/* While the function is still able to read entries from the old file */
	while (0 == read_entry(base_file, cur_entry) && entries_read < (entries - lines_to_remove)) {
		entries_read++;
		/* write new entry in new file as we get each old entry */
		append_entry_to_file(cur_entry, new_file_name);
	}
	/* Delete original file */
	remove(file_path);
	/* Copy temp file to original file path */
	rename(new_file_name, file_path);
	free(cur_entry);
	fclose(base_file);
	return 0;
}

int main(int argc, char **argv) {
	int opt;
	struct option opt_table[] = {
		/* Add (or create if necessary) a player entry/player entry file
		 * from user input */
		{ "add-entry",		required_argument,	NULL,	'a' },
		/* Run through a given bracket file making the necessary updates
		 * to the glicko2 scores */
		{ "bracket",		required_argument,	NULL,	'b' },
		{ "brackets",		required_argument,	NULL,	'B' },
		{ "use-games",		no_argument,		NULL,	'g' },
		/* Output given player file in human readable form */
		{ "human",			required_argument,	NULL,	'h' },
		/* Output last entry in given player file in human readable form */
		{ "last-entry",		required_argument,	NULL,	'l' },
		/* Output a file with a sorted list of players and their ratings */
		{ "power-rating",	required_argument,	NULL,	'p' },
		{ "P",				required_argument,	NULL,	'P' },
		{ "output",			required_argument,	NULL,	'o' },
		{ "refactor",		required_argument,	NULL,	'r' },
		{ "weight",			required_argument,	NULL,	'w' },
		{ "remove-entries",	required_argument,	NULL,	'x' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, \
		"a:b:B:gh:l:p:P:o:r:w:x:", opt_table, NULL)) != -1) {
		if (opt == 'h') {
			char *full_player_path = file_path_with_player_dir(optarg);
			print_player_file(full_player_path);
			free(full_player_path);
		} else if (opt == 'l') {
			char *full_player_path = file_path_with_player_dir(optarg);
			if (0 == read_last_entry(full_player_path, &temp)) {
				print_entry(temp);
			}
			free(full_player_path);
		} else if (opt == 'r') {
			char *full_player_path = file_path_with_player_dir(optarg);
			refactor_file(full_player_path);
			free(full_player_path);
		} else if (opt == 'x') {
			char *full_player_path = file_path_with_player_dir(optarg);
			remove_line_from_file(full_player_path);
			free(full_player_path);
		}

		switch (opt) {
			case 'a':
				write_entry_from_input(file_path_with_player_dir(optarg));
				break;
			case 'b':
				update_players(optarg);
				break;
			case 'B':
				run_brackets(optarg);
				break;
			case 'g':
				use_games = 1;
				break;
			case 'p':
				o_generate_pr = 1;
				strncpy(pr_list_file_path, optarg, sizeof(pr_list_file_path) - 1);
				break;
			case 'w':
				outcome_weight = strtod(optarg, NULL);
				break;
			case 'P':
				calc_absent_players = 1;
				strncpy(player_list_file, optarg, sizeof(player_list_file) - 1);
				break;
			case 'o':
				if (o_generate_pr) {
					generate_ratings_file(pr_list_file_path, optarg);
					/* The pr has been generated,
					 * o is no longer set to make one */
					o_generate_pr = 0;
				}
				break;
		}
	}
}
