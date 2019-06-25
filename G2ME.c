/* Non-windows includes */
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
#include <time.h>
#include <unistd.h>
/* Windows includes */
#ifdef _WIN32
#include <windows.h>
#endif

#include "G2ME.h"
#include "entry_file.h"
#include "fileops.h"
#include "glicko2.h"
#include "player_dir.h"
#include "printing.h"
#include "sorting.h"

char COMMENT_SYMBOL[] = { "#" };
#ifdef __linux__
char PLAYER_DIR[] = { ".players/" };
char DIR_TERMINATOR = '/';
#elif _WIN32
char PLAYER_DIR[] = { ".players\\" };
char DIR_TERMINATOR = '\\';
#else
char PLAYER_DIR[] = { ".players/" };
char DIR_TERMINATOR = '/';
#endif

const char ERROR_PLAYER_DIR_DNE[] = { "Error: 'player_dir' either could not be "
	"created or does not exist\n"};
const char ERROR_PLAYER_DNE[] = { "Error: the given player could not be found "
	"in working directory or the given player directory\n"};

char flag_output_to_stdout = 0;
char flag_time_program = 0;
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
char tournament_names[256][256];
unsigned char tournament_names_len = 0;
char filter_file_path[MAX_FILE_PATH_LEN];
char f_flag_used = 0;
char p_flag_used = 0;
char player_dir[MAX_FILE_PATH_LEN];

int get_record(char *, char *, struct record *);
struct record *get_all_records(char *, int *);

