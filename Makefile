# G2ME - A C implementation of Glicko-2 with a
# C program to make managing a Glicko-2 system easier.
# © 2026 Julian Speedie

#------------------------------------------------------------------------------
#   Manually Specified Information
#------------------------------------------------------------------------------
# Install locations
PREFIX := /usr/local
MANPREFIX := /usr/share/man
# TODO: set a default include for the project? Right now [nothing] makes the
# most sense.
INCS := 
# TODO: set a default value for this library variable? Right now [nothing]
# makes the most sense.
LIBS :=
# TODO: maybe create a separate variable for the debug flags as well as a
# separate variable for optimization flags for the release build
CFLAGS = -Wall -ggdb3
# CFLAGS = -Wall
# Compilers and linkers
CC = gcc
# TODO: move the windows compilers to windows.mk with all the other windows
# specific stuff
WCC32 = i686-w64-mingw32-gcc
WCC64 = x86_64-w64-mingw32-gcc

#------------------------------------------------------------------------------
#   Include other makefiles
#------------------------------------------------------------------------------

# `compile` first because we want Make to just compile all the executable, and
# the default target of a given Makefile is always the the first target that
# doesn't begin with '.'.
.PHONY: all
compile: G2ME G2ME-server G2ME-client

.PHONY: install
install: G2ME-install G2ME-server-install G2ME-client-install

.PHONY: uninstall
uninstall: G2ME-uninstall G2ME-server-uninstall G2ME-client-uninstall

.PHONY: clean
clean: G2ME-clean G2ME-server-clean G2ME-client-clean tests-clean

include G2ME.mk
include G2ME-client.mk
include G2ME-server.mk
include tests.mk

# This rule requires the mingw-w64-gcc compilers. They can be installed
# with:
#
#     sudo pacman -S mingw-w64-gcc    # Arch
#
windows: G2ME.c $(G2MEDEP)
	@# --static to create a statically linked binary to avoid issues with
	@# pthreads on windows
	$(WCC32) --static $(CFLAGS) $< $(G2MEDEP) -o G2ME32.exe
	$(WCC64) --static $(CFLAGS) $< $(G2MEDEP) -o G2ME64.exe

