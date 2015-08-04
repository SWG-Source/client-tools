AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libarchiveshared.la
libarchiveshared_la_CXXFLAGS=-I$(archive_dir)/include \
	-I$(stlport_dir)/stlport \
	-Wno-ctor-dtor-privacy
libarchiveshared_la_SOURCES=Archive.h \
	AutoByteStream.cpp \
	AutoByteStream.h \
	AutoDeltaByteStream.cpp \
	AutoDeltaByteStream.h \
	AutoDeltaMap.h \
	AutoDeltaPackedMap.cpp \
	AutoDeltaPackedMap.h \
	AutoDeltaObserverOps.h \
	AutoDeltaQueue.h \
	AutoDeltaVariableCallback.h \
	AutoDeltaVariableObserver.h \
	AutoDeltaVector.h \
	AutoDeltaVectorObserver.h \
	ByteStream.cpp \
	ByteStream.h \
	FirstArchive.h

