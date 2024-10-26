/* General Includes */
#include <dirent.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
/* Windows includes */
#ifdef _WIN32
#include <io.h>
//#include <windows.h>
//#include <fcntl.h>
#endif
/* Local Includes */
#include "p_files.h"
#include "opp_files.h"
#include "tournament_files.h"
#include "player_dir.h"


int p_file_read_start_from_file(char *, struct entry *);
int p_file_get_outcome_count(char *);
int p_file_get_events_attended_count(char *);
char *p_file_get_events_attended(char *, int *);
double p_file_get_glicko_change_since_last_event(char *);


/* [short | opp_id] [3 double | glicko data]
 * [4 char | game counts and date] [2 short | year and tournament_id] */
long int SIZE_OF_AN_ENTRY = (1 * sizeof(short)) + (3 * sizeof(double)) \
	+ (4 * sizeof(char)) + (3 * sizeof(short));


/** Reads contents of a player file to a struct entry. Returns 0 upon success,
 * and a negative number upon failure. Function expects that starter data
 * has already been passed and that the FILE is on an entry.
 *
 * \param '*f' the file being read.
 * \param '*E' the struct entry which will have the read entry's contents
 *     copied to.
 * \return 0 upon success, or a negative number upon failure.
 */
int p_file_open_read_entry(FILE *f, struct entry *E) {
	// Read opponent name id
	if (1 != fread(&E->opp_id, sizeof(short), 1, f)) { return -1; }
	if (1 != fread(&E->rating, sizeof(double), 1, f)) { return -2; }
	if (1 != fread(&E->RD, sizeof(double), 1, f)) { return -3; }
	if (1 != fread(&E->vol, sizeof(double), 1, f)) { return -4; }
	if (1 != fread(&E->gc, sizeof(char), 1, f)) { return -5; }
	if (1 != fread(&E->opp_gc, sizeof(char), 1, f)) { return -6; }
	if (1 != fread(&E->day, sizeof(char), 1, f)) { return -7; }
	/* If 'day' bitwise ANDed with 10000000 != 0, then this is a
	 * non-competitor entry */
	if ( (E->day & (1 << ((sizeof(E->day) * 8) - 1))) != 0) {
		E->is_competitor = 0;
		/* Set leftmost bit to 0 so it becomes a normal char */
		E->day = E->day &  ~(1 << ((sizeof(E->day) * 8) - 1));
	} else {
		E->is_competitor = 1;
	}
	if (1 != fread(&E->month, sizeof(char), 1, f)) { return -8; }
	if (1 != fread(&E->year, sizeof(short), 1, f)) { return -9; }
	if (1 != fread(&E->tournament_id, sizeof(short), 1, f)) { return -10; }
	if (1 != fread(&E->season_id, sizeof(short), 1, f)) { return -11; }
	/* Sets opp_name and len_opp_name of E to be according to opponent
	 * name E->opp_id */
	int r;
	if (0 != (r = opp_file_get_name_from_id(E))) {
		fprintf(stderr, "Error (%d) on opp_file_get_name_from_id() searching for id (%d)\n", r, E->opp_id);
		return -12;
	}
	/* Sets t_name and len_t_name of E to be according to tournament
	 * name E->tournament_id */
	if (0 != (r = t_file_get_tournament_name_from_id(E))) {
		fprintf(stderr, "Error (%d) on entry_get_tournament_name_from_id() searching for id (%d)\n", r, E->tournament_id);
		return -13;
	}

	return 0;
}


/** Reads contents of a player file to a struct entry. Returns 0 upon success,
 * and a negative number upon failure. Function expects that starter data
 * has already been passed and that the FILE is on an entry
 *
 * \param '*f' the file being read
 * \param '*E' the struct entry to store an entry found in the file too
 * \param 'opp_id' the opp_id being searched for. The function will only
 *     read an entry that has the same opp_id as the one provided.
 * \return 0 upon success, or a negative number upon failure.
 */
int p_file_open_read_next_opp_entry(FILE *f, struct entry *E, short opp_id) {
	/* Set to starter data. No opp_id will ever be negative, just starts
	 * the loop */
	if (1 != fread(&E->opp_id, sizeof(short), 1, f)) { return -1; } //2

	while (E->opp_id != opp_id) {
		/* If the entry isn't for the opp being searched for, skip to next entry */
		if (0 != fseek(f, SIZE_OF_AN_ENTRY - sizeof(short), SEEK_CUR)) {
			return -2;
		}
		if (1 != fread(&E->opp_id, sizeof(short), 1, f)) { return -1; } //2
	}
	if (1 != fread(&E->rating, sizeof(double), 1, f)) { return -3; } //8 10
	if (1 != fread(&E->RD, sizeof(double), 1, f)) { return -4; } // 8 18
	if (1 != fread(&E->vol, sizeof(double), 1, f)) { return -5; } //8 26

	if (1 != fread(&E->gc, sizeof(char), 1, f)) { return -6; } //1 27
	if (1 != fread(&E->opp_gc, sizeof(char), 1, f)) { return -7; } //1 28
	if (1 != fread(&E->day, sizeof(char), 1, f)) { return -8; } //1 29
	/* If 'day' bitwise ANDed with 10000000 != 0, then this is a
	 * non-competitor entry */
	if ( (E->day & (1 << ((sizeof(E->day) * 8) - 1))) != 0) {
		E->is_competitor = 0;
		/* Set leftmost bit to 0 so it becomes a normal char */
		E->day = E->day &  ~(1 << ((sizeof(E->day) * 8) - 1));
	} else {
		E->is_competitor = 1;
	}
	if (1 != fread(&E->month, sizeof(char), 1, f)) { return -9; } //1 30

	if (1 != fread(&E->year, sizeof(short), 1, f)) { return -10; } //2 32
	if (1 != fread(&E->tournament_id, sizeof(short), 1, f)) { return -11; } //2 32
	if (1 != fread(&E->season_id, sizeof(short), 1, f)) { return -12; } //2 34
	/* Sets opp_name and len_opp_name of E to be according to opponent
	 * name E->opp_id */
	int r;
	if (0 != (r = opp_file_get_name_from_id(E))) {
		perror("opp_file_get_name_from_id (read_entry)");
		return -12;
	}
	/* Sets t_name and len_t_name of E to be according to tournament
	 * name E->tournament_id */
	if (0 != (r = t_file_get_tournament_name_from_id(E))) {
		fprintf(stderr, "Error: t_file_get_tournament_name_from_id (%d)", r);
		return -13;
	}

	return 0;
}


