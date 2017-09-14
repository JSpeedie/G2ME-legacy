#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "glicko2.h"


char use_games = 0;

typedef struct entry {
	char len_name;
	char len_opp_name;
	char name[128];
	char opp_name[128];
	double rating;
	double RD;
	double vol;
	char gc;
	char opp_gc;
	char day;
	char month;
	short year;
}Entry;

long int get_last_entry_offset(char* file_path) {
	FILE *entry_file = fopen(file_path, "rb");
	if (entry_file == NULL) {
		perror("fopen (get_last_entry_offset)");
		return 0;
	}

	char len_of_name;
	char len_of_opp_name;
	long int last_entry_offset = ftell(entry_file);

	while (0 != fread(&len_of_name, sizeof(char), 1, entry_file)) {

		if (0 == fread(&len_of_opp_name, sizeof(char), 1, entry_file)) {
			break;
		}

		long int size_of_current_entry =
			len_of_name + len_of_opp_name + (3 * sizeof(double)) + (4 * sizeof(char)) + sizeof(short);
		fseek(entry_file, size_of_current_entry, SEEK_CUR);

		last_entry_offset = ftell(entry_file) - size_of_current_entry - (2 * sizeof(char));
	}

	fclose(entry_file);

	return last_entry_offset;
}

void clear_file(char* file) {

	FILE* victim = fopen(file, "w");
	if (victim == NULL) {
		perror("fopen (clear_file)");
		return;
	}

	fprintf(victim, "");
	fclose(victim);
	return;
}

void append_entry_to_file(struct entry* E, char* file_path) {
	FILE *entry_file = fopen(file_path, "ab+");
	if (entry_file == NULL) {
		perror("fopen (append_entry_to_file)");
		return;
	}
	// TODO find nice way to check all fwrite calls
	/* Write lengths of names */
	char len_name = strlen(E->name);
	char len_opp_name = strlen(E->opp_name);
	fwrite(&len_name, sizeof(char), 1, entry_file);
	fwrite(&len_opp_name, sizeof(char), 1, entry_file);
	/* Write names */
	fwrite(E->name, sizeof(char), strlen(E->name), entry_file);
	fwrite(E->opp_name, sizeof(char), strlen(E->opp_name), entry_file);
	/* Write glicko data */
	fwrite(&E->rating, sizeof(double), 1, entry_file);
	fwrite(&E->RD, sizeof(double), 1, entry_file);
	fwrite(&E->vol, sizeof(double), 1, entry_file);
	/* Write game counts */
	fwrite(&E->gc, sizeof(char), 1, entry_file);
	fwrite(&E->opp_gc, sizeof(char), 1, entry_file);
	/* Write date data */
	fwrite(&E->day, sizeof(char), 1, entry_file);
	fwrite(&E->month, sizeof(char), 1, entry_file);
	fwrite(&E->year, sizeof(short), 1, entry_file);
	fclose(entry_file);
}

void append_pr_entry_to_file(struct entry* E, char* file_path) {
	FILE *entry_file = fopen(file_path, "a+");
	if (entry_file == NULL) {
		perror("fopen (append_pr_entry_to_file)");
		return;
	}
	if (fprintf(entry_file, "%s %.1lf %.2lf %.8lf\n", E->name, E->rating, E->RD, E->vol) < 0) {
		perror("fprintf");
	}

	fclose(entry_file);
	return;
}

void write_entry_from_input(char* file_path) {
	printf("[Name] [Opp] [Rating] [RD] [Vol] [gc] [opp gc] [day] [month] [year]: ");

	struct entry input_entry;
	scanf("%s %s %lf %lf %lf %hhd %hhd %hhd %hhd %hd",
		input_entry.name, input_entry.opp_name, &input_entry.rating,
		&input_entry.RD, &input_entry.vol, &input_entry.gc,
		&input_entry.opp_gc, &input_entry.day, &input_entry.month,
		&input_entry.year);
	append_entry_to_file(&input_entry, file_path);
}

void print_player(struct player *P) {
	printf("%16.14lf %16.14lf %16.14lf\n", getRating(P), getRd(P), P->vol);
}

