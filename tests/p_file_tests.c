#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <errno.h>

#include "../entry.h"
#include "../fileops.h"
#include "../p_files.h"
#include "../player_dir.h"
#include "testing.h"


const char *SUITE_EXT = "p_file_tests";


void p_file_suite_setup(void) {
}


void p_file_suite_teardown(void) {
}


TestSuite(p_file_tests, .init=p_file_suite_setup, .fini=p_file_suite_teardown);


Test(p_file_tests, initializePlayerFile) {
	/* {{{ */
	/* Setup */
	Entry E_partial;
	const char *TEST_EXT = "initializePlayerFile";
	char *suite_test_ext = extend_path(SUITE_EXT, TEST_EXT);

	/* Construct two paths:
	 * DATA_DIR_EXPECTED_ROOT / SUITE_EXT / TEST_EXT / DEFAULT_PLAYER_DIR
	 * (e.g. "tests_expected/p_file_tests/initializePlayerFile/.players" )
	 * DATA_DIR_ACTUAL_ROOT / SUITE_EXT / TEST_EXT / DEFAULT_PLAYER_DIR
	 * (e.g. "tests_actual/p_file_tests/initializePlayerFile/.players" )
	 */
	const char *player_dir_expected = \
		extend_path2(DATA_DIR_EXPECTED_ROOT, suite_test_ext, DEFAULT_PLAYER_DIR);
	const char *player_dir_actual = \
		extend_path2(DATA_DIR_ACTUAL_ROOT, suite_test_ext, DEFAULT_PLAYER_DIR);
	free(suite_test_ext);

	int ret;
	if (0 != (ret = make_dir(player_dir_actual))) {
		cr_expect_eq(ret, 0, \
			"%s: make_dir(\"%s\"): " \
			"Failed to create the directory.\n", \
			TEST_EXT, player_dir_actual);
		return;
	}

	const char *testname = "PlayerTestName";
	E_partial.len_name = strlen(testname);
	strncpy(E_partial.name, testname, sizeof(E_partial.name));

	const char *actual_p_fp = \
		extend_path(player_dir_actual, testname);
	const char *expected_p_fp = \
		extend_path(player_dir_expected, testname);

	/* Test */
	p_file_initialize(&E_partial, actual_p_fp);

	/* Check */
	FILE *actual = fopen(actual_p_fp, "rb");
	if (actual == NULL) {
		cr_expect_neq(actual, NULL, \
			"%s: fopen(\"%s\"): %s\n", \
			TEST_EXT, actual_p_fp, strerror(errno));
		return;
	}

	FILE *expected = fopen(expected_p_fp, "rb");
	if (expected == NULL) {
		cr_expect_neq(expected, NULL, \
			"%s: fopen(\"%s\"): %s\n", \
			TEST_EXT, expected_p_fp, strerror(errno));
		return;
	}

	cr_expect_file_contents_eq(actual, expected, \
		"Initializing a player file should work.");
	/* }}} */
}


Test(p_file_tests, appendEntryToPlayerFile) {
	/* {{{ */
	/* Setup */
	Entry E;
	Player P;
	const char *TEST_EXT = "appendEntryToPlayerFile";
	char *suite_test_ext = extend_path(SUITE_EXT, TEST_EXT);

	/* Construct two paths:
	 * DATA_DIR_EXPECTED_ROOT / SUITE_EXT / TEST_EXT / DEFAULT_PLAYER_DIR
	 * (e.g. "tests_expected/p_file_tests/initializePlayerFile/.players" )
	 * DATA_DIR_ACTUAL_ROOT / SUITE_EXT / TEST_EXT / DEFAULT_PLAYER_DIR
	 * (e.g. "tests_actual/p_file_tests/initializePlayerFile/.players" )
	 */
	const char *player_dir_expected = \
		extend_path2(DATA_DIR_EXPECTED_ROOT, suite_test_ext, DEFAULT_PLAYER_DIR);
	const char *player_dir_input = \
		extend_path2(DATA_DIR_INPUT_ROOT, suite_test_ext, DEFAULT_PLAYER_DIR);
	const char *player_dir_actual = \
		extend_path2(DATA_DIR_ACTUAL_ROOT, suite_test_ext, DEFAULT_PLAYER_DIR);
	free(suite_test_ext);

	int ret;
	if (0 != (ret = make_dir(player_dir_actual))) {
		cr_expect_eq(ret, 0, \
			"%s: make_dir(\"%s\"): " \
			"Failed to create the directory.\n", \
			TEST_EXT, player_dir_actual);
		return;
	}

	char *testname = "PlayerFreshlyInitialized";

	const char *actual_p_fp = \
		extend_path(player_dir_actual, testname);
	const char *input_p_fp = \
		extend_path(player_dir_input, testname);
	const char *expected_p_fp = \
		extend_path(player_dir_expected, testname);

	/* Copy the input file into the actual dir since our test work will modify
	 * the file and we don't want to modify the input file */
	if (0 != (ret = copy_file(input_p_fp, actual_p_fp))) {
		cr_expect_eq(ret, 0, \
			"%s: copy_file(\"%s\", \"%s\"): " \
			"Failed to copy the input file to the actual directory.\n", \
			TEST_EXT, input_p_fp, actual_p_fp);
		return;
	}

	/* Initialize 'P' to (rating=1383.3581, rd=286.9272, vol=0.05999919) using
	 * the appropriate functions */
	setRating(&P, 1383.3581);
	setRd(&P, 286.9272);
	P.vol = 0.05999919;
	/* Populate 'E' with valid data before appending it */
	E = create_entry(&P, testname, "MockOpponent", \
		/* Game count */
		0, 3, \
		/* Date (dd-mm-yyyy) */
		28, 3, 2024, \
		/* Tournament name, season id */
		"tournament0", 0, \
		/* Is competitor */
		1);
	E.opp_id = 0;
	E.tournament_id = 0;

	/* Test */
	ret = p_file_append_entry_to_file_id(&E, actual_p_fp);

	/* Check */
	FILE *actual = fopen(actual_p_fp, "rb");
	if (actual == NULL) {
		cr_expect_neq(actual, NULL, \
			"%s: fopen(\"%s\"): %s\n", \
			TEST_EXT, actual_p_fp, strerror(errno));
		return;
	}

	FILE *expected = fopen(expected_p_fp, "rb");
	if (expected == NULL) {
		cr_expect_neq(expected, NULL, \
			"%s: fopen(\"%s\"): %s\n", \
			TEST_EXT, expected_p_fp, strerror(errno));
		return;
	}

	cr_expect_file_contents_eq(actual, expected, \
		"Appending this entry to this player file should work.");
	/* }}} */
}


