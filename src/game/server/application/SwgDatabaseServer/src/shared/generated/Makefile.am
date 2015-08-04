AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libSwgDatabaseServersharedgenerated.la
libSwgDatabaseServersharedgenerated_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(srcdir)/../../../include/private \
	-I$(srcdir)/../../../src/$(platform) \
	-I$(srcdir)/../../../include/public/SwgDatabaseServer \
	-I$(engine_dir)/server/library/serverUtility/include/public \
	-I$(engine_dir)/server/library/serverDatabase/include/public \
	-I$(engine_dir)/server/library/serverGame/include/public \
	-I$(engine_dir)/server/library/serverMetrics/include/public \
	-I$(engine_dir)/server/library/serverNetworkMessages/include/public \
	-I$(engine_dir)/server/library/serverPathfinding/include/public \
	-I$(engine_dir)/server/library/serverScript/include/public \
	-I$(engine_dir)/shared/library/sharedCollision/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDatabaseInterface/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedGame/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMathArchive/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedPathfinding/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSkillSystem/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedTerrain/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(singleton_dir)/include \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(crypto_dir)/include \
	-I$(localization_dir)/include \
	-I$(localization_dir)/include \
	-I$(localizationArchive_dir)/include/public \
	-I$(fileInterface_dir)/include/public \
	-I$(unicode_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(game_dir)/shared/library/swgSharedUtility/include/public \
	-Wno-ctor-dtor-privacy
libSwgDatabaseServersharedgenerated_la_SOURCES=Schema.h \
	Decoder.cpp \
	Decoder_cpp.template \
	Encoder.cpp \
	Encoder_cpp.template \
	LoadObject.cpp \
	LoadObject_cpp.template \
	ObjectQueries.cpp \
	ObjectQueries_cpp.template \
	ObjectQueries.h \
	ObjectQueries_h.template \
	Schema_h.template

Decoder.cpp: Schema.h

$(srcdir)/Schema.h: Schema_h.template $(engine_dir)/server/library/codegen/make_schema_h.pl $(game_dir)/server/database/schema/*.tab
	cp $(srcdir)/Schema_h.template $(srcdir)/Schema.h
	chmod +w $(srcdir)/Schema.h
	perl $(engine_dir)/server/library/codegen/make_schema_h.pl --ddldirectory $(game_dir)/server/database/schema  --output $(srcdir)/Schema.h

$(srcdir)/Encoder.cpp:Encoder_cpp.template ObjectQueries.h $(engine_dir)/server/library/codegen/package_data.txt $(engine_dir)/server/library/codegen/make_packages.pl $(srcdir)/Schema.h
	cp $(srcdir)/Encoder_cpp.template $(srcdir)/Encoder.cpp
	chmod +w $(srcdir)/Encoder.cpp
	perl $(engine_dir)/server/library/codegen/make_packages.pl --datafile $(engine_dir)/server/library/codegen/package_data.txt --encoder $(srcdir)/Encoder.cpp

$(srcdir)/Decoder.cpp: Decoder_cpp.template ObjectQueries.h $(engine_dir)/server/library/codegen/package_data.txt $(engine_dir)/server/library/codegen/make_packages.pl $(srcdir)/Schema.h
	cp $(srcdir)/Decoder_cpp.template $(srcdir)/Decoder.cpp
	chmod +w $(srcdir)/Decoder.cpp
	perl $(engine_dir)/server/library/codegen/make_packages.pl --datafile $(engine_dir)/server/library/codegen/package_data.txt --decoder $(srcdir)/Decoder.cpp

$(srcdir)/LoadObject.cpp: LoadObject_cpp.template ObjectQueries.h $(engine_dir)/server/library/codegen/package_data.txt $(engine_dir)/server/library/codegen/make_packages.pl
	cp $(srcdir)/LoadObject_cpp.template $(srcdir)/LoadObject.cpp
	chmod +w $(srcdir)/LoadObject.cpp
	perl $(engine_dir)/server/library/codegen/make_packages.pl --datafile $(engine_dir)/server/library/codegen/package_data.txt --loadobject $(srcdir)/LoadObject.cpp

$(srcdir)/ObjectQueries.cpp: ObjectQueries_cpp.template ObjectQueries.h $(engine_dir)/server/library/codegen/make_queries.pl $(game_dir)/server/database/packages/persister.plsqlh $(srcdir)/Schema.h
	cp $(srcdir)/ObjectQueries_cpp.template $(srcdir)/ObjectQueries.cpp
	chmod +w $(srcdir)/ObjectQueries.cpp
	perl $(engine_dir)/server/library/codegen/make_queries.pl --plsql $(game_dir)/server/database/packages/persister.plsqlh --source $(srcdir)/ObjectQueries.cpp

$(srcdir)/ObjectQueries.h: ObjectQueries_h.template $(engine_dir)/server/library/codegen/make_queries.pl $(game_dir)/server/database/packages/persister.plsqlh
	cp $(srcdir)/ObjectQueries_h.template $(srcdir)/ObjectQueries.h
	chmod +w $(srcdir)/ObjectQueries.h
	perl $(engine_dir)/server/library/codegen/make_queries.pl --plsql $(game_dir)/server/database/packages/persister.plsqlh --header $(srcdir)/ObjectQueries.h
