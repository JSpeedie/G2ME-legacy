/* Windows includes */
#ifdef _WIN32
#include <io.h>
//#include <windows.h>
//#include <fcntl.h>
#endif
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
#include <unistd.h>

#include "G2ME.h"
#include "player_dir.h"

char *entry_file_get_events_attended(char *, int *);
int entry_file_get_outcome_count(char *);
int entry_file_get_events_attended_count(char *);
double entry_file_get_glicko_change_since_last_event(char *);
int entry_file_read_start_from_file(char *, struct entry *);

/* [short | opp_id] [3 double | glicko data]
 * [4 char | game counts and date] [2 short | year and tournament_id] */
long int SIZE_OF_AN_ENTRY = (1 * sizeof(short)) + (3 * sizeof(double)) \
	+ (4 * sizeof(char)) + (3 * sizeof(short));

/** Takes an open entry file and moves the file cursor to the number of
 * opponents data.
 *
 * TODO:
 */
int entry_file_open_skip_to_num_opp(FILE* f) {
	char ln;
	if (1 != fread(&ln, sizeof(char), 1, f)) return -1;

	/* Skip past name and number of outcomes/tournaments attended,
	 * relative offset for end of opps and relative offset for end
	 * of tournaments */
	long name_and_counts = ln * sizeof(char) + 4 * sizeof(long);
	if (0 != fseek(f, name_and_counts, SEEK_CUR)) return -2;

	return 0;
}

/** Takes an open entry file and moves the file cursor to the number of
 * tournaments data.
 *
 * TODO:
 */
int entry_file_open_skip_to_num_t(FILE* f) {
	char ln;
	if (1 != fread(&ln, sizeof(char), 1, f)) return -1;

	/* Skip past name and number of outcomes/tournaments attended */
	long name_and_counts = ln * sizeof(char) + 2 * sizeof(long);
	long end_of_opps_roffset;
	if (0 != fseek(f, name_and_counts, SEEK_CUR)) return -2;
	/* Read offset from current spot to end of opps list (num t data) */
	if (1 != fread(&end_of_opps_roffset, sizeof(long), 1, f)) return -3;
	if (0 != fseek(f, end_of_opps_roffset, SEEK_CUR)) return -4;

	return 0;
}

/** Function takes an opponents name and a file path to the entry file you
 * want to check and returns an int representing whether it found the
 * opponent in the file to be examined.
 *
 * TODO: finish
 */
int entry_file_contains_opponent(char *opp_name, char* file_path) {
	int ret = -1;
	FILE* base_file = fopen(file_path, "rb+");
	if (base_file == NULL) {
		perror("fopen (entry_file_contains_opponent)");
		return -2;
	}

	if (0 != entry_file_open_skip_to_num_opp(base_file)) return -3;

	unsigned short num_opp;
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) return -7;

	for (int i = 0; i < num_opp; i++) {
		int j = 0;
		char read = '\1';
		char right_name = 1;
		/* Read first byte and add to temp_name */
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -9;
		if (read != opp_name[j]) { right_name = 0; }
		j++;
		/* Provided it hasn't hit a null terminator or end of file */
		while (read != '\0' && j < MAX_NAME_LEN && !(feof(base_file))) {
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -11;
			/* If the name differs at any point, it isn't the same name */
			if (read != opp_name[j]) right_name = 0;
			j++;
		}
		/* If this opponent name is already in the file */
		if (right_name == 1) ret = i;
	}
	fclose(base_file);
	/* The opponent name was not found, return -1 */
	return ret;
}




int entry_file_add_new_opponent(struct entry *E, char* file_path) {
#ifdef __linux__
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_add_new_tournament)");
		return -1;
	}

	char new_file_name[] = { "tempG2MEXXXXXX\0" };
	int r = mkstemp(new_file_name);
	close(r);
	unlink(new_file_name);

	FILE *new_file = fopen(new_file_name, "wb+");
	if (new_file == NULL) {
		perror("fopen (entry_file_add_new_tournament)");
		return -2;
	}
//#elif _WIN32
#else
	// TODO: switch to windows temp file stuff
	/* Get the name for the temp file TODO what if .[original name] already exists? */
	char dir[strlen(file_path) + 1];
	char base[strlen(file_path) + 1];
	memset(dir, 0, sizeof(dir));
	memset(base, 0, sizeof(base));
	strncpy(dir, file_path, sizeof(dir) - 1);
	strncpy(base, file_path, sizeof(base) - 1);

	char new_file_name[MAX_NAME_LEN + 1];
	memset(new_file_name, 0, sizeof(new_file_name));
	/* Add the full path up to the file */
	strncat(new_file_name, dirname(dir), sizeof(new_file_name) - 1);
	strncat(new_file_name, "/", sizeof(new_file_name) - strlen(new_file_name) - 1);
	/* Add the temp file */
	strncat(new_file_name, ".", sizeof(new_file_name) - strlen(new_file_name) - 1);
	strncat(new_file_name, basename(base), sizeof(new_file_name) - strlen(new_file_name) - 1);

	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_add_new_tournament)");
		return -1;
	}
	FILE *new_file = fopen(new_file_name, "wb+");
	if (new_file == NULL) {
		perror("fopen (entry_file_add_new_tournament)");
		return -2;
	}
