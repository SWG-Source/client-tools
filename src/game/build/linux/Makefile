###############################################################################
#
# Master Makefile for Game
#
# This file invokes the makefiles for specific game projects as needed.
#
# Organization:
#   Targets
#   Definitions and includes
#   Application build rules (all applications build required libs implicitly)
#   Special targets (e.g. clean, publish, dependcheck)
#
# Typically used targets:
#   make game_debug         Builds the debug version of all libs/applications
#   make game_release       Builds the release version of all libs/applications
#   make <servername>  Builds a server process and any libraries needed by that process.
#                      e.g.  make SwgGameServer_d
###############################################################################

game_debug:	SwgGameServer_d		\
		SwgDatabaseServer_d


###############################################################################
# Definitions and includes
###############################################################################

SRCDIR         =../../..

SERVER_GAME_APP_DIR = $(SRCDIR)/game/server/application
SHARED_GAME_APP_DIR = $(SRCDIR)/game/shared/application

###############################################################################
# Rules to invoke the makefiles for specific applications
###############################################################################

debug_libs:
	+@make -C $(SERVER_GAME_APP_DIR)/PhonyApp/build/linux		debug

SwgGameServer_d:
	+@make -C $(SERVER_GAME_APP_DIR)/SwgGameServer/build/linux 	debug

SwgDatabaseServer_d:
	+@make -C $(SERVER_GAME_APP_DIR)/SwgDatabaseServer/build/linux 	debug


###############################################################################
# Special targets
###############################################################################

clean:
	+@make -C $(SERVER_GAME_APP_DIR)/SwgGameServer/build/linux		clean
	+@make -C $(SERVER_GAME_APP_DIR)/SwgDatabaseServer/build/linux		clean


