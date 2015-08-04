AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=TransferServer
TransferServer_CXXFLAGS=-I$(srcdir)/../$(platform) \
	-I$(srcdir)/../shared \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(singleton_dir)/include \
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
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedRegex/include/public \
	-I$(engine_dir)/shared/library/sharedTemplate/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(stlport_dir)/stlport \
	-I$(perforce_dir)/include \
	-Wno-ctor-dtor-privacy
TransferServer_SOURCES=main.cpp
TransferServer_LDADD=$(top_builddir)/../engine/shared/library/sharedMemoryManager/libsharedMemoryManager.la \
	../shared/libTransferServershared.la \
	$(top_builddir)/../engine/server/library/serverKeyShare/libserverKeyShare.la \
	$(top_builddir)/../engine/shared/library/sharedDebug/libsharedDebug.la \
	$(top_builddir)/../engine/shared/library/sharedCommandParser/libsharedCommandParser.la \
	$(top_builddir)/../engine/shared/library/sharedUtility/libsharedUtility.la \
	$(top_builddir)/../engine/shared/library/sharedXml/libsharedXml.la \
	$(top_builddir)/../engine/shared/library/sharedNetwork/libsharedNetwork.la \
	$(top_builddir)/../engine/shared/library/sharedLog/libsharedLog.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/core/libsharedNetworkMessagessharedcore.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/common/libsharedNetworkMessagessharedcommon.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/libsharedNetworkMessagessharedclientGameServer.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/sui/libsharedGamesharedsui.la \
	$(top_builddir)/../engine/server/library/serverUtility/libserverUtility.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/libsharedNetworkMessagesshared.la \
	$(top_builddir)/../engine/shared/library/sharedMessageDispatch/libsharedMessageDispatch.la \
	$(top_builddir)/../engine/shared/library/sharedFoundation/libsharedFoundation.la \
	$(top_builddir)/../engine/shared/library/sharedFile/libsharedFile.la \
	$(top_builddir)/../engine/shared/library/sharedCompression/libsharedCompression.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/transferServer/libserverNetworkMessagessharedtransferServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralGameServer/libserverNetworkMessagessharedcentralGameServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameGameServer/libserverNetworkMessagessharedgameGameServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/loginCentralServer/libserverNetworkMessagessharedloginCentralServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/libserverNetworkMessagesshared.la \
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
	$(soePlatform_dir)/CTServiceGameAPI/libCTServiceGameAPI.a \
	$(udplibrary_dir)/output/debug/libudplibrary.a 

TransferServer_LDFLAGS=-lpthread -ldl -lz -lm -lncurses

../shared/libTransferServershared.la:
	cd ../shared && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/libsharedNetworkMessagessharedclientGameServer.la:
	cd $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/libsharedNetworkMessagesshared.la:
	cd $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared && $(MAKE)

$(top_builddir)/../game/shared/library/swgSharedUtility/libswgSharedUtility.la:
	cd $(top_builddir)/../game/shared/library/swgSharedUtility && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedSkillSystem/libsharedSkillSystem.la:
	cd $(top_builddir)/../engine/shared/library/sharedSkillSystem && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedImage/libsharedImage.la:
	cd $(top_builddir)/../engine/shared/library/sharedImage && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedFractal/libsharedFractal.la:
	cd $(top_builddir)/../engine/shared/library/sharedFractal && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedTerrain/libsharedTerrain.la:
	cd $(top_builddir)/../engine/shared/library/sharedTerrain && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedCollision/libsharedCollision.la:
	cd $(top_builddir)/../engine/shared/library/sharedCollision && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedObject/libsharedObject.la:
	cd $(top_builddir)/../engine/shared/library/sharedObject && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedGame/libsharedGame.la:
	cd $(top_builddir)/../engine/shared/library/sharedGame && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedMath/libsharedMath.la:
	cd $(top_builddir)/../engine/shared/library/sharedMath && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedCommandParser/libsharedCommandParser.la:
	cd $(top_builddir)/../engine/shared/library/sharedCommandParser && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedRandom/libsharedRandom.la:
	cd $(top_builddir)/../engine/shared/library/sharedRandom && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedUtility/libsharedUtility.la:
	cd $(top_builddir)/../engine/shared/library/sharedUtility && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedXml/libsharedXml.la:
	cd $(top_builddir)/../engine/shared/library/sharedXml && $(MAKE)

