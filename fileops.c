/* Non-windows includes */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
/* Windows includes */
#ifdef _WIN32
#include <windows.h>
#endif
/* Local Includes */
#include "fileops.h"


/** Returns an int representing whether the given file path points to a
 * directory or not.

 * \param '*path' the file path to the file to be checked.
 * \return negative int if there was an error, 0 if the file path leads to a
 *     directory, 1 if it does not.
 */
int is_dir(const char *path) {
	struct stat file_info;
	if (stat(path, &file_info) < 0) {
		/* stat() failed, return -1 */
		return -1;
	}

	if (S_ISDIR(file_info.st_mode)) {
		return 0;
	} else {
		return 1;
	}
}


/* Takes a path a directory and attempts to create all necessary parent
 * directories before attempting to create the final directory pointed to by
 * '*dir_path'.
 *
 * \param '*dir_path' the file path to the directory of interest.
 * \return a negative int if a non-directory already exists at that path, or
 *     if there was an error, and 0 if a directory already exists at that path
 *     or if the directory and all previously-non-existent parent directories
 *     were created successfully.
 */
int make_dir(const char *dir_path) {
	int ret = is_dir(dir_path);
	/* If the path already points to an existing directory */
	if (0 == ret) {
		return 0;
	/* If the path already points to an existing file which is NOT a
	 * directory */
	} else if (1 == ret) {
		return -1;
	}

	/* If we make it here, it means '*dir_path' points to nothing, and we
	 * need to create the directory, and possibly its parent directories */

	/* Get the path to the immediate parent of '*dir_path' */
	char *tmp = malloc(strlen(dir_path) + 1);
	if (tmp == NULL) {
		return -1;
	}
	strncpy(tmp, dir_path, strlen(dir_path));
	char *dir_path_parent = dirname(tmp);

	int parent_exists = (0 == access(dir_path_parent, F_OK));
	int parent_is_dir = (0 == is_dir(dir_path_parent));
	int can_write_to_parent = (0 == access(dir_path_parent, W_OK));

	/* If '*dir_path''s next parent directory does NOT exist, then recurse
	 * further */
	if (!parent_exists) {
		/* Attempt to make the next parent directory, and all parent
		 * directories it depends on if they do not yet exist. If it
		 * succeeds... */
		if (0 == make_dir(dir_path_parent)) {
			/* Refresh the 'can_write_to_parent' var since we just created the
			 * parent dir */
			can_write_to_parent = (0 == access(dir_path_parent, W_OK));
			if (can_write_to_parent) {
				/* Then we can now create the directory this function was given */
				if (0 == mkdir(dir_path, 0755)) {
					free(tmp);
					return 0;
				} else {
					free(tmp);
					return -1;
				}
			} else {
				free(tmp);
				return -1;
			}
		} else {
			free(tmp);
			return -1;
		}
	/* If '*dir_path''s next parent directory does exist ... */
	} else {
		if (parent_is_dir) {
			if (can_write_to_parent) {
				/* Then we can now create the directory this function was
				 * given */
				if (0 == mkdir(dir_path, 0755)) {
					free(tmp);
					return 0;
				} else {
					free(tmp);
					return -1;
				}
			} else {
				free(tmp);
				return -1;
			}
		/* '*dir_path' was not a valid path - it contained a path to a
		 * non-directory immediately followed by a directory symbol (like '/')
		 * and then more path */
		} else {
			free(tmp);
			return -1;
		}
	}
}


/** Writes nothing to a file, clearing it.
 *
 * \param '*file' the file path for the file to be wiped
 * \return void
 */
void clear_file(char* file) {

#ifdef __linux__
	FILE* victim = fopen(file, "w");
	if (victim == NULL) {
		perror("fopen (clear_file)");
		return;
	}
	fclose(victim);
#elif _WIN32
	/* Open the file, with read and write, Share for reading,
	* no security, open regardless, normal file with no attributes */
	HANDLE victim = CreateFile(file, GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
		NULL);
	long unsigned ret;
	WriteFile(victim, "", sizeof(""), &ret, NULL);
	CloseHandle(victim);
#else
	FILE* victim = fopen(file, "w");
	if (victim == NULL) {
		perror("fopen (clear_file)");
		return;
	}
	fclose(victim);
#endif

	return;
}


/** Takes a path to a root directory and a relative path and extends the root
 * path by adding the relative path to it.
 *
 * \return A malloc'd string that is the file path to the root directory
 *     '*root' + the given relative path '*ext'. Since the return value is
 *     malloc'd, it will need to be freed manually by the caller of this
 *     function.
 */
char *extend_path(const char *root, const char *ext) {
	int path_size = \
		sizeof(char) * (strlen(root) + 1 + strlen(ext) + 1);
	char *path = (char *) malloc(path_size);

	/* Copy the root directory file path into the return string */
	strncpy(path, root, strlen(root) + 1);
	size_t len_path = strlen(path);
	/* If the last character in the root directory path is a directory
	 * terminator character like '/' or '\' */
	if (path[len_path - 1] != DIR_TERMINATOR) {
		/* Append a directory terminator character like '/' or '\' */
		path[len_path] = DIR_TERMINATOR;
		path[len_path + 1] = '\0';
	}
	strncat(path, ext, strlen(ext) + 1);

	return path;
}


/** Takes a path to a root directory and two relative paths and extends the
 * root path by adding/appending both relative paths to it starting with
 * '*ext1' and finishing with '*ext2'.
 *
 * \return A malloc'd string that is the file path to the root directory
 *     '*root' + the given relative path '*ext1' + the given relative path
 *     '*ext2'. Since the return value is malloc'd, it will need to be freed
 *     manually by the caller of this function.
 */
char *extend_path2(const char *root, const char *ext1, const char *ext2) {
	int path_size = \
		sizeof(char) * (strlen(root) + 1 + strlen(ext1) + 1 + strlen(ext2) + 1);
	char *path = (char *) malloc(path_size);

	/* Copy the root directory file path into the return string */
	strncpy(path, root, strlen(root) + 1);

	size_t len_path = strlen(path);
	/* If the last character in the root directory path is a directory
	 * terminator character like '/' or '\' */
	if (path[len_path - 1] != DIR_TERMINATOR) {
		/* Append a directory terminator character like '/' or '\' */
		path[len_path] = DIR_TERMINATOR;
		path[len_path + 1] = '\0';
	}
	strncat(path, ext1, strlen(ext1) + 1);

	len_path = strlen(path);
	/* If the last character in the path is a directory
	 * terminator character like '/' or '\' */
	if (path[len_path - 1] != DIR_TERMINATOR) {
		/* Append a directory terminator character like '/' or '\' */
		path[len_path] = DIR_TERMINATOR;
		path[len_path + 1] = '\0';
	}
	strncat(path, ext2, strlen(ext2) + 1);

	return path;
}
