AUTOMAKE_OPTIONS=foreign

bin_PROGRAMS=LocalizationToolCon

INCLUDES=-I$(stlport_dir)/stlport -I$(fileInterface_dir)/include/public -I$(localization_dir)/include -I$(unicode_dir)/include
LocalizationToolCon_SOURCES=main.cpp
LocalizationToolCon_LDADD=$(stlport_dir)/lib/libstlport_gcc.a \
	$(top_builddir)/external/ours/library/unicode/libunicode.la \
	$(top_builddir)/external/ours/library/localization/liblocalization.la \
	$(top_builddir)/external/ours/library/fileInterface/libfileInterface.la
LocalizationToolCon_LDFLAGS=-lpthread

$(stlport_dir)/lib/libstlport_gcc.a:
	cd $(stlport_dir)/src && $(MAKE) -j1 -f gcc-linux.mak