/** Reads contents of a player file to a struct entry. Returns 0 upon success,
 * and a negative number upon failure.Function expects that starter data
 * has already been passed and that the FILE is on an entry
 *
 * Note: this function doesn't read the entire entry into the
 * struct entry. It is the minimal version and reads only the glicko2 data
 * plus the season id.
 *
 * \param '*f' the file being read
 * \param '*E' the struct entry to store an entry found in the file too
 * \return 0 upon success, or a negative number upon failure.
 */
int p_file_open_read_entry_minimal(FILE *f, struct entry *E) {
	/* SKip over opp id */
	if (0 != fseek(f, sizeof(short), SEEK_CUR)) { return -1; } //2
	/* Read Glicko2 data */
	if (1 != fread(&E->rating, sizeof(double), 1, f)) { return -3; } //8 10
	if (1 != fread(&E->RD, sizeof(double), 1, f)) { return -4; } // 8 18
	if (1 != fread(&E->vol, sizeof(double), 1, f)) { return -5; } //8 26
	/* SKip over game counts, date, and tournament id */
	if (0 != fseek(f, sizeof(char) * 4 + sizeof(short) * 2 , SEEK_CUR)) { return -1; } //2
	/* Read season id */
	if (1 != fread(&E->season_id, sizeof(short), 1, f)) { return -12; } //2 34

	return 0;
}


/** Reads contents of a player file to a struct entry. Returns 0 upon success,
 * and a negative number upon failure.Function expects that starter data
 * has already been passed and that the FILE is on an entry
 *
 * Note: this function doesn't read the entire entry into the
 * struct entry. It is the absent version and reads only the glicko2 data,
 * the date data, the tournament and season id.
 *
 * \param '*f' the file being read
 * \param '*E' the struct entry to store an entry found in the file too
 * \return 0 upon success, or a negative number upon failure.
 */
int p_file_open_read_entry_absent(FILE *f, struct entry *E) {
	/* SKip over opp id */
	if (0 != fseek(f, sizeof(short), SEEK_CUR)) { return -1; } //2
	/* Read glicko2 data */
	if (1 != fread(&E->rating, sizeof(double), 1, f)) { return -3; } //8 10
	if (1 != fread(&E->RD, sizeof(double), 1, f)) { return -4; } // 8 18
	if (1 != fread(&E->vol, sizeof(double), 1, f)) { return -5; } //8 26
	/* Skip over game counts */
	if (0 != fseek(f, sizeof(char) * 2, SEEK_CUR)) { return -1; } //2
	/* Read date data */
	if (1 != fread(&E->day, sizeof(char), 1, f)) { return -8; } //1 29
	/* If 'day' bitwise ANDed with 10000000 != 0, then this is a
	 * non-competitor entry */
	if ( (E->day & (1 << ((sizeof(E->day) * 8) - 1))) != 0) {
		E->is_competitor = 0;
		/* Set leftmost bit to 0 so it becomes a normal char */
		E->day = E->day &  ~(1 << ((sizeof(E->day) * 8) - 1));
	} else {
		E->is_competitor = 1;
	}
	if (1 != fread(&E->month, sizeof(char), 1, f)) { return -9; } //1 30
	if (1 != fread(&E->year, sizeof(short), 1, f)) { return -10; } //2 32
	if (1 != fread(&E->tournament_id, sizeof(short), 1, f)) { return -11; } //2 32
	if (1 != fread(&E->season_id, sizeof(short), 1, f)) { return -12; } //2 34
	/* Sets t_name and len_t_name of E to be according to tournament
	 * name E->tournament_id */
	int r;
	if (0 != (r = t_file_get_tournament_name_from_id(E))) {
		fprintf(stderr, "Error: t_file_get_tournament_name_from_id (%d)", r);
		return -13;
	}

	return 0;
}


/** Reads contents of a player file to a struct entry. Returns 0 upon success,
 * and a negative number upon failure. Function expects that starter data
 * has already been passed and that the FILE is on an entry
 *
 * \param '*f' the file being read
 * \param '*E' the struct entry to store an entry found in the file too
 * \return 0 upon success, or a negative number upon failure.
 */
int p_file_open_read_entry_tournament_id(FILE *f, struct entry *E) {
	/* Skip all entry data up until tournament id */
	if (0 != fseek(f, \
		sizeof(short) + \
		sizeof(double) + \
		sizeof(double) + \
		sizeof(double) + \
		sizeof(char) + \
		sizeof(char) + \
		sizeof(char) + \
		sizeof(char) + \
		sizeof(short), SEEK_CUR)) { return -1; }
	/* Read tournament id and seek to the end of the entry */
	if (1 != fread(&E->tournament_id, sizeof(short), 1, f)) { return -2; }
	if (0 != fseek(f, sizeof(short), SEEK_CUR)) { return -3; }

	return 0;
}


/** Reads all the starter data in a player entry file leaving the FILE '*f' at
 * a position where it can start reading entries.
 *
 * \param '*f' a player entry file opened in 'rb' mode.
 */
int p_file_open_position_at_start_of_entries(FILE *f) {
	char ln;
	if (1 != fread(&ln, sizeof(char), 1, f)) return -1;

	/* Skip past name and number of outcomes/tournaments attended */
	long name_and_counts = ln * sizeof(char) + 2 * sizeof(long);
	if (0 != fseek(f, name_and_counts, SEEK_CUR)) return -2;

	return 0;
}


/** Reads all the starter data in a player entry file and seeks such the FILE
 * '*f' is at a position where it can start appending a new entry.
 *
 * \param '*f' a player entry file opened in 'rb' mode.
 * \return a negative int on failure, 0 upon success.
 */
int p_file_open_position_for_appending_entry(FILE *f) {
	if (0 != fseek(f, 0, SEEK_END)) {
		return -1;
	}

	return 0;
}


