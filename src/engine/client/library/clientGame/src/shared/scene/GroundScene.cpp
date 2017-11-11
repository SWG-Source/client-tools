// ======================================================================
//
// GroundScene.cpp
// Copyright 2000-2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GroundScene.h"

#include "clientAnimation/PlaybackScriptManager.h"
#include "clientAudio/Audio.h"
#include "clientGame/AlarmManager.h"
#include "clientGame/AuctionManagerClient.h"
#include "clientGame/AutoCommManager.h"
#include "clientGame/AwayFromKeyBoardManager.h"
#include "clientGame/CellObject.h"
#include "clientGame/ClientAsteroidManager.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ClientInteriorLayoutManager.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientObjectTerrainModificationNotification.h"
#include "clientGame/ClientPathObject.h"
#include "clientGame/ClientSecureTradeManager.h"
#include "clientGame/ClientStringIds.h"
#include "clientGame/ClientWeaponObjectTemplate.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CockpitCamera.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/CustomerServiceManager.h"
#include "clientGame/DebugPortalCamera.h"
#include "clientGame/FlyByCamera.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/FreeChaseCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GameLight.h"
#include "clientGame/GameMusicManager.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/HyperspaceIoWin.h"
#include "clientGame/InputScheme.h"
#include "clientGame/LightsaberCollisionManager.h"
#include "clientGame/MatchMakingManager.h"
#include "clientGame/MissileManager.h"
#include "clientGame/NebulaManagerClient.h"
#include "clientGame/NonTrackingProjectile.h"
#include "clientGame/OverheadMap.h"
#include "clientGame/Panorama.h"
#include "clientGame/PauseGame.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerMusicManager.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/PlotterManager.h"
#include "clientGame/ProjectileManager.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientGame/QuestJournalManager.h"
#include "clientGame/ScreenShot360.h"
#include "clientGame/ShipController.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientGame/ShipTurretCamera.h"
#include "clientGame/ShipWeaponGroupManager.h"
#include "clientGame/SpaceDeath.h"
#include "clientGame/SpacePreloadedAssetManager.h"
#include "clientGame/SpaceTargetBracketOverlay.h"
#include "clientGame/StructurePlacementCamera.h"
#include "clientGame/WorldSnapshot.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSetTemplate.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientObject/CameraController.h"
#include "clientObject/MouseCursor.h"
#include "clientObject/SaveCameraParameters.h"
#include "clientObject/ShadowManager.h"
#include "clientObject/ShadowVolume.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientSkeletalAnimation/CharacterLodManager.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "clientTerrain/GroundEnvironment.h"
#include "clientTerrain/WaterManager.h"
#include "clientTerrain/WeatherManager.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCachedAvatarManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiLoadingManager.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiModifierManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "fileInterface/StdioFile.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/PixCounter.h"
#include "sharedDebug/Profiler.h"
#include "sharedDebug/VTune.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/FileManifest.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/CrcStringTable.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/SharedShipObjectTemplate.h"
#include "sharedInputMap/InputMap.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedLog/Log.h"
#include "sharedMath/Vector2d.h"
#include "sharedNetworkMessages/AiDebuggingMessages.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/ClientOpenContainerMessage.h"
#include "sharedNetworkMessages/CreateClientPathMessage.h"
#include "sharedNetworkMessages/CreateMissileMessage.h"
#include "sharedNetworkMessages/CreateNebulaLightningMessage.h"
#include "sharedNetworkMessages/CreateProjectileMessage.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedNetworkMessages/DestroyClientPathMessage.h"
#include "sharedNetworkMessages/DestroyShipComponentMessage.h"
#include "sharedNetworkMessages/DestroyShipMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/HyperspaceMessage.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/NewbieTutorialRequest.h"
#include "sharedNetworkMessages/NewbieTutorialResponse.h"
#include "sharedNetworkMessages/PlayMusicMessage.h"
#include "sharedNetworkMessages/SceneChannelMessages.h"
#include "sharedNetworkMessages/ServerTimeMessage.h"
#include "sharedNetworkMessages/ServerWeatherMessage.h"
#include "sharedNetworkMessages/ShipUpdateTransformCollisionMessage.h"
#include "sharedNetworkMessages/ShipUpdateTransformMessage.h"
#include "sharedNetworkMessages/SlowDownEffectMessage.h"
#include "sharedNetworkMessages/UpdateCellPermissionMessage.h"
#include "sharedNetworkMessages/UpdateContainmentMessage.h"
#include "sharedNetworkMessages/UpdateMissileMessage.h"
#include "sharedNetworkMessages/UpdatePostureMessage.h"
#include "sharedNetworkMessages/UpdatePvpStatusMessage.h"
#include "sharedNetworkMessages/UpdateTransformMessage.h"
#include "sharedNetworkMessages/UpdateTransformWithParentMessage.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/DebugNotification.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/CachedFileManager.h"
#include "sharedUtility/FileName.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include <cstdio>

#ifdef _DEBUG
#include "clientGame/AiDebugStringManager.h"
#endif // _DEBUG

//-----------------------------------------------------------------

//lint -esym(641,CameraIds)
//lint -esym(641,GameControllerMessage)
//lint -esym(641,ShipStation)
//lint -esym(641,WorldObjectLists)
//lint -esym(1060,World::getConstObject)
//lint -esym(1060,World::getNumberOfObjects)
//lint -esym(1060,World::getObject)
//lint -esym(613,GroundScene::m_cockpitCamera)
//lint -esym(613,GroundScene::m_debugPortalCamera)
//lint -esym(613,GroundScene::m_debugPortalCameraInputMap)
//lint -esym(613,GroundScene::m_freeCamera)
//lint -esym(613,GroundScene::m_freeCameraInputMap)
//lint -esym(613,GroundScene::m_freeChaseCamera)
//lint -esym(613,GroundScene::m_inputMap)
//lint -esym(613,GroundScene::m_overheadMap)
//lint -esym(613,GroundScene::m_shipTurretCamera)
//lint -esym(613,GroundScene::m_structurePlacementCamera)

//-----------------------------------------------------------------

namespace PlayerCreatureControllerNamespace
{
#if PRODUCTION == 0
	extern bool s_usePlayerServerSpeed;
#endif
	extern float ms_playerServerMovementSpeed;
}

namespace GroundSceneNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if PRODUCTION == 0
	typedef std::map<unsigned long int, int>  IntMap;
	typedef std::map<NetworkId, int>          NetworkIdIntMap;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool ms_logCreateMessages = false;
	bool ms_testCollision = false;
	bool ms_noDraw = false;
	bool ms_useBuildoutClip = true;

#if PRODUCTION == 0
	bool             ms_logReceivedMessages;
	IntMap           ms_receivedMessageMap;

	bool             ms_logUpdateTransformMessages;
	NetworkIdIntMap  ms_updateTransformMessageMap;

	PixCounter::ResetInteger ms_createObjectCountPerFrame;
	PixCounter::ResetString  ms_createObjectsPerFrame;
	PixCounter::ResetInteger ms_destroyObjectCountPerFrame;
	PixCounter::ResetString  ms_destroyObjectsPerFrame;
#endif

	namespace UnnamedMessages
	{
		const char * const ConnectionServerConnectionClosed = "ConnectionServerConnectionClosed";
	}

	const std::string cms_newbieTutorialRequestZoomCamera ("zoomCamera");

	void zoomCameraCallback (void* /*context*/)
	{
		const NewbieTutorialResponse response (cms_newbieTutorialRequestZoomCamera);
		GameNetwork::send (response, true);
	}

	const std::string cms_newbieTutorialRequestChangeLookAtTarget ("changeLookAtTarget");

	void changeLookAtTargetCallback (void* /*context*/)
	{
		const NewbieTutorialResponse response (cms_newbieTutorialRequestChangeLookAtTarget);
		GameNetwork::send (response, true);
	}

	const std::string cms_newbieTutorialRequestOverlayMap ("overlayMap");

	void overlayMapCallback (void* /*context*/)
	{
		const NewbieTutorialResponse response (cms_newbieTutorialRequestOverlayMap);
		GameNetwork::send (response, true);
	}

#if PRODUCTION == 0
	void  reportReceivedMessages();
	void  reportUpdateTransformMessages();
#endif

	float const cms_tutorialFarPlaneDistance = 1024.f;
	float ms_cameraFarPlane           = 4096.f;
	float ms_cameraFarPlaneSpace      = 16384.f;
	float ms_cameraFieldOfViewDegrees = 60.f;

	float ms_mouseSensitivity = 1.0f;

	bool  ms_invertMouse      = false;
	bool  ms_listenerFollowsPlayer = false;

	bool s_installed          = false;

	bool  ms_loadingScreenRender;
	float ms_loadingScreenRenderYaw;

	CrcStringTable ms_objectTemplateCrcStringTable;

	bool ms_renderDetailLevel;
	GroundScene::RenderDetailLevelFunction ms_renderDetailLevelFunction;
	void renderDetailLevel ();
	void renderDetailLevelObjectList (int objectListIndex);

	void remove ();

	char ms_terrainName[128];
	char ms_playerPosition[128];

	bool ms_renderProjectilePath;
	bool ms_letterBoxedViewport;
	bool ms_testSpaceDeathUsingPanorama;

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CreateMissileMessage &> createMissile;
	}
}

using namespace GroundSceneNamespace;
using namespace PlayerCreatureControllerNamespace;

// ======================================================================
// namespace GroundSceneNamespace
// ======================================================================

#if PRODUCTION == 0

void GroundSceneNamespace::reportReceivedMessages()
{
	if (ms_receivedMessageMap.empty())
		return;

	LOG("GroundSceneReceivedMessages", ("received [%d] messages on graphics frame [%d]", static_cast<int>(ms_receivedMessageMap.size()), Graphics::getFrameNumber()));

	IntMap::iterator const endIt = ms_receivedMessageMap.end();
	for (IntMap::iterator it = ms_receivedMessageMap.begin(); it != endIt; ++it)
	{
		char const *const messageName = LabelHash::findLabel("MessageDispatch", it->first);
		if (messageName && *messageName)
			LOG("GroundSceneReceivedMessages", ("message name=[%s]: called [%d] times", messageName, it->second));
		else
			LOG("GroundSceneReceivedMessages", ("message hash=[%u]: called [%d] times", it->first, it->second));
	}

	ms_receivedMessageMap.clear();
}

#endif

// ----------------------------------------------------------------------

#if PRODUCTION == 0

void GroundSceneNamespace::reportUpdateTransformMessages()
{
	if (ms_updateTransformMessageMap.empty())
		return;

	LOG("GroundSceneUpdateTransformMessages", ("received [%d] UpdateTransformMessage messages on graphics frame [%d].", static_cast<int>(ms_updateTransformMessageMap.size()), Graphics::getFrameNumber()));

	Vector const playerPosition_w = (Game::getPlayer() ? Game::getPlayer()->getPosition_w() : Vector::zero);

	NetworkIdIntMap::iterator endIt = ms_updateTransformMessageMap.end();
	for (NetworkIdIntMap::iterator it = ms_updateTransformMessageMap.begin(); it != endIt; ++it)
	{
		Object *const object = NetworkIdManager::getObjectById(it->first);
		if (object)
		{
			float const distanceFromPlayer = playerPosition_w.magnitudeBetween(object->getPosition_w());
			LOG("GroundSceneUpdateTransformMessages", ("[%d] UpdateNetworkMessage messages for object id=[%s],distanceFromPlayer=[%g],template=[%s]", it->second, it->first.getValueString().c_str(), distanceFromPlayer, object->getObjectTemplateName()));
		}
		else
			LOG("GroundSceneUpdateTransformMessages", ("[%d] UpdateNetworkMessage messages for object id=[%s],distanceFromPlayer=[<not on client>],template=[<unknown>]", it->second, it->first.getValueString().c_str()));
	}

	ms_updateTransformMessageMap.clear();
}

#endif

// ======================================================================
// class GroundScene: PUBLIC STATIC
// ======================================================================

void GroundScene::install ()
{
	InstallTimer const installTimer("GroundScene::install");

	DEBUG_FATAL (s_installed, ("alerady installed"));

#if PRODUCTION == 0
	ms_createObjectCountPerFrame.bindToCounter("CreateObjectCount");
	ms_createObjectsPerFrame.bindToCounter("CreateObjects");
	ms_destroyObjectCountPerFrame.bindToCounter("DestroyObjectCount");
	ms_destroyObjectsPerFrame.bindToCounter("DestroyObjects");
#endif

	//-- get defaults
	ms_mouseSensitivity         = ConfigClientGame::getMouseSensitivity  ();
	ms_cameraFarPlane           = ConfigClientGame::getCameraFarPlane    ();
	ms_cameraFarPlaneSpace      = ConfigClientGame::getCameraFarPlaneSpace();
	ms_cameraFieldOfViewDegrees = ConfigClientGame::getCameraFieldOfView ();
	ms_invertMouse              = ConfigClientGame::getInvertMouse       ();

	DebugFlags::registerFlag (ms_logCreateMessages, "ClientGame", "logCreateMessages");
	DebugFlags::registerFlag (ms_testCollision, "ClientGame", "testCollision");
	DebugFlags::registerFlag (ms_renderDetailLevel, "ClientGame", "renderDetailLevel");
	DebugFlags::registerFlag (ms_renderProjectilePath, "ClientGame", "renderProjectilePath");
	DebugFlags::registerFlag (ms_letterBoxedViewport, "ClientGame", "letterBoxedViewport");
	DebugFlags::registerFlag (ms_testSpaceDeathUsingPanorama, "ClientGame/GroundScene", "testSpaceDeathUsingPanorama");
	DebugFlags::registerFlag (ms_noDraw, "ClientGame", "groundSceneNoDraw");
	DebugFlags::registerFlag (ms_useBuildoutClip, "ClientGame", "useBuildoutClip");

#if PRODUCTION == 0
	DebugFlags::registerFlag (ms_logReceivedMessages, "ClientGame/GroundScene", "logReceivedMessages", reportReceivedMessages);
	DebugFlags::registerFlag (ms_logUpdateTransformMessages, "ClientGame/GroundScene", "logUpdateTransformMessages", reportUpdateTransformMessages);
#endif

	//-- register options
	LocalMachineOptionManager::registerOption (ms_mouseSensitivity, "ClientGame", "mouseSensitivity", 1);
	LocalMachineOptionManager::registerOption (ms_cameraFarPlane, "ClientGame", "cameraFarPlane");
	LocalMachineOptionManager::registerOption (ms_cameraFarPlaneSpace, "ClientGame", "cameraFarPlaneSpace");
	LocalMachineOptionManager::registerOption (ms_cameraFieldOfViewDegrees, "ClientGame", "cameraFieldOfView");
	LocalMachineOptionManager::registerOption (ms_invertMouse, "ClientGame", "invertMouse");
	LocalMachineOptionManager::registerOption (ms_listenerFollowsPlayer, "ClientGame", "listenerFollowsPlayer");

	s_installed = true;

	ExitChain::add (GroundSceneNamespace::remove, "GroundSceneNamespace::remove");

	ms_objectTemplateCrcStringTable.load("misc/object_template_crc_string_table.iff");

	strcpy(ms_terrainName, "Terrain: none\n");
	CrashReportInformation::addDynamicText(ms_terrainName);
	strcpy(ms_playerPosition, "Player: none\n");
	CrashReportInformation::addDynamicText(ms_playerPosition);
}

//-----------------------------------------------------------------

void GroundSceneNamespace::remove ()
{
	DebugFlags::unregisterFlag (ms_logCreateMessages);
	DebugFlags::unregisterFlag (ms_testCollision);
	DebugFlags::unregisterFlag (ms_renderDetailLevel);
	DebugFlags::unregisterFlag (ms_renderProjectilePath);
	DebugFlags::unregisterFlag (ms_letterBoxedViewport);
	DebugFlags::unregisterFlag (ms_testSpaceDeathUsingPanorama);

#if PRODUCTION == 0
	DebugFlags::unregisterFlag (ms_logReceivedMessages);
#endif

	CrashReportInformation::removeDynamicText(ms_terrainName);
	CrashReportInformation::removeDynamicText(ms_playerPosition);
}

//-----------------------------------------------------------------

void GroundScene::setRenderDetailLevelFunction (RenderDetailLevelFunction renderDetailLevelFunction)
{
	ms_renderDetailLevelFunction = renderDetailLevelFunction;
}

//-----------------------------------------------------------------

const Object* GroundScene::getSoundObject () const
{
	if (ms_listenerFollowsPlayer)
	{
		if (getCurrentView() == CI_shipTurret)
		{
			ShipTurretCamera const * const shipTurretCamera = safe_cast<ShipTurretCamera const *>(getCurrentCamera());
			if (shipTurretCamera && shipTurretCamera->getTarget())
				return shipTurretCamera->getTarget();
		}

		if (getPlayer())
			return getPlayer();
	}
	
	return getCurrentCamera();
}

