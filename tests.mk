BASE_T_D=tests
BIN_T_D_NAME=bin
BIN_T_D=$(BASE_T_D)/$(BIN_T_D_NAME)
# Flags
# TODO: maybe create a separate variable for the debug flags as well as a
# separate variable for optimization flags for the release build
CFLAGS=-Wall -ggdb3
# CFLAGS=-Wall
# Compiler and linker
CC=gcc

#------------------------------------------------------------------------------
#   Manually Specified Information
#------------------------------------------------------------------------------
# `NAME` cannot refer to multiple binaries
# We don't use the `NAME` variable in this script because we are building
# multiple executables.
NAME:=
BASE_SRCS_T:=player_tests.c entry_tests.c p_file_tests.c output_tests.c
# e.g. SRCS_T = tests/player_tests.c tests/entry_tests.c [...]
SRCS_T=$(addprefix $(BASE_T_D)/, $(BASE_SRCS_T))
SRCS_G?=G2ME.c glicko2.c entry.c p_files.c pr.c opp_files.c \
	tournament_files.c printing.c fileops.c sorting.c player_dir.c data_dir.c
# `INCS_T` and `LIBS_T` expect you to specify, respectively, include flags and
# their arguments and library flags and their arguments
INCS_T:=
LIBS_T:=-lm -lcriterion
LIBS_G?=-lm -lpthread

#------------------------------------------------------------------------------
#   Automatically Specified Information
#------------------------------------------------------------------------------
BASE_OBJS_T:=${BASE_SRCS_T:.c=.o}
# e.g. OBJS_T = tests/player_tests.o tests/entry_tests.o [...]
OBJS_T=$(addprefix $(BASE_T_D)/, $(BASE_OBJS_T))
OBJS_T_E=$(wildcard $(OBJS_T))
OBJS_G?=${SRCS_G:.c=.o}

BASE_DEPS_T:=${BASE_SRCS_T:.c=.d}
# e.g. DEPS_T = tests/player_tests.d tests/entry_tests.d [...]
DEPS_T=$(addprefix $(BASE_T_D)/, $(BASE_DEPS_T))
DEPS_T_E=$(wildcard $(DEPS_T))

BASE_BIN_T:=${BASE_SRCS_T:.c=}
# e.g. BIN_T = tests/bin/player_tests tests/bin/entry_tests [...]
BIN_T=$(addprefix $(BIN_T_D)/, $(BASE_BIN_T))
BIN_T_E=$(wildcard $(BIN_T))

#------------------------------------------------------------------------------
#   Additional Rules
#------------------------------------------------------------------------------
# e.g. BIN_T_LOCAL = bin/player_tests bin/entry_tests [...]
BIN_T_LOCAL=$(addprefix $(BIN_T_D_NAME)/, $(BASE_BIN_T))

# TODO: do any of the tests reliably make use of this? Or does the repo simply
# include a test/bin directory and so the recipe of this rule is never
# executed?
#
# If '$(BIN_T_D)' does not exist, make it
$(BIN_T_D):
	mkdir $@

$(BIN_T_D)/player_tests: $(BASE_T_D)/player_tests.c glicko2.c | $(BIN_T_D)
	$(CC) $(CFLAGS) $^ $(INCS_T) $(LIBS_T) -o $@

# $(BIN_T_D)/entry_tests: $(BASE_T_D)/entry_tests.c glicko2.c entry.c
$(BIN_T_D)/entry_tests: $(BASE_T_D)/entry_tests.c glicko2.c entry.c | $(BIN_T_D)
	$(CC) $(CFLAGS) $^ $(INCS_T) $(LIBS_T) -o $@

# $(BIN_T_D)/p_file_tests: $(BASE_T_D)/p_file_tests.c p_files.c opp_files.c tournament_files.c entry.c player_dir.c data_dir.c glicko2.c fileops.c
$(BIN_T_D)/p_file_tests: $(BASE_T_D)/p_file_tests.c p_files.c opp_files.c tournament_files.c entry.c player_dir.c data_dir.c glicko2.c fileops.c | $(BIN_T_D)
	$(CC) $(CFLAGS) $^ $(INCS_T) $(LIBS_T) -o $@

# TODO: does this tests get OBJ_G and G2MELIBS? I think it isnt specified
# anywhere
$(BIN_T_D)/output_tests: $(BASE_T_D)/output_tests.c $(OBJ_G) | $(BIN_T_D)
	$(CC) $(CFLAGS) $^ $(INCS_T) $(G2MELIBS) $(LIBS_T) -o $@

$(BASE_T_D)/%.o: $(BASE_T_D)/%.c
	$(CC) $(CFLAGS) $< $(INCS_T) -c -o $@

# A rule to create a dependency file for every `.c` file
$(BASE_T_D)/%.d: $(BASE_T_D)/%.c
	$(CC) $(CFLAGS) $< $(INCS_T) -MM -MF $@

# Include all the dependency files IF the commandline goal given to Make was
# not `clean`
ifneq ($(MAKECMDGOALS),clean)
include $(DEPS_T)
endif

.PHONY: test
test: unit_test integration_test

.PHONY: unit_test
unit_test: compile $(BIN_T) $(BIN_T_D)
	@echo "Running unit tests:"
	@cd tests && \
	for test in $(BIN_T_LOCAL); do \
		echo "Running $$test"; \
		./$$test --verbose=1; \
	done; cd ..

.PHONY: integration_test
integration_test: compile
	@echo "Running integration tests:"
	@cd tests && \
	python3 runtestcases.py; \
	cd ..

.PHONY: tests-clean
tests-clean:
ifneq ($(strip $(OBJS_T_E)),)
	rm -f $(OBJS_T_E)
else
	$(info No testing-related .o files to remove)
endif
ifneq ($(strip $(DEPS_T_E)),)
	rm -f $(DEPS_T_E)
else
	$(info No testing-related .d files to remove)
endif
