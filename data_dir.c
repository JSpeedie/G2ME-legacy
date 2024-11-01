/* Non-windows includes */
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
/* Windows includes */
#ifdef _WIN32
#include <windows.h>
#endif
/* Local Includes */
#include "data_dir.h"
#include "sorting.h" /* Includes G2ME.h -> glicko2.h, p_files.h */
#include "fileops.h"


#ifdef __linux__
const char *DEFAULT_DATA_DIR = ".data/";
#elif _WIN32
const char *DEFAULT_DATA_DIR = ".\\.data\\";
#else
const char *DEFAULT_DATA_DIR = ".data/";
#endif


int data_dir_check_and_create(const char *data_dir_file_path) {
	DIR *d = opendir(data_dir_file_path);
	/* If 'player_dir' DOES exist */
	if (d) closedir(d);
	else if (errno == ENOENT) {
		fprintf(stderr, \
			"G2ME: Warning: 'data_dir' did not exist, creating...\n");
		/* If there was an error making the data directory */
#ifdef __linux__
		if (0 != mkdir(data_dir_file_path, 0700)) {
#elif _WIN32
		if (0 != mkdir(data_dir_file_path)) {
#else
		if (0 != mkdir(data_dir_file_path, 0700)) {
#endif
			perror("mkdir (data_dir_check_and_create)");
			return -1;
		}
	} else {
		perror("opendir (data_dir_check_and_create)");
		return -2;
	}

	/* Get file paths for universal files */
	char *full_opp_file_path = \
		data_dir_file_path_opp_file(data_dir_file_path);
	char *full_opp_id_file_path = \
		data_dir_file_path_opp_id_file(data_dir_file_path);
	char *full_t_file_path = \
		data_dir_file_path_t_file(data_dir_file_path);
	char *full_t_id_file_path = \
		data_dir_file_path_t_id_file(data_dir_file_path);
	char *full_season_file_path = \
		data_dir_file_path_season_file(data_dir_file_path);
#ifdef _WIN32
	char of_exist = _access(full_opp_file_path, 0) != -1;
	char oif_exist = _access(full_opp_id_file_path, 0) != -1;
	char tf_exist = _access(full_t_file_path, 0) != -1;
	char tif_exist = _access(full_t_id_file_path, 0) != -1;
	char sf_exist = _access(full_season_file_path, 0) != -1;
#else
	char of_exist = access(full_opp_file_path, R_OK | W_OK) != -1;
	char oif_exist = access(full_opp_id_file_path, R_OK | W_OK) != -1;
	char tf_exist = access(full_t_file_path, R_OK | W_OK) != -1;
	char tif_exist = access(full_t_id_file_path, R_OK | W_OK) != -1;
	char sf_exist = access(full_season_file_path, R_OK | W_OK) != -1;
#endif

	short num_opp = 1; /* 1 because we add the rd adjustment name */
	short num_t = 0;
	short season_num = -1;

	/* If sf doesn't exist, create it */
	if (!sf_exist) {
		FILE *s_file = fopen(full_season_file_path, "wb+");
		if (s_file == NULL) {
			perror("fopen (data_dir_check_and_create)");
			return -1;
		}
		if (1 != fwrite(&season_num, sizeof(short), 1, s_file)) return -12;
		fclose(s_file);
		free(full_season_file_path);
	}
	/* If of doesn't exist, create it */
	if (!of_exist) {
		FILE *opp_file = fopen(full_opp_file_path, "wb+");
		if (opp_file == NULL) {
			perror("fopen (data_dir_check_and_create)");
			return -1;
		}
		if (1 != fwrite(&num_opp, sizeof(short), 1, opp_file)) return -12;
		opp_file_open_write_rd_adj_name_with_id(opp_file);
		free(full_opp_file_path);
		fclose(opp_file);
	}

	/* If oif doesn't exist, create it */
	if (!oif_exist) {
		FILE *opp_id_file = fopen(full_opp_id_file_path, "wb+");
		if (opp_id_file == NULL) {
			perror("fopen (data_dir_check_and_create)");
			return -1;
		}
		if (1 != fwrite(&num_opp, sizeof(short), 1, opp_id_file)) return -12;
		opp_id_file_open_write_rd_adj_name(opp_id_file);
		fclose(opp_id_file);
		free(full_opp_id_file_path);
	}
	/* If tf doesn't exist, create it */
	if (!tf_exist) {
		FILE *t_file = fopen(full_t_file_path, "wb+");
		if (t_file == NULL) {
			perror("fopen (data_dir_check_and_create)");
			return -1;
		}
		if (1 != fwrite(&num_t, sizeof(short), 1, t_file)) return -12;
		free(full_t_file_path);
		fclose(t_file);
	}

	/* If tif doesn't exist, create it */
	if (!tif_exist) {
		FILE *t_id_file = fopen(full_t_id_file_path, "wb+");
		if (t_id_file == NULL) {
			perror("fopen (data_dir_check_and_create)");
			return -1;
		}
		if (1 != fwrite(&num_t, sizeof(short), 1, t_id_file)) return -12;
		fclose(t_id_file);
		free(full_t_id_file_path);
	}

	return 0;
}


/* Deletes every file in the data directory 'data_dir', creating
 * the universal G2ME files, necessary for usage.
 *
 * \return an int representing if the function succeeded or not.
 *     Negative if there was an error, 0 on success.
 */
int data_dir_reset(const char *data_dir_file_path) {
	char *full_opp_file_path = \
		data_dir_file_path_opp_file(data_dir_file_path);
	char *full_opp_id_file_path = \
		data_dir_file_path_opp_id_file(data_dir_file_path);
	char *full_t_file_path = \
		data_dir_file_path_t_file(data_dir_file_path);
	char *full_t_id_file_path = \
		data_dir_file_path_t_id_file(data_dir_file_path);
	char *full_season_file_path = \
		data_dir_file_path_season_file(data_dir_file_path);
	remove(full_opp_file_path);
	remove(full_opp_id_file_path);
	remove(full_t_file_path);
	remove(full_t_id_file_path);
	remove(full_season_file_path);

	short num_opp = 1; /* 1 because we add the rd adjustment name */
	short num_t = 0;
	short season_num = -1;

	/* Write the most recent season number (default to -1) */
	FILE *s_file = fopen(full_season_file_path, "wb+");
	if (s_file == NULL) {
		perror("fopen (data_dir_reset_players)");
		return -1;
	}
	if (1 != fwrite(&season_num, sizeof(short), 1, s_file)) return -12;
	fclose(s_file);
	free(full_season_file_path);

	FILE *opp_file = fopen(full_opp_file_path, "wb+");
	if (opp_file == NULL) {
		perror("fopen (data_dir_reset_players)");
		return -1;
	}
	if (1 != fwrite(&num_opp, sizeof(short), 1, opp_file)) return -12;
	opp_file_open_write_rd_adj_name_with_id(opp_file);
	fclose(opp_file);
	free(full_opp_file_path);

	FILE *opp_id_file = fopen(full_opp_id_file_path, "wb+");
	if (opp_id_file == NULL) {
		perror("fopen (data_dir_reset_players)");
		return -1;
	}
	if (1 != fwrite(&num_opp, sizeof(short), 1, opp_id_file)) return -12;
	opp_id_file_open_write_rd_adj_name(opp_id_file);
	fclose(opp_id_file);
	free(full_opp_id_file_path);

	FILE *t_file = fopen(full_t_file_path, "wb+");
	if (t_file == NULL) {
		perror("fopen (data_dir_reset_players)");
		return -1;
	}
	if (1 != fwrite(&num_t, sizeof(short), 1, t_file)) return -12;
	fclose(t_file);
	free(full_t_file_path);

	FILE *t_id_file = fopen(full_t_id_file_path, "wb+");
	if (t_id_file == NULL) {
		perror("fopen (data_dir_reset_players)");
		return -1;
	}
	if (1 != fwrite(&num_t, sizeof(short), 1, t_id_file)) return -12;
	fclose(t_id_file);
	free(full_t_id_file_path);

	return 0;
}


/** Takes a string representing a path to a data file (usually just the name
 * of the data file) and prepends the data directory file path to it
 * creating a full path to the data file.
 *
 * \return A string that is the file path to the data directory +
 *     the data file.
 */
char *data_dir_file_path_with_data_dir(const char *data_dir_file_path, \
	const char *data_file_path) {

	return extend_path(data_dir_file_path, data_file_path);
}


/** Takes no arguments, returns the full file path to the opp file
 *
 * \return A string that is the file path to opp file.
 */
char *data_dir_file_path_opp_file(const char *data_dir_file_path) {
	return data_dir_file_path_with_data_dir(data_dir_file_path, \
		OPP_FILE_NAME);
}


/** Takes no arguments, returns the full file path to the opp id file
 *
 * \return A string that is the file path to opp id file.
 */
char *data_dir_file_path_opp_id_file(const char *data_dir_file_path) {
	return data_dir_file_path_with_data_dir(data_dir_file_path, \
		OPP_ID_FILE_NAME);
}


/** Takes no arguments, returns the full file path to the tournament file
 *
 * \return A string that is the file path to tournament file.
 */
char *data_dir_file_path_t_file(const char *data_dir_file_path) {
	return data_dir_file_path_with_data_dir(data_dir_file_path, T_FILE_NAME);
}


/** Takes no arguments, returns the full file path to the tournament id file
 *
 * \return A string that is the file path to tournament id file.
 */
char *data_dir_file_path_t_id_file(const char *data_dir_file_path) {
	return data_dir_file_path_with_data_dir(data_dir_file_path, \
		T_ID_FILE_NAME);
}


/** Takes no arguments, returns the full file path to the season file
 *
 * \return A string that is the file path to season file.
 */
char *data_dir_file_path_season_file(const char *data_dir_file_path) {
	return data_dir_file_path_with_data_dir(data_dir_file_path, \
		SEASON_FILE_NAME);
}


/** Helper function for data_dir. Takes an open opp file and writes the
 * RD adjustment opponent id (always 0) and its name to the specified file.
 *
 * \param '*f' an open opp file in binary mode.
 * \return 0 upon success, a negative integer upon failure.
 */
int opp_file_open_write_rd_adj_name_with_id(FILE *f) {
	char zero = '\0';
	short szero = 0;

	if (1 != fwrite(&szero, sizeof(short), 1, f)) return -1;
	if (1 != fwrite("-", sizeof(char), 1, f)) return -2;

	for (unsigned long i = 0; i < MAX_NAME_LEN + 1 - strlen("-"); i++) {
		if (1 != fwrite(&zero, sizeof(char), 1, f)) return -3;
	}

	return 0;
}


/** Helper function for data_dir. Takes an open opp id file and writes the RD
 * adjustment opponent name to the specified file.
 *
 * \param '*f' an open opp id file in binary mode.
 * \return 0 upon success, a negative integer upon failure.
 */
int opp_id_file_open_write_rd_adj_name(FILE *f) {
	char zero = '\0';

	if (1 != fwrite("-", sizeof(char), 1, f)) return -1;

	for (unsigned long i = 0; i < MAX_NAME_LEN + 1 - strlen("-"); i++) {
		if (1 != fwrite(&zero, sizeof(char), 1, f)) return -2;
	}

	return 0;
}
