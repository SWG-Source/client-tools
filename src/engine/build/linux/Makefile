###############################################################################
#
# Master Makefile for Engine
#
# This file invokes the makefiles for specific engine projects as needed.
#
# Organization:
#   Targets
#   Definitions and includes
#   Application build rules (all applications build required libs implicitly)
#   Special targets (e.g. clean, publish, dependcheck)
#
# Typically used targets:
#   make engine_debug         Builds the debug version of all libs/applications
#   make engine_release       Builds the release version of all libs/applications
#   make noGameServer_<debug/release>   Builds everything except the SwgGameServer and
#					SwgDatabaseServer procs. (libgameServer.a is 
#					enormous, so excluding it is sometimes a good
#					idea, if you do not really need to rebuild it.)
#   make <servername>  Builds a server process and any libraries needed by that process.
#                      e.g.  make GameServer_d
###############################################################################

noGameServer_debug: 	CentralServer_d 	\
			ConnectionServer_d 	\
			LoginServer_d 		\
			ServerConsole_d 	\
			PlanetServer_d 		\
			TaskManager_d 		\
			ChatServer_d 		\
			LogServer_d 		\
			MetricsServer_d 	\
			LoginPing_d 		\
			CustomerServiceServer_d	\
			CommoditiesServer_d 	\

noGameServer_release: 	CentralServer_r 	\
			ConnectionServer_r 	\
			LoginServer_r 		\
			ServerConsole_r 	\
			PlanetServer_r 		\
			TaskManager_r 		\
			ChatServer_r 		\
			LogServer_r 		\
			MetricsServer_r 	\
			LoginPing_r 		\
			CustomerServiceServer_r	\
			CommoditiesServer_r 	\

engine_debug:		noGameServer_debug	\

engine_release: 	noGameServer_release	\
			SwgGameServer_r		\
			SwgDatabaseServer_r 	\

###############################################################################
# Definitions and includes
###############################################################################

SRCDIR         =../../..
SERVER_ENGINE_DIR =$(SRCDIR)/engine/server/application
SHARED_ENGINE_DIR =$(SRCDIR)/engine/shared/application

SERVER_GAME_DIR =$(SRCDIR)/game/server/application

###############################################################################
# Rules to invoke the makefiles for specific applications
###############################################################################

debug_libs:
	+@make -C $(SERVER_ENGINE_DIR)/PhonyApp/build/linux debug

CentralServer_d:
	+@make -C $(SERVER_ENGINE_DIR)/CentralServer/build/linux debug

CentralServer_r:
	+@make -C $(SERVER_ENGINE_DIR)/CentralServer/build/linux release

ConnectionServer_d:
	+@make -C $(SERVER_ENGINE_DIR)/ConnectionServer/build/linux debug

ConnectionServer_r:
	+@make -C $(SERVER_ENGINE_DIR)/ConnectionServer/build/linux release

LoginServer_d:
	+@make -C $(SERVER_ENGINE_DIR)/LoginServer/build/linux debug

LoginServer_r:
	+@make -C $(SERVER_ENGINE_DIR)/LoginServer/build/linux release

ServerConsole_d:
	+@make -C $(SERVER_ENGINE_DIR)/ServerConsole/build/linux debug

ServerConsole_r:
	+@make -C $(SERVER_ENGINE_DIR)/ServerConsole/build/linux release

PlanetServer_d:
	+@make -C $(SERVER_ENGINE_DIR)/PlanetServer/build/linux debug

PlanetServer_r:
	+@make -C $(SERVER_ENGINE_DIR)/PlanetServer/build/linux release

TaskManager_d:
	+@make -C $(SERVER_ENGINE_DIR)/TaskManager/build/linux debug

TaskManager_r:
	+@make -C $(SERVER_ENGINE_DIR)/TaskManager/build/linux release

ChatServer_d: 
	+@make -C $(SERVER_ENGINE_DIR)/ChatServer/build/linux debug

ChatServer_r: 
	+@make -C $(SERVER_ENGINE_DIR)/ChatServer/build/linux release

CustomerServiceServer_d: 
	+@make -C $(SERVER_ENGINE_DIR)/CustomerServiceServer/build/linux debug

CustomerServiceServer_r: 
	+@make -C $(SERVER_ENGINE_DIR)/CustomerServiceServer/build/linux release

LoginPing_d: 
	+@make -C $(SERVER_ENGINE_DIR)/LoginPing/build/linux debug

