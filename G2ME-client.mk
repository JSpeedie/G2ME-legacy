
#------------------------------------------------------------------------------
#   Manually specified information
#------------------------------------------------------------------------------
# `NAME` cannot contain more than one element
NAME:=G2ME-client
SRCS:=client.c clientserverutil.c
# `-I`, `-L`, `-l`
INCS:=
LIBRARIES:=
LIBS:=

#------------------------------------------------------------------------------
#   Include template makefile(s)
#------------------------------------------------------------------------------
# Create Make recipes for `$(NAME)_release`, `$(NAME)_release_install`,
# `$(NAME)_release_uninstall`, and `$(NAME)_release_clean`
include template-release.mk
# Create Make recipes for `$(NAME)_debug`, `$(NAME)_debug_install`,
# `$(NAME)_debug_uninstall`, and `$(NAME)_debug_clean`
# TODO: uncomment
# include template-debug.mk
