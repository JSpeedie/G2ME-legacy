#   Build information
#------------------------------------------------------------------------------
# `-W`, `-D`, `-std`, etc.
WFLAGS:=-Wall -Werror=implicit-function-declaration -Wimplicit-fallthrough
STDFLAG:=-std=gnu99
DFLAGS:=
CFLAGS:=-O2
# Compiler and linker
# This makefile makes use of the `mingw-w64-gcc` compilers. They can be
# installed with:
#
#     sudo pacman -S mingw-w64-gcc      # Arch
#     sudo apt install gcc-mingw-w64    # Debian (Confirmed 2026-06-14)
#
WCC32:=i686-w64-mingw32-gcc
WCC64:=x86_64-w64-mingw32-gcc

# The root directory for this build
DIR_ROOT:=Windows

#------------------------------------------------------------------------------
#   Information generated in reference to the preface makefile
#------------------------------------------------------------------------------
DIR_BIN:=$(DIR_ROOT)

#------------------------------------------------------------------------------
#   Directories
#------------------------------------------------------------------------------
ifndef __DIRECTORIES_WINDOWS_RELEASE__
	__DIRECTORIES_WINDOWS_RELEASE__ = 1

.PHONY: directory_windows_release_root
directory_windows_release_root: DIR_ROOT:=$(DIR_ROOT)
directory_windows_release_root:
	@if [ ! -d $(DIR_ROOT) ]; then \
		mkdir -p $(DIR_ROOT); \
	fi

.PHONY: directory_windows_release_bin
directory_windows_release_bin: DIR_BIN:=$(DIR_BIN)
directory_windows_release_bin: | directory_windows_release_root
	@if [ ! -d $(DIR_BIN) ]; then \
		mkdir -p $(DIR_BIN); \
	fi

endif

#------------------------------------------------------------------------------
#   Set the default goal/target
#------------------------------------------------------------------------------
.DEFAULT_GOAL:=$(NAME)_windows_release

#------------------------------------------------------------------------------
#   Building
#------------------------------------------------------------------------------
.PHONY: $(NAME)_windows_release
$(NAME)_windows_release: $(DIR_BIN)/$(NAME)

# "Bake" all the variables into the recipe so that if other makefiles set a
# variable of the same name it won't affect this recipe.
$(DIR_BIN)/$(NAME): WCC32:=$(WCC32)
$(DIR_BIN)/$(NAME): WCC64:=$(WCC64)
# TODO: You don't use INCS to LIBS in your recipe
$(DIR_BIN)/$(NAME): INCS:=$(INCS)
$(DIR_BIN)/$(NAME): DFLAGS:=$(DFLAGS)
$(DIR_BIN)/$(NAME): STDFLAG:=$(STDFLAG)
$(DIR_BIN)/$(NAME): WFLAGS:=$(WFLAGS)
$(DIR_BIN)/$(NAME): CFLAGS:=$(CFLAGS)
$(DIR_BIN)/$(NAME): LIBRARIES:=$(LIBRARIES)
$(DIR_BIN)/$(NAME): LIBS:=$(LIBS)
# `--static` to create a statically linked binary to avoid issues with pthreads
# on windows
$(DIR_BIN)/$(NAME): $(SRCS) | directory_windows_release_bin
	$(WCC32) --static $(CFLAGS) $^ -o $(DIR_BIN)/G2ME32.exe
	$(WCC64) --static $(CFLAGS) $^ -o $(DIR_BIN)/G2ME64.exe