#endif
	char ln;
	char zero = '\0';
	unsigned short num_opp;
	/* Read player 1 name length and name and write to temp file */
	if (1 != fread(&ln, sizeof(char), 1, base_file)) return -3;
	if (1 != fwrite(&ln, sizeof(char), 1, new_file)) return -4;
	char name[ln];
	if ((size_t)ln != fread(&name[0], sizeof(char), ln, base_file)) return -5;
	name[(int) ln] = '\0';
	if ((size_t)ln != fwrite(&name[0], sizeof(char), ln, new_file)) return -6;

	/* Read number of outcomes and number of tournaments attended, write to
	 * new file */
	unsigned long temp;
	if (1 != fread(&temp, sizeof(long), 1, base_file)) return -7;
	if (1 != fwrite(&temp, sizeof(long), 1, new_file)) return -8;
	if (1 != fread(&temp, sizeof(long), 1, base_file)) return -9;
	if (1 != fwrite(&temp, sizeof(long), 1, new_file)) return -10;
	/* Read the relative offsets, adjust accordingly, and write to new file */
	if (1 != fread(&temp, sizeof(long), 1, base_file)) return -7;
	temp += E->len_opp_name + 1;
	if (1 != fwrite(&temp, sizeof(long), 1, new_file)) return -8;
	if (1 != fread(&temp, sizeof(long), 1, base_file)) return -9;
	temp += E->len_opp_name + 1;
	if (1 != fwrite(&temp, sizeof(long), 1, new_file)) return -10;

	/* Read number of opponents and write [said number + 1] to temp file */
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) return -11;
	/* Correct the opp_id (0 indexe id) */
	E->opp_id = num_opp;
	num_opp += 1;
	if (1 != fwrite(&num_opp, sizeof(short), 1, new_file)) return -12;

	/* Read and write all the names of the opponents to the temp file */
	for (int i = 0; i < num_opp - 1; i++) {
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -13;
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -14;
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -15;
		}
		if (1 != fwrite(&zero, sizeof(char), 1, new_file)) return -16;
	}
	/* Write new opponent name to the file and a null terminator */
	if (E->len_opp_name != fwrite(&E->opp_name, sizeof(char), E->len_opp_name, new_file)) {
		return -13;
	}
	if (1 != fwrite(&zero, sizeof(char), 1, new_file)) return -17;

	/* Write the tournament data an entry data (aka every other byte in the
	 * file, in order) into the temp file */
	char read;
	while (!feof(base_file)) {
		if (1 == fread(&read, sizeof(char), 1, base_file)) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -18;
		}
	}
	fclose(new_file);
	fclose(base_file);
	/* Delete original file */
	remove(file_path);
	/* Copy temp file to original file path */
	rename(new_file_name, file_path);
	return 0;
}

int entry_file_contains_tournament(char *t_name, char* file_path) {
	int ret = -1;
	FILE* base_file = fopen(file_path, "rb+");
	if (base_file == NULL) {
		perror("fopen (entry_file_contains_tournament)");
		return -2;
	}

	if (0 != entry_file_open_skip_to_num_t(base_file)) return -3;

	unsigned short num_t;
	if (1 != fread(&num_t, sizeof(short), 1, base_file)) return -8;

	char temp_tournament[MAX_NAME_LEN];
	for (int i = 0; i < num_t; i++) {
		int j = 0;
		char read = '\1';
		/* Read first byte and add to temp_name */
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -9;
		temp_tournament[j] = read;
		j++;
		/* Provided it hasn't hit a null terminator or end of file */
		while (read != '\0' && j < MAX_NAME_LEN && !(feof(base_file))) {
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -10;
			/* If the current section of reading is still shorter than the
			 * name being searched for */
			temp_tournament[j] = read;
			j++;
		}
		temp_tournament[j] = '\0';
		/* If this tournament name is already in the file */
		if (0 == strcmp(temp_tournament, t_name)) ret = i;
	}
	fclose(base_file);
	/* The tournament name was not found, return 1 */
	return ret;
}

int entry_file_add_new_tournament(struct entry *E, char* file_path) {
#ifdef __linux__
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_add_new_tournament)");
		return -1;
	}

	char new_file_name[] = { "tempG2MEXXXXXX\0" };
	int r = mkstemp(new_file_name);
	close(r);
	unlink(new_file_name);

	FILE *new_file = fopen(new_file_name, "wb+");
	if (new_file == NULL) {
		perror("fopen (entry_file_add_new_tournament)");
		return -2;
	}
//#elif _WIN32
#else
	// TODO: switch to windows temp file stuff
	/* Get the name for the temp file TODO what if .[original name] already exists? */
	char dir[strlen(file_path) + 1];
	char base[strlen(file_path) + 1];
	memset(dir, 0, sizeof(dir));
	memset(base, 0, sizeof(base));
	strncpy(dir, file_path, sizeof(dir) - 1);
	strncpy(base, file_path, sizeof(base) - 1);

	char new_file_name[MAX_NAME_LEN + 1];
	memset(new_file_name, 0, sizeof(new_file_name));
	/* Add the full path up to the file */
	strncat(new_file_name, dirname(dir), sizeof(new_file_name) - 1);
	strncat(new_file_name, "/", sizeof(new_file_name) - strlen(new_file_name) - 1);
	/* Add the temp file */
	strncat(new_file_name, ".", sizeof(new_file_name) - strlen(new_file_name) - 1);
	strncat(new_file_name, basename(base), sizeof(new_file_name) - strlen(new_file_name) - 1);

	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_add_new_tournament)");
		return -1;
	}
	FILE *new_file = fopen(new_file_name, "wb+");
	if (new_file == NULL) {
		perror("fopen (entry_file_add_new_tournament)");
		return -2;
	}
