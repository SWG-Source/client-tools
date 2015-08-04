AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libsharedDatabaseInterfacesharedcore.la
libsharedDatabaseInterfacesharedcore_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDatabaseInterface/src_oci \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-I$(unicodeArchive_dir)/include/public \
	-Wno-ctor-dtor-privacy \
	-DDBLIBRARY_OCI

libsharedDatabaseInterfacesharedcore_la_SOURCES=Bindable.cpp \
	Bindable.h \
	BindableImplementation.h \
	BindableNetworkId.cpp \
	BindableNetworkId.h \
	BufferString.cpp \
	BufferString.h \
	DbBatchQuery.h \
	DbBindableBase.cpp \
	DbBindableBase.h \
	DbBindableBitArray.cpp \
	DbBindableBitArray.h \
	DbBindableBool.cpp \
	DbBindableBool.h \
	DbBindableDouble.cpp \
	DbBindableDouble.h \
	DbBindableInt64.cpp \
	DbBindableInt64.h \
	DbBindableLong.cpp \
	DbBindableLong.h \
	DbBindableString.cpp \
	DbBindableString.h \
	DbBindableUnicode.cpp \
	DbBindableUnicode.h \
	DbBufferRow.cpp \
	DbBufferRow.h \
	DbException.cpp \
	DbException.h \
	DbModeQuery.cpp \
	DbModeQuery.h \
	DbProfiler.cpp \
	DbProfiler.h \
	DbProtocol.def \
	DbQuery.cpp \
	DbQuery.h \
	DbQueryImplementation.h \
	DbRow.cpp \
	DbRow.h \
	DbServer.cpp \
	DbServer.h \
	DbSession.cpp \
	DbSession.h \
	FirstSharedDatabaseInterface.h \
	NullEncodedUnicodeString.cpp \
	NullEncodedUnicodeString.h \
	NullEncodedStandardString.cpp \
	NullEncodedStandardString.h

libsharedDatabaseInterfacesharedcore_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
