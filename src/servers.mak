# makefile for the servers
.phony:	dependcheck

all: dependcheck
	+@make -C engine/server/application/LoginServer/build/linux
	+@make -C engine/server/application/CentralServer/build/linux
	+@make -C game/server/application/SwgGameServer/build/linux
	+@make -C game/server/application/SwgDatabaseServer/build/linux
	+@make -C engine/server/application/ConnectionServer/build/linux	
#	+@make -C engine/server/application/TaskManager/build/linux	
	+@make -C engine/server/application/PlanetServer/build/linux

debug: dependcheck
	+@make -C engine/server/application/LoginServer/build/linux debug
	+@make -C engine/server/application/CentralServer/build/linux debug 
	+@make -C game/server/application/SwgGameServer/build/linux debug
	+@make -C game/server/application/SwgDatabaseServer/build/linux debug
	+@make -C engine/server/application/ConnectionServer/build/linux debug	 
#	+@make -C engine/server/application/TaskManager/build/linux debug	 
	+@make -C engine/server/application/PlanetServer/build/linux debug

release: dependcheck
	+@make -C engine/server/application/LoginServer/build/linux release
	+@make -C engine/server/application/CentralServer/build/linux release 
	+@make -C game/server/application/SwgGameServer/build/linux release
	+@make -C game/server/application/SwgDatabaseServer/build/linux release
	+@make -C engine/server/application/ConnectionServer/build/linux release	 
#	+@make -C engine/server/application/TaskManager/build/linux release	 
	+@make -C engine/server/application/PlanetServer/build/linux release

lint:
	+@make -C engine/server/application/LoginServer/build/linux lint
	+@make -C engine/server/application/CentralServer/build/linux lint
	+@make -C game/server/application/SwgGameServer/build/linux lint
	+@make -C game/server/application/SwgDatabaseServer/build/linux lint
	+@make -C engine/server/application/ConnectionServer/build/linux lint
#	+@make -C engine/server/application/TaskManager/build/linux lint
	+@make -C engine/server/application/PlanetServer/build/linux lint

publish:
	+@make -C engine/server/application/LoginServer/build/linux publish
	+@make -C engine/server/application/CentralServer/build/linux publish
	+@make -C game/server/application/SwgGameServer/build/linux publish
	+@make -C game/server/application/SwgDatabaseServer/build/linux publish
	+@make -C engine/server/application/ConnectionServer/build/linux publish
#	+@make -C engine/server/application/TaskManager/build/linux publish
	+@make -C engine/server/application/PlanetServer/build/linux publish

depend:
	+@make -C engine/server/application/LoginServer/build/linux depend
	+@make -C engine/server/application/CentralServer/build/linux depend
	+@make -C game/server/application/SwgGameServer/build/linux depend
	+@make -C game/server/application/SwgDatabaseServer/build/linux depend
	+@make -C engine/server/application/ConnectionServer/build/linux depend
#	+@make -C engine/server/application/TaskManager/build/linux depend
	+@make -C engine/server/application/PlanetServer/build/linux depend

clean:
	+@make -C engine/server/application/LoginServer/build/linux clean
	+@make -C engine/server/application/CentralServer/build/linux clean 
	+@make -C game/server/application/SwgGameServer/build/linux clean
	+@make -C game/server/application/SwgDatabaseServer/build/linux clean
	+@make -C engine/server/application/ConnectionServer/build/linux clean	 
#	+@make -C engine/server/application/TaskManager/build/linux clean	 
	+@make -C engine/server/application/PlanetServer/build/linux clean

dependcheck: .cppsums
	-@mv .cppsums .cppsums~
	@find . -name "*.cpp" | sum > .cppsums
	@if ! /usr/bin/diff .cppsums .cppsums~ > /dev/null; then find . -name .depend -exec rm {} \; ; fi;
	@rm .cppsums~

.cppsums:
	@find . -name .depend -exec rm {} \;
	@find . -name "*.cpp" | sum > .cppsums
	
cleanall:
	@rm -f `find . -name .depend`
	@find . -type d -name compile -exec rm -rf {} \;

cleandepend:
	@find . -name .depend -exec rm {} \;