/** Reads the all entries in a player file, returns the number of unique
 * opponents within, and alters '**ret_opp_id_list' to pointer to an
 * array of shorts containing all the unique opponent ids.
 *
 * \param '*file_path' file path to a player file.
 * \param '**ret_opp_id_list' pointer to a short pointer. The short pointer
 *     will be updated to memory address of a opp_id_list created in this
 *     function. It must be freed after.
 * \return a long representing the number of unique opponents in this player
 *     file.
 */
long p_file_number_of_opponents(char *file_path, short **ret_opp_id_list) {
	int ret = 0;
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_number_of_opponents)");
		ret = -1;
	}
	long opp_id_list_size = 64;
	short *opp_id_list = (short *)malloc(sizeof(short) * opp_id_list_size);
	short num_opp_ids = 0;
	struct entry E;

	p_file_open_position_at_start_of_entries(p_file);
	/* While the function is still able to read entries from the old file */
	while (0 == p_file_open_read_entry(p_file, &E)) {
		char already_in = 0;
		int i = 0;
		for (i = 0; i < num_opp_ids; i++) {
			if (E.opp_id == opp_id_list[i]) {
				already_in = 1;
				break;
			}
		}
		if (already_in == 0) {
			/* If there is no space to add this opponent, reallocate */
			if (num_opp_ids + 1 > opp_id_list_size) {
				opp_id_list_size *= 2;
				opp_id_list = (short *)\
					realloc(opp_id_list, sizeof(short) * opp_id_list_size);
				if (opp_id_list == NULL) {
					perror("realloc (p_file_number_of_opponents)");
					return 0;
				}
			}
			opp_id_list[num_opp_ids] = E.opp_id;
			num_opp_ids++;
		}
	}

	fclose(p_file);
	*ret_opp_id_list = opp_id_list;
	/* If there were no errors, return the number of opponents */
	if (ret == 0) ret = num_opp_ids;
	return ret;
}


/** Takes a file path to a player file and returns the number of
 * events this player has attended that weren't RD adjustments.
 *
 * \param '*file_path' the player file to read
 * \return upon success, a positive integer representing the number
 *     of opponents this player has played.
 */
long p_file_number_of_events(char *file_path) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_number_of_events)");
		return -1;
	}

	unsigned long num_events = 0;
	char ln;
	if (1 != fread(&ln, sizeof(char), 1, p_file)) return -2;
	/* Skip over player name and number of number of valid outcomes */
	if (0 != fseek(p_file, ln + sizeof(long), SEEK_CUR)) { return -3; }
	if (1 != fread(&num_events, sizeof(long), 1, p_file)) return -4;

	fclose(p_file);
	return num_events;
}


/** Reads a player file at the given file path and returns the number
 * of entries contained in that file.
 *
 * \param '*file_path' the file path of the file to be read.
 * \return 0 upon success, or a negative number upon failure.
 */
int p_file_get_number_of_entries(char *file_path) {
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (p_file_get_number_of_entries)");
		return -1;
	}

	// TODO: just do math, offset at start of entries, offset at end of file,
	// substract and divide?

	int entries = 0;
	/* Read entry from old file */
	struct entry *cur_entry = (struct entry *)malloc(sizeof(struct entry));
	p_file_open_position_at_start_of_entries(base_file);
	/* While the function is still able to read entries from the old file */
	while (0 == p_file_open_read_entry(base_file, cur_entry)) {
		entries++;
	}
	free(cur_entry);
	fclose(base_file);

	return entries;
}


/** Reads a player file at the given file path and returns the number
 * of entries contained in that file where the opponent is '*player2'.
 *
 * \param '*file_path' the file path of the file to be read.
 * \param '*player2' the file name (not path) of a player file
 * \return 0 upon success, or a negative number upon failure.
 */
long p_file_get_number_of_outcomes_against(char *file_path, char *player2) {
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (p_file_get_number_of_outcomes_against)");
		return -1;
	}

	int entries = 0;
	/* Read entry from old file */
	struct entry *cur_entry = (struct entry *)malloc(sizeof(struct entry));
	p_file_open_position_at_start_of_entries(base_file);
	/* While the function is still able to read entries from the old file */
	while (0 == p_file_open_read_entry(base_file, cur_entry)) {
		if (0 == strcmp(cur_entry->opp_name, player2)) {
			entries++;
		}
	}
	free(cur_entry);
	fclose(base_file);

	return entries;
}


/** Reads a player file at the given file path and returns an array of longs
 * that represent the number of times player1 has played that player in an
 * event. The array follows the order of '*opp_id_list', where '*opp_id_list'
 * is an unordered list of all the unique opp_ids in this player file.
 *
 * NOTE: the return value is calloc'd and as such, free() must be called
 * on it once it is no longer being used.
 *
 * \param '*file_path' the file path of the file to be read.
 * \param 'num_opp_ids' the number of unique opponents in this file
 * \param '*opp_id_list' an array of shorts representing every unique
 *     opp_id in this player file.
 * \return NULL upon failure, an array of longs (pointer) upon success.
 */
long *p_file_get_all_number_of_outcomes_against(char *file_path, \
	long num_opp_ids, short *opp_id_list) {

	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (p_file_get_all_number_of_outcomes_against)");
		return NULL;
	}

	long *outcomes = (long *)calloc(num_opp_ids, sizeof(long));
	/* Read entry from old file */
	struct entry E;
	p_file_open_position_at_start_of_entries(base_file);
	/* While the function is still able to read entries from the old file */
	while (0 == p_file_open_read_entry(base_file, &E)) {
		int i = 0;
		for (i = 0; i < num_opp_ids; i++) {
			if (E.opp_id == opp_id_list[i]) {
				break;
			}
		}
		outcomes[i] += 1;
	}
	fclose(base_file);

	return outcomes;
}


/** Returns the offset within a player file at which the last entry begins.
 *
 * \param '*file_path' a string of the file path of a player file for the
 *     function to find the offset of the last entry
 * \return a long representing the offset within the file at which the last
 *     entry begins
 */
