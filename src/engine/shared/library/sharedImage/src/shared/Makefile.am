AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedImageshared.la
libsharedImageshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedImageshared_la_SOURCES=FirstSharedImage.h \
	ConfigSharedImage.cpp \
	ConfigSharedImage.h \
	Image.cpp \
	ImageFormat.cpp \
	ImageFormat.h \
	ImageFormatList.cpp \
	ImageFormatList.h \
	Image.h \
	ImageManipulation.cpp \
	ImageManipulation.h \
	SetupSharedImage.cpp \
	SetupSharedImage.h \
	TargaFormat.cpp \
	TargaFormat.h

include $(engine_dir)/../make_include/engine.include
