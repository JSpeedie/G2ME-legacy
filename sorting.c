/* Non-windows includes */
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
/* Windows includes */
#ifdef _WIN32
#include <windows.h>
#endif

#include "glicko2.h"
#include "G2ME.h"

void merge(struct entry *first_array, int first_length, \
	struct entry *second_array, int second_length, struct entry *output_array) {

	int first_index = 0;
	int second_index = 0;
	int final_index = 0;

	while (first_index < first_length && second_index < second_length) {
		/* If the next element in the first array is greater than the second... */
		if (first_array[first_index].rating >= second_array[second_index].rating) {
			/* Add the first array element to the final array */
			output_array[final_index] = first_array[first_index];
			first_index++;
		} else {
			/* Add the second array element to the final array */
			output_array[final_index] = second_array[second_index];
			second_index++;
		}
		final_index++;
	}
	int elements_to_add = first_length - first_index;
	/* When one side array has been added to the output array before the
	 * other has been fully added */
	for (int i = 0; i < elements_to_add; i++) {
		output_array[final_index] = first_array[first_index];
		first_index++;
		final_index++;
	}
	elements_to_add = second_length - second_index;
	for (int i = 0; i < elements_to_add; i++) {
		output_array[final_index] = second_array[second_index];
		second_index++;
		final_index++;
	}
}

void merge_sort_pr_entry_array(struct entry *pr_entries, int array_size) {
	if (array_size <= 1) {
		return;
	} else if (array_size == 2) {
		if (pr_entries[0].rating < pr_entries[1].rating) {
			struct entry swap = pr_entries[0];
			pr_entries[0] = pr_entries[1];
			pr_entries[1] = swap;
		} else {
			return;
		}
	} else {
		/* split into 2 calls and recurse */
		int middle_index = (int) floor(array_size / 2.00);
		int len_sec_half = (int) ceil(array_size / 2.00);
		merge_sort_pr_entry_array(pr_entries, middle_index);
		merge_sort_pr_entry_array(&pr_entries[middle_index], len_sec_half);
		/* merge 2 resulting arrays */
		struct entry ret[array_size];
		merge(pr_entries, middle_index, &pr_entries[middle_index], len_sec_half, &ret[0]);
		/* Copy merged array contents into original array */
		for (int i = 0; i < array_size; i++) {
			pr_entries[i] = ret[i];
		}
		return;
	}
}

void merge_player_records(struct record *first_array, int first_length, \
	struct record *second_array, int second_length, struct record *output_array) {

	int first_index = 0;
	int second_index = 0;
	int final_index = 0;

	while (first_index < first_length && second_index < second_length) {
		/* If the second name is < the first name */
		if (strcmp(second_array[second_index].opp_name, first_array[first_index].opp_name) < 0) {
			output_array[final_index] = second_array[second_index];
			second_index++;
		/* If the first name is < the second name */
		} else {
			output_array[final_index] = first_array[first_index];
			first_index++;
		}
		final_index++;
	}
	int elements_to_add = first_length - first_index;
	/* When one side array has been added to the output array before the
	 * other has been fully added */
	for (int i = 0; i < elements_to_add; i++) {
		/* Add the first array element to the final array */
		output_array[final_index] = first_array[first_index];
		first_index++;
		final_index++;
	}
	elements_to_add = second_length - second_index;
	for (int i = 0; i < elements_to_add; i++) {
		/* Add the second array element to the final array */
		output_array[final_index] = second_array[second_index];
		second_index++;
		final_index++;
	}
}

void merge_sort_player_records(struct record *records, int array_size) {
	if (array_size <= 1) {
		return;
	} else if (array_size == 2) {
		/* If there is less data on the first player or if there is equal
		 * data, but the second name < first name */
		if (strcmp(records[1].opp_name, records[0].opp_name) < 0) {
			struct record swap;
			/* Save data from first player to swap variables */
			swap = records[0];
			/* Put second player data in first player spot */
			records[0] = records[1];
			/* Put first player (swap) data in second player spot */
			records[1] = swap;
		} else {
			return;
		}
	} else {
		/* split into 2 calls and recurse */
		int middle_index = (int) floor(array_size / 2.00);
		int len_sec_half = (int) ceil(array_size / 2.00);
		merge_sort_player_records(records, middle_index);
		merge_sort_player_records(&records[middle_index], len_sec_half);
		/* merge 2 resulting arrays */
		struct record ret[array_size];
		merge_player_records(records, middle_index, \
			&records[middle_index], len_sec_half, ret);
		/* Copy merged array contents into original array */
		for (int i = 0; i < array_size; i++) {
			records[i] = ret[i];
		}
		return;
	}
}

void merge_tournament_attendees(struct tournament_attendee *first_array, \
	int first_length, struct tournament_attendee *second_array, \
	int second_length, struct tournament_attendee *output_array) {

	int first_index = 0;
	int second_index = 0;
	int final_index = 0;

	while (first_index < first_length && second_index < second_length) {
		/* If the second name is < the first name */
		if (strcmp(second_array[second_index].name, first_array[first_index].name) < 0) {
			output_array[final_index] = second_array[second_index];
			second_index++;
		/* If the first name is < the second name */
		} else {
			output_array[final_index] = first_array[first_index];
			first_index++;
		}
		final_index++;
	}
	int elements_to_add = first_length - first_index;
	/* When one side array has been added to the output array before the
	 * other has been fully added */
	for (int i = 0; i < elements_to_add; i++) {
		/* Add the first array element to the final array */
		output_array[final_index] = first_array[first_index];
		first_index++;
		final_index++;
	}
	elements_to_add = second_length - second_index;
	for (int i = 0; i < elements_to_add; i++) {
		/* Add the second array element to the final array */
		output_array[final_index] = second_array[second_index];
		second_index++;
		final_index++;
	}
}

void merge_sort_tournament_attendees(struct tournament_attendee *attendees, \
	int array_size) {

	if (array_size <= 1) {
		return;
	} else if (array_size == 2) {
		/* If there is less data on the first player or if there is equal
		 * data, but the second name < first name */
		if (strcmp(attendees[1].name, attendees[0].name) < 0) {
			struct tournament_attendee swap;
			/* Save data from first player to swap variables */
			swap = attendees[0];
			/* Put second player data in first player spot */
			attendees[0] = attendees[1];
			/* Put first player (swap) data in second player spot */
			attendees[1] = swap;
		} else {
			return;
		}
	} else {
		/* split into 2 calls and recurse */
		int middle_index = (int) floor(array_size / 2.00);
		int len_sec_half = (int) ceil(array_size / 2.00);
		merge_sort_tournament_attendees(attendees, middle_index);
		merge_sort_tournament_attendees(&attendees[middle_index], len_sec_half);
		/* merge 2 resulting arrays */
		struct tournament_attendee ret[array_size];
		merge_tournament_attendees(attendees, middle_index, \
			&attendees[middle_index], len_sec_half, ret);
		/* Copy merged array contents into original array */
		for (int i = 0; i < array_size; i++) {
			attendees[i] = ret[i];
		}
		return;
	}
}