#endif

	char ln;
	char zero = '\0';
	unsigned short num_opp, num_t;
	/* Read player 1 name length and name and write to temp file */
	if (1 != fread(&ln, sizeof(char), 1, base_file)) return -3;
	if (1 != fwrite(&ln, sizeof(char), 1, new_file)) return -4;
	char name[ln];
	if ((size_t) ln != fread(&name[0], sizeof(char), ln, base_file)) return -5;
	name[(int) ln] = '\0';
	if ((size_t) ln != fwrite(&name[0], sizeof(char), ln, new_file)) return -6;

	unsigned long temp;
	/* Read number of outcomes and number of tournaments attended, write to
	 * new file */
	if (1 != fread(&temp, sizeof(long), 1, base_file)) return -7;
	if (1 != fwrite(&temp, sizeof(long), 1, new_file)) return -8;
	if (1 != fread(&temp, sizeof(long), 1, base_file)) return -9;
	if (1 != fwrite(&temp, sizeof(long), 1, new_file)) return -10;
	/* Read relative offset for end of opps and end of tournaments to new file */
	if (1 != fread(&temp, sizeof(long), 1, base_file)) return -7;
	if (1 != fwrite(&temp, sizeof(long), 1, new_file)) return -8;
	if (1 != fread(&temp, sizeof(long), 1, base_file)) return -9;
	/* Adjust offset accordingly */
	temp += E->len_t_name + 1;
	if (1 != fwrite(&temp, sizeof(long), 1, new_file)) return -10;

	/* Read number of tournaments and write said number + 1 to temp file */
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) return -11;
	if (1 != fwrite(&num_opp, sizeof(short), 1, new_file)) return -12;

	/* Read and write all the names of the tournaments to the temp file */
	for (int i = 0; i < num_opp; i++) {
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -13;
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -14;
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -15;
		}
		if (1 != fwrite(&zero, sizeof(char), 1, new_file)) return -16;
	}
	/* Read number of tournaments and write said number + 1 to temp file */
	if (1 != fread(&num_t, sizeof(short), 1, base_file)) return -17;
	/* Correct the tournament_id (0 indexed) */
	E->tournament_id = num_t;
	num_t += 1;
	if (1 != fwrite(&num_t, sizeof(short), 1, new_file)) return -18;
	/* Read and write all the names of the tournaments to the temp file */
	for (int i = 0; i < num_t - 1; i++) {
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -19;
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -20;
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -21;
		}
		if (1 != fwrite(&zero, sizeof(char), 1, new_file)) return -22;
	}
	/* Write new tournament name to the file and a null terminator */
	if (E->len_t_name != fwrite(&E->t_name, sizeof(char), E->len_t_name, new_file)) {
		return -23;
	}
	if (1 != fwrite(&zero, sizeof(char), 1, new_file)) return -24;
	/* Write the entry data (aka every other byte in the
	 * file, in order) into the temp file */
	char read;
	while (!feof(base_file)) {
		if (1 == fread(&read, sizeof(char), 1, base_file)) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -25;
		}
	}
	fclose(new_file);
	fclose(base_file);
	/* Delete original file */
	remove(file_path);
	/* Copy temp file to original file path */
	rename(new_file_name, file_path);
	return 0;
}

int entry_get_name_from_id(FILE *f, struct entry *E) {
	int ret = 0;
	long int return_to = ftell(f);
	unsigned short num_opp;
	char temp[MAX_NAME_LEN];
	fseek(f, 0, SEEK_SET);

	if (0 != entry_file_open_skip_to_num_opp(f)) return -2;

	/* Read number of opponents */
	if (1 != fread(&num_opp, sizeof(short), 1, f)) return -3;

	for (int i = 0; i < num_opp; i++) {
		int j = 0;
		char read = '1';
		if (1 != fread(&read, sizeof(char), 1, f)) return -4;
		temp[j] = read;
		j++;
		while (read != '\0' && j < MAX_NAME_LEN && !(feof(f))) {
			if (1 != fread(&read, sizeof(char), 1, f)) return -5;
			temp[j] = read;
			j++;
		}
		temp[j] = '\0';
		/* If this is the name being searched for, add a null terminator
		 * and set len_opp_name */
		if (i == E->opp_id) {
			strncpy(E->opp_name, &temp[0], MAX_NAME_LEN);
			E->len_opp_name = strlen(E->opp_name);
			break;
		}
	}
	/* Leave the FILE the way it was found */
	fseek(f, return_to, SEEK_SET);
	return ret;
}

int entry_file_get_id_from_name(FILE *f, struct entry *E) {
	int ret = 0;
	long int return_to = ftell(f);
	unsigned short num_opp;
	char temp[MAX_NAME_LEN];
	fseek(f, 0, SEEK_SET);

	if (0 != entry_file_open_skip_to_num_opp(f)) return -2;

	/* Read number of opponents */
	if (1 != fread(&num_opp, sizeof(short), 1, f)) return -3;

	for (int i = 0; i < num_opp; i++) {
		int j = 0;
		char read = '1';
		if (1 != fread(&read, sizeof(char), 1, f)) return -4;
		temp[j] = read;
		j++;
		while (read != '\0' && j < MAX_NAME_LEN && !(feof(f))) {
			if (1 != fread(&read, sizeof(char), 1, f)) return -5;
			temp[j] = read;
			j++;
		}
		temp[j] = '\0';
		/* If this is the name being searched for, add a null terminator
		 * and set len_opp_name */
		if (0 == strcmp(E->opp_name, &temp[0])) {
			E->opp_id = i;
			break;
		}
	}
	/* Leave the FILE the way it was found */
	fseek(f, return_to, SEEK_SET);
	return ret;
}

int entry_file_get_tournament_name_from_id(FILE *f, struct entry *E) {
	int ret = 0;
	long int return_to = ftell(f);
	char temp[MAX_NAME_LEN];
	fseek(f, 0, SEEK_SET);

	if (0 != entry_file_open_skip_to_num_t(f)) return -2;

	/* Read number of tournaments */
	unsigned short num_t;
	if (1 != fread(&num_t, sizeof(short), 1, f)) return -6;

	/* Read all the names of the tournaments */
	for (int i = 0; i < num_t; i++) {
		int j = 0;
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, f)) return -7;
		temp[j] = read;
		j++;
		while (read != '\0' && j < MAX_NAME_LEN && !(feof(f))) {
			if (1 != fread(&read, sizeof(char), 1, f)) return -5;
			temp[j] = read;
			j++;
		}
		temp[j] = '\0';
		/* If this is the name being searched for, add a null terminator
		 * and set len_t_name */
		if (i == E->tournament_id) {
			strncpy(E->t_name, &temp[0], MAX_NAME_LEN);
			E->len_t_name = strlen(E->t_name);
		}
	}
	/* Leave the FILE the way it was found */
	fseek(f, return_to, SEEK_SET);
	return ret;
}

/** Reads contents of a player file to a struct entry. Returns 0 upon success,
 * and a negative number upon failure. Function expects that starter data
 * has already been passed and that the FILE is on an entry
 *
 * \param '*f' the file being read
 * \param '*E' the struct entry to store an entry found in the file too
 * \return 0 upon success, or a negative number upon failure.
 */