long p_file_get_last_entry_offset(char *file_path) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_get_last_entry_offset)");
		return 0;
	}

	/* Read to the end of the starter data in the file */
	fseek(p_file, 0, SEEK_SET);
	int ret = p_file_open_position_at_start_of_entries(p_file);
	if (ret != 0) {
		fprintf(stderr, \
			"p_file_open_position_at_start_of_entries (p_file_get_last_entry_offset) returned %d", \
			ret);
	}

	fseek(p_file, 0, SEEK_END);
	long int last_entry_offset = ftell(p_file) - SIZE_OF_AN_ENTRY;

	fclose(p_file);

	return last_entry_offset;
}


/** Modifies a struct entry to be that of the last entry found in a player
 * file.
 *
 * \param '*file_path' the file path of the player file to be read.
 * \param '*ret' a struct entry pointer to have the data read into.
 * \return 0 upon success, a negative int upon failure.
 */
int p_file_read_last_entry(char *file_path, struct entry *ret) {
	/* Open files for reading contents */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_read_last_entry)");
		/* If the file could not be read for any reason, return accordingly */
		return -1;
	}

	/* Read the player's name from the file */
	if (0 != p_file_open_read_start_from_file(p_file, ret)) {
		perror("p_file_open_read_start_from_file (p_file_read_last_entry)");
		return -2;
	}

	fseek(p_file, -SIZE_OF_AN_ENTRY, SEEK_END);
	/* If reading the last entry failed */
	int r = 0;
	if (0 != (r = p_file_open_read_entry(p_file, ret))) {
		fprintf(stderr, "Error (%d) on p_file_open_read_entry()\n", r);
		return -3;
	}
	fclose(p_file);

	return 0;
}


/** Modifies a struct entry to be that of the last entry found in a player
 * file. Note that this function doesn't read the entire entry into the
 * struct entry. It is the minimal version and reads only the glicko2 data
 * plus the season id.
 *
 * \param '*file_path' the file path of the player file to be read.
 * \param '*ret' a struct entry pointer to have the data read into.
 * \return 0 upon success, a negative int upon failure.
 */
int p_file_read_last_entry_minimal(char* file_path, struct entry *ret) {
	/* Open files for reading contents */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_read_last_entry)");
		/* If the file could not be read for any reason, return accordingly */
		return -1;
	}

	fseek(p_file, -SIZE_OF_AN_ENTRY, SEEK_END);
	/* If reading the last entry failed */
	if (0 != p_file_open_read_entry_minimal(p_file, ret)) return -1;
	fclose(p_file);

	return 0;
}


/** Modifies a struct entry to be that of the last entry found in a player
 * file. Note that this function doesn't read the entire entry into the
 * struct entry. It is the absentee version and reads only the glicko2 data,
 * the date, the tournament and season id.
 *
 * \param '*file_path' the file path of the player file to be read.
 * \param '*ret' a struct entry pointer to have the data read into.
 * \return 0 upon success, a negative int upon failure.
 */
int p_file_read_last_entry_absent(char* file_path, struct entry *ret) {
	/* Open files for reading contents */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_read_last_entry)");
		/* If the file could not be read for any reason, return accordingly */
		return -1;
	}

	fseek(p_file, -SIZE_OF_AN_ENTRY, SEEK_END);
	/* If reading the last entry failed */
	if (0 != p_file_open_read_entry_absent(p_file, ret)) return -1;
	fclose(p_file);

	return 0;
}


/** Modifies a struct entry to be that of the last entry found in a player
 * file. Note that this function doesn't read the entire entry into the
 * struct entry. It is the tournament_id version and reads only the
 * tournament id.
 *
 * \param '*file_path' the file path of the player file to be read.
 * \param '*ret' a struct entry pointer to have the data read into.
 * \return 0 upon success, a negative int upon failure.
 */
int p_file_read_last_entry_tournament_id(char* file_path, struct entry *ret) {
	/* Open files for reading contents */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_read_last_entry)");
		/* If the file could not be read for any reason, return accordingly */
		return -1;
	}

	fseek(p_file, -SIZE_OF_AN_ENTRY, SEEK_END);
	/* If reading the last entry failed */
	if (0 != p_file_open_read_entry_tournament_id(p_file, ret)) return -1;
	fclose(p_file);

	return 0;
}


/** Modifies a struct entry to be that of the last entry found in an open
 * player file. Note that this function doesn't read the entire entry into the
 * struct entry. It is the tournament_id version and reads only the
 * tournament id.
 *
 * \param '*f' the open player file to be read.
 * \param '*ret' a struct entry pointer to have the data read into.
 * \return 0 upon success, a negative int upon failure.
 */
int p_file_open_read_last_entry_tournament_id(FILE *f, struct entry *ret) {

	fseek(f, -SIZE_OF_AN_ENTRY, SEEK_END);
	/* If reading the last entry failed */
	if (0 != p_file_open_read_entry_tournament_id(f, ret)) return -1;

	return 0;
}


/** Appends an RD adjustment entry to a given player file and return an int
 * representing whether the function succeeded or not.
 *
 * NOTE: This is the 'id' version of this function, and it will expect
 * that E->opp_id, and E->tournament_id have been correctly set.
 *
 * \param '*E' the struct entry to be appended.
 * \param '*file_path' the file path of the player file.
 * \return int that is 0 upon the function succeeding and negative upon
 *     any sort of failure.
 */
int p_file_append_adjustment_to_file_id(struct entry *E, char *file_path) {
	/* File guaranteed to exist as it was found by reading player
	 * directory contents */

	/* Open file for appending */
	FILE *p_file = fopen(file_path, "ab+");
	if (p_file == NULL) {
		fprintf(stderr, "Error opening file %s (p_file_append_adjustment_to_file_id): ", file_path);
		perror("");
		return -10;
	}
	/* Write length of opp name and opp name */
	if (1 != fwrite(&E->opp_id, sizeof(short), 1, p_file)) { return -9; }
	/* Write glicko data */
	if (1 != fwrite(&E->rating, sizeof(double), 1, p_file)) { return -10; }
	if (1 != fwrite(&E->RD, sizeof(double), 1, p_file)) { return -11; }
	if (1 != fwrite(&E->vol, sizeof(double), 1, p_file)) { return -12; }
	/* Write game counts */
	if (1 != fwrite(&E->gc, sizeof(char), 1, p_file)) { return -13; }
	if (1 != fwrite(&E->opp_gc, sizeof(char), 1, p_file)) { return -14; }
	/* Write date data */
	if (1 != fwrite(&E->day, sizeof(char), 1, p_file)) { return -15; }
	if (1 != fwrite(&E->month, sizeof(char), 1, p_file)) { return -16; }
	if (1 != fwrite(&E->year, sizeof(short), 1, p_file)) { return -17; }
	if (1 != fwrite(&E->tournament_id, sizeof(short), 1, p_file)) { return -18; }
	if (1 != fwrite(&E->season_id, sizeof(short), 1, p_file)) { return -19; }

	fclose(p_file);

	/* Guaranteed not to be a competitor entry */

	return 0;
}


