AUTOMAKE_OPTIONS=foreign

INCLUDES = -I$(top_srcdir)/external/3rd/library/platform \
	-I$(platform_dir)/utils/Base/
METASOURCES = AUTO
noinst_LTLIBRARIES = libAuctionTransferGameAPIlinux.la
libAuctionTransferGameAPIlinux_la_LDFLAGS = -avoid-version
libAuctionTransferGameAPIlinux_la_SOURCES = Archive.h Platform.cpp Platform.h \
	Types.h Mutex.cpp Mutex.h
AM_CXXFLAGS = -DEXTERNAL_DISTRO -DNAMESPACE=AuctionTransfer -DUSE_TCP_LIBRARY
