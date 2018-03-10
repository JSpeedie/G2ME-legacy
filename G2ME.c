#include <dirent.h>
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

#define MAX_NAME_LEN 128
#define MAX_FILE_PATH_LEN 256
#define DEF_VOL 0.06
#define DEF_RATING 1500.0
#define DEF_RD 350.0
#define DEF_TAU 0.5
#define REALLOC_PR_ENTRIES_INC 4
#define SIZE_PR_ENTRY 128

char *NORMAL = "\x1B[0m";
char *RED = "\x1B[31m";
char *GREEN = "\x1B[32m";
char *YELLOW = "\x1B[33m";
char *BLUE = "\x1B[34m";
char *MAGENTA = "\x1B[35m";
char *CYAN = "\x1B[36m";
char *WHITE = "\x1B[37m";

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

typedef struct entry {
	unsigned short opp_id;
	unsigned short tournament_id;
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
	unsigned short year;
	unsigned char len_t_name;
	char t_name[MAX_NAME_LEN];
}Entry;

typedef struct record {
	char name[MAX_NAME_LEN];
	char opp_name[MAX_NAME_LEN];
	unsigned char wins;
	unsigned char ties;
	unsigned char losses;
}Record;

struct entry temp;

char *get_player_attended(char *, int *);
int get_player_outcome_count(char *);

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
	if (ln != fread(&name[0], sizeof(char), ln, base_file)) return -4;
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
	strncat(new_file_name, "/", sizeof(new_file_name) - strlen(new_file_name) - 1);
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
	if (ln != fread(&name[0], sizeof(char), ln, base_file)) return -5;
	name[(int) ln] = '\0';
	if (ln != fwrite(&name[0], sizeof(char), ln, new_file)) return -6;
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
	if (ln != fread(&name[0], sizeof(char), ln, base_file)) return -4;
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
	unsigned short num_opp, num_t;
	/* Read player 1 name length and name and write to temp file */
	if (1 != fread(&ln, sizeof(char), 1, base_file)) return -3;
	if (1 != fwrite(&ln, sizeof(char), 1, new_file)) return -4;
	char name[ln];
	if (ln != fread(&name[0], sizeof(char), ln, base_file)) return -5;
	name[(int) ln] = '\0';
	if (ln != fwrite(&name[0], sizeof(char), ln, new_file)) return -6;
	/* Read number of opponents and write said number + 1 to temp file */
	if (1 != fread(&num_opp, sizeof(short), 1, base_file)) return -7;
	if (1 != fwrite(&num_opp, sizeof(short), 1, new_file)) return -8;

	/* Read and write all the names of the opponents to the temp file */
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
	if (ln != fread(&name[0], sizeof(char), ln, f)) return -2;
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

