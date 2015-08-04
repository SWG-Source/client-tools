AUTOMAKE_OPTIONS=foreign

INCLUDES = -I$(top_srcdir)/external/3rd/library/platform \
	-I$(platform_dir)/utils/Base/
METASOURCES = AUTO
libAuctionTransferGameAPITcpLibrary_la_LDFLAGS = -avoid-version
noinst_LTLIBRARIES = libAuctionTransferGameAPITcpLibrary.la
libAuctionTransferGameAPITcpLibrary_la_SOURCES = Clock.cpp Clock.h IPAddress.cpp IPAddress.h\
	TcpBlockAllocator.cpp TcpBlockAllocator.h TcpConnection.cpp\
	TcpConnection.h TcpHandlers.h TcpManager.cpp TcpManager.h
AM_CXXFLAGS = -DEXTERNAL_DISTRO -DNAMESPACE=AuctionTransfer -DUSE_TCP_LIBRARY
