AUTOMAKE_OPTIONS=foreign

INCLUDES = -I$(top_srcdir)/external/3rd/library/platform \
	-I$(platform_dir)/utils/Base/
METASOURCES = AUTO
libAuctionTransferGameAPI_la_LDFLAGS = -avoid-version
noinst_LTLIBRARIES = libAuctionTransferGameAPI.la
libAuctionTransferGameAPI_la_SOURCES = AuctionTransferAPICore.cpp\
	AuctionTransferAPICore.h AuctionTransferAPI.cpp AuctionTransferAPI.h\
	AuctionTransferEnum.h Character.cpp Character.h Request.cpp Request.h\
	Response.cpp Response.h
AM_CXXFLAGS = -DEXTERNAL_DISTRO -DNAMESPACE=AuctionTransfer -DUSE_TCP_LIBRARY
SUBDIRS = zip Base TcpLibrary ATGenericAPI
