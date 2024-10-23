#ifndef G2ME_PR
#define G2ME_PR


int append_pr_entry_to_file(struct entry *E, char *file_path, \
	int longest_name_length, bool output_to_stdout);

int append_pr_entry_to_file_verbose(struct entry *E, \
	char *file_path, int longest_name_length, int longest_attended_count, \
	int longest_outcome_count, bool output_to_stdout);

#endif
