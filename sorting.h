#include "G2ME.h"

void merge(struct entry *, int, struct entry *, int, struct entry *);
void merge_sort_pr_entry_array(struct entry *, int);
void merge_player_records(struct record *, int, \
	struct record *, int, struct record *);
void merge_sort_player_records(struct record *, int);
void merge_tournament_attendees(struct tournament_attendee *, int, \
	struct tournament_attendee *, int, struct tournament_attendee *);
void merge_sort_tournament_attendees(struct tournament_attendee *, int);
