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

#include "opp_files.h"
#include "G2ME.h"
#include "player_dir.h"


/** Takes an open opp_file, and an  opponents name and returns an int
 * representing whether it found the opponent in the system.
 *
 * \param '*f' the open opp_file for reading.
 * \param '*opp_name' the name of the opponent to be searched for.
 * \return integer representing whether the function failed or succeeded.
 *     It will return < 0 if the function failed, and opponent's id (>=0)
 *     if it succeeded.
 */
int opp_file_open_contains_opponent(FILE *f, char *opp_name) {

	fseek(f, 0, SEEK_SET);

	int ret = -1;
	unsigned short num_opp;
	char temp_name[MAX_NAME_LEN + 1];
	if (1 != fread(&num_opp, sizeof(short), 1, f)) return -2;

	/* Binary search on file to find given name's id */
	long L = 0;
	long R = num_opp - 1;
	long m;
	long oldm = 0;
	while (L <= R) {
		m = floor(((double) (L + R)) / 2.0);
		/* Seek to mth spot of array */
		fseek(f, (m - oldm) * (MAX_NAME_LEN + 1 + sizeof(short)), SEEK_CUR);
		/* Read corresponding opp_id of the array[m] */
		unsigned long start_of_m = ftell(f);
		short opp_id = -1;
		if (1 != fread(&opp_id, sizeof(short), 1, f)) return -3;

		/* Read opp name of the array[m] */
		int j = 0;
		if (1 != fread(&temp_name[j], sizeof(char), 1, f)) return -4;
		/* Provided it hasn't hit a null terminator or end of file */
		while (temp_name[j] != '\0' && j < MAX_NAME_LEN && !(feof(f))) {
			j++;
			if (1 != fread(&temp_name[j], sizeof(char), 1, f)) return -5;
		}
		/* Seek to start of array[m] so fseek to next spot isn't offset */
		fseek(f, start_of_m, SEEK_SET);

		int comp = strncmp(&temp_name[0], opp_name, MAX_NAME_LEN);
		if (0 > comp) {
			oldm = m;
			L = m + 1;
		} else if (0 < comp) {
			oldm = m;
			R = m - 1;
		} else {
			ret = opp_id;
			R = L - 1; /* Terminate loop */
		}
	}

	/* The opponent name was not found, return -1 */
	return ret;
}


/** Takes an opponents name and returns an int representing whether it
 * found the opponent in the system.
 *
 * \param '*opp_name' the name of the opponent to be searched for.
 * \return integer representing whether the function failed or succeeded.
 *     It will return < 0 if the function failed, and opponent's id (>=0)
 *     if it succeeded.
 */
int opp_file_contains_opponent(char *opp_name) {

	char *full_opp_file_path = data_dir_file_path_opp_file();

	FILE* opp_file = fopen(full_opp_file_path, "rb+");
	if (opp_file == NULL) {
		fprintf(stderr, \
			"Error: opp_file_contains_opponent(): " \
			"opening file \"%s\": ", \
			full_opp_file_path);
		perror("");
		return -1;
	}

	int ret = -1;
	unsigned short num_opp;
	char temp_name[MAX_NAME_LEN + 1];
	if (1 != fread(&num_opp, sizeof(short), 1, opp_file)) return -2;

	/* Binary search on file to find given name's id */
	long L = 0;
	long R = num_opp - 1;
	long m;
	long oldm = 0;
	while (L <= R) {
		m = floor(((double) (L + R)) / 2.0);
		/* Seek to mth spot of array */
		fseek(opp_file, \
			(m - oldm) * (MAX_NAME_LEN + 1 + sizeof(short)), \
			SEEK_CUR);
		/* Read corresponding opp_id of the array[m] */
		unsigned long start_of_m = ftell(opp_file);
		short opp_id = -1;
		if (1 != fread(&opp_id, sizeof(short), 1, opp_file)) return -3;

		/* Read opp name of the array[m] */
		int j = 0;
		if (1 != fread(&temp_name[j], sizeof(char), 1, opp_file)) return -4;
		/* Provided it hasn't hit a null terminator or end of file */
		while (temp_name[j] != '\0' && j < MAX_NAME_LEN && !(feof(opp_file))) {
			j++;
			if (1 != fread(&temp_name[j], sizeof(char), 1, opp_file)) {
				return -5;
			}
		}
		/* Seek to start of array[m] so fseek to next spot isn't offset */
		fseek(opp_file, start_of_m, SEEK_SET);

		int comp = strncmp(&temp_name[0], opp_name, MAX_NAME_LEN);
		if (0 > comp) {
			oldm = m;
			L = m + 1;
		} else if (0 < comp) {
			oldm = m;
			R = m - 1;
		} else {
			ret = opp_id;
			R = L - 1; /* Terminate loop */
		}
	}

	fclose(opp_file);
	free(full_opp_file_path);
	/* The opponent name was not found, return -1 */
	return ret;
}