//-----------------------------------------------------------------

float GroundScene::getCameraFieldOfViewDegrees ()
{
	return ms_cameraFieldOfViewDegrees;
}

//-------------------------------------------------------------------

void GroundScene::setCameraFieldOfViewDegrees (const float fieldOfViewDegrees)
{
	ms_cameraFieldOfViewDegrees = fieldOfViewDegrees;

	GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
	if (groundScene)
	{
		int i;
		for (i = 0; i < groundScene->getNumberOfViews (); ++i)
			groundScene->getCamera (i)->setHorizontalFieldOfView (convertDegreesToRadians (fieldOfViewDegrees));
	}
}

//-------------------------------------------------------------------

float GroundScene::getCameraFarPlane ()
{
	return ms_cameraFarPlane;
}

//-------------------------------------------------------------------

float GroundScene::getCameraFarPlaneSpace()
{
	return ms_cameraFarPlaneSpace;
}

//-------------------------------------------------------------------

void GroundScene::setCameraFarPlane (const float farPlane)
{
	ms_cameraFarPlane = farPlane;

	if (Game::isSpace())
		return;

	ShadowManager::setVolumetricShadowDistanceLevel ((farPlane - 512.f) / (2048.f - 512.f));

	GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
	if (groundScene)
	{
		float const actualFarPlane = groundScene->isTutorial() ? cms_tutorialFarPlaneDistance : farPlane;
		int i;
		for (i = 0; i < groundScene->getNumberOfViews (); ++i)
			groundScene->getCamera (i)->setFarPlane (actualFarPlane);
	}
}

//----------------------------------------------------------------------

void GroundScene::setCameraFarPlaneSpace(const float farPlane)
{
	ms_cameraFarPlaneSpace = farPlane;

	if (!Game::isSpace())
		return;

	ShadowManager::setVolumetricShadowDistanceLevel ((farPlane - 512.f) / (2048.f - 512.f));

	GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
	if (groundScene)
	{
		bool const isSpace = Game::isSpace();

		float const actualFarPlane = groundScene->isTutorial() ? cms_tutorialFarPlaneDistance : (isSpace ? farPlane : farPlane);
		int i;
		for (i = 0; i < groundScene->getNumberOfViews (); ++i)
			groundScene->getCamera (i)->setFarPlane (actualFarPlane);
	}
}
//----------------------------------------------------------------------

bool  GroundScene::getInvertMouseLook          ()
{
	return ms_invertMouse;
}

//----------------------------------------------------------------------

void  GroundScene::setInvertMouseLook          (bool b)
{
	ms_invertMouse = b;
}


//----------------------------------------------------------------------

bool  GroundScene::getListenerFollowsPlayer          ()
{
	return ms_listenerFollowsPlayer;
}

//----------------------------------------------------------------------

void  GroundScene::setListenerFollowsPlayer          (bool b)
{
	ms_listenerFollowsPlayer = b;
}

//-------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
#if 0
#include "sharedObject/PortalProperty.h"
static void SkeletalAppearanceLoadTest()
{
	const char *objectNames[] =
	{
		"object/creature/player/shared_human_male.iff",
		"object/creature/player/shared_human_female.iff",
		"object/creature/player/shared_bothan_female.iff",
		"object/creature/player/shared_bothan_male.iff",
		"object/creature/player/shared_moncal_female.iff",
		"object/creature/player/shared_moncal_male.iff",
		"object/creature/player/shared_rodian_female.iff",
		"object/creature/player/shared_rodian_male.iff",
		"object/creature/player/shared_trandoshan_female.iff",
		"object/creature/player/shared_trandoshan_male.iff",
		"object/creature/player/shared_twilek_female.iff",
		"object/creature/player/shared_twilek_male.iff",
		"object/creature/player/shared_wookiee_female.iff",
		"object/creature/player/shared_wookiee_male.iff",
		"object/creature/player/shared_zabrak_female.iff",
		"object/creature/player/shared_zabrak_male.iff",
		"object/creature/player/shared_ithorian_female.iff",
		"object/creature/player/shared_ithorian_male.iff"
		"object/creature/player/shared_sullustan_female.iff",
		"object/creature/player/shared_sullustan_male.iff"
	};
	int numObjects = sizeof(objectNames) / sizeof(*objectNames);

	const Vector playerPosition = dynamic_cast<NetworkScene*>(Game::getScene ())->getPlayer()->getPosition_w();

	int index = 0;
	for (int x = -4; x < 4; ++x)
	{
		for (int z = -4; z < 4; ++z, ++index)
		{
			if (index==numObjects)
			{
				index=0;
			}

			// Create the object.
			ClientObject * const obj = safe_cast<ClientObject *>(ObjectTemplate::createObject (objectNames[index]));

			if (!obj)
			{
				REPORT_LOG_PRINT(true, ("failed to create an object from specified object template\n"));
				continue;
			}

			// Setup position.			
			Vector v = playerPosition + Vector(static_cast<float>(x) * 2, 0, static_cast<float>(z) * 2);
			const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
			if (terrainObject)
			{
				float height;
				if (terrainObject->getHeight (v, height))
					v.y = height;
			}

			char buf [128];
			IGNORE_RETURN (_itoa (static_cast<int>(obj->getNetworkId().getValue()), buf, 10));

			obj->setPosition_p (v);
			obj->setObjectName (Unicode::narrowToWide ("Object #") + Unicode::narrowToWide (buf));

			RenderWorld::addObjectNotifications (*obj);

			PortalProperty* const property = obj->getPortalProperty ();
			if (property)
				property->clientSinglePlayerInitializeFirstTimeObject ();

			// Add object to world.
			obj->endBaselines ();
			obj->addToWorld();
		}
	}
}
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void GroundScene::init (const char* const terrainFilename, CreatureObject* const player, const float timeInSeconds)
{
	DEBUG_REPORT_LOG (true, ("+=======================================================================\n"));
	DEBUG_REPORT_LOG (true, ("+ GroundScene::GroundScene\n"));
	DEBUG_REPORT_LOG (true, ("+    terrain = %s\n", terrainFilename));
	DEBUG_REPORT_LOG (true, ("+    player object id = %s\n", player->getNetworkId ().getValueString ().c_str ()));
	DEBUG_REPORT_LOG (true, ("+    player object template = %s\n", player->getObjectTemplateName ()));
	DEBUG_REPORT_LOG (true, ("+    player container = %s\n", player->getParentCell () ? player->getParentCell ()->getOwner ().getNetworkId ().getValueString ().c_str () : "null"));
	DEBUG_REPORT_LOG (true, ("+    player cell position = %1.2f, %1.2f, %1.2f\n", player->getPosition_p ().x, player->getPosition_p ().y, player->getPosition_p ().z));
	DEBUG_REPORT_LOG (true, ("+    player world position = %1.2f, %1.2f, %1.2f\n", player->getPosition_w ().x, player->getPosition_w ().y, player->getPosition_w ().z));
	DEBUG_REPORT_LOG (true, ("+    player yaw = %1.2f\n", player->getObjectFrameK_w().theta()));
	DEBUG_REPORT_LOG (true, ("+=======================================================================\n"));

	DEBUG_OUTPUT_CHANNEL("Game\\GroundScene", ("GroundScene::GroundScene - starting %s...\n", terrainFilename));

	player->addNotification(DebugNotification::getInstance());

	IoWinManager::discardUserInputUntilNextProcessEvents();

	//-- install all systems
	ClientWorld::install ();
	RenderWorld::setDisableOcclusionCulling(strstr(terrainFilename, "space_") != 0);
	AlterScheduler::setPostAlterHookFunction (CollisionWorld::update);
	WeatherManager::addWeatherChangedFunction (ParticleEffectAppearance::setGlobalWind);

	//-- cameras have to be created first to load the lights
	m_cockpitCamera = new CockpitCamera();
	m_cameras[CI_cockpit] = m_cockpitCamera;
	m_cockpitCamera->setActive(false);
	ClientWorld::addCamera(m_cockpitCamera);

	m_shipTurretCamera = new ShipTurretCamera();
	m_cameras[CI_shipTurret] = m_shipTurretCamera;
	m_shipTurretCamera->setActive(false);
	ClientWorld::addCamera(m_shipTurretCamera);

	m_freeChaseCamera        = NON_NULL (new FreeChaseCamera ());
	m_cameras [CI_freeChase] = m_freeChaseCamera;
	m_freeChaseCamera->setActive (false);
	ClientWorld::addCamera (m_freeChaseCamera);

	m_freeCamera             = NON_NULL (new FreeCamera ());
	m_cameras [CI_free]      = m_freeCamera;
	m_freeCamera->setActive (false);
	ClientWorld::addCamera (m_freeCamera);

	m_debugPortalCamera             = NON_NULL (new DebugPortalCamera ());
	m_cameras [CI_debugPortal]      = m_debugPortalCamera;
	m_debugPortalCamera->setActive (false);
	ClientWorld::addCamera (m_debugPortalCamera);

	m_structurePlacementCamera        = NON_NULL (new StructurePlacementCamera ());
	m_cameras [CI_structurePlacement] = m_structurePlacementCamera;
	m_structurePlacementCamera->setActive (false);
	ClientWorld::addCamera (m_structurePlacementCamera);

	m_flyByCamera = NON_NULL(new FlyByCamera());
	m_cameras[CI_flyBy] = m_flyByCamera;
	m_flyByCamera->setActive(false);
	ClientWorld::addCamera(m_flyByCamera);

	//-- preload the mission
	preload ();

	//-- load the mission
	load (terrainFilename, timeInSeconds);

	//-- validate the player's position
	TerrainObject const *const terrain = TerrainObject::getInstance();
	if (terrain)
	{
		// It's okay if this object is contained and the position is not in world space.
		// This is still a good upper-bound sanity check on the value.
		Vector const newPosition = player->getPosition_w ();
		if (!terrain->isWithinTerrainBoundaries (newPosition))
		{
			WARNING (true, ("GroundScene::init (): object id=[%s] received invalid objectToParent position x=[%0.2f], y=[%0.2f], z=[%0.2f], setting to 1,1,1.", player->getNetworkId ().getValueString ().c_str (), newPosition.x, newPosition.y, newPosition.z));
			player->setPosition_w (Vector::xyz111);
		}
	}

	//-----------------------------------------------------------------
	//-- setup the player controller

	PlayerCreatureController* const controller = NON_NULL (new PlayerCreatureController (player));
	player->setController (controller);
	setPlayer (player);

	//-- post load mission
	postload ();

	GroundScene::setView (CI_freeChase);

	CameraController* const freeCameraController = NON_NULL (new CameraController (m_freeCamera));
	m_freeCamera->setController (freeCameraController);

	m_freeCameraInputMap = new InputMap ("input/groundinputmap_freecamera.iff", 0, 0);
	m_freeCameraInputMap->setMessageQueue (freeCameraController->getMessageQueue ());

	CameraController* const debugPortalCameraController = NON_NULL (new CameraController (m_debugPortalCamera));
	m_debugPortalCamera->setController (debugPortalCameraController);

	m_debugPortalCameraInputMap = new InputMap ("input/groundinputmap_debugportalcamera.iff", 0, 0);
	m_debugPortalCameraInputMap->setMessageQueue (debugPortalCameraController->getMessageQueue ());

	CameraController* const structurePlacementCameraController = NON_NULL (new CameraController (m_structurePlacementCamera));
	m_structurePlacementCamera->setController (structurePlacementCameraController);

	m_structurePlacementCameraInputMap = new InputMap ("input/groundinputmap_swg.iff", 0, 0);
	m_structurePlacementCameraInputMap->setMessageQueue (structurePlacementCameraController->getMessageQueue ());

	//-- set anything that needs to be set AFTER the scene loads
	NOT_NULL (player);

	//-----------------------------------------------------------------
	//--

	m_cockpitCamera->setTarget(player);
	m_cockpitCamera->setMessageQueue(controller->getMessageQueue());

	m_shipTurretCamera->setTarget(player);
	m_shipTurretCamera->setMessageQueue(controller->getMessageQueue());

	m_freeChaseCamera->setTarget (player);
	m_freeChaseCamera->setMessageQueue (controller->getMessageQueue ());

	m_freeCamera->setTarget (player);
	m_freeCamera->setMessageQueue (m_freeCameraInputMap->getMessageQueue ());

	m_debugPortalCamera->setTarget (player);
	m_debugPortalCamera->setMessageQueue (m_debugPortalCameraInputMap->getMessageQueue ());

	m_structurePlacementCamera->setTarget (player);
	m_structurePlacementCamera->setMessageQueue (m_structurePlacementCameraInputMap->getMessageQueue ());

	//-- put iowin on stack
	IoWin::open ();

	Game::_setScene(this);

	//Don't start the loading process until the scene has been set
	CuiLoadingManager::setPlanetTerrainFilename(terrainFilename);
	CuiLoadingManager::setFullscreenLoadingEnabled (true);
	CuiLoadingManager::setFullscreenBackButtonEnabled (false);
	CuiLoadingManager::setFullscreenLoadingString (CuiStringIds::loading.localize ());

	loadInputMap();

	//--
	FileName fileName (terrainFilename);
	fileName.stripPathAndExt ();
	std::string buildoutAreaName = SharedBuildoutAreaManager::getBuildoutNameForPosition( std::string(fileName.getString()), player->getPosition_w(), true );
	GameMusicManager::install ( buildoutAreaName.c_str() );

	//--
	start ();

	//SkeletalAppearanceLoadTest();

	connectToMessage ("SceneCreateObjectByCrc");
	connectToMessage ("SceneDestroyObject");
	connectToMessage ("SceneEndBaselines");
	connectToMessage ("ServerTimeMessage");
	connectToMessage (ServerWeatherMessage::cms_name);
	connectToMessage ("BaselinesMessage");
	connectToMessage ("DeltasMessage");
	connectToMessage ("UpdatePostureMessage");
	connectToMessage ("UpdateCellPermissionMessage");
	connectToMessage ("UpdateTransformMessage");
	connectToMessage ("UpdateTransformWithParentMessage");
	connectToMessage ("PlayMusicMessage");
	connectToMessage ("ClientOpenContainerMessage");
	connectToMessage (UnnamedMessages::ConnectionServerConnectionClosed);
	connectToMessage (NewbieTutorialRequest::cms_name);
	connectToMessage (CuiIoWin::Messages::POINTER_INPUT_TOGGLED);
	connectToMessage ("LogoutMessage");
	connectToMessage ("DebugTransformMessage");
	connectToMessage ("UpdateContainmentMessage");
	connectToMessage (CreateClientPathMessage::cms_name);
	connectToMessage (DestroyClientPathMessage::cms_name);
	connectToMessage ("CmdSceneReady");
	connectToMessage (UpdatePvpStatusMessage::MessageType);
	connectToMessage ("CreateProjectileMessage");
	connectToMessage (CreateNebulaLightningMessage::MessageType);
	connectToMessage ("CreateMissileMessage");
	connectToMessage ("UpdateMissileMessage");
	connectToMessage ("ShipUpdateTransformMessage");
	connectToMessage ("ShipUpdateTransformCollisionMessage");
	connectToMessage (SlowDownEffectMessage::MessageType);
	connectToMessage (DestroyShipComponentMessage::MessageType);
	connectToMessage (DestroyShipMessage::MessageType);
	connectToMessage ("CommPlayerMessage");
	connectToMessage ("HyperspaceMessage");
	connectToMessage ("IsFlattenedTheaterMessage");
	connectToMessage ("AiDebugString");
	connectToMessage ("SaveTextOnClient");
	connectToMessage ("LaunchBrowserMessage");
	connectToMessage ("SetDefaultAction");
	connectToMessage ("SetCommandCooldown");
	connectToMessage ("fca11a62d23041008a4f0df36aa7dca6"); // Player Server Speed Message
}

//-------------------------------------------------------------------
/** This constructor is called for single player mode
*
*/
GroundScene::GroundScene(
	const char *const     terrainFilename, 
	const char *const     playerFilename,
	CreatureObject *const customizedPlayer
	)
