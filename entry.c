/* General includes */
#include <string.h>
#include <unistd.h>
/* Windows includes */
#ifdef _WIN32
#include <windows.h>
#endif
/* Local Includes */
#include "entry.h"
#include "glicko2.h"


/** Initializes a struct player based off of the information found in a
 * struct entry.
 *
 * \param '*P' the struct player to be initialized
 * \param '*E' the struct entry from which to get the data to initialize the
 *     struct player
 * \return void
 */
void init_player_from_entry(Player * P, Entry * E) {
	setRating(P, E->rating);
	setRd(P, E->RD);
	P->vol = E->vol;
	return;
}


/** Creates a struct entry which contains all the important data about a set
 *
 * \param '*P' a struct player that represents player 1
 * \param '*name' the name of player 1
 * \param '*opp_name' the name of player 2
 * \param 'gc' the number of games won by player 1 in the set
 * \param 'opp_gc' the number of games won by player 2 in the set
 * \param 'day' a char representing the day of the month the set was played on
 * \param 'month' a char representing the month the set was played in
 * \param 'year' a short representing the year the set was played in
 * \param 't_name' a string containing the name of the tournament this
 *     outcome took place at.
 * \param 'season_id' a short representing the id of the season this entry
 *     took place during.
 * \param 'is_comp' a char representing if this is a valid competitive entry
 *     as opposed to an RD adjustment.
 * \return a struct entry containing all that information
 */
struct entry create_entry(struct player* P, char* name, char* opp_name, \
	char gc, char opp_gc, char day, char month, short year, \
	char* t_name, short season_id, char is_comp) {

	struct entry ret;
	ret.len_name = strlen(name);
	ret.len_opp_name = strlen(opp_name);
	strncpy(ret.name, name, sizeof(ret.name));
	strncpy(ret.opp_name, opp_name, sizeof(ret.opp_name));
	ret.rating = getRating(P);
	ret.RD = getRd(P);
	ret.vol = P->vol;
	ret.gc = gc;
	ret.opp_gc = opp_gc;
	ret.day = day;
	ret.month = month;
	ret.year = year;
	ret.len_t_name = strlen(t_name);
	ret.is_competitor = is_comp;
	strncpy(ret.t_name, t_name, sizeof(ret.t_name));
	ret.season_id = season_id;

	return ret;
}