int entry_file_read_entry(FILE *f, struct entry *E) {
	// Read opponent name id
	if (1 != fread(&E->opp_id, sizeof(short), 1, f)) { return -1; } //2
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
	if (0 != (r = entry_get_name_from_id(f, E))) {
		perror("entry_get_name_from_id (read_entry)");
		return -12;
	}
	/* Sets t_name and len_t_name of E to be according to tournament
	 * name E->tournament_id */
	if (0 != (r = entry_file_get_tournament_name_from_id(f, E))) {
		perror("entry_file_get_tournament_name_from_id (read_entry)");
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
int entry_file_read_next_opp_entry(FILE *f, struct entry *E, short opp_id) {
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
	if (0 != (r = entry_get_name_from_id(f, E))) {
		perror("entry_get_name_from_id (read_entry)");
		return -12;
	}
	/* Sets t_name and len_t_name of E to be according to tournament
	 * name E->tournament_id */
	if (0 != (r = entry_file_get_tournament_name_from_id(f, E))) {
		perror("entry_file_get_tournament_name_from_id (read_entry)");
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
int entry_file_read_entry_minimal(FILE *f, struct entry *E) {
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
int entry_file_read_entry_absent(FILE *f, struct entry *E) {
	/* SKip over opp id */
	if (0 != fseek(f, sizeof(short), SEEK_CUR)) { return -1; } //2
	if (1 != fread(&E->rating, sizeof(double), 1, f)) { return -3; } //8 10
	if (1 != fread(&E->RD, sizeof(double), 1, f)) { return -4; } // 8 18
	if (1 != fread(&E->vol, sizeof(double), 1, f)) { return -5; } //8 26
	if (0 != fseek(f, sizeof(char) * 2, SEEK_CUR)) { return -1; } //2
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
	if (0 != entry_file_get_tournament_name_from_id(f, E)) {
		perror("entry_file_get_tournament_name_from_id (read_entry)");
		return -13;
	}

	return 0;
}

/** Reads the all the starter data in a player entry file leaving
 * the FILE '*base_file' at a position where it can start reading entries
 *
 * \param '*base_file' a player entry file opened in 'rb' mode
 */
int entry_file_get_to_entries(FILE *f) {
	char ln;
	if (1 != fread(&ln, sizeof(char), 1, f)) return -1;

	/* Skip past name and number of outcomes/tournaments attended, and
	 * past the end of opps relative offset */
	long name_and_counts = ln * sizeof(char) + 3 * sizeof(long);
	long end_of_t_roffset;
	if (0 != fseek(f, name_and_counts, SEEK_CUR)) return -2;
	/* Read offset from current spot to end of opps list (num t data) */
	if (1 != fread(&end_of_t_roffset, sizeof(long), 1, f)) return -3;
	if (0 != fseek(f, end_of_t_roffset, SEEK_CUR)) return -4;

	return 0;
}

int entry_file_number_of_opponents(char *file_path) {
	int ret = 0;
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_number_of_opponents)");
		ret = -1;
	}
	short num_opp;

	if (0 != entry_file_open_skip_to_num_opp(base_file)) return -2;

	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) ret = -3;

	fclose(base_file);
	/* If there were no errors, return the number of opponents */
	if (ret == 0) ret = num_opp;
	return ret;
}

// TODO: description innaccurate, and there's a faster method to do
// what is described in this doc using new attended long in file
/** Takes a file path to a player file and returns the number of
 * events this player has attended that weren't RD adjustments
 * (or NULL).
 *
 * \param '*file_path' the player file to read
 * \return upon success, a positive integer representing the number
 *     of opponents this player has played. Upon failure of any kind,
 *     this function returns a negative integer depending on the error.
 */
int entry_file_number_of_events(char *file_path) {
	int ret = 0;
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_number_of_events)");
		ret = -1;
	}
	short num_opp, num_t;

	if (0 != entry_file_open_skip_to_num_opp(base_file)) return -2;

	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) ret = -3;

	for (int i = 0; i < num_opp; i++) {
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, base_file)) ret = -9;
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fread(&read, sizeof(char), 1, base_file)) ret = -11;
		}
	}

	if (1 != fread(&num_t, sizeof(short), 1, base_file)) { ret = -4; }

	fclose(base_file);
	/* If there were no errors, ret = the number of events */
	if (ret == 0) ret = num_t;
	return ret;
}

/** Reads a player file at the given file path and returns the number
 * of entries contained in that file.
 *
 * \param '*file_path' the file path of the file to be read.
 * \return 0 upon success, or a negative number upon failure.
 */
int entry_file_get_number_of_entries(char *file_path) {
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_get_number_of_entries)");
		return -1;
	}

	// TODO: just do math, offset at start of entries, offset at end of file,
	// substract and divide?

	int entries = 0;
	/* Read entry from old file */
	struct entry *cur_entry = (struct entry *)malloc(sizeof(struct entry));
	entry_file_get_to_entries(base_file);
	/* While the function is still able to read entries from the old file */
	while (0 == entry_file_read_entry(base_file, cur_entry)) {
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
int entry_file_get_number_of_outcomes_against(char *file_path, char *player2) {
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_get_number_of_outcomes_against)");
		return -1;
	}

	int entries = 0;
	/* Read entry from old file */
	struct entry *cur_entry = (struct entry *)malloc(sizeof(struct entry));
	entry_file_get_to_entries(base_file);
	/* While the function is still able to read entries from the old file */
	while (0 == entry_file_read_entry(base_file, cur_entry)) {
		if (0 == strcmp(cur_entry->opp_name, player2)) {
			entries++;
		}
	}
	free(cur_entry);
	fclose(base_file);

	return entries;
}

/** Reads a player file at the given file path and returns an array of longs
 * that represent the number of times player1 has met that player in an event.
 * The array is indexed by the opponent IDs set in the player1 player file.
 * NOTE: the return value is calloc'd and as such, free() must be called on it
 * once it is no longer being used.
 *
 * \param '*file_path' the file path of the file to be read.
 * \return NULL upon failure, an array of longs (pointer) upon success.
 */
