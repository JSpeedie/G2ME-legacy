#ifndef G2ME_ENTRY
#define G2ME_ENTRY

/* Local Includes */
#include "glicko2.h"

#define MAX_NAME_LEN 64

typedef struct entry {
	unsigned short id;
	unsigned short opp_id;
	unsigned short tournament_id;
	unsigned short season_id;
	unsigned char len_name;
	unsigned char len_opp_name;
	char name[MAX_NAME_LEN];
	char opp_name[MAX_NAME_LEN];
	double rating;
	double RD;
	double vol;
	signed char gc;
	signed char opp_gc;
	unsigned char is_competitor;
	unsigned char day;
	unsigned char month;
	unsigned short year;
	unsigned char len_t_name;
	char t_name[MAX_NAME_LEN];
}Entry;


void init_player_from_entry(Player * P, Entry * E);

struct entry create_entry(struct player* P, char* name, char* opp_name, \
	char gc, char opp_gc, char day, char month, short year, \
	char* t_name, short season_id, char is_comp);

#endif