int entry_get_tournament_name_from_id(FILE *f, struct entry *E) {
	int ret = 0;
	long int return_to = ftell(f);
	char ln;
	unsigned short num_opp;
	unsigned short num_t;
	fseek(f, 0, SEEK_SET);
	if (1 != fread(&ln, sizeof(char), 1, f)) return -1;
	char name[ln];
	if (ln != fread(&name[0], sizeof(char), ln, f)) return -2;
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
int read_entry(FILE *f, struct entry *E) {
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
	if (0 != (r = entry_get_tournament_name_from_id(f, E))) {
		perror("entry_get_tournament_name_from_id (read_entry)");
		return -13;
	}

	return 0;
}

/** Reads the all the starter data in a player entry file leaving
 * the FILE '*base_file' at a position where it can start reading entries
 *
 * \param '*base_file' a player entry file opened in 'rb' mode
 */
int get_to_entries_in_file(FILE *base_file) {
	char ln;
	short num_opp, num_t;
	if (1 != fread(&ln, sizeof(char), 1, base_file)) {
		return -1;
	}
	char name[ln];
	if (ln != fread(&name[0], sizeof(char), ln, base_file)) {
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
	get_to_entries_in_file(base_file);
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

	/* Read to the end of the starter data in the file */
	int ret = get_to_entries_in_file(entry_file);
	if (ret != 0) printf("get_to_entries_in_file (get_last_entry_offset) returned %d", ret);
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
	/* If the file did not exist */
	char existed = access(file_path, R_OK) != -1;

	/* If the player entry file did not exist, create a new one with the
	 * standard starting infomartion of length of name, name, and 2 zeros
	 * because they have 0 opponents or tournaments played/attedned */
	if (!existed) {
		/* Open file for appending */
		FILE *entry_file = fopen(file_path, "ab+");
		if (entry_file == NULL) {
			perror("fopen (append_entry_to_file)");
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
		perror("fopen (append_entry_to_file)");
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
		return -3;
	}

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
int append_pr_entry_to_file_verbose(struct entry* E, char* file_path, \
	int longest_name_length, int longest_attended_count, \
	int longest_outcome_count) {

	FILE *entry_file = fopen(file_path, "a+");
	if (entry_file == NULL) {
		perror("fopen (append_pr_entry_to_file_verbose)");
		return -1;
	}

	char *full_player_path = file_path_with_player_dir(E->name);
	int attended_count;
	get_player_attended(full_player_path, &attended_count);
	int outcome_count = get_player_outcome_count(full_player_path);
	free(full_player_path);

	/* If unable to write to the file */
	if (fprintf(entry_file, "%*s  %6.1lf  %5.1lf  %10.8lf  %*d  %*d\n", \
		longest_name_length, E->name, E->rating, E->RD, E->vol, \
		longest_attended_count, attended_count, \
		longest_outcome_count, outcome_count) < 0) {

		perror("fprintf (append_pr_entry_to_file_verbose)");
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
	append_entry_to_file(&input_entry, file_path);
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

	printf("%d %d %-10s %-10s %16.14lf %16.14lf %16.14lf %d-%d %s %d %-10s\n", \
		E.len_name, E.len_opp_name, E.name, E.opp_name, E.rating, E.RD, \
		E.vol, E.gc, E.opp_gc, date, E.len_t_name, E.t_name);
}

/** Prints a string representation of a struct entry to stdout
 *
 * \param 'E' the struct entry to print
 * \param 'longest_name' the length in characters to print the opponent
 *     name in/with.
 */
void print_entry_name(struct entry E, int longest_nl, int longest_opp_nl, \
	int longest_name, int longest_rating, int longest_RD, int longest_vol, \
	int longest_date) {
	/* Process date data into one string */
	char date[32];
	sprintf(date, "%d/%d/%d", E.day, E.month, E.year);

	printf("%*d %*d %-*s %-*s %*lf %*lf %*.*lf %d-%d %-*s %s\n", \
		longest_nl, E.len_name, longest_opp_nl, E.len_opp_name, \
		E.len_name, E.name, longest_name, E.opp_name, longest_rating, \
		E.rating, longest_RD, E.RD, longest_vol, longest_vol-2, E.vol, \
		E.gc, E.opp_gc, longest_date, date, E.t_name);
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

/** Prints all the contents of a player file to stdout with each entry
 * on a new line.
 *
 * \param '*file_path' the file path of the player file to be read
 * \return 0 if the function succeeded and a negative number if there was
 *     a failure.
 */
int print_player_file(char* file_path) {
	struct entry line;
	read_start_from_file(file_path, &line);

	/* Open file for reading */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (print_player_file)");
		return -1;
	}

	int longest_name = 0;
	char temp[64];
	memset(temp, 0, sizeof(temp));
	int longest_nl = 0;
	int longest_opp_nl = 0;
	int longest_rating = 0;
	int longest_RD = 0;
	int longest_vol = 0;
	int longest_date = 0;

	fseek(p_file, 0, SEEK_SET);
	get_to_entries_in_file(p_file);
	/* Get the longest lengths of the parts of an entry in string form */
	while (read_entry(p_file, &line) == 0) {
		sprintf(temp, "%d", line.len_name);
		if (strlen(line.opp_name) > longest_name) {
			longest_name = strlen(line.opp_name);
		}
		if (strlen(temp) > longest_nl) longest_nl = strlen(temp);
		sprintf(temp, "%d", line.len_opp_name);
		if (strlen(temp) > longest_opp_nl) longest_opp_nl = strlen(temp);
		sprintf(temp, "%lf", line.rating);
		if (strlen(temp) > longest_rating) longest_rating = strlen(temp);
		sprintf(temp, "%lf", line.RD);
		if (strlen(temp) > longest_RD) longest_RD = strlen(temp);
		sprintf(temp, "%10.8lf", line.vol);
		if (strlen(temp) > longest_vol) longest_vol = strlen(temp);
		sprintf(temp, "%d/%d/%d", line.day, line.month, line.year);
		if (strlen(temp) > longest_date) longest_date = strlen(temp);
	}

	fseek(p_file, 0, SEEK_SET);
	get_to_entries_in_file(p_file);

	while (read_entry(p_file, &line) == 0) {
		print_entry_name(line, longest_nl, longest_opp_nl, longest_name, \
			longest_rating, longest_RD, longest_vol, longest_date);
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
		perror("fopen (read_last_entry)");
		/* If the file could not be read for any reason, return accordingly */
		return -1;
	}

	/* Read the player's name from the file */
	read_start_from_file(file_path, ret);
	/* Set file position to be at the latest entry for that player */
	long int offset = get_last_entry_offset(file_path);
	fseek(p_file, offset, SEEK_SET);
	/* If reading the last entry failed */
	int t;
	if (0 != (t = read_entry(p_file, ret))) {
		return -2;
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
		if (0 == (t = read_last_entry(full_p1_path, &p1_latest))) {
			init_player_from_entry(p1, &p1_latest);
		} else {
			perror("read_last_entry (update_player_on_outcome)");
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
		if (0 == read_last_entry(full_p2_path, &p2_latest)) {
			init_player_from_entry(p2, &p2_latest);
		} else {
			perror("read_last_entry (update_player_on_outcome)");
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
	append_entry_to_file(&p1_new_entry, full_p1_path);

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
					read_last_entry(file_path_with_player_dir(entry->d_name), \
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
						append_entry_to_file(&latest_ent, \
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
					read_last_entry(file_path_with_player_dir(line), \
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
						append_entry_to_file(&latest_ent, \
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
		if (0 == read_last_entry(full_player_path, &temp)) {
			int num_events;
			get_player_attended(full_player_path, &num_events);
			if (longest_attended < num_events) longest_attended = num_events;
			int num_outcomes = get_player_outcome_count(full_player_path);
			if (longest_outcomes < num_outcomes) longest_outcomes = num_outcomes;
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
			append_pr_entry_to_file_verbose(&players_pr_entries[i], \
				output_file_path, longest_name_length, longest_attended, \
				longest_outcomes);
		} else {
			append_pr_entry_to_file(&players_pr_entries[i], output_file_path, \
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
				if (0 == read_last_entry(full_player_path, &temp)) {
					int num_events;
					get_player_attended(full_player_path, &num_events);
					if (longest_attended < num_events) longest_attended = num_events;
					int num_outcomes = get_player_outcome_count(full_player_path);
					if (longest_outcomes < num_outcomes) longest_outcomes = num_outcomes;
					get_player_attended(full_player_path, &num_events);
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
				append_pr_entry_to_file_verbose(&players_pr_entries[i], \
					output_file_path, longest_name_length, longest_attended, \
					longest_outcomes);
			} else {
				append_pr_entry_to_file(&players_pr_entries[i], output_file_path, \
					longest_name_length);
			}
		}
		return 0;
	} else {
		perror("opendir (generate_ratings_file_full)");
		return -1;
	}
}

/** Takes a file path of a player file, prompts the user for the new name,
 * and renames Player 1 to the new name. Note that this does not rename
 * the file itself which may create errors unless fixed.
 *
 * \param '*file_path' the file path of the player file.
 * \return 0 upon success, a negative number upon failure.
 */
int refactor_file(char *file_path) {
	char new_name[MAX_NAME_LEN];
	printf("New player name: ");
	scanf("%s", new_name);
	/* Read entry from old file */
	struct entry cur_entry;
	read_start_from_file(file_path, &cur_entry);
	strncpy(&(cur_entry.name[0]), new_name, MAX_NAME_LEN);
	cur_entry.len_name = strlen(cur_entry.name);
	printf("new name %s\n", cur_entry.name);
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
	if (temp != \
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
	read_start_from_file(file_path, cur_entry);
	/* Get to the entries and begin reading them */
	fseek(base_file, 0, SEEK_SET);
	get_to_entries_in_file(base_file);
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
	struct entry ent;
	/* Read the starter data in the file */
	read_start_from_file(file_path, &ent);
	/* Go back to beginning of entries for reading */
	fseek(p_file, 0, SEEK_SET);
	get_to_entries_in_file(p_file);

	while (read_entry(p_file, &ent) == 0) {
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

int get_player_outcome_count(char *file_path) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (get_player_outcome_count)");
		return -1;
	}

	long num_outcomes = 0;
	struct entry cur_entry;
	/* Read the starter data in the file */
	read_start_from_file(file_path, &cur_entry);
	/* Go back to beginning of entries for reading */
	fseek(p_file, 0, SEEK_SET);
	get_to_entries_in_file(p_file);

	while (read_entry(p_file, &cur_entry) == 0) {
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
char *get_player_attended(char *file_path, int *ret_count) {
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (get_player_attended)");
		return NULL;
	}

	char len_of_name;
	char name[MAX_NAME_LEN];
	long tourneys_size = SIZE_PR_ENTRY;
	long tourneys_num = 0;
	char *tourneys = calloc(SIZE_PR_ENTRY * MAX_NAME_LEN, sizeof(char));
	char in_tourneys = 0;
	struct entry cur_entry;
	memset(name, 0, sizeof(name));
	/* Read the starter data in the file */
	if (1 != fread(&len_of_name, sizeof(char), 1, p_file)) return NULL;
	if (len_of_name != fread(name, sizeof(char), len_of_name, p_file)) {
		return NULL;
	}
	/* Get to the entries in the file and start reading them */
	fseek(p_file, 0, SEEK_SET);
	get_to_entries_in_file(p_file);

	while (read_entry(p_file, &cur_entry) == 0) {
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
				tourneys = realloc(tourneys, \
					SIZE_PR_ENTRY * MAX_NAME_LEN * sizeof(char));
				if (tourneys == NULL) {
					perror("realloc (get_player_attended)");
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

void print_player_attended(char *attended, int count) {
	// Print names of all tournaments attended by the player
	for (int i = 0; i < count; i++) {
		printf("%s\n", attended + i * MAX_NAME_LEN);
	}
}

char *players_in_player_dir(char *players, int *num) {
	DIR *p_dir;
	struct dirent *entry;

	if ((p_dir = opendir(player_dir)) != NULL) {
		*num = 0;
		while ((entry = readdir(p_dir)) != NULL) {
			// Make sure it doesn't count directories
			if (entry->d_type != DT_DIR) {
				int num_events;
				char *full_player_path = file_path_with_player_dir(entry->d_name);
				get_player_attended(full_player_path, &num_events);
				// If the player attended the minimum number of events
				if (num_events >= pr_minimum_events) {
					strncpy(&players[MAX_NAME_LEN * *(num)], entry->d_name, MAX_NAME_LEN);
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

char *players_in_player_dir_lexio(char *players, int *num) {
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
				char *full_player_path = file_path_with_player_dir(entry->d_name);
				get_player_attended(full_player_path, &num_events);
				// If the player attended the minimum number of events
				if (num_events >= pr_minimum_events) {
					int i = MAX_NAME_LEN * (*(num) - 1);
					// Find the right index to insert the name at
					while (strcmp(&players[i], entry->d_name) > 0 && i >= 0) {
						// Move later-occuring name further in the array
						strncpy(&players[i + MAX_NAME_LEN], &players[i], MAX_NAME_LEN);
						i -= MAX_NAME_LEN;
					}
					strncpy(&players[i + MAX_NAME_LEN], entry->d_name, MAX_NAME_LEN);
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

int get_record(char *player1, char *player2, struct record *ret) {
	char *full_player1_path = file_path_with_player_dir(player1);

	/* Read the starter data in the file */
	struct entry ent;
	read_start_from_file(full_player1_path, &ent);
	free(full_player1_path);

	FILE *p_file = fopen(full_player1_path, "rb");
	if (p_file == NULL) {
		perror("fopen (get_record)");
		return -1;
	}
	strncpy(ret->name, ent.name, MAX_NAME_LEN);
	// TODO: actually get player2 name from their file. '*player2' is just a file name
	strncpy(ret->opp_name, player2, MAX_NAME_LEN);
	ret->wins = 0;
	ret->losses = 0;
	ret->ties = 0;

	while (read_entry(p_file, &ent) == 0) {
		// If the opponent for the given entry is the player of interest
		if (0 == strcmp(ent.opp_name, player2)) {
			if (ent.gc > ent.opp_gc) ret->wins += 1;
			else if (ent.gc == ent.opp_gc) ret->ties += 1;
			else if (ent.gc < ent.opp_gc) ret->losses += 1;
		}
	}
	fclose(p_file);

	return 0;
}

int longest_name(char *players, int array_len) {
	int ret = 0;
	for (int i = 0; i < array_len; i++) {
		if (strlen(&players[MAX_NAME_LEN * i]) > ret) {
			ret = strlen(&players[MAX_NAME_LEN * i]);
		}
	}

	return ret;
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
	players_in_player_dir_lexio(players, &num_players);
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
		int col_width = strlen(&players[i * MAX_NAME_LEN]) + space_between_columns;
		char col[col_width];
		snprintf(col, col_width, "%-*s", col_width, &players[i * MAX_NAME_LEN]);
		strcat(output[0], col);
	}
	printf("%s\n", output[0]);

	if ((p_dir = opendir(player_dir)) != NULL) {
		for (int i = 0; i < num_players; i++) {
			// Add row title
			snprintf(output[i + 1], longest_n + space_between_columns, "%*s%*s", \
				longest_n, &players[i * MAX_NAME_LEN], space_between_columns, "");
			for (int j = 0; j < num_players; j++) {
				struct record temp_rec;
				get_record(&players[i * MAX_NAME_LEN], \
					&players[j * MAX_NAME_LEN], &temp_rec);
				// Make column width to be the length of the column title
				// plus a space character on each side
				char col[strlen(&players[j * MAX_NAME_LEN]) + space_between_columns];
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
	players_in_player_dir_lexio(players, &num_players);
	// 'num_players + 1' to accomodate one player per row and an extra row
	// for the column titles
	char output[num_players + 1][1024];
	// Empty the first line of output
	memset(output[0], 0, 1024);
	// Topmost, leftmost cell should be empty
	sprintf(output[0], ",");
	// Fill in column titles with player names + a comma delimiter
	for (int i = 0; i < num_players; i++) {
		strncat(output[0], &players[i * MAX_NAME_LEN], 1024 - 1 - strlen(output[0]));
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

int reset_players(void) {
	DIR *p_dir;
	struct dirent *entry;
	if ((p_dir = opendir(player_dir)) != NULL) {
		while ((entry = readdir(p_dir)) != NULL) {
			// Make sure it doesn't count directories
			if (entry->d_type != DT_DIR) {
				char *full_player_path = file_path_with_player_dir(entry->d_name);
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

	/* Initialize the player directory default file path */
	memset(player_dir, 0, sizeof(player_dir));
	strncpy(player_dir, ".players/", sizeof(player_dir) - 1);

	while ((opt = getopt_long(argc, argv, \
		"0a:A:b:B:c:Cd:gh:kl:m:MnNo:p:P:r:R:vw:x:", opt_table, NULL)) != -1) {
		if (opt == 'A') {
			int count;
			char *full_player_path = file_path_with_player_dir(optarg);
			char *attended = get_player_attended(full_player_path, &count);
			print_player_attended(attended, count);
			free(full_player_path);
			free(attended);
		} else if (opt == 'c') {
			char *full_player_path = file_path_with_player_dir(optarg);
			printf("%d\n", get_player_outcome_count(full_player_path));
			free(full_player_path);
		} else if (opt == 'd') {
			memset(player_dir, 0, sizeof(player_dir));
			strncpy(player_dir, optarg, sizeof(player_dir) - 1);
		} else if (opt == 'h') {
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
		} else if (opt == 'R') {
			char *full_player_path = file_path_with_player_dir(optarg);
			print_player_records(full_player_path);
			free(full_player_path);
		} else if (opt == 'x') {
			char *full_player_path = file_path_with_player_dir(optarg);
			remove_line_from_file(full_player_path);
			free(full_player_path);
		}

		switch (opt) {
			case '0':
				calc_absent_players = 0;
				break;
			case 'a':
				write_entry_from_input(file_path_with_player_dir(optarg));
				break;
			case 'b':
				if (keep_players == 0) reset_players();
				update_players(optarg);
				break;
			case 'C':
				print_matchup_table_csv();
				break;
			case 'B':
				if (keep_players == 0) reset_players();
				run_brackets(optarg);
				break;
			case 'g':
				use_games = 1;
				break;
			case 'k':
				keep_players = 1;
				break;
			case 'm':
				pr_minimum_events = atoi(optarg);
				break;
			case 'M':
				print_matchup_table();
				break;
			case 'n':
				colour_output = 0;
				break;
			case 'N':
				print_ties = 0;
				break;
			case 'p':
				o_generate_pr = 1;
				strncpy(pr_list_file_path, optarg, sizeof(pr_list_file_path) - 1);
				break;
			case 'w':
				outcome_weight = strtod(optarg, NULL);
				break;
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
			case 'v':
				verbose = 1;
				break;
		}
	}
}
