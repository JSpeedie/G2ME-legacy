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

#include "glicko2.h"
#include "G2ME.h"

char *entry_file_get_events_attended(char *, int *);
int entry_file_get_outcome_count(char *);
double entry_file_get_glicko_change_since_last_event(char *);
int entry_file_read_start_from_file(char *, struct entry *);

int entry_file_contains_opponent(char *opp_name, char* file_path) {
	int ret = -1;
	FILE* base_file = fopen(file_path, "rb+");
	if (base_file == NULL) {
		perror("fopen (entry_file_contains_opponent)");
		return -2;
	}
	char ln;
	unsigned short num_opp;
	if (1 != fread(&ln, sizeof(char), 1, base_file)) return -3;
	char name[ln];
	if ((size_t) ln != fread(&name[0], sizeof(char), ln, base_file)) return -4;
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) return -5;

	char temp_name[MAX_NAME_LEN];
	for (int i = 0; i < num_opp; i++) {
		int j = 0;
		char read = '\1';
		/* Read first byte and add to temp_name */
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -9;
		temp_name[j] = read;
		j++;
		/* Provided it hasn't hit a null terminator or end of file */
		while (read != '\0' && j < MAX_NAME_LEN && !(feof(base_file))) {
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -11;
			temp_name[j] = read;
			j++;
		}
		temp_name[j] = '\0';
		/* If this opponent name is already in the file */
		if (0 == strcmp(temp_name, opp_name)) {
			ret = i;
		}
	}
	fclose(base_file);
	/* The opponent name was not found, return -1 */
	return ret;
}

int entry_file_add_new_opponent(struct entry *E, char* file_path) {
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
	strncat(new_file_name, "/", \
		sizeof(new_file_name) - strlen(new_file_name) - 1);
	/* Add the temp file */
	strncat(new_file_name, ".", sizeof(new_file_name) - strlen(new_file_name) - 1);
	strncat(new_file_name, basename(base), sizeof(new_file_name) - strlen(new_file_name) - 1);

	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_add_new_opponent)");
		return -1;
	}
	FILE *new_file = fopen(new_file_name, "wb+");
	if (new_file == NULL) {
		perror("fopen (entry_file_add_new_opponent)");
		return -2;
	}
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
	/* Read number of opponents and write said number + 1 to temp file */
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) return -7;
	/* Correct the opp_id (0 indexe id) */
	E->opp_id = num_opp;
	num_opp += 1;
	if (1 != fwrite(&num_opp, sizeof(short), 1, new_file)) return -8;

	/* Read and write all the names of the opponents to the temp file */
	for (int i = 0; i < num_opp - 1; i++) {
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -9;
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -10;
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -11;
		}
		if (1 != fwrite(&zero, sizeof(char), 1, new_file)) return -12;
	}
	/* Write new opponent name to the file and a null terminator */
	if (E->len_opp_name != fwrite(&E->opp_name, sizeof(char), E->len_opp_name, new_file)) {
		return -13;
	}
	if (1 != fwrite(&zero, sizeof(char), 1, new_file)) return -14;

	/* Write the tournament data an entry data (aka every other byte in the
	 * file, in order) into the temp file */
	char read;
	while (!feof(base_file)) {
		if (1 == fread(&read, sizeof(char), 1, base_file)) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -15;
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
	char ln;
	unsigned short num_opp, num_t;
	if (1 != fread(&ln, sizeof(char), 1, base_file)) return -3;
	char name[ln];
	if ((size_t) ln != fread(&name[0], sizeof(char), ln, base_file)) return -4;
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) return -5;
	for (int i = 0; i < num_opp; i++) {
		char read = '1';
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -6;
		}
	}
	if (1 != fread(&num_t, sizeof(short), 1, base_file)) return -6;

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
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -11;
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
	/* Read number of tournaments and write said number + 1 to temp file */
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) return -7;
	if (1 != fwrite(&num_opp, sizeof(short), 1, new_file)) return -8;

	/* Read and write all the names of the tournaments to the temp file */
	for (int i = 0; i < num_opp; i++) {
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -9;
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -10;
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -11;
		}
		if (1 != fwrite(&zero, sizeof(char), 1, new_file)) return -12;
	}
	/* Read number of tournaments and write said number + 1 to temp file */
	if (1 != fread(&num_t, sizeof(short), 1, base_file)) return -9;
	/* Correct the tournament_id (0 indexed) */
	E->tournament_id = num_t;
	num_t += 1;
	if (1 != fwrite(&num_t, sizeof(short), 1, new_file)) return -10;
	/* Read and write all the names of the tournaments to the temp file */
	for (int i = 0; i < num_t - 1; i++) {
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -9;
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -10;
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -11;
		}
		if (1 != fwrite(&zero, sizeof(char), 1, new_file)) return -12;
	}
	/* Write new tournament name to the file and a null terminator */
	if (E->len_t_name != fwrite(&E->t_name, sizeof(char), E->len_t_name, new_file)) {
		return -13;
	}
	if (1 != fwrite(&zero, sizeof(char), 1, new_file)) return -14;
	/* Write the entry data (aka every other byte in the
	 * file, in order) into the temp file */
	char read;
	while (!feof(base_file)) {
		if (1 == fread(&read, sizeof(char), 1, base_file)) {
			if (1 != fwrite(&read, sizeof(char), 1, new_file)) return -11;
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
	char ln;
	unsigned short num_opp;
	fseek(f, 0, SEEK_SET);
	if (1 != fread(&ln, sizeof(char), 1, f)) return -1;
	char name[ln];
	if ((size_t) ln != fread(&name[0], sizeof(char), ln, f)) return -2;
	name[(int) ln] = '\0';
	char temp[MAX_NAME_LEN];
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
		}
	}
	/* Leave the FILE the way it was found */
	fseek(f, return_to, SEEK_SET);
	return ret;
}