long *entry_file_get_all_number_of_outcomes_against(char *file_path) {
	long array_size = entry_file_number_of_opponents(file_path);
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_get_all_number_of_outcomes_against)");
		return NULL;
	}

	long *entries = (long *)calloc(array_size, sizeof(long));
	/* Read entry from old file */
	struct entry cur_entry;
	entry_file_get_to_entries(base_file);
	/* While the function is still able to read entries from the old file */
	while (0 == entry_file_read_entry(base_file, &cur_entry)) {
		entries[cur_entry.opp_id] = entries[cur_entry.opp_id] + 1;
	}
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
long int entry_file_get_last_entry_offset(char* file_path) {
	FILE *entry_file = fopen(file_path, "rb");
	if (entry_file == NULL) {
		perror("fopen (entry_file_get_last_entry_offset)");
		return 0;
	}

	/* Read to the end of the starter data in the file */
	fseek(entry_file, 0, SEEK_SET);
	int ret = entry_file_get_to_entries(entry_file);
	if (ret != 0) printf("entry_file_get_to_entries (entry_file_get_last_entry_offset) returned %d", ret);

	fseek(entry_file, 0, SEEK_END);
	long int last_entry_offset = ftell(entry_file) - SIZE_OF_AN_ENTRY;

	fclose(entry_file);

	return last_entry_offset;
}

/** Modifies a struct entry to be that of the last entry found in a player file.
 *
 * \param '*file_path' the file path of the player file to be read
 * \param '*ret' a struct entry pointer to have the data read into.
 * \return 0 upon success, a negative int upon failure.
 */
int entry_file_read_last_entry(char* file_path, struct entry *ret) {
	/* Open files for reading contents */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (entry_file_read_last_entry)");
		/* If the file could not be read for any reason, return accordingly */
		return -1;
	}

	/* Read the player's name from the file */
	if (0 != entry_file_open_read_start_from_file(p_file, ret)) {
		perror("entry_file_open_read_start_from_file (entry_file_read_last_entry)");
		return -2;
	}

	fseek(p_file, -SIZE_OF_AN_ENTRY, SEEK_END);
	/* If reading the last entry failed */
	if (0 != entry_file_read_entry(p_file, ret)) return -1;
	fclose(p_file);

	return 0;
}

/** Modifies a struct entry to be that of the last entry found in a player
 * file. Note that this function doesn't read the entire entry into the
 * struct entry. It is the minimal version and reads only the glicko2 data
 * plus the season id.
 *
 * \param '*file_path' the file path of the player file to be read
 * \param '*ret' a struct entry pointer to have the data read into.
 * \return 0 upon success, a negative int upon failure.
 */
int entry_file_read_last_entry_minimal(char* file_path, struct entry *ret) {
	/* Open files for reading contents */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (entry_file_read_last_entry)");
		/* If the file could not be read for any reason, return accordingly */
		return -1;
	}

	fseek(p_file, -SIZE_OF_AN_ENTRY, SEEK_END);
	/* If reading the last entry failed */
	if (0 != entry_file_read_entry_minimal(p_file, ret)) return -1;
	fclose(p_file);

	return 0;
}

/** Modifies a struct entry to be that of the last entry found in a player
 * file. Note that this function doesn't read the entire entry into the
 * struct entry. It is the absentee version and reads only the glicko2 data,
 * the date, the tournament and season id.
 *
 * \param '*file_path' the file path of the player file to be read
 * \param '*ret' a struct entry pointer to have the data read into.
 * \return 0 upon success, a negative int upon failure.
 */
int entry_file_read_last_entry_absent(char* file_path, struct entry *ret) {
	/* Open files for reading contents */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (entry_file_read_last_entry)");
		/* If the file could not be read for any reason, return accordingly */
		return -1;
	}

	fseek(p_file, -SIZE_OF_AN_ENTRY, SEEK_END);
	/* If reading the last entry failed */
	if (0 != entry_file_read_entry_absent(p_file, ret)) return -1;
	fclose(p_file);

	return 0;
}

/** Appends an entry to a given player file and return an int representing
 * whether the function succeeded or not.
 *
 * \param '*E' the struct entry to be appended
 * \param '*file_path' the file path of the player file
 * \return int that is 0 upon the function succeeding and negative upon
 *     any sort of failure.
 */