/** Helper function that writes the opp_id and opponent name in the provided
 * entry to an open file.
 *
 * \param '*E' a pointer to a struct entry containing the info to be written.
 * \param '*f' an open file pointer where the content will be written.
 * \return 0 upon success, a negative integer upon failure.
 */
int write_new_name(struct entry *E, FILE *f) {
	if (1 != fwrite(&E->opp_id, sizeof(short), 1, f)) return -1;
	if (E->len_opp_name != \
		fwrite(&E->opp_name, sizeof(char), E->len_opp_name, f)) {

		return -2;
	}

	char zero = '\0';

	for (int i = 0; i < MAX_NAME_LEN + 1 - E->len_opp_name; i++) {
		if (1 != fwrite(&zero, sizeof(char), 1, f)) return -3;
	}
	return 0;
}



/** Takes a struct entry containing a filled in 'opp_name' and set 'opp_id',
 * and adds it to the system.
 *
 * \param '*E' a struct entry with a set 'opp_name' and 'opp_id'.
 * \return integer representing whether the function failed or succeeded.
 *     It will return < 0 if the function failed, and 0 if it succeeded.
 */
int opp_file_add_new_opponent(struct entry *E) {
#ifdef _WIN32
	// TODO: switch to windows temp file stuff
	/* Get the name for the temp file */
	// TODO what if .[original name] already exists? */
	char dir[strlen(file_path) + 1];
	char base[strlen(file_path) + 1];
	memset(dir, 0, sizeof(dir));
	memset(base, 0, sizeof(base));
	strncpy(dir, file_path, sizeof(dir) - 1);
	strncpy(base, file_path, sizeof(base) - 1);

	char new_file_name[MAX_FILE_PATH_LEN + 1];
	memset(new_file_name, 0, sizeof(new_file_name));
	/* Add the full path up to the file */
	strncat(new_file_name, dirname(dir), sizeof(new_file_name) - 1);
	strncat(new_file_name, "/", \
		sizeof(new_file_name) - strlen(new_file_name) - 1);
	/* Add the temp file */
	strncat(new_file_name, ".", \
		sizeof(new_file_name) - strlen(new_file_name) - 1);
	strncat(new_file_name, basename(base), \
		sizeof(new_file_name) - strlen(new_file_name) - 1);

	FILE *opp_file = fopen(file_path, "rb");
	if (opp_file == NULL) {
		fprintf(stderr, \
			"Error: opp_file_add_new_opponent(): " \
			"opening file \"%s\": ", \
			full_opp_file_path);
		perror("");
		return -1;
	}
	FILE *new_file = fopen(new_file_name, "wb+");
	if (new_file == NULL) {
		fprintf(stderr, \
			"Error: opp_file_add_new_opponent(): " \
			"opening file \"%s\": ", \
			new_file_name);
		return -2;
	}
/* If this is being compiled on macOS or Linux */
#else
	char *full_opp_file_path = data_dir_file_path_opp_file();

	FILE *opp_file = fopen(full_opp_file_path, "rb");
	if (opp_file == NULL) {
		fprintf(stderr, \
			"Error: opp_file_add_new_opponent(): " \
			"opening file \"%s\": ", \
			full_opp_file_path);
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
			"Error: opp_file_add_new_opponent(): " \
			"opening file \"%s\": ", \
			new_file_name);
		perror("");
		return -2;
	}
