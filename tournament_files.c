/* Windows includes */
#ifdef _WIN32
#include <io.h>
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


/** Takes an tournaments name and returns an int representing whether it
 * found the tournament in the system.
 *
 * \param '*t_name' the name of the tournament to be searched for.
 * \return integer representing whether the function failed or succeeded.
 *     It will return < 0 if the function failed, and tournament's id (>=0)
 *     if it succeeded.
 */
int t_file_contains_tournament(char *t_name) {
	char *full_t_file_path = data_dir_file_path_t_file();

	FILE* t_file = fopen(full_t_file_path, "rb+");
	if (t_file == NULL) {
		fprintf(stderr, \
			"Error: t_file_contains_tournament(): " \
			"opening file \"%s\": ", \
			full_t_file_path);
		perror("");
		return -2;
	}

	int ret = -1;
	unsigned short num_t;
	char temp_name[MAX_NAME_LEN + 1];
	if (1 != fread(&num_t, sizeof(short), 1, t_file)) return -7;

	/* Binary search on file to find given name's id */
	long L = 0;
	long R = num_t - 1;
	long m;
	long oldm = 0;
	while (L <= R) {
		m = floor(((double) (L + R)) / 2.0);
		/* Seek to mth spot of array */
		fseek(t_file, \
			(m - oldm) * (MAX_NAME_LEN + 1 + sizeof(short)), SEEK_CUR);
		/* Read corresponding t_id of the array[m] */
		unsigned long start_of_m = ftell(t_file);
		short t_id = -1;
		if (1 != fread(&t_id, sizeof(short), 1, t_file)) return -8;

		/* Read t name of the array[m] */
		int j = 0;
		if (1 != fread(&temp_name[j], sizeof(char), 1, t_file)) return -11;
		/* Provided it hasn't hit a null terminator or end of file */
		while (temp_name[j] != '\0' && j < MAX_NAME_LEN && !(feof(t_file))) {
			j++;
			if (1 != fread(&temp_name[j], sizeof(char), 1, t_file)) return -11;
		}
		/* Seek to start of array[m] so fseek to next spot isn't offset */
		fseek(t_file, start_of_m, SEEK_SET);

		int comp = strncmp(&temp_name[0], t_name, MAX_NAME_LEN);
		if (0 > comp) {
			oldm = m;
			L = m + 1;
		} else if (0 < comp) {
			oldm = m;
			R = m - 1;
		} else {
			ret = t_id;
			R = L - 1; /* Terminate loop */
		}
	}

	fclose(t_file);
	free(full_t_file_path);
	/* The tournament name was not found, return -1 */
	return ret;
}


/** Helper function that writes the t_id and tournament name in the provided
 * entry to an open file.
 *
 * \param '*E' a pointer to a struct entry containing the info to be written.
 * \param '*f' an open file pointer where the content will be written.
 * \return 0 upon success, a negative integer upon failure.
 */
int write_new_t_name(struct entry *E, FILE *f) {
	if (1 != fwrite(&E->tournament_id, sizeof(short), 1, f)) return -1;
	if (E->len_t_name != \
		fwrite(&E->t_name, sizeof(char), E->len_t_name, f)) {

		return -2;
	}

	char zero = '\0';

	for (int i = 0; i < MAX_NAME_LEN + 1 - E->len_t_name; i++) {
		if (1 != fwrite(&zero, sizeof(char), 1, f)) return -3;
	}
	return 0;
}


/** Takes a struct entry containing a filled in 't_name' and set
 * 'tournament_id', and adds it to the system.
 *
 * \param '*E' a struct entry with a set 't_name' and 'tournament_id'.
 * \return integer representing whether the function failed or succeeded.
 *     It will return < 0 if the function failed, and 0 if it succeeded.
 */
