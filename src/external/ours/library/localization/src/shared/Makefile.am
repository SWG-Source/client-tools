AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=liblocalizationshared.la
liblocalizationshared_la_CXXFLAGS=-I../../include \
	-I$(stlport_dir)/stlport \
	-I$(fileInterface_dir)/include/public \
	-I$(unicode_dir)/include \
	-Wno-ctor-dtor-privacy
liblocalizationshared_la_SOURCES=FirstLocalization.h \
	LocalizationManager.cpp \
	LocalizationManager.h \
	LocalizedString.cpp \
	LocalizedString.h \
	LocalizedStringTable.cpp \
	LocalizedStringTable.h \
	LocalizedStringTableReaderWriter.cpp \
	LocalizedStringTableReaderWriter.h \
	StringId.cpp \
	StringId.h