#endif
	char zero = '\0';
	unsigned short num_opp;
	char wrote_new_name = 0;

	/* Read number of opponents and write [said number + 1] to temp file */
	if (1 != fread(&num_opp, sizeof(short), 1, opp_file)) return -11;
	/* Correct the opp_id (0 indexed id) */
	E->opp_id = num_opp;
	num_opp += 1;
	if (1 != fwrite(&num_opp, sizeof(short), 1, new_file)) return -12;

	/* Read and write all the names of the opponents to the temp file */
	for (int i = 0; i < num_opp - 1; i++) {
		unsigned short id = -1;
		char name[MAX_NAME_LEN + 1];

		if (1 != fread(&id, sizeof(short), 1, opp_file)) return -13;
		if (MAX_NAME_LEN + 1 != \
			fread(&name[0], sizeof(char), MAX_NAME_LEN + 1, opp_file)) {

			return -15;
		}

		/* If the name to be inserted is lexiographically before the name about
		 * to be written, write the new name first */
		if (wrote_new_name != 1) {
			if (0 > strncmp(E->opp_name, &name[0], E->len_opp_name)) {
				if (0 != write_new_name(E, new_file)) return -16;
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
		if (0 != write_new_name(E, new_file)) return -16;
	}

	fclose(new_file);
	fclose(opp_file);
	/* Delete original file */
	remove(full_opp_file_path);
	/* Copy temp file to original file path */
	rename(new_file_name, full_opp_file_path);
	free(full_opp_file_path);

	char *full_opp_id_file_path = data_dir_file_path_opp_id_file();
	FILE *opp_id_file = fopen(full_opp_id_file_path, "rb+");
	if (opp_id_file == NULL) {
		fprintf(stderr, \
			"Error: opp_file_add_new_opponent(): " \
			"opening file \"%s\": ", \
			full_opp_file_path);
		perror("");
		return -1;
	}
	/* Update counter at start of file */
	if (1 != fwrite(&num_opp, sizeof(short), 1, opp_id_file)) return -12;

	if (0 != fseek(opp_id_file, 0, SEEK_END)) return -13;

	if (E->len_opp_name != \
		fwrite(&E->opp_name, sizeof(char), E->len_opp_name, opp_id_file)) {

		return -14;
	}

	for (int i = 0; i < MAX_NAME_LEN + 1 - E->len_opp_name; i++) {
		if (1 != fwrite(&zero, sizeof(char), 1, opp_id_file)) return -14;
	}
	fclose(opp_id_file);
	free(full_opp_id_file_path);
	return 0;
}


/** Takes a struct entry with a set 'opp_id', finds its associated
 * opp_name and fills in the struct entry's 'opp_name' with it.
 *
 * \param '*E' the struct entry with a set 'opp_id'
 * \return integer representing whether the function failed or succeeded.
 *     It will return < 0 if the function failed, and 0 if it succeeded.
 */
int opp_file_get_name_from_id(struct entry *E) {

	int j = 0;
	char *full_opp_id_file_path = data_dir_file_path_opp_id_file();
	FILE* opp_id_file = fopen(full_opp_id_file_path, "rb+");
	if (opp_id_file == NULL) {
		fprintf(stderr, \
			"Error: opp_file_get_name_from_id(): " \
			"opening file \"%s\": ", \
			full_opp_id_file_path);
		perror("");
		return -1;
	}

	unsigned short num_opp;
	if (1 != fread(&num_opp, sizeof(short), 1, opp_id_file )) return -2;
	/* If the id is outside of the range of the array */
	if (E->opp_id >= num_opp) return -3;
	/* Seek to the location of the name in the file by using its id */
	fseek(opp_id_file, \
		sizeof(short) + E->opp_id * (MAX_NAME_LEN + 1), SEEK_SET);

	/* Read first byte and add to name */
	if (1 != fread(&E->opp_name[j], sizeof(char), 1, opp_id_file )) return -4;
	/* Provided it hasn't hit a null terminator or end of file */
	while (E->opp_name[j] != '\0' \
		&& j < MAX_NAME_LEN \
		&& !(feof(opp_id_file))) {

		j++;
		if (1 != fread(&E->opp_name[j], sizeof(char), 1, opp_id_file)) {
			return -5;
		}
	}
	E->len_opp_name = j;

	fclose(opp_id_file);
	free(full_opp_id_file_path);

	return 0;
}


/** Takes a struct entry with a set 'opp_name', finds its associated
 * opp_id and fills in the struct entry's 'opp_id with it.
 *
 * \param '*E' the struct entry with a set 'opp_name'
 * \return integer representing whether the function failed or succeeded.
 *     It will return < 0 if the function failed, and >= 0 if it succeeded.
 */
int opp_file_get_id_from_name(struct entry *E) {

	char *full_opp_file_path = data_dir_file_path_opp_file();
	FILE* opp_file = fopen(full_opp_file_path, "rb+");
	if (opp_file == NULL) {
		fprintf(stderr, \
			"Error: opp_file_get_id_from_name(): " \
			"opening file \"%s\": ", \
			full_opp_file_path);
		perror("");
		return -1;
	}

	int ret = -1;
	unsigned short num_opp;
	char temp_name[MAX_NAME_LEN + 1];
	if (1 != fread(&num_opp, sizeof(short), 1, opp_file)) return -2;

	/* Binary search on file to find given name's id */
	long L = 0;
	long R = num_opp - 1;
	long m;
	long oldm = 0;
	while (L <= R) {
		m = floor(((double) (L + R)) / 2.0);
		/* Seek to mth spot of array */
		fseek(opp_file, \
			(m - oldm) * (MAX_NAME_LEN + 1 + sizeof(short)), SEEK_CUR);
		/* Read corresponding opp_id of the array[m] */
		unsigned long start_of_m = ftell(opp_file);
		short opp_id = -1;
		if (1 != fread(&opp_id, sizeof(short), 1, opp_file)) return -3;

		/* Read opp name of the array[m] */
		int j = 0;
		if (1 != fread(&temp_name[j], sizeof(char), 1, opp_file)) return -4;
		/* Provided it hasn't hit a null terminator or end of file */
		while (temp_name[j] != '\0' && j < MAX_NAME_LEN && !(feof(opp_file))) {
			j++;
			if (1 != fread(&temp_name[j], sizeof(char), 1, opp_file)) {
				return -5;
			}
		}
		/* Seek to start of array[m] so fseek to next spot isn't offset */
		fseek(opp_file, start_of_m, SEEK_SET);

		int comp = strncmp(&temp_name[0], E->opp_name, MAX_NAME_LEN);
		if (0 > comp) {
			oldm = m;
			L = m + 1;
		} else if (0 < comp) {
			oldm = m;
			R = m - 1;
		} else {
			ret = opp_id;
			E->opp_id = opp_id;
			R = L - 1; /* Terminate loop */
		}
	}

	fclose(opp_file);
	free(full_opp_file_path);
	/* The opponent name was not found, return -1 */
	return ret;
}


/** Takes no arguments, and returns the number of opponents in the system.
 * This number will include the RD adjustment opponent unless 'exclude_rd_adj'
 * is set to 'EXCLUDE_RD_ADJ'.
 *
 * \param 'exclude_rd_adj' if set to 'EXCLUDE_RD_ADJ', this function will not
 *     include the RD adjustment opponent name in the final count.
 * \return a positive integer representing the number of opponents in the
 *     system upon success, and a negative integer upon failure.
 */
int opp_file_num_opponents(char exclude_rd_adj) {

	char *full_opp_file_path = data_dir_file_path_opp_file();
	FILE* opp_file = fopen(full_opp_file_path, "rb+");
	if (opp_file == NULL) {
		fprintf(stderr, \
			"Error: opp_file_num_opponents(): " \
			"opening file \"%s\": ", \
			full_opp_file_path);
		perror("");
		return -1;
	}

	unsigned short num_opp;
	if (1 != fread(&num_opp, sizeof(short), 1, opp_file)) return -2;

	/* If the RD adjustment name is to be excluded, lower the number of
	 * opponents by 1 since the RD adjustment opponent is guaranteed exist */
	if (exclude_rd_adj == EXCLUDE_RD_ADJ) num_opp -= 1;

	fclose(opp_file);
	free(full_opp_file_path);
	return num_opp;
}


/** Takes one argument, telling the function whether it should include the RD
 * adjustment opponent name, and reads all the names of the opponents in the
 * system into an array, which is then returned. The array returned is sorted
 * lexiographically. The array is malloc'd and must be free'd by the caller.
 *
 * \param 'exclude_rd_adj' if set to 'EXCLUDE_RD_ADJ', this function will not
 *     include the RD adjustment opponent name.
 * \param '*ret_array_len' a pointer to a short that will have its value
 *     updated to the length of the array.
 * \return Upon success, a pointer to an array of (MAX_NAME_LEN + 1) blocks,
 *     each containing the name of an opponent in the system. Returns NULL,
 *     upon failure.
 */
char *opp_file_get_all_opponent_names(char exclude_rd_adj, \
	short *ret_array_len) {

	char *full_opp_file_path = data_dir_file_path_opp_file();

	FILE* opp_file = fopen(full_opp_file_path, "rb+");
	if (opp_file == NULL) {
		fprintf(stderr, \
			"Error: opp_file_get_all_opponent_names(): " \
			"opening file \"%s\": ", \
			full_opp_file_path);
		perror("");
		return NULL;
	}

	short temp;
	if (1 != fread(&temp, sizeof(short), 1, opp_file)) return NULL;
	/* If the RD adjustment name is to be excluded, lower the number of
	 * opponents by 1 since the RD adjustment opponent is guaranteed exist */
	if (exclude_rd_adj == EXCLUDE_RD_ADJ) *ret_array_len = temp - 1;
	else *ret_array_len = temp;

	char *ret = \
		(char *)malloc(sizeof(char) * (MAX_NAME_LEN + 1) * *ret_array_len);
	short j = 0;

	for (int i = 0; i < temp; i++) {
		/* Read corresponding opp_id of the array[i] */
		short id;
		if (1 != fread(&id, sizeof(short), 1, opp_file)) return NULL;

		/* If the RD adjustment name is to be excluded */
		if (exclude_rd_adj == EXCLUDE_RD_ADJ) {

			/* If this opponent has id 0, it is the RD adj opponent */
			if (id == 0) {
				/* Seek over the name */
				if (0 != fseek(opp_file, sizeof(char) * (MAX_NAME_LEN + 1), \
					SEEK_CUR)) {

					return NULL;
				}
				continue;
			}
		}

		if (MAX_NAME_LEN + 1 != \
			fread(&ret[(MAX_NAME_LEN + 1) * j], sizeof(char), \
			MAX_NAME_LEN + 1, opp_file)) {

			return NULL;
		}
		j++;
	}

	fclose(opp_file);
	free(full_opp_file_path);
	return ret;
}
