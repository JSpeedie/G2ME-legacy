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
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
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
char DATA_DIR[] = { ".data/" };
char DIR_TERMINATOR = '/';
#elif _WIN32
char PLAYER_DIR[] = { ".players\\" };
char DATA_DIR[] = { ".data\\" };
char DIR_TERMINATOR = '\\';
#else
char PLAYER_DIR[] = { ".players/" };
char DATA_DIR[] = { ".data/" };
char DIR_TERMINATOR = '/';
#endif

const char ERROR_PLAYER_DIR_DNE[] = { "Error: 'player_dir' either could not be "
	"created or does not exist\n"};
const char ERROR_PLAYER_DNE[] = { "Error: the given player could not be found "
	"in working directory or the given player directory\n"};

char flag_output_to_stdout = 0;
char verbose = 0;
char use_games = 0;
char keep_players = 0;
int pr_minimum_events = 0;
char colour_output = 1;
char print_ties = 1;
char player_list_file[MAX_FILE_PATH_LEN];
char calc_absent_players = 1;
double outcome_weight = 1;
struct tournament_attendee *tourn_atten;
unsigned long tourn_atten_len = 0;
unsigned long tourn_atten_size = SIZE_TOURNAMENT_NAMES_LEN;
char filter_file_path[MAX_OUTCOME_STRING_LEN];
char f_flag_used = 0;
char player_dir[MAX_FILE_PATH_LEN];
char data_dir[MAX_FILE_PATH_LEN];

int get_record(char *, char *, struct record *);
struct record *get_all_records(char *, long *);

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
 * \param 't_id' a short representing the tournament id of the tournament
 *     this outcome took place at.
 * \param 't_name' a string containing the name of the tournament this outcome
 *     took place at.
 * \return void
 */
