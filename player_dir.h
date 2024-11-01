#ifndef G2ME_PLAYER_DIR
#define G2ME_PLAYER_DIR


extern const char *DEFAULT_PLAYER_DIR;


/* Player Dir functions */
int player_dir_check_and_create(const char *);
int player_dir_reset_players(const char *);
void player_dir_num_players(const char *, int *);
char *player_dir_players_list(const char *, const char *, char *, int *, \
	char, int);
char *player_dir_file_path_with_player_dir(const char *, const char *);

#endif
