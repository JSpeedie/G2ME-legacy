#include <criterion/criterion.h>

#include "../glicko2.h"


struct player P_ref;


void player_suite_setup(void) {
	P_ref.__rating = 0.0; // 1500
	P_ref.__rd = 2.0147618724; // 350
	P_ref.vol = 0.06;
}


void player_suite_teardown(void) {
}


TestSuite(player_tests, .init=player_suite_setup, .fini=player_suite_teardown);


Test(player_tests, setRatingGetRating) {
	/* Init */
	struct player P_test;
	setRating(&P_test, 1500.0);

	/* Test and Check */
	cr_expect_float_eq(getRating(&P_test), 1500.0, 0.0000001, \
		"Getting a rating for a Player should return the same " \
		"original-scale value it was set with");
}


Test(player_tests, setRdGetRd) {
	/* Init */
	struct player P_test;
	setRd(&P_test, 350.0);

	/* Test and Check */
	cr_expect_float_eq(getRd(&P_test), 350.0, 0.0000001, \
		"Getting an RD for a Player should return the same " \
		"original-scale value it was set with");
}