int t_file_add_new_tournament(struct entry *E) {
#ifdef _WIN32
	char *full_t_file_path = data_dir_file_path_t_file();

	FILE *t_file = fopen(full_t_file_path, "rb");
	if (t_file == NULL) {
		fprintf(stderr, \
			"Error: t_file_add_new_tournament(): " \
			"opening file \"%s\": ", \
			full_t_file_path);
		perror("");
		return -1;
	}

	char new_file_name[] = { "tempG2MEXXXXXX\0" };
	int r = mkstemp(new_file_name);
	close(r);
	unlink(new_file_name);

	FILE *new_file = fopen(new_file_name, "wb+");
	if (new_file == NULL) {
		fprintf(stderr, \
			"Error: t_file_add_new_tournament(): " \
			"opening file \"%s\": ", \
			new_file_name);
		perror("");
		return -2;
	}
/* If compiling on macOS or Linux */
#else
	char *full_t_file_path = data_dir_file_path_t_file();

	FILE *t_file = fopen(full_t_file_path, "rb");
	if (t_file == NULL) {
		fprintf(stderr, \
			"Error: t_file_add_new_tournament(): " \
			"opening file \"%s\": ", \
			full_t_file_path);
		perror("");
		return -1;
	}

	char new_file_name[] = { "tempG2MEXXXXXX\0" };
	int r = mkstemp(new_file_name);
	close(r);
	unlink(new_file_name);

	FILE *new_file = fopen(new_file_name, "wb+");
	if (new_file == NULL) {
		fprintf(stderr, \
			"Error: t_file_add_new_tournament(): " \
			"opening file \"%s\": ", \
			new_file_name);
		perror("");
		return -2;
	}
#endif
	char zero = '\0';
	unsigned short num_t;
	char wrote_new_name = 0;

	/* Read number of tournaments and write [said number + 1] to temp file */
	if (1 != fread(&num_t, sizeof(short), 1, t_file)) return -11;
	/* Correct the t_id (0 indexed id) */
	E->tournament_id = num_t;
	num_t += 1;
	if (1 != fwrite(&num_t, sizeof(short), 1, new_file)) return -12;

	/* Read and write all the names of the tournaments to the temp file */
	for (int i = 0; i < num_t - 1; i++) {
		unsigned short id = -1;
		char name[MAX_NAME_LEN + 1];

		if (1 != fread(&id, sizeof(short), 1, t_file)) return -13;
		if (MAX_NAME_LEN + 1 != \
			fread(&name[0], sizeof(char), MAX_NAME_LEN + 1, t_file)) {

			return -15;
		}

		/* If the name to be inserted is lexiographically before the name about
		 * to be written, write the new name first */
		if (wrote_new_name != 1) {
			if (0 > strncmp(E->t_name, &name[0], E->len_t_name)) {
				if (0 != write_new_t_name(E, new_file)) return -16;
				wrote_new_name = 1;
			}
		}
		if (1 != fwrite(&id, sizeof(short), 1, new_file)) return -14;
		if (MAX_NAME_LEN + 1 != \
			fwrite(&name[0], sizeof(char), MAX_NAME_LEN + 1, new_file)) {

			return -14;
		}
	}

	if (wrote_new_name != 1) {
		if (0 != write_new_t_name(E, new_file)) return -16;
	}

	fclose(new_file);
	fclose(t_file);
	/* Delete original file */
	remove(full_t_file_path);
	/* Copy temp file to original file path */
	rename(new_file_name, full_t_file_path);
	free(full_t_file_path);

	char *full_t_id_file_path = data_dir_file_path_t_id_file();
	FILE *t_id_file = fopen(full_t_id_file_path, "rb+");
	if (t_id_file == NULL) {
		fprintf(stderr, \
			"Error: t_file_add_new_tournament(): " \
			"opening file \"%s\": ", \
			full_t_id_file_path);
		perror("");
		return -1;
	}
	/* Update counter at start of file */
	if (1 != fwrite(&num_t, sizeof(short), 1, t_id_file)) return -12;

	if (0 != fseek(t_id_file, 0, SEEK_END)) return -13;

	if (E->len_t_name != \
		fwrite(&E->t_name, sizeof(char), E->len_t_name, t_id_file)) return -14;
	for (int i = 0; i < MAX_NAME_LEN + 1 - E->len_t_name; i++) {
		if (1 != fwrite(&zero, sizeof(char), 1, t_id_file)) return -14;
	}
	fclose(t_id_file);
	free(full_t_id_file_path);
	return 0;
}


/** Takes an entry containing the tournament id to be searched for. Updates the
 * struct entry's 't_name' field to the tournament name associated with the
 * given 'tournament_id'.
 *
 * \param '*E' the struct entry containing the tournament id to be
 *     searched for, and which will have its 't_name' field updated to
 *     the name, if this function is successful.
 * \return 0 upon success, and a negative value if there was an error.
 */
int t_file_get_tournament_name_from_id(struct entry *E) {

	char *full_t_id_file_path = data_dir_file_path_t_id_file();
	FILE* t_id_file = fopen(full_t_id_file_path, "rb+");
	if (t_id_file == NULL) {
		fprintf(stderr, \
			"Error: t_file_get_tournament_name_from_id(): " \
			"opening file \"%s\": ", \
			full_t_id_file_path);
		perror("");
		return -1;
	}

	unsigned short num_t;
	if (1 != fread(&num_t, sizeof(short), 1, t_id_file)) return -2;
	/* If the id is outside of the range of the array */
	if (E->tournament_id >= num_t) return -3;
	/* Seek to the location of the name in the file by using its id */
	fseek(t_id_file, \
		sizeof(short) + E->tournament_id * (MAX_NAME_LEN + 1), SEEK_SET);

	int j = 0;
	/* Read first byte and add to name */
	if (1 != fread(&E->t_name[j], sizeof(char), 1, t_id_file )) return -4;
	/* Provided it hasn't hit a null terminator or end of file */
	while (E->t_name[j] != '\0' && j < MAX_NAME_LEN && !(feof(t_id_file ))) {
		j++;
		if (1 != fread(&E->t_name[j], sizeof(char), 1, t_id_file)) return -5;
	}
	E->len_t_name = j - 1;

	fclose(t_id_file);
	free(full_t_id_file_path);

	return 0;
}