int entry_file_get_tournament_name_from_id(FILE *f, struct entry *E) {
	int ret = 0;
	long int return_to = ftell(f);
	char ln;
	unsigned short num_opp;
	unsigned short num_t;
	fseek(f, 0, SEEK_SET);
	if (1 != fread(&ln, sizeof(char), 1, f)) return -1;
	char name[ln];
	if ((size_t) ln != fread(&name[0], sizeof(char), ln, f)) return -2;
	name[(int) ln] = '\0';
	char temp[MAX_NAME_LEN];
	/* Read number of opponents */
	if (1 != fread(&num_opp, sizeof(short), 1, f)) return -3;

	/* Read all the names of the opponents */
	for (int i = 0; i < num_opp; i++) {
		char read = '1';
		if (1 != fread(&read, sizeof(char), 1, f)) return -4;
		while (read != '\0' && !(feof(f))) {
			if (1 != fread(&read, sizeof(char), 1, f)) return -5;
		}
	}

	/* Read number of tournaments */
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
	if (1 != fread(&E->month, sizeof(char), 1, f)) { return -9; } //1 30
	if (1 != fread(&E->year, sizeof(short), 1, f)) { return -10; } //2 32
	if (1 != fread(&E->tournament_id, sizeof(short), 1, f)) { return -11; } //2 32
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

/** Reads the all the starter data in a player entry file leaving
 * the FILE '*base_file' at a position where it can start reading entries
 *
 * \param '*base_file' a player entry file opened in 'rb' mode
 */
int entry_file_get_to_entries(FILE *base_file) {
	char ln;
	short num_opp, num_t;
	if (1 != fread(&ln, sizeof(char), 1, base_file)) {
		return -1;
	}
	char name[ln];
	if ((size_t) ln != fread(&name[0], sizeof(char), ln, base_file)) {
		return -2;
	}
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) {
		return -3;
	}

	for (int i = 0; i < num_opp; i++) {
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -9;
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -11;
		}
	}

	if (1 != fread(&num_t, sizeof(short), 1, base_file)) {
		return -4;
	}

	for (int i = 0; i < num_t; i++) {
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, base_file)) return -9;
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fread(&read, sizeof(char), 1, base_file)) return -11;
		}
	}
	return 0;
}

int entry_file_number_of_opponents(char *file_path) {
	int ret = 0;
	FILE *base_file = fopen(file_path, "rb");
	if (base_file == NULL) {
		perror("fopen (entry_file_number_of_opponents)");
		ret = -1;
	}
	char ln;
	short num_opp;
	if (1 != fread(&ln, sizeof(char), 1, base_file)) ret = -1;
	char name[ln];
	if ((size_t) ln != fread(&name[0], sizeof(char), ln, base_file)) {
		ret = -2;
	}
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) ret = -3;

	fclose(base_file);
	/* If there were no errors, return the number of opponents */
	if (ret == 0) ret = num_opp;
	return ret;
}

