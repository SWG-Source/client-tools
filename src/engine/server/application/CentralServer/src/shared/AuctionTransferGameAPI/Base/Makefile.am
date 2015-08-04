AUTOMAKE_OPTIONS=foreign

INCLUDES = -I$(top_srcdir)/external/3rd/library/platform \
	-I$(platform_dir)/utils/Base/
METASOURCES = AUTO
libAuctionTransferGameAPIBase_la_LDFLAGS = -avoid-version
noinst_LTLIBRARIES = libAuctionTransferGameAPIBase.la
libAuctionTransferGameAPIBase_la_SOURCES = Archive.cpp Archive.h Platform.h
SUBDIRS = linux
AM_CXXFLAGS = -DEXTERNAL_DISTRO -DNAMESPACE=AuctionTransfer -DUSE_TCP_LIBRARY