void print_entry(struct entry E) {
	/* Process date data into one string */
	char date[32];
	sprintf(date, "%d/%d/%d", E.day, E.month, E.year);

	printf("%d %d %-10s %-10s %16.14lf %16.14lf %16.14lf %d-%d %s\n", \
		E.len_name, E.len_opp_name, E.name, E.opp_name, E.rating, E.RD, E.vol, E.gc, E.opp_gc, date);
}

int read_entry(FILE *f, struct entry *E) {
	if (0 == fread(&E->len_name, sizeof(char), 1, f)) { return -1; }
	if (0 == fread(&E->len_opp_name, sizeof(char), 1, f)) { return -2; }
	/* Read player names */
	if (0 == fread(E->name, sizeof(char), E->len_name, f)) { return -3; }
	if (0 == fread(E->opp_name, sizeof(char), E->len_opp_name, f)) { return -4; }
	/* Add null terminators */
	E->name[E->len_name] = '\0';
	E->opp_name[E->len_opp_name] = '\0';
	if (0 == fread(&E->rating, sizeof(double), 1, f)) { return -5; }
	if (0 == fread(&E->RD, sizeof(double), 1, f)) { return -6; }
	if (0 == fread(&E->vol, sizeof(double), 1, f)) { return -7; }
	if (0 == fread(&E->gc, sizeof(char), 1, f)) { return -8; }
	if (0 == fread(&E->opp_gc, sizeof(char), 1, f)) { return -9; }
	if (0 == fread(&E->day, sizeof(char), 1, f)) { return -10; }
	if (0 == fread(&E->month, sizeof(char), 1, f)) { return -11; }
	if (0 == fread(&E->year, sizeof(short), 1, f)) { return -12; }

	return 0;
}

void print_player_file(char* file_path) {
	FILE *p_file = fopen(file_path, "rb+");
	if (p_file == NULL) {
		perror("fopen (print_player_file)");
		return;
	}

	struct entry line;

	while (read_entry(p_file, &line) == 0) {
		print_entry(line);
	}

	fclose(p_file);

	return;
}

struct entry read_last_entry(char* file_path) {
	struct entry line;
	/* Open files for reading contents */
	FILE *p_file = fopen(file_path, "rb");
	if (p_file == NULL) {
		perror("fopen (read_last_entry)");
		return line;
	}

	/* Set file position to be at the latest entry for that player */
	long int offset = get_last_entry_offset(file_path);
	fseek(p_file, offset, SEEK_SET);
	read_entry(p_file, &line);
	fclose(p_file);

	return line;
}

void init_player_from_entry(struct player* P, struct entry* E) {
	setRating(P, E->rating);
	setRd(P, E->RD);
	P->vol = E->vol;
	return;
}

struct entry create_entry(struct player* P, char* name, char* opp_name,
	char gc, char opp_gc, char day, char month, short year) {

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

	return ret;
}

void update_player_on_outcome(char* p1_name, char* p2_name,
	struct player* p1, struct player* p2, double* p1_gc, double* p2_gc,
	char day, char month, short year) {

	/* If the file does not exist, init the player struct to defaults */
	if (access(p1_name, R_OK | W_OK) == -1) {
		setRating(p1, 1500.0);
		setRd(p1, 350.0);
		p1->vol = 0.06;
	} else {
		/* Read latest entries into usable data */
		struct entry p1_latest = read_last_entry(p1_name);
		init_player_from_entry(p1, &p1_latest);
	}
	/* If the file does not exist, init the player struct to defaults */
	if (access(p2_name, R_OK | W_OK) == -1) {
		setRating(p2, 1500.0);
		setRd(p2, 350.0);
		p2->vol = 0.06;
	} else {
		/* Read latest entries into usable data */
		struct entry p2_latest = read_last_entry(p2_name);
		init_player_from_entry(p2, &p2_latest);
	}

	p1->_tau = 0.5;
	p2->_tau = 0.5;

	struct player new_p1 = *p1;
	struct player new_p2 = *p2;

	update_player(&new_p1, &p2->__rating, 1, &p2->__rd, p1_gc);
	update_player(&new_p2, &p1->__rating, 1, &p1->__rd, p2_gc);
	struct entry p1_new_entry =
		create_entry(&new_p1, p1_name, p2_name, *p1_gc, *p2_gc, day, month, year);
	struct entry p2_new_entry =
		create_entry(&new_p2, p2_name, p1_name, *p2_gc, *p1_gc, day, month, year);
	append_entry_to_file(&p1_new_entry, p1_name);
	append_entry_to_file(&p2_new_entry, p2_name);

	return;
}