:	NetworkScene("GroundScene"),
	m_inputMap (0),
	m_debugPortalCameraInputMap (0),
	m_structurePlacementCameraInputMap (0),
	m_freeCameraInputMap (0),
	m_mouseCursor (new MouseCursor (0, MouseCursor::S_relative)),
	m_lastYawPitchMod (new Vector2d),
	m_cockpitCamera(0),
	m_shipTurretCamera(0),
	m_freeChaseCamera (0),
	m_freeCamera (0),
	m_debugPortalCamera (0),
	m_structurePlacementCamera (0),
	m_flyByCamera(NULL),
	m_currentView (-1),
	m_disableWorldSnapshot(ConfigClientGame::getDisableWorldSnapshot()),
	m_usingGodClientCamera(false),
	m_usingGodClientInteriorCamera(false),
	m_loading (true),
	m_sentSceneChannel(false),
	m_receivedSceneReady(true), // in single player the server doesn't need to be ready
	m_noDraw(false),
	m_currentLoadCount (0),
	m_debugKeyContext (0),
	m_debugKeySubContext (),
	m_debugKeyContextWeaponObjectTemplate (0),
	m_serverTimeOffset (0),
	m_overheadMap (new OverheadMap),
	m_modeCallback (0),
	m_context (0),
	m_currentMode (false),
	m_spaceTargetBracketOverlay(new SpaceTargetBracketOverlay),
	m_clientPathObject (0),
	m_debugPointList (new PointList),
	m_debugLineList (new LineList),
	m_isTutorial (false),
	m_destroyObjectSet(new DestroyObjectSet),
	m_destroyObjectTimer(Random::randomReal(0.5f, 1.f))
{
	Audio::setLargePreMixBuffer();
	Audio::silenceAllNonBackgroundMusic();

	ms_loadingScreenRender = true;
	ms_loadingScreenRenderYaw = 0.0f;

	//-- create the player
	CreatureObject * player = customizedPlayer;

	if (!player)
	{
		// must not have gone through customization screen or customization screen couldn't create a valid object.
		// Note: no customization parameters are present this way.

		// Force player to load synchronously so we have valid mesh generators for the player.
		bool const wasEnabled = AsynchronousLoader::isEnabled();
		AsynchronousLoader::disable();

		player = dynamic_cast<CreatureObject*>(ObjectTemplate::createObject(playerFilename));
		FATAL (!player, ("cannot load template %s\n",playerFilename));

		if (wasEnabled)
			AsynchronousLoader::enable();
	}

	player->setPosition_p(ConfigClientGame::getSinglePlayerStartLocation());

	//-- make sure player gets drawn.
	NOT_NULL(player);
	RenderWorld::addObjectNotifications(*player);
	CellProperty::addPortalCrossingNotification(*player);

	player->setObjectName (Unicode::narrowToWide (ConfigClientGame::getPlayerName ()));
	player->setDebugName  (ConfigClientGame::getPlayerName ());

	//-- make sure the player's animation state reflects items the game may start the player with.
	//   Normally this is handled via the container interface, but in this case the container system
	//   is bypassed.
	player->setAppearanceHeldItemState ();

	init (terrainFilename, player, ConfigClientTerrain::getEnvironmentStartTime ());

	player->endBaselines ();

	if (!player->isInWorld ())
		player->addToWorld ();

	CollisionProperty * collision = player->getCollisionProperty();
	if(collision)
		collision->setPlayer(true);

	Controller *const controller = player->getController ();
	NOT_NULL (controller);
	safe_cast<PlayerCreatureController *>(controller)->setAuthoritative (true);
}

//-----------------------------------------------------------------------
/** this constructor is called for multi-player mode
*
*/

GroundScene::GroundScene(
	const char *const terrainFilename, 
	const NetworkId & playerOID, 
	const char *const templateName, 
	const Vector &    startPosition, 
	const float       startYaw, 
	const float       timeInSeconds, 
	const bool        disableSnapshot
	)
:	NetworkScene ("GroundScene"),
	m_inputMap (0),
	m_debugPortalCameraInputMap (0),
	m_structurePlacementCameraInputMap (0),
	m_freeCameraInputMap (0),
	m_mouseCursor (new MouseCursor (0, MouseCursor::S_relative)),
	m_lastYawPitchMod (new Vector2d),
	m_cockpitCamera(0),
	m_shipTurretCamera(0),
	m_freeChaseCamera (0),
	m_freeCamera (0),
	m_debugPortalCamera (0),
	m_structurePlacementCamera (0),
	m_flyByCamera(NULL),
	m_currentView (-1),
	m_disableWorldSnapshot(disableSnapshot),
	m_usingGodClientCamera(false),
	m_usingGodClientInteriorCamera(false),
	m_loading (true),
	m_sentSceneChannel(false),
	m_receivedSceneReady(false),
	m_noDraw(false),
	m_currentLoadCount (0),
	m_debugKeyContext (0),
	m_debugKeySubContext (),
	m_debugKeyContextWeaponObjectTemplate (0),
	m_serverTimeOffset (0),
	m_overheadMap (new OverheadMap),
	m_modeCallback (0),
	m_context (0),
	m_currentMode (false),
	m_spaceTargetBracketOverlay(new SpaceTargetBracketOverlay),
	m_clientPathObject (0),
	m_debugPointList (new PointList),
	m_debugLineList (new LineList),
	m_isTutorial (false),
	m_destroyObjectSet(new DestroyObjectSet),
	m_destroyObjectTimer(Random::randomReal(0.5f, 1.f))
{
	Audio::setLargePreMixBuffer();
	Audio::silenceAllNonBackgroundMusic();

	ms_loadingScreenRender = true;
	ms_loadingScreenRenderYaw = 0.0f;

	//-- Create the player.  Disable asynchronous loading so character
	//   is properly loaded and baked at construction time.
	bool const wasEnabled = AsynchronousLoader::isEnabled();
	AsynchronousLoader::disable();

	CreatureObject* player = dynamic_cast<CreatureObject*>(ObjectTemplate::createObject(templateName));
	FATAL (!player, ("cannot load template %s\n", templateName));

	if (wasEnabled)
		AsynchronousLoader::enable();

	player->setNetworkId(playerOID);
	player->yaw_o(startYaw);
	player->setPosition_p(startPosition);
	RenderWorld::addObjectNotifications(*player);
	CellProperty::addPortalCrossingNotification(*player);

	CollisionProperty * collision = player->getCollisionProperty();
	if(collision)
		collision->setPlayer(true);

	// TODO: remove this and retrieve the name from the ConnectionManager-alike
	player->setDebugName (ConfigClientGame::getPlayerName ());

	init (terrainFilename, player, timeInSeconds);
}

//-----------------------------------------------------------------

GroundScene::~GroundScene (void)
{
	IGNORE_RETURN (CuiCachedAvatarManager::saveScenePlayer ());

	Audio::stopAllSounds ();

	GameMusicManager::remove ();

	if (m_loading)
	{
		_onFinishedLoading();
	}

	if (Game::getScene() == this)
	{
		Game::_setScene(0);
	}

	//--
	if (m_inputMap)
	{
		m_inputMap->setMessageQueue        (0);
		InputScheme::releaseGroundInputMap (*m_inputMap);
		m_inputMap = 0;
	}

	Object * const player = getPlayer ();
	if (player)
	{
		setPlayer (0);
		delete player;
	}

	//-- delete cameras
	ClientWorld::removeCamera(m_cockpitCamera);
	delete m_cockpitCamera;
	m_cockpitCamera = 0;

	ClientWorld::removeCamera(m_shipTurretCamera);
	delete m_shipTurretCamera;
	m_shipTurretCamera = 0;

	ClientWorld::removeCamera (m_freeChaseCamera);
	delete m_freeChaseCamera;
	m_freeChaseCamera = 0;

	ClientWorld::removeCamera (m_freeCamera);
	delete m_freeCamera;
	m_freeCamera = 0;

	ClientWorld::removeCamera (m_debugPortalCamera);
	delete m_debugPortalCamera;
	m_debugPortalCamera = 0;

	ClientWorld::removeCamera (m_structurePlacementCamera);
	delete m_structurePlacementCamera;
	m_structurePlacementCamera = 0;

	ClientWorld::removeCamera(m_flyByCamera);
	delete m_flyByCamera;
	m_flyByCamera = 0;

	//-- remove all systems
	AlterScheduler::setPostAlterHookFunction (0);
	ClientWorld::remove ();
	WeatherManager::removeWeatherChangedFunction (ParticleEffectAppearance::setGlobalWind);

	delete m_freeCameraInputMap;
	m_freeCameraInputMap = 0;

	delete m_debugPortalCameraInputMap;
	m_debugPortalCameraInputMap = 0;

	delete m_structurePlacementCameraInputMap;
	m_structurePlacementCameraInputMap = 0;

	delete m_mouseCursor;
	m_mouseCursor = 0;

	delete m_lastYawPitchMod;
	m_lastYawPitchMod = 0;

	if (m_debugKeyContextWeaponObjectTemplate)
		m_debugKeyContextWeaponObjectTemplate->releaseReference ();

	delete m_overheadMap;
	delete m_spaceTargetBracketOverlay;

	delete m_debugPointList;
	delete m_debugLineList;

	delete m_destroyObjectSet;
}

//-------------------------------------------------------------------

void GroundScene::setView (int newView, float value)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, newView, getNumberOfViews ());
	UNREF (value);

	newView = clamp (0, newView, getNumberOfViews () - 1);

	if (m_currentView != -1 && m_currentView != newView)
	{
		//-- reset the old view
		if (m_currentView != -1)
		{
			if (m_currentView == CI_cockpit)
			{
				if (m_inputMap)
					m_inputMap->handleInputReset ();
			}

			if (m_currentView == CI_shipTurret)
			{
				if (m_inputMap)
					m_inputMap->handleInputReset ();
			}

			if (m_currentView == CI_freeChase)
			{
				if (m_inputMap)
					m_inputMap->handleInputReset ();
			}

			if (m_currentView == CI_free)
			{
				if (m_freeCameraInputMap)
					m_freeCameraInputMap->handleInputReset ();
			}

			if (m_currentView == CI_debugPortal)
			{
				if (m_debugPortalCameraInputMap)
					m_debugPortalCameraInputMap->handleInputReset ();
			}

			if (m_currentView == CI_structurePlacement)
			{
				if (m_structurePlacementCameraInputMap)
					m_structurePlacementCameraInputMap->handleInputReset ();
			}

			m_cameras [m_currentView]->setActive (false);
		}

		if (newView == CI_free)
		{
			NOT_NULL (m_freeCameraInputMap);
			m_freeCameraInputMap->beginFrame ();
			m_freeCameraInputMap->beginFrame ();
		}

		if (newView == CI_debugPortal)
		{
			NOT_NULL (m_debugPortalCameraInputMap);
			m_debugPortalCameraInputMap->beginFrame ();
			m_debugPortalCameraInputMap->beginFrame ();
		}

		if (newView == CI_structurePlacement)
		{
			NOT_NULL (m_structurePlacementCameraInputMap);
			m_structurePlacementCameraInputMap->beginFrame ();
			m_structurePlacementCameraInputMap->beginFrame ();
		}
	}

	m_currentView = newView;
	m_cameras [m_currentView]->setActive (true);
}

//-------------------------------------------------------------------

int GroundScene::getNumberOfViews (void) const
{
	return CI_COUNT;
}

//-------------------------------------------------------------------

int GroundScene::getCurrentView () const
{
	return m_currentView;
}

//-------------------------------------------------------------------

GameCamera* GroundScene::getCamera (int view)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, view, getNumberOfViews ());
	return m_cameras [view];
} //lint !e1762 // logically nonconst

//-------------------------------------------------------------------

const GameCamera* GroundScene::getCamera (int view) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, view, getNumberOfViews ());
	return m_cameras [view];
}

//-------------------------------------------------------------------

GameCamera* GroundScene::getCurrentCamera ()
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, m_currentView, getNumberOfViews ());
	return m_cameras [m_currentView];
} //lint !e1762 // logically nonconst

//-------------------------------------------------------------------

const GameCamera* GroundScene::getCurrentCamera (void) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, m_currentView, getNumberOfViews ());
	return m_cameras [m_currentView];
}

//-------------------------------------------------------------------

void GroundScene::reloadTerrain ()
{
	TerrainObject* const terrain = TerrainObject::getInstance ();
	if (terrain)
	{
		if (terrain->isInWorld ())
			terrain->removeFromWorld ();

		NOT_NULL (terrain->getAppearance ());
		NOT_NULL (terrain->getAppearance ()->getAppearanceTemplate ());
		NOT_NULL (terrain->getAppearance ()->getAppearanceTemplate ()->getName ());

		char* const terrainFileName = DuplicateString (terrain->getAppearance ()->getAppearanceTemplate ()->getName ());
		DEBUG_REPORT_LOG(true, ("reloading %s", terrainFileName));

		delete TerrainObject::getInstance ();
		AppearanceTemplateList::garbageCollect();
		load (terrainFileName, ConfigClientTerrain::getEnvironmentStartTime ());

		delete [] terrainFileName;
	}
}

//-------------------------------------------------------------------

void GroundScene::load (const char* const terrainFileName, const float timeInSeconds)
{
	snprintf(ms_terrainName, sizeof(ms_terrainName), "Terrain: %s\n", terrainFileName);

	FATAL(!TreeFile::exists(terrainFileName), ("%s is not a valid terrain file", terrainFileName));

	TerrainObject * const terrainObject = new TerrainObject(ClientWorld::getIntangibleNotification());
	terrainObject->setDebugName("terrain");
	TerrainObject::setUseCache(false);
	RenderWorld::addObjectNotifications(*terrainObject);

	Appearance * const appearance = AppearanceTemplateList::createAppearance(terrainFileName);
	terrainObject->setAppearance(appearance);

	ProceduralTerrainAppearance * const proceduralTerrainAppearance = dynamic_cast<ProceduralTerrainAppearance *>(appearance);
	if (proceduralTerrainAppearance)
		ClientObjectTerrainModificationNotification::setTerrainAppearance(proceduralTerrainAppearance);

	terrainObject->addToWorld();

	//-- allow override of passed in start time with a normalized start time if requested.

	if (!terrainObject->isTimeLocked())
	{
		float normalizedTime;

		if (ConfigClientTerrain::getUseNormalizedTime ())
			normalizedTime = fmodf (ConfigClientTerrain::getEnvironmentNormalizedStartTime (), 1.f);
		else
			normalizedTime = fmodf (timeInSeconds, terrainObject->getEnvironmentCycleTime ()) / terrainObject->getEnvironmentCycleTime ();

		terrainObject->setTime (normalizedTime, true);
	}

	m_isTutorial = strstr (terrainFileName, "tutorial") != 0;

	bool const isSpace = strstr(terrainFileName, "space_") != 0;

		//TODO this belongs in a SpaceScene class, once we make one
		ClientAsteroidManager::setupStaticFields(Game::getSceneIdFromTerrainFilename(terrainFileName));

	//-- set camera preferences
	int i;
	for (i = 0; i < getNumberOfViews (); i++)
	{
		m_cameras [i]->setNearPlane (ConfigClientGame::getCameraNearPlane ());
		m_cameras [i]->setFarPlane  (isTutorial () ? cms_tutorialFarPlaneDistance : (isSpace ? ms_cameraFarPlaneSpace : ms_cameraFarPlane));
		m_cameras [i]->setHorizontalFieldOfView (convertDegreesToRadians (ms_cameraFieldOfViewDegrees));
	}

	//-- Tell ShaderPrimitiveSetTemplate it doesn't need to create the upward indices
	ShaderPrimitiveSetTemplate::setAllowUpwardIndicesCreation(!isSpace);

#if PRODUCTION == 1
	FATAL(isSpace && proceduralTerrainAppearance, ("0xA50MM3R5 unknown error 1"));
	FATAL(!isSpace && !proceduralTerrainAppearance, ("0xA50MM3R5 unknown error 2"));
#endif
}

//-------------------------------------------------------------------

void GroundScene::preload (void)
{
}

//-------------------------------------------------------------------

void GroundScene::postload (void)
{
	FileName fileName (TerrainObject::getConstInstance ()->getAppearance ()->getAppearanceTemplate ()->getName ());
	fileName.stripPathAndExt ();

	if (!m_disableWorldSnapshot)
	{
		WorldSnapshot::load(fileName);
	}
	else
	{
		SharedBuildoutAreaManager::load(fileName.getString());
	}

	SpacePreloadedAssetManager::load(fileName);
}

//-------------------------------------------------------------------

