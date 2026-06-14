# G2ME - A C implementation of Glicko-2 with a
# C program to make managing a Glicko-2 system easier.
# © 2026 Julian Speedie

#   ----------------------------------------------------------------------------
#   All binaries and test binaries in the project
#   ----------------------------------------------------------------------------
# All items in `BINARIES` must have an associated `.mk` file that:
# (1) is included by this file,
# (2) includes `template-release.mk`,
# (3) has a value for `NAME` that matches an item in `BINARIES`
BINARIES := G2ME

#   ----------------------------------------------------------------------------
#   Categorized lists of all build targets
#   ----------------------------------------------------------------------------
BINARIES_R := $(BINARIES:%=%_release)

#   ----------------------------------------------------------------------------
#   The makefiles for each build result
#   ----------------------------------------------------------------------------
include G2ME.mk

#   ----------------------------------------------------------------------------
#   High-level rules for the developer to invoke from the commandline
#   ----------------------------------------------------------------------------
.DEFAULT_GOAL := release

.PHONY: release
release: $(BINARIES_R)

# # This rule requires the mingw-w64-gcc compilers. They can be installed
# # with:
# #
# #     sudo pacman -S mingw-w64-gcc    # Arch
# #
# windows: G2ME.c $(G2MEDEP)
# 	@# --static to create a statically linked binary to avoid issues with
# 	@# pthreads on windows
# 	$(WCC32) --static $(CFLAGS) $< $(G2MEDEP) -o G2ME32.exe
# 	$(WCC64) --static $(CFLAGS) $< $(G2MEDEP) -o G2ME64.exe
