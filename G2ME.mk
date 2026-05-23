PREFIX:=/usr/local
MANPREFIX:=/usr/share/man
# Flags
# TODO: maybe create a separate variable for the debug flags as well as a
# separate variable for optimization flags for the release build
CFLAGS:=-Wall -ggdb3
# CFLAGS=-Wall
# Compiler and linker
CC:=gcc

#------------------------------------------------------------------------------
#   Manually Specified Information
#------------------------------------------------------------------------------
# `NAME_G` cannot refer to multiple binaries
NAME_G:=G2ME
SRCS_G:=G2ME.c glicko2.c entry.c p_files.c pr.c opp_files.c \
	tournament_files.c printing.c fileops.c sorting.c player_dir.c data_dir.c
# `INCS_G` and `LIBS_G` expect you to specify, respectively, include flags and
# their arguments and library flags and their arguments
INCS_G:=
LIBS_G:=-lm -lpthread

#------------------------------------------------------------------------------
#   Automatically Specified Information
#------------------------------------------------------------------------------
OBJS_G:=${SRCS_G:.c=.o}
OBJS_G_E=$(wildcard $(OBJS_G))
DEPS_G:=${SRCS_G:.c=.d}
DEPS_G_E=$(wildcard $(DEPS_G))
# `MAN_G` cannot refer to multiple files
# Populate `MAN_G` with all the array elements of `NAME_G` but with a ".1.gz"
# appended to each element.
MAN_G=$(patsubst %, %.1.gz, $(NAME_G))

#------------------------------------------------------------------------------
#   Additional Rules
#------------------------------------------------------------------------------
G2ME: $(OBJS_G)
	$(CC) $(CFLAGS) $^ $(INCS_G) $(LIBS_G) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< $(INCS_G) -c -o $@

# A rule to create a dependency file for every `.c` file
%.d: %.c
	$(CC) $(CFLAGS) $< $(INCS_G) -MM -MF $@

# Include all the dependency files IF the commandline goal given to Make was
# not `clean`
ifneq ($(MAKECMDGOALS),clean)
include $(DEPS_G)
endif

.PHONY: G2ME-install
G2ME-install: $(NAME_G)
	mkdir -p $(PREFIX)/bin
	cp -f $(NAME_G) $(PREFIX)/bin/$(NAME_G); \
	chmod 755 $(PREFIX)/bin/$(NAME_G); \
	mkdir -p $(MANPREFIX)/man1/
	cp -f $(MAN_G) $(MANPREFIX)/man1/$(MAN_G); \

.PHONY: G2ME-uninstall
G2ME-uninstall:
	rm -f $(PREFIX)/bin/$(NAME_G)
	rm -f $(MANPREFIX)/man1/$(MAN_G)

.PHONY: G2ME-clean
G2ME-clean:
ifneq ($(strip $(OBJS_G_E)),)
	rm -f $(OBJS_G_E)
else
	$(info No $(NAME_G) .o files to remove)
endif
ifneq ($(strip $(DEPS_G_E)),)
	rm -f $(DEPS_G_E)
else
	$(info No $(NAME_G) .d files to remove)
endif
