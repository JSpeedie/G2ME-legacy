#ifndef G2ME_FILEOPS
#define G2ME_FILEOPS

#define MAX_FILE_PATH_LEN 512


#ifdef __linux__
static const char DIR_TERMINATOR = '/';
#elif _WIN32
static const char DIR_TERMINATOR = '\\';
#else
static const char DIR_TERMINATOR = '/';
#endif


int is_dir(const char *);
int make_dir(const char *);
void clear_file(char *);
char *extend_path(const char *, const char *);
char *extend_path2(const char *, const char *, const char *);
int copy_file(const char *, const char *);

#endif
