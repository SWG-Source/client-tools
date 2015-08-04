AUTOMAKE_OPTIONS=foreign
SUBDIRS = src .
lib_LTLIBRARIES=libarchive.la
libarchive_la_LIBADD=src/shared/libarchiveshared.la src/$(platform)/libarchive$(platform).la
libarchive_la_SOURCES=src/shared/FirstArchive.h

src/shared/libarchiveshared.la:
	make -C src/shared

src/$(platform)/libarchive$(platform).la:
	make -C src/$(platform)

libarchive_la_LDFLAGS=-avoid-version
