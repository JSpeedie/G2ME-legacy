#include <criterion/criterion.h>

#include "../glicko2.h"
#include "../entry_file.h"

struct player P;
struct entry E;

void suite_setup(void) {
	E.rating = 1500;
	E.RD = 350;
	E.vol = 0.06;
}

void suite_teardown(void) {
}

TestSuite(player_tests, .init=suite_setup, .fini=suite_teardown);

Test(player_tests, init_player_from_entry) {
	/* Test */
	/* init_player_from_entry(&P, &E); */

	/* Test and Check */
	cr_expect_float_eq(getRating(&P), 0.0, 0.1, \
		"Initializing a Player with an original-scale Entry rating of 1500" \
		"should give a Player a Glicko-2-scale rating of 0.0");
}