void GroundScene::scanInputMapForSceneMessages (InputMap * inputMap)
{
	NOT_NULL (inputMap);
	const MessageQueue* const queue = inputMap->getMessageQueue ();
	NOT_NULL (queue);

	bool resetInputMap = false;

	int i;
	for (i = 0; i < queue->getNumberOfMessages (); i++)
	{
		int  message;
		float value;

		queue->getMessage (i, &message, &value);

		switch (message)
		{
		case CM_freeChaseCamera:
			{
				if (m_usingGodClientInteriorCamera)
					setView (CI_free);
				else
				{
					int view = CI_freeChase;

					CreatureObject const * const player = safe_cast<CreatureObject const *>(getPlayer());
					if (player)
					{
						int const shipStation = player->getShipStation();
						if (shipStation == ShipStation::ShipStation_Pilot || shipStation == ShipStation::ShipStation_Operations)
							view = CI_cockpit;
						else if (shipStation >= ShipStation::ShipStation_Gunner_First && shipStation <= ShipStation::ShipStation_Gunner_Last)
							view = CI_shipTurret;
					}
					setView(view);
				}

				resetInputMap = true;
			}
			break;

		case CM_uiGameMenuActivate:
			resetInputMap = true;
			break;

		case CM_radarMap:
			m_overheadMap->toggle ();
			break;

		case CM_radarZoom:
			m_overheadMap->zoom (value > 0);
			break;

#if PRODUCTION == 0

		case CM_freeCamera:
			setView (CI_free);
			resetInputMap = true;
			break;

		case CM_debugPortalCamera:
			setView (CI_debugPortal);
			resetInputMap = true;
			break;

		case CM_fillMode:
			{
				static bool solid = true;
				if (solid)
				{
					solid = false;
					Graphics::setFillMode (GFM_wire);
				}
				else
				{
					solid = true;
					Graphics::setFillMode (GFM_solid);
				}
			}
			break;

		case CM_warpPlayerToFreeCamera:
			{
				if (getCurrentView () == CI_free || getCurrentView () == CI_debugPortal)
				{
					if (Game::getSinglePlayer ())
					{
						Camera const * const camera = getCurrentCamera ();
						CellProperty const * const cellProperty = camera->getParentCell ();
						Object * const player = getPlayer ();

						player->setParentCell (const_cast<CellProperty *> (cellProperty));
						CellProperty::setPortalTransitionsEnabled (false);
							player->setTransform_o2p (camera->getTransform_o2p ());
						CellProperty::setPortalTransitionsEnabled (true);

						CollisionWorld::objectWarped (player);
					}
					else
					{
						Camera const * const camera = getCurrentCamera ();
						NetworkId const & networkId = camera->getParentCell ()->getOwner ().getNetworkId ();
						Vector const & position_w = camera->getPosition_w ();
						Vector const & position_p = camera->getPosition_p ();

						char buffer [1024];
						IGNORE_RETURN(snprintf(buffer, sizeof(buffer)-1, "%1.2f %1.2f %1.2f %s %1.2f %1.2f %1.2f", position_w.x, position_w.y, position_w.z, networkId.getValueString().c_str(), position_p.x, position_p.y, position_p.z));
						buffer[sizeof(buffer)-1] = '\0';
						IGNORE_RETURN(ClientCommandQueue::enqueueCommand ("teleport", NetworkId::cms_invalid, Unicode::narrowToWide (buffer)));
					}
				}
			}
			break;

		case CM_environmentPause:
			{
				// please ignore the const_cast behind the curtain! - jww
				TerrainObject * const terrainObject = TerrainObject::getInstance ();
				if (terrainObject)
				{
					terrainObject->setPauseEnvironment (!terrainObject->getPauseEnvironment ());

					if (terrainObject->getPauseEnvironment ())
						CuiSystemMessageManager::sendFakeSystemMessage (Unicode::narrowToWide ("GroundEnvironment paused"));
					else
						CuiSystemMessageManager::sendFakeSystemMessage (Unicode::narrowToWide ("GroundEnvironment resumed"));
				}
			}
			break;

		case CM_terrainShowLodBorderChunks:
			ClientProceduralTerrainAppearance::setShowChunkExtents (!ClientProceduralTerrainAppearance::getShowChunkExtents ());
			break;

		case CM_terrainShowLod:
			{
				NOT_NULL (TerrainObject::getInstance ());
				ClientProceduralTerrainAppearance * cmtat = dynamic_cast <ClientProceduralTerrainAppearance *> (TerrainObject::getInstance ()->getAppearance ());
				if (cmtat)
					ClientProceduralTerrainAppearance::setShowChunkExtents (!ClientProceduralTerrainAppearance::getShowChunkExtents ());
			}
			break;

		case CM_pause:
			IGNORE_RETURN (new PauseGame (getCurrentCamera (), false));
			break;

		case CM_panorama:
			{
				// used for debugging
				//IGNORE_RETURN(new HyperspaceIoWin(m_cockpitCamera, Game::getPlayerContainingShip(), HyperspaceIoWin::S_enter, true));

				if (ms_testSpaceDeathUsingPanorama)
					IGNORE_RETURN(new SpaceDeath(getCurrentCamera(), Game::getPlayerContainingShip(), 0.75f, true));
				else
					IGNORE_RETURN(new Panorama(getCurrentCamera (), true));
			}
			break;

		case CM_screenShot360:
			{
				switch (m_currentView)
				{
				default:
				case CI_freeChase:
					{
						NOT_NULL (m_freeChaseCamera);
						if (m_freeChaseCamera->getTarget ())
							IGNORE_RETURN (new ScreenShot360 (m_freeChaseCamera, m_freeChaseCamera->getParentCell (), m_freeChaseCamera->getTarget ()->getTransform_o2p (), m_freeChaseCamera->getOffset (), m_freeChaseCamera->getPitch (), m_freeChaseCamera->getYaw (), m_freeChaseCamera->getZoom (), m_freeChaseCamera->getRadius ()));
					}
					break;
				}
			}
			break;


		case CM_debugKeyContext1:
			handleDebugKeyContextKey1 ();
			break;

		case CM_debugKeyContext2:
			handleDebugKeyContextKey2 ();
			break;

		case CM_vtuneResume:
			VTune::resume();
			CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("VTune Resumed"));
			break;

		case CM_vtunePause:
			VTune::pause();
			CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("VTune Paused"));
			break;

#endif

		default:
			break;
		}
	}

	if (resetInputMap)
		inputMap->handleInputReset ();

}

//-------------------------------------------------------------------

void GroundScene::handleInputMapEvent (IoEvent* event)
{
	//-- handle the cursor
	m_mouseCursor->processEvent (event);

	//-- handle mouse wheel
	bool handleMouseWheel = event->type == IOET_MouseMove && event->arg2 == 2;

	//--
	InputMap* inputMap = 0;
	switch (m_currentView)
	{
	case CI_free:
		inputMap = m_freeCameraInputMap;
		handleMouseWheel = false;
		break;

	case CI_debugPortal:
		inputMap = m_debugPortalCameraInputMap;
		handleMouseWheel = false;
		break;

	case CI_structurePlacement:
		inputMap = m_structurePlacementCameraInputMap;
		break;

	default:
		inputMap = m_inputMap;
		break;
	}

	if (inputMap)
	{
		if (handleMouseWheel)
		{
			if (inputMap->getShiftState () == 2)  //-- control key
				inputMap->getMessageQueue ()->appendMessage (CM_radarZoom, event->arg3);
			else
				inputMap->getMessageQueue ()->appendMessage (CM_mouseWheel, event->arg3);
		}

		inputMap->processEvent (event);

		if (event->type == IOET_Update)
			inputMap->handleRepeats (event->arg3);
	}
}

//-------------------------------------------------------------------

void GroundScene::handleInputMapScan (void)
{
	switch (m_currentView)
	{
	case CI_free:
		{
			//-- send this to the freeCamera inputmap
			NOT_NULL (m_freeCameraInputMap);
			scanInputMapForSceneMessages (m_freeCameraInputMap);
		}
		break;

	case CI_debugPortal:
		{
			//-- send this to the debugPortalCamera inputmap
			NOT_NULL (m_debugPortalCameraInputMap);
			scanInputMapForSceneMessages (m_debugPortalCameraInputMap);
		}
		break;

	case CI_structurePlacement:
		{
			//-- send this to the debugPortalCamera inputmap
			NOT_NULL (m_structurePlacementCameraInputMap);
			scanInputMapForSceneMessages (m_structurePlacementCameraInputMap);
		}
		break;

	default:
		{
			NOT_NULL (m_inputMap);
			scanInputMapForSceneMessages (m_inputMap);
		}
		break;
	}
}

//-------------------------------------------------------------------

void GroundScene::handleInputMapUpdate (void)
{
	//--
	const GameCamera* const camera = NON_NULL (getCurrentCamera ());

	Rectangle2d rect;
	camera->getViewport (rect.x0, rect.y0, rect.x1, rect.y1);

	if (rect.x0 == rect.x1 || rect.y0 == rect.y1)
		return;

	const Vector2d mousePos (static_cast<float> (m_mouseCursor->getX ()), static_cast<float> (m_mouseCursor->getY ()));
	float yawMod   = (PI * mousePos.x * ms_mouseSensitivity)        / rect.getWidth ();
	float pitchMod = (PI * mousePos.y * ms_mouseSensitivity * 0.4f) / rect.getHeight ();  // pitching is less sensitive than yaw

	const float inertia = std::max (0.0f, CuiManager::getCameraInertia ());
	const float inertia_multiplier = RECIP (inertia + 1.0f);

	//-- products is used to determine if the current and last mod are of the same sign.
	//-- if they are, the product is positive, otherwise negative

	const Vector2d products (yawMod * m_lastYawPitchMod->x, pitchMod * m_lastYawPitchMod->y);

	if (products.x >= 0 && fabs (yawMod) > fabs (m_lastYawPitchMod->x))
		yawMod   = (yawMod   + (m_lastYawPitchMod->x * inertia)) * inertia_multiplier;

	if (products.y >= 0 && fabs (pitchMod) > fabs (m_lastYawPitchMod->y))
		pitchMod = (pitchMod + (m_lastYawPitchMod->y * inertia)) * inertia_multiplier;

	if (CuiManager::getPointerMotionCapturedByUiX ())
		yawMod = 0.0f;
	if (CuiManager::getPointerMotionCapturedByUiY ())
		pitchMod = 0.0f;

	m_lastYawPitchMod->set (yawMod, pitchMod);

	if (yawMod == 0.0f && pitchMod == 0.0f)
		return;

	if (ms_invertMouse)
		pitchMod = -pitchMod;

	//-- pass analog mouse info to normal inputmap or camera inputmap
	switch (m_currentView)
	{

	case CI_free:
		{
			if(m_usingGodClientInteriorCamera)
			{
				//-- just send it to the normal player inputmap
				NOT_NULL (m_inputMap);

				if (yawMod != CONST_REAL (0))
					m_inputMap->getMessageQueue ()->appendMessage (CM_cameraYawMouse,   yawMod);

				if (pitchMod != CONST_REAL (0))
					m_inputMap->getMessageQueue ()->appendMessage (CM_cameraPitchMouse, pitchMod);
			}
			else
			{
				//-- send this to the freeCamera inputmap
				NOT_NULL (m_freeCameraInputMap);

				if (yawMod != CONST_REAL (0))
					m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraYawMouse,   yawMod);

				if (pitchMod != CONST_REAL (0))
					m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraPitchMouse, pitchMod);
			}
		}
		break;

	case CI_debugPortal:
		{
			//-- send this to the debugPortalCamera inputmap
			NOT_NULL (m_debugPortalCameraInputMap);

			if (yawMod != CONST_REAL (0))
				m_debugPortalCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraYawMouse,   yawMod);

			if (pitchMod != CONST_REAL (0))
				m_debugPortalCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraPitchMouse, pitchMod);
		}
		break;

	case CI_structurePlacement:
		{
			//-- send this to the structurePlacementCamera inputmap
			NOT_NULL (m_structurePlacementCameraInputMap);

			if (yawMod != CONST_REAL (0))
				m_structurePlacementCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraYawMouse,   yawMod);

			if (pitchMod != CONST_REAL (0))
				m_structurePlacementCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraPitchMouse, pitchMod);
		}
		break;

	case CI_cockpit:
	case CI_shipTurret:
	case CI_freeChase:
		{
			//-- just send it to the normal player inputmap
			NOT_NULL (m_inputMap);

			if (yawMod != CONST_REAL (0))
				m_inputMap->getMessageQueue ()->appendMessage (CM_cameraYawMouse,   yawMod);

			if (pitchMod != CONST_REAL (0))
				m_inputMap->getMessageQueue ()->appendMessage (CM_cameraPitchMouse, pitchMod);
		}
		break;

	default:
		{
			//-- just send it to the normal player inputmap
			NOT_NULL (m_inputMap);

			if (yawMod != CONST_REAL (0))
				m_inputMap->getMessageQueue ()->appendMessage (CM_yaw, yawMod);
		}
		break;
	}
}

//-------------------------------------------------------------------

bool GroundScene::isFinishedLoading() const
{
	bool const cachedFileManagerDone = CachedFileManager::donePreloading();
	bool const spacePreloadedAssetManagerDone = SpacePreloadedAssetManager::donePreloading();
	bool const worldSnapshotDone = WorldSnapshot::donePreloading();
	bool const loaderIsIdle = AsynchronousLoader::isIdle();
	bool terrainGenerationStabilized = true;
	ClientProceduralTerrainAppearance * clientProceduralTerrainAppearance = dynamic_cast <ClientProceduralTerrainAppearance *> (TerrainObject::getInstance ()->getAppearance ());
	if (clientProceduralTerrainAppearance)
	{
		terrainGenerationStabilized = clientProceduralTerrainAppearance->terrainGenerationStabilized();
	}
	bool const hasPlayerObject = (Game::getPlayerObject() != NULL);

	return (cachedFileManagerDone
			&& spacePreloadedAssetManagerDone
			&& worldSnapshotDone
			&& loaderIsIdle
			&& terrainGenerationStabilized
			&& hasPlayerObject);
}

//----------------------------------------------------------------------

void GroundScene::_onFinishedLoading()
{
	m_loading=false;

	Audio::setNormalPreMixBuffer();
	Audio::unSilenceAllNonBackgroundMusic();
}

//----------------------------------------------------------------------

void GroundScene::updateCuiLoading()
{
	bool const cachedFileManagerDone = CachedFileManager::donePreloading();
	bool const worldSnapshotDone = WorldSnapshot::donePreloading();
	bool const loaderIsIdle = AsynchronousLoader::isIdle();
	bool terrainGenerationStabilized = true;
	ClientProceduralTerrainAppearance * clientProceduralTerrainAppearance = dynamic_cast <ClientProceduralTerrainAppearance *> (TerrainObject::getInstance ()->getAppearance ());
	if (clientProceduralTerrainAppearance)
	{
		terrainGenerationStabilized = clientProceduralTerrainAppearance->terrainGenerationStabilized();
	}
	bool const playerReady = m_receivedSceneReady && (getPlayer() != 0) && (getPlayer()->isInWorld());

	CuiLoadingManager::setFileCachingComplete(cachedFileManagerDone);
	CuiLoadingManager::setWorldLoaded(worldSnapshotDone);
	CuiLoadingManager::setServerObjectsReceived(loaderIsIdle);
	CuiLoadingManager::setTerrainGenerated(terrainGenerationStabilized);
	CuiLoadingManager::setPlayerReady(playerReady);

	if(!terrainGenerationStabilized)
	{
		CuiLoadingManager::setFullscreenLoadingString(CuiStringIds::generatingterrain.localize());
	}
	else if (!loaderIsIdle)
	{
		CuiLoadingManager::setFullscreenLoadingString(CuiStringIds::loadingobjects.localize());
	}
}

//----------------------------------------------------------------------

