
#------------------------------------------------------------------------------
#   Build information
#------------------------------------------------------------------------------
PREFIX:=/usr/local
MANPREFIX:=/usr/share/man
# `-W`, `-D`, `-std`, etc.
WFLAGS:=-Wall -Werror=implicit-function-declaration -Wimplicit-fallthrough
STDFLAG:=-std=gnu99
DFLAGS:=
CFLAGS:=-O2
# Compiler and linker
CC:=gcc

# The root directory for this build
DIR_ROOT:=Release

#------------------------------------------------------------------------------
#   Information generated in reference to the preface makefile
#------------------------------------------------------------------------------
DIR_BIN:=$(DIR_ROOT)
DIR_OBJS:=$(DIR_ROOT)
DIR_DEPS:=$(DIR_ROOT)
OBJS:=${SRCS:%.c=$(DIR_OBJS)/%.o}
OBJS_E=$(wildcard $(OBJS))
DEPS:=${SRCS:%.c=$(DIR_DEPS)/%.d}
DEPS_E=$(wildcard $(DEPS))

#------------------------------------------------------------------------------
#   Directories
#------------------------------------------------------------------------------
ifndef __DIRECTORIES_RELEASE__
	__DIRECTORIES_RELEASE__ = 1

.PHONY: directory_release_root
directory_release_root: DIR_ROOT:=$(DIR_ROOT)
directory_release_root:
	@if [ ! -d $(DIR_ROOT) ]; then \
		mkdir -p $(DIR_ROOT); \
	fi

.PHONY: directory_release_bin
directory_release_bin: DIR_BIN:=$(DIR_BIN)
directory_release_bin: | directory_release_root
	@if [ ! -d $(DIR_BIN) ]; then \
		mkdir -p $(DIR_BIN); \
	fi

.PHONY: directory_release_objs
directory_release_objs: DIR_OBJS:=$(DIR_OBJS)
directory_release_objs: | directory_release_root
	@if [ ! -d $(DIR_OBJS) ]; then \
		mkdir -p $(DIR_OBJS); \
	fi

.PHONY: directory_release_deps
directory_release_deps: DIR_DEPS:=$(DIR_DEPS)
directory_release_deps: | directory_release_root
	@if [ ! -d $(DIR_DEPS) ]; then \
		mkdir -p $(DIR_DEPS); \
	fi

endif

#------------------------------------------------------------------------------
#   Set the default goal/target
#------------------------------------------------------------------------------
.DEFAULT_GOAL:=$(NAME)_release

#------------------------------------------------------------------------------
#   Building
#------------------------------------------------------------------------------
.PHONY: $(NAME)_release
$(NAME)_release: $(DIR_BIN)/$(NAME)

# "Bake" all the variables into the recipe so that if other makefiles set a
# variable of the same name it won't affect this recipe.
$(DIR_BIN)/$(NAME): CC:=$(CC)
$(DIR_BIN)/$(NAME): INCS:=$(INCS)
$(DIR_BIN)/$(NAME): DFLAGS:=$(DFLAGS)
$(DIR_BIN)/$(NAME): STDFLAG:=$(STDFLAG)
$(DIR_BIN)/$(NAME): WFLAGS:=$(WFLAGS)
$(DIR_BIN)/$(NAME): CFLAGS:=$(CFLAGS)
$(DIR_BIN)/$(NAME): LIBRARIES:=$(LIBRARIES)
$(DIR_BIN)/$(NAME): LIBS:=$(LIBS)
$(DIR_BIN)/$(NAME): $(OBJS) | directory_release_bin
	$(CC) $(INCS) $(DFLAGS) $(STDFLAG) $(WFLAGS) $(CFLAGS) $(LIBRARIES) $^ $(LIBS) -o $@

