#define MAX_NAME_LEN 128
#define MAX_FILE_PATH_LEN 512
#define REALLOC_PR_ENTRIES_INC 4
#define SIZE_PR_ENTRY 128
#define LEXIO 1

typedef struct entry {
	unsigned short opp_id;
	unsigned short tournament_id;
	unsigned char len_name;
	unsigned char len_opp_name;
	char name[MAX_NAME_LEN];
	char opp_name[MAX_NAME_LEN];
	double rating;
	double RD;
	double vol;
	unsigned char gc;
	unsigned char opp_gc;
	unsigned char day;
	unsigned char month;
	unsigned short year;
	unsigned char len_t_name;
	char t_name[MAX_NAME_LEN];
}Entry;

typedef struct record {
	char name[MAX_NAME_LEN];
	char opp_name[MAX_NAME_LEN];
	unsigned char wins;
	unsigned char ties;
	unsigned char losses;
}Record;

char *file_path_with_player_dir(char *);