void GroundScene::update(float elapsedTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("GroundScene update");

	//-- scan input map for scene messages
	handleInputMapScan ();

	//-- tell the terrain which object is the reference object
	if (TerrainObject::getInstance ())
	{
		if (ConfigClientTerrain::getUseClientServerProceduralTerrainAppearanceTemplate ())
		{
			if (!TerrainObject::getInstance ()->isReferenceObject (getCurrentCamera ()))
				TerrainObject::getInstance ()->addReferenceObject (getCurrentCamera ());
		}
		else
		{
			TerrainObject::getInstance ()->removeAllReferenceObjects ();
			TerrainObject::getInstance ()->addReferenceObject (m_loading || getCurrentView () == CI_freeChase ? getPlayer () : getCurrentCamera ());
		}
	}

	//-- handle accumulated mouse move messages
	handleInputMapUpdate ();

	//-- update combat and playback scripts
	PlaybackScriptManager::update (elapsedTime);

	//-- update the world
	m_overheadMap->update (elapsedTime);
	SpaceTargetBracketOverlay::update(elapsedTime);
	GroundEnvironment::getInstance().setReferenceCamera(getCurrentCamera());
	ClientProceduralTerrainAppearance::setReferenceCamera (getCurrentCamera ());
	GroundEnvironment::getInstance().setReferenceObject(m_loading || getCurrentView () == CI_freeChase ? getPlayer () : getCurrentCamera());
	FloraManager::setReferenceObject (m_loading || getCurrentView () == CI_freeChase ? getPlayer () : getCurrentCamera ());
	GameMusicManager::setReferenceObject (m_loading || getCurrentView () == CI_freeChase ? getPlayer () : getCurrentCamera ());
	TerrainQuadTree::setUseDistance2d (m_loading || getCurrentView () == CI_freeChase);

	if (Game::getSinglePlayer() && m_currentView == CI_debugPortal)
	{
#ifdef _DEBUG
		static bool *debugFlag = DebugFlags::findFlag("ClientGraphics/Dpvs", "lockViewFrustum");
		if (debugFlag && *debugFlag)
			WorldSnapshot::update(CellProperty::getWorldCellProperty(), RenderWorld::getCameraPosition());
		else
#endif
			WorldSnapshot::update(m_debugPortalCamera->getParentCell(), m_debugPortalCamera->getPosition_w());
	}
	else
	{
		Vector const playerPosition = getPlayer()->getPosition_w();
		snprintf(ms_playerPosition, sizeof(ms_playerPosition), "Player: %5.2f %5.2f %5.2f\n", playerPosition.x, playerPosition.y, playerPosition.z);
		WorldSnapshot::update(getPlayer()->getParentCell(), playerPosition);
	}

	//-- Handle destruction of any queued objects
	if (m_destroyObjectTimer.updateZero(elapsedTime))
	{
		m_destroyObjectTimer.setExpireTime(Random::randomReal(0.5f, 1.f));
		
		if (!m_destroyObjectSet->empty())
		{
			NetworkId const & networkId = *m_destroyObjectSet->begin();
			m_destroyObjectSet->erase(m_destroyObjectSet->begin());

			Object * const object = NetworkIdManager::getObjectById(networkId);
			if (object)
			{
#if PRODUCTION == 0
				++ms_destroyObjectCountPerFrame;
				if (PixCounter::connectedToPixProfiler())
					ms_destroyObjectsPerFrame.append("%s\n", object->getDebugInformation().c_str());
#endif
				DEBUG_REPORT_LOG_PRINT(ms_logCreateMessages, ("SceneDestroyObject: deleting queued object %s\n", object->getDebugInformation().c_str()));
				object->kill();
			}
			else
				DEBUG_WARNING(true, ("SceneDestroyObject: queued object for delete does not exist %s\n", networkId.getValueString().c_str()));
		}
	}

	if (ms_useBuildoutClip || FileManifest::shouldUpdateManifest())
	{
		Vector const & playerPos_w = getPlayer()->getPosition_w();
		BuildoutArea const * buildoutArea = NULL;		
		buildoutArea = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(playerPos_w.x, playerPos_w.z, false);

		if (FileManifest::shouldUpdateManifest())
		{
			// update the sceneId in the FileManifest - it saddens me that both ground and space scenes are loaded through ground scene...
			std::string manifestScene = Game::getSceneId();
			if (buildoutArea)
				FileManifest::setSceneId((manifestScene + ":" + buildoutArea->areaName).c_str());
			else
				FileManifest::setSceneId(manifestScene.c_str());
		}

		if (ms_useBuildoutClip)
		{
			if (buildoutArea && buildoutArea->useClipRect)
			{
				ShaderPrimitiveSorter::setUseClipRectangle(true);
				Rectangle2d const & rect = buildoutArea->clipRect;
				ShaderPrimitiveSorter::setClipRectangle(rect.x0, rect.y0, rect.x1, rect.y1);
				ShaderPrimitiveSorter::setClipEnvironmentFlags(buildoutArea->clipEnvironmentFlags);
			}
			else
			{
				ShaderPrimitiveSorter::setUseClipRectangle(false);
				ShaderPrimitiveSorter::setClipEnvironmentFlags(0xffff);
			}
		}
	}
	else
		ShaderPrimitiveSorter::setUseClipRectangle(false);

	ClientInteriorLayoutManager::update();
	ClientWorld::update (elapsedTime);

	GameMusicManager::update (elapsedTime);

	PlayerMusicManager::alter (elapsedTime);
	CommunityManager::alter (elapsedTime);
	MatchMakingManager::alter (elapsedTime);
	AlarmManager::alter (elapsedTime);
	AwayFromKeyBoardManager::alter (elapsedTime);
	CustomerServiceManager::alter (elapsedTime);
	LightsaberCollisionManager::alter (elapsedTime);

#ifdef _DEBUG
	AiDebugStringManager::alter();
#endif // _DEBUG

	ClientAsteroidManager::update(elapsedTime);
	NebulaManagerClient::update(elapsedTime);
	ShipWeaponGroupManager::update(elapsedTime);
	AutoCommManager::update(elapsedTime);

	QuestJournalManager::update(elapsedTime);

	GameLight::setNight (!TerrainObject::getInstance ()->isDay ());

	//-- handle the mode callback
	if (m_modeCallback)
	{
		if (m_currentMode != m_overheadMap->getRenderMap ())
		{
			m_modeCallback (m_context);

			m_modeCallback = 0;
			m_context = 0;
		}
	}

	updateLoading();
}

//----------------------------------------------------------------------

void GroundScene::updateLoading()
{
		//-- update the loading screen
	if (!m_loading)
		return;
	
	bool const finishedLoading = isFinishedLoading();
	
	updateCuiLoading();
	
	if ((!Game::isClient() || (!ms_loadingScreenRender && finishedLoading)) && !m_sentSceneChannel)
	{
		WorldSnapshot::update(getPlayer()->getCellProperty(), getPlayer()->getPosition_w());
		GameNetwork::setSceneChannel();
		m_sentSceneChannel = true;
		
		// remove all mods; the server send us the list of mods
		CuiModifierManager::removeAllModifiers();
		
		// request for updated list of commodities item type if the list has changed
		AuctionManagerClient::requestItemTypeList();
		AuctionManagerClient::requestResourceTypeList();

		// request for updated list of collection "server first" if the list has changed
		Game::requestCollectionServerFirstList();
	}
	else
	{
		WorldSnapshot::preloadSomeAssets();
		bool const isSpace = Game::isSpace();
		if (isSpace)
		{
			SpacePreloadedAssetManager::preloadSomeAssets();
		}
		CachedFileManager::preloadSomeAssets ();
		CuiLoadingManager::setFullscreenLoadingPercent (std::min(SpacePreloadedAssetManager::getLoadingPercent(), std::min (WorldSnapshot::getLoadingPercent (), CachedFileManager::getLoadingPercent ())));
	}
	
	if (m_sentSceneChannel && finishedLoading)
	{
		CuiLoadingManager::setFullscreenLoadingEnabled (false);
		_onFinishedLoading();
		
		PlayerObject * p = Game::getPlayerObject();
		if(p)
		{
			p->applyDeferredWaypointCreation();
			ClientMissionObject::applyDeferredMissionObjectProcessing();
		}
		else
		{
			DEBUG_WARNING(true, ("Can't get PlayerObject in GroundScene::processEvent"));
		}
		
		if (!Game::getSinglePlayer ())
		{
			const NewbieTutorialResponse response ("clientReady");
			GameNetwork::send (response, true);
		}
		
		if (HyperspaceIoWin::getPlayHyperspaceBeforeSceneChange())
		{
			HyperspaceIoWin::setPlayHyperspaceBeforeSceneChange(false);
			if ((Game::isSpace()) && (m_cockpitCamera != 0))
			{
				ShipObject * const shipObject = Game::getPlayerContainingShip();
				
				if (shipObject != 0)
				{
					IGNORE_RETURN(new HyperspaceIoWin(m_cockpitCamera, shipObject, HyperspaceIoWin::S_leave));
				}
			}
		}
	}
	
}

//-------------------------------------------------------------------

IoResult GroundScene::processEvent(IoEvent *event)
{
	handleInputMapEvent (event);

	IoResult r = NetworkScene::processEvent(event);
	if(r != IOR_Pass)
	{
		return r;
	}

	//-- are we finished?
	if (getIsOver ())
	{
		return IOR_BlockKillMe;
	}

	switch (event->type)
	{
	case IOET_WindowKill:
		return IOR_PassKillMe;

	case IOET_Update:
		{
			const float elapsedTime = event->arg3;
			update(elapsedTime);
		}
		break;

	default:
		break;
	} //lint !e788 //case not used

	return IOR_Block;
}

//-------------------------------------------------------------------

void GroundSceneNamespace::renderDetailLevelObjectList (const int objectListIndex)
{
	int i;
	for (i = 0; i < ClientWorld::getNumberOfObjects (objectListIndex); ++i)
	{
		const Object* const object = ClientWorld::getConstObject (objectListIndex, i);
		if (object)
			ms_renderDetailLevelFunction (object);
	}
}

//-------------------------------------------------------------------

void GroundSceneNamespace::renderDetailLevel ()
{
	int i;
	for (i = WOL_MarkerRenderedStart; i < WOL_MarkerRenderedEnd; ++i)
		renderDetailLevelObjectList (i);
}

//-------------------------------------------------------------------

void GroundScene::draw (void) const
{
	if (ms_noDraw || m_noDraw)
		return;

	NP_PROFILER_AUTO_BLOCK_DEFINE("GroundScene::draw");

	TerrainObject const * const terrainObject = TerrainObject::getInstance();
	NOT_NULL(terrainObject);

	// hack to render everything around the player before the loading screen can go away
	if (ms_loadingScreenRender)
	{
		if (ms_loadingScreenRenderYaw >= PI_TIMES_2)
		{
			int newCameraIndex = CI_freeChase;
			CreatureObject const * const player = safe_cast<CreatureObject const *>(getPlayer());
			int shipStation = player ? player->getShipStation() : ShipStation::ShipStation_None;
			if (shipStation == ShipStation::ShipStation_Pilot || shipStation == ShipStation::ShipStation_Operations)
				newCameraIndex = CI_cockpit;
			else if (shipStation >= ShipStation::ShipStation_Gunner_First && shipStation <= ShipStation::ShipStation_Gunner_Last)
				newCameraIndex = CI_shipTurret;
			const_cast<GroundScene*>(this)->setView(newCameraIndex);
			ms_loadingScreenRender = false;

			// auto quit after loading screen is finished if config option is set
			if(ConfigClientGame::getAutoQuitAfterLoadScreen())
				Game::quit();

#if PRODUCTION == 0
			Graphics::pixSetMarker(L"CloseLoadingScreen");
			PixCounter::enable();
#endif
		}
		else
		{
			const_cast<GroundScene*>(this)->setView (CI_debugPortal);
			m_debugPortalCamera->setParentCell (getPlayer ()->getParentCell());
			m_debugPortalCamera->setPosition_p (getPlayer ()->getPosition_p());
			m_debugPortalCamera->move_o(Vector(0.0f, 2.0f, 0.0f));
			m_debugPortalCamera->resetRotate_o2p();
			m_debugPortalCamera->yaw_o(ms_loadingScreenRenderYaw);

			bool const finishedLoading = isFinishedLoading();

			if (finishedLoading)
			{
				ms_loadingScreenRenderYaw += PI / 8.0f;
			}
		}
	}

	//-- test collision
#ifdef _DEBUG
	if (ms_testCollision)
	{
		const Camera* const camera = getCurrentCamera ();
		if (camera)
		{
			CollisionInfo result;
			if (ClientWorld::collide (camera->getParentCell (), camera->getPosition_w (), camera->getPosition_w () + camera->getObjectFrameK_w () * camera->getFarPlane (), CollideParameters::cms_default, result))
			{
				{
					const Transform& transform_o = result.getObject ()->getTransform_o2w ();
					const Vector start_o = transform_o.rotateTranslate_p2l (result.getPoint ());
					const Vector end_o   = start_o + transform_o.rotate_p2l (2.f * result.getNormal ());

					camera->addDebugPrimitive (new Line3dDebugPrimitive (UtilityDebugPrimitive::S_z, transform_o, start_o, end_o, PackedArgb::solidGreen));
					camera->addDebugPrimitive (new SphereDebugPrimitive (UtilityDebugPrimitive::S_z, transform_o, start_o, 1.f, 4, 4));
				}

				{
					const Transform& transform_w = Transform::identity;
					const Vector start_w = result.getPoint () + Vector::xyz111 * 0.02f;
					const Vector end_w   = start_w + (2.f * result.getNormal ());

					camera->addDebugPrimitive (new Line3dDebugPrimitive (UtilityDebugPrimitive::S_z, transform_w, start_w, end_w, PackedArgb::solidGreen));
					camera->addDebugPrimitive (new SphereDebugPrimitive (UtilityDebugPrimitive::S_z, transform_w, start_w, 1.f, 4, 4));
				}
			}
		}
	}
#endif

	//-- hack under water fog parameters
	SaveCameraParameters cameraParameters [CI_COUNT];

	bool const isUnderWater = terrainObject->isBelowWater(getCurrentCamera()->getPosition_w());

	//--
	PackedRgb backgroundColor = terrainObject->getClearColor();

	if (isUnderWater)
	{
		int i;
		for (i = 0; i < getNumberOfViews (); i++)
		{
			cameraParameters [i].save (getCamera (i));
			cameraParameters [i].setUnderWater (const_cast<GameCamera*> (getCamera (i)), backgroundColor);
		}
	}

	Graphics::setViewport(0, 0, Graphics::getCurrentRenderTargetWidth(), Graphics::getCurrentRenderTargetHeight());

	if (ms_letterBoxedViewport)
	{
		Graphics::clearViewport(true, PackedRgb::solidBlack.asUint32(), true, 1.0f, true, 0);

		int const viewportTop = Graphics::getCurrentRenderTargetHeight() / 6;
		int const viewportBottom = Graphics::getCurrentRenderTargetHeight() * 2 / 3;
		Graphics::setViewport(0, viewportTop, Graphics::getCurrentRenderTargetWidth(), viewportBottom);

		m_cameras[m_currentView]->setViewport(0, viewportTop, Graphics::getCurrentRenderTargetWidth(), viewportBottom);
	}
	else
		m_cameras[m_currentView]->setViewport(0, 0, Graphics::getCurrentRenderTargetWidth(), Graphics::getCurrentRenderTargetHeight());

	Graphics::clearViewport(true, backgroundColor.asUint32(), true, 1.0f, true, 0);

#ifdef _DEBUG
	if (m_debugPointList->size ())
	{
		const Camera* const camera = getCurrentCamera ();

		uint i;
		for (i = 0; i < m_debugPointList->size (); ++i)
		{
			const Vector point = (*m_debugPointList) [i];
			const Vector startPoint (point.x, 1000.f, point.z);
			const Vector endPoint (point.x, -1000.f, point.z);
			camera->addDebugPrimitive (new Line3dDebugPrimitive (UtilityDebugPrimitive::S_z, Transform::identity, startPoint, endPoint, PackedArgb::solidGreen));
		}
	}

	if (m_debugLineList->size ())
	{
		Camera const * const camera = getCurrentCamera ();

		for (uint i = 0; i < m_debugLineList->size (); ++i)
		{
			Line const & line = (*m_debugLineList) [i];
			camera->addDebugPrimitive (new Line3dDebugPrimitive (UtilityDebugPrimitive::S_z, Transform::identity, line.m_start_w, line.m_end_w, line.m_color));
		}
	}
#endif

	ShadowManager::setTimeOfDay(GroundEnvironment::getInstance().getTime());

	//-- allow shadow submissions (the ui will try to submit shadow volumes as well)
	ShadowVolume::setAllowShadowSubmissions (true);

	//
	//-- render world
	//
	ClientWorld::draw ();

	//-- render nebulas
	NebulaManagerClient::render();

	//-- disallow shadow submissions
	ShadowVolume::setAllowShadowSubmissions (false);

	//-- render shadow alpha
	ShadowVolume::renderShadowAlpha (getCurrentCamera ());

	//--
	Graphics::setViewport(0, 0, Graphics::getCurrentRenderTargetWidth(), Graphics::getCurrentRenderTargetHeight());

	if (isUnderWater)
	{
		int i;
		for (i = 0; i < getNumberOfViews (); i++)
			cameraParameters [i].restore (const_cast<GameCamera*> (getCamera (i)));
	}

	const Camera* const camera = NON_NULL (m_cameras [m_currentView]);
	drawNetworkIds (*camera);

	//-- tell character system to plan the detail levels for next frame based
	//   on characters rendered this frame and the current camera's position.
	if (CharacterLodManager::isEnabled ())
		CharacterLodManager::planNextFrame (camera->getPosition_w ());
}

//-------------------------------------------------------------------

