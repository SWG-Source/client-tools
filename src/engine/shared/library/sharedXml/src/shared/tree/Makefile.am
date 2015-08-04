AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedXmlsharedtree.la
libsharedXmlsharedtree_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(srcdir)/../../../include/private \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(boost_dir) \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-I$(localization_dir)/include \
	-I$(unicode_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(libxml_dir)/include \
	-Wno-ctor-dtor-privacy
libsharedXmlsharedtree_la_SOURCES=XmlTreeDocument.cpp \
	XmlTreeDocument.h \
	XmlTreeDocumentList.cpp \
	XmlTreeDocumentList.h \
	XmlTreeNode.cpp \
	XmlTreeNode.h

include $(engine_dir)/../make_include/engine.include