/** Takes a file path to a player file and * returns the number of
 * events this player has attended * that weren't RD adjustments
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
	char ln;
	short num_opp, num_t;
	if (1 != fread(&ln, sizeof(char), 1, base_file)) ret = -1;
	char name[ln];
	if ((size_t) ln != fread(&name[0], sizeof(char), ln, base_file)) {
		ret = -2;
	}
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) ret = -3;

	for (int i = 0; i < num_opp; i++) {
		char read = '\1';
		if (1 != fread(&read, sizeof(char), 1, base_file)) ret = -9;
		while (read != '\0' && !(feof(base_file))) {
			if (1 != fread(&read, sizeof(char), 1, base_file)) ret = -11;
		}
	}

	if (1 != fread(&num_t, sizeof(short), 1, base_file)) {
		ret = -4;
	}

	fclose(base_file);
	/* If there were no errors, ret = the number of events */
	if (ret == 0) ret = num_opp;
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
	int ret = entry_file_get_to_entries(entry_file);
	if (ret != 0) printf("entry_file_get_to_entries (entry_file_get_last_entry_offset) returned %d", ret);
	long int last_entry_offset = ftell(entry_file);
	/* [short | opp_id] [3 double | glicko data]
	 * [4 char | game counts and date] [2 short | year and tournament_id] */
	long int size_of_an_entry = \
		(1 * sizeof(short)) + (3 * sizeof(double)) \
		+ (4 * sizeof(char)) + (2 * sizeof(short));
	void *temp = malloc(size_of_an_entry);

	/* Attempt to read a whole entry at a time, when it fails, exit */
	while (1 == fread(temp, size_of_an_entry, 1, entry_file)) {
		last_entry_offset = ftell(entry_file) - size_of_an_entry;
	}

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
	if (0 != entry_file_read_start_from_file(file_path, ret)) {
		perror("entry_file_read_start_from_file (entry_file_read_last_entry)");
		return -2;
	}
	/* Set file position to be at the latest entry for that player */
	long int offset = entry_file_get_last_entry_offset(file_path);
	fseek(p_file, offset, SEEK_SET);
	/* If reading the last entry failed */
	int t;
	if (0 != (t = entry_file_read_entry(p_file, ret))) {
		return -2;
	}
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
		/* Write the number of opponents and tournaments this player has
		 * attended. If you are creating the file, it must be 0 and 0 */
		unsigned short zero = 0;
		if (1 != fwrite(&zero, sizeof(short), 1, entry_file)) return -4;
		if (1 != fwrite(&zero, sizeof(short), 1, entry_file)) return -5;
		fclose(entry_file);
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

	char *full_player_path = file_path_with_player_dir(E->name);
	int attended_count;
	entry_file_get_events_attended(full_player_path, &attended_count);
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
	if ((size_t) temp !=													\
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

	long num_outcomes = 0;
	struct entry cur_entry;
	/* Read the starter data in the file */
	entry_file_read_start_from_file(file_path, &cur_entry);
	/* Go back to beginning of entries for reading */
	fseek(p_file, 0, SEEK_SET);
	entry_file_get_to_entries(p_file);

	while (entry_file_read_entry(p_file, &cur_entry) == 0) {
		// If the entry was NOT an RD adjustment due to absense
		if (strcmp(cur_entry.opp_name, "-") != 0 && !(cur_entry.gc == 0 \
			&& cur_entry.opp_gc == 0)) {
			num_outcomes++;
		}
	}

	fclose(p_file);
	return num_outcomes;
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
		/* If this tournament wasn't in the array, and the opponent was
		 * actually a system RD adjustment, add it to the array */
		if (in_tourneys == 0 && strcmp(cur_entry.opp_name, "-") != 0) {
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
	struct entry second_last_entry;
	struct entry last_entry;
	struct entry actual_last;
	second_last_entry.rating = 0.0;
	last_entry.rating = 0.0;
	/* Set starting values of 'second_last_entry' such that if they have only
	 * been to one event, this function returns the right value */
	second_last_entry.rating = DEF_RATING;
	second_last_entry.RD = DEF_RD;
	second_last_entry.vol = DEF_VOL;

	if (0 != entry_file_read_last_entry(file_path, &actual_last)) return 0;

	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (entry_file_get_glicko_change_since_last_event)");
		return 0;
	}

	entry_file_get_to_entries(p_file);

	while (0 == entry_file_read_entry(p_file, &last_entry)) {
		/* If it reads an entry that is the last event or an event
		 * that occurred on the same day as the last event (an
		 * amateur bracket for instance) */
		if (0 == strcmp(last_entry.t_name, actual_last.t_name)
			|| (last_entry.day == actual_last.day
			&& last_entry.month == actual_last.month
			&& last_entry.year == actual_last.year)) {
			break;
		}
		second_last_entry = last_entry;
	}
	fclose(p_file);

	ret = actual_last.rating - second_last_entry.rating;
	return ret;
}