LoginPing_r: 
	+@make -C $(SERVER_ENGINE_DIR)/LoginPing/build/linux release

MetricsServer_d: 
	+@make -C $(SERVER_ENGINE_DIR)/MetricsServer/build/linux debug

MetricsServer_r: 
	+@make -C $(SERVER_ENGINE_DIR)/MetricsServer/build/linux release

LogServer_d:
	+@make -C $(SERVER_ENGINE_DIR)/LogServer/build/linux debug

LogServer_r:
	+@make -C $(SERVER_ENGINE_DIR)/LogServer/build/linux release

TemplateCompiler_d:
	+@make -C $(SHARED_ENGINE_DIR)/TemplateCompiler/build/linux debug

TemplateCompiler_r:
	+@make -C $(SHARED_ENGINE_DIR)/TemplateCompiler/build/linux release

DataTableTool_d:
	+@make -C $(SHARED_ENGINE_DIR)/DataTableTool/build/linux debug

DataTableTool_r:
	+@make -C $(SHARED_ENGINE_DIR)/DataTableTool/build/linux release

GameServer_d:
	+@make -C $(SERVER_ENGINE_DIR)/GameServer/build/linux debug

GameServer_r:
	+@make -C $(SERVER_ENGINE_DIR)/GameServer/build/linux release

CommoditiesServer_d:
	@echo " BOGUS CommoditiesServer_d rule, ignoring..."
#	[ ! -f $(SRCDIR)/../dev/linux/CommoditiesServer_d ] || \
#		chmod u+w $(SRCDIR)/../dev/linux/CommoditiesServer_d
#	cp $(SRCDIR)/../exe/linux/CommoditiesServer_d $(SRCDIR)/../dev/linux	

CommoditiesServer_r:
	[ ! -f $(SRCDIR)/../dev/linux/CommoditiesServer_r ] || \
		chmod u+w $(SRCDIR)/../dev/linux/CommoditiesServer_r
	cp $(SRCDIR)/../exe/linux/CommoditiesServer_r $(SRCDIR)/../dev/linux	

###############################################################################
# Special targets
###############################################################################

clean:
	+@make -C $(SERVER_ENGINE_DIR)/LoginServer/build/linux 		clean
	+@make -C $(SERVER_ENGINE_DIR)/ServerConsole/build/linux 		clean
	+@make -C $(SERVER_ENGINE_DIR)/CentralServer/build/linux 		clean
	+@make -C $(SERVER_GAME_DIR)/SwgGameServer/build/linux 		clean
	+@make -C $(SERVER_GAME_DIR)/SwgDatabaseServer/build/linux 		clean
	+@make -C $(SERVER_ENGINE_DIR)/ConnectionServer/build/linux 	clean	 
	+@make -C $(SERVER_ENGINE_DIR)/TaskManager/build/linux 		clean	 
	+@make -C $(SERVER_ENGINE_DIR)/PlanetServer/build/linux 		clean
	+@make -C $(SERVER_ENGINE_DIR)/ChatServer/build/linux 		clean
	+@make -C $(SERVER_ENGINE_DIR)/CustomerServiceServer/build/linux 	clean
	+@make -C $(SERVER_ENGINE_DIR)/LogServer/build/linux 		clean
	+@make -C $(SERVER_ENGINE_DIR)/MetricsServer/build/linux 		clean
	+@make -C $(SERVER_ENGINE_DIR)/LoginPing/build/linux 		clean

lint:
	+@make -C $(SERVER_ENGINE_DIR)/LoginServer/build/linux 		lint
	+@make -C $(SERVER_ENGINE_DIR)/ServerConsole/build/linux 		lint
	+@make -C $(SERVER_ENGINE_DIR)/CentralServer/build/linux 		lint
	+@make -C $(SERVER_ENGINE_DIR)/SwgGameServer/build/linux 		lint
	+@make -C $(SERVER_ENGINE_DIR)/SwgDatabaseServer/build/linux 		lint
	+@make -C $(SERVER_ENGINE_DIR)/ConnectionServer/build/linux 	lint
	+@make -C $(SERVER_ENGINE_DIR)/TaskManager/build/linux 		lint
	+@make -C $(SERVER_ENGINE_DIR)/PlanetServer/build/linux 		lint
	+@make -C $(SERVER_ENGINE_DIR)/ChatServer/build/linux 		lint
	+@make -C $(SERVER_ENGINE_DIR)/LogServer/build/linux 		lint

