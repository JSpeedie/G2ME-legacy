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

#include "sorting.h" /* Includes G2ME.h -> glicko2.h, entry_file.h */
#include "fileops.h"

char OPP_FILE_NAME[] = { "of" };
char OPP_ID_FILE_NAME[] = { "oif" };

/** Takes a string and prepends the player directory file path to it.
 *
 * \return A string that is the file path to the player directory +
 *     the given string.
 */
char *player_dir_file_path_with_player_dir(char *s) {
	int new_path_size = sizeof(char) * (MAX_FILE_PATH_LEN - MAX_NAME_LEN);
	char *new_path = (char *) malloc(new_path_size);

	/* Copy the player directory file path into the return string */
	strncpy(new_path, player_dir, new_path_size - 1);
	size_t len_new_path = strlen(new_path);
	/* If the last character in the player directory path is not a '/' */
	if (new_path[len_new_path - 1] != DIR_TERMINATOR) {
		/* Append a '/' or '\' */
		new_path[len_new_path] = DIR_TERMINATOR;
	}

	/* Append the player file to the player dir file path */
	strncat(new_path, s, new_path_size - len_new_path - 1);

	return new_path;
}

/** Takes a string and prepends the data directory file path to it.
 *
 * \return A string that is the file path to the data directory +
 *     the given string.
 */
char *data_dir_file_path_with_data_dir(char *s) {
	int new_path_size = sizeof(char) * (MAX_FILE_PATH_LEN - MAX_NAME_LEN);
	char *new_path = (char *) malloc(new_path_size);

	/* Copy the data directory file path into the return string */
	strncpy(new_path, data_dir, new_path_size - 1);
	size_t len_new_path = strlen(new_path);
	/* If the last character in the data directory path is not a '/' */
	if (new_path[len_new_path - 1] != DIR_TERMINATOR) {
		/* Append a '/' or '\' */
		new_path[len_new_path] = DIR_TERMINATOR;
	}

	/* Append the data file to the data dir file path */
	strncat(new_path, s, new_path_size - len_new_path - 1);

	return new_path;
}

/** Takes no arguments, returns the full file path to the opp file
 *
 * \return A string that is the file path to opp file.
 */
char *data_dir_file_path_opp_file(void) {
	return data_dir_file_path_with_data_dir(OPP_FILE_NAME);
}

/** Takes no arguments, returns the full file path to the opp id file
 *
 * \return A string that is the file path to opp id file.
 */
char *data_dir_file_path_opp_id_file(void) {
	return data_dir_file_path_with_data_dir(OPP_ID_FILE_NAME);
}

/* Deletes every player file in the player directory 'player_dir'.
 *
 * \return an int representing if the function succeeded or not.
 *     Negative if there was an error, 0 on success.
 */
