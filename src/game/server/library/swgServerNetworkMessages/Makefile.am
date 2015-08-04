AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libswgServerNetworkMessages.la
libswgServerNetworkMessages_la_LIBADD=src/shared/combat/libswgServerNetworkMessagessharedcombat.la \
	src/shared/core/libswgServerNetworkMessagessharedcore.la \
	src/shared/jedi/libswgServerNetworkMessagessharedjedi.la \
	src/shared/money/libswgServerNetworkMessagessharedmoney.la \
	src/shared/resource/libswgServerNetworkMessagessharedresource.la \
	src/shared/travel/libswgServerNetworkMessagessharedtravel.la
libswgServerNetworkMessages_la_SOURCES=src/shared/FirstSwgServerNetworkMessages.h
libswgServerNetworkMessages_la_LDFLAGS=-avoid-version