void update_players(char* bracket_file_path) {

	FILE *bracket_file = fopen(bracket_file_path, "r");
	if (bracket_file == NULL) {
		perror("fopen (bracket_file)");
		return;
	}

	char line[256];
	char p1_name[128];
	char p2_name[128];
	char p1_gc;
	char p2_gc;
	char day;
	char month;
	short year;

	while (fgets(line, sizeof(line), bracket_file)) {
		/* Read data from one line of bracket file into all the variables */
		sscanf(line, "%s %s %hhd %hhd %hhd %hhd %hd",
			p1_name, p2_name, &p1_gc, &p2_gc, &day, &month, &year);

		struct player p1;
		struct player p2;
		double p1_out;
		double p2_out;
		if (use_games == 1) {
			p1_out = 1;
			p2_out = 0;
			for (int i = 0; i < p1_gc; i++) {
				update_player_on_outcome(p1_name, p2_name, &p1, &p2, &p1_out, &p2_out, day, month, year);
			}
			p1_out = 0;
			p2_out = 1;
			for (int i = 0; i < p2_gc; i++) {
				update_player_on_outcome(p2_name, p1_name, &p2, &p1, &p2_out, &p1_out, day, month, year);
			}
		} else {
			p1_out = p1_gc > p2_gc;
			p2_out = p1_gc < p2_gc;
			update_player_on_outcome(p1_name, p2_name, &p1, &p2, &p1_out, &p2_out, day, month, year);
			update_player_on_outcome(p2_name, p1_name, &p2, &p1, &p2_out, &p1_out, day, month, year);
		}
	}

	// Print out everyones before and after with a (+/- change here)
	fclose(bracket_file);
}

void generate_ratings_file(char* file_path) {
	FILE *players = fopen(file_path, "r");
	if (players == NULL) {
		perror("fopen (generate_ratings_file)");
		return;
	}

	char output_file[256];
	printf("What file would like to output the power ratings to? ");
	scanf("%s", output_file);
	clear_file(output_file);

	char line[256];
	struct entry temp;

	while (fgets(line, sizeof(line), players)) {
		/* Replace newline with null terminator */
		*strchr(line, '\n') = '\0';
		temp = read_last_entry(line);
		append_pr_entry_to_file(&temp, output_file);
	}

	fclose(players);
	return;
}

int main(int argc, char **argv) {
	int opt;
	struct option opt_table[] = {
		{ "use-games",	no_argument,	NULL,	'g' },
		/* Output given player file in human readable form */
		{ "human",		required_argument,	NULL,	'h' },
		/* Output last entry in given player file in human readable form */
		{ "last-entry",	required_argument,	NULL,	'l' },
		/* Add (or create if necessary) a player entry/player entry file
		 * from user input */
		{ "add-entry",	required_argument,	NULL,	'a' },
		/* Run through a given bracket file making the necessary updates
		 * to the glicko2 scores */
		{ "bracket",	required_argument,	NULL,	'b' },
		/* Output a file with a sorted list of players and their ratings */
		{ "power-rating",required_argument,	NULL,	'p' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, \
		"gh:l:a:b:p:", opt_table, NULL)) != -1) {
		switch (opt) {
			case 'g':
				use_games = 1;
				break;
			case 'h':
				print_player_file(optarg);
				break;
			case 'l':
				print_entry(read_last_entry(optarg));
				break;
			case 'a':
				write_entry_from_input(optarg);
				break;
			case 'b':
				update_players(optarg);
				break;
			case 'p':
				generate_ratings_file(optarg);
				break;
		}
	}
}