/** Initializes a new, fresh entry file at '*file_path' with the info contained
 * in the struct entry '*E'.
 *
 * \param '*E' a struct entry containing the len_name, and name, of the player
 *     we want to initialize an entry file for.
 * \param '*file_path' a file path to where we want the entry file.
 * \return 0 upon success, a negative integer upon failure.
 */
int p_file_initialize(struct entry *E, char *file_path) {
	/* Open file for writing */
	/* FILE *p_file = fopen(file_path, "wb+"); */
	FILE *p_file = fopen(file_path, "w+b");
	if (p_file == NULL) {
		fprintf(stderr, "Error initializing an entry file \"%s\"" \
			"(p_file_initialize): ", file_path);
		perror("");
		return -1;
	}

	int len_name = strlen(E->name);
	if (1 != fwrite(&len_name, sizeof(char), 1, p_file)) {
		fprintf(stderr, "Error initializing an entry file \"%s\"" \
			"(p_file_initialize): ", file_path);
		perror("");
		return -2;
	}
	if (strlen(E->name)
		!= fwrite(E->name, sizeof(char), strlen(E->name), p_file)) {

		fprintf(stderr, "Error initializing an entry file \"%s\"" \
			"(p_file_initialize): ", file_path);
		perror("");
		return -3;
	}
	/* Write the number of outcomes and tournaments attended this player has.
	 * Since we initializing the entry file, both values must be 0 */
	unsigned long lzero = 0;
	if (1 != fwrite(&lzero, sizeof(long), 1, p_file)) {
		fprintf(stderr, "Error initializing an entry file \"%s\"" \
			"(p_file_initialize): ", file_path);
		perror("");
		return -4;
	}
	if (1 != fwrite(&lzero, sizeof(long), 1, p_file)) {
		fprintf(stderr, "Error initializing an entry file \"%s\"" \
			"(p_file_initialize): ", file_path);
		perror("");
		return -5;
	}
	fclose(p_file);

	return 0;
}


/** Appends an entry to a given player file and return an int representing
 * whether the function succeeded or not.
 *
 * NOTE: This is the 'id' version of this function, and it will expect
 * that E->opp_id, and E->tournament_id have been correctly set.
 *
 * \param '*E' the struct entry to be appended.
 * \param '*file_path' the file path of the player file.
 * \return int that is 0 upon the function succeeding and negative upon
 *     any sort of failure.
 */
int p_file_append_entry_to_file_id(struct entry *E, char *file_path) {
	/* If the file did not exist */
#ifdef __linux__
	char existed = access(file_path, R_OK) != -1;
#elif _WIN32
	int access = _access(file_path, 06);
	/* If there was an error with the _access() call */
	if (access == -1) {
		if (errno == EACCES) {
			fprintf(stderr, "ERROR: p_file_append_entry_to_file_id(): " \
				"_access(): File access denied: \"%s\"\n", file_path);
			return -1;
		} else if (errno == EINVAL) {
			fprintf(stderr, "ERROR: p_file_append_entry_to_file_id(): " \
				"_access(): Invalid parameter: \"%s\"\n", file_path);
			return -1;
		}
	}
	char existed = (access == 0);
#else
	char existed = access(file_path, R_OK) != -1;
#endif
	/* If the player entry file did not exist, create a new one */
	if (!existed) {
		if (0 != p_file_initialize(E, file_path)) {
			fprintf(stderr, "ERROR: p_file_append_entry_to_file_id(): " \
				"Failed to initialize entry file: \"%s\"\n", file_path);
		}
	}

	/* Entry that is used later to check if this entry is at a new tournament */
	struct entry E2;
	/* FILE *p_file = fopen(file_path, "ab+"); */
	FILE *p_file = fopen(file_path, "a+b");
	if (p_file == NULL) {
		fprintf(stderr, "ERROR: p_file_append_entry_to_file_id(): " \
			"Could not open file to append new entry: \"%s\":", file_path);
		perror("");
		return -6;
	}

	unsigned long out_count = p_file_open_get_outcome_count(p_file);
	printf("entry file %s was read as having %ld number of outcomes\n", file_path, out_count);
	if (0 != out_count) {
		if (0 != p_file_open_read_last_entry_tournament_id(p_file, &E2)) {
			return -7;
		}
	}

	/* After getting what this function needed from the starter data,
	 * reposition the cursor so that the next write will append an entry */
	if (0 != p_file_open_position_for_appending_entry(p_file)) {
		return -8;
	}

	/* Write length of opp name and opp name */
	if (1 != fwrite(&E->opp_id, sizeof(short), 1, p_file)) { return -8; }
	/* Write glicko data */
	size_t return_val_thing;
	/* if (sizeof(double) != (return_val_thing = fwrite(&(E->rating), 1, sizeof(double), p_file))) { */
	if (1 != (return_val_thing = fwrite(&(E->rating), sizeof(double), 1, p_file))) {
		perror("couldn't write value to file:");
		printf("\"%s\"\n", file_path);
		printf("fwrite returned %ld/%ld\n", return_val_thing, sizeof(double));
		return -9;
	}
	if (1 != fwrite(&E->RD, sizeof(double), 1, p_file)) {return -10; }
	if (1 != fwrite(&E->vol, sizeof(double), 1, p_file)) { return -11; }
	/* Write game counts */
	if (1 != fwrite(&E->gc, sizeof(char), 1, p_file)) { return -12; }
	if (1 != fwrite(&E->opp_gc, sizeof(char), 1, p_file)) { return -13; }
	/* Write date data */
	if (1 != fwrite(&E->day, sizeof(char), 1, p_file)) { return -14; }
	if (1 != fwrite(&E->month, sizeof(char), 1, p_file)) { return -15; }
	if (1 != fwrite(&E->year, sizeof(short), 1, p_file)) { return -16; }
	/* if (1 != fwrite(&E->tournament_id, sizeof(short), 1, p_file)) { */
	if (sizeof(short) != (return_val_thing = fwrite(&E->tournament_id, 1, sizeof(short), p_file))) {
		perror("couldn't write value to file:");
		printf("\"%s\"\n", file_path);
		printf("fwrite returned %ld/%ld\n", return_val_thing, sizeof(short));
		return -17;
	}
	if (1 != fwrite(&E->season_id, sizeof(short), 1, p_file)) { return -18; }

	fclose(p_file);

	// TODO: make this its own function
	/* If this entry is a competitor entry */
	if ( (E->day & (1 << ((sizeof(E->day) * 8) - 1))) == 0) {
		/* Update the number of outcome data in the entry file */
		FILE *p_file2 = fopen(file_path, "rb+");
		if (p_file2 == NULL) {
			fprintf(stderr, "Error opening file \"%s\" and updating " \
				"number_of info (p_file_append_entry_to_file_id): ", \
				file_path);
			perror("");
			return -19;
		}
		/* Read the starter data in the file */
		char ln;
		if (1 != fread(&ln, sizeof(char), 1, p_file2)) {
			return -20;
		}
		/* Move to point in file where number of outcomes resides */
		if (0 != fseek(p_file2, ln * sizeof(char), SEEK_CUR)) {
			return -21;
		}
		unsigned long number_of_outcomes;
		unsigned long number_of_tournaments_attended;
		if (1 != fread(&number_of_outcomes, sizeof(long), 1, p_file2)) {
			return -22;
		}
		if (1 != fread(&number_of_tournaments_attended, sizeof(long), 1, p_file2)) {
			return -23;
		}
		number_of_outcomes++;
		/* Move back to the point in file where number of outcomes resides */
		if (0 != fseek(p_file2, -1 * 2 * ((long int) sizeof(long)), SEEK_CUR)) {
			return -24;
		}
		if (1 != fwrite(&number_of_outcomes, sizeof(long), 1, p_file2)) {
			return -25;
		}
		/* Only increase number of tournaments attended if this outcome is
		 * at a new tournament */
		if (E2.tournament_id != E->tournament_id || \
			0 == out_count) {

			number_of_tournaments_attended++;
			if (1 != fwrite(&number_of_tournaments_attended, sizeof(long), 1, p_file2)) {
				return -26;
			}
		}
		fclose(p_file2);
	}

	return 0;
}


