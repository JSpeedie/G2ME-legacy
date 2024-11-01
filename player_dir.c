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
#include "player_dir.h"
#include "sorting.h" /* Includes G2ME.h -> glicko2.h, p_files.h */
#include "fileops.h"


#ifdef __linux__
const char *DEFAULT_PLAYER_DIR = ".players/";
#elif _WIN32
const char *DEFAULT_PLAYER_DIR = ".\\.players\\";
#else
const char *DEFAULT_PLAYER_DIR = ".players/";
#endif


/** Checks if the player directory exists. If it does not,
 * it creates one.
 *
 * \return negative int on failure, 0 upon success.
 */
int player_dir_check_and_create(const char *player_dir_file_path) {
	DIR *d = opendir(player_dir_file_path);
	/* If 'player_dir' DOES exist */
	if (d) closedir(d);
	else if (errno == ENOENT) {
		fprintf(stderr, \
			"G2ME: Warning: 'player_dir' did not exist, creating...\n");
		/* If there was an error making the player_directory */
#ifdef __linux__
		if (0 != mkdir(player_dir_file_path, 0700)) {
#elif _WIN32
		if (0 != mkdir(player_dir_file_path)) {
#else
		if (0 != mkdir(player_dir_file_path, 0700)) {
#endif
			perror("mkdir (player_dir_check_and_create)");
			return -1;
		}
	} else {
		perror("opendir (player_dir_check_and_create)");
		return -2;
	}

	return 0;
}


/* Deletes every player file in the player directory 'player_dir'.
 *
 * \return an int representing if the function succeeded or not.
 *     Negative if there was an error, 0 on success.
 */
int player_dir_reset_players(const char *player_dir_file_path) {
	DIR *p_dir;
	struct dirent *entry;
	if ((p_dir = opendir(player_dir_file_path)) != NULL) {
		while ((entry = readdir(p_dir)) != NULL) {
			// Make sure it doesn't count directories
			char *path_to_entry = \
				extend_path(player_dir_file_path, entry->d_name);
			/* If 'path_to_entry' points to a file that is not a directory */
			if (1 == is_dir(path_to_entry)) {
				char *full_player_path = \
					player_dir_file_path_with_player_dir( \
						player_dir_file_path, entry->d_name);
				remove(full_player_path);
				free(full_player_path);
			}
			free(path_to_entry);
		}
		closedir(p_dir);
	} else {
		perror("opendir (player_dir_reset_players)");
		return -1;
	}

	return 0;
}


/* Takes a pointer to an integer, modifies '*num_of_players' to the
 * number of players in the 'player_dir'. Really just counts the number
 * of non-directory "files" in the player directory.
 *
 * \param '*num_of_players' an int pointer that will be modified to contain
 *     the number of player names in the directory.
 * \return a pointer to the return array.
 */
void player_dir_num_players(const char *player_dir_file_path, \
	int *num_of_players) {

	DIR *p_dir;
	struct dirent *entry;

	if ((p_dir = opendir(player_dir_file_path)) != NULL) {
		*num_of_players = 0;
		while ((entry = readdir(p_dir)) != NULL) {
			/* Make sure it doesn't count directories */
			char *path_to_entry = \
				extend_path(player_dir_file_path, entry->d_name);
			if (1 == is_dir(path_to_entry)) {
				*num_of_players = (*num_of_players) + 1;
			}
			free(path_to_entry);
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
char *player_dir_players_list(const char *data_dir_file_path, \
	const char *player_dir_file_path, char *players, int *num, char type, \
	int minimum_events) {

	DIR *p_dir;
	struct dirent *entry;
	// TODO: reallocate '*players' if necessary make it required that
	// '*players' is a pointer made by a calloc or malloc call

	if ((p_dir = opendir(player_dir_file_path)) != NULL) {
		*num = 0;
		while ((entry = readdir(p_dir)) != NULL) {
			/* Make sure it doesn't count directories */
			char *path_to_entry = \
				extend_path(player_dir_file_path, entry->d_name);
			if (1 == is_dir(path_to_entry)) {
				int num_events;
				char *full_player_path = \
					player_dir_file_path_with_player_dir( \
						player_dir_file_path, entry->d_name);

				p_file_get_events_attended(data_dir_file_path, full_player_path, &num_events);
				/* If the player attended the minimum number of events */
				if (num_events >= minimum_events) {
					if (type == LEXIO) {
						int i = (MAX_NAME_LEN + 1) * (*(num) - 1);
						/* Find the right index to insert the name at */
						while (strcmp(&players[i], entry->d_name) > 0 \
							&& i >= 0) {

							/* Move later-occuring name further in the array */
							strncpy(&players[i + (MAX_NAME_LEN + 1)], &players[i], \
								MAX_NAME_LEN);
							i -= (MAX_NAME_LEN + 1);
						}
						strncpy(&players[i + (MAX_NAME_LEN + 1)], entry->d_name, \
							MAX_NAME_LEN);
					} else {
						strncpy(&players[(MAX_NAME_LEN + 1) * *(num)], \
							entry->d_name, MAX_NAME_LEN);
					}
					/* Add null terminator to each name */
					players[(MAX_NAME_LEN + 1) * (*(num) + 1)] = '\0';
					*num = *(num) + 1;
				}
				free(full_player_path);
			}
			free(path_to_entry);
		}
		closedir(p_dir);
	}

	return players;
}


/** Takes a string representing a path to a player file (usually just the name
 * of the player file) and prepends the player directory file path to it
 * creating a full path to the player file.
 *
 * \return A string that is the file path to the player directory +
 *     the player file.
 */
char *player_dir_file_path_with_player_dir(const char *player_dir_file_path, \
	const char *player_file_path) {

	return extend_path(player_dir_file_path, player_file_path);
}
