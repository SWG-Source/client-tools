AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedCollision.la
libsharedCollision_la_LIBADD=src/shared/core/libsharedCollisionsharedcore.la \
	src/shared/extent/libsharedCollisionsharedextent.la
libsharedCollision_la_SOURCES=src/shared/FirstSharedCollision.h
libsharedCollision_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