int entry_file_append_entry_to_file(struct entry* E, char* file_path) {
	/* If the file did not exist */
#ifdef __linux__
	char existed = access(file_path, R_OK) != -1;
#elif _WIN32
	char existed = _access(file_path, 0) != -1;
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
		FILE *entry_file = fopen(file_path, "ab+");
		if (entry_file == NULL) {
			perror("fopen (entry_file_append_entry_to_file)");
			return -1;
		}

		int len_name = strlen(E->name);
		if (1 != fwrite(&len_name, sizeof(char), 1, entry_file)) return -2;
		if (strlen(E->name)
			!= fwrite(E->name, sizeof(char), strlen(E->name), entry_file)) {
			return -3;
		}
		/* Write the number of outcomes and tournaments attended this player
		 * has. If you are creating the file, it must be 0 and 0 */
		unsigned long lzero = 0;
		if (1 != fwrite(&lzero, sizeof(long), 1, entry_file)) return -4;
		if (1 != fwrite(&lzero, sizeof(long), 1, entry_file)) return -5;
		/* Write the relative offsets.
		 * For a freshly created file, it must be 10 and 4 */
		unsigned long lten = 10;
		unsigned long lfour = 4;
		if (1 != fwrite(&lten, sizeof(long), 1, entry_file)) return -4;
		if (1 != fwrite(&lfour, sizeof(long), 1, entry_file)) return -5;
		/* Write the number of opponents and tournaments this player has
		 * attended. If you are creating the file, it must be 0 and 0 */
		unsigned short zero = 0;
		if (1 != fwrite(&zero, sizeof(short), 1, entry_file)) return -4;
		if (1 != fwrite(&zero, sizeof(short), 1, entry_file)) return -5;
		fclose(entry_file);
	}

	/* Entry that is used later to check if this entry is at a new tournament */
	struct entry E2;
	int ret2;
	// TODO: add check to see if there are any entries
	/* If it failed to read the last entry, it is because this is a fresh file */
	if (0 != (ret2 = entry_file_read_last_entry(file_path, &E2))) {
		E2.t_name[0] = '\0';
	}

	int ret;
	/* If the entry file does not already contain an id for this opponent */
	if (-1 == (ret = entry_file_contains_opponent(E->opp_name, file_path))) {
		/* Add the new opponent to the entry file. This also corrects
		 * the opp_id if it is incorrect */
		if (0 != entry_file_add_new_opponent(E, file_path)) return -6;
	/* If there was an error */
	} else if (ret < -1) {
		return -7;
	/* If the entry file does contain an id for this opponent */
	} else {
		/* Fix the opp_id in case it wasn't set */
		E->opp_id = (unsigned short) ret;
	}
	/* If the entry file does not already contain an id for this tournament */
	if (-1 == (ret = entry_file_contains_tournament(E->t_name, file_path))) {
		/* Add the new tournament to the entry file. This also corrects
		 * the t_id if it is incorrect */
		if (0 != entry_file_add_new_tournament(E, file_path)) return -8;
	/* If there was an error */
	} else if (ret < -1) {
		return -9;
	/* If the entry file does contain an id for this tournament */
	} else {
		/* Fix the tournament_id in case it wasn't set */
		E->tournament_id = (unsigned short) ret;
	}


	/* Open file for appending */
	FILE *entry_file = fopen(file_path, "ab+");
	if (entry_file == NULL) {
		perror("fopen (entry_file_append_entry_to_file)");
		return -10;
	}
	//fwrite(test, sizeof(char), 34, entry_file);
	/* Write length of opp name and opp name */
	if (1 != fwrite(&E->opp_id, sizeof(short), 1, entry_file)) { return -9; } // 2
	/* Write glicko data */
	if (1 != fwrite(&E->rating, sizeof(double), 1, entry_file)) { return -10; } // 8 10
	if (1 != fwrite(&E->RD, sizeof(double), 1, entry_file)) { return -11; } // 8 18
	if (1 != fwrite(&E->vol, sizeof(double), 1, entry_file)) { return -12; } //8 26
	/* Write game counts */
	if (1 != fwrite(&E->gc, sizeof(char), 1, entry_file)) { return -13; } //1 27
	if (1 != fwrite(&E->opp_gc, sizeof(char), 1, entry_file)) { return -14; } //1 28
	/* Write date data */
	if (1 != fwrite(&E->day, sizeof(char), 1, entry_file)) { return -15; } //1 29
	if (1 != fwrite(&E->month, sizeof(char), 1, entry_file)) { return -16; } //1 30
	if (1 != fwrite(&E->year, sizeof(short), 1, entry_file)) { return -17; } //2 32
	if (1 != fwrite(&E->tournament_id, sizeof(short), 1, entry_file)) { return -18; } //2 34
	if (1 != fwrite(&E->season_id, sizeof(short), 1, entry_file)) { return -19; } //2 36

	fclose(entry_file);

	// TODO: make this its own function
	/* If this entry is a competitor entry */
	if ( (E->day & (1 << ((sizeof(E->day) * 8) - 1))) == 0) {
		/* Update the number of outcome data in the entry file */
		FILE *entry_file2 = fopen(file_path, "rb+");
		if (entry_file2 == NULL) {
			perror("fopen (entry_file_append_entry_to_file)");
			return -11;
		}
		/* Read the starter data in the file */
		char ln;
		if (1 != fread(&ln, sizeof(char), 1, entry_file2)) {
			return -2;
		}
		/* Move to point in file where number of outcomes resides */
		if (0 != fseek(entry_file2, ln * sizeof(char), SEEK_CUR)) {
			return -3;
		}
		unsigned long number_of_outcomes;
		unsigned long number_of_tournaments_attended;
		if (1 != fread(&number_of_outcomes, sizeof(long), 1, entry_file2)) {
			return -4;
		}
		if (1 != fread(&number_of_tournaments_attended, sizeof(long), 1, entry_file2)) {
			return -5;
		}
		number_of_outcomes++;
		/* Move back to the point in file where number of outcomes resides */
		if (0 != fseek(entry_file2, -1 * 2 * ((long int) sizeof(long)), SEEK_CUR)) {
			return -6;
		}
		if (1 != fwrite(&number_of_outcomes, sizeof(long), 1, entry_file2)) { return -7; }
		/* Only increase number of tournaments attended if this outcome is
		 * at a new tournament */
		if (0 != strncmp(E2.t_name, E->t_name, E->len_t_name)) {
			number_of_tournaments_attended++;
			if (1 != fwrite(&number_of_tournaments_attended, sizeof(long), 1, entry_file2)) { return -8; }
		}
		fclose(entry_file2);
	}

	return 0;
}


/** Appends a pr entry (the name and glicko2 data for a player) to a given
 * file. Returns an int representing success.
 *
 * \param '*E' the struct entry to append to the pr file
 * \param '*file_path' the file path for the pr file
 * \return 0 upon success, negative number on failure.
 */
int entry_file_append_pr_entry_to_file(struct entry* E, char* file_path, \
	int longest_name_length) {

	FILE *entry_file;

	if (flag_output_to_stdout == 0) {
		entry_file = fopen(file_path, "a+");
		if (entry_file == NULL) {
			perror("fopen (entry_file_append_pr_entry_to_file)");
			return -1;
		}
	} else {
		entry_file = stdout;
	}

	if (fprintf(entry_file, "%*s  %6.1lf  %5.1lf  %10.8lf\n", \
		longest_name_length, E->name, E->rating, E->RD, E->vol) < 0) {

		perror("fprintf");
		return -3;
	}

	if (flag_output_to_stdout == 0) {
		fclose(entry_file);
	}
	return 0;
}

/** Appends a pr entry (the name and glicko2 data for a player) to a given
 * file. Returns an int representing success.
 *
 * Difference with verbosity: Adds 3 columns: Events attended,
 * outcomes gone through, and glicko rating change since last event.
 * Changes with verbosity: Adds more decimals to output of glicko variables.
 *
 * \param '*E' the struct entry to append to the pr file
 * \param '*file_path' the file path for the pr file
 * \return 0 upon success, negative number on failure.
 */
