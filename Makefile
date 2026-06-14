# G2ME - A C implementation of Glicko-2 with a
# C program to make managing a Glicko-2 system easier.
# © 2026 Julian Speedie

#   ----------------------------------------------------------------------------
#   All binaries and test binaries in the project
#   ----------------------------------------------------------------------------
# All items in `BINARIES` must have an associated `.mk` file that:
# (1) is included by this file,
# (2) includes `template-release.mk` AND `template-debug.mk`,
# (3) has a value for `NAME` that matches an item in `BINARIES`
BINARIES := G2ME

#   ----------------------------------------------------------------------------
#   Categorized lists of all build targets
#   ----------------------------------------------------------------------------
BINARIES_RELEASE := $(BINARIES:%=%_release)
BINARIES_RELEASE_INSTALL := $(BINARIES:%=%_release_install)
BINARIES_RELEASE_UNINSTALL := $(BINARIES:%=%_release_uninstall)
BINARIES_RELEASE_CLEAN := $(BINARIES:%=%_release_clean)
BINARIES_DEBUG := $(BINARIES:%=%_debug)
BINARIES_DEBUG_INSTALL := $(BINARIES:%=%_debug_install)
BINARIES_DEBUG_UNINSTALL := $(BINARIES:%=%_debug_uninstall)
BINARIES_DEBUG_CLEAN := $(BINARIES:%=%_debug_clean)

#   ----------------------------------------------------------------------------
#   The makefiles for each build result
#   ----------------------------------------------------------------------------
include G2ME.mk
include G2ME-server.mk
include G2ME-client.mk

#   ----------------------------------------------------------------------------
#   High-level rules for the developer to invoke from the commandline
#   ----------------------------------------------------------------------------
.DEFAULT_GOAL := release

.PHONY: release
release: $(BINARIES_RELEASE)

.PHONY: release_install
release_install: $(BINARIES_RELEASE_INSTALL)

.PHONY: release_uninstall
release_uninstall: $(BINARIES_RELEASE_UNINSTALL)

.PHONY: release_clean
release_clean: $(BINARIES_RELEASE_clean)

.PHONY: debug
debug: $(BINARIES_DEBUG)

.PHONY: debug_install
debug_install: $(BINARIES_RELEASE_INSTALL)

.PHONY: debug_uninstall
debug_uninstall: $(BINARIES_RELEASE_UNINSTALL)

.PHONY: debug_clean
debug_clean: $(BINARIES_RELEASE_clean)

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