/** Takes an struct entry containing the tournament name to be
 * searched for. Returns the id of a tournament, given the name of a
 * tournament. Returns -1 if the tournament of that name could not be found.
 * Returns < -1 if there was an error.
 *
 * \param '*E' the struct entry containing the tournament name to be
 *     searched for and which will have its tournament_id field updated to
 *     the id if this function is successful.
 * \return non-negative number upon success (the tournament's id), -1 if the
 *     name could not be found, and < -1 if there was an error.
 */
int t_file_get_tournament_id_from_name(struct entry *E) {

	char *full_t_file_path = data_dir_file_path_t_file();
	FILE* t_file = fopen(full_t_file_path, "rb+");
	if (t_file == NULL) {
		fprintf(stderr, \
			"Error: t_file_get_tournament_id_from_name(): " \
			"opening file \"%s\": ", \
			full_t_file_path);
		perror("");
		return -1;
	}

	int ret = -1;
	unsigned short num_t;
	char temp_name[MAX_NAME_LEN + 1];
	if (1 != fread(&num_t, sizeof(short), 1, t_file)) return -7;

	/* Binary search on file to find given name's id */
	long L = 0;
	long R = num_t - 1;
	long m;
	long oldm = 0;
	while (L <= R) {
		m = floor(((double) (L + R)) / 2.0);
		/* Seek to mth spot of array */
		fseek(t_file, \
			(m - oldm) * (MAX_NAME_LEN + 1 + sizeof(short)), SEEK_CUR);
		/* Read corresponding t_id of the array[m] */
		unsigned long start_of_m = ftell(t_file);
		short t_id = -1;
		if (1 != fread(&t_id, sizeof(short), 1, t_file)) return -8;

		/* Read t name of the array[m] */
		int j = 0;
		if (1 != fread(&temp_name[j], sizeof(char), 1, t_file)) return -11;
		/* Provided it hasn't hit a null terminator or end of file */
		while (temp_name[j] != '\0' && j < MAX_NAME_LEN && !(feof(t_file))) {
			j++;
			if (1 != fread(&temp_name[j], sizeof(char), 1, t_file)) return -11;
		}
		/* Seek to start of array[m] so fseek to next spot isn't offset */
		fseek(t_file, start_of_m, SEEK_SET);

		int comp = strncmp(&temp_name[0], E->t_name, MAX_NAME_LEN);
		if (0 > comp) {
			oldm = m;
			L = m + 1;
		} else if (0 < comp) {
			oldm = m;
			R = m - 1;
		} else {
			ret = t_id;
			E->tournament_id = t_id;
			R = L - 1; /* Terminate loop */
		}
	}

	/* The tournament name was not found, return -1 */
	return ret;
}


/** On success, returns the id of the most recent season. Returns < -1 if there
 * was an error. -1 is a reserved return value since season ids are 0 indexed,
 * and an empty system's most recent season should be -1.
 *
 * \return number >= -1 upon success (the most recent season id),
 *     and < -2 if there was an error.
 */
short s_file_get_latest_season_id(void) {

	char *full_season_file_path = data_dir_file_path_season_file();
	FILE* s_file = fopen(full_season_file_path, "rb+");
	if (s_file == NULL) {
		fprintf(stderr, \
			"Error: s_file_get_latest_season_id(): " \
			"opening file \"%s\": ", \
			full_season_file_path);
		perror("");
		return -2;
	}

	int ret = -3;
	if (1 != fread(&ret, sizeof(short), 1, s_file)) return -4;
	fclose(s_file);

	return ret;
}


/** Writes the latest season id 'new_s_id' to the global season file.
 *
 * \return 0 upon success, a negative number if there was an error.
 */
int s_file_set_latest_season_id(int new_s_id) {

	char *full_season_file_path = data_dir_file_path_season_file();
	FILE* s_file = fopen(full_season_file_path, "rb+");
	if (s_file == NULL) {
		fprintf(stderr, \
			"Error: s_file_set_latest_season_id(): " \
			"opening file \"%s\": ", \
			full_season_file_path);
		perror("");
		return -1;
	}

	if (1 != fwrite(&new_s_id, sizeof(short), 1, s_file)) { return -2; }
	fclose(s_file);

	return 0;
}
