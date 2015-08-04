AUTOMAKE_OPTIONS=foreign
SUBDIRS=generated \
	buffers \
	core \
	persistable \
	queries \
	tasks \
	cleanup \
	.

noinst_LTLIBRARIES=libSwgDatabaseServershared.la
libSwgDatabaseServershared_la_SOURCES=core/FirstSwgDatabaseServer.h
libSwgDatabaseServershared_la_LIBADD=buffers/libSwgDatabaseServersharedbuffers.la \
	core/libSwgDatabaseServersharedcore.la \
	generated/libSwgDatabaseServersharedgenerated.la \
	persistable/libSwgDatabaseServersharedpersistable.la \
	queries/libSwgDatabaseServersharedqueries.la \
	tasks/libSwgDatabaseServersharedtasks.la \
	cleanup/libSwgDatabaseServersharedcleanup.la
