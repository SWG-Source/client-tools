AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libWeaponExporterToolshared.la
libWeaponExporterToolshared_la_SOURCES=\
WeaponExporterTool.cpp \
WeaponExporterTool.h \
ConfigWeaponExporterTool.cpp \
ConfigWeaponExporterTool.h \
FirstWeaponExporterTool.cpp \
FirstWeaponExporterTool.h
libWeaponExporterToolshared_la_CXXFLAGS=-I$(srcdir)/../$(platform) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
        -I$(fileInterface_dir)/include/public \
	-I$(engine_dir)/server/library/serverGame/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedGame/include/public \
	-I$(engine_dir)/shared/library/sharedIoWin/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedRegex/include/public \
	-I$(engine_dir)/shared/library/sharedTemplate/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(engine_dir)/shared/library/sharedXml/include/public \
	-I$(engine_dir)/shared/library/sharedTemplateDefinition/include/public \
	-I$(stlport_dir)/stlport \
	-I$(perforce_dir)/include \
	-I$(pcre_dir)/include \
	-I$(boost_dir) \
	-Wno-ctor-dtor-privacy

include $(engine_dir)/../make_include/engine.include
