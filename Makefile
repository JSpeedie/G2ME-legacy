# G2ME - A C implementation of a python implementation of glicko2 with a
# C program to make managing a glicko2 system easier.
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
# CC = gcc -g
CC = gcc

G2MEDEP = glicko2.c entry_file.c opp_files.c tournament_files.c printing.c \
	fileops.c sorting.c player_dir.c
G2MESERVERDEP = clientserverutil.o
G2MECLIENTDEP = clientserverutil.o

SRC = G2ME.c server.c client.c
OBJ = ${SRC:.c=.o}
BIN = G2ME G2ME-server G2ME-client
# MAN should be defined as all the array elements in BIN appended with a ".1.gz"
# e.g. MAN = G2ME.1.gz G2ME-server.1.gz G2ME-client.1.gz
MAN := $(shell find man/ -name '*.1.gz')


# `compile` first because we want `make` to just compile the program, and the
# default target is always the the first one that doesn't begin with "."
compile: G2ME server client
# compile: G2ME server client clean

G2ME: G2ME.c $(G2MEDEP)
	$(CC) $(CFLAGS) G2ME.c $(G2MEDEP) $(INCS) $(G2MELIBS) -o G2ME

# Build the object file needed for G2ME-server and G2ME-client targets
# -c flag stops gcc from linking
clientserverutil.o: clientserverutil.c
	$(CC) $(CFLAGS) clientserverutil.c -c

# Build the object file (-c flag stops gcc from linking)
server.o: server.c
	$(CC) $(CFLAGS) server.c -c

# Build the object file (-c flag stops gcc from linking)
client.o: client.c
	$(CC) $(CFLAGS) client.c -c

server: server.o $(G2MESERVERDEP)
	$(CC) $(CFLAGS) server.o $(G2MESERVERDEP) $(INCS) $(LIBS) -o G2ME-server

client: client.o $(G2MECLIENTDEP)
	$(CC) $(CFLAGS) client.o $(G2MECLIENTDEP) $(INCS) $(LIBS) -o G2ME-client

all: compile install

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

test: compile
	@cd tests && sh runtestcases.sh

# Needs to be tested - should simply remove all the object files
clean:
	@echo cleaning
	@rm -f ${OBJ}
