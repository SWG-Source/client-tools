AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=SwgDatabaseServer
SwgDatabaseServer_CXXFLAGS=-I$(srcdir)/../$(platform) \
	-I$(srcdir)/../shared \
	-I$(srcdir)/../../include/public/SwgDatabaseServer \
	-I$(srcdir)/../../include/public \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(singleton_dir)/include \
	-I$(engine_dir)/server/library/serverDatabase/include/public \
	-I$(engine_dir)/server/library/serverKeyShare/include/public \
	-I$(engine_dir)/server/library/serverNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedCommandParser/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedIoWin/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedRegex/include/public \
	-I$(engine_dir)/shared/library/sharedTemplate/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(fileInterface_dir)/include/public \
	-I$(game_dir)/shared/library/swgSharedNetworkMessages/include/public \
	-I$(game_dir)/server/library/swgServerNetworkMessages/include/public \
	-Wno-ctor-dtor-privacy

SwgDatabaseServer_SOURCES=main.cpp

SwgDatabaseServer_LDADD=$(top_builddir)/../engine/shared/library/sharedMemoryManager/libsharedMemoryManager.la \
	../shared/libSwgDatabaseServershared.la \
	$(top_builddir)/../game/server/library/swgServerNetworkMessages/libswgServerNetworkMessages.la \
	$(top_builddir)/../game/shared/library/swgSharedNetworkMessages/libswgSharedNetworkMessages.la \
	$(top_builddir)/../game/shared/library/swgSharedUtility/libswgSharedUtility.la \
	$(top_builddir)/../engine/server/library/serverKeyShare/libserverKeyShare.la \
	$(top_builddir)/../engine/shared/library/sharedDebug/libsharedDebug.la \
	$(top_builddir)/../engine/server/library/serverDatabase/libserverDatabase.la \
	$(top_builddir)/../engine/server/library/serverUtility/libserverUtility.la \
	$(top_builddir)/../engine/server/library/serverMetrics/libserverMetrics.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/core/libserverNetworkMessagessharedcore.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralConnectionServer/libserverNetworkMessagessharedcentralConnectionServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/libserverNetworkMessagesshared.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameGameServer/libserverNetworkMessagessharedgameGameServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameTaskManager/libserverNetworkMessagessharedgameTaskManager.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/loginCentralServer/libserverNetworkMessagessharedloginCentralServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/transferServer/libserverNetworkMessagessharedtransferServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralGameServer/libserverNetworkMessagessharedcentralGameServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gamePlanetServer/libserverNetworkMessagessharedgamePlanetServer.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/metricsMessages/libserverNetworkMessagessharedmetricsMessages.la \
	$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/commoditiesSwgDatabase/libserverNetworkMessagessharedcommoditiesSwgDatabase.la \
	$(top_builddir)/../engine/shared/library/sharedSkillSystem/libsharedSkillSystem.la \
	$(top_builddir)/../engine/shared/library/sharedTerrain/libsharedTerrain.la \
	$(top_builddir)/../engine/shared/library/sharedImage/libsharedImage.la \
	$(top_builddir)/../engine/shared/library/sharedFractal/libsharedFractal.la \
	$(top_builddir)/../engine/shared/library/sharedCommandParser/libsharedCommandParser.la \
	$(top_builddir)/../engine/shared/library/sharedCollision/src/shared/core/libsharedCollisionsharedcore.la \
	$(top_builddir)/../engine/shared/library/sharedCollision/src/shared/extent/libsharedCollisionsharedextent.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/object/libsharedObjectsharedobject.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/customization/libsharedObjectsharedcustomization.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/world/libsharedObjectsharedworld.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/property/libsharedObjectsharedproperty.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/container/libsharedObjectsharedcontainer.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/movement/libsharedObjectsharedmovement.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/dynamics/libsharedObjectshareddynamics.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/controller/libsharedObjectsharedcontroller.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/lot/libsharedObjectsharedlot.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/portal/libsharedObjectsharedportal.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/core/libsharedObjectsharedcore.la \
	$(top_builddir)/../engine/shared/library/sharedObject/src/shared/appearance/libsharedObjectsharedappearance.la \
	$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/queue/libsharedDatabaseInterfacesharedqueue.la \
	$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/core/libsharedDatabaseInterfacesharedcore.la \
	$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src_oci/libsharedDatabaseInterfacesrc_oci.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/appearance/libsharedGamesharedappearance.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/core/libsharedGamesharedcore.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/combat/libsharedGamesharedcombat.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/mount/libsharedGamesharedmount.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/object/libsharedGamesharedobject.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/objectTemplate/libsharedGamesharedobjectTemplate.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/quest/libsharedGamesharedquest.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/space/libsharedGamesharedspace.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/sui/libsharedGamesharedsui.la \
	$(top_builddir)/../engine/shared/library/sharedGame/src/shared/travel/libsharedGamesharedtravel.la \
	$(top_builddir)/../engine/shared/library/sharedUtility/libsharedUtility.la \
	$(top_builddir)/../engine/shared/library/sharedXml/libsharedXml.la \
	$(top_builddir)/../engine/shared/library/sharedNetwork/libsharedNetwork.la \
	$(top_builddir)/../engine/shared/library/sharedLog/libsharedLog.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/core/libsharedNetworkMessagessharedcore.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/libsharedNetworkMessagessharedclientGameServer.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/libsharedNetworkMessagesshared.la \
	$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/common/libsharedNetworkMessagessharedcommon.la \
	$(top_builddir)/../engine/shared/library/sharedMessageDispatch/libsharedMessageDispatch.la \
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
	$(ORACLE_HOME)/lib/libclntsh.so \
	$(udplibrary_dir)/output/debug/libudplibrary.a 