# "Bake" all the variables into the recipe so that if other makefiles set a
# variable of the same name it won't affect this recipe.
$(DIR_OBJS)/%.o: CC:=$(CC)
$(DIR_OBJS)/%.o: INCS:=$(INCS)
$(DIR_OBJS)/%.o: DFLAGS:=$(DFLAGS)
$(DIR_OBJS)/%.o: STDFLAG:=$(STDFLAG)
$(DIR_OBJS)/%.o: WFLAGS:=$(WFLAGS)
$(DIR_OBJS)/%.o: CFLAGS:=$(CFLAGS)
$(DIR_OBJS)/%.o: %.c | directory_release_objs
	$(CC) $(INCS) $(DFLAGS) $(STDFLAG) $(WFLAGS) $(CFLAGS) $< -c -o $@

# "Bake" all the variables into the recipe so that if other makefiles set a
# variable of the same name it won't affect this recipe.
$(DIR_DEPS)/%.d: CC:=$(CC)
$(DIR_DEPS)/%.d: INCS:=$(INCS)
$(DIR_DEPS)/%.d: DFLAGS:=$(DFLAGS)
$(DIR_DEPS)/%.d: STDFLAG:=$(STDFLAG)
$(DIR_DEPS)/%.d: CFLAGS:=$(CFLAGS)
# `-MM`: Do not mention system header files in dependencies.
# `-MF`: Output dependency file to file pointed to by the filepath given to
#        this flag.
# `-MT`: The target of the dependency should be the text given to this flag. We
#        have to use `-MT` to manually set the target of our automatic
#        dependency rules because our `.o` files will not be in the same
#        directory that we run `make` in. If we don't set the target, then the
#        dependency rules will be for `.o` files that our build never builds.
#        `$(@:%.d=%.o)` is a text substitution on the `$@` automatic variable.
$(DIR_DEPS)/%.d: %.c | directory_release_deps
	$(CC) $(INCS) $(DFLAGS) $(STDFLAG) $(CFLAGS) $< -MM -MT $(@:%.d=%.o) -MF $@

# TODO: How does this work in this makefile setup? Is this necessary? Can it
# be improved? What we really want is to only include the dependency files
# if `$(NAME)_release_clean` will not be called.
#
# Include all the dependency files IF the commandline goal given to Make was
# not `clean`
ifneq ($(MAKECMDGOALS),clean)
include $(DEPS)
endif

#------------------------------------------------------------------------------
#   Installing and uninstalling
#------------------------------------------------------------------------------
.PHONY: $(NAME)_release_install
$(NAME)_release_install: PREFIX:=$(PREFIX)
$(NAME)_release_install: DIR_BIN:=$(DIR_BIN)
$(NAME)_release_install: NAME:=$(NAME)
$(NAME)_release_install: $(NAME)
	mkdir -p $(PREFIX)/bin && \
	cp -f $(DIR_BIN)/$(NAME) $(PREFIX)/bin/$(NAME) && \
	chmod 755 $(PREFIX)/bin/$(NAME)

.PHONY: $(NAME)_release_uninstall
$(NAME)_release_uninstall: PREFIX:=$(PREFIX)
$(NAME)_release_uninstall: NAME:=$(NAME)
$(NAME)_release_uninstall:
	rm -f $(PREFIX)/bin/$(NAME)

#------------------------------------------------------------------------------
#   Cleaning
#------------------------------------------------------------------------------
.PHONY: $(NAME)_release_clean
$(NAME)_release_clean: OBJS_E:=$(OBJS_E)
$(NAME)_release_clean: NAME:=$(NAME)
$(NAME)_release_clean: DEPS_E:=$(DEPS_E)
$(NAME)_release_clean:
ifneq ($(strip $(OBJS_E)),)
	rm -f $(OBJS_E)
else
	$(info No $(NAME) .o files to remove)
endif
ifneq ($(strip $(DEPS_E)),)
	rm -f $(DEPS_E)
else
	$(info No $(NAME) .d files to remove)
endif
