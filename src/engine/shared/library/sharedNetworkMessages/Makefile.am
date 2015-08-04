AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedNetworkMessages.la
libsharedNetworkMessages_la_LIBADD=src/shared/libsharedNetworkMessagesshared.la \
	src/shared/chat/libsharedNetworkMessagessharedchat.la \
	src/shared/clientGameServer/libsharedNetworkMessagessharedclientGameServer.la \
	src/shared/clientLoginServer/libsharedNetworkMessagessharedclientLoginServer.la \
	src/shared/common/libsharedNetworkMessagessharedcommon.la \
	src/shared/core/libsharedNetworkMessagessharedcore.la \
	src/shared/customerService/libsharedNetworkMessagessharedcustomerService.la \
	src/shared/planetWatch/libsharedNetworkMessagessharedplanetWatch.la \
	src/shared/voicechat/libsharedNetworkMessagessharedvoicechat.la

libsharedNetworkMessages_la_SOURCES=src/shared/FirstSharedNetworkMessages.h
libsharedNetworkMessages_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