int player_dir_reset_players(void) {
	DIR *p_dir;
	struct dirent *entry;
	if ((p_dir = opendir(player_dir)) != NULL) {
		while ((entry = readdir(p_dir)) != NULL) {
			// Make sure it doesn't count directories
			if (1 == check_if_dir(player_dir, entry->d_name)) {
				char *full_player_path = \
					player_dir_file_path_with_player_dir(entry->d_name);
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

	/* Create universal G2ME files, necessary for usage */
	short num_opp = 0;
	char *full_opp_file_path = data_dir_file_path_opp_file();
	FILE *opp_file = fopen(full_opp_file_path, "wb");
	if (opp_file == NULL) {
		perror("fopen (player_dir_check_and_create)");
		return -1;
	}
	if (1 != fwrite(&num_opp, sizeof(short), 1, opp_file)) return -12;
	free(full_opp_file_path);
	fclose(opp_file);
	
	char *full_opp_id_file_path = data_dir_file_path_opp_id_file();
	FILE *opp_id_file = fopen(full_opp_id_file_path, "wb");
	if (opp_id_file == NULL) {
		perror("fopen (player_dir_check_and_create)");
		return -1;
	}
	if (1 != fwrite(&num_opp, sizeof(short), 1, opp_id_file)) return -12;
	fclose(opp_id_file);
	free(full_opp_id_file_path);

	return 0;
}

int data_dir_check_and_create(void) {
	DIR *d = opendir(data_dir);
	/* If 'player_dir' DOES exist */
	if (d) closedir(d);
	else if (errno == ENOENT) {
		fprintf(stderr, \
			"G2ME: Warning: 'data_dir' did not exist, creating...\n");
		/* If there was an error making the data directory */
#ifdef __linux__
		if (0 != mkdir(data_dir, 0700)) {
#elif _WIN32
		if (0 != mkdir(data_dir)) {
#else
		if (0 != mkdir(data_dir, 0700)) {
#endif
			perror("mkdir (main)");
			return -1;
		}
	} else {
		perror("opendir (main)");
		return -2;
	}

	/* Get file paths for universal files */
	char *full_opp_file_path = data_dir_file_path_opp_file();
	char *full_opp_id_file_path = data_dir_file_path_opp_id_file();
#ifdef __linux__
	char of_exist = access(full_opp_file_path, R_OK) != -1;
	char oif_exist = access(full_opp_id_file_path, R_OK) != -1;
#elif _WIN32
	char of_exist = _access(full_opp_file_path, 0) != -1;
	char oif_exist = _access(full_opp_id_file_path, 0) != -1;
#else
	char of_exist = access(full_opp_file_path, R_OK) != -1;
	char oif_exist = access(full_opp_id_file_path, R_OK) != -1;
#endif
	short num_opp = 0;
	/* If of doesn't exist, create it */
	if (!of_exist) {
		FILE *opp_file = fopen(full_opp_file_path, "wb");
		if (opp_file == NULL) {
			perror("fopen (player_dir_check_and_create)");
			return -1;
		}
		if (1 != fwrite(&num_opp, sizeof(short), 1, opp_file)) return -12;
		free(full_opp_file_path);
		fclose(opp_file);
	}
	
	/* If oif doesn't exist, create it */
	if (!oif_exist) {
		FILE *opp_id_file = fopen(full_opp_id_file_path, "wb");
		if (opp_id_file == NULL) {
			perror("fopen (player_dir_check_and_create)");
			return -1;
		}
		if (1 != fwrite(&num_opp, sizeof(short), 1, opp_id_file)) return -12;
		fclose(opp_id_file);
		free(full_opp_id_file_path);
	}

	return 0;
}

/** Checks if the player directory exists. If it does not,
 * it creates one.
 *
 * \return negative int on failure, 0 upon success.
 */
int player_dir_check_and_create(void) {
	DIR *d = opendir(player_dir);
	/* If 'player_dir' DOES exist */
	if (d) closedir(d);
	else if (errno == ENOENT) {
		fprintf(stderr, \
			"G2ME: Warning: 'player_dir' did not exist, creating...\n");
		/* If there was an error making the player_directory */
#ifdef __linux__
		if (0 != mkdir(player_dir, 0700)) {
#elif _WIN32
		if (0 != mkdir(player_dir)) {
#else
		if (0 != mkdir(player_dir, 0700)) {
#endif
			perror("mkdir (main)");
			return -1;
		}
	} else {
		perror("opendir (main)");
		return -2;
	}

	return data_dir_check_and_create();
}


/* Takes a pointer to an integer, modifies '*num_of_players' to the
 * number of players in the 'player_dir'. Really just counts the number
 * of non-directory "files" in the player directory.
 *
 * \param '*num_of_players' an int pointer that will be modified to contain
 *     the number of player names in the directory.
 * \return a pointer to the return array.
 */
void player_dir_num_players(int *num_of_players) {
	DIR *p_dir;
	struct dirent *entry;

	if ((p_dir = opendir(player_dir)) != NULL) {
		*num_of_players = 0;
		while ((entry = readdir(p_dir)) != NULL) {
			// Make sure it doesn't count directories
			if (1 == check_if_dir(player_dir, entry->d_name)) {
				*num_of_players = (*num_of_players) + 1;
			}
		}
	}
	closedir(p_dir);
}

/* Takes a char pointer created by malloc or calloc and a pointer
 * to an integer. Gets the name of every player in the player directory
 * 'player_dir' into the array in lexiographical order. Modifies '*num'
 * to point to the number of elements in the array upon completion.
 *
 * \param '*players' a char pointer created by calloc or malloc which
 *     will be modified to contain all the player names,
 *     'MAX_NAME_LEN' apart and in lexiographical order.
 * \param '*num' a int pointer that will be modified to contain
 *     the number of player names in the array when this function
 *     has completed.
 * \param 'type' a char representing the type of sort the returned
 *     array should be of. Options are 'LEXIO' for a lexiograpically
 *     sorted array. Any value that is not 'LEXIO' will make
 *     this function return an orderless array.
 * \return a pointer to the return array.
 */
// TODO: change to check syscalls and to return int
char *player_dir_players_list(char *players, int *num, char type) {
	DIR *p_dir;
	struct dirent *entry;
	// TODO: reallocate '*players' if necessary make it required that
	// '*players' is a pointer made by a calloc or malloc call

	if ((p_dir = opendir(player_dir)) != NULL) {
		*num = 0;
		while ((entry = readdir(p_dir)) != NULL) {
			// Make sure it doesn't count directories
			if (1 == check_if_dir(player_dir, entry->d_name)) {
				int num_events;
				char *full_player_path = \
					player_dir_file_path_with_player_dir(entry->d_name);
				entry_file_get_events_attended(full_player_path, &num_events);
				// If the player attended the minimum number of events
				if (num_events >= pr_minimum_events) {
					if (type == LEXIO) {
						int i = MAX_NAME_LEN * (*(num) - 1);
						// Find the right index to insert the name at
						while (strcmp(&players[i], entry->d_name) > 0 \
							&& i >= 0) {

							// Move later-occuring name further in the array
							strncpy(&players[i + MAX_NAME_LEN], &players[i], \
								MAX_NAME_LEN);
							i -= MAX_NAME_LEN;
						}
						strncpy(&players[i + MAX_NAME_LEN], entry->d_name, \
							MAX_NAME_LEN);
					} else {
						strncpy(&players[MAX_NAME_LEN * *(num)], \
							entry->d_name, MAX_NAME_LEN);
					}
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