/** Appends an entry to a given player file and return an int representing
 * whether the function succeeded or not.
 *
 * \param '*E' the struct entry to be appended.
 * \param '*file_path' the file path of the player file.
 * \return int that is 0 upon the function succeeding and negative upon
 *     any sort of failure.
 */
int p_file_append_entry_to_file(struct entry *E, char *file_path) {
	/* If the file did not exist */
#ifdef __linux__
	char existed = access(file_path, R_OK) != -1;
#elif _WIN32
	char existed = access(file_path, 0) != -1;
#else
	char existed = access(file_path, R_OK) != -1;
#endif
	/* If the player entry file did not exist, create a new one with the
	 * standard starting infomartion of length of name, name, and 2 zeros
	 * because they have 0 opponents or tournaments played/attedned */
	if (!existed) {
/* #ifdef _WIN32 */
/* 		/\* Open the file, with read and write, Share for reading, */
/* 		* no security, open regardless, normal file with no attributes *\/ */
/* 		HANDLE victim = CreateFile(file_path, GENERIC_WRITE | GENERIC_READ, */
/* 			FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL); */
/* 		WriteFile(victim, "", sizeof(""), NULL, NULL); */
/* 		fprintf(stderr, " GetLastError(0x%x) = %s\n", GetLastError(), */
/* 		(LPCTSTR) "hi"); */
/* 		CloseHandle(victim); */
/* #endif */
		/* Open file for appending */
		FILE *p_file = fopen(file_path, "ab+");
		if (p_file == NULL) {
			perror("fopen (p_file_append_entry_to_file)");
			return -1;
		}

		int len_name = strlen(E->name);
		if (1 != fwrite(&len_name, sizeof(char), 1, p_file)) return -2;
		if (strlen(E->name)
			!= fwrite(E->name, sizeof(char), strlen(E->name), p_file)) {
			return -3;
		}
		/* Write the number of outcomes and tournaments attended this player
		 * has. If you are creating the file, it must be 0 and 0 */
		unsigned long lzero = 0;
		if (1 != fwrite(&lzero, sizeof(long), 1, p_file)) return -4;
		if (1 != fwrite(&lzero, sizeof(long), 1, p_file)) return -5;
		fclose(p_file);
	}

	/* Entry that is used later to check if this entry is at a new tournament */
	struct entry E2;
	unsigned long out_count = p_file_get_outcome_count(file_path);
	if (0 != out_count) {
		if (0 != p_file_read_last_entry_tournament_id(file_path, &E2)) return -6;
	}

	int ret;
	/* If the entry file does not already contain an id for this opponent */
	if (-1 == (ret = opp_file_contains_opponent(E->opp_name))) {
		/* Add the new opponent to the entry file. This also corrects
		 * the opp_id if it is incorrect */
		if (0 != (ret = opp_file_add_new_opponent(E))) {
			fprintf(stderr, "Error (%d) on opp_file_add_new_opponent(E, %s)\n", ret, file_path);
			return -7;
		}
	/* If there was an error */
	} else if (ret < -1) {
		fprintf(stderr, "Error (%d) on opp_file_contains_opponent(%s, %s)\n", ret, E->opp_name, file_path);
		return -8;
	/* If the entry file does contain an id for this opponent */
	} else {
		/* Fix the opp_id in case it wasn't set */
		E->opp_id = (unsigned short) ret;
	}
	/* If the entry file does not already contain an id for this tournament */
	if (-1 == (ret = t_file_contains_tournament(E->t_name))) {
		/* Add the new tournament to the entry file. This also corrects
		 * the t_id if it is incorrect */
		if (0 != t_file_add_new_tournament(E)) return -9;
	/* If there was an error */
	} else if (ret < -1) {
		printf("experienced issue with 't_file_contains_tournament()', which " \
				"returned %d\n", ret);
		return -10;
	/* If the entry file does contain an id for this tournament */
	} else {
		/* Fix the tournament_id in case it wasn't set */
		E->tournament_id = (unsigned short) ret;
	}

	/* Open file for appending */
	FILE *p_file = fopen(file_path, "ab+");
	if (p_file == NULL) {
		perror("fopen (p_file_append_entry_to_file)");
		return -11;
	}
	/* Write length of opp name and opp name */
	if (1 != fwrite(&E->opp_id, sizeof(short), 1, p_file)) { return -12; } // 2
	/* Write glicko data */
	if (1 != fwrite(&E->rating, sizeof(double), 1, p_file)) { return -13; } // 8 10
	if (1 != fwrite(&E->RD, sizeof(double), 1, p_file)) { return -14; } // 8 18
	if (1 != fwrite(&E->vol, sizeof(double), 1, p_file)) { return -15; } //8 26
	/* Write game counts */
	if (1 != fwrite(&E->gc, sizeof(char), 1, p_file)) { return -16; } //1 27
	if (1 != fwrite(&E->opp_gc, sizeof(char), 1, p_file)) { return -17; } //1 28
	/* Write date data */
	if (1 != fwrite(&E->day, sizeof(char), 1, p_file)) { return -18; } //1 29
	if (1 != fwrite(&E->month, sizeof(char), 1, p_file)) { return -19; } //1 30
	if (1 != fwrite(&E->year, sizeof(short), 1, p_file)) { return -20; } //2 32
	if (1 != fwrite(&E->tournament_id, sizeof(short), 1, p_file)) { return -21; } //2 34
	if (1 != fwrite(&E->season_id, sizeof(short), 1, p_file)) { return -22; } //2 36

	fclose(p_file);

	// TODO: make this its own function
	/* If this entry is a competitor entry */
	if ( (E->day & (1 << ((sizeof(E->day) * 8) - 1))) == 0) {
		/* Update the number of outcome data in the entry file */
		FILE *p_file2 = fopen(file_path, "rb+");
		if (p_file2 == NULL) {
			perror("fopen (p_file_append_entry_to_file)");
			return -23;
		}
		/* Read the starter data in the file */
		char ln;
		if (1 != fread(&ln, sizeof(char), 1, p_file2)) {
			return -24;
		}
		/* Move to point in file where number of outcomes resides */
		if (0 != fseek(p_file2, ln * sizeof(char), SEEK_CUR)) {
			return -25;
		}
		unsigned long number_of_outcomes;
		unsigned long number_of_tournaments_attended;
		if (1 != fread(&number_of_outcomes, sizeof(long), 1, p_file2)) {
			return -26;
		}
		if (1 != fread(&number_of_tournaments_attended, sizeof(long), 1, p_file2)) {
			return -27;
		}
		number_of_outcomes++;
		/* Move back to the point in file where number of outcomes resides */
		if (0 != fseek(p_file2, -1 * 2 * ((long int) sizeof(long)), SEEK_CUR)) {
			return -28;
		}
		if (1 != fwrite(&number_of_outcomes, sizeof(long), 1, p_file2)) {
			return -29;
		}
		/* Only increase number of tournaments attended if this outcome is
		 * at a new tournament */
		if (E2.tournament_id != E->tournament_id || \
			0 == out_count) {

			number_of_tournaments_attended++;
			if (1 != fwrite(&number_of_tournaments_attended, sizeof(long), 1, p_file2)) {
				return -30;
			}
		}
		fclose(p_file2);
	}

	return 0;
}