void GroundScene::drawOverlays (void) const
{
	getCurrentCamera ()->renderFlash ();

#if PRODUCTION == 0
	if (ms_renderDetailLevel && ms_renderDetailLevelFunction)
		renderDetailLevel ();
#endif

	//-- render map
	if (m_overheadMap)
		m_overheadMap->render ();

	if (m_spaceTargetBracketOverlay)
		m_spaceTargetBracketOverlay->render();

#if PRODUCTION == 0
	PlotterManager::draw();
#endif // PRODUCTION == 0
}

//-------------------------------------------------------------------

void GroundScene::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{
	const GameNetworkMessage * gnm = dynamic_cast<const GameNetworkMessage *>(&message);

	//----------------------------------------------------------------------

#if PRODUCTION == 0
	//-- Log count of messages per frame by type.
	if (ms_logReceivedMessages)
		++ms_receivedMessageMap[NON_NULL(gnm)->getType()];
#endif

	if(message.isType("SceneCreateObjectByName") || message.isType("SceneCreateObjectByCrc"))
	{
#if PRODUCTION == 0
		++ms_createObjectCountPerFrame;
#endif

		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();

		Transform     transform(Transform::IF_none);
		NetworkId     networkId;
		char          objectTemplateName[Os::MAX_PATH_LENGTH];
		bool          hyperspace = false;

		if (message.isType("SceneCreateObjectByName"))
		{
			const SceneCreateObjectByName o(ri);

			transform = o.getTransform ();
			networkId = o.getNetworkId ();
			strcpy(objectTemplateName, o.getTemplateName ().c_str ());
			hyperspace = o.getHyperspace();
		}
		else
		{
			const SceneCreateObjectByCrc o(ri);

			ConstCharCrcString cc = ObjectTemplateList::lookUp(o.getTemplateCrc());
			if (cc.isEmpty())
			{
				WARNING(true, ("SceneCreateObjectByCrc crc %08x was not found in object template list", cc.getCrc()));
				return;
			}

			transform = o.getTransform ();
			networkId = o.getNetworkId ();
			hyperspace = o.getHyperspace();
			strcpy(objectTemplateName, cc.getString());
		}

#if PRODUCTION == 0
		if (PixCounter::connectedToPixProfiler())
			ms_createObjectsPerFrame.append("%s\t%s\n", networkId.getValueString ().c_str(), objectTemplateName);
#endif

		//-- validate any parameters
#ifdef _DEBUG
		IGNORE_RETURN(transform.validate());
#endif

		//-- validate the player's position
		TerrainObject const *const terrain = TerrainObject::getInstance();
		if (terrain)
		{
			// It's okay if this object is contained and the position is not in world space.
			// This is still a good upper-bound sanity check on the value.
			Vector const newPosition = transform.getPosition_p ();
			if (!terrain->isWithinTerrainBoundaries (newPosition))
			{
				WARNING (true, ("SceneCreateObject* handler: object id=[%s] [%s] received invalid position x=[%0.1f], y=[%0.1f], z=[%0.1f], setting to 1,1,1.", networkId.getValueString ().c_str (), objectTemplateName, newPosition.x, newPosition.y, newPosition.z));
				transform.setPosition_p (Vector::xyz111);
			}
		}

		DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneCreateObject: networkId=%s, objectTemplateName=%s, position=<%1.2f, %1.2f, %1.2f>\n", networkId.getValueString ().c_str (), objectTemplateName, transform.getPosition_p ().x, transform.getPosition_p ().y, transform.getPosition_p ().z));

		//-- see if the object has already been created
		Object* existingObject = NetworkIdManager::getObjectById(networkId);

		//-- see if it is in the cache
		if (existingObject)
		{
			ClientObject* const clientObject = safe_cast<ClientObject*> (existingObject);

			if (clientObject->isClientCached ())
			{
				DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneCreateObject: networkId=%s, recreating existing client-cached object\n", networkId.getValueString ().c_str ()));

				delete clientObject;
				existingObject = 0;

				//-- mark the object so it never gets created again
				WorldSnapshot::removeObject (static_cast<int> (networkId.getValue ()));
			}
			else
			{
				if (existingObject == getPlayer ())
				{
					DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneCreateObject: networkId=%s, received create message for player\n", networkId.getValueString ().c_str ()));
				}
				else if (existingObject->getKill())
				{
					// if we're replacing an object that has already been marked for deletion, go ahead and delete it.
					delete existingObject;
					existingObject = 0;
				}
				else
				{
					DestroyObjectSet::iterator iter = m_destroyObjectSet->find(networkId);
					if (iter != m_destroyObjectSet->end())
					{
						DEBUG_REPORT_LOG_PRINT(ms_logCreateMessages, ("SceneCreateObject: object queued for destroy %s\n", existingObject->getDebugInformation().c_str()));

						//-- If we have received a create for an object that has been queued for deletion, just delete the object
						m_destroyObjectSet->erase(iter);

						delete existingObject;
						existingObject = 0;
					}
					else
						DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneCreateObject: networkId=%s, received create message for existing object\n", networkId.getValueString ().c_str ()));
				}

				if (existingObject)
				{
					CellProperty::setPortalTransitionsEnabled (false);
						existingObject->setTransform_o2p (transform);
					CellProperty::setPortalTransitionsEnabled (true);

					CollisionWorld::objectWarped (existingObject);

					if (!clientObject->isInitialized())
						clientObject->beginBaselines();
				}
			}
		}

		if (!existingObject)
		{
			ClientObject* const clientObject = safe_cast<ClientObject*> (ObjectTemplate::createObject (objectTemplateName));
			if (clientObject)
			{
				RenderWorld::addObjectNotifications (*clientObject);
				CellProperty::addPortalCrossingNotification (*clientObject);
				clientObject->setTransform_o2p (transform);
				clientObject->createDefaultController ();
				clientObject->setNetworkId (networkId);
				NetworkController *const controller = safe_cast<NetworkController*> (clientObject->getController ());
				if (controller)
					controller->setAuthoritative (false);
				if (!clientObject->isInitialized())
					clientObject->beginBaselines();

				ShipObject * const shipObject = clientObject->asShipObject();
				if ((shipObject != 0) && (shipObject != Game::getPlayerContainingShip()))
				{
					// player controlled ships will always hyperspace in
					ObjectTemplate const * const objectTemplate = shipObject->getObjectTemplate();
					SharedShipObjectTemplate const * const sharedShipObjectTemplate = dynamic_cast<SharedShipObjectTemplate const * const>(objectTemplate);
					if ((sharedShipObjectTemplate != 0) && (sharedShipObjectTemplate->getPlayerControlled()))
					{
						hyperspace = true;
					}

					if(hyperspace)
					{
						DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneCreateObject: networkId=%s, running hyperspace-specific logic\n", networkId.getValueString ().c_str ()));
						shipObject->onEnterByHyperspace();
					}
				}
			}
			else
				DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneCreateObject: networkId=%s, object could not be created from template %s\n", networkId.getValueString ().c_str (), objectTemplateName));
		}
	}

	//----------------------------------------------------------------------

	else if(message.isType("SceneDestroyObject"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const SceneDestroyObject o(ri);

		//-- extract parameters
		const NetworkId& networkId = o.getNetworkId ();

		DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneDestroyObject: networkId=%s\n", networkId.getValueString ().c_str ()));

		Object * const existingObject = NetworkIdManager::getObjectById(networkId);
		if (existingObject != 0)
		{
			ClientObject * const clientObject = safe_cast<ClientObject *>(existingObject);

			if(o.getHyperspace())
			{
				DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneDestroyObject: networkId=%s, hyperspacing a ship away, then destroy\n", networkId.getValueString ().c_str ()));

				ShipObject * const shipObject = clientObject->asShipObject();
				if (shipObject != 0)
				{
					shipObject->onLeaveByHyperspace();
				}
			}
			else
			{
				if (clientObject->isClientCached ())
				{
					DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneDestroyObject: networkId=%s, ignoring destroy for client-cached object\n", networkId.getValueString ().c_str ()));

					// Removed for now - the server may send destroy messages for client cached objects if the originiating server
					// cannot resolve the object to know it is client-cached.

					//-- mark the object so it never gets created again
					//WorldSnapshot::removeObject (static_cast<int> (o.getNetworkId ().getValue ()));
				}
				else
				{
					if (dynamic_cast<CellObject *> (clientObject))
					{
						DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneDestroyObject: networkId=%s, ignoring destroy for cell object\n", networkId.getValueString ().c_str ()));
					}
					else
					{
						if (existingObject == getPlayer ())
						{
							DEBUG_WARNING(true, ("SceneDestroyObject: networkId=%s, cannot destroy player", networkId.getValueString ().c_str ()));
							existingObject->removeFromWorld();
						}
						else
						{
							// Don't allow us to destroy the player's mount.  This will cause the player to get destroyed.
							// Alternatives to this: have the server dismount us prior to scene warp, then remount us post scene warp.
							CreatureObject *const creatureObject = clientObject->asCreatureObject();
							bool const isPlayerMount = creatureObject && creatureObject->isMountForThisClientPlayer();

							if (isPlayerMount)
							{
								DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneDestroyObject: networkId=%s, ignoring destroy for this client player's mount\n", networkId.getValueString ().c_str ()));
//								creatureObject->removeFromWorld();
							}
							else
							{
								bool const isPlayerPilotedShip = (Game::getPlayerPilotedShip() == clientObject);

								if (isPlayerPilotedShip)
								{
									DEBUG_WARNING (true, ("SceneDestroyObject: networkId=%s, ignoring destroy for this client player's ship", networkId.getValueString ().c_str ()));
								}
								else
								{
									TangibleObject * const tangible = clientObject->asTangibleObject();
									ShipObject * const ship = clientObject->asShipObject();

									//-- don't immediately destroy objects which are already in the process of being destroyed
									if (NULL != tangible && tangible->hasCondition(TangibleObject::C_destroying))
									{
										if (NULL != ship)
											ship->setShipKillableByDestructionSequence(true);
									}
#if 1
									else
									{
										//-- Tell the object to kill itself next alter.
										existingObject->kill();
									}
									
									//-- Ensure the object gets an alter.
									existingObject->scheduleForAlter();
#else
									else
									{
										//-- Remove this object from the world and queue it for delete
										DEBUG_REPORT_LOG_PRINT(ms_logCreateMessages, ("SceneDestroyObject: queueing object for delete %s\n", existingObject->getDebugInformation().c_str()));
										if (existingObject->isInWorld())
											existingObject->removeFromWorld();
										m_destroyObjectSet->insert(networkId);
									}
#endif
								}
							}
						}
					}
				}
			}
		}
		else
			DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("SceneDestroyObject: networkId=%s, object does not exist\n", networkId.getValueString ().c_str ()));
	}

	//----------------------------------------------------------------------

	else if(message.isType("UpdateContainmentMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const UpdateContainmentMessage o(ri);
		DEBUG_REPORT_LOG_PRINT (ms_logCreateMessages, ("UpdateContainment: networkId=%s, newContainer=%s, newArrangement=%d\n", o.getNetworkId().getValueString().c_str(), o.getContainerId().getValueString().c_str(), o.getSlotArrangement()));

		ClientObject * target = dynamic_cast<ClientObject *>(NetworkIdManager::getObjectById(o.getNetworkId()));
		if (target)
		{
			//-- Handle mounts.

			// If the target is a rider, make sure we handle the dismount visuals before
			// a transfer to a cell could ever happen.  This prevents an "already attached"
			// FATAL from triggering since the rider is actually both a child object of the
			// saddle (client only) and is contained by the mount.
			CreatureObject *const targetAsCreature = target->asCreatureObject();
			if (targetAsCreature)
			{
				if (targetAsCreature->isRidingMount())
				{
					// Find network id for the mount.
					CreatureObject const *const mount = targetAsCreature->getMountedCreature();
					NetworkId mountId = (mount != NULL) ? mount->getNetworkId() : NetworkId::cms_invalid;

					// Tell the rider to do visuals for dismounting.
					if (o.getContainerId() != mountId)
						targetAsCreature->riderDoDismountVisuals(mountId);
				}
				else if (targetAsCreature->getShipStation() != ShipStation::ShipStation_None)
				{
					// If we are piloting / gunning / a droid for a ship and moving to a
					// container other than the one we're in, detach before
					// the updateContainment call since it would cause a cell change.
					Object const * const oldContainingObject = ContainerInterface::getContainedByObject(*target);
					if (o.getContainerId() != oldContainingObject->getNetworkId() && target->getAttachedTo())
						target->detachFromObject(Object::DF_none);
				}
			}
			else
			{
				Object const * const oldContainingObject = ContainerInterface::getContainedByObject(*target);
				if (oldContainingObject && (o.getContainerId() != oldContainingObject->getNetworkId() && target->getAttachedTo()))
					target->detachFromObject(Object::DF_none);
			}

			//-- Do the containment update.
			target->updateContainment(o.getContainerId(), o.getSlotArrangement());

			//-- Sanity check the target's position when attached to something (cell/vehicle/mount).
			if (target->getAttachedTo())
			{
				Vector const newPosition = target->getPosition_p();
				float const maxValidCellCoordinate = CellProperty::getMaximumValidCellSpaceCoordinate();

				if ((abs(newPosition.x) > maxValidCellCoordinate) || (abs(newPosition.y) > maxValidCellCoordinate) || (abs(newPosition.z) > maxValidCellCoordinate))
				{
					WARNING (true, ("UpdateContainmentMessage: object id=[%s] received containment update with bad o2p position info x=[%0.2f], y=[%0.2f], z=[%0.2f], setting to origin of parent.", target->getNetworkId ().getValueString ().c_str (), newPosition.x, newPosition.y, newPosition.z));

					CellProperty::setPortalTransitionsEnabled (false);
						target->setPosition_p (Vector::zero);
					CellProperty::setPortalTransitionsEnabled (true);

					CollisionWorld::objectWarped(target);
				}
			}

			//-- Handle initializing player for this client when contained.
			// Fix up the player's position.  A player in a cell will have had its local-to-parent
			// position set to world position erroneously.
			const bool isPlayer = (target==Game::getPlayer());
			if (  isPlayer 
				&& !target->isInitialized()
				)
			{
				CellProperty *const playerCellProperty = target->getParentCell();
				if (playerCellProperty != CellProperty::getWorldCellProperty())
				{
					// update position.
					CellProperty::setPortalTransitionsEnabled (false);
						target->setPosition_w(target->getPosition_w());
					CellProperty::setPortalTransitionsEnabled (true);

					CollisionWorld::objectWarped(target);
				}
			}
		}
	}

	//----------------------------------------------------------------------

	else if(message.isType("CmdSceneReady"))
	{
		CreatureObject *player = dynamic_cast<CreatureObject *>(getPlayer());
		NOT_NULL(player);

		//-- force an update to the camera targets in case the player appearance/size/scale changed
		m_cockpitCamera->setTarget(player);
		m_shipTurretCamera->setTarget(player);
		m_freeChaseCamera->setTarget          (player);
		m_freeCamera->setTarget               (player);
		m_debugPortalCamera->setTarget        (player);
		m_structurePlacementCamera->setTarget (player);

		m_receivedSceneReady = true;
	}

	//----------------------------------------------------------------------

	else if(message.isType("UpdateCellPermissionMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const UpdateCellPermissionMessage updateCellPermissionMessage(ri);
		CellObject * const target = dynamic_cast<CellObject *>(NetworkIdManager::getObjectById(updateCellPermissionMessage.getTarget()));
		if (target)
			target->setAccessAllowed(updateCellPermissionMessage.getAllowed());
	}

	//----------------------------------------------------------------------

	else if(message.isType("SceneEndBaselines"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const SceneEndBaselines o(ri);
		CachedNetworkId newObjectId(o.getNetworkId());
		ClientObject * const target = dynamic_cast<ClientObject *>(newObjectId.getObject());
		if (target)
			handleEndBaselines(target);
	}

	//----------------------------------------------------------------------

	else if(message.isType("ServerTimeMessage"))
	{
		//-- see if the terrain exists
		TerrainObject* const terrainObject = TerrainObject::getInstance ();
		if (terrainObject)
		{
			Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
			const ServerTimeMessage stm(ri);

			setServerTimeOffset(static_cast<unsigned int>(stm.getTimeSeconds())-static_cast<unsigned int>(time(0)));

			//-- time from server is in seconds, terrain time is from 0 to 1
			const float normalizedTime = fmodf (static_cast<float> (stm.getTimeSeconds ()), terrainObject->getEnvironmentCycleTime ()) / terrainObject->getEnvironmentCycleTime ();
			terrainObject->setTime (normalizedTime);
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType (ServerWeatherMessage::cms_name))
	{
		//-- see if the terrain exists
		TerrainObject* const terrainObject = TerrainObject::getInstance ();
		if (terrainObject)
		{
			Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
			const ServerWeatherMessage serverWeatherMessage (ri);

			//-- push this information to the various systems
			WeatherManager::setNormalizedWindVelocity_w(serverWeatherMessage.getWindVelocity_w());
			GroundEnvironment::getInstance().setWeatherIndex(serverWeatherMessage.getIndex());
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType (NewbieTutorialRequest::cms_name))
	{
		//-- what type of request is it?
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
		const NewbieTutorialRequest newbieTutorialRequest (ri);

		if (m_freeChaseCamera && newbieTutorialRequest.getRequest () == cms_newbieTutorialRequestZoomCamera)
			m_freeChaseCamera->setModeCallback (GroundSceneNamespace::zoomCameraCallback, this);

		if (newbieTutorialRequest.getRequest () == cms_newbieTutorialRequestOverlayMap)
			setModeCallback (GroundSceneNamespace::overlayMapCallback, this);

		if (getPlayer () && newbieTutorialRequest.getRequest () == cms_newbieTutorialRequestChangeLookAtTarget)
		{
			PlayerCreatureController* const playerCreatureController = dynamic_cast<PlayerCreatureController*> (getPlayer ()->getController ());
			if (playerCreatureController)
				playerCreatureController->setModeCallback (GroundSceneNamespace::changeLookAtTargetCallback, this);
			else
				DEBUG_WARNING (true, ("player does not have a PlayerCreatureController"));
		}
	}

	//----------------------------------------------------------------------

	else if(message.isType("BaselinesMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const BaselinesMessage bm(ri);
		ClientObject * const target = dynamic_cast<ClientObject *>(NetworkIdManager::getObjectById(bm.getTarget()));
		if (target)
			target->applyBaselines(bm);
	}

	//----------------------------------------------------------------------

	else if(message.isType("DeltasMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const DeltasMessage dm(ri);
		ClientObject * const target = dynamic_cast<ClientObject *>(NetworkIdManager::getObjectById(dm.getTarget()));
		if (target)
		{
			target->applyDeltas(dm);

			//-- Make sure this client object gets an alter next time we do alter processing.
			target->scheduleForAlter();
		}
	}

	else if (message.isType("UpdateTransformMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const UpdateTransformMessage utm(ri);

#if PRODUCTION == 0
		if (ms_logUpdateTransformMessages)
		{
			//-- Log message.
			++ms_updateTransformMessageMap[utm.getNetworkId()];
		}
#endif

		Object * const object = NetworkIdManager::getObjectById(utm.getNetworkId());
		if(object)
		{
			// get the controller
			NetworkController * const networkController = safe_cast<NetworkController *>(object->getController());
			if(networkController)
			{
				//	This is a valid controller, message is either allocated or 0.
				//	Append it to the controller's message queue so that it may be
				//	processed during alter.

				uint32 flags = GameControllerMessageFlags::SOURCE_REMOTE_SERVER;
				flags |= GameControllerMessageFlags::DEST_PROXY_CLIENT;

				ClientController * const clientController = dynamic_cast<ClientController*>(networkController);

				if (clientController && clientController->isSettingBaselines())
				{
					MessageQueueDataTransform msg(0, utm.getSequenceNumber(), utm.getTransform(), static_cast<float>(utm.getSpeed()), utm.getLookAtYaw(), utm.getUseLookAtYaw());
					clientController->handleMessage(CM_netUpdateTransform, 0.f, &msg, flags);
				}
				else
				{
					MessageQueueDataTransform * const msg = new MessageQueueDataTransform(0, utm.getSequenceNumber(), utm.getTransform(), static_cast<float>(utm.getSpeed()), utm.getLookAtYaw(), utm.getUseLookAtYaw());
					networkController->appendMessage(CM_netUpdateTransform, 0.0f, msg, flags);
				}
			}
		}
	}
	//----------------------------------------------------------------------
	else if (message.isType("UpdateTransformWithParentMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const UpdateTransformWithParentMessage utm(ri);

		Object * const object = NetworkIdManager::getObjectById(utm.getNetworkId());
		if(object)
		{
			// get the controller
			NetworkController * const networkController = safe_cast<NetworkController *>(object->getController());
			if(networkController)
			{
				//	This is a valid controller, message is either allocated or 0.
				//	Append it to the controller's message queue so that it may be
				//	processed during alter.

				uint32 flags = GameControllerMessageFlags::SOURCE_REMOTE_SERVER;
				flags |= GameControllerMessageFlags::DEST_PROXY_CLIENT;

				ClientController * const clientController = dynamic_cast<ClientController*>(networkController);

				if (clientController && clientController->isSettingBaselines())
				{
					MessageQueueDataTransformWithParent const msg(0, utm.getSequenceNumber(), utm.getCellId(), utm.getTransform(), static_cast<float>(utm.getSpeed()), utm.getLookAtYaw(), utm.getUseLookAtYaw());
					clientController->handleMessage(CM_netUpdateTransformWithParent, 0.0f, &msg, flags);
				}
				else
				{
					MessageQueueDataTransformWithParent * const msg = new MessageQueueDataTransformWithParent(0, utm.getSequenceNumber(), utm.getCellId(), utm.getTransform(), static_cast<float>(utm.getSpeed()), utm.getLookAtYaw(), utm.getUseLookAtYaw());
					networkController->appendMessage(CM_netUpdateTransformWithParent, 0.0f, msg, flags);
				}
			}
		}
	}
	//----------------------------------------------------------------------

	else if (message.isType("PlayMusicMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		PlayMusicMessage const msg(ri);

		if (msg.getPlayType() == 0)
			GameMusicManager::setMusicTrackOverride(msg.getMusicName().c_str());
		else
			GameMusicManager::setSoundTrackOverride(msg.getMusicName().c_str());
	}

	//----------------------------------------------------------------------

	else if (message.isType("ClientOpenContainerMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		ClientOpenContainerMessage msg(ri);
		ClientObject * const target = dynamic_cast<ClientObject *>(NetworkIdManager::getObjectById(msg.getContainerId()));
		if (target)
		{
			CuiInventoryManager::handleItemOpen(*target, msg.getSlot(), 0, 0, true, true);
		}
	}

	//----------------------------------------------------------------------
	//-- catch server crashes while loading

	else if (message.isType (UnnamedMessages::ConnectionServerConnectionClosed))
	{
		if (m_loading)
		{
			//-- there is no way for the client to know how long it will take to receive the player, so we can't progress
			CuiLoadingManager::setFullscreenLoadingPercent (0);
			CuiLoadingManager::setFullscreenLoadingEnabled (false);
			_onFinishedLoading();
			Audio::setNormalPreMixBuffer();
			quit ();
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType("LogoutMessage"))
	{
		// send to character select
		CuiLoginManager::disconnectFromCluster();
		Game::cleanupScene();
	}

	// ----------------------------------------------------------------------

	else if (message.isType("DebugTransformMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		DebugTransformMessage const debugTransformMessage(ri);

		PlayerCreatureController * const controller = safe_cast<PlayerCreatureController *>(getPlayer()->getController());

		controller->setServerTransform(debugTransformMessage.getTransform());

		controller->setServerCellObject(NetworkIdManager::getObjectById(debugTransformMessage.getCellId()));
	}

	// ----------------------------------------------------------------------

	else if (message.isType (CreateClientPathMessage::cms_name))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<GameNetworkMessage const *> (&message))->getByteStream ().begin ();
		CreateClientPathMessage const createClientPathMessage (ri);

		if (m_clientPathObject)
		{
			delete m_clientPathObject;
			m_clientPathObject = 0;
		}

		std::vector<Vector> const & pointList = createClientPathMessage.getPointList ();
		if (pointList.size ())
		{
			m_clientPathObject = new ClientPathObject (pointList);
			m_clientPathObject->addNotification (ClientWorld::getIntangibleNotification ());
			m_clientPathObject->addToWorld ();

			m_overheadMap->setClientPath (pointList);
		}
	}

	// ----------------------------------------------------------------------

	else if (message.isType (DestroyClientPathMessage::cms_name))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<GameNetworkMessage const *> (&message))->getByteStream ().begin ();
		DestroyClientPathMessage const destroyClientPathMessage (ri);

		if (m_clientPathObject)
		{
			delete m_clientPathObject;
			m_clientPathObject = 0;
		}

		m_overheadMap->clearClientPath ();
	}

	// ----------------------------------------------------------------------

	else if (message.isType(UpdatePvpStatusMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const UpdatePvpStatusMessage updatePvpStatusMessage(ri);
		TangibleObject * const target = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(updatePvpStatusMessage.getTarget()));
		if (target)
		{
			if (target == getPlayer())
			{
				/**
				* @todo: make this work right

				if ((target->getPvpFlags() ^ updatePvpStatusMessage.getFlags()) & PvpStatusFlags::HasEnemies)
				{
					if (updatePvpStatusMessage.getFlags() & PvpStatusFlags::HasEnemies)
						CuiSystemMessageManager::sendFakeSystemMessage (ClientStringIds::enemy_flags.localize());
					else
						CuiSystemMessageManager::sendFakeSystemMessage (ClientStringIds::no_enemy_flags.localize());
				}
				*/
			}
			target->setPvpFlags(updatePvpStatusMessage.getFlags());
		}
	}

	//----------------------------------------------------------------------

	else if(message.isType("CreateProjectileMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		CreateProjectileMessage const createProjectileMessage(ri);

		ShipObject * const shipObject = ShipObject::findShipByShipId(createProjectileMessage.getShipId());
		if (shipObject)
		{
			//-- Compute remaining time for the projectile
			float const deltaTime = GameNetwork::computeDeltaTimeInSeconds(createProjectileMessage.getSyncStampLong());

			int const weaponIndex = createProjectileMessage.getWeaponIndex();

			//-- Tell projectile system to create projectile(s)
			float const speed = shipObject->getWeaponProjectileSpeed(weaponIndex);
			float const timeToLive = shipObject->computeWeaponProjectileTimeToLive(weaponIndex);

			Transform serverTransform_p(createProjectileMessage.getTransform_p());
			serverTransform_p.move_l(Vector::unitZ * deltaTime * speed);

			float const expireTime = timeToLive - deltaTime;
			if (expireTime > 0.f)
				ProjectileManager::createServerProjectile(shipObject, weaponIndex, createProjectileMessage.getProjectileIndex(), serverTransform_p, speed, expireTime);

			//--
#ifdef _DEBUG
			if (ms_renderProjectilePath)
			{
				Line line;
				line.m_start_w = createProjectileMessage.getTransform_p().getPosition_p();
				line.m_end_w = line.m_start_w + (createProjectileMessage.getTransform_p().getLocalFrameK_p() * speed * timeToLive);
				line.m_color = PackedArgb::solidRed;
				m_debugLineList->push_back (line);
			}
#endif
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType(CreateNebulaLightningMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		CreateNebulaLightningMessage const createNebulaLightningMessage(ri);
		NebulaManagerClient::enqueueLightning(createNebulaLightningMessage.getNebulaLightningData());
	}

	//----------------------------------------------------------------------

	else if(message.isType("CreateMissileMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		CreateMissileMessage cmm(ri);

		// Add to the missile manager.
		MissileManager::addMissile(cmm.getMissileId(), cmm.getSource(), cmm.getTarget(), cmm.getSourceLocation(), cmm.getTargetLocation(), cmm.getImpactTime(), cmm.getMissileTypeId(), cmm.getWeaponId(), cmm.getTargetComponent());

		// Send this to the target ship.
		Object * const object = NetworkIdManager::getObjectById(cmm.getTarget());
		ClientObject * const clientObject = object ? safe_cast<ClientObject *>(object) : NULL;
		ShipObject * const shipObject = clientObject ? clientObject->asShipObject() : NULL;
		if (shipObject)
		{
			shipObject->onCreateMissileMessage(cmm.getTarget());
		}


		// Dispatch to the UI or other systems.
		Transceivers::createMissile.emitMessage(cmm);
	}

	//----------------------------------------------------------------------

	else if(message.isType("UpdateMissileMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		UpdateMissileMessage umm(ri);

		switch (umm.getUpdateType())
		{
			case UpdateMissileMessage::UT_hit:
				MissileManager::onServerMissileHitTarget(umm.getMissileId());
				break;

			case UpdateMissileMessage::UT_miss:
				MissileManager::onServerMissileMissedTarget(umm.getMissileId());
				break;

			case UpdateMissileMessage::UT_countermeasured:
				MissileManager::onServerMissileCountermeasured(umm.getMissileId(), umm.getCountermeasureType());
				break;

			case UpdateMissileMessage::UT_countermeasureFailed:
				MissileManager::onServerMissileCountermeasureFailed(umm.getShipId(), umm.getCountermeasureType());
				break;

			default:
				DEBUG_FATAL(true,("Programmer bug:  unrecognized code in UpdateMissileMessage"));
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType("ShipUpdateTransformMessage"))
	{
		Archive::ReadIterator readIterator = NON_NULL(gnm)->getByteStream().begin();
		ShipUpdateTransformMessage const shipUpdateTransformMessage(readIterator);

		ShipObject * const ship = ShipObject::findShipByShipId(shipUpdateTransformMessage.getShipId());
		if (ship)
		{
			ShipController * const shipController = safe_cast<ShipController *>(ship->getController());
			if (shipController)
				shipController->receiveTransform(shipUpdateTransformMessage);
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType(DestroyShipComponentMessage::MessageType))
	{
		Archive::ReadIterator readIterator = NON_NULL(gnm)->getByteStream().begin();
		DestroyShipComponentMessage const destroyShipComponentMessage(readIterator);

		ClientObject * const object = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(destroyShipComponentMessage.getShipId()));
		if (object != NULL)
		{
			ShipObject * const shipObject = object->asShipObject();
			if (shipObject != NULL)
			{
				shipObject->handleComponentDestruction(destroyShipComponentMessage.getChassisSlot(), destroyShipComponentMessage.getSeverity());
			}
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType(DestroyShipMessage::MessageType))
	{
		Archive::ReadIterator readIterator = NON_NULL(gnm)->getByteStream().begin();
		DestroyShipMessage const destroyShipMessage(readIterator);

		ClientObject * const object = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(destroyShipMessage.getShipId()));
		if (object != NULL)
		{
			ShipObject * const shipObject = object->asShipObject();
			if (shipObject != NULL)
			{
				if (shipObject != Game::getPlayerContainingShip())
				{
					shipObject->startShipDestructionSequence(destroyShipMessage.getSeverity());
				}
				else
				{
					IGNORE_RETURN(new SpaceDeath(getCurrentCamera(), Game::getPlayerContainingShip(), destroyShipMessage.getSeverity()));
				}
			}
		}

	}

	//----------------------------------------------------------------------

	else if (message.isType("ShipUpdateTransformCollisionMessage"))
	{
		Archive::ReadIterator readIterator = NON_NULL(gnm)->getByteStream().begin();
		ShipUpdateTransformCollisionMessage const shipUpdateTransformCollisionMessage(readIterator);

		Object * const object = NetworkIdManager::getObjectById(shipUpdateTransformCollisionMessage.getNetworkId());
		if (object != 0)
		{
			Object const * playerShip = Game::getPlayerPilotedShip();
			if (object == playerShip)
			{
				PlayerShipController * const shipController = safe_cast<PlayerShipController *>(object->getController());
				if (shipController != 0)
					shipController->receiveTransform(shipUpdateTransformCollisionMessage);
			}
			else
				DEBUG_WARNING(true, ("GroundScene::receiveMessage: received ShipUpdateTransformCollisionMessage for object %s that is not the player's ship %s", object->getDebugInformation().c_str(), playerShip ? playerShip->getDebugInformation().c_str() : 0));
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType("CommPlayerMessage"))
	{
		Archive::ReadIterator readIterator = NON_NULL(gnm)->getByteStream().begin();
		GenericValueTypeMessage<std::pair<bool, std::pair<NetworkId, std::pair<Unicode::String, std::pair<uint32, std::pair<std::string, float> > > > > > tauntMessage(readIterator);

		NetworkId const &taunter = tauntMessage.getValue().second.first;

		bool const chronicleMessage = tauntMessage.getValue().first;

		Unicode::String const &prosePackageOOB = tauntMessage.getValue().second.second.first;

		uint32 const appearanceOverloadSharedTemplateCrc = tauntMessage.getValue().second.second.second.first;
		char buffer[64];
		_itoa(appearanceOverloadSharedTemplateCrc, buffer, 10);
		std::string crcString(buffer);

		// do not allow empty sound effect strings.
		std::string const & soundEffectParam = tauntMessage.getValue().second.second.second.second.first;
		std::string const & soundEffect = soundEffectParam.empty() ? "null" : soundEffectParam;

		float const duration = tauntMessage.getValue().second.second.second.second.second;
		char durationBuffer[64];
		sprintf(durationBuffer, "%f", duration);

		Unicode::String result;
		IGNORE_RETURN(ProsePackageManagerClient::appendAllProsePackages(prosePackageOOB, result));

		Unicode::String actionParams =	Unicode::narrowToWide(taunter.getValueString() + " ") + 
										Unicode::narrowToWide(crcString + " ") + 
										Unicode::narrowToWide(soundEffect + " ") + 
										Unicode::narrowToWide(std::string(durationBuffer) + " ") +
										result;
		if(chronicleMessage)
			IGNORE_RETURN(CuiActionManager::performAction(CuiActions::commPlayerQuest, actionParams));
		else
			IGNORE_RETURN(CuiActionManager::performAction(CuiActions::commPlayer, actionParams));
	}

	else if(message.isType("HyperspaceMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		HyperspaceMessage const hyperspaceMessage(ri);

		ShipObject * const shipObject = safe_cast<ShipObject *>(NetworkIdManager::getObjectById(hyperspaceMessage.getOwnerId()));
		if ((shipObject != 0) && (shipObject != Game::getPlayerContainingShip()))
		{
			shipObject->onLeaveByHyperspace();
		}
	}

	// ----------------------------------------------------------------------

	else if (message.isType("IsFlattenedTheaterMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const GenericValueTypeMessage<std::pair<NetworkId, bool> > isFlattenedTheaterMessage(ri);
		IntangibleObject * const target = dynamic_cast<IntangibleObject *>(NetworkIdManager::getObjectById(isFlattenedTheaterMessage.getValue().first));
		if (target != NULL && isFlattenedTheaterMessage.getValue().second)
		{
			target->makeFlattenedTheater();
		}
	}
	//----------------------------------------------------------------------

	else if(message.isType("AiDebugString"))
	{
#ifdef _DEBUG
		Archive::ReadIterator readIterator = NON_NULL (gnm)->getByteStream().begin();
		GenericValueTypeMessage<std::pair<NetworkId, std::string> > genericMessage(readIterator);

		//DEBUG_REPORT_LOG(true, ("AiDebugString: %s\n", FormattedString<4096>().sprintf("%s %s", genericMessage.getValue().first.getValueString().c_str(), genericMessage.getValue().second.c_str())));

		AiDebugStringManager::add(genericMessage.getValue().first, genericMessage.getValue().second);
#endif // _DEBUG
	}
	else if (message.isType("SaveTextOnClient"))
	{
#if PRODUCTION == 0
		Archive::ReadIterator readIterator = NON_NULL (gnm)->getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::string, std::string> > genericMessage(readIterator);
		std::string const &filename = genericMessage.getValue().first;
		std::string const &filetext = genericMessage.getValue().second;
		// assure filename is [a-zA-Z][a-zA-Z0-9-_./]* and does not contain '..'
		if (!filename.empty() && isalpha(filename[0]))
		{
			bool ok = true;
			for (unsigned int i = 1; i < filename.length(); ++i)
			{
				if (   (   !isalnum(filename[i])
				        && filename[i] != '_'
				        && filename[i] != '/'
				        && filename[i] != '.'
				        && filename[i] != '-')
				    || (filename[i] == '.' && filename[i+1] == '.'))
				{
					ok = false;
					break;
				}
			}
			if (ok)
			{
				StdioFile outFile(filename.c_str(), "wb");
				if (outFile.isOpen())
				{
					outFile.write(filetext.size(), filetext.data());
					// tell the client about it
					std::string messageText("Saved file on client: ");
					messageText += filename;
					CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide(messageText));
				}
			}
		}
#endif // PRODUCTION == 0
	}
	else if (message.isType(SlowDownEffectMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		SlowDownEffectMessage msg(ri);

		// get the attacker
		Object * obj = NetworkIdManager::getObjectById(msg.getSource());
		if (obj != NULL && obj->asClientObject() != NULL && obj->asClientObject()->asCreatureObject() != NULL)
		{
			CreatureObject * attacker = obj->asClientObject()->asCreatureObject();

			// get the target
			obj = NetworkIdManager::getObjectById(msg.getTarget());
			if (obj != NULL && obj->asClientObject() != NULL && obj->asClientObject()->asTangibleObject() != NULL)
			{
				TangibleObject * defender = obj->asClientObject()->asTangibleObject();
				attacker->addSlowDownEffect(*defender, msg.getConeLength(), msg.getConeAngle(), msg.getSlopeAngle(), msg.getExpireTime());
			}
		}
	}
	else if (message.isType("LaunchBrowserMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const GenericValueTypeMessage<std::string> message(ri);
		Os::launchBrowser(message.getValue());
	}
	else if (message.isType("SetCommandCooldown"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const GenericValueTypeMessage<std::pair<uint32, std::pair<uint32, uint32> > > message(ri);
		ClientCommandQueue::setCommandCooldown(message.getValue().first, message.getValue().second.first, message.getValue().second.second);
	}
	else if (message.isType("fca11a62d23041008a4f0df36aa7dca6"))
	{
		Archive::ReadIterator ri = NON_NULL (gnm)->getByteStream().begin();
		const GenericValueTypeMessage<float> message(ri);

#if PRODUCTION == 0
		if (s_usePlayerServerSpeed)
#endif
		ms_playerServerMovementSpeed = message.getValue();
	}
}

//-----------------------------------------------------------------

namespace
{
	inline void testObject(Object & object, Object *& minimumObject, float & minimumDistanceSquared, const Vector & worldStart, const Vector & worldEnd )
	{
		if (object.getAppearance () == 0)
			return;

		const Vector & objectStart = object.rotateTranslate_w2o (worldStart);
		const Vector & objectEnd   = object.rotateTranslate_w2o (worldEnd);

		CollisionInfo result;
		if (object.getAppearance()->collide(objectStart, objectEnd, CollideParameters::cms_toolPickDefault, result))
		{
			Vector const & point = result.getPoint();
			float const distSquared = point.magnitudeBetweenSquared(objectStart);

			if (distSquared < minimumDistanceSquared)
			{
				minimumDistanceSquared = distSquared;
				minimumObject          = &object;
			}
		}
	}
}

//-----------------------------------------------------------------

Object * GroundScene::findObject (int x, int y) const
{
	const Camera * const camera = NON_NULL (m_cameras [m_currentView]);

	const Vector worldStart (camera->getPosition_w ());
	const Vector worldEnd   (worldStart + (CONST_REAL (10000) * camera->rotate_o2w (camera->reverseProjectInScreenSpace (x, y)))); //lint !e1702 // bug

	//-- test against all objects in world
	float        minimumDistanceSquared = REAL_MAX;
	Object *    minimumObject          = 0;

	TerrainObject * const terrainObject = TerrainObject::getInstance ();

	int i = 0;
	int otype = static_cast<int>(WOL_Intangible);
	for (i = 0; i < ClientWorld::getNumberOfObjects (otype); i++)
	{
		Object * const obj = ClientWorld::getObject (otype, i);
		if (obj && terrainObject != obj)
			testObject (*obj, minimumObject, minimumDistanceSquared, worldStart, worldEnd);
	}

	otype = static_cast<int>(WOL_Tangible);
	for (i = 0; i < ClientWorld::getNumberOfObjects (otype); i++)
	{
		Object * const obj = ClientWorld::getObject (otype, i);
		if (obj)
			testObject (*obj, minimumObject, minimumDistanceSquared, worldStart, worldEnd);
	}

	otype = static_cast<int>(WOL_TangibleNotTargetable);
	for (i = 0; i < ClientWorld::getNumberOfObjects (otype); i++)
	{
		Object * const obj = ClientWorld::getObject (otype, i);
		if (obj)
			testObject (*obj, minimumObject, minimumDistanceSquared, worldStart, worldEnd);
	}

	return minimumObject;
}

//-----------------------------------------------------------------

int GroundScene::findObjects (float left, float top, float right, float bottom, ClientObject **& result) const
{
	const Camera * const camera = NON_NULL (m_cameras [m_currentView]);

	std::vector<ClientObject *> objects;

	int i = 0;
	int otype = static_cast<int>(WOL_Intangible);
	for (i = 0; i < ClientWorld::getNumberOfObjects (otype); i++)
	{
		ClientObject * obj = dynamic_cast<ClientObject *>(ClientWorld::getObject (otype, i));
		if (obj == 0)
			continue;

		Vector screenpos;
		if (camera->projectInWorldSpace (obj->getPosition_w (), &screenpos.x, &screenpos.y, 0) &&
			screenpos.x >= left && screenpos.x <= right && screenpos.y >= top && screenpos.y <= bottom)
		{
			objects.push_back (obj);
		}
	}

	otype = static_cast<int>(WOL_Tangible);
	for (i = 0; i < ClientWorld::getNumberOfObjects (otype); i++)
	{
		ClientObject * obj = dynamic_cast<ClientObject *>(ClientWorld::getObject (otype, i));
		if (obj == 0)
			continue;

		Vector screenpos;
		if (camera->projectInWorldSpace (obj->getPosition_w (), &screenpos.x, &screenpos.y, 0) &&
			screenpos.x >= left && screenpos.x <= right && screenpos.y >= top && screenpos.y <= bottom)
		{
			objects.push_back (obj);
		}
	}

	otype = static_cast<int>(WOL_TangibleNotTargetable);
	for (i = 0; i < ClientWorld::getNumberOfObjects (otype); i++)
	{
		ClientObject * obj = dynamic_cast<ClientObject *>(ClientWorld::getObject (otype, i));
		if (obj == 0)
			continue;

		Vector screenpos;
		if (camera->projectInWorldSpace (obj->getPosition_w (), &screenpos.x, &screenpos.y, 0) &&
			screenpos.x >= left && screenpos.x <= right && screenpos.y >= top && screenpos.y <= bottom)
		{
			objects.push_back (obj);
		}
	}

	if (!objects.empty ())
	{
		result = new ClientObject * [objects.size ()];
		IGNORE_RETURN (std::copy (objects.begin (), objects.end (), result));
		return static_cast<int>(objects.size ());
	}

	return 0;
}

//-----------------------------------------------------------------
void GroundScene::pivotRotate (const float x, const float y)
{
	if (m_currentView != CI_free && !m_usingGodClientInteriorCamera)
		setView (CI_free);

	NOT_NULL (m_freeCameraInputMap);

	m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraPivotYaw,   x);
	m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraPivotPitch, y);
}
//-----------------------------------------------------------------
void GroundScene::pivotTranslate (const float x, const float y)
{
	if (m_currentView != CI_free && !m_usingGodClientInteriorCamera)
		setView (CI_free);

	NOT_NULL (m_freeCameraInputMap);

	if (x)
		m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraPivotTranslateX, x);
	if (y)
		m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraPivotTranslateY, y);
}
//-----------------------------------------------------------------
void GroundScene::pivotZoom (const float z)
{
	if (m_currentView != CI_free && !m_usingGodClientInteriorCamera)
		setView (CI_free);

	NOT_NULL (m_freeCameraInputMap);

	m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraPivotZoom, z);
}
//-----------------------------------------------------------------
void GroundScene::flyRotate (const float x, const float y)
{
	if (m_currentView != CI_free && !m_usingGodClientInteriorCamera)
		setView (CI_free);

	NOT_NULL (m_freeCameraInputMap);

	m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraYawMouse,   x);
	m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraPitchMouse, y);
}

//-----------------------------------------------------------------

void GroundScene::flyTranslate (const float x, const float y, const float z)
{
	if (m_currentView != CI_free && !m_usingGodClientInteriorCamera)
		setView (CI_free);

	NOT_NULL (m_freeCameraInputMap);

	if (x)
		m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraTranslateX, x);
	if (y)
		m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraTranslateY, y);
	if (z)
		m_freeCameraInputMap->getMessageQueue ()->appendMessage (CM_cameraTranslateZ, z);

}

//-----------------------------------------------------------------

void GroundScene::activateGodClientCamera ()
{
	if(!m_usingGodClientInteriorCamera)
	{
		if (m_currentView != CI_free)
			setView (CI_free);
	}
	m_usingGodClientCamera = true;
}

//-----------------------------------------------------------------

void GroundScene::deactivateGodClientCamera ()
{
	m_usingGodClientCamera = false;
}

//-----------------------------------------------------------------

void GroundScene::activateGodClientInteriorCamera ()
{
	m_usingGodClientInteriorCamera = true;
}

//-----------------------------------------------------------------

void GroundScene::deactivateGodClientInteriorCamera ()
{
	m_usingGodClientInteriorCamera = false;
}

//----------------------------------------------------------------------

MessageQueue * GroundScene::getCurrentMessageQueue ()
{
	if (m_currentView == CI_free)
		return m_freeCameraInputMap->getMessageQueue ();

	if (m_currentView == CI_debugPortal)
		return m_debugPortalCameraInputMap->getMessageQueue ();

	return m_inputMap->getMessageQueue ();
}

//----------------------------------------------------------------------

bool GroundScene::isFirstPerson () const
{
	if (m_currentView == CI_freeChase)
		return m_freeChaseCamera->isFirstPerson ();

	if (m_currentView == CI_cockpit)
		return m_cockpitCamera->isFirstPerson();

	if (   m_currentView == CI_debugPortal
	    || m_currentView == CI_free
	    || m_currentView == CI_shipTurret)
		return true;

	return false;
}

//----------------------------------------------------------------------

bool GroundScene::isLoading () const
{
	return m_loading;
}

//-------------------------------------------------------------------

void GroundScene::setServerTimeOffset(unsigned int offset)
{
	m_serverTimeOffset = offset;
}

//-------------------------------------------------------------------

unsigned int GroundScene::getServerTime() const
{
	return static_cast<unsigned int>(time(0))+m_serverTimeOffset;
}

//-------------------------------------------------------------------

void GroundScene::zoomCamera (const bool in)
{
	getPlayer ()->getController ()->appendMessage (CM_mouseWheel, in ? -1.f : 1.f);
}

//-------------------------------------------------------------------

void GroundScene::setModeCallback (ModeCallback modeCallback, void* const context)
{
	m_modeCallback = modeCallback;
	m_context = context;
	m_currentMode = m_overheadMap->getRenderMap ();
}

//----------------------------------------------------------------------

float GroundScene::getMouseSensitivity         ()
{
	return ms_mouseSensitivity;
}

//----------------------------------------------------------------------

void  GroundScene::setMouseSensitivity         (float f)
{
	ms_mouseSensitivity = f;
}

//----------------------------------------------------------------------

void GroundScene::handleEndBaselines(ClientObject *target)
{
	if (!target->isInitialized())
	{
		DEBUG_REPORT_LOG_PRINT(ms_logCreateMessages, ("end baselines id=%s\n", target->getNetworkId().getValueString().c_str()));
		target->endBaselines();
		ClientSecureTradeManager::onObjectCreated(CachedNetworkId(*target));
	}

	ContainedByProperty * prop = ContainerInterface::getContainedByProperty(*target);

	ClientObject *containedBy = target->getContainedBy();
	Container *container = NULL;
	if (containedBy)
	{
		container = ContainerInterface::getContainer(*containedBy);

		ClientObject *containedByBy = containedBy->getContainedBy();
		if (containedByBy)
		{
			Container *containerContainer = ContainerInterface::getContainer(*containedByBy);
			UNREF(containerContainer);
		}
	}
	else if (prop->getContainedByNetworkId() != NetworkId::cms_invalid)
	{
//		DEBUG_WARNING(true, ("Got endBaselines on %s contained by %s but could not find container.", target->getNetworkId().getValueString().c_str(), prop->getContainedByNetworkId().getValueString().c_str()));
///		delete target;
		return;
	}

	if (   !target->isInWorld()
	    && (   !containedBy
	        || (   containedBy->isInWorld()
	            && NON_NULL(container)->isContentItemExposedWith(*target))))
	{
		target->addToWorld();
	}
}

//----------------------------------------------------------------------

void GroundScene::loadInputMap()
{
	if (m_inputMap)
	{
		m_inputMap->setMessageQueue        (0);
		InputScheme::releaseGroundInputMap (*m_inputMap);
		m_inputMap = 0;
	}

	//-----------------------------------------------------------------
	//-- setup this inputmaps after the scene has been set into Game

	m_inputMap = InputScheme::fetchGroundInputMap ();
	NOT_NULL (m_inputMap);
	m_inputMap->setMessageQueue (getPlayer()->getController()->getMessageQueue ());

}

//----------------------------------------------------------------------

void GroundScene::setNoDraw(bool noDraw)
{
	m_noDraw=noDraw;
}

//----------------------------------------------------------------------

void GroundScene::turnOffOverheadMap()
{
	NOT_NULL(m_overheadMap);
	if(m_overheadMap->getRenderMap())
		m_overheadMap->toggle();
}

//----------------------------------------------------------------------
