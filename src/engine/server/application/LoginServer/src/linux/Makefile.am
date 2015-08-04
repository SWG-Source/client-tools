AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=LoginServer
LoginServer_CXXFLAGS=-I$(srcdir)/../$(platform) \
	-I$(srcdir)/../shared \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(singleton_dir)/include \
	-I$(engine_dir)/server/library/serverNetworkMessages/include/public \
	-I$(engine_dir)/server/library/serverKeyShare/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedRegex/include/public \
	-I$(engine_dir)/shared/library/sharedTemplate/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(stlport_dir)/stlport \
	-I$(perforce_dir)/include \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-Wno-ctor-dtor-privacy

#        -I$(soePlatform_dir)/ChatAPI2/ChatAPI/utils/ 
#        -I$(soePlatform_dir)/ChatAPI2/ChatAPI/projects/Chat 

LoginServer_SOURCES=main.cpp
LoginServer_LDADD=$(top_builddir)/../engine/shared/library/sharedMemoryManager/libsharedMemoryManager.la \
	../shared/libLoginServershared.la \
	$(top_builddir)/../engine/server/library/serverKeyShare/libserverKeyShare.la \
	$(top_builddir)/../engine/shared/library/sharedDebug/libsharedDebug.la \
	$(top_builddir)/../engine/shared/library/sharedCommandParser/libsharedCommandParser.la \
	$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/queue/libsharedDatabaseInterfacesharedqueue.la \
	$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/core/libsharedDatabaseInterfacesharedcore.la \
	$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src_oci/libsharedDatabaseInterfacesrc_oci.la \
	$(top_builddir)/../engine/shared/library/sharedUtility/libsharedUtility.la \
	$(top_builddir)/../engine/shared/library/sharedXml/libsharedXml.la \
	$(top_builddir)/../engine/shared/library/sharedNetwork/libsharedNetwork.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/sui/libsharedGamesharedsui.la \
	$(top_builddir)/../engine/shared/library/sharedMessageDispatch/libsharedMessageDispatch.la \
	$(top_builddir)/../engine/shared/library/sharedLog/libsharedLog.la \
	$(top_builddir)/../engine/shared/library/sharedFile/libsharedFile.la \
	$(top_builddir)/../engine/shared/library/sharedCompression/libsharedCompression.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/core/libsharedNetworkMessagessharedcore.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/common/libsharedNetworkMessagessharedcommon.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/libsharedNetworkMessagessharedclientGameServer.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/libsharedNetworkMessagesshared.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientLoginServer/libsharedNetworkMessagessharedclientLoginServer.la \
	$(top_builddir)/../engine/server/library/serverUtility/libserverUtility.la \
	$(top_builddir)/../engine/server/library/serverMetrics/libserverMetrics.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/loginCentralServer/libserverNetworkMessagessharedloginCentralServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralGameServer/libserverNetworkMessagessharedcentralGameServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralConnectionServer/libserverNetworkMessagessharedcentralConnectionServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameGameServer/libserverNetworkMessagessharedgameGameServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/transferServer/libserverNetworkMessagessharedtransferServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/metricsMessages/libserverNetworkMessagessharedmetricsMessages.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/libserverNetworkMessagesshared.la \
	$(top_builddir)/../engine/shared/library/sharedFoundation/libsharedFoundation.la \
	$(top_builddir)/../engine/shared/library/sharedThread/libsharedThread.la \
	$(top_builddir)/../engine/shared/library/sharedSynchronization/libsharedSynchronization.la \
	$(top_builddir)/../engine/shared/library/sharedMath/libsharedMath.la \
	$(top_builddir)/../engine/shared/library/sharedRandom/libsharedRandom.la \
	$(top_builddir)/../external/ours/library/localizationArchive/liblocalizationArchive.la \
	$(top_builddir)/../external/ours/library/localization/liblocalization.la \
	$(top_builddir)/../external/ours/library/unicodeArchive/libunicodeArchive.la \
	$(top_builddir)/../external/ours/library/unicode/libunicode.la \
	$(top_builddir)/../external/ours/library/fileInterface/libfileInterface.la \
	$(top_builddir)/../external/ours/library/crypto/libcrypto.la \
	$(top_builddir)/../external/ours/library/archive/libarchive.la \
	$(libxml_dir)/libxml2.la \
	$(stlport_dir)/lib/libstlport_gcc.a \
	$(pcre_dir)/lib/libpcre.a \
	$(top_builddir)/../external/3rd/library/platform/MonAPI2/libmonapi.la \
	$(platform_dir)/lib/release/libBase.a \
	$(platform_dir)/lib/release/libLoginAPI.a \
	$(platform_dir)/lib/release/libCommonAPI.a \
	$(udplibrary_dir)/output/debug/libudplibrary.a 

LoginServer_LDFLAGS=-L$(ORACLE_HOME)/lib -lpthread -ldl -lm -lz -lclntsh -lncurses

../shared/libLoginServershared.la:
	make -C ../shared

