AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libcrypto.la
libcrypto_la_LIBADD=src/shared/original/libcryptosharedoriginal.la \
	src/shared/wrapper/libcryptosharedwrapper.la
libcrypto_la_SOURCES=src/shared/FirstLocalizationArchive.h
libcrypto_la_LDFLAGS=-avoid-version