int entry_file_append_pr_entry_to_file_verbose(struct entry* E, char* file_path, \
	int longest_name_length, int longest_attended_count, \
	int longest_outcome_count) {

	FILE *entry_file;

	if (flag_output_to_stdout == 0) {
		entry_file = fopen(file_path, "a+");
		if (entry_file == NULL) {
			perror("fopen (entry_file_append_pr_entry_to_file_verbose)");
			return -1;
		}
	} else {
		entry_file = stdout;
	}

	char *full_player_path = player_dir_file_path_with_player_dir(E->name);
	int attended_count = entry_file_get_events_attended_count(full_player_path);
	int outcome_count = entry_file_get_outcome_count(full_player_path);
	double glicko_change = entry_file_get_glicko_change_since_last_event(full_player_path);
	free(full_player_path);
	char temp[MAX_NAME_LEN];
	// TODO: fix magic number thing
	sprintf(temp, "%4.3lf", E->rating);
	unsigned int rating_length = strlen(temp);
	sprintf(temp, "%3.3lf", E->RD);
	unsigned int rd_length = strlen(temp);

	/* <= 0.0001 to handle double rounding errors */
	if (fabs(glicko_change) <= 0.0001) {
		/* If unable to write to the file */
		// TODO: remove magic numbers 7 and 6
		if (fprintf(entry_file, "%*s  %*s%4.3lf  %*s%3.3lf  %10.8lf  %*d  %*d\n", \
			longest_name_length, E->name, 8-rating_length, "", \
			E->rating, 7-rd_length, "", E->RD, E->vol, \
			longest_attended_count, attended_count, \
			longest_outcome_count, outcome_count) < 0) {

			perror("fprintf (append_pr_entry_to_file_verbose)");
			return -2;
		}
	} else {
		/* If unable to write to the file */
		if (fprintf(entry_file, "%*s  %*s%4.3lf  %*s%3.3lf  %10.8lf  %*d  %*d  %+5.1lf\n", \
			longest_name_length, E->name, 8-rating_length, "",
			E->rating, 7-rd_length, "", E->RD, E->vol, \
			longest_attended_count, attended_count, \
			longest_outcome_count, outcome_count, \
			glicko_change) < 0) {

			perror("fprintf (entry_file_append_pr_entry_to_file_verbose)");
			return -3;
		}
	}

	if (flag_output_to_stdout == 0) {
		fclose(entry_file);
	}
	return 0;
}


/** Reads a player file at the given file path, reads the "Player 1"
 * data into the given entry parameter.
 *
 * \param '*file_path' the file path of the file to be read.
 * \return 0 upon success, or a negative number upon failure.
 */
int entry_file_read_start_from_file(char *file_path, struct entry *E) {
	/* Open file for appending */
	FILE *entry_file = fopen(file_path, "rb");
	if (entry_file == NULL) {
		perror("fopen (entry_file_read_start_from_file)");
		return -1;
	}

	/* Read the starter data in the file */
	if (1 != fread(&E->len_name, sizeof(char), 1, entry_file)) {
		return -2;
	}
	if (E->len_name != fread(E->name, sizeof(char), E->len_name, entry_file)) {
		return -3;
	}
	E->name[E->len_name] = '\0';

	fclose(entry_file);
	return 0;
}

/** Reads a open player file, reads the "Player 1"
 * data into the given entry parameter.
 *
 * \param '*f' the FILE pointer of the file to be read
 * \return 0 upon success, or a negative number upon failure.
 */
int entry_file_open_read_start_from_file(FILE *f, struct entry *E) {
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

/** Takes a file path of a player file, prompts the user for the new name,
 * and renames Player 1 to the new name. Note that this does not rename
 * the file itself which may create errors unless fixed.
 *
 * \param '*file_path' the file path of the player file.
 * \return 0 upon success, a negative number upon failure.
 */
int entry_file_refactor_name(char *file_path) {
	char new_name[MAX_NAME_LEN];
	fprintf(stdout, "New player name: ");
	scanf("%s", new_name);
	/* Read entry from old file */
	struct entry cur_entry;
	entry_file_read_start_from_file(file_path, &cur_entry);
	strncpy(&(cur_entry.name[0]), new_name, MAX_NAME_LEN);
	cur_entry.len_name = strlen(cur_entry.name);
	fprintf(stdout, "new name %s\n", cur_entry.name);
	/* Get the name for the temp file TODO what if .[original name] already exists? */
	char dir[strlen(file_path) + 1];
	char base[strlen(file_path) + 1];
	memset(dir, 0, sizeof(dir));
	memset(base, 0, sizeof(base));
	strncpy(dir, file_path, sizeof(dir) - 1);
	strncpy(base, file_path, sizeof(base) - 1);

	char new_file_name[MAX_NAME_LEN + 1];
	memset(new_file_name, 0, sizeof(new_file_name));
	/* Add the full path up to the file */
	strncat(new_file_name, dirname(dir), sizeof(new_file_name) - 1);
	strncat(new_file_name, "/", sizeof(new_file_name) - strlen(new_file_name) - 1);
	/* Add the temp file */
	strncat(new_file_name, ".", sizeof(new_file_name) - strlen(new_file_name) - 1);
	strncat(new_file_name, basename(base), sizeof(new_file_name) - strlen(new_file_name) - 1);

	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (refactor_file)");
		return -1;
	}
	FILE *new_file = fopen(new_file_name, "wb+");
	if (new_file == NULL) {
		perror("fopen (refactor_file)");
		return -2;
	}

	/* Read old data */
	char temp;
	char temp_name[MAX_NAME_LEN];
	if (1 != fread(&temp, sizeof(char), 1, base_file)) return -3;
	if ((size_t) temp != \
		fread(&temp_name, sizeof(char), temp, base_file)) return -4;
	/* Write the new name info to the temp file */
	if (1 != fwrite(&(cur_entry.len_name), sizeof(char), 1, new_file)) return -5;
	if (cur_entry.len_name != \
		fwrite(&(cur_entry.name[0]), sizeof(char), cur_entry.len_name, new_file)) return -6;
	/* Write the tournament data an entry data (aka every other byte in the
	 * file, in order) into the temp file */
	char read;
	while (!feof(base_file)) {
		if (1 == fread(&read, sizeof(char), 1, base_file)) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -7;
		}
	}
	fclose(new_file);
	fclose(base_file);
	/* Delete original file */
	remove(file_path);
	/* Copy temp file to original file path */
	rename(new_file_name, file_path);
	return 0;
}