struct entry temp;

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
	char gc, char opp_gc, char day, char month, short year, char* t_name, short season_id) {

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
	ret.season_id = season_id;

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
	struct player* p1, struct player* p2, char* p1_gc, char* p2_gc,
	char day, char month, short year, char* t_name, short season_id) {

	char *full_p1_path = player_dir_file_path_with_player_dir(p1_name);
	char *full_p2_path = player_dir_file_path_with_player_dir(p2_name);
	/* If the file does not exist, init the player struct to defaults */
#ifdef __linux__
	if (access(full_p1_path, R_OK | W_OK) == -1) {
#elif _WIN32
	if (_access(full_p1_path, 0) == -1) {
#else
	if (access(full_p1_path, R_OK | W_OK) == -1) {
#endif
		setRating(p1, DEF_RATING);
		setRd(p1, DEF_RD);
		p1->vol = DEF_VOL;
	} else {
		/* Read latest entries into usable data */
		struct entry p1_latest;
		int t;
		if (0 == (t = entry_file_read_last_entry(full_p1_path, &p1_latest))) {
			init_player_from_entry(p1, &p1_latest);
			/* If this outcome was not a part of a season, write the season
			 * as the same as the latest season the player was in */
			if (season_id == -1) {
				season_id = p1_latest.season_id;
			}
		} else {
			perror("entry_file_read_last_entry (update_player_on_outcome)");
		}
	}
	/* If the file does not exist, init the player struct to defaults */
#ifdef __linux__
	if (access(full_p2_path, R_OK | W_OK) == -1) {
#elif _WIN32
	if (_access(full_p2_path, 0) == -1) {
#else
	if (access(full_p2_path, R_OK | W_OK) == -1) {
#endif
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
	double p1_result = (double) (*(p1_gc) > *(p2_gc));
	double p2_result = (double) (*(p1_gc) < *(p2_gc));

	update_player(&new_p1, &p2->__rating, 1, &p2->__rd, &p1_result);
	update_player(&new_p2, &p1->__rating, 1, &p1->__rd, &p2_result);
	/* Adjust changes in glicko data based on weight of given game/set */
	new_p1.__rating = p1->__rating + ((new_p1.__rating - p1->__rating) * outcome_weight);
	new_p1.__rd = p1->__rd + ((new_p1.__rd - p1->__rd) * outcome_weight);
	new_p1.vol = p1->vol + ((new_p1.vol - p1->vol) * outcome_weight);
	struct entry p1_new_entry =
		create_entry(&new_p1, p1_name, p2_name, *p1_gc, *p2_gc, day, month, year, t_name, season_id);
	int ret = entry_file_append_entry_to_file(&p1_new_entry, full_p1_path);
	if (ret != 0) {
		fprintf(stderr, "Error appending entry to file \"%s\"\n", full_p1_path);
	}

	free(full_p1_path);
	free(full_p2_path);

	return;
}

/* Takes a player file, some date info and an event name, and adjusts
 * the given player's RD in their file provided it does not conflict
 * with the date rule of no more than 1 adjustment in a day.
 *
 * \param '*player_file' a file path to a player entry-file
 *     which will have their RD adjusted.
 * \param 'day' the day of the RD adjustment
 * \param 'month' the month of the RD adjustment
 * \param 'year' the year of the RD adjustment
 * \param '*t_name' the name of the event that they were absent from.
 */
void adjust_absent_player(char *player_file, char day, char month, short year, \
	char *t_name) {

	/* If the player who did not compete has a player file */
#ifdef __linux__
	if (access(player_dir_file_path_with_player_dir(player_file), \
		R_OK | W_OK) != -1) {
#elif _WIN32
	if (_access(player_dir_file_path_with_player_dir(player_file), \
		R_OK | W_OK) != -1) {
#else
	if (access(player_dir_file_path_with_player_dir(player_file), \
		R_OK | W_OK) != -1) {
#endif
		struct player P;
		struct entry latest_ent;
		if (0 == \
			entry_file_read_last_entry(
				player_dir_file_path_with_player_dir(player_file), \
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
				/* Modify first bit of 'day' to tell whether this was a real
				 * set/game. Day should always be <= 31 leaving 3 extra bits.
				 * Bitwise OR with 10000000 */
				latest_ent.day = latest_ent.day | (1 << ((sizeof(latest_ent.day) * 8) - 1));
				latest_ent.month = month;
				latest_ent.year = year;
				strncpy(latest_ent.t_name, t_name, MAX_NAME_LEN - 1);
				latest_ent.t_name[strlen(latest_ent.t_name)] = '\0';
				latest_ent.len_t_name = strlen(latest_ent.t_name);
				entry_file_append_entry_to_file(&latest_ent, \
					player_dir_file_path_with_player_dir(player_file));
			}
		}
	}
	/* If they do not then they have never competed, so skip them */
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
	/* If the directory could not be accessed, print error and return */
	if ((p_dir = opendir(player_dir)) == NULL) {
		perror("opendir (adjust_absent_players_no_file)");
		return;
	}

	/* Get list of player names that did not compete
	 * apply step 6 to them and append to player file */
	while ((entry = readdir(p_dir)) != NULL) {
		/* If the directory item is a directory, skip */
		if (0 == check_if_dir(player_dir, entry->d_name)) continue;

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
			adjust_absent_player(entry->d_name, day, month, year, t_name);
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

	char line[MAX_FILE_PATH_LEN];
	char did_not_comp = 1;
	/* get list of player names that did not compete
	 * apply step 6 to them and append to player file */

	/* Iterate through list of all the players the system manager wants
	 * to track */
	while (fgets(line, sizeof(line), player_file)) {
		/* Reset variable to assume player did not compete */
		did_not_comp = 1;
		/* Replace newline with null terminator */
		char *end_of_line = strchr(line, '\n');
		if (end_of_line == NULL) {
			perror("strchr (filter_player_list)");
			return;
		}
		*end_of_line = '\0';
		for (int i = 0; i < tournament_names_len; i++) {
			/* If the one of the player who the system manager wants to track
			 * is found in the list of competitors at the tourney */
			if (0 == strcmp(line, tournament_names[i])) {
				did_not_comp = 0;
				break;
			}
		}

		if (did_not_comp) {
			adjust_absent_player(line, day, month, year, t_name);
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
// TODO: break up function.
// TODO: make sscanfs safe/secure.
//       Right now they risk a stack/buffer overflow
int update_players(char* bracket_file_path, short season_id) {
	/* Set to 0 since the bracket is beginning and no names are stored */
	tournament_names_len = 0;
	FILE *bracket_file = fopen(bracket_file_path, "r");
	if (bracket_file == NULL) {
		perror("fopen (bracket_file)");
		return -1;
	}

	char line[MAX_FILE_PATH_LEN];
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
		/* Remove comments from text */
		char parse_line = 0;
		unsigned long i = 0;
		while (i < strlen(line)) {
			/* If it reaches a non-whitespace character, check if it's
			 * a comment */
			if (line[i] != '	' && line[i] != ' ') {
				/* If there is space for the comment symbol in the
				 * line, continue */
				if (i + strlen(COMMENT_SYMBOL) <= strlen(line)) {
					/* If there is a comment, do not parse the line */
					if (strncmp(&line[i], COMMENT_SYMBOL, strlen(COMMENT_SYMBOL)) == 0) {
						line[i] = '\0';
						break;
					}
				}
			}
			i++;
		}
		/* Check if the line is non-empty */
		i = 0;
		while (i < strlen(line)) {
			/* If it reaches a non-whitespace character */
			if (line[i] != '	' && line[i] != ' ' && line[i] != '\n' && line[i] != '\r') {
				parse_line = 1;
				break;
			}
			i++;
		}
		if (parse_line == 1) {
			/* Read data from one line of bracket file into all the variables */
#ifdef __linux__
			sscanf(line, "%s %s %hhd %hhd %hhd %hhd %hd",
				p1_name, p2_name, &p1_gc, &p2_gc, &day, &month, &year);
#elif _WIN32

			char *token = strtok(line, " ");
			int temp;

			if (token == NULL) fprintf(stderr, "Not enough arguments given in bracket file\n");
			strncpy(p1_name, token, MAX_NAME_LEN);

			token = strtok(NULL, " ");
			if (token == NULL) fprintf(stderr, "Not enough arguments given in bracket file\n");
			strncpy(p2_name, token, MAX_NAME_LEN);

			token = strtok(NULL, " ");
			if (token == NULL) fprintf(stderr, "Not enough arguments given in bracket file\n");
			sscanf(token, "%d", &temp);
			p1_gc = (char)temp;

			token = strtok(NULL, " ");
			if (token == NULL) fprintf(stderr, "Not enough arguments given in bracket file\n");
			sscanf(token, "%d", &temp);
			p2_gc = (char)temp;

			token = strtok(NULL, " ");
			if (token == NULL) fprintf(stderr, "Not enough arguments given in bracket file\n");
			sscanf(token, "%d", &temp);
			day = (char)temp;

			token = strtok(NULL, " ");
			if (token == NULL) fprintf(stderr, "Not enough arguments given in bracket file\n");
			sscanf(token, "%d", &temp);
			month = (char)temp;

			token = strtok(NULL, " ");
			if (token == NULL) fprintf(stderr, "Not enough arguments given in bracket file\n");
			sscanf(token, "%d", &temp);
			year = (short)temp;
#else
			sscanf(line, "%s %s %hhd %hhd %hhd %hhd %hd",
				p1_name, p2_name, &p1_gc, &p2_gc, &day, &month, &year);
#endif

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
			char p1_out;
			char p2_out;
			if (use_games == 1) {
				p1_out = 1;
				p2_out = 0;
				for (int i = 0; i < p1_gc; i++) {
					update_player_on_outcome(p1_name, p2_name, &p1, &p2, \
						&p1_out, &p2_out, day, month, year, t_name, season_id);
					update_player_on_outcome(p2_name, p1_name, &p2, &p1, \
						&p2_out, &p1_out, day, month, year, t_name, season_id);
				}
				p1_out = 0;
				p2_out = 1;
				for (int i = 0; i < p2_gc; i++) {
					update_player_on_outcome(p1_name, p2_name, &p1, &p2, \
						&p1_out, &p2_out, day, month, year, t_name, season_id);
					update_player_on_outcome(p2_name, p1_name, &p2, &p1, \
						&p2_out, &p1_out, day, month, year, t_name, season_id);
				}
			} else {
				update_player_on_outcome(p1_name, p2_name, &p1, &p2, \
					&p1_gc, &p2_gc, day, month, year, t_name, season_id);
				update_player_on_outcome(p2_name, p1_name, &p2, &p1, \
					&p2_gc, &p1_gc, day, month, year, t_name, season_id);
			}
		}
	}

	fclose(bracket_file);

	if (calc_absent_players == 1) {
		adjust_absent_players_no_file(day, month, year, t_name);
	} else if (calc_absent_players_with_file) {
		adjust_absent_players(player_list_file, day, month, year, t_name);
	}

	return 0;
}


/* Takes a file path to a bracket file, and runs the bracket.
 *
 * \param '*bracket_file_path' a file path to a bracket file
 * \return an int representing if the function succeeded or failed.
 *     Negative on failure. 0 upon success.
 */
int run_single_bracket(char *bracket_file_path) {
	if (use_games == 1) {
		fprintf(stdout, "running \"%s\" using games ...", bracket_file_path);
	} else {
		fprintf(stdout, "running \"%s\" ...", bracket_file_path);
	}
	int ret = update_players(bracket_file_path, -1);
	if (ret == 0) {
		fprintf(stdout, "DONE\n");
		return 0;
	} else {
		fprintf(stdout, "ERROR\n");
		return 0;
	}
}

/* Takes a file path to a bracket list file, for each line in the file,
 * attempts to access a file of the name [line_in_bracket_list_file_path_file]
 * and run the bracket.
 *
 * \param '*bracket_list_file_path' a file path to a bracket list file
 * \return an int representing if the function succeeded or failed.
 *     Negative on failure. 0 upon success.
 */
int run_brackets(char *bracket_list_file_path) {
	FILE *bracket_list_file = fopen(bracket_list_file_path, "r");
	if (bracket_list_file == NULL) {
		perror("fopen (run_brackets)");
		return -1;
	}

	short latest_season_id = -1;
	char line[MAX_FILE_PATH_LEN + 2];
	/* Get the latest season from a recent player */
	/* search for a player with a non -1 season */
	DIR *p_dir;
	struct dirent *entry;
	/* If the directory could not be accessed, print error and return */
	if ((p_dir = opendir(player_dir)) == NULL) {
		perror("opendir (run_brackets)");
		return -1;
	}

	/* Find highest season number from the player. Note that if
	 * absentee adjustments aren't used then there is no guarantee that
	 * any given player will have the latest season id */
	while ((entry = readdir(p_dir)) != NULL) {
		/* If the directory item is a directory, skip */
		if (0 == check_if_dir(player_dir, entry->d_name)) continue;
		char *full_player_path = player_dir_file_path_with_player_dir(entry->d_name);
		if (access(full_player_path, R_OK | W_OK) == -1) {
			fprintf(stderr, ERROR_PLAYER_DNE);
			return -1;
		}
		struct entry temp;
		if (0 == (entry_file_read_last_entry(full_player_path, &temp))) {
			if (temp.season_id > latest_season_id) {
				latest_season_id = temp.season_id;
			}
		}
		free(full_player_path);

	}
	closedir(p_dir);

	while (fgets(line, sizeof(line), bracket_list_file)) {
		/* Remove comments from text */
		char parse_line = 0;
		unsigned long i = 0;
		while (i < strlen(line)) {
			/* If it reaches a non-whitespace character, check if it's
			 * a comment */
			if (line[i] != '	' && line[i] != ' ') {
				/* If there is space for the comment symbol in the
				 * line, continue */
				if (i + strlen(COMMENT_SYMBOL) <= strlen(line)) {
					/* If there is a comment, do not parse the line */
					if (strncmp(&line[i], COMMENT_SYMBOL, strlen(COMMENT_SYMBOL)) == 0) {
						line[i] = '\0';
						break;
					}
				}
			}
			i++;
		}
		/* Check if the line is non-empty */
		i = 0;
		while (i < strlen(line)) {
			/* If it reaches a non-whitespace character */
			if (line[i] != '	' && line[i] != ' ' && line[i] != '\n' && line[i] != '\r') {
				parse_line = 1;
				break;
			}
			i++;
		}
		if (parse_line == 1) {
			/* Code to catch all several forms of newline such as:
			 * '\n', "\r\n", '\r', "\n\m". Actually catches "[\n\r].*"
			 * If the search for a newline didn't fail, the line ended in a newline
			 * which must be replaced */
			char *end_of_line = strchr(line, '\n');
			if (end_of_line != NULL) {
				*end_of_line = '\0';
			} else {
				end_of_line = strchr(line, '\r');
				if (end_of_line != NULL) {
					*end_of_line = '\0';
				}
			}
			if (p_flag_used == 1) {
				char run_input = '\0';
				fprintf(stdout, "run \"%s\"? [Y/n] ", line);
				scanf(" %c", &run_input);
				if (run_input == 'Y') {
					if (use_games == 1) {
						fprintf(stdout, "running \"%s\" using games ...", line);
					} else {
						fprintf(stdout, "running \"%s\" ...", line);
					}
					update_players(line, latest_season_id + 1);
					fprintf(stdout, "DONE\n");
				} else if (run_input == 'n') {
						fprintf(stdout, "Skipping...\n");
				} else {
					fprintf(stderr, "ERROR: input not recognized\n");
				}
			} else {
				if (use_games == 1) {
					fprintf(stdout, "running \"%s\" using games ...", line);
				} else {
					fprintf(stdout, "running \"%s\" ...", line);
				}
				update_players(line, latest_season_id + 1);
				fprintf(stdout, "DONE\n");
			}
		}
	}

	fclose(bracket_list_file);
	return 0;
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
// TODO: Combine with generate_ratings_file_full once it has been divided
int generate_ratings_file(char* file_path, char* output_file_path) {
	FILE *players = fopen(file_path, "r");
	if (players == NULL) {
		perror("fopen (generate_ratings_file)");
		return -1;
	}

	if (flag_output_to_stdout == 0) {
		clear_file(output_file_path);
	}

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
		(struct entry *)malloc(sizeof(struct entry) * pr_entries_size);
	struct entry temp;

	while (fgets(line, sizeof(line), players)) {
		/* Replace newline with null terminator */
		char *end_of_line = strchr(line, '\n');
		if (end_of_line == NULL) {
			perror("strchr (generate_ratings_file)");
			return -2;
		}
		*end_of_line = '\0';
		char *full_player_path = player_dir_file_path_with_player_dir(line);
		/* If player in filter file does NOT have a player file */
#ifdef __linux__
		if (access(full_player_path, R_OK | W_OK) == -1) {
#elif _WIN32
		if (_access(full_player_path, R_OK | W_OK) == -1) {
#else
		if (access(full_player_path, R_OK | W_OK) == -1) {
#endif
			continue;
		}

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
					players_pr_entries = (struct entry *) \
						realloc(players_pr_entries, \
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

/* Takes a file path, clears the contents of the file and writes
 * player rating data for every player in the player directory
 * 'player_dir', one line per player, for all players who pass
 * any applicable filters.
 *
 * \param '*output_file_path' the file path to create the
 *     ratings file at.
 * \return an int representing if the function succeeded or failed.
 *     Negative on failure, 0 upon success.
 */
// TODO: holy moly divide this function into it's parts. One for
//       making the list of struct records, one for printing at least.
int generate_ratings_file_full(char *output_file_path) {
	DIR *p_dir;
	struct dirent *entry;
	if ((p_dir = opendir(player_dir)) != NULL) {
		if (flag_output_to_stdout == 0) {
			clear_file(output_file_path);
		}

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
			(struct entry *)malloc(sizeof(struct entry) * pr_entries_size);
		struct entry temp;

		while ((entry = readdir(p_dir)) != NULL) {
			// Make sure it doesn't count directories
			if (1 == check_if_dir(player_dir, entry->d_name)) {
				char *full_player_path = player_dir_file_path_with_player_dir(entry->d_name);
				/* If the player file was able to be read properly... */
				if (0 == entry_file_read_last_entry(full_player_path, &temp)) {
					int num_events = \
						entry_file_number_of_events(full_player_path);
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
							players_pr_entries = (struct entry *) \
								realloc(players_pr_entries, \
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

	char *full_player1_path = player_dir_file_path_with_player_dir(player1);
	/* Read the starter data in the file */
	struct entry ent;
	entry_file_read_start_from_file(full_player1_path, &ent);

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

	int num_ent = entry_file_get_number_of_outcomes_against(full_player1_path, player2);
	int cur_opp_ent_num = 0;
	unsigned long num_of_last_outcomes = sizeof(ret->last_outcomes) - 1;

	/* Get to the entries in the player file */
	int r = entry_file_get_to_entries(p_file);
	if (r != 0) {
		perror("get_record (entry_file_get_to_entries)");
		return -2;
	}

	while (entry_file_read_entry(p_file, &ent) == 0) {
		/* If the opponent for the given entry is the player of interest */
		if (0 == strcmp(ent.opp_name, player2)) {
			if (ent.gc > ent.opp_gc) ret->wins += 1;
			else if (ent.gc == ent.opp_gc) ret->ties += 1;
			else if (ent.gc < ent.opp_gc) ret->losses += 1;
			/* If the current entry is one of the last x many,
			 * add it to the recent outcome list */
			if ((unsigned long)(num_ent - cur_opp_ent_num)
				< num_of_last_outcomes) {

				if (ent.gc > ent.opp_gc) {
					ret->last_outcomes[cur_opp_ent_num] = 'W';
				} else if (ent.gc == ent.opp_gc) {
					ret->last_outcomes[cur_opp_ent_num] = 'T';
				/* Assert: (ent.gc < ent.opp_gc) */
				} else {
					ret->last_outcomes[cur_opp_ent_num] = 'L';
				}
			}
			cur_opp_ent_num++;
		}
	}
	ret->last_outcomes[cur_opp_ent_num] = '\0';
	free(full_player1_path);
	fclose(p_file);

	return 0;
}

/* Takes a path to a player file and a pointer to an int.
 * Reads the player file, creates an array of 'struct record's, one
 * for every player player1 has ever played, containing player1's wins, ties,
 * losses, etc. and returns a pointer to said array. Modifies '*num_of_records'
 * to contain the number of elements in that array.
 *
 * \param '*file_path' A file path to an player-file.
 * \param '*num_of_records' An int pointer that will be moified to contain
 *     the number of records in the returned array.
 * \return a pointer to a 'struct record' that is an array of 'struct record's
 *     indexed by 'opp_id'. Returns NULL on failure.
 */
struct record *get_all_records(char *file_path, int *num_of_records) {

	*num_of_records = entry_file_number_of_opponents(file_path);
	struct record *ret = (struct record *)malloc(sizeof(struct record) * *num_of_records);
	/* Read the starter data in the file */
	struct entry ent;
	entry_file_read_start_from_file(file_path, &ent);

	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (get_all_records)");
		return NULL;
	}

	for (int i = 0; i < *num_of_records; i++) {
		strncpy(ret[i].name, ent.name, MAX_NAME_LEN);
		ret[i].wins = 0;
		ret[i].losses = 0;
		ret[i].ties = 0;
	}

	// CONSIDER: get number of entries for every opponent (long array) (only if verbose?)
	long *num_outcome_all = entry_file_get_all_number_of_outcomes_against(file_path);
	// Array containing the current number of entries for a given opponent
	int *cur_opp_ent_num = calloc(*num_of_records, sizeof(int));
	struct record temp;
	unsigned long num_of_last_outcomes = sizeof(temp.last_outcomes) - 1;

	/* Get to the entries in the player file */
	int r = entry_file_get_to_entries(p_file);
	if (r != 0) {
		perror("get_all_records (entry_file_get_to_entries)");
		free(num_outcome_all);
		free(cur_opp_ent_num);
		return NULL;
	}

	short prev_entrys_season = 0;
	while (entry_file_read_entry(p_file, &ent) == 0) {
		// CONSIDER: OPT: replace this triple check every entry with a function
		//            that sets names once.
		/* If this is the first time updating a player's record */
		if (ret[ent.opp_id].wins == 0
			&& ret[ent.opp_id].ties == 0
			&& ret[ent.opp_id].losses == 0) {

			// TODO: actually get player2 name from their file.
			// '*player2' is just a file name
			strncpy(ret[ent.opp_id].opp_name, ent.opp_name, MAX_NAME_LEN);
		}

		/* If the entry is a non-competitor (RD-adjustment), ignore */
		if (ent.is_competitor == 0) continue;
		if (ent.gc > ent.opp_gc) ret[ent.opp_id].wins += 1;
		else if (ent.gc == ent.opp_gc) ret[ent.opp_id].ties += 1;
		else if (ent.gc < ent.opp_gc) ret[ent.opp_id].losses += 1;

		/* If the season changed, add season markers to output strings */
		if (ent.season_id != prev_entrys_season) {
			for (int i = 0; i < *num_of_records; i++) {
				ret[i].last_outcomes[cur_opp_ent_num[i]] = '|';
				cur_opp_ent_num[i]++;
			}
			prev_entrys_season = ent.season_id;
		}
		/* If the current entry is one of the last x many,
		 * add it to the recent outcome list */
		if ((unsigned long)(num_outcome_all[ent.opp_id] - cur_opp_ent_num[ent.opp_id] + prev_entrys_season)
			< num_of_last_outcomes) {

			if (ent.gc > ent.opp_gc) {
				ret[ent.opp_id].last_outcomes[cur_opp_ent_num[ent.opp_id]] = 'W';
			} else if (ent.gc == ent.opp_gc) {
				ret[ent.opp_id].last_outcomes[cur_opp_ent_num[ent.opp_id]] = 'T';
			/* Assert: (ent.gc < ent.opp_gc) */
			} else {
				ret[ent.opp_id].last_outcomes[cur_opp_ent_num[ent.opp_id]] = 'L';
			}
		}
		cur_opp_ent_num[ent.opp_id]++;
	}
	for (int i = 0; i < *num_of_records; i++) {
		ret[i].last_outcomes[cur_opp_ent_num[i]] = '\0';
	}
	free(num_outcome_all);
	free(cur_opp_ent_num);
	fclose(p_file);

	return ret;
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
 * \param '*filter_file_path' the file path of a pr list file.
 * \return an integer representing the success or failure of
 *     this function. 0 Means sucess, negative numbers mean failure.
 */
int filter_player_list(char **players_pointer, int *num_players, \
	char *filter_file_path) {

	FILE *filter_file = fopen(filter_file_path, "r");
	if (filter_file == NULL) {
		perror("fopen (filter_player_list)");
		return -1;
	}

	int app_ind = 0;
	char line[MAX_NAME_LEN];
	char *players = *(players_pointer);
	char *filtered_players = \
		(char *)malloc(sizeof(char) * MAX_NAME_LEN * (*num_players));

	while (fgets(line, sizeof(line), filter_file)) {
		/* Replace newline with null terminator */
		char *end_of_line = strchr(line, '\n');
		if (end_of_line == NULL) {
			perror("strchr (filter_player_list)");
			return -2;
		}
		*end_of_line = '\0';

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

int main(int argc, char **argv) {


	int opt;
	struct option opt_table[] = {
		/* Don't make RD adjustments for players absent
		 * from some tournaments */
		{ "no-adjustment",	no_argument,		NULL,	'0' },
		/* Add (or create if necessary) a player entry/player entry file
		 * from user input */
		{ "events-attended",required_argument,	NULL,	'A' },
		/* Run through a given bracket file making the necessary updates
		 * to the glicko2 scores */
		{ "bracket",		required_argument,	NULL,	'b' },
		{ "brackets",		required_argument,	NULL,	'B' },
		{ "count-outcomes",	required_argument,	NULL,	'c' },
		{ "matchup-csv",	required_argument,	NULL,	'C' },
		{ "player-dir",		required_argument,	NULL,	'd' },
		{ "reset-players",		no_argument,	NULL,	'e' },
		{ "filter",		required_argument,	NULL,	'f' },
		{ "use-games",		no_argument,		NULL,	'g' },
		/* Output given player file in human readable form */
		{ "human",			required_argument,	NULL,	'h' },
		/* Don't delete the player files when running a new bracket */
		{ "keep-players",	no_argument,		NULL,	'k' },
		/* Output last entry in given player file in human readable form */
		{ "min-events",		required_argument,	NULL,	'm' },
		{ "matchup-table",	required_argument,	NULL,	'M' },
		{ "no-colour",		required_argument,	NULL,	'n' },
		{ "no-ties",		required_argument,	NULL,	'N' },
		{ "output",			required_argument,	NULL,	'o' },
		{ "stdout",			no_argument,	NULL,	'O' },
		{ "prompt",				no_argument,	NULL,	'p' },
		{ "P",				required_argument,	NULL,	'P' },
		{ "refactor",		required_argument,	NULL,	'r' },
		{ "records",		required_argument,	NULL,	'R' },
		{ "time",			no_argument,		NULL,	't' },
		{ "verbose",		no_argument,		NULL,	'v' },
		{ "weight",			required_argument,	NULL,	'w' },
		{ 0, 0, 0, 0 }
	};
	char opt_string[] = { "0A:b:B:c:Cd:ef:gh:km:MnNo:OpP:r:R:tvw:" };

	/* 1. Initialize player_dir to the file path for the player directory */
	memset(player_dir, 0, sizeof(player_dir));
	strncpy(player_dir, PLAYER_DIR, sizeof(player_dir) - 1);

	clock_t t;
	t = clock();

	while ((opt = getopt_long(argc, argv, opt_string, opt_table, NULL)) != -1) {
		if (opt == 'A') {
			if (0 == player_dir_check_and_create()) {
				int count;
				char *full_player_path = player_dir_file_path_with_player_dir(optarg);
				char *attended = \
					entry_file_get_events_attended(full_player_path, &count);
				print_player_attended(attended, count);
				free(full_player_path);
				free(attended);
			} else {
				fprintf(stderr, "Error: 'player_dir' either could not be created" \
						"or does not exist");
			}
		} else if (opt == 'c') {
			if (0 == player_dir_check_and_create()) {
				char *full_player_path = player_dir_file_path_with_player_dir(optarg);
				fprintf(stdout, "%d\n", \
					entry_file_get_outcome_count(full_player_path));
				free(full_player_path);
			} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
		} else if (opt == 'd') {
			memset(player_dir, 0, sizeof(player_dir));
			strncpy(player_dir, optarg, sizeof(player_dir) - 1);
		} else if (opt == 'h') {
			if (0 == player_dir_check_and_create()) {
				char *full_player_path = player_dir_file_path_with_player_dir(optarg);
				if (access(full_player_path, R_OK | W_OK) == -1) {
					fprintf(stderr, ERROR_PLAYER_DNE);
					return -1;
				}
				if (verbose == 1) print_player_file_verbose(full_player_path);
				else print_player_file(full_player_path);
				free(full_player_path);
			} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
		} else if (opt == 'r') {
			if (0 == player_dir_check_and_create()) {
				char *full_player_path = player_dir_file_path_with_player_dir(optarg);
				if (access(full_player_path, R_OK | W_OK) == -1) {
					fprintf(stderr, ERROR_PLAYER_DNE);
					return -1;
				}
				entry_file_refactor_name(full_player_path);
				free(full_player_path);
			} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
		} else if (opt == 'R') {
			if (0 == player_dir_check_and_create()) {
				char *full_player_path = player_dir_file_path_with_player_dir(optarg);
				if (access(full_player_path, R_OK | W_OK) == -1) {
					fprintf(stderr, ERROR_PLAYER_DNE);
					return -1;
				}
				print_player_records(full_player_path);
				free(full_player_path);
			} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
		}

		switch (opt) {
			case '0': calc_absent_players = 0; break;
			case 'b':
				if (0 == player_dir_check_and_create()) {
					if (keep_players == 0) player_dir_reset_players();
					run_single_bracket(optarg);
				} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
				break;
			case 'C':
				if (0 == player_dir_check_and_create()) {
					print_matchup_table_csv(); break;
				} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
			case 'B':
				if (0 == player_dir_check_and_create()) {
					if (keep_players == 0) player_dir_reset_players();
					run_brackets(optarg);
				} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
				break;
			case 'e':
				if (0 == player_dir_check_and_create()) {
					player_dir_reset_players();
				} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
				break;
			case 'f':
				f_flag_used = 1;
				strncpy(filter_file_path, optarg, \
					sizeof(filter_file_path) - 1);
				break;
			case 'g': use_games = 1; break;
			case 'k': keep_players = 1; break;
			case 'm': pr_minimum_events = atoi(optarg); break;
			case 'M': print_matchup_table(); break;
			case 'n': colour_output = 0; break;
			case 'N': print_ties = 0; break;
			case 'w': outcome_weight = strtod(optarg, NULL); break;
			case 'p':
				p_flag_used = 1;
				break;
			case 'P':
				calc_absent_players_with_file = 1;
				strncpy(player_list_file, optarg, sizeof(player_list_file) - 1);
				break;
			case 'o':
				if (0 == player_dir_check_and_create()) {
					if (f_flag_used) {
						int ret = generate_ratings_file(filter_file_path, optarg);
						if (ret != 0) return ret;
					} else {
						int ret = generate_ratings_file_full(optarg);
						if (ret != 0) return ret;
					}
				} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
				break;
			case 'O':
				if (0 == player_dir_check_and_create()) {
					flag_output_to_stdout = 1;
					if (f_flag_used) {
						int ret = generate_ratings_file(filter_file_path, optarg);
						if (ret != 0) return ret;
					} else {
						int ret = generate_ratings_file_full(optarg);
						if (ret != 0) return ret;
					}
				} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
				break;
			case 'v': verbose = 1; break;
			case 't': flag_time_program = 1; break;
		}
	}

	if (flag_time_program == 1) {
		t = clock() - t;
		/* Convert to seconds */
		double time_taken = ((double)t)/CLOCKS_PER_SEC;
    	fprintf(stdout, "NOTE: took %lf seconds\n", time_taken);
	}
    return 0;
}
