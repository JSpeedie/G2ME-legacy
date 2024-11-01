#ifndef G2ME_DATA_DIR
#define G2ME_DATA_DIR


#include <stdio.h>


extern const char *DEFAULT_DATA_DIR;
static const char OPP_FILE_NAME[] = { "of" };
static const char OPP_ID_FILE_NAME[] = { "oif" };
static const char T_FILE_NAME[] = { "tf" };
static const char T_ID_FILE_NAME[] = { "tif" };
static const char SEASON_FILE_NAME[] = { "sf" };


/* Data dir functions */
int data_dir_check_and_create(const char *);
int data_dir_reset(const char *);
char *data_dir_file_path_with_data_dir(const char *, const char *);
char *data_dir_file_path_opp_file(const char *);
char *data_dir_file_path_opp_id_file(const char *);
char *data_dir_file_path_t_file(const char *);
char *data_dir_file_path_t_id_file(const char *);
char *data_dir_file_path_season_file(const char *);

/* Opp (id) file functions */
int opp_file_open_write_rd_adj_name_with_id(FILE *);
int opp_id_file_open_write_rd_adj_name(FILE *);

#endif
