# G2ME - A C implementation of Glicko-2 with a
# C program to make managing a Glicko-2 system easier.
# © 2017 Julian Speedie

PREFIX = /usr/local
MANPREFIX = /usr/share/man
# gcc flags for includes
INCS = -I. -I/usr/include
G2MELIBS = -L/usr/lib -lm -lpthread
LIBS = -L/usr/lib
# Flags
CFLAGS = -Wall
# Compiler and linker
# CC = gcc -ggdb3
CC = gcc
WCC32 = i686-w64-mingw32-gcc
WCC64 = x86_64-w64-mingw32-gcc

G2MEDEP = glicko2.c entry.c p_files.c pr.c opp_files.c tournament_files.c printing.c \
	fileops.c sorting.c player_dir.c data_dir.c
G2MEOBJ = ${G2MEDEP:.c=.o}
G2MESERVEROBJ = clientserverutil.o
G2MECLIENTOBJ = clientserverutil.o

SRC = G2ME.c server.c client.c
OBJ = ${SRC:.c=.o}
BIN = G2ME G2ME-server G2ME-client
# MAN should be defined as all the array elements in BIN appended with a ".1.gz"
# e.g. MAN = G2ME.1.gz G2ME-server.1.gz G2ME-client.1.gz
MAN := $(shell find man/ -name '*.1.gz')

TESTDIR = tests
# TESTSRC = Add an element '$(TESTDIR)/*.c' for all files that match the pattern
TESTSRC = $(wildcard $(TESTDIR)/*.c)
# TESTBIN = For all elements in '$(TESTSRC)' matching '$(TESTDIR)/%.c', add an element '$(TESTDIR)/bin/%'
TESTBIN = $(patsubst $(TESTDIR)/%.c, $(TESTDIR)/bin/%, $(TESTSRC))
# TESTBINLOCAL = For all elements in '$(TESTBIN)' matching '$(TESTDIR)/bin/%', add an element 'bin/%'
TESTBINLOCAL = $(patsubst $(TESTDIR)/bin/%, bin/%, $(TESTBIN))
# TESTOBJ = For all elements in '$(G2MEDEP)' matching '%.c', add an element '%.o'
# TESTOBJ = $(patsubst %.c, %.o, $(G2MEDEP))
TESTDEP = player_dir.c data_dir.c sorting.c fileops.c printing.c \
	tournament_files.c opp_files.c p_files.c entry.c glicko2.c

# `compile` first because we want `make` to just compile the program, and the
# default target is always the the first one that doesn't begin with "."
compile: G2ME server client
# compile: G2ME server client clean

G2ME: G2ME.o $(G2MEOBJ)
	$(CC) $(CFLAGS) $< $(G2MEOBJ) $(INCS) $(G2MELIBS) -o G2ME

# This rule requires the mingw-w64-gcc compilers. On Arch they can be installed
# with 'sudo pacman -S mingw-w64-gcc'
windows: G2ME.c $(G2MEDEP)
	@# --static to create a statically linked binary to avoid issues with
	@# pthreads on windows
	$(WCC32) --static $(CFLAGS) $< $(G2MEDEP) -o G2ME32.exe
	$(WCC64) --static $(CFLAGS) $< $(G2MEDEP) -o G2ME64.exe

# Build the object file needed for G2ME-server and G2ME-client targets
# -c flag stops gcc from linking
clientserverutil.o: clientserverutil.c
	$(CC) $(CFLAGS) $< -c

# Build the object file (-c flag stops gcc from linking)
server.o: server.c
	$(CC) $(CFLAGS) $< -c

# Build the object file (-c flag stops gcc from linking)
client.o: client.c
	$(CC) $(CFLAGS) $< -c

server: server.o $(G2MESERVEROBJ)
	$(CC) $(CFLAGS) $< $(G2MESERVEROBJ) $(INCS) $(LIBS) -o G2ME-server

client: client.o $(G2MECLIENTOBJ)
	$(CC) $(CFLAGS) $< $(G2MECLIENTOBJ) $(INCS) $(LIBS) -o G2ME-client

all: compile install

# If '$(TESTDIR)/bin' does not exist, make it
$(TESTDIR)/bin:
	mkdir $@

# $(TESTDIR)/bin/%: $(TESTOBJ)
# 	$(CC) $(CFLAGS) $(TESTOBJ) $(G2MELIBS) -lcriterion -o $@
# $(TESTDIR)/bin/%: $(TESTDIR)/%.c $(TESTDEP)
# 	$(CC) $(CFLAGS) $(TESTDEP) $< $(G2MELIBS) -lcriterion -o $@

$(TESTDIR)/bin/player_tests: glicko2.c tests/player_tests.c
	$(CC) $(CFLAGS) $^ -lm -lcriterion -o $@

$(TESTDIR)/bin/entry_tests: glicko2.c entry.c tests/entry_tests.c
	$(CC) $(CFLAGS) $^ -lm -lcriterion -o $@

$(TESTDIR)/bin/p_file_tests: p_files.o opp_files.o tournament_files.o entry.o player_dir.o data_dir.o glicko2.o fileops.o tests/p_file_tests.o
	$(CC) $(CFLAGS) $^ -lm -lcriterion -o $@

# $(TESTDIR)/bin/output_tests: G2ME.o $(G2MEOBJ) tests/output_tests.c
$(TESTDIR)/bin/output_tests: G2ME.c $(G2MEOBJ) tests/output_tests.c
	$(CC) $(CFLAGS) $^ $(INCS) $(G2MELIBS) -lcriterion -o $@

test: unit_test integration_test

unit_test: compile $(TESTDIR)/bin $(TESTBIN)
	@echo "Running unit tests:"
	@cd tests && for test in $(TESTBINLOCAL); do echo "Running $$test"; ./$$test --verbose=1; done; cd ..

integration_test: compile
	@echo "Running integration tests:"
	@cd tests && python3 runtestcases.py; cd ..

install: $(BIN)
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f $(BIN) ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/G2ME
	@mkdir -p $(MANPREFIX)/man1/
	@cp -f $(MAN) $(MANPREFIX)/man1/

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@for i in $(BIN); do \
		rm -f ${DESTDIR}${PREFIX}/bin/$$i; \
	done
	@for page in $(MAN); do \
		rm -f $(MANPREFIX)/man1/$$page; \
	done
	@rm -f G2ME G2ME-server G2ME-client ${OBJ} G2ME-${VERSION}.tar.gz

# Needs to be tested - should simply remove all the object files
clean:
	@echo cleaning
	@rm -f ${OBJ}