void update_player_on_outcome(short p1_id, char* p1_name, short p2_id, char* p2_name, \
	struct player* p1, struct player* p2, char* p1_gc, char* p2_gc, \
	char day, char month, short year, short t_id, char* t_name, short season_id) {

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
		int ret;
		struct entry p1_latest;
		if (0 == (ret = entry_file_read_last_entry_minimal(full_p1_path, &p1_latest))) {
			init_player_from_entry(p1, &p1_latest);
			/* If this outcome was not a part of a season, write the season
			 * as the same as the latest season the player was in */
			if (season_id == -1) {
				season_id = p1_latest.season_id;
			}
		} else {
			printf("entry_file_read_last_entry (%d) (update_player_on_outcome)", ret);
			perror("");
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
		int ret;
		struct entry p2_latest;
		if (0 == (ret = entry_file_read_last_entry_minimal(full_p2_path, &p2_latest))) {
			init_player_from_entry(p2, &p2_latest);
		} else {
			printf("entry_file_read_last_entry (%d) (update_player_on_outcome)", ret);
			perror("");
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
	p1_new_entry.opp_id = p2_id;
	p1_new_entry.tournament_id = t_id;
	int ret = entry_file_append_entry_to_file_id(&p1_new_entry, full_p1_path);
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
 * \param '*t_id' the id of the event that they were absent from.
 * \param '*t_name' the name of the event that they were absent from.
 */
void adjust_absent_player(char *player_file, char day, char month, short year, \
	short t_id, char *t_name) {

	char* full_file_path = player_dir_file_path_with_player_dir(player_file);

	/* If the player who did not compete has a player file */
#ifdef __linux__
	if (access(full_file_path, R_OK | W_OK) != -1) {
#elif _WIN32
	//if (_access(full_file_path) != -1) {
	if (access(full_file_path, R_OK | W_OK) != -1) {
#else
	if (access(full_file_path, R_OK | W_OK) != -1) {
#endif
		struct player P;
		struct entry latest_ent;
		if (0 == entry_file_read_last_entry_absent(full_file_path, &latest_ent)) {

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
				 * did_not_compete. opp_id of 0 is the opp_id of "-" */
				latest_ent.opp_id = 0;
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
				latest_ent.tournament_id = t_id;
				strncpy(latest_ent.t_name, t_name, MAX_NAME_LEN);
				latest_ent.len_t_name = strlen(latest_ent.t_name);
				latest_ent.t_name[latest_ent.len_t_name] = '\0';
				entry_file_append_adjustment_to_file_id(&latest_ent, full_file_path);
			}
		}
	}
	free(full_file_path);
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
 * \param '*t_id' a short representing the id of the tournament.
 * \param '*t_name' a string containing the name of the tournament.
 * \return void.
*/
void adjust_absent_players_no_file(char day, char month, \
	short year, short t_id, char* t_name) {

	DIR *p_dir;
	struct dirent *entry;
	/* If the directory could not be accessed, print error and return */
	if ((p_dir = opendir(player_dir)) == NULL) {
		perror("opendir (adjust_absent_players_no_file)");
		closedir(p_dir);
		return;
	}

	int max_forks;
/* Set the max number of forks to the number of processors available */
#ifdef _WIN32
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	max_forks = info.dwNumberOfProcessors;
#else
	max_forks = sysconf(_SC_NPROCESSORS_ONLN);
#endif
	if (max_forks < 1) max_forks = 8;
	int num_players = 0;
	player_dir_num_players(&num_players);
	char file_names[num_players][MAX_NAME_LEN + 1];
	int total_num_adjustments = 0;
	char did_not_comp = 1;

	/* Create a list of player files */
	while ((entry = readdir(p_dir)) != NULL) {
		/* Reset variable to assume player did not compete */
		did_not_comp = 1;
		/* If the directory item is a directory, skip */
		if (0 == check_if_dir(player_dir, entry->d_name)) continue;

		/* Binary search on file to find given name's id */
		long L = 0;
		long R = tourn_atten_len - 1;
		long m;
		while (L <= R) {
			m = floor(((double) (L + R)) / 2.0);
			/* Compare array[m] with the name being searched for */
			int comp = strncmp(&tourn_atten[m].name[0], entry->d_name, MAX_NAME_LEN);
			if (0 > comp) {
				L = m + 1;
			} else if (0 < comp) {
				R = m - 1;
			} else {
				did_not_comp = 0;
				R = L - 1; /* Terminate loop */
			}
		}

		if (did_not_comp) {
			strncpy(&file_names[total_num_adjustments][0], entry->d_name, MAX_NAME_LEN + 1);
			total_num_adjustments++;
		}
	}

	closedir(p_dir);
	unsigned long adj_per_process = total_num_adjustments / max_forks;
	unsigned long extra = total_num_adjustments - ((total_num_adjustments / max_forks) * max_forks);
	/* Calculate the appropriate maximum size for the array of names to
	 * be adjusted */
	unsigned long size_of_names = MINIMUM_ADJ_BEFORE_FORK;
	if (size_of_names < adj_per_process + extra) {
		size_of_names = adj_per_process + extra;
	}
	pthread_t thread_id[max_forks - 1];


	/* Define a struct for passing arguments to the thread */
	typedef struct thread_args {
		unsigned long num_adjustments;
		char files[size_of_names][MAX_NAME_LEN + 1];
	}ThreadArgs;

	ThreadArgs args[max_forks - 1];

	/* Function for adjusting a list of players */
	void *adjust_p(void *arg) {
		struct thread_args *t = (struct thread_args *) arg;
		/* Get this processes' list of player names that did not compete and
		 * apply step 6 to them and append to player file */
		for (int j = 0; j < t->num_adjustments; j++) {
			adjust_absent_player(&(t->files[j][0]), day, month, year, t_id, t_name);
		}
		return NULL;
	}

	/* If there is reason not to use every thread */
	if (adj_per_process < MINIMUM_ADJ_BEFORE_FORK) {
		int num_min_threads = total_num_adjustments / MINIMUM_ADJ_BEFORE_FORK;
		for (int f = 0; f < num_min_threads; f++) {
			/* Copy arguments need for player adjustment into argument struct */
			args[f].num_adjustments = MINIMUM_ADJ_BEFORE_FORK;
			for (int k = 0; k < MINIMUM_ADJ_BEFORE_FORK; k++) {
				strncpy(&args[f].files[k][0], &file_names[(f * MINIMUM_ADJ_BEFORE_FORK) + k][0], MAX_NAME_LEN);
			}
			/* Create new thread, RD adjusting list of players */
			pthread_create(&thread_id[f], NULL, adjust_p, &args[f]);
		}
		/* Player adjustment for parent thread. Catches stragglers
		 * through the use of 'extra' */
		extra = total_num_adjustments - (num_min_threads * MINIMUM_ADJ_BEFORE_FORK);
		if (extra > 0) {
			struct thread_args parent_arg;
			parent_arg.num_adjustments = extra;
			for (int k = 0; k < parent_arg.num_adjustments; k++) {
				strncpy(&parent_arg.files[k][0], &file_names[((num_min_threads) * MINIMUM_ADJ_BEFORE_FORK) +  k][0], MAX_NAME_LEN);
			}
			adjust_p(&parent_arg);
		}

		/* Wait for all threads to finish */
		for (int f = 0; f < num_min_threads; f++) {
			pthread_join(thread_id[f], NULL);
		}
	/* If there are more than 'MINIMUM_ADJ_BEFORE_FORK' adjustments per
	 * thread, each thread has lots of work. Create a maximum
	 * number of threads */
	} else {
		for (int f = 0; f < max_forks - 1; f++) {
			/* Copy arguments need for player adjustment into argument struct */
			args[f].num_adjustments = adj_per_process;
			for (int k = 0; k < adj_per_process; k++) {
				strncpy(&args[f].files[k][0], &file_names[(f * adj_per_process) + k][0], MAX_NAME_LEN);
			}
			/* Create new thread, RD adjusting list of players */
			pthread_create(&thread_id[f], NULL, adjust_p, &args[f]);
		}


		/* Player adjustment for parent thread. Catches stragglers
		 * through the use of 'extra' */
		struct thread_args parent_arg;
		parent_arg.num_adjustments = adj_per_process + extra;
		for (int k = 0; k < parent_arg.num_adjustments; k++) {
			strncpy(&parent_arg.files[k][0], &file_names[((max_forks - 1) * adj_per_process) +  k][0], MAX_NAME_LEN);
		}
		adjust_p(&parent_arg);

		/* Wait for all threads to finish */
		for (int f = 0; f < max_forks - 1; f++) {
			pthread_join(thread_id[f], NULL);
		}
	}
}


/** Takes a bracket file and updates the ratings of all the players
 * mentioned in the bracket file as well as performing an RD adjustment
 * on players who were absent.
 *
 * \param '*bracket_file_path' the file path of a bracket file which, on
 *     each line has the format of "[p1_name] [p2_name] [p1_game_count]
 *     [p2_game_count] [day] [month] [year]"
 * \return 0 upon sucess, < 0 upon failure.
 */
// TODO: break up function.
// TODO: make sscanfs safe/secure.
//       Right now they risk a stack/buffer overflow
int update_players(char* bracket_file_path, short season_id) {
	/* Set to 0 since the bracket is beginning and no names are stored */
	tourn_atten_len = 0;
	tourn_atten = \
		(struct tournament_attendee *)malloc((sizeof(struct tournament_attendee)) * tourn_atten_size);

	FILE *bracket_file = fopen(bracket_file_path, "r");
	if (bracket_file == NULL) {
		perror("fopen (update_players)");
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

	struct entry Et;
	strncpy(Et.t_name, t_name, sizeof(t_name));
	Et.len_t_name = strlen(Et.t_name);
	int ret = 0;
	/* If the entry file does not already contain an id for this tournament */
	if (-1 == (ret = t_file_contains_tournament(Et.t_name))) {
		/* Add the new tournament to the entry file. This also corrects
		 * the t_id if it is incorrect */
		if (0 != t_file_add_new_tournament(&Et)) return -8;
	/* If there was an error */
	} else if (ret < -1) {
		return -9;
	/* If the entry file does contain an id for this tournament */
	} else {
		/* Fix the tournament_id in case it wasn't set */
		Et.tournament_id = (unsigned short) ret;
	}

	int outcomes_size = SIZE_OUTCOMES;
	char *outcomes = \
		(char *)malloc((MAX_FILE_PATH_LEN + 1) * outcomes_size);

	int num_outcomes = 0;
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
			/* If there is no space to add this bracket path, reallocate */
			if (num_outcomes + 1 > outcomes_size) {
				outcomes_size += REALLOC_OUTCOMES_INC;
				outcomes = (char *) realloc(outcomes, \
					(MAX_FILE_PATH_LEN + 1) * outcomes_size);
				if (outcomes == NULL) {
					perror("realloc (update_players)");
					return -2;
				}
			}
			strncpy(&outcomes[num_outcomes * (MAX_FILE_PATH_LEN + 1)], \
				&line[0], MAX_FILE_PATH_LEN);
			num_outcomes++;
		}
	}

	fclose(bracket_file);

	for (int j = 0; j < num_outcomes; j++) {
		/* Read data from one line of bracket file into all the variables */
#ifdef __linux__
		sscanf(&outcomes[j * (MAX_FILE_PATH_LEN + 1)], \
			"%s %s %hhd %hhd %hhd %hhd %hd", \
			p1_name, p2_name, &p1_gc, &p2_gc, &day, &month, &year);
#elif _WIN32

		char *token = \
			strtok(&outcomes[j * (MAX_FILE_PATH_LEN + 1)], " ");
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
		sscanf(&outcomes[j * (MAX_FILE_PATH_LEN + 1)], \
			"%s %s %hhd %hhd %hhd %hhd %hd", \
			p1_name, p2_name, &p1_gc, &p2_gc, &day, &month, &year);
#endif
		char p1_found = 0;
		char p2_found = 0;
		unsigned long p1_index;
		unsigned long p2_index;
		if (calc_absent_players == 1) {
			char already_in = 0;
			char already_in2 = 0;
			// TODO: binary search insert? (can't search yet, it isn't sorted)
			for (int i = 0; i < tourn_atten_len; i++) {
				/* If the name already exists in the list of entrants,
				 * don't add */
				if (0 == strcmp(p1_name, &tourn_atten[i].name[0])) {
					already_in = 1;
					p1_found = 1;
					p1_index = i;
					/* If both names need to be added, get to work. No need
					 * to search further */
					if (already_in2 == 1) {
						break;
					}
				}
				if (0 == strcmp(p2_name, &tourn_atten[i].name[0])) {
					already_in2 = 1;
					p2_found = 1;
					p2_index = i;
					/* If both names need to be added, get to work. No need
					 * to search further */
					if (already_in == 1) {
						break;
					}
				}
			}
			int ret = 0;
			if (!already_in) {
				if (tourn_atten_len + 1 > tourn_atten_size) {
					tourn_atten_size *= REALLOC_TOURNAMENT_NAMES_FACTOR;
					tourn_atten = (struct tournament_attendee *) realloc(tourn_atten, \
						(sizeof(struct tournament_attendee)) * tourn_atten_size);
					if (tourn_atten == NULL) {
						perror("realloc (update_players)");
						return -2;
					}
				}
				strncpy(&tourn_atten[tourn_atten_len].name[0], \
					p1_name, MAX_NAME_LEN);

				/* Get opp_ids for all players who attended this tournament */
				/* If the entry file does not already contain an id for this opponent */
				struct entry E;
				if (-1 == (ret = \
					opp_file_contains_opponent(&tourn_atten[tourn_atten_len].name[0]))) {
					/* Add the new opponent to the entry file. This also corrects
					 * the t_id if it is incorrect */
					strncpy(&E.opp_name[0], &tourn_atten[tourn_atten_len].name[0], MAX_NAME_LEN);
					E.len_opp_name = strlen(E.opp_name);
					if (0 != opp_file_add_new_opponent(&E)) return -8;
				/* If there was an error */
				} else if (ret < -1) {
					return -9;
				/* If the entry file does contain an id for this opponent */
				} else {
					/* Fix the opp_id in case it wasn't set */
					E.opp_id = (unsigned short) ret;
				}
				tourn_atten[tourn_atten_len].id = E.opp_id;
				tourn_atten_len++;
			}
			if (!already_in2) {
				if (tourn_atten_len + 1 > tourn_atten_size) {
					tourn_atten_size *= REALLOC_TOURNAMENT_NAMES_FACTOR;
					tourn_atten = (struct tournament_attendee *) realloc(tourn_atten, \
						sizeof(struct tournament_attendee) * tourn_atten_size);
					if (tourn_atten == NULL) {
						perror("realloc (update_players)");
						return -2;
					}
				}
				strncpy(&tourn_atten[tourn_atten_len].name[0], \
					p2_name, MAX_NAME_LEN);
				/* Get opp_ids for all players who attended this tournament */
				/* If the entry file does not already contain an id for this opponent */
				struct entry E;
				if (-1 == (ret = \
					opp_file_contains_opponent(&tourn_atten[tourn_atten_len].name[0]))) {
					/* Add the new opponent to the entry file. This also corrects
					 * the t_id if it is incorrect */
					strncpy(&E.opp_name[0], &tourn_atten[tourn_atten_len].name[0], MAX_NAME_LEN);
					E.len_opp_name = strlen(E.opp_name);
					if (0 != opp_file_add_new_opponent(&E)) return -8;
				/* If there was an error */
				} else if (ret < -1) {
					return -9;
				/* If the entry file does contain an id for this opponent */
				} else {
					/* Fix the opp_id in case it wasn't set */
					E.opp_id = (unsigned short) ret;
				}
				tourn_atten[tourn_atten_len].id = E.opp_id;
				tourn_atten_len++;
			}
		}
		short p1_id;
		short p2_id;
		if (p1_found == 1) {
			p1_id = tourn_atten[p1_index].id;
		}
		if (p2_found == 1) {
			p2_id = tourn_atten[p2_index].id;
		}
		if (p1_found == 0 || p2_found == 0) {
			// TODO binary search (can do, just divide into 2 searches)
			for (int k = 0; k < tourn_atten_len; k++) {
				if (p1_found == 0) {
					if (0 == strncmp(p1_name, &tourn_atten[k].name[0], MAX_NAME_LEN)) {
						p1_id = tourn_atten[k].id;
						p1_found = 1;
					}
				}
				if (p2_found == 0) {
					if (0 == strncmp(p2_name, &tourn_atten[k].name[0], MAX_NAME_LEN)) {
						p2_id = tourn_atten[k].id;
						p2_found = 1;
					}
				}
				if (p1_found == 1 && p2_found == 1) break;
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
				update_player_on_outcome(p1_id, p1_name, p2_id, p2_name, &p1, &p2, \
					&p1_out, &p2_out, day, month, year, Et.tournament_id, t_name, season_id);
				update_player_on_outcome(p2_id, p2_name, p1_id, p1_name, &p2, &p1, \
					&p2_out, &p1_out, day, month, year, Et.tournament_id, t_name, season_id);
			}
			p1_out = 0;
			p2_out = 1;
			for (int i = 0; i < p2_gc; i++) {
				update_player_on_outcome(p1_id, p1_name, p2_id, p2_name, &p1, &p2, \
					&p1_out, &p2_out, day, month, year, Et.tournament_id, t_name, season_id);
				update_player_on_outcome(p2_id, p2_name, p1_id, p1_name, &p2, &p1, \
					&p2_out, &p1_out, day, month, year, Et.tournament_id, t_name, season_id);
			}
		} else {
			update_player_on_outcome(p1_id, p1_name, p2_id, p2_name, &p1, &p2, \
				&p1_gc, &p2_gc, day, month, year, Et.tournament_id, t_name, season_id);
			update_player_on_outcome(p2_id, p2_name, p1_id, p1_name, &p2, &p1, \
				&p2_gc, &p1_gc, day, month, year, Et.tournament_id, t_name, season_id);
		}
	}

	if (calc_absent_players == 1) {
		merge_sort_tournament_attendees(tourn_atten, tourn_atten_len);
		adjust_absent_players_no_file(day, month, year, Et.tournament_id, t_name);
	}

	return 0;
}


/* Takes a file path to a bracket file, and runs the bracket.
 *
 * \param '*bracket_file_path' a file path to a bracket file.
 * \return an int representing if the function succeeded or failed.
 *     Negative on failure. 0 upon success.
 */
int run_single_bracket(char *bracket_file_path) {
	if (use_games == 1) {
		fprintf(stdout, "running \"%s\" using games ...", bracket_file_path);
		fflush(stdout);
	} else {
		fprintf(stdout, "running \"%s\" ...", bracket_file_path);
		fflush(stdout);
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

/* Takes a file path to a bracket list file. For each line in the file,
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
	char line[MAX_FILE_PATH_LEN + 2]; /* + 1 for \n and +1 for \0 */
	int bracket_paths_size = SIZE_BRACKET_PATHS;
	char *bracket_paths = \
		(char *)malloc((MAX_FILE_PATH_LEN + 1) * bracket_paths_size);


	int num_brk = 0;
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

			/* If there is no space to add this bracket path, reallocate */
			if (num_brk + 1 > bracket_paths_size) {
				bracket_paths_size += REALLOC_BRACKET_PATHS_INC;
				bracket_paths = (char *) realloc(bracket_paths, \
					(MAX_FILE_PATH_LEN + 1) * bracket_paths_size);
				if (bracket_paths == NULL) {
					perror("realloc (generate_ratings_file)");
					return -2;
				}
			}
			strncpy(&bracket_paths[num_brk * (MAX_FILE_PATH_LEN + 1)], \
				&line[0], MAX_FILE_PATH_LEN);
			num_brk++;
		}
	}

	for (int j = 0; j < num_brk; j++) {
		if (use_games == 1) {
			fprintf(stdout, "running \"%s\" using games ...", \
				&bracket_paths[j * (MAX_FILE_PATH_LEN + 1)]);
			fflush(stdout);
		} else {
			fprintf(stdout, "running \"%s\" ...", \
				&bracket_paths[j * (MAX_FILE_PATH_LEN + 1)]);
			fflush(stdout);
		}
		update_players(&bracket_paths[j * (MAX_FILE_PATH_LEN + 1)], \
			latest_season_id + 1);
		s_file_set_latest_season_id(latest_season_id + 1);
		fprintf(stdout, "DONE\n");
	}

	fclose(bracket_list_file);
	return 0;
}

/** Creates a file listing all the players' glicko details at the location
 * 'output_file_path'.
 *
 * \param '*file_path' the file path for a file containing, on each line,
 *     the file path of a player file generated by this program.
 * \param '*output_file_path' the file path at which to create the pr
 *     file.
 * \return int representing if the function failed or succeeded. 0 upon
 *     success, < 0 upon failure.
 */
// TODO: Combine with generate_ratings_file_full once it has been divided
int generate_ratings_file(char* filter_file_path, char* output_file_path) {
	FILE *players = fopen(filter_file_path, "r");
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
			// TODO: something here
			int num_events = entry_file_get_events_attended_count(full_player_path);
			if (longest_attended < num_events) longest_attended = num_events;
			int num_outcomes = entry_file_get_outcome_count(full_player_path);
			if (longest_outcomes < num_outcomes) {
				longest_outcomes = num_outcomes;
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
// TODO: holy moly divide this function into its parts. One for
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
					// TODO: finish this, if the m flag isn't used, no need to do all this
					//if (pr_minimum_events > 0) {
					int num_events = \
						entry_file_get_events_attended_count(full_player_path);
					if (longest_attended < num_events) longest_attended = num_events;

					int num_outcomes = entry_file_get_outcome_count(full_player_path);
					if (longest_outcomes < num_outcomes) {
						longest_outcomes = num_outcomes;
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

/* Takes 2 player names and a struct record, modifies the given struct record
 * to be the first players record/head-to-head/matchup on the second player.
 *
 * \param '*player1' the name of the first player.
 * \param '*player2' the name of the second player.
 * \param '*ret' A struct record, which upon the successful completion of this
 *     function, will contain the first player's record against
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
		fprintf(stderr, "Error opening file %s (get_record): ", full_player1_path);
		perror("");
		return -1;
	}
	strncpy(ret->name, ent.name, MAX_NAME_LEN);
	strncpy(ent.opp_name, player2, MAX_NAME_LEN);
	// TODO: actually get player2 name from their file.
	// '*player2' is just a file name
	strncpy(ret->opp_name, player2, MAX_NAME_LEN);
	ret->wins = 0;
	ret->losses = 0;
	ret->ties = 0;

	int num_ent = entry_file_get_number_of_outcomes_against(full_player1_path, player2);
	int cur_opp_ent_num = 0;
	unsigned long num_of_last_outcomes = sizeof(ret->last_outcomes) - 1;

	/* If there was an error with this function */
	if (opp_file_get_id_from_name(&ent) < 0) return -3;

	/* Get to the entries in the player file */
	int r = entry_file_get_to_entries(p_file);
	if (r != 0) {
		perror("get_record (entry_file_get_to_entries)");
		return -2;
	}

	while (entry_file_read_next_opp_entry(p_file, &ent, ent.opp_id) == 0) {
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
 * Reads the player file, creates an array of struct records, one
 * for every player player1 has ever played, containing player1's wins, ties,
 * losses, etc. And returns a pointer to said array. Modifies '*num_of_records'
 * to contain the number of elements in that array.
 *
 * \param '*file_path' A file path to an player-file.
 * \param '*num_of_records' An int pointer that will be moified to contain
 *     the number of records in the returned array.
 * \return a pointer to a 'struct record' that is an array of 'struct record's
 *     indexed by 'opp_id'. Returns NULL on failure.
 */
struct record *get_all_records(char *file_path, long *num_of_records) {

	short *opp_id_list = NULL;
	short **p_opp_id_list = &opp_id_list;
	*num_of_records = entry_file_number_of_opponents(file_path, p_opp_id_list);
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

	/* Get number of entries for every opponent */
	// CONSIDER: only do if verbose? (Only used if verbose)
	long *num_outcome_all = \
		entry_file_get_all_number_of_outcomes_against(file_path, *num_of_records, opp_id_list);
	for (int k = 0; k < *num_of_records; k++) {
		struct entry E2;
		E2.opp_id = opp_id_list[k];
		opp_file_get_name_from_id(&E2);
	}
	/* Array containing the current number of entries for a given opponent */
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
		int j = 0;
		/* Find position (j) of opp_id being searched for */
		for (j = 0; j < *num_of_records; j++) {
			if (ent.opp_id == opp_id_list[j]) {
				break;
			}
		}
		// CONSIDER: OPT: replace this triple check every entry with a fu nction
		//            that sets names once.
		/* if this is the first time updating the player's record */
		if (ret[j].wins == 0 \
			&& ret[j].ties == 0 \
			&& ret[j].losses == 0) {

			// TODO: actually get player2 name from their file.
			// '*player2' is just a file name
			strncpy(ret[j].opp_name, ent.opp_name, MAX_NAME_LEN);
		}

		/* Only update if the player has sets against this opponent */
		if (num_outcome_all[j] > 0) {
			/* If the entry is a non-competitor (RD-adjustment), ignore */
			if (ent.is_competitor == 0) continue;
			if (ent.gc > ent.opp_gc) ret[j].wins += 1;
			else if (ent.gc == ent.opp_gc) ret[j].ties += 1;
			else if (ent.gc < ent.opp_gc) ret[j].losses += 1;

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
			if ((unsigned long)(num_outcome_all[j] - cur_opp_ent_num[j] + prev_entrys_season)
				< num_of_last_outcomes) {

				if (ent.gc > ent.opp_gc) {
					ret[j].last_outcomes[cur_opp_ent_num[j]] = 'W';
				} else if (ent.gc == ent.opp_gc) {
					ret[j].last_outcomes[cur_opp_ent_num[j]] = 'T';
				/* Assert: (ent.gc < ent.opp_gc) */
				} else {
					ret[j].last_outcomes[cur_opp_ent_num[j]] = 'L';
				}
			}
			cur_opp_ent_num[j]++;
		}
	}
	for (int i = 0; i < *num_of_records; i++) {
		ret[i].last_outcomes[cur_opp_ent_num[i]] = '\0';
	}
	free(opp_id_list);
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
		{ "history",			required_argument,	NULL,	'h' },
		/* Don't delete the player files when running a new bracket */
		{ "keep-players",	no_argument,		NULL,	'k' },
		/* Output last entry in given player file in human readable form */
		{ "min-events",		required_argument,	NULL,	'm' },
		{ "matchup-table",	required_argument,	NULL,	'M' },
		{ "no-colour",		required_argument,	NULL,	'n' },
		{ "no-ties",		required_argument,	NULL,	'N' },
		{ "output",			required_argument,	NULL,	'o' },
		{ "stdout",			no_argument,	NULL,	'O' },
		{ "records",		required_argument,	NULL,	'R' },
		{ "verbose",		no_argument,		NULL,	'v' },
		{ "weight",			required_argument,	NULL,	'w' },
		{ 0, 0, 0, 0 }
	};
	char opt_string[] = { "0A:b:B:c:Cd:ef:gh:km:MnNo:OR:vw:" };

	/* 1.1. Initialize player_dir to the file path for the player directory */
	memset(player_dir, 0, sizeof(player_dir));
	strncpy(player_dir, PLAYER_DIR, sizeof(player_dir) - 1);
	/* 1.2. Initialize data_dir to the file path for the data directory */
	memset(data_dir, 0, sizeof(data_dir));
	strncpy(data_dir, DATA_DIR, sizeof(data_dir) - 1);

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
			case 'B':
				if (0 == player_dir_check_and_create()) {
					if (keep_players == 0) player_dir_reset_players();
					run_brackets(optarg);
				} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
				break;
			case 'C':
				if (0 == player_dir_check_and_create()) {
					print_matchup_table_csv(); break;
				} else fprintf(stderr, ERROR_PLAYER_DIR_DNE);
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
			case 'w': outcome_weight = strtod(optarg, NULL); break;
		}
	}

    return 0;
}