/** Reads a player file at the given file path, reads the "Player 1"
 * data into the given entry parameter.
 *
 * \param '*file_path' the file path of the file to be read.
 * \return 0 upon success, or a negative number upon failure.
 */
int p_file_read_start_from_file(char *file_path, struct entry *E) {
	/* Open file for appending */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_read_start_from_file)");
		return -1;
	}

	/* Read the starter data in the file */
	if (1 != fread(&E->len_name, sizeof(char), 1, p_file)) {
		return -2;
	}
	if (E->len_name != fread(E->name, sizeof(char), E->len_name, p_file)) {
		return -3;
	}
	E->name[E->len_name] = '\0';

	fclose(p_file);
	return 0;
}


/** Takes an open player file and reads the "Player 1"
 * data into the given entry parameter.
 *
 * \param '*f' the FILE pointer of the file to be read
 * \return 0 upon success, or a negative number upon failure.
 */
int p_file_open_read_start_from_file(FILE *f, struct entry *E) {
	/* Read the starter data in the file */
	if (1 != fread(&E->len_name, sizeof(char), 1, f)) {
		return -2;
	}
	if (E->len_name != fread(E->name, sizeof(char), E->len_name, f)) {
		return -3;
	}
	E->name[E->len_name] = '\0';

	return 0;
}


/** Takes a file path to a entry file and returns the number of valid
 * outcomes (or sets, games, what have you) this player has played in
 * the history of the system.
 *
 * \param '*file_path' the file path to an entry file.
 * \return an integer representing whether the function succeeded or not.
 *     0 upon success, and a negative value upon failure.
 */
int p_file_get_outcome_count(char *file_path) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_get_outcome_count)");
		return -1;
	}

	unsigned long num_outcomes = 0;
	char ln;
	if (1 != fread(&ln, sizeof(char), 1, p_file)) return -2;
	if (0 != fseek(p_file, ln, SEEK_CUR)) { return -3; }
	if (1 != fread(&num_outcomes, sizeof(long), 1, p_file)) return -4;

	fclose(p_file);
	return num_outcomes;
}


/** Takes an open entry file and returns the number of valid
 * outcomes (or sets, games, what have you) this player has played in
 * the history of the system. This function resets to the start of the file.
 *
 * \param '*f' the open entry file.
 * \return an integer representing whether the function succeeded or not.
 *     0 upon success, and a negative value upon failure.
 */
