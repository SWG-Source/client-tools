AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedNetworklinux.la
libsharedNetworklinux_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(srcdir)/../../src/shared \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-Wno-ctor-dtor-privacy
libsharedNetworklinux_la_SOURCES=Address.cpp \
	NetworkGetHostName.cpp \
	Sock.cpp \
	Sock.h \
	TcpClient.cpp \
	TcpClient.h \
	TcpServer.cpp \
	TcpServer.h \
	UdpSock.cpp

include $(engine_dir)/../make_include/engine.include
