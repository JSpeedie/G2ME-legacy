/* Non-windows includes */
#include <dirent.h>
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
#include "fileops.h"


/** Returns an int represent whether the file path is to
 * a directory or not.

 * \param '*dir_path' the file path of the parent directory
 *     of the file to be checked.
 * \param '*file_name' the file name of the file to be checked.
 * \return negative int if there was an error, 0 if the full file
 *     path leads to a directory, 1 if it does not.
 */
int check_if_dir(char *dir_path, char *file_name) {
	char original_dir[MAX_FILE_PATH_LEN + 1];
	int ret = 1;
	if (NULL == getcwd(original_dir, MAX_FILE_PATH_LEN)) {
		perror("getcwd (check_if_dir)");
		return -1;
	}
	if (0 != chdir(dir_path)) {
		perror("chdir (check_if_dir)");
		return -2;
	}
	char fixed_dir[MAX_FILE_PATH_LEN + 1];
	if (NULL == getcwd(fixed_dir, MAX_FILE_PATH_LEN)) {
		perror("getcwd (check_if_dir)");
		ret = -3;
	}

	struct stat file_info;
	if(stat(file_name, &file_info) < 0) {
		perror("stat (check_if_dir)");
		ret = -4;
	}
	if (0 != chdir(original_dir)) {
		perror("chdir (check_if_dir)");
		ret = -5;
	}
	if (S_ISDIR(file_info.st_mode)) {
		return 0;
	}
	return ret;
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
