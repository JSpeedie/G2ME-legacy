#include <stdio.h>
#include <criterion/criterion.h>
/* #include <criterion/redirect.h> */

#include "../player_dir.h"


void o_flag_suite_setup(void) {
	printf("doing setup\n");
	/* player_dir_reset_players(); */
	/* hashtable_reset(); */
	/* run_brackets("tests/test_season.sea"); */
	printf("finished setup\n");
}


void o_flag_suite_teardown(void) {
}


TestSuite(o_flag_tests, .init=o_flag_suite_setup, .fini=o_flag_suite_teardown);


Test(o_flag_tests, o_flag) {
	/* Test */
	/* cr_redirect_stdout(); */
	// TODO: call function
	int ret = 1;
	/* int ret = generate_ratings_file_full("does not matter"); */
	cr_expect_eq(0, ret, "Generating a rating file should not have failed in" \
		"this situation.");
	fflush(stdout);
	printf("finished test\n");

	/* Check */
	/* cr_expect_stdout_eq_str( \ */
	/* 	"   Ayman  2078.7  133.4  0.05999612" \ */
	/* 	" Valerie  1933.9  139.3  0.05999746" \ */
	/* 	"     Mai  1854.4  128.1  0.05999373" \ */
	/* 	"  Cedric  1783.5  125.6  0.05999299" \ */
	/* 	"Victoria  1718.3  133.9  0.05999394" \ */
	/* 	"  Odonna  1702.3  172.6  0.05999835" \ */
	/* 	" Madison  1581.1  146.7  0.05999305" \ */
	/* 	" Darnell  1558.9  153.0  0.05999432" \ */
	/* 	"  Pranav  1523.8  191.4  0.05999747" \ */
	/* 	"   Aiden  1503.5  197.4  0.05999661" \ */
	/* 	" Hussein  1492.1  196.9  0.05999684" \ */
	/* 	" Richard  1417.6  217.8  0.06000064" \ */
	/* 	"   Kevin  1408.0  215.2  0.05999858" \ */
	/* 	"   Mateo  1407.8  165.1  0.05999568" \ */
	/* 	"   James  1287.6  192.4  0.05999715" \ */
	/* 	"   Maria  1287.4  243.8  0.05999833" \ */
	/* 	" Siobhan  1283.7  243.2  0.05999837" \ */
	/* 	"  Jaeyun  1247.6  247.4  0.05999906" \ */
	/* 	"  Jayden m1149.9  202.1  0.05999757" \ */
	/* 	, \ */
	/* 	"The output from generating a ratings file did not look the way it " \ */
	/* 	"was expected to."); */
	printf("finished check\n");
}
