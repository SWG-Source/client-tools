AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=SwgBattlefieldTool
SwgBattlefieldTool_CXXFLAGS=-I$(srcdir)/../$(platform) \
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
	-I$(engine_dir)/shared/library/sharedIoWin/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedRegex/include/public \
	-I$(engine_dir)/shared/library/sharedTemplate/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
SwgBattlefieldTool_SOURCES=FirstSwgBattlefieldTool.cpp \
	FirstSwgBattlefieldTool.h \
	Makefile.am \
	SwgBattlefieldTool.cpp

SwgBattlefieldTool_LDADD=$(top_builddir)/../engine/shared/library/sharedDebug/libsharedDebug.la \
	$(top_builddir)/../engine/shared/library/sharedIoWin/libsharedIoWin.la \
	$(top_builddir)/../engine/shared/library/sharedFractal/libsharedFractal.la \
	$(top_builddir)/../engine/shared/library/sharedUtility/libsharedUtility.la \
	$(top_builddir)/../engine/shared/library/sharedXml/libsharedXml.la \
	$(top_builddir)/../engine/shared/library/sharedFoundation/libsharedFoundation.la \
	$(top_builddir)/../engine/shared/library/sharedFile/libsharedFile.la \
	$(top_builddir)/../engine/shared/library/sharedCompression/libsharedCompression.la \
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
	 \
	$(pcre_dir)/lib/libpcre.a \
	$(udplibrary_dir)/output/debug/libudplibrary.a \
	$(top_builddir)/../engine/shared/library/sharedMemoryManager/libsharedMemoryManager.la

SwgBattlefieldTool_LDFLAGS=-lpthread -ldl -lz -lm -lncurses

$(top_builddir)/../game/shared/library/swgSharedUtility/libswgSharedUtility.la:
	make -C $(top_builddir)/../game/shared/library/swgSharedUtility

$(top_builddir)/../engine/shared/library/sharedSkillSystem/libsharedSkillSystem.la:
	make -C $(top_builddir)/../engine/shared/library/sharedSkillSystem

$(top_builddir)/../engine/shared/library/sharedImage/libsharedImage.la:
	make -C $(top_builddir)/../engine/shared/library/sharedImage

$(top_builddir)/../engine/shared/library/sharedFractal/libsharedFractal.la:
	make -C $(top_builddir)/../engine/shared/library/sharedFractal

$(top_builddir)/../engine/shared/library/sharedTerrain/libsharedTerrain.la:
	make -C $(top_builddir)/../engine/shared/library/sharedTerrain

$(top_builddir)/../engine/shared/library/sharedCollision/libsharedCollision.la:
	make -C $(top_builddir)/../engine/shared/library/sharedCollision

$(top_builddir)/../engine/shared/library/sharedObject/libsharedObject.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject

$(top_builddir)/../engine/shared/library/sharedGame/libsharedGame.la:
	make -C $(top_builddir)/../engine/shared/library/sharedGame

$(top_builddir)/../engine/shared/library/sharedMath/libsharedMath.la:
	make -C $(top_builddir)/../engine/shared/library/sharedMath

$(top_builddir)/../engine/shared/library/sharedCommandParser/libsharedCommandParser.la:
	make -C $(top_builddir)/../engine/shared/library/sharedCommandParser

$(top_builddir)/../engine/shared/library/sharedRandom/libsharedRandom.la:
	make -C $(top_builddir)/../engine/shared/library/sharedRandom

$(top_builddir)/../engine/shared/library/sharedUtility/libsharedUtility.la:
	make -C $(top_builddir)/../engine/shared/library/sharedUtility

$(top_builddir)/../engine/shared/library/sharedXml/libsharedXml.la:
	make -C $(top_builddir)/../engine/shared/library/sharedXml

$(top_builddir)/../engine/server/library/serverMetrics/libserverMetrics.la:
	make -C $(top_builddir)/../engine/server/library/serverMetrics

$(top_builddir)/../engine/server/library/serverNetworkMessages/libserverNetworkMessages.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages

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


	

$(top_builddir)/../game/shared/library/swgSharedNetworkMessages/libswgSharedNetworkMessages.la:
	make -C $(top_builddir)/../game/shared/library/swgSharedNetworkMessages

$(top_builddir)/../engine/shared/library/sharedIoWin/libsharedIoWin.la:
	make -C $(top_builddir)/../engine/shared/library/sharedIoWin
