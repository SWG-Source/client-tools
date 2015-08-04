AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedGame.la
libsharedGame_la_LIBADD=src/shared/libsharedGameshared.la \
	src/shared/appearance/libsharedGamesharedappearance.la \
	src/shared/collision/libsharedGamesharedcollision.la \
	src/shared/core/libsharedGamesharedcore.la \
	src/shared/combat/libsharedGamesharedcombat.la \
	src/shared/command/libsharedGamesharedcommand.la \
	src/shared/dynamics/libsharedGameshareddynamics.la \
	src/shared/mount/libsharedGamesharedmount.la \
	src/shared/object/libsharedGamesharedobject.la \
	src/shared/objectTemplate/libsharedGamesharedobjectTemplate.la \
	src/shared/space/libsharedGamesharedspace.la \
	src/shared/sui/libsharedGamesharedsui.la \
	src/shared/travel/libsharedGamesharedtravel.la
libsharedGame_la_SOURCES=src/shared/FirstSharedGame.h
libsharedGame_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