SwgDatabaseServer_LDFLAGS=-lpthread -ldl -lz -lm -lncurses

../shared/libSwgDatabaseServershared.la:
	make -C ../shared

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/libsharedNetworkMessagesshared.la:
		make -C $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/common/libsharedNetworkMessagessharedcommon.la:
		make -C $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/common

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/libsharedNetworkMessagessharedclientGameServer.la:
	make -C $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer

$(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/core/libsharedNetworkMessagessharedcore.la:
		make -C $(top_builddir)/../engine/shared/library/sharedNetworkMessages/src/shared/core

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameGameServer/libserverNetworkMessagessharedgameGameServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameGameServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/loginCentralServer/libserverNetworkMessagessharedloginCentralServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/loginCentralServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gamePlanetServer/libserverNetworkMessagessharedgamePlanetServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gamePlanetServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/metricsMessages/libserverNetworkMessagessharedmetricsMessages.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/metricsMessages

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/libserverNetworkMessagesshared.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/transferServer/libserverNetworkMessagessharedtransferServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/transferServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralGameServer/libserverNetworkMessagessharedcentralGameServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralGameServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralConnectionServer/libserverNetworkMessagessharedcentralConnectionServer.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/centralConnectionServer

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameTaskManager/libserverNetworkMessagessharedgameTaskManager.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/gameTaskManager

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/commoditiesSwgDatabase/libserverNetworkMessagessharedcommoditiesSwgDatabase.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/commoditiesSwgDatabase

$(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/core/libserverNetworkMessagessharedcore.la:
	make -C $(top_builddir)/../engine/server/library/serverNetworkMessages/src/shared/core

$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src_oci/libsharedDatabaseInterfacesrc_oci.la:
	make -C $(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src_oci

$(top_builddir)/../game/server/library/swgServerNetworkMessages/libswgServerNetworkMessages.la:
	make -C $(top_builddir)/../game/server/library/swgServerNetworkMessages

$(top_builddir)/../game/shared/library/swgSharedNetworkMessages/libswgSharedNetworkMessages.la:
	make -C $(top_builddir)/../game/shared/library/swgSharedNetworkMessages

$(top_builddir)/../engine/server/library/serverDatabase/libserverDatabase.la:
	make -C $(top_builddir)/../engine/server/library/serverDatabase

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

$(top_builddir)/../engine/shared/library/sharedCollision/src/shared/extent/libsharedCollisionsharedextent.la:
	make -C $(top_builddir)/../engine/shared/library/sharedCollision/src/shared/extent

$(top_builddir)/../engine/shared/library/sharedCollision/src/shared/core/libsharedCollisionsharedcore.la:
	make -C $(top_builddir)/../engine/shared/library/sharedCollision/src/shared/core

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/customization/libsharedObjectsharedcustomization.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/customization

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/appearance/libsharedObjectsharedappearance.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/appearance

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/world/libsharedObjectsharedworld.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/world

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/container/libsharedObjectsharedcontainer.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/container

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/core/libsharedObjectsharedcore.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/core

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/movement/libsharedObjectsharedmovement.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/movement

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/portal/libsharedObjectsharedportal.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/portal

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/controller/libsharedObjectsharedcontroller.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/controller

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/dynamics/libsharedObjectshareddynamics.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/dynamics

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/property/libsharedObjectsharedproperty.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/property

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/lot/libsharedObjectsharedlot.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/lot

$(top_builddir)/../engine/shared/library/sharedObject/src/shared/object/libsharedObjectsharedobject.la:
	make -C $(top_builddir)/../engine/shared/library/sharedObject/src/shared/object

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/appearance/libsharedGamesharedappearance.la:
	make -C $(top_builddir)/../engine/shared/library/sharedGame/src/shared/appearance

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/core/libsharedGamesharedcore.la:
	make -C $(top_builddir)/../engine/shared/library/sharedGame/src/shared/core

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/combat/libsharedGamesharedcombat.la:
	make -C $(top_builddir)/../engine/shared/library/sharedGame/src/shared/combat

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/mount/libsharedGamesharedmount.la:
	make -C $(top_builddir)/../engine/shared/library/sharedGame/src/shared/mount

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/object/libsharedGamesharedobject.la:
	make -C $(top_builddir)/../engine/shared/library/sharedGame/src/shared/object

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/objectTemplate/libsharedGamesharedobjectTemplate.la:
	make -C $(top_builddir)/../engine/shared/library/sharedGame/src/shared/objectTemplate

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/space/libsharedGamesharedspace.la:
	make -C $(top_builddir)/../engine/shared/library/sharedGame/src/shared/space

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/sui/libsharedGamesharedsui.la:
	make -C $(top_builddir)/../engine/shared/library/sharedGame/src/shared/sui

$(top_builddir)/../engine/shared/library/sharedGame/src/shared/travel/libsharedGamesharedtravel.la:
	make -C $(top_builddir)/../engine/shared/library/sharedGame/src/shared/travel

$(top_builddir)/../engine/shared/library/sharedMath/libsharedMath.la:
	make -C $(top_builddir)/../engine/shared/library/sharedMath

$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/queue/libsharedCommandParsersharedqueue.la:
	make -C $(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/queue

$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/core/libsharedCommandParsersharedcore.la:
	make -C $(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/core

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


	
