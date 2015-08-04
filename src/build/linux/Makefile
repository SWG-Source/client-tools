###############################################################################
#
# Master Makefile for Engine
#
# This file invokes the makefiles for specific projects as needed.
#
# Typically used targets:
#   make debug         Builds the debug version of all applications
#   make release       Builds the release version of all applications
#   make publish       Does the same thing as "make debug", then copies the
#                      newly-build executables to the exe/linux directory
###############################################################################
###############################################################################
# Definitions and includes
###############################################################################

SRCDIR          =../..
ENGINE_MAKE_DIR =$(SRCDIR)/engine/build/linux
GAME_MAKE_DIR   =$(SRCDIR)/game/build/linux

###############################################################################
# Rules to invoke the makefiles for specific applications
###############################################################################

debug_engine:
	+@make -C $(ENGINE_MAKE_DIR) 	engine_debug

debug_game:
	+@make -C $(GAME_MAKE_DIR) 	game_debug

debug:
	+@make -C $(GAME_MAKE_DIR) 	debug_libs
	+@make	debug_engine
	+@make	debug_game

###############################################################################
# Special targets
###############################################################################

clean_engine:
	+@make -C $(ENGINE_MAKE_DIR) clean

clean_game:
	+@make -C $(GAME_MAKE_DIR) clean

clean: 	clean_engine clean_game


lint_engine:
	+@make -C $(ENGINE_MAKE_DIR) lint

lint_game:
	+@make -C $(GAME_MAKE_DIR) lint

lint: 	lint_engine lint_game

###############################################################################
# More Special targets
###############################################################################

cleanall:
	@find $(SRCDIR) -name .depend -exec rm {} \;
	@find $(SRCDIR) -name "*.d" -exec rm -f {} \;
	@find $(SRCDIR) -type d -name compile -depth -exec rm -rf {} \;
	@find $(SRCDIR) -name "*.o" -exec rm -f {} \;
	@find $(SRCDIR) -name "*.a" | grep -v libz.a | grep -v libpcre.a | xargs rm -f;
# TODO So the above is pretty hacky, we stillhave these two lame .a files checked into p4.

cleandepend:
	@find $(SRCDIR) -name .depend -exec rm {} \;

#dependcheck: .cppsums
#	-@mv .cppsums .cppsums~
#	@find $(SRCDIR) -name "*.cpp" | sum > .cppsums
#	@if ! /usr/bin/diff .cppsums .cppsums~ > /dev/null; then find $(SRCDIR) -name .depend -exec rm {} \; ; fi;
#	@rm .cppsums~

#.cppsums:
#	@find $(SRCDIR) -name .depend -exec rm {} \;
#	@find $(SRCDIR) -name "*.cpp" | sum > .cppsums