$(top_builddir)/../engine/server/library/serverMetrics/libserverMetrics.la:
	cd $(top_builddir)/engine/server/library/serverMetrics && $(MAKE)

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/transferServer/libserverNetworkMessagessharedtransferServer.la:
	cd $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/transferServer && $(MAKE)

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralGameServer/libserverNetworkMessagessharedcentralGameServer.la:
	cd 	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralGameServer && $(MAKE)

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameGameServer/libserverNetworkMessagessharedgameGameServer.la:
	cd $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameGameServer && $(MAKE)

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/libserverNetworkMessagesshared.la:
	cd $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared && $(MAKE)

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/loginCentralServer/libserverNetworkMessagessharedloginCentralServer.la:
	cd $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/loginCentralServer && $(MAKE)

$(top_builddir)/../engine/server/library/serverUtility/libserverUtility.la:
	cd $(top_builddir)/../engine/server/library/serverUtility && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedCompression/libsharedCompression.la:
	cd $(top_builddir)/../engine/shared/library/sharedCompression && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedDebug/libsharedDebug.la:
	cd $(top_builddir)/../engine/shared/library/sharedDebug && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedFile/libsharedFile.la:
	cd $(top_builddir)/../engine/shared/library/sharedFile && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedFoundation/libsharedFoundation.la:
	cd $(top_builddir)/../engine/shared/library/sharedFoundation && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedLog/libsharedLog.la:
	cd $(top_builddir)/../engine/shared/library/sharedLog && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedMemoryManager/libsharedMemoryManager.la:
	cd $(top_builddir)/../engine/shared/library/sharedMemoryManager && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedMessageDispatch/libsharedMessageDispatch.la:
	cd $(top_builddir)/../engine/shared/library/sharedMessageDispatch && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedNetwork/libsharedNetwork.la:
	cd $(top_builddir)/../engine/shared/library/sharedNetwork && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/core/libsharedNetworkMessagessharedcore.la:
	cd $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/core && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/common/libsharedNetworkMessagessharedcommon.la:
	cd $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/common && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedSynchronization/libsharedSynchronization.la:
	cd $(top_builddir)/../engine/shared/library/sharedSynchronization && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedThread/libsharedThread.la:
	cd $(top_builddir)/../engine/shared/library/sharedThread && $(MAKE)

$(top_builddir)/../engine/server/library/serverKeyShare/libserverKeyShare.la:
	cd $(top_builddir)/../engine/server/library/serverKeyShare && $(MAKE)

$(top_builddir)/../external/ours/library/unicodeArchive/libunicodeArchive.la:
	cd $(top_builddir)/../external/ours/library/unicodeArchive && $(MAKE)

$(top_builddir)/../external/ours/library/unicode/libunicode.la:
	cd $(top_builddir)/../external/ours/library/unicode && $(MAKE)

$(top_builddir)/../external/ours/library/localizationArchive/liblocalizationArchive.la:
	cd $(top_builddir)/../external/ours/library/localizationArchive && $(MAKE)

$(top_builddir)/../external/ours/library/localization/liblocalization.la:
	cd $(top_builddir)/../external/ours/library/localization && $(MAKE)

$(top_builddir)/../external/ours/library/fileInterface/libfileInterface.la:
	cd $(top_builddir)/../external/ours/library/fileInterface && $(MAKE)

$(top_builddir)/../external/ours/library/crypto/libcrypto.la:
	cd $(top_builddir)/../external/ours/library/crypto && $(MAKE)

$(top_builddir)/../external/ours/library/archive/libarchive.la:
	cd $(top_builddir)/../external/ours/library/archive && $(MAKE)

$(udplibrary_dir)/output/debug/libudplibrary.a:
	cd $(udplibrary_dir) && $(MAKE)

$(stlport_dir)/lib/libstlport_gcc.a:
	cd $(stlport_dir)/src && $(MAKE) -f gcc-linux.mak

$(soePlatform_dir)/CTServiceGameAPI/libCTServiceGameAPI.a:
	cd $(soePlatform_dir)/CTServiceGameAPI && $(MAKE)

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/sui/libsharedGamesharedsui.la:
	cd 	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/sui && $(MAKE)


include $(engine_dir)/../make_include/engine.include
