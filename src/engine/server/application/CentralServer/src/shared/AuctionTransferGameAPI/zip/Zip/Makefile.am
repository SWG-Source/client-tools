INCLUDES = -I$(top_srcdir)/external/3rd/library/platform \
	-I$(platform_dir)/utils/Base/
METASOURCES = AUTO
libAuctionTransferGameAPIZip_la_LDFLAGS = -avoid-version
lib_LTLIBRARIES = libAuctionTransferGameAPIZip.la
libAuctionTransferGameAPIZip_la_SOURCES = adler32.c compress.c crc32.c deflate.c deflate.h gzio.c\
	infblock.c infblock.h infcodes.c infcodes.h inffast.c inffast.h\
	inffixed.h inflate.c inftrees.c inftrees.h infutil.c infutil.h\
	maketree.c trees.c trees.h uncompr.c zconf.h zlib.h zutil.c zutil.h

AM_CXXFLAGS = -DEXTERNAL_DISTRO -DNAMESPACE=AuctionTransfer -DUSE_TCP_LIBRARY
AM_CFLAGS = -DEXTERNAL_DISTRO -DNAMESPACE=AuctionTransfer -DUSE_TCP_LIBRARY
