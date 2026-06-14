
#------------------------------------------------------------------------------
#   Manually specified information
#------------------------------------------------------------------------------
# `NAME` cannot contain more than one element
NAME:=G2ME
SRCS:=G2ME.c glicko2.c entry.c p_files.c pr.c opp_files.c \
	tournament_files.c printing.c fileops.c sorting.c player_dir.c data_dir.c
# `-I`, `-L`, `-l`
INCS:=
LIBRARIES:=
LIBS:=-lm -lpthread

#------------------------------------------------------------------------------
#   Include template makefile(s)
#------------------------------------------------------------------------------
# Create Make recipes for `$(NAME)_release`, `$(NAME)_release_install`,
# `$(NAME)_release_uninstall`, and `$(NAME)_release_clean`
include template-release.mk
# Create Make recipes for `$(NAME)_debug`, `$(NAME)_debug_install`,
# `$(NAME)_debug_uninstall`, and `$(NAME)_debug_clean`
include template-debug.mk
