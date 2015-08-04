AUTOMAKE_OPTIONS=foreign
SUBDIRS=combat \
	console \
	controller \
	core \
	lint \
	object \
	objectTemplate \
	snapshot \
	.

noinst_LTLIBRARIES=libSwgGameServershared.la
libSwgGameServershared_la_SOURCES=core/FirstSwgDatabaseServer.h
libSwgGameServershared_la_LIBADD=combat/libSwgGameServersharedcombat.la \
	console/libSwgGameServersharedconsole.la \
	controller/libSwgGameServersharedcontroller.la \
	core/libSwgGameServersharedcore.la \
	lint/libSwgGameServersharedlint.la \
	object/libSwgGameServersharedobject.la \
	objectTemplate/libSwgGameServersharedobjectTemplate.la \
	snapshot/libSwgGameServersharedsnapshot.la
