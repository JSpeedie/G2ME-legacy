PREFIX=/usr/local
MANPREFIX=/usr/share/man
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
# `NAME_GS` cannot refer to multiple binaries
NAME_GS:=G2ME-server
SRCS_GS:=server.c clientserverutil.c
# `INCS_GS` and `LIBS_GS` expect you to specify, respectively, include flags and
# their arguments and library flags and their arguments
INCS_GS:=
LIBS_GS:=

#------------------------------------------------------------------------------
#   Automatically Specified Information
#------------------------------------------------------------------------------
OBJS_GS:=${SRCS_GS:.c=.o}
OBJS_GS_E=$(wildcard $(OBJS_GS))
DEPS_GS:=${SRCS_GS:.c=.d}
DEPS_GS_E=$(wildcard $(DEPS_GS))
# `MAN_GS` cannot refer to multiple files
# Populate `MAN_GS` with all the array elements of `NAME_GS` but with a ".1.gz"
# appended to each element.
MAN_GS:=$(patsubst %, %.1.gz, $(NAME_GS))

#------------------------------------------------------------------------------
#   Additional Rules
#------------------------------------------------------------------------------
G2ME-server: $(OBJS_GS)
	$(CC) $(CFLAGS) $^ $(INCS_GS) $(LIBS_GS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< $(INCS_GS) -c -o $@

# A rule to create a dependency file for every `.c` file
%.d: %.c
	$(CC) $(CFLAGS) $< $(INCS_GS) -MM -MF $@

# Include all the dependency files IF the commandline goal given to Make was
# not `clean`
ifneq ($(MAKECMDGOALS),clean)
include $(DEPS_GS)
endif

.PHONY: G2ME-server-install
G2ME-server-install: $(NAME_GS)
	mkdir -p $(PREFIX)/bin
	cp -f $(NAME_GS) $(PREFIX)/bin/$(NAME_GS); \
	chmod 755 $(PREFIX)/bin/$(NAME_GS); \
	mkdir -p $(MANPREFIX)/man1/
	cp -f $(MAN_GS) $(MANPREFIX)/man1/$(MAN_GS); \

.PHONY: G2ME-server-uninstall
G2ME-server-uninstall:
	rm -f $(PREFIX)/bin/$(NAME_GS)
	rm -f $(MANPREFIX)/man1/$(MAN_GS)

.PHONY: G2ME-server-clean
G2ME-server-clean:
ifneq ($(strip $(OBJS_GS_E)),)
	rm -f $(OBJS_GS_E)
else
	$(info No $(NAME_GS) .o files to remove)
endif
ifneq ($(strip $(DEPS_GS_E)),)
	rm -f $(DEPS_GS_E)
else
	$(info No $(NAME_GS) .d files to remove)
endif