int p_file_open_get_outcome_count(FILE *f) {

	unsigned long num_outcomes = 0;
	char ln;

	/* Seek to the beginning of the file */
	if (0 != fseek(f, 0, SEEK_SET)) {
		return -1;
	}
	/* Read the first piece of data in the entry file, the length of the
	 * player's name */
	if (1 != fread(&ln, sizeof(char), 1, f)) {
		return -2;
	}
	/* Use the read player name length to seek past the player's name. This
	 * places the cursor right at the beginning of the number of valid outcomes
	 * this player has experienced */
	if (0 != fseek(f, ln, SEEK_CUR)) {
		return -3;
	}
	/* Read the number of valid outcomes */
	if (1 != fread(&num_outcomes, sizeof(long), 1, f)) {
		return -4;
	}

	return num_outcomes;
}


/** Takes a file path to a entry file and returns the number of valid
 * events (or tournaments, what have you) this player has played at in
 * the history of the system.
 *
 * \param '*file_path' the file path to an entry file.
 * \return an integer representing whether the function succeeded or not.
 *     0 upon success, and a negative value upon failure.
 */
int p_file_get_events_attended_count(char *file_path) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_get_events_attended_count)");
		return -1;
	}

	unsigned long num_attended = 0;

	char ln;
	if (1 != fread(&ln, sizeof(char), 1, p_file)) return -2;
	/* + sizeof(long) to get past num outcomes to num tournaments */
	if (0 != fseek(p_file, ln + sizeof(long), SEEK_CUR)) { return -3; }
	if (1 != fread(&num_attended, sizeof(long), 1, p_file)) return -4;

	fclose(p_file);
	return num_attended;
}


/** Takes a file path to a player file and a pointer to an integer. Returns an
 * array of all the names of the tournaments the player has attended that
 * weren't RD adjustments (or NULL) and modifies '*ret_count' to be the number
 * of elements in that array.
 *
 * \param '*file_path' the player file to read
 * \param '*ret_count' the integer to modify to contain the number of
 *     tournaments attended by the player
 * \return upon success, a pointer to a char array containing the names of the
 *     tournaments attended by the player that MUST BE FREED LATER. Upon
 *     failure, it returns a pointer to NULL.
 */
char *p_file_get_events_attended(char *file_path, int *ret_count) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (get_player_attended)");
		return NULL;
	}

	char len_of_name;
	char name[MAX_NAME_LEN + 1];
	long tourneys_size = TOURN_LIST_START_SIZE;
	long tourneys_num = 0;
	char *tourneys = \
		(char *)calloc(TOURN_LIST_START_SIZE * (MAX_NAME_LEN + 1), sizeof(char));
	char in_tourneys = 0;
	struct entry cur_entry;
	memset(name, 0, sizeof(name));
	/* Read the starter data in the file */
	if (1 != fread(&len_of_name, sizeof(char), 1, p_file)) return NULL;
	if ((size_t) len_of_name
		!= fread(name, sizeof(char), len_of_name, p_file)) {

		return NULL;
	}
	/* Get to the entries in the file and start reading them */
	fseek(p_file, 0, SEEK_SET);
	p_file_open_position_at_start_of_entries(p_file);

	while (p_file_open_read_entry(p_file, &cur_entry) == 0) {
		in_tourneys = 0;
		/* Check if the tournament that entry was from
		 * is already in the array */
		for (int i = 0; i < tourneys_num; i++) {
			if (strcmp(cur_entry.t_name, &tourneys[i * (MAX_NAME_LEN + 1)]) == 0) {
				in_tourneys = 1;
			}
		}
		/* If this tournament wasn't in the array, and the opponent wasn't
		 * actually a system RD adjustment, add it to the array */
		if (in_tourneys == 0 && cur_entry.is_competitor == 1) {
			/* If there is no space to add this tournament, reallocate */
			if (tourneys_num + 1 > tourneys_size) {
				tourneys_size += TOURN_LIST_REALLOC_INC;
				tourneys = (char*)realloc(tourneys, \
					TOURN_LIST_START_SIZE * (MAX_NAME_LEN + 1) * sizeof(char));
				if (tourneys == NULL) {
					perror("realloc (p_file_get_events_attended)");
					return NULL;
				}
			}
			/* Add the tournament to the array */
			strncpy(&tourneys[tourneys_num * (MAX_NAME_LEN + 1)], \
				cur_entry.t_name, MAX_NAME_LEN);
			tourneys_num++;
		}
	}
	*ret_count = tourneys_num;

	fclose(p_file);
	return tourneys;
}


/** Takes a file path to an entry file and returns the change in Glicko2
 * rating since the last tournament.
 *
 * \param '*file_path' a file path to an entry file.
 * \return a double representing the change in rating since the last tournament
 *     this player attended.
 */
double p_file_get_glicko_change_since_last_event(char* file_path) {

	double ret = 0;
	struct entry last_entry;
	struct entry actual_last;
	/* Set starting values of 'second_last_entry' such that if they have only
	 * been to one event, this function returns the right value */
	last_entry.rating = DEF_RATING;

	if (0 != p_file_read_last_entry(file_path, &actual_last)) return 0;
	ret = actual_last.rating - last_entry.rating;

	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (p_file_get_glicko_change_since_last_event)");
		return 0;
	}

	if (0 != p_file_open_position_at_start_of_entries(p_file)) return 0;
	unsigned long entries_begin = ftell(p_file);

	/* If they didn't attend the last event, return 0 */
	if (actual_last.is_competitor == 0) return 0;
	/* If this is their first event,
	 * return their current rating - the default */
	if (actual_last.tournament_id == 0) return ret;

	/* Go to the second last entry */
	if (0 != fseek(p_file, -2 * SIZE_OF_AN_ENTRY, SEEK_END)) return 0;

	while (ftell(p_file) >= entries_begin \
		&& 0 == p_file_open_read_entry(p_file, &last_entry) ) {
		/* If it reads an entry has a different name and date to the last
		 * tournament */
		if (0 != strcmp(last_entry.t_name, actual_last.t_name)
			&& (last_entry.day != actual_last.day
			|| last_entry.month != actual_last.month
			|| last_entry.year != actual_last.year)) {

			ret = actual_last.rating - last_entry.rating;
			break;
		}
		/* Go back one entry */
		if (0 != fseek(p_file, -2 * SIZE_OF_AN_ENTRY, SEEK_CUR)) return 0;
	}

	fclose(p_file);

	return ret;
}
