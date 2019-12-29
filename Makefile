# G2ME - A C implementation of a python implementation of glicko2 with a
# C program to make managing a glicko2 system easier.
# © 2017 Julian Speedie

PREFIX = /usr/local
MANPREFIX = /usr/share/man
# gcc flags for includes
INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lm -lpthread
# Flags
CFLAGS = -Wall
# Compiler and linker
# CC = gcc -ggdb
CC = gcc

SRC = G2ME.c
DEP = glicko2.c entry_file.c opp_files.c tournament_files.c printing.c \
	fileops.c sorting.c player_dir.c
OBJ = ${SRC:.c=.o}
BIN = ${SRC:.c=}
MAN = $(SRC:.c=.1.gz)

all: compile install

compile: G2ME.c $(DEP)
	$(CC) $(CFLAGS) G2ME.c $(DEP) $(INCS) $(LIBS) -o G2ME

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

test: compile
	@cd tests && sh runtestcases.sh

# Needs to be fixed
clean:
	@echo cleaning
	@rm -f G2ME ${OBJ} G2ME-${VERSION}.tar.gz
