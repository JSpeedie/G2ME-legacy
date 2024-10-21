#include <criterion/criterion.h>

#include "../glicko2.h"
#include "../entry.h"


Player P;
Entry E_from_P;
Player P2_from_E;


void entry_suite_setup(void) {
	/* Initialize 'P' to (rating=1500, rd=350, vol=0.06) using the
	 * appropriate functions */
	setRating(&P, 1500.0);
	setRd(&P, 350.0);
	P.vol = 0.06;

	E_from_P = create_entry(&P, "player_name", "opponent_name", \
		/* Game count */
		3, 1, \
		/* Date (dd-mm-yyyy) */
		1, 1, 2024, \
		/* Tournament name, season id */
		"first_tournament", 0, \
		/* Is competitor */
		1);
	
	init_player_from_entry(&P2_from_E, &E_from_P);
}


void entry_suite_teardown(void) {
}


TestSuite(entry_tests, .init=entry_suite_setup, .fini=entry_suite_teardown);


Test(entry_tests, createEntry) {
	/* Test and Check */
	cr_expect_eq(E_from_P.season_id, 0, \
		"Setting Entry.season_id using create_entry() should work.");
	cr_expect_eq(E_from_P.len_name, strlen(E_from_P.name), \
		"Entry.len_name should be equal to strlen() of Entry.name.");
	cr_expect_eq(E_from_P.len_opp_name, strlen(E_from_P.opp_name), \
		"Entry.len_opp_name should be equal to strlen() of Entry.opp_name.");
	cr_expect_str_eq(E_from_P.name, "player_name", \
		"Setting Entry.name using create_entry() should work.");
	cr_expect_str_eq(E_from_P.opp_name, "opponent_name", \
		"Setting Entry.opp_name using create_entry() should work.");
	cr_expect_float_eq(E_from_P.rating, getRating(&P), 0.00001, \
		"Setting Entry.rating using create_entry() should work.");
	cr_expect_float_eq(E_from_P.RD, getRd(&P), 0.00001, \
		"Setting Entry.RD using create_entry() should work.");
	cr_expect_float_eq(E_from_P.vol, P.vol, 0.00001, \
		"Setting Entry.vol using create_entry() should work.");
	cr_expect_eq(E_from_P.gc, 3, \
		"Entry.gc using create_entry() should work.");
	cr_expect_eq(E_from_P.opp_gc, 1, \
		"Entry.opp_gc using create_entry() should work.");
	cr_expect_eq(E_from_P.day, 1, \
		"Entry.day using create_entry() should work.");
	cr_expect_eq(E_from_P.month, 1, \
		"Entry.month using create_entry() should work.");
	cr_expect_eq(E_from_P.year, 2024, \
		"Entry.year using create_entry() should work.");
	cr_expect_eq(E_from_P.len_t_name, strlen(E_from_P.t_name), \
		"Entry.len_t_name should be equal to strlen() of Entry.t_name.");
	cr_expect_str_eq(E_from_P.t_name, "first_tournament", \
		"Setting Entry.t_name using create_entry() should work.");
	cr_expect_eq(E_from_P.is_competitor, 1, \
		"Entry.month using create_entry() should work.");
}


Test(entry_tests, initPlayerFromEntry) {
	/* Test and Check */
	cr_expect_float_eq(P2_from_E.__rating, P.__rating, 0.00001, \
		"Player initialized from Entry has the wrong rating");
	cr_expect_float_eq(P2_from_E.__rd, P.__rd, 0.00001, \
		"Player initialized from Entry has the wrong RD");
	cr_expect_float_eq(P2_from_E.vol, P.vol, 0.00001, \
		"Player initialized from Entry has the wrong volatility");
}
