AUTOMAKE_OPTIONS=foreign

INCLUDES = \
	-I$(top_srcdir)/server/application/CentralServer/src/shared/AuctionTransferGameAPI \
	-I$(top_srcdir)/external/3rd/library/platform -I.. \
	-I$(platform_dir)/utils/Base/
METASOURCES = AUTO
libAuctionTransferGameAPIATGenericAPI_la_LDFLAGS = -avoid-version
noinst_LTLIBRARIES = libAuctionTransferGameAPIATGenericAPI.la
libAuctionTransferGameAPIATGenericAPI_la_SOURCES = GenericApiCore.cpp GenericApiCore.h\
	GenericConnection.cpp GenericConnection.h GenericMessage.cpp\
	GenericMessage.h
AM_CXXFLAGS = -DEXTERNAL_DISTRO -DNAMESPACE=AuctionTransfer -DUSE_TCP_LIBRARY
