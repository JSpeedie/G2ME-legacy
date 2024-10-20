#include <criterion/criterion.h>

#include "../glicko2.h"


struct player P_ref;
struct player P_test;


void player_suite_setup(void) {
	/* Manually initialize P_ref to (rating=1500, rd=350, vol=0.06) */
	P_ref.__rating = 0.0; // 1500
	P_ref.__rd = 2.0147618724; // 350
	P_ref.vol = 0.06;
	/* Initialize P_test to (rating=1500, rd=350, vol=0.06) using the
	 * appropriate functions */
	setRating(&P_test, 1500.0);
	setRd(&P_test, 350.0);
}


void player_suite_teardown(void) {
}


TestSuite(player_tests, .init=player_suite_setup, .fini=player_suite_teardown);


Test(player_tests, setRating) {
	/* Test and Check */
	cr_expect_float_eq(P_test.__rating, P_ref.__rating, 0.0000001, \
		"Setting a rating for a Player should correctly convert the given " \
		"original-scale value to a Glicko-2-scale value stored in the " \
		"struct.");
}


Test(player_tests, setRatingGetRating) {
	/* Test and Check */
	cr_expect_float_eq(getRating(&P_test), 1500.0, 0.0000001, \
		"Getting a rating for a Player should return the same " \
		"original-scale value it was set with.");
}


Test(player_tests, setRd) {
	/* Test and Check */
	cr_expect_float_eq(P_test.__rd, P_ref.__rd, 0.0000001, \
		"Setting an RD for a Player should correctly convert the given " \
		"original-scale value to a Glicko-2-scale value stored in the " \
		"struct.");
}


Test(player_tests, setRdGetRd) {
	/* Test and Check */
	cr_expect_float_eq(getRd(&P_test), 350.0, 0.0000001, \
		"Getting an RD for a Player should return the same " \
		"original-scale value it was set with.");
}
