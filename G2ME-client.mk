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
# `NAME_GC` cannot refer to multiple binaries
NAME_GC:=G2ME-client
SRCS_GC:=client.c clientserverutil.c
# `INCS_GC` and `LIBS_GC` expect you to specify, respectively, include flags and
# their arguments and library flags and their arguments
INCS_GC:=
LIBS_GC:=

#------------------------------------------------------------------------------
#   Automatically Specified Information
#------------------------------------------------------------------------------
OBJS_GC:=${SRCS_GC:.c=.o}
OBJS_GC_E=$(wildcard $(OBJS_GC))
DEPS_GC:=${SRCS_GC:.c=.d}
DEPS_GC_E=$(wildcard $(DEPS_GC))
# `MAN_GC` cannot refer to multiple files
# Populate `MAN_GC` with all the array elements of `NAME_GC` but with a ".1.gz"
# appended to each element.
MAN_GC:=$(patsubst %, %.1.gz, $(NAME_GC))

#------------------------------------------------------------------------------
#   Additional Rules
#------------------------------------------------------------------------------
G2ME-client: $(OBJS_GC)
	$(CC) $(CFLAGS) $^ $(INCS_GC) $(LIBS_GC) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< $(INCS_GC) -c -o $@

# A rule to create a dependency file for every `.c` file
%.d: %.c
	$(CC) $(CFLAGS) $< $(INCS_GC) -MM -MF $@

# Include all the dependency files IF the commandline goal given to Make was
# not `clean`
ifneq ($(MAKECMDGOALS),clean)
include $(DEPS_GC)
endif

.PHONY: G2ME-client-install
G2ME-client-install: $(NAME_GC)
	mkdir -p $(PREFIX)/bin
	cp -f $(NAME_GC) $(PREFIX)/bin/$(NAME_GC); \
	chmod 755 $(PREFIX)/bin/$(NAME_GC); \
	mkdir -p $(MANPREFIX)/man1/
	cp -f $(MAN_GC) $(MANPREFIX)/man1/$(MAN_GC); \

.PHONY: G2ME-client-uninstall
G2ME-client-uninstall:
	rm -f $(PREFIX)/bin/$(NAME_GC)
	rm -f $(MANPREFIX)/man1/$(MAN_GC)

.PHONY: G2ME-client-clean
G2ME-client-clean:
ifneq ($(strip $(OBJS_GC_E)),)
	rm -f $(OBJS_GC_E)
else
	$(info No $(NAME_GC) .o files to remove)
endif
ifneq ($(strip $(DEPS_GC_E)),)
	rm -f $(DEPS_GC_E)
else
	$(info No $(NAME_GC) .d files to remove)
endif