// TODO: Dangerous function. Not safe for usage, doesn't update key data
// at header of file.
/** Takes a file path of a player file, and removes the last entry in it.
 *
 * \param '*file_path' the file path of the player file.
 * \return 0 upon success, a negative number upon failure.
 */
int entry_file_remove_entry(char *file_path) {
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_remove_entry)");
		return -1;
	}
	char dir[strlen(file_path) + 1];
	char base[strlen(file_path) + 1];
	memset(dir, 0, sizeof(dir));
	memset(base, 0, sizeof(base));
	strncpy(dir, file_path, sizeof(dir) - 1);
	strncpy(base, file_path, sizeof(base) - 1);

	int lines_to_remove = 1;
	int entries = entry_file_get_number_of_entries(file_path);
	int entries_read = 0;
	/* Read entry from old file */
	struct entry *cur_entry = (struct entry *)malloc(sizeof(struct entry));
	char new_file_name[MAX_NAME_LEN + 1];
	memset(new_file_name, 0, sizeof(new_file_name));
	/* Add the full path up to the file */
	strncat(new_file_name, dirname(dir), sizeof(new_file_name) - 1);
	strncat(new_file_name, "/", sizeof(new_file_name) - strlen(new_file_name) - 1);
	/* Add the temp file */
	strncat(new_file_name, ".", sizeof(new_file_name) - strlen(new_file_name) - 1);
	strncat(new_file_name, basename(base), sizeof(new_file_name) - strlen(new_file_name) - 1);
	/* Read the starter data in the file */
	entry_file_read_start_from_file(file_path, cur_entry);
	/* Get to the entries and begin reading them */
	fseek(base_file, 0, SEEK_SET);
	entry_file_get_to_entries(base_file);
	/* While the function is still able to read entries from the old file */
	while (0 == entry_file_read_entry(base_file, cur_entry) && entries_read < (entries - lines_to_remove)) {
		entries_read++;
		/* write new entry in new file as we get each old entry */
		entry_file_append_entry_to_file(cur_entry, new_file_name);
	}
	/* Delete original file */
	remove(file_path);
	/* Copy temp file to original file path */
	rename(new_file_name, file_path);
	free(cur_entry);
	fclose(base_file);
	return 0;
}

int entry_file_get_outcome_count(char *file_path) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (entry_file_get_outcome_count)");
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

int entry_file_get_events_attended_count(char *file_path) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (entry_file_get_events_attended_count)");
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

/** Takes a file path to a player file and a pointer to an integer.
 * Returns an array of all the names of the tournaments the player
 * has attended that weren't RD adjustments (or NULL) and modifies '*ret_count'
 * to be the number of elements in that array.
 *
 * \param '*file_path' the player file to read
 * \param '*ret_count' the integer to modify to contain the number of
 *     tournaments attended by the player
 * \return upon success, a pointer to a char array containing the names of the
 *     tournaments attended by the player that MUST BE FREED LATER. Upon
 *     failure, it returns a pointer to NULL.
 */
char *entry_file_get_events_attended(char *file_path, int *ret_count) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (get_player_attended)");
		return NULL;
	}

	char len_of_name;
	char name[MAX_NAME_LEN];
	long tourneys_size = SIZE_PR_ENTRY;
	long tourneys_num = 0;
	char *tourneys = (char *)calloc(SIZE_PR_ENTRY * MAX_NAME_LEN, sizeof(char));
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
	entry_file_get_to_entries(p_file);

	while (entry_file_read_entry(p_file, &cur_entry) == 0) {
		in_tourneys = 0;
		/* Check if the tournament that entry was from
		 * is already in the array */
		for (int i = 0; i < tourneys_num; i++) {
			if (strcmp(cur_entry.t_name, tourneys + i * MAX_NAME_LEN) == 0) {
				in_tourneys = 1;
			}
		}
		/* If this tournament wasn't in the array, and the opponent wasn't
		 * actually a system RD adjustment, add it to the array */
		if (in_tourneys == 0 && cur_entry.is_competitor == 1) {
			/* If there is no space to add this tournament, reallocate */
			if (tourneys_num + 1 > tourneys_size) {
				tourneys_size += REALLOC_PR_ENTRIES_INC;
				tourneys = (char*)realloc(tourneys, \
					SIZE_PR_ENTRY * MAX_NAME_LEN * sizeof(char));
				if (tourneys == NULL) {
					perror("realloc (entry_file_get_events_attended)");
					return NULL;
				}
			}
			/* Add the tournament to the array */
			strncpy(tourneys + tourneys_num * MAX_NAME_LEN, \
				cur_entry.t_name, MAX_NAME_LEN);
			tourneys_num++;
		}
	}
	*ret_count = tourneys_num;

	fclose(p_file);
	return tourneys;
}

double entry_file_get_glicko_change_since_last_event(char* file_path) {

	double ret = 0;
	struct entry last_entry;
	struct entry actual_last;
	/* Set starting values of 'second_last_entry' such that if they have only
	 * been to one event, this function returns the right value */
	last_entry.rating = DEF_RATING;

	if (0 != entry_file_read_last_entry(file_path, &actual_last)) return 0;
	ret = actual_last.rating - last_entry.rating;

	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (entry_file_get_glicko_change_since_last_event)");
		return 0;
	}

	if (0 != entry_file_get_to_entries(p_file)) return 0;
	unsigned long entries_begin = ftell(p_file);

	/* If they didn't attend the last event, return 0 */
	if (actual_last.is_competitor == 0) return 0;
	/* If this is their first event,
	 * return their current rating - the default */
	if (actual_last.tournament_id == 0) return ret;

	/* Go to the second last entry */
	if (0 != fseek(p_file, -2 * SIZE_OF_AN_ENTRY, SEEK_END)) return 0;

	while (ftell(p_file) >= entries_begin \
		&& 0 == entry_file_read_entry(p_file, &last_entry) ) {
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