$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/queue/libsharedDatabaseInterfacesharedqueue.la:
	make -C $(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/queue

$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/core/libsharedDatabaseInterfacesharedcore.la:
	make -C $(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/core

$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src_oci/libsharedDatabaseInterfacesrc_oci.la:
	make -C $(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src_oci

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/core/libsharedNetworkMessagessharedcore.la:
		make -C $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/core

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/common/libsharedNetworkMessagessharedcommon.la:
		make -C $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/common

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/libsharedNetworkMessagessharedclientGameServer.la:
		make -C $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/libsharedNetworkMessagesshared.la:
		make -C $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientLoginServer/libsharedNetworkMessagessharedclientLoginServer.la:
		make -C $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientLoginServer

$(top_builddir)/../engine/shared/library/sharedMath/libsharedMath.la:
	make -C $(top_builddir)/../engine/shared/library/sharedMath

$(top_builddir)/../engine/shared/library/sharedRandom/libsharedRandom.la:
	make -C $(top_builddir)/../engine/shared/library/sharedRandom

$(top_builddir)/../engine/shared/library/sharedUtility/libsharedUtility.la:
	make -C $(top_builddir)/../engine/shared/library/sharedUtility

$(top_builddir)/../engine/shared/library/sharedXml/libsharedXml.la:
	make -C $(top_builddir)/../engine/shared/library/sharedXml

$(top_builddir)/../engine/server/library/serverMetrics/libserverMetrics.la:
	make -C $(top_builddir)/../engine/server/library/serverMetrics

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/loginCentralServer/libserverNetworkMessagessharedloginCentralServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/loginCentralServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/libserverNetworkMessagesshared.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralGameServer/libserverNetworkMessagessharedcentralGameServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralGameServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralConnectionServer/libserverNetworkMessagessharedcentralConnectionServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralConnectionServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameGameServer/libserverNetworkMessagessharedgameGameServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameGameServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/transferServer/libserverNetworkMessagessharedtransferServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/transferServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/metricsMessages/libserverNetworkMessagessharedmetricsMessages.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/metricsMessages

$(top_builddir)/../engine/server/library/serverUtility/libserverUtility.la:
	make -C $(top_builddir)/../engine/server/library/serverUtility

$(top_builddir)/../engine/shared/library/sharedCompression/libsharedCompression.la:
	make -C $(top_builddir)/../engine/shared/library/sharedCompression

$(top_builddir)/../engine/shared/library/sharedDebug/libsharedDebug.la:
	make -C $(top_builddir)/../engine/shared/library/sharedDebug

$(top_builddir)/../engine/shared/library/sharedFile/libsharedFile.la:
	make -C $(top_builddir)/../engine/shared/library/sharedFile

$(top_builddir)/../engine/shared/library/sharedFoundation/libsharedFoundation.la:
	make -C $(top_builddir)/../engine/shared/library/sharedFoundation

$(top_builddir)/../engine/shared/library/sharedLog/libsharedLog.la:
	make -C $(top_builddir)/../engine/shared/library/sharedLog

$(top_builddir)/../engine/shared/library/sharedMemoryManager/libsharedMemoryManager.la:
	make -C $(top_builddir)/../engine/shared/library/sharedMemoryManager

$(top_builddir)/../engine/shared/library/sharedMessageDispatch/libsharedMessageDispatch.la:
	make -C $(top_builddir)/../engine/shared/library/sharedMessageDispatch

$(top_builddir)/../engine/shared/library/sharedNetwork/libsharedNetwork.la:
	make -C $(top_builddir)/../engine/shared/library/sharedNetwork

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/libsharedNetworkMessages.la:
	make -C $(top_builddir)/../engine/shared/library/sharedNetworkMessages

$(top_builddir)/../engine/shared/library/sharedSynchronization/libsharedSynchronization.la:
	make -C $(top_builddir)/../engine/shared/library/sharedSynchronization

$(top_builddir)/../engine/shared/library/sharedThread/libsharedThread.la:
	make -C $(top_builddir)/../engine/shared/library/sharedThread

$(top_builddir)/../engine/server/library/serverKeyShare/libserverKeyShare.la:
	make -C $(top_builddir)/../engine/server/library/serverKeyShare

$(top_builddir)/../external/ours/library/unicodeArchive/libunicodeArchive.la:
	make -C $(top_builddir)/../external/ours/library/unicodeArchive

$(top_builddir)/../external/ours/library/unicode/libunicode.la:
	make -C $(top_builddir)/../external/ours/library/unicode

$(top_builddir)/../external/ours/library/localizationArchive/liblocalizationArchive.la:
	make -C $(top_builddir)/../external/ours/library/localizationArchive

$(top_builddir)/../external/ours/library/localization/liblocalization.la:
	make -C $(top_builddir)/../external/ours/library/localization

$(top_builddir)/../external/ours/library/fileInterface/libfileInterface.la:
	make -C $(top_builddir)/../external/ours/library/fileInterface

$(top_builddir)/../external/ours/library/crypto/libcrypto.la:
	make -C $(top_builddir)/../external/ours/library/crypto

$(top_builddir)/../external/ours/library/archive/libarchive.la:
	make -C $(top_builddir)/../external/ours/library/archive

$(udplibrary_dir)/output/debug/libudplibrary.a:
	make -C $(udplibrary_dir)

$(stlport_dir)/lib/libstlport_gcc.a:
	make -C $(stlport_dir)/src -f gcc-linux.mak

$(top_builddir)/../external/3rd/library/platform/MonAPI2/libmonapi.la:
	make -C $(top_builddir)/../external/3rd/library/platform/MonAPI2

$(platform_dir)/lib/release/libBase.a:
	make -C $(platform_dir)/utils/Base/linux

$(platform_dir)/lib/release/libLoginAPI.a:
	$(platform_dir)/projects/Session/LoginAPI

$(platform_dir)/lib/release/libCommonAPI.a:
	$(platform_dir)/projects/Session/CommonAPI

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/sui/libsharedGamesharedsui.la:
	cd 	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/sui && $(MAKE)


include $(engine_dir)/../make_include/engine.include
