#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "glicko2.h"

#define MAX_NAME_LEN 128

char use_games = 0;
char player_list_file[256];
char calc_absent_players = 0;
double outcome_weight = 1;
char tournament_names[128][128];
char tournament_names_len = 0;

typedef struct entry {
	char len_name;
	char len_opp_name;
	char name[MAX_NAME_LEN];
	char opp_name[MAX_NAME_LEN];
	double rating;
	double RD;
	double vol;
	char gc;
	char opp_gc;
	char day;
	char month;
	short year;
}Entry;

int get_entries_in_file(char *file_path) {
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (get_entries_in_file)");
		return;
	}

	int entries = 0;
	/* Read entry from old file */
	struct entry *cur_entry = malloc(sizeof(struct entry));
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

	char len_of_name;
	char len_of_opp_name;
	long int last_entry_offset = ftell(entry_file);

	while (0 != fread(&len_of_name, sizeof(char), 1, entry_file)) {

		if (0 == fread(&len_of_opp_name, sizeof(char), 1, entry_file)) {
			break;
		}

		long int size_of_current_entry =
			len_of_name + len_of_opp_name + (3 * sizeof(double)) + (4 * sizeof(char)) + sizeof(short);
		fseek(entry_file, size_of_current_entry, SEEK_CUR);

		last_entry_offset = ftell(entry_file) - size_of_current_entry - (2 * sizeof(char));
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

/** Appends an entry to a given player file.
 *
 * \param '*E' the struct entry to be appended
 * \param '*file_path' the file path of the player file
 * \return void
 */
void append_entry_to_file(struct entry* E, char* file_path) {
	FILE *entry_file = fopen(file_path, "ab+");
	if (entry_file == NULL) {
		perror("fopen (append_entry_to_file)");
		return;
	}
	// TODO find nice way to check all fwrite calls
	/* Write lengths of names */
	char len_name = strlen(E->name);
	char len_opp_name = strlen(E->opp_name);
	fwrite(&len_name, sizeof(char), 1, entry_file);
	fwrite(&len_opp_name, sizeof(char), 1, entry_file);
	/* Write names */
	fwrite(E->name, sizeof(char), strlen(E->name), entry_file);
	fwrite(E->opp_name, sizeof(char), strlen(E->opp_name), entry_file);
	/* Write glicko data */
	fwrite(&E->rating, sizeof(double), 1, entry_file);
	fwrite(&E->RD, sizeof(double), 1, entry_file);
	fwrite(&E->vol, sizeof(double), 1, entry_file);
	/* Write game counts */
	fwrite(&E->gc, sizeof(char), 1, entry_file);
	fwrite(&E->opp_gc, sizeof(char), 1, entry_file);
	/* Write date data */
	fwrite(&E->day, sizeof(char), 1, entry_file);
	fwrite(&E->month, sizeof(char), 1, entry_file);
	fwrite(&E->year, sizeof(short), 1, entry_file);
	fclose(entry_file);
}

/** Appends a pr entry (the name and glicko2 data for a player) to a given
 * file.
 *
 * \param '*E' the struct entry to append to the pr file
 * \param '*file_path' the file path for the pr file
 * \return void
 */
void append_pr_entry_to_file(struct entry* E, char* file_path) {
	FILE *entry_file = fopen(file_path, "a+");
	if (entry_file == NULL) {
		perror("fopen (append_pr_entry_to_file)");
		return;
	}
	if (fprintf(entry_file, "%s %.1lf %.2lf %.8lf\n", E->name, E->rating, E->RD, E->vol) < 0) {
		perror("fprintf");
	}

	fclose(entry_file);
	return;
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

/** Reads contents of a player file to a struct entry. Returns 0 upon success,
 * and a negative number upon failure.
 *
 * \param '*f' the file being read
 * \param '*E' the struct entry to store an entry found in the file too
 * \return 0 upon success, or a negative number upon failure.
 */
int read_entry(FILE *f, struct entry *E) {
	if (0 == fread(&E->len_name, sizeof(char), 1, f)) { return -1; }
	if (0 == fread(&E->len_opp_name, sizeof(char), 1, f)) { return -2; }
	/* Read player names */
	if (0 == fread(E->name, sizeof(char), E->len_name, f)) { return -3; }
	if (0 == fread(E->opp_name, sizeof(char), E->len_opp_name, f)) { return -4; }
	/* Add null terminators */
	E->name[E->len_name] = '\0';
	E->opp_name[E->len_opp_name] = '\0';
	if (0 == fread(&E->rating, sizeof(double), 1, f)) { return -5; }
	if (0 == fread(&E->RD, sizeof(double), 1, f)) { return -6; }
	if (0 == fread(&E->vol, sizeof(double), 1, f)) { return -7; }
	if (0 == fread(&E->gc, sizeof(char), 1, f)) { return -8; }
	if (0 == fread(&E->opp_gc, sizeof(char), 1, f)) { return -9; }
	if (0 == fread(&E->day, sizeof(char), 1, f)) { return -10; }
	if (0 == fread(&E->month, sizeof(char), 1, f)) { return -11; }
	if (0 == fread(&E->year, sizeof(short), 1, f)) { return -12; }

	return 0;
}

/** Prints all the contents of a player file to stdout with each entry
 * on a new line.
 *
 * \param '*file_path' the file path of the player file to be read
 * \return void
 */
void print_player_file(char* file_path) {
	FILE *p_file = fopen(file_path, "rb+");
	if (p_file == NULL) {
		perror("fopen (print_player_file)");
		return;
	}

	struct entry line;

	while (read_entry(p_file, &line) == 0) {
		print_entry(line);
	}

	fclose(p_file);

	return;
}

/** Creates a struct entry from the last entry found in a player file.
 *
 * \param '*file_path' the file path of the player file to be read
 * \return a struct entry representing the last entry in the file
 */
struct entry read_last_entry(char* file_path) {
	struct entry line;
	/* Open files for reading contents */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (read_last_entry)");
		return line;
	}

	/* Set file position to be at the latest entry for that player */
	long int offset = get_last_entry_offset(file_path);
	fseek(p_file, offset, SEEK_SET);
	read_entry(p_file, &line);
	fclose(p_file);

	return line;
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

	/* If the file does not exist, init the player struct to defaults */
	if (access(p1_name, R_OK | W_OK) == -1) {
		setRating(p1, 1500.0);
		setRd(p1, 350.0);
		p1->vol = 0.06;
	} else {
		/* Read latest entries into usable data */
		struct entry p1_latest = read_last_entry(p1_name);
		init_player_from_entry(p1, &p1_latest);
	}
	/* If the file does not exist, init the player struct to defaults */
	if (access(p2_name, R_OK | W_OK) == -1) {
		setRating(p2, 1500.0);
		setRd(p2, 350.0);
		p2->vol = 0.06;
	} else {
		/* Read latest entries into usable data */
		struct entry p2_latest = read_last_entry(p2_name);
		init_player_from_entry(p2, &p2_latest);
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
	append_entry_to_file(&p1_new_entry, p1_name);

	return;
}

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
			printf("player did not compete: %s\n", line);
			struct player P;
			struct entry latest_ent = read_last_entry(line);
			init_player_from_entry(&P, &latest_ent);
			print_entry(latest_ent);
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
			print_entry(latest_ent);
			append_entry_to_file(&latest_ent, line);
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

/** Creates a file listing the player's glicko details specified by user
 * input in stdin
 *
 * \param '*file_path' the file path for a file containing, on each line,
 *     the file path of a player file generated by this program
 * \return void
 */
void generate_ratings_file(char* file_path) {
	FILE *players = fopen(file_path, "r");
	if (players == NULL) {
		perror("fopen (generate_ratings_file)");
		return;
	}

	char output_file[256];
	printf("What file would like to output the power ratings to? ");
	scanf("%s", output_file);
	clear_file(output_file);

	char line[256];
	struct entry temp;

	while (fgets(line, sizeof(line), players)) {
		/* Replace newline with null terminator */
		*strchr(line, '\n') = '\0';
		temp = read_last_entry(line);
		append_pr_entry_to_file(&temp, output_file);
	}

	fclose(players);
	return;
}

void refactor_file(char *file_path) {
	char new_name[MAX_NAME_LEN];
	printf("New player name: ");
	scanf("%s", new_name);

	FILE *base_file = fopen(file_path, "ab+");
	if (base_file == NULL) {
		perror("fopen (refactor_file)");
		return;
	}

	/* Read entry from old file */
	struct entry *cur_entry = malloc(sizeof(struct entry));
	int ret = 0;
	/* While the function is still able to read entries from the old file */
	while (0 == (ret = read_entry(base_file, cur_entry))) {
		/* Update entry information to have the new name */
		strncpy(cur_entry->name, new_name, MAX_NAME_LEN - 1);
		cur_entry->len_name = strlen(new_name);

		// write new entry in new file as we get each old entry
		append_entry_to_file(cur_entry, new_name);
	}
	free(cur_entry);
	fclose(base_file);
}

void remove_line_from_file(char *file_path) {
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (remove_line_from_file)");
		return;
	}

	int lines_to_remove = 1;
	int entries = get_entries_in_file(file_path);
	int entries_read = 0;
	/* Read entry from old file */
	struct entry *cur_entry = malloc(sizeof(struct entry));
	char new_file_name[MAX_NAME_LEN + 1] = strcat(".", cur_entry->name);
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
}

int main(int argc, char **argv) {
	int opt;
	struct option opt_table[] = {
		{ "use-games",		no_argument,		NULL,	'g' },
		/* Output given player file in human readable form */
		{ "human",			required_argument,	NULL,	'h' },
		/* Output last entry in given player file in human readable form */
		{ "last-entry",		required_argument,	NULL,	'l' },
		/* Add (or create if necessary) a player entry/player entry file
		 * from user input */
		{ "add-entry",		required_argument,	NULL,	'a' },
		/* Run through a given bracket file making the necessary updates
		 * to the glicko2 scores */
		{ "bracket",		required_argument,	NULL,	'b' },
		/* Output a file with a sorted list of players and their ratings */
		{ "power-rating",	required_argument,	NULL,	'p' },
		{ "refactor",		required_argument,	NULL,	'r' },
		{ "weight",			required_argument,	NULL,	'w' },
		{ "P",				required_argument,	NULL,	'P' },
		{ "remove-line",	required_argument,	NULL,	'x' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, \
		"gh:l:a:b:p:r:w:P:", opt_table, NULL)) != -1) {
		switch (opt) {
			case 'g':
				use_games = 1;
				break;
			case 'h':
				print_player_file(optarg);
				break;
			case 'l':
				print_entry(read_last_entry(optarg));
				break;
			case 'a':
				write_entry_from_input(optarg);
				break;
			case 'b':
				update_players(optarg);
				break;
			case 'p':
				generate_ratings_file(optarg);
				break;
			case 'r':
				refactor_file(optarg);
				break;
			case 'w':
				outcome_weight = strtod(optarg, NULL);
				break;
			case 'P':
				calc_absent_players = 1;
				strncpy(player_list_file, optarg, sizeof(player_list_file) - 1);
				break;
			case 'x':
				remove_line_from_file(optarg);
				break;
		}
	}
}
