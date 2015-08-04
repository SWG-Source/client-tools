// ======================================================================
//
// Game.cpp
// Portions Copyright 1999, Bootprint Entertainment, Inc.
// Portions Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/Game.h"

#include "clientAnimation/PlaybackScriptManager.h"
#include "clientAudio/Audio.h"
#include "clientAudio/Sound2.h"
#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientAudio/SwgAudioCapture.h"
#include "clientDirectInput/DirectInput.h"
#include "clientGame/AuctionManagerClient.h"
#include "clientGame/CellObject.h"
#include "clientGame/ClientCombatPlaybackManager.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientCommandTable.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientRegionManager.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/CollisionTest.h"
#include "clientGame/CommandCppFuncs.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/CutScene.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/DynamicNormalMapTest.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GroundScene.h"
#include "clientGame/HyperspaceIoWin.h"
#include "clientGame/InputActivityManager.h"
#include "clientGame/JgrillsTest.h"
#include "clientGame/LodShaderTest.h"
#include "clientGame/MoodManagerClient.h"
#include "clientGame/NebulaManagerClient.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/PlanetMapManagerClient.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/PreloadedAssetManager.h"
#include "clientGame/QuestJournalManager.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGame/RoadmapManager.h"
#include "clientGame/SetTextureTest.h"
#include "clientGame/ShadowVolumeTest.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientGame/SplitMeshTest.h"
#include "clientGame/TangibleObject.h"
#include "clientGame/TestIoWin.h"
#include "clientGame/TimeOfDayTest.h"
#include "clientGame/WaterTest.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderEffectList.h"
#include "clientGraphics/ShaderImplementation.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientGraphics/VideoPlaybackManager.h"
#include "clientObject/DetailAppearanceTemplate.h"
#include "clientObject/GameCamera.h"
#include "clientObject/ShadowManager.h"
#include "clientSkeletalAnimation/AnimationStateNameId.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"
#include "clientSkeletalAnimation/CharacterLodManager.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplateList.h"
#include "clientSkeletalAnimation/LookAtTransformModifier.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "clientSkeletalAnimation/TargetPitchTransformModifier.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_Cache.h"
#include "clientTextureRenderer/TextureRendererList.h"
#include "clientTextureRenderer/TextureRendererManager.h"
#include "clientTextureRenderer/TextureRendererTemplate.h"
#include "clientUserInterface/CuiLoadingManager.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedDebug/DataLint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/DebugMonitor.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/PixCounter.h"
#include "sharedDebug/Profiler.h"
#include "sharedDebug/VTune.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/FileManifest.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/GameScheduler.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/QuestManager.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedInputMap/InputMap.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/MessageManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ArrangementDescriptor.h"
#include "sharedObject/ArrangementDescriptorList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedObject/PortalPropertyTemplateList.h"
#include "sharedObject/SlotDescriptor.h"
#include "sharedObject/SlotDescriptorList.h"
#include "sharedObject/World.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include "swgSharedNetworkMessages/MessageQueueCombatAction.h"
#include "swgSharedUtility/Postures.def"
#include "libMozilla/libMozilla.h"

#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "UnicodeUtils.h"

#include <cstdio>

#if PRODUCTION == 0
#include "clientGraphics/SwgVideoCapture.h"
#endif // PRODUCTION

//-----------------------------------------------------------------

namespace GameNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const Game::Messages::DebugPrintUi::Payload &,   Game::Messages::DebugPrintUi>   debugPrintUi;
		MessageDispatch::Transceiver<const Game::Messages::ChatStartInput::Payload &, Game::Messages::ChatStartInput> chatStartInput;
		MessageDispatch::Transceiver<const Game::Messages::SceneChanged::Payload &,   Game::Messages::SceneChanged>   sceneChanged;
		MessageDispatch::Transceiver<const Game::Messages::CollectionServerFirstChanged::Payload &, Game::Messages::CollectionServerFirstChanged> collectionServerFirstChanged;
		MessageDispatch::Transceiver<const Game::Messages::CollectionShowServerFirstOptionChanged::Payload &, Game::Messages::CollectionShowServerFirstOptionChanged> collectionShowServerFirstOptionChanged;
	}

	class Sender : public MessageDispatch::Emitter
	{
		~Sender () {}
	};

	// =============================================================================

	class SceneCreator
	{
	public:
		// - - - - - - - - - - - - - - - - - - - - - - - -
		SceneCreator(
			const char *const terrainFilename, 
			const Vector &    startPosition
			)
		:	 m_terrainFilename(DuplicateString(terrainFilename))
			,m_startPosition(startPosition)
		{}
		virtual ~SceneCreator() { delete [] m_terrainFilename; }
		// - - - - - - - - - - - - - - - - - - - - - - - -
		virtual GroundScene *create()=0;
		virtual NetworkId getPlayerNetworkId()=0;
		virtual void beginDeferredCreation() {}
		virtual void endDeferredCreation() {}

		// - - - - - - - - - - - - - - - - - - - - - - - -
		char *const   m_terrainFilename;
		const Vector  m_startPosition;
		// - - - - - - - - - - - - - - - - - - - - - - - -
	private:
		SceneCreator(const SceneCreator &);
		SceneCreator&operator=(const SceneCreator&);
	};

	//-----------------------------------------------------------------

	class SinglePlayerSceneCreator : public SceneCreator
	{
	public:
		// - - - - - - - - - - - - - - - - - - - - - - - -

		SinglePlayerSceneCreator(
			const char* terrainFilename, 
			const char* playerFilename, 
			CreatureObject* customizedPlayer
			)
		:	 SceneCreator(terrainFilename, ConfigClientGame::getSinglePlayerStartLocation())
			,m_playerFilename(DuplicateString(playerFilename))
			,m_customizedPlayer(customizedPlayer)
		{}

		~SinglePlayerSceneCreator() { delete [] m_playerFilename; }

		// - - - - - - - - - - - - - - - - - - - - - - - -

		GroundScene *create() { return new GroundScene(m_terrainFilename, m_playerFilename, m_customizedPlayer); }
		NetworkId getPlayerNetworkId() { return NetworkId(); }

		// - - - - - - - - - - - - - - - - - - - - - - - -
		char *const           m_playerFilename;
		CreatureObject *const m_customizedPlayer;
		// - - - - - - - - - - - - - - - - - - - - - - - -
	private:
		SinglePlayerSceneCreator(const SinglePlayerSceneCreator &);
		SinglePlayerSceneCreator&operator=(const SinglePlayerSceneCreator&);
	};

	//-----------------------------------------------------------------

	class MultiPlayerSceneCreator : public SceneCreator
	{
	public:
		MultiPlayerSceneCreator(
			const char *const terrainFilename, 
			const NetworkId&  playerNetworkId, 
			const char *const templateName, 
			const Vector &    startPosition, 
			const float       startYaw, 
			const float       timeInSeconds, 
			const bool        disableSnapshot
			)
		:	 SceneCreator(terrainFilename, startPosition)
			,m_playerNetworkId(playerNetworkId)
			,m_templateName(DuplicateString(templateName))
			,m_startYaw(startYaw)
			,m_timeInSeconds(timeInSeconds)
			,m_disableSnapshot(disableSnapshot)
			,m_deferred(false)
		{}

		~MultiPlayerSceneCreator()
		{
			if (m_deferred)
			{
				endDeferredCreation();
			}
			delete [] m_templateName;
		}

		// - - - - - - - - - - - - - - - - - - - - - - - -

		GroundScene *create()
		{
			return new GroundScene(
				m_terrainFilename,
				m_playerNetworkId, 
				m_templateName, 
				m_startPosition, 
				m_startYaw, 
				m_timeInSeconds, 
				m_disableSnapshot
			);
		}

		NetworkId getPlayerNetworkId() { return m_playerNetworkId; }

		void beginDeferredCreation()
		{
			GameNetwork::beginDeferringConnectionServerMessages();
			m_deferred=true;
		}

		void endDeferredCreation() 
		{
			if (m_deferred)
			{
				m_deferred=false;
				GameNetwork::endDeferringConnectionServerMessages();
			}
		}

		// - - - - - - - - - - - - - - - - - - - - - - - -

		const NetworkId   m_playerNetworkId;
		char *const       m_templateName;
		const float       m_startYaw;
		const float       m_timeInSeconds;
		const bool        m_disableSnapshot;

		// - - - - - - - - - - - - - - - - - - - - - - - -

		bool              m_deferred;

		// - - - - - - - - - - - - - - - - - - - - - - - -
	private:
		MultiPlayerSceneCreator(const MultiPlayerSceneCreator &);
		MultiPlayerSceneCreator&operator=(const MultiPlayerSceneCreator&);
	};

	// =============================================================================

	float const cs_lookatConeAngle = 160.0f * PI_OVER_180;
	const char * const  s_professionMovieFormatString        = "video/profession_%s.bik";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Timer ms_networkTimer (RECIP (20));

	bool ms_garbageCollectNextFrame;

	bool ms_reportNetworkTraffic;

	float s_historyTimeSeconds = 0.0f;

	int s_receivedUncompressedBytesPerSecond = 0;
	int s_receivedHistoryUncompressedBytesPerSecond = 0;
	unsigned long s_receivedHistoryUncompressedBytes = 0;

	int s_receivedCompressedBytesPerSecond = 0;
	int s_receivedHistoryCompressedBytesPerSecond = 0;
	unsigned long s_receivedHistoryCompressedBytes = 0;

	int s_sentUncompressedBytesPerSecond = 0;
	int s_sentHistoryUncompressedBytesPerSecond = 0;
	unsigned long s_sentHistoryUncompressedBytes = 0;

	int s_sentCompressedBytesPerSecond = 0;
	int s_sentHistoryCompressedBytesPerSecond = 0;
	unsigned long s_sentHistoryCompressedBytes = 0;

	Game::GameOptionChangedCallback ms_gameOptionChangedCallback = NULL;
	Game::ExternalCommandHandler ms_externalCommandHandler = NULL;

	ConstCharCrcLowerString const  cs_combatAnimationStateName("combat");
	AnimationStateNameId           s_combatAnimationStateId;

	float                          s_cosHalfLookatConeAngle;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  allowLookAtTarget(SkeletalAppearance2 const &lookerAppearance);
	void  destroyedAttachmentWearableCallback(Object &object);
	bool  manageCharacterLodCallback(Object &object);
	void  preloadAssets ();
	void  alterNetworkBandwidthCalculation(const float deltaTime);
	bool  isCellAccessAllowed(CellProperty const &cellProperty);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	unsigned long ms_bytesAllocated[5];
	char ms_bytesAllocatedBuffer[256];

	char   ms_loopCountBuffer[64];
	time_t ms_installTime;
	char   ms_upTimeBuffer[64];

	bool ms_verifyGuardPatterns;
	bool ms_verifyFreePatterns;

	bool ms_isSpace;
	bool ms_useSpaceHudOnly = false;

	float ms_groundBrightness;
	float ms_groundContrast;
	float ms_groundGamma;
	float ms_spaceBrightness;
	float ms_spaceContrast;
	float ms_spaceGamma;

	bool ms_endCutScene;
	bool ms_cutScene;
	SceneCreator *ms_nextScene;
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string s_collectionServerFirstListVersion;
}

using namespace GameNamespace;

//-------------------------------------------------------------------

const char * const  Game::Messages::SCENE_CHANGED        = "Game::SCENE_CHANGED";

//-----------------------------------------------------------------

Scene*                     Game::ms_scene;
bool                       Game::ms_singlePlayer;
int                        Game::ms_loops;
bool                       Game::ms_done;
Game::Application          Game::ms_application;
MessageDispatch::Emitter * Game::ms_emitter;
float                      Game::ms_elapsedTime;
std::string                Game::ms_sceneId;
std::string                Game::ms_nonInstanceSceneId;
bool                       Game::ms_useExitTimer;
Timer                      Game::ms_exitTimer;
NetworkId                  Game::ms_lastPlayerId;
std::string                Game::ms_lastPlayerCluster;
std::string                Game::ms_lastPlayerLoginId;
bool                       Game::ms_profanityFiltered = true;
Game::SceneType            Game::ms_lastHudSceneType = Game::ST_numTypes;
float                      Game::ms_radarRange = 2048.f;
int                        Game::ms_gameFeatureBits = 0;
int                        Game::ms_subscriptionFeatureBits = 0;
int                        Game::ms_serverSideGameFeatureBits = 0;
int                        Game::ms_serverSideSubscriptionFeatureBits = 0;
int                        Game::ms_connectionServerId = 0;

#ifdef _DEBUG
void runDataLint(char const *responsePath);
void verifyUpdateRanges (const char* filename);
#endif // _DEBUG

// ======================================================================
// namespace GameNamespace
// ======================================================================

void Game::garbageCollect (bool const immediate)
{
	const int numberOfMegabytesAllocated = MemoryManager::getCurrentNumberOfBytesAllocated () / (1024 * 1024);
	const int limit = MemoryManager::getLimit() ;

	if (ms_garbageCollectNextFrame || immediate || numberOfMegabytesAllocated > (limit * 9) / 10)
	{
		ms_garbageCollectNextFrame = false;

		//-- garbage collect object templates
		ObjectTemplateList::garbageCollect ();

		//-- garbage collect appearance templates
		AppearanceTemplateList::garbageCollect ();

		//-- garbage collect terrain system
		ClientProceduralTerrainAppearance::Cache::garbageCollect ();

		//-- garbage collect animation system
		LogicalAnimationTableTemplateList::garbageCollect ();

//		LocalizationManager::getManager ().garbageCollectUnused ();
#ifdef _DEBUG
		size_t const memorySaved = numberOfMegabytesAllocated - MemoryManager::getCurrentNumberOfBytesAllocated () / (1024 * 1024);
		DEBUG_REPORT_LOG(memorySaved > 0, ("GameNamespace::garbageCollect: just freed %i MB\n", memorySaved));
#endif
	}
}

// ----------------------------------------------------------------------

bool GameNamespace::allowLookAtTarget(SkeletalAppearance2 const &lookerAppearance)
{
	//-- Disallow if looker is dead or incapacitated.
	Object const *owner = lookerAppearance.getOwner();
	if (!owner)
		return false;

	ClientObject const *clientOwner = owner->asClientObject();
	if (clientOwner)
	{
		CreatureObject const *creatureOwner = clientOwner->asCreatureObject();
		if (creatureOwner)
		{
			Postures::Enumerator const posture = creatureOwner->getVisualPosture();
			if ((posture == Postures::Incapacitated) || (posture == Postures::Dead))
				return false;
		}
	}

	//-- Disallow if looker is in combat.
	// Get looker object's animation controller.
	StateHierarchyAnimationController const *const controller = safe_cast<StateHierarchyAnimationController const*>(lookerAppearance.getAnimationController());
	if (controller)
	{
		//-- if looker is in combat, the head turning needs to be turned off since several combat animations,
		//   particularly combo moves, explicitly want to control the neck.  An alternate solution would be to
		//   work in a way for the .LAT file to specify which animations allow/disallow look at target processing.
		AnimationStatePath const &path = controller->getDestinationPath();
		if (path.hasState(s_combatAnimationStateId))
		{
			// Prevent look at due to looker in the combat branch of the animation state hierarchy.
			return false;
		}
	}

	//-- Disallow if look-at target doesn't exist.
	// Get skeletal appearance for looker.
	SkeletalAppearance2 const *lookerSkeletalAppearance = lookerAppearance.asSkeletalAppearance2();
	if (!lookerSkeletalAppearance)
		return false;

	// Get target object for looker.
	Object const *const targetObject = lookerSkeletalAppearance->getTargetObject();
	if (!targetObject)
		return lookerSkeletalAppearance->isTargetPositionValid ();

	//-- Disallow if look-at target is the player's mount.
	CreatureObject *const playerCreatureObject = Game::getPlayerCreature();
	if (playerCreatureObject && playerCreatureObject->isRidingMount() && (playerCreatureObject->getMountedCreature() == targetObject))
		return false;

	//-- Disallow if look-at target is outside the view cone.

	// Get target object position in looker space.
	Vector targetDirection_lookerSpace = lookerSkeletalAppearance->getTargetPosition_o();//owner->getTransform_o2w().rotateTranslate_p2l(targetObject->getPosition_w());
	targetDirection_lookerSpace.y = 0.0f;
	if (!targetDirection_lookerSpace.normalize())
		return false;

	float const  cosAngleBetween = Vector::unitZ.dot(targetDirection_lookerSpace);

	//-- Allow if angle between (in X-Z plane) is within view cone.
	return (cosAngleBetween >= s_cosHalfLookatConeAngle);
}

// ----------------------------------------------------------------------

void GameNamespace::destroyedAttachmentWearableCallback(Object &object)
{
	//-- Tell the tangible object (most likely a creature) to do whatever it should do when container contents have changed.
	TangibleObject *const tangibleObject = dynamic_cast<TangibleObject*>(&object);
	if (tangibleObject)
	{
		DEBUG_WARNING(true, ("skeletal appearance had to report a container change back to object [%s] because the container code didn't tell the object/appearance about it.", object.getObjectTemplateName()));
		tangibleObject->doPostContainerChangeProcessing();
	}
}

// ----------------------------------------------------------------------

bool GameNamespace::manageCharacterLodCallback(Object &object)
{
	//-- Don't have the character lod manager manage the player.  We'll set that here ourselves.
	if (&object == Game::getPlayer())
	{
		return false;
	}

	//-- If we want the player's group or enemies to be higher res, we can do that here.
	return true;
} //lint !e1764 // ref

// ----------------------------------------------------------------------

void GameNamespace::preloadAssets ()
{
	if (ConfigFile::getKeyBool ("ClientGame", "disablePreloadedAssetManager", false))
		return;

	unsigned long const startAllocatedBytes = MemoryManager::getCurrentNumberOfBytesAllocated();
	unsigned long const startTimeMs = Clock::timeMs();

	PreloadedAssetManager::install ();

	unsigned long const stopTimeMs = Clock::timeMs();
	unsigned long const stopAllocatedBytes = MemoryManager::getCurrentNumberOfBytesAllocated();

	REPORT_LOG(true, ("Preloading took [%.2f] seconds and increased memory usage by [%.2f] MB.\n",
		static_cast<float>(stopTimeMs - startTimeMs) / 1000.0f,
		static_cast<float>(stopAllocatedBytes - startAllocatedBytes) / (1024.0f * 1024.0f)));
}

//-------------------------------------------------------------------

void GameNamespace::alterNetworkBandwidthCalculation(const float deltaTime)
{
	static float timer = 0.0f;

	timer += deltaTime;

	if (timer > 1.0f)
	{
		// Received uncompressed
		{
			static int previousReceivedTotalUncompressedBytes = 0;
			unsigned long const deltaReceivedTotalUncompressedBytes = static_cast<unsigned long>(NetworkHandler::getRecvTotalUncompressedByteCount() - previousReceivedTotalUncompressedBytes);
			s_receivedUncompressedBytesPerSecond = static_cast<int>(static_cast<float>(deltaReceivedTotalUncompressedBytes) / timer);
			previousReceivedTotalUncompressedBytes = NetworkHandler::getRecvTotalUncompressedByteCount();
			s_receivedHistoryUncompressedBytes += deltaReceivedTotalUncompressedBytes;
		}

		// Received compressed
		{
			static int previousReceivedTotalCompressedBytes = 0;
			unsigned long const deltaReceivedTotalCompressedBytes = static_cast<unsigned long>(NetworkHandler::getRecvTotalCompressedByteCount() - previousReceivedTotalCompressedBytes);
			s_receivedCompressedBytesPerSecond = static_cast<int>(static_cast<float>(deltaReceivedTotalCompressedBytes) / timer);
			previousReceivedTotalCompressedBytes = NetworkHandler::getRecvTotalCompressedByteCount();
			s_receivedHistoryCompressedBytes += deltaReceivedTotalCompressedBytes;
		}

		// Sent uncompressed
		{
			static int previousSentTotalUncompressedBytes = 0;
			unsigned long const deltaSentTotalUncompressedBytes = static_cast<unsigned long>(NetworkHandler::getSendTotalUncompressedByteCount() - previousSentTotalUncompressedBytes);
			s_sentUncompressedBytesPerSecond = static_cast<int>(static_cast<float>(deltaSentTotalUncompressedBytes) / timer);
			previousSentTotalUncompressedBytes = NetworkHandler::getSendTotalUncompressedByteCount();
			s_sentHistoryUncompressedBytes += deltaSentTotalUncompressedBytes;
		}

		// Sent compressed
		{
			static int previousSentTotalCompressedBytes = 0;
			unsigned long const deltaSentTotalCompressedBytes = static_cast<unsigned long>(NetworkHandler::getSendTotalCompressedByteCount() - previousSentTotalCompressedBytes);
			s_sentCompressedBytesPerSecond = static_cast<int>(static_cast<float>(deltaSentTotalCompressedBytes) / timer);
			previousSentTotalCompressedBytes = NetworkHandler::getSendTotalCompressedByteCount();
			s_sentHistoryCompressedBytes += deltaSentTotalCompressedBytes;
		}

		// Scale to avoid overflow and to decrease importance of traffic from the past
		{
			s_historyTimeSeconds += timer;

			static const float resetTimeSecond = static_cast<float>(ConfigClientGame::getNetworkTrafficMetricsResetTimeSecond());
			static const unsigned long resetScale = static_cast<unsigned long>(ConfigClientGame::getNetworkTrafficMetricsResetScaleFactor());
			if ((s_historyTimeSeconds > resetTimeSecond) && (resetTimeSecond >= 1.0f) && (resetScale > 1))
			{
				do
				{
					s_historyTimeSeconds               /= resetScale;
					s_receivedHistoryUncompressedBytes /= resetScale;
					s_receivedHistoryCompressedBytes   /= resetScale;
					s_sentHistoryUncompressedBytes     /= resetScale;
					s_sentHistoryCompressedBytes       /= resetScale;
				} while (s_historyTimeSeconds > resetTimeSecond);
			}
		}

		// Recalculate
		{
			s_receivedHistoryUncompressedBytesPerSecond = static_cast<int>(static_cast<float>(s_receivedHistoryUncompressedBytes) / s_historyTimeSeconds);
			s_receivedHistoryCompressedBytesPerSecond   = static_cast<int>(static_cast<float>(s_receivedHistoryCompressedBytes) / s_historyTimeSeconds);
			s_sentHistoryUncompressedBytesPerSecond     = static_cast<int>(static_cast<float>(s_sentHistoryUncompressedBytes) / s_historyTimeSeconds);
			s_sentHistoryCompressedBytesPerSecond       = static_cast<int>(static_cast<float>(s_sentHistoryCompressedBytes) / s_historyTimeSeconds);
		}

		DEBUG_REPORT_PRINT (ms_reportNetworkTraffic, ("r: %i bytes  s: %i bytes (per second)\n", s_receivedCompressedBytesPerSecond, s_sentCompressedBytesPerSecond));

		timer = 0.0f;
	}
}

// ----------------------------------------------------------------------
/**
 * Called by CellProperty to determine if a cell is accessible to the player.
 */

bool GameNamespace::isCellAccessAllowed(CellProperty const &cellProperty)
{
	//-- Mounts: check if the player is riding a mount. If so, the player
	//   is not allowed to enter any cells.
	CreatureObject const *const player = Game::getPlayerCreature();
	if (player && player->getState(States::RidingMount))
		return false;

	//-- If we're in the world cell, we're allowed to access the cell
	if (&cellProperty == CellProperty::getWorldCellProperty())
		return true;

	//-- Check the associated CellObject to see if the player can enter the cell.
	CellObject const *const cellObject = safe_cast<CellObject const*>(&(cellProperty.getOwner()));
	if (cellObject)
		return cellObject->getAccessAllowed();

	return true;
}

// ======================================================================
// class Game
// ======================================================================

void Game::install(Application const application)
{
	InstallTimer const installTimer("Game::install");

	DEBUG_FATAL (ms_emitter, ("already installed\n"));
	DebugFlags::registerFlag (ms_garbageCollectNextFrame, "ClientGame", "garbageCollectNextFrame");
	DebugFlags::registerFlag (ms_reportNetworkTraffic, "ClientGame", "reportNetworkTraffic");
	DebugFlags::registerFlag (ms_verifyGuardPatterns, "ClientGame", "verifyGuardPatterns");
	DebugFlags::registerFlag (ms_verifyFreePatterns, "ClientGame", "verifyFreePatterns");
	DebugFlags::registerFlag(ms_useSpaceHudOnly, "ClientGame", "useSpaceHudOnly");

	sprintf(ms_bytesAllocatedBuffer, "BytesAllocated: %d %d %d %d %d\n", -1, -1, -1, -1, -1);
	CrashReportInformation::addDynamicText(ms_bytesAllocatedBuffer);

	sprintf(ms_loopCountBuffer, "MainLoop: %d\n", -1);
	CrashReportInformation::addDynamicText(ms_loopCountBuffer);

	IGNORE_RETURN(time(&ms_installTime));
	sprintf(ms_upTimeBuffer, "UpTime: %d\n", -1);
	CrashReportInformation::addDynamicText(ms_upTimeBuffer);

	ms_elapsedTime = 0.0f;
	ms_emitter = new Sender;

	//-- add Game::remove to ExitChain
	ExitChain::add(Game::remove, "Game::remove");

	ms_groundBrightness = LocalMachineOptionManager::findFloat("Graphics", "brightness", Game::getDefaultBrightness());
	LocalMachineOptionManager::registerOption(ms_groundBrightness, "Game", "groundBrightness");

	ms_groundContrast = LocalMachineOptionManager::findFloat("Graphics", "contrast", Game::getDefaultContrast());
	LocalMachineOptionManager::registerOption(ms_groundContrast, "Game", "groundContrast");

	ms_groundGamma = LocalMachineOptionManager::findFloat("Graphics", "gamma", Game::getDefaultGamma());
	LocalMachineOptionManager::registerOption(ms_groundGamma, "Game", "groundGamma");

	ms_spaceBrightness = 1.f;
	LocalMachineOptionManager::registerOption(ms_spaceBrightness, "Game", "spaceBrightness");

	ms_spaceContrast = 1.f;
	LocalMachineOptionManager::registerOption(ms_spaceContrast, "Game", "spaceContrast");

	ms_spaceGamma = 1.f;
	LocalMachineOptionManager::registerOption(ms_spaceGamma, "Game", "spaceGamma");

	//-- this viewer does not need the UI
	if (ConfigClientGame::getTestIoWin ())
	{
		if (_stricmp (ConfigClientGame::getTestIoWin (),      "TestIoWin") == 0)
			(new TestIoWin ())->open();
		else if (_stricmp (ConfigClientGame::getTestIoWin (), "ShadowVolumeTest") == 0)
			(new ShadowVolumeTest ())->open();
		else if (_stricmp (ConfigClientGame::getTestIoWin (), "DynamicNormalMapTest") == 0)
			(new DynamicNormalMapTest ())->open ();
		else if (_stricmp (ConfigClientGame::getTestIoWin (), "TimeOfDayTest") == 0)
			(new TimeOfDayTest ())->open ();
		else if (_stricmp (ConfigClientGame::getTestIoWin (), "JgrillsTest") == 0)
			(new JgrillsTest ())->open ();
		else if (_stricmp (ConfigClientGame::getTestIoWin (), "LodShaderTest") == 0)
			(new LodShaderTest ())->open ();
		else if (_stricmp (ConfigClientGame::getTestIoWin (), "SetTextureTest") == 0)
			(new SetTextureTest ())->open ();
		else if (_stricmp (ConfigClientGame::getTestIoWin (), "WaterTest") == 0)
			(new WaterTest ())->open ();
		else if (_stricmp (ConfigClientGame::getTestIoWin (), "SplitMeshTest") == 0)
			(new SplitMeshTest)->open();
		else
			FATAL (true, ("testIoWin specified but not recognized"));
	}
	else
	{
		ms_singlePlayer = true;
		ms_application = application;


#if PRODUCTION == 0
		if ((application == A_particleEditor || application == A_animationEditor)
			&& !ConfigFile::getKeyBool("ClientTools","loadHud",false)
		)
		{

			ClientRegionManager::install();
			ExitChain::add(ClientRegionManager::remove, "ClientRegionManager::remove");

		}
		else
#endif
		{

			ms_profanityFiltered = ConfigClientGame::isProfanityFiltered ();
			CurrentUserOptionManager::registerOption(ms_profanityFiltered, "ClientGame", "profanityFiltered");

			MoodManagerClient::install ();
			ExitChain::add(MoodManagerClient::remove, "MoodManagerClient::remove");

			ObjectAttributeManager::install ();
			ExitChain::add(ObjectAttributeManager::remove, "ObjectAttributeManager::remove");

			DraftSchematicManager::install ();
			ExitChain::add(DraftSchematicManager::remove, "DraftSchematicManager::remove");

			AuctionManagerClient::install ();
			ExitChain::add(AuctionManagerClient::remove, "AuctionManagerClient::remove");

			PlanetMapManagerClient::install ();
			ExitChain::add(PlanetMapManagerClient::remove, "PlanetMapManagerClient::remove");

			ResourceIconManager::install ();
			ExitChain::add(ResourceIconManager::remove, "ResourceIconManager::remove");

			ClientRegionManager::install();
			ExitChain::add(ClientRegionManager::remove, "ClientRegionManager::remove");

			QuestJournalManager::install();
			ExitChain::add(QuestJournalManager::remove, "QuestJournalManager::remove");

			RoadmapManager::install();
			ExitChain::add(RoadmapManager::remove, "RoadmapManager::remove");

			QuestManager::install();

			// install any client side command table functions
			CommandCppFuncs::install();

			// install the command queue
			ClientCommandQueue::install();

			//-- setup client combat playback manager
			ClientCombatPlaybackManager::install("combat/combat_manager.iff");

#ifdef _DEBUG
			MessageQueueCombatAction::setActionNameLookupFunction(ClientCombatPlaybackManager::lookupActionName);
#endif

			//-- Setup player portal access control.
			CellProperty::setAccessAllowedHookFunction(isCellAccessAllowed);

			//-- setup what to do if a skeletal appearance detects an unnotified deletion of a wearable or attached item.
			//   note this shouldn't be needed but our container system doesn't always tell me about these.
			SkeletalAppearance2::setContainsDestroyedAttachmentWearableCallback(destroyedAttachmentWearableCallback);
		}

		//-- setup network
		GameNetwork::install();

		// -----------------------------------------------------------------------------------

		CutScene::install();

		// -----------------------------------------------------------------------------------

#ifdef _DEBUG
		//-- see if the gameserver is to be run in a mode to scan update ranges
		if (ConfigFile::getKeyBool ("SwgClient", "verifyUpdateRanges", false))
		{
			verifyUpdateRanges ("../../exe/win32/sharedobjecttemplates.txt");
		}
		else
		if (!ConfigFile::getKeyBool("ClientGame/DataLint", "disable", true))
		{
			runDataLint("../../exe/win32/DataLint.rsp");
		}
		else
#endif // _DEBUG
		{
#if PRODUCTION == 0
			if (application == A_particleEditor || application == A_animationEditor)
			{
				ExitChain::add(Game::cleanupScene, "Game::cleanupScene");
				CuiManager::install ();
				ExitChain::add(CuiManager::remove, "CuiManager::remove");

				preloadAssets ();

				ms_singlePlayer = true;
				setScene(
					false,
					ConfigClientGame::getParticleEditorGroundScene(), 
					ConfigClientGame::getParticleEditorAvatarSelection(), 
					0
				);
			}
			//-- this viewer does not need the scene
			else if (application == A_npcEditor)
			{
				ExitChain::add(Game::cleanupScene, "Game::cleanupScene");
				CuiManager::install ();
				ExitChain::add(CuiManager::remove, "CuiManager::remove");

				ms_singlePlayer = true;
			}
			//-- this viewer does not need the UI
			else if (ConfigClientGame::getCuiTest())
			{
				std::string result;
				if (CuiManager::test (result))
					DEBUG_FATAL (true, ("Cui Test Failed:\n%s\n", result.c_str ()));
				else
					REPORT_LOG_PRINT (true, ("Cui Test Ok\n"));
				quit ();
			}
			//-- this viewer does not need the UI
			else if (ConfigClientGame::getCuiStringList())
			{
				CuiManager::generateStringList ();
				quit ();
			}
			//-- this viewer does not need the UI
			else if (ConfigFile::getKeyBool("ClientGame", "collisionTest", false))
			{
				(new CollisionTest())->open();
			}
			//-- everything else needs the UI
			else
#endif
			{
				ExitChain::add(Game::cleanupScene, "Game::cleanupScene");

				//-- install UI Layer

				CuiManager::install ();
				ExitChain::add(CuiManager::remove, "CuiManager::remove");

				{
					char asynchronousLoaderFileName[64];
					sprintf(asynchronousLoaderFileName, "misc/asynchronous_loader_data_%d.iff", GetShaderCapabilityMajor(Graphics::getShaderCapability()));
					AsynchronousLoader::install(asynchronousLoaderFileName);
				}

				MeshGeneratorTemplateList::assignAsynchronousLoaderFunctions();
				PaletteArgbList::assignAsynchronousLoaderFunctions();
				TextureList::assignAsynchronousLoaderFunctions();
				ShaderImplementation::Pass::VertexShader::assignAsynchronousLoaderFunctions();
				ShaderImplementation::Pass::PixelShader::Program::assignAsynchronousLoaderFunctions();
				ShaderEffectList::assignAsynchronousLoaderFunctions();
				ShaderTemplateList::assignAsynchronousLoaderFunctions();

				s_combatAnimationStateId = AnimationStateNameIdManager::createId(cs_combatAnimationStateName);
				s_cosHalfLookatConeAngle = cos(0.5f * cs_lookatConeAngle);
				LookAtTransformModifier::setAllowLookAtTargetFunction(allowLookAtTarget);
				CharacterLodManager::setManageLodCallback(manageCharacterLodCallback);

#if PRODUCTION == 0
				//-- Tell these classes how to get the current camera for debug purposes.
				LookAtTransformModifier::setGetCameraFunction(Game::getConstCamera);
				TargetPitchTransformModifier::setGetCameraFunction(Game::getConstCamera);

				//-- menu viewer
				if (ConfigClientGame::getMenuViewer () != 0 && strlen (ConfigClientGame::getMenuViewer ()))
				{
					ms_singlePlayer = true;
					IGNORE_RETURN(CuiMediatorFactory::activate (CuiMediatorTypes::Backdrop));
					IGNORE_RETURN(CuiMediatorFactory::activate (ConfigClientGame::getMenuViewer ()));
				}
				else if (ConfigClientGame::getGroundScene () && ConfigClientGame::getAvatarSelection ())
				{
					FATAL (!TreeFile::exists (ConfigClientGame::getGroundScene ()) || _strnicmp (ConfigClientGame::getGroundScene (), "scene", 5) == 0, ("%s is not a valid file for loading single-player mode", ConfigClientGame::getGroundScene ()));

					preloadAssets ();

					ms_singlePlayer = true;
					setScene(
						false,
						ConfigClientGame::getGroundScene(), 
						ConfigClientGame::getAvatarSelection(),
						0
					);
				}
				else
#endif
				{
					IGNORE_RETURN(CuiMediatorFactory::activate (CuiMediatorTypes::Splash));
					IGNORE_RETURN(CuiMediatorFactory::activate (CuiMediatorTypes::Backdrop));

					preloadAssets ();
				}
			}
		}
	}
}

//-------------------------------------------------------------------

void Game::remove(void)
{
	DebugFlags::unregisterFlag (ms_garbageCollectNextFrame);
	DebugFlags::unregisterFlag (ms_verifyGuardPatterns);
	DebugFlags::unregisterFlag (ms_verifyFreePatterns);
	DebugFlags::unregisterFlag(ms_useSpaceHudOnly);

	CrashReportInformation::removeDynamicText(ms_bytesAllocatedBuffer);
	CrashReportInformation::removeDynamicText(ms_loopCountBuffer);
	CrashReportInformation::removeDynamicText(ms_upTimeBuffer);

	//-- Cleanup up some library systems that may have some transient data hanging around from the last frame.  Prevents false memory leak reporting.
	PlaybackScriptManager::cleanup();

	NOT_NULL (ms_emitter);
	delete ms_emitter;
	ms_emitter = 0;
}

//-------------------------------------------------------------------

void Game::cleanupScene(void)
{
	if (ms_scene)
	{
		ms_scene->quit();
		_setScene(0);
	}
}

//-------------------------------------------------------------------

void Game::quit()
{
#if PRODUCTION == 0
	Graphics::pixSetMarker(L"Quit");
	PixCounter::disable();
#endif
	ms_done = true;
}

//-------------------------------------------------------------------

void Game::startExitTimer ()
{
	if (ConfigClientGame::getDisableExitTimer ())
		return;

	ms_useExitTimer = true;
	ms_exitTimer.setExpireTime (30.0f);
}

//-------------------------------------------------------------------

bool Game::isOver(void)
{
	return ms_done || !IoWinManager::haveWindow() || Os::isGameOver();
}


//-------------------------------------------------------------------

void Game::run(void)
{
	// -------------------------------------------
	// setup
	install(Game::A_client);
	ms_loops = 0;
	// -------------------------------------------

	// -------------------------------------------
	// THE loop.
	while (!isOver())
	{
		runGameLoopOnce(false, NULL, 0, 0);
	}
	// -------------------------------------------

	// -------------------------------------------
	// cleanup.
	if (ms_nextScene)
	{
		SceneCreator *nextScene = ms_nextScene;
		ms_nextScene=0;
		delete nextScene;
	}
	CutScene::stop(); 
	// -------------------------------------------
}

//-------------------------------------------------------------------

void Game::runGameLoopOnce(bool presentToWindow, HWND hwnd, int width, int height)
{
	bool result;
	float elapsedTime = 0.0f;

#if PRODUCTION == 0
	VTune::beginFrame();
#endif

	{
		NP_PROFILER_AUTO_BLOCK_DEFINE("main loop");

		NP_PROFILER_NAMED_AUTO_BLOCK_DEFINE(profilerMainLoop, "os update");
		result = Os::update();

		// ----------------------------------------------------------------------------------------------------
		// service the video manager - this has to be done several times per frame for Bink to work correctly.
		VideoPlaybackManager::service();
		// ----------------------------------------------------------------------------------------------------

		sprintf(ms_loopCountBuffer, "MainLoop: %d\n", Os::getNumberOfUpdates());

		{
			time_t now;
			IGNORE_RETURN(time(&now));
			sprintf(ms_upTimeBuffer, "UpTime: %d\n", static_cast<int>(now - ms_installTime));
		}

		if (ms_verifyGuardPatterns || ms_verifyFreePatterns)
			MemoryManager::verify(ms_verifyGuardPatterns, ms_verifyFreePatterns);

		elapsedTime = Clock::frameTime ();

		ms_bytesAllocated[0] = ms_bytesAllocated[1];
		ms_bytesAllocated[1] = ms_bytesAllocated[2];
		ms_bytesAllocated[2] = ms_bytesAllocated[3];
		ms_bytesAllocated[3] = ms_bytesAllocated[4];
		ms_bytesAllocated[4] = MemoryManager::getCurrentNumberOfBytesAllocated();
		sprintf(ms_bytesAllocatedBuffer, "BytesAllocated: %lu %lu %lu %lu %lu\n", ms_bytesAllocated[0], ms_bytesAllocated[1], ms_bytesAllocated[2], ms_bytesAllocated[3], ms_bytesAllocated[4]);

#if PRODUCTION == 0
		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "debug");
		DebugMonitor::clearScreen();
		DebugFlags::callReportRoutines();
#endif

		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "garbage collect");
		garbageCollect (false);

		if (ms_useExitTimer && ms_exitTimer.updateZero(elapsedTime))
		{
			CuiLoginManager::disconnectFromCluster();
			Game::cleanupScene();
		}

		if (AsynchronousLoader::isInstalled())
		{
			NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "AsynchronousLoader::processCallbacks");
			AsynchronousLoader::processCallbacks();
		}

		// ----------------------------------------------------------------------------------------------------
		// service the video manager - this has to be done several times per frame for Bink to work correctly.
		VideoPlaybackManager::service();
		// ----------------------------------------------------------------------------------------------------

		if (!result)
		{
			ms_done = true;
			return;
		}

		if (GetActiveWindow() == Os::getWindow())
		{
			NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "GameScheduler update");
			GameScheduler::alter(elapsedTime);
		}

		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "directInput update");
		DirectInput::update();

		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "network update");

		// -------------------------------------------------------
		if (ms_cutScene && ms_endCutScene)
		{
			_endCutScene();
		}
		// -------------------------------------------------------

		if (ms_networkTimer.updateZero(elapsedTime) || isSceneLoading())
		{
			GameNetwork::update();
		}

		// ----------------------------------------------------------------------------------------------------
		// service the video manager - this has to be done several times per frame for Bink to work correctly.
		VideoPlaybackManager::service();
		// ----------------------------------------------------------------------------------------------------

		ClientCommandQueue::update(elapsedTime);
		ObjectAttributeManager::update(elapsedTime);
		DraftSchematicManager::update(elapsedTime);
		ClientWaypointObject::checkWaypoints(elapsedTime);

		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "update");
		CutScene::update();
		IoWinManager::processEvents(elapsedTime);

		// I wish I had a better place to put this....
		if (Graphics::getLastError() != StringId::cms_invalid)
		{
			IGNORE_RETURN(CuiMessageBox::createInfoBox(Graphics::getLastError().localize()));
			Graphics::clearLastError();
		}

		//-- update the UI system
		if (CuiManager::getInstalled())
		{
			NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "ui heartbeats");
			CuiManager::update (elapsedTime);
		}

		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "clienteffect heartbeat");
		ClientEffectManager::sendHeartbeat(elapsedTime);

		// ----------------------------------------------------------------------------------------------------
		// service the video manager - this has to be done several times per frame for Bink to work correctly.
		VideoPlaybackManager::service();
		// ----------------------------------------------------------------------------------------------------

		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "audio alter");
		GroundScene *groundScene = dynamic_cast<GroundScene *>(Game::getScene());
		Audio::alter(elapsedTime, (groundScene ? groundScene->getSoundObject () : NULL));

		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "texture baking");
		TextureRendererManager::alter(elapsedTime);

		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "draw");
		if (!isOver())
		{
			Object::setDisallowObjectDelete (true);

			NP_PROFILER_NAMED_AUTO_BLOCK_DEFINE(profilerDraw, "beginFrame");

			Graphics::update(elapsedTime);
			Graphics::beginScene();

			Appearance::beginNewFrame();

			NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerDraw, "IoWinManager::draw");
			IoWinManager::draw();

			NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerDraw, "Graphics::endFrame");

			Object::setDisallowObjectDelete (false);

			VideoPlaybackManager::performDrawing();

			Graphics::endScene();

			VideoPlaybackManager::performBlitting();

			if (presentToWindow)
				IGNORE_RETURN(Graphics::present(hwnd, width, height));
			else
				IGNORE_RETURN(Graphics::present());
		}

#if PRODUCTION == 0
		// this is here to be consistent with the behavior when PIX is running, since the polling will happen when Graphics::present() is called
		PixCounter::update();
#endif

		//-- time out appearance templates
		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "AppearanceTemplateList");
		AppearanceTemplateList::update(elapsedTime);

		alterNetworkBandwidthCalculation(elapsedTime);

		// ----------------------------------------------------------------------------------------------------
		// service the video manager - this has to be done several times per frame for Bink to work correctly.
		VideoPlaybackManager::service();
		// ----------------------------------------------------------------------------------------------------

		NP_PROFILER_NAMED_AUTO_BLOCK_TRANSFER(profilerMainLoop, "frame limit");
		Clock::limitFrameRate();
	}

	++ms_loops;
	ms_elapsedTime += elapsedTime;
}

//-------------------------------------------------------------------

std::string Game::getSceneIdFromTerrainFilename(const char *terrainFilename)
{
	std::string result = terrainFilename;

	const size_t dotpos   = result.rfind ('.');
	const size_t slashpos = result.rfind ('/');

	if (dotpos == std::string::npos) //lint !e650 !e737 // stl bug
	{
		if (slashpos != std::string::npos) //lint !e650 !e737 // stl bug
			result = result.substr (slashpos + 1);
	}
	else
	{
		if (slashpos != std::string::npos) //lint !e650 !e737 // stl bug
			result = result.substr (slashpos + 1, (dotpos - slashpos) - 1);
		else
			result = result.substr (0, dotpos);
	}

	return calculateNonInstanceSceneId(result);
}

//-------------------------------------------------------------------

void Game::setScene(
	const bool  immediately,
	const char* terrainFilename, 
	const char* playerFilename, 
	CreatureObject* customizedPlayer
)
{
	SinglePlayerSceneCreator *sc = new SinglePlayerSceneCreator(terrainFilename, playerFilename, customizedPlayer);
	_setScene(*sc, immediately);
}

//-------------------------------------------------------------------

void Game::setScene(
	const bool  immediately,
	const char* terrainFilename, 
	const NetworkId& playerNetworkId, 
	const char* const templateName, 
	const Vector& startPosition, 
	const float startYaw, 
	float timeInSeconds, 
	bool disableSnapshot
)
{
	MultiPlayerSceneCreator *sc = new MultiPlayerSceneCreator(terrainFilename, playerNetworkId, templateName, startPosition, startYaw, timeInSeconds, disableSnapshot);
	_setScene(*sc, immediately);
}

//-------------------------------------------------------------------

void Game::_setScene(SceneCreator &sc, bool immediately)
{
	if (ms_nextScene)
	{
		Game::endCutScene(true);
		delete ms_nextScene;
		ms_nextScene=0;
	}

	// --------------------------------------------

	ms_nextScene = &sc;

	// --------------------------------------------

	char cutScene[2048];
	cutScene[0]=0;

	if (!immediately)
	{
		_getCutSceneFromTerrainFilename(cutScene, sc.m_terrainFilename, sc.m_startPosition);
	}

	// --------------------------------------------

	bool deferScene=false;

	if (CutScene::isRunning())
	{
		deferScene=true;
	}
	else if (cutScene && *cutScene)
	{
		deferScene = _startCutScene(cutScene, true);
	}

	if (deferScene)
	{
		// a cut-scene is playing.
		ms_nextScene->beginDeferredCreation();

		CuiLoadingManager::setFullscreenLoadingEnabled(true);
	}
	else
	{
		_startScene();
	}
}

//-------------------------------------------------------------------

void Game::_startScene()
{
	if (ms_nextScene)
	{
		SceneCreator *sc = ms_nextScene;
		ms_nextScene=0;
		GroundScene *newScene = sc->create();
		Game::_setScene(newScene);
		sc->endDeferredCreation();
		delete sc;
	}
}

//-------------------------------------------------------------------

void Game::_setScene(Scene* newScene)
{
	if (ms_scene == newScene)
		return;

	ms_useExitTimer = false;

	if (ms_scene)
	{
		Unicode::String dummy;
		IGNORE_RETURN(getPlayerPath (ms_lastPlayerLoginId, ms_lastPlayerCluster, dummy, ms_lastPlayerId));
	}

	ms_scene = newScene;

	if (!newScene)
		ClientCommandQueue::clear ();

	const TerrainObject * const terrain = TerrainObject::getInstance ();

	ms_sceneId.clear ();

	if (ms_scene)
	{
		if (terrain)
		{
			const Appearance * const app = terrain->getAppearance ();

			if (app)
			{
				const AppearanceTemplate * const appTemplate = app->getAppearanceTemplate ();

				if (appTemplate)
				{
					const char * const name = appTemplate->getName ();

					if (name)
						ms_sceneId = name;
					else
						WARNING (true, ("Game::setScene with null terrain appearance template name"));
				}
				else
					WARNING (true, ("Game::setScene with null terrain appearance template"));
			}
			else
				WARNING (true, ("Game::setScene with null terrain appearance"));
		}
		else
			WARNING (true, ("Game::setScene with null terrain"));

		ms_sceneId = getSceneIdFromTerrainFilename(ms_sceneId.c_str());

		WARNING (ms_sceneId.empty (), ("Game::setScene with empty sceneId"));
	}

#if PRODUCTION == 0
	// update the sceneId in the FileManifest (also done more specifically in groundscene to get buildout info)
	// however, we get updates to a null string here to tell us that we are transitioning to a new scene
	if (FileManifest::shouldUpdateManifest())
	{
		FileManifest::setSceneId(ms_sceneId.c_str());
	}
#endif
	
	ms_nonInstanceSceneId = calculateNonInstanceSceneId(ms_sceneId);

	//@todo: need a better way to find these
	ms_isSpace = isSpaceSceneName(ms_sceneId);

	NebulaManagerClient::loadScene(ms_sceneId);
	ClientCommandTable::load();

	Game::setBrightness(Game::getBrightness());
	Game::setContrast(Game::getContrast());
	Game::setGamma(Game::getGamma());

	ShadowManager::setAllowed(!ms_isSpace);

	CollisionWorld::handleSceneChange(ms_sceneId);

	//@todo We don't actually need to do this on even scene transition, 
	//just when the player changes.  But no easy way to know this. --ARH
	CuiSkillManager::clearCachedSkillTemplate();

	emitSceneChange();
}

//-----------------------------------------------------------------

bool Game::playProfessionMovie( const char * professionName )
{
	char tmp[512];
	snprintf(tmp, sizeof(tmp)-1, s_professionMovieFormatString, professionName );
	return playCutScene(tmp, false);
}

//-----------------------------------------------------------------

bool Game::playCutScene(const char *fileName, bool doReplayCheck)
{
	if (!fileName || !*fileName)
	{
		return false;
	}
	return Game::_startCutScene(fileName, doReplayCheck);
}

//-----------------------------------------------------------------

void Game::endCutScene(bool immediate)
{
	if (ms_cutScene)
	{
		ms_endCutScene = true;
		CutScene::stop();
		if (immediate)
		{
			_endCutScene();
		}
	}
}

//-----------------------------------------------------------------

void Game::skipCutScene()
{
	if (ms_cutScene && !CutScene::isRunningPremier())
	{
		endCutScene();
	}
}

//-----------------------------------------------------------------

bool Game::isPlayingCutScene()
{
	return ms_cutScene;
}

//-----------------------------------------------------------------

bool Game::_startCutScene(const char *fileName, bool doReplayCheck)
{
	if (ConfigClientGame::getDisableCutScenes())
	{
		return false;
	}

	if (ms_cutScene)
	{
		return false;
	}

	if (!Game::isClient())
	{
		return false;
	}

	//-----------------------------------------------------------
	bool alreadySeen=false;
	std::string playerBaseFileName;
	if (doReplayCheck)
	{
		std::string loginId;
		std::string clusterName;
		NetworkId id;

		Game::getLastPlayerInfo(loginId, clusterName, id);

		// - - - - - - - - - - - - - - - - - - - - - - 
		loginId = GameNetwork::getUserName ();
		if (loginId.empty ())
			loginId = "default_temporary";
		// - - - - - - - - - - - - - - - - - - - - - - 

		// - - - - - - - - - - - - - - - - - - - - - - 
		if (Game::getSinglePlayer())
		{
			clusterName = "single_player";
		}
		else
		{
			std::string networkCluster = GameNetwork::getCentralServerName();
			if (!networkCluster.empty())
			{
				clusterName = networkCluster;
			}
		}
		// - - - - - - - - - - - - - - - - - - - - - - 

		// - - - - - - - - - - - - - - - - - - - - - - 
		const ClientObject * const clientPlayer = Game::getClientPlayer();
		if (clientPlayer)
		{
			id = clientPlayer->getNetworkId ();
		}
		else if (ms_nextScene)
		{
			id = ms_nextScene->getPlayerNetworkId();
		}
		// - - - - - - - - - - - - - - - - - - - - - - 
		
		static const std::string prefix = "profiles/";
		static const std::string slash = "/";

		playerBaseFileName = prefix + loginId + slash + clusterName + slash + id.getValueString();

		alreadySeen = CutScene::getCutSceneSeen(fileName, playerBaseFileName.c_str());
	}

	//-----------------------------------------------------------

	if (  alreadySeen
		&& !ConfigClientGame::getReplayCutScenes()
		)
	{
		return false;
	}

	//-----------------------------------------------------------

	const bool isPremier = doReplayCheck && !alreadySeen;

	if (!CutScene::start(fileName, isPremier))
	{
		return false;
	}

	ms_endCutScene=false;
	ms_cutScene=true;

	//-----------------------------------------------------------

	GroundScene *groundScene = dynamic_cast<GroundScene *>(getScene());
	if (groundScene)
	{
		groundScene->setNoDraw(true);
	}

	//-----------------------------------------------------------
	if (  doReplayCheck
		&& !alreadySeen
		&& !playerBaseFileName.empty()
		)
	{
		CutScene::setCutSceneSeen(fileName, playerBaseFileName.c_str(), true);
	}
	//-----------------------------------------------------------

	return true;
}

//-----------------------------------------------------------------

void Game::_endCutScene()
{
	if (ms_cutScene)
	{
		_startScene();
		CutScene::stop();
		ms_cutScene=false;
		ms_endCutScene=false;

		// ------------------------------------

		GroundScene *groundScene = dynamic_cast<GroundScene *>(getScene());
		if (groundScene)
		{
			groundScene->setNoDraw(false);
		}
	}
}

//-------------------------------------------------------------------

void Game::_getCutSceneFromTerrainFilename(char *o_name, const char *terrainFileName, const Vector &startPosition)
{
	o_name[0]=0;

	std::string sceneId = Game::getSceneIdFromTerrainFilename(terrainFileName);
	const BuildoutArea *buildoutArea = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(sceneId.c_str(), startPosition.x, startPosition.z, false);
	const char *cutScene=0;
	if (buildoutArea)
	{
		cutScene=CutScene::lookupCutScene(buildoutArea->areaName.c_str());
		if (cutScene)
		{
			strcpy(o_name, cutScene);
		}
	}

	if (!o_name[0])
	{
		cutScene=CutScene::lookupCutScene(sceneId.c_str());
		if (cutScene)
		{
			strcpy(o_name, cutScene);
		}
	}

	/*
	if (!o_name[0])
	{
		const char *buildoutName = (buildoutArea) ? buildoutArea->areaName.c_str() : "(no buildout)";
		WARNING(true, ("Failed to find cut-scene for %s / %s.\n", sceneId.c_str(), buildoutName));
	}
	*/
}

//-----------------------------------------------------------------

void Game::startChatInput (const Unicode::String & str)
{
	Transceivers::chatStartInput.emitMessage (str);
}

//----------------------------------------------------------------------

/**
 * The will pre-pend "[DEVL]" onto strings passed to it.  It is meant to be
 * used as a fast, temporary way to see features or for actual debug
 * messages that the player won't see.
 * Use CuiChatRoomManager::sendPrelocalizedChat(const Unicode::String &)
 * as the replacement once a StringId exists.  Do not hard-code a string
 * using sendPrelocalizedChat.
 */
void Game::debugPrintUi (const char * str)
{
	NOT_NULL (str);
	debugPrintUi (Unicode::narrowToWide (str));
}

//----------------------------------------------------------------------

/**
 * The will pre-pend "[DEVL]" onto strings passed to it.  It is meant to be
 * used as a fast, temporary way to see features or for actual debug
 * messages that the player won't see.
 * Use CuiChatRoomManager::sendPrelocalizedChat(const Unicode::String &)
 * as the replacement once a StringId exists.  Do not hard-code a string
 * using sendPrelocalizedChat.
 */
void Game::debugPrintUi (const Unicode::String & str)
{
	Transceivers::debugPrintUi.emitMessage (Unicode::narrowToWide("[DEVL]") + str);
}

//----------------------------------------------------------------------

Object * Game::getPlayer ()
{
	NetworkScene * const ns = dynamic_cast<NetworkScene *> (ms_scene);
	return ns ? ns->getPlayer () : 0;
}

//----------------------------------------------------------------------

NetworkId Game::getPlayerNetworkId()
{
	Object const * const player = Game::getPlayer();
	return player ? player->getNetworkId() : NetworkId::cms_invalid;
}

//----------------------------------------------------------------------

const Object * Game::getConstPlayer ()
{
	return getPlayer();
}

//----------------------------------------------------------------------

PlayerObject * Game::getPlayerObject ()
{
	PlayerObject *result = NULL;
	Object *object = getPlayer();

	if (object)
	{
		ClientObject * const clientObject = object->asClientObject();
		CreatureObject * const creatureObject = clientObject ? clientObject->asCreatureObject() : NULL;

		if (creatureObject)
		{
			result = creatureObject->getPlayerObject();
		}
	}

	return result;
}

//----------------------------------------------------------------------

const PlayerObject *Game::getConstPlayerObject ()
{
	return getPlayerObject();
}

//----------------------------------------------------------------------

ClientObject * Game::getClientPlayer ()
{
	NetworkScene * const ns = safe_cast<NetworkScene *> (ms_scene);
	return ns ? safe_cast<ClientObject *>(ns->getPlayer ()) : 0;
}

//----------------------------------------------------------------------

CreatureObject * Game::getPlayerCreature ()
{
	NetworkScene * const ns = safe_cast<NetworkScene *> (ms_scene);
	if (ns)
	{
		Object * const o = ns->getPlayer ();
		if (o && o->getObjectType () == SharedCreatureObjectTemplate::SharedCreatureObjectTemplate_tag)
		{
			ClientObject* co = o->asClientObject();
			if(co)
			{
				return co->asCreatureObject();
			}
		}
	}

	return 0;
}

//----------------------------------------------------------------------

ShipObject * Game::getPlayerPilotedShip ()
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(player)
	{
		return player->getPilotedShip();
	}
	return NULL;
}

//----------------------------------------------------------------------

ShipObject const * Game::getConstPlayerPilotedShip()
{
	return getPlayerPilotedShip();
}

//----------------------------------------------------------------------

ShipObject * Game::getPlayerContainingShip()
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(player)
	{
		return ShipObject::getContainingShip(*player);
	}
	return NULL;
}

//----------------------------------------------------------------------

bool Game::isPlayerSquelched()
{
	NetworkScene const * const ns = dynamic_cast<NetworkScene *>(ms_scene);
	Object const * const object = ns ? ns->getPlayer() : NULL;
	ClientObject const * const clientObject = object ? object->asClientObject() : NULL;
	CreatureObject const * const creatureObject = clientObject ? clientObject->asCreatureObject() : NULL;
	PlayerObject const * const playerObject = creatureObject ? creatureObject->getPlayerObject() : NULL;

	return (playerObject && (0 != playerObject->getSecondsUntilUnsquelched()));
}

//----------------------------------------------------------------------

bool Game::isPlayerPermanentlySquelched()
{
	NetworkScene const * const ns = dynamic_cast<NetworkScene *>(ms_scene);
	Object const * const object = ns ? ns->getPlayer() : NULL;
	ClientObject const * const clientObject = object ? object->asClientObject() : NULL;
	CreatureObject const * const creatureObject = clientObject ? clientObject->asCreatureObject() : NULL;
	PlayerObject const * const playerObject = creatureObject ? creatureObject->getPlayerObject() : NULL;

	return (playerObject && (playerObject->getSecondsUntilUnsquelched() < 0));
}

//----------------------------------------------------------------------

bool Game::playerIsLastPlayer ()
{
	std::string     loginId;
	std::string     clusterName;
	NetworkId       id;
	Unicode::String name;

	if (getPlayerPath (loginId, clusterName, name, id))
	{
		return (id == ms_lastPlayerId && clusterName == ms_lastPlayerCluster && loginId == ms_lastPlayerLoginId);
	}

	return false;
}

//----------------------------------------------------------------------

void Game::getLastPlayerInfo (std::string & loginId, std::string & clusterName, NetworkId & id)
{
	id          = ms_lastPlayerId;
	loginId     = ms_lastPlayerLoginId;
	clusterName = ms_lastPlayerCluster;
}

//----------------------------------------------------------------------

bool Game::getPlayerPath (std::string & loginId, std::string & clusterName, Unicode::String & playerName, NetworkId & id)
{
	const ClientObject * const clientObject = getClientPlayer ();

	if (clientObject)
	{
		id          = clientObject->getNetworkId ();
		loginId     = GameNetwork::getUserName ();

		if (loginId.empty ())
			loginId = "default_temporary";

		if (Game::getSinglePlayer ())
		{
			if(Game::isParticleEditor() || Game::isAnimationEditor())
			{
				if (ConfigFile::getKeyBool ("ClientTools", "loadHud", false))
				{
					clusterName = "single_player_client_tools_hud";
				}
				else
				{
					clusterName = "single_player_client_tools_nohud";
				}
			}
			else
			{
				clusterName = "single_player";
			}
		}
		else
		{
			clusterName = GameNetwork::getCentralServerName ();
			if (clusterName.empty ())
				clusterName = ms_lastPlayerCluster;
		}

		playerName  = clientObject->getLocalizedName ();
		return true;
	}

	return false;
}

//-----------------------------------------------------------------

MessageQueue * Game::getGameMessageQueue ()
{
	GroundScene * const gs = dynamic_cast<GroundScene *> (ms_scene);
	return gs ? gs->getCurrentMessageQueue () : 0;
}

//----------------------------------------------------------------------

InputMap * Game::getGameInputMap ()
{
	GroundScene * const gs = dynamic_cast<GroundScene *> (ms_scene);
	return gs ? gs->getInputMap () : 0;
}

//-----------------------------------------------------------------

Camera * Game::getCamera ()
{
	GroundScene * const gs = dynamic_cast<GroundScene *> (ms_scene);
	return gs ? gs->getCurrentCamera () : 0;
}

//----------------------------------------------------------------------

Camera const * Game::getConstCamera()
{
	GroundScene const * const gs = dynamic_cast<GroundScene *> (ms_scene);
	return gs ? gs->getCurrentCamera () : 0;
}

//----------------------------------------------------------------------

bool Game::isViewFirstPerson ()
{
	GroundScene * const gs = dynamic_cast<GroundScene *> (ms_scene);
	return (gs != 0) && gs->isFirstPerson ();
}

//----------------------------------------------------------------------

bool Game::isViewFreeCamera()
{
	GroundScene * const gs = dynamic_cast<GroundScene *> (ms_scene);
	return (gs != 0) && (gs->getCurrentView() == static_cast<int>(GroundScene::CI_free));
}

//----------------------------------------------------------------------

bool Game::isSceneLoading ()
{
	GroundScene * const gs = dynamic_cast<GroundScene *> (ms_scene);
	return (gs != 0) && gs->isLoading ();
}

//----------------------------------------------------------------------

void Game::setProfanityFiltered (bool profanityFiltered)
{
	ms_profanityFiltered = profanityFiltered;
}

//----------------------------------------------------------------------

bool Game::isProfanityFiltered()
{
	return ms_profanityFiltered;
}

//----------------------------------------------------------------------

#ifdef _DEBUG

void verifyUpdateRanges (const char* const filename)
{
	FILE* const infile = fopen (filename, "rt");
	if (!infile)
	{
		DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: response file %s could not be opened\n", filename));
		return;
	}

	char sharedObjectTemplateName [1024];
	while (fscanf (infile, "%s", sharedObjectTemplateName) != EOF)
	{
		//-- does the name contain test?
		if (strstr (sharedObjectTemplateName, "test") != 0)
		{
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: skipping test object template %s\n", sharedObjectTemplateName));
			continue;
		}

		if (strstr (sharedObjectTemplateName, "e3_") != 0)
		{
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: skipping e3 object template %s\n", sharedObjectTemplateName));
			continue;
		}

		//-- get the object template
		const ObjectTemplate* const objectTemplate = ObjectTemplateList::fetch (sharedObjectTemplateName);
		if (!objectTemplate)
		{
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: %s is not a valid object template\n", sharedObjectTemplateName));
			continue;
		}

		//-- make sure it's a shared template
		const SharedObjectTemplate* const sharedObjectTemplate = safe_cast<const SharedObjectTemplate*> (ObjectTemplateList::fetch (sharedObjectTemplateName));
		if (!sharedObjectTemplate)
		{
			objectTemplate->releaseReference ();
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: %s is not a valid shared object template\n", sharedObjectTemplateName));
			continue;
		}

		//-- find the appearance template name
		char appearanceTemplateName [1024];
		{
			const std::string& pobName = sharedObjectTemplate->getPortalLayoutFilename ();
			if (pobName.empty ())
				strcpy (appearanceTemplateName, sharedObjectTemplate->getAppearanceFilename ().c_str ());
			else
			{
				//-- open the pob
				const CrcLowerString pobCrcName (pobName.c_str ());
				if (!TreeFile::exists (pobCrcName.getString ()))
				{
					objectTemplate->releaseReference ();
					DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: shared object template [%s] has missing pob [%s]\n", sharedObjectTemplateName, pobCrcName.getString ()));
					continue;
				}

				const PortalPropertyTemplate* const portalPropertyTemplate = PortalPropertyTemplateList::fetch (pobCrcName);
				NOT_NULL (portalPropertyTemplate);

				//--
				strcpy (appearanceTemplateName, portalPropertyTemplate->getExteriorAppearanceName ());
				portalPropertyTemplate->release ();
			}
		}

		//-- make sure the appearance template is valid
		if (!*appearanceTemplateName)
			continue;

		if (!TreeFile::exists (appearanceTemplateName))
		{
			objectTemplate->releaseReference ();
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: shared object template [%s] has missing apppearance template [%s]\n", sharedObjectTemplateName, appearanceTemplateName));
			continue;
		}

		if (strstr (appearanceTemplateName, ".sat") != 0)
		{
			objectTemplate->releaseReference ();
			DEBUG_REPORT_LOG (true, ("verifyUpdateRanges: skipping object template %s because it has a skeletal appearance\n", sharedObjectTemplateName));
			continue;
		}

		//-- create the appearance
		const AppearanceTemplate* const appearanceTemplate = AppearanceTemplateList::fetch (appearanceTemplateName);
		Appearance* const appearance = appearanceTemplate->createAppearance ();

		//-- get the radius
		const float sphereRadius = appearance->getSphere ().getRadius ();

		//-- determine the maximum render radius
		float maximumRenderRadius = 5000.0f;
		const DetailAppearanceTemplate* const detailAppearanceTemplate = dynamic_cast<const DetailAppearanceTemplate*> (appearanceTemplate);
		if (detailAppearanceTemplate)
			maximumRenderRadius = detailAppearanceTemplate->getFarDistance (0);

		//-- delete the appearances
		DEBUG_REPORT_LOG (true, ("OK:\t%s\t%s\t%1.1f %1.1f\n", sharedObjectTemplateName, appearanceTemplateName, sphereRadius, maximumRenderRadius));

		AppearanceTemplateList::release (appearanceTemplate);
		delete appearance;
		objectTemplate->releaseReference ();
	}
}

#include <deque>

static std::string                            currentLintedAsset;
static std::deque<AppearanceTemplate const *> dataLintAppearanceTemplateDeque;
static std::deque<ObjectTemplate const *>     dataLintObjectTemplateDeque;
static std::deque<ShaderTemplate const *>     dataLintShaderTemplateDeque;

//-----------------------------------------------------------------------------
static int ExceptionHandler(LPEXCEPTION_POINTERS exceptionPointers)
{
	UNREF(exceptionPointers);

	static bool entered = false;

	// make the routine safe from re-entrance

	if (entered)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	entered = true;

	// tell the Os not to abort so we can rethrow the exception

	Os::returnFromAbort();

	// Let the ExitChain do its job

	DataLint::pushAsset(currentLintedAsset.c_str());
	FATAL(true, ("ExceptionHandler invoked - A crash just occured. The asset is bad or it is constructed improperly with DataLint."));
	DataLint::popAsset(); //lint !e527 //unreachable

	// rethrow the exception so that the debugger can catch it

	return EXCEPTION_CONTINUE_SEARCH;  //lint !e527 // Unreachable
}

//-----------------------------------------------------------------------------
static void lintType(char const *filePath, DataLint::AssetType const assetType)
{
	DataLint::setCurrentAssetType(assetType);

	static int assetNumber = 0;
	DEBUG_REPORT_LOG (true, ("%5i Linting file %s... ", ++assetNumber, filePath));

	try
	{
		switch (assetType)
		{
			case DataLint::AT_appearance:
				{
					const AppearanceTemplate* const appearanceTemplate = AppearanceTemplateList::fetch(filePath);
					if (appearanceTemplate)
					{
						Object* const     object     = new Object ();
						object->setAppearance (appearanceTemplate->createAppearance ());
						IGNORE_RETURN(object->alter (0.0f));
						object->conclude ();
						delete object;

						dataLintAppearanceTemplateDeque.push_back(appearanceTemplate);
					}
				}
				break;
			case DataLint::AT_arrangementDescriptor:
				{
					ArrangementDescriptor const *arrangementDescriptor = ArrangementDescriptorList::fetch(filePath);
					if (arrangementDescriptor)
					{
						arrangementDescriptor->release();
					}
				}
				break;
			case DataLint::AT_localizedStringTable:
				{
					// Special case push/pop here because LocalizedStringTable does not include sharedDebug.

					DataLint::pushAsset(filePath);

					TreeFile::TreeFileFactory treeFileFactory;
					LocalizedStringTable *localizedStringTable = LocalizedStringTable::load(treeFileFactory, filePath);
					delete localizedStringTable;

					DataLint::popAsset();
				}
				break;
			case DataLint::AT_objectTemplate:
				{
					if (strstr(filePath, "construction_contract") != NULL)
					{
					}
					const ObjectTemplate *objectTemplate = ObjectTemplateList::fetch(filePath);
					if (objectTemplate)
					{
						objectTemplate->testValues();

						// if file has the directory "base" in it's path, don't
						// create an object
						if (strstr(filePath, "/base/") == NULL)
						{
							Object* const object = objectTemplate->createObject ();
							if (object != NULL)
							{
								if (dynamic_cast<ClientObject *> (object))
									static_cast<ClientObject *> (object)->endBaselines (); //lint !e1774 //downcast
								IGNORE_RETURN(object->alter (0.01f));
								object->conclude ();
								delete object;
							}
						}

						dataLintObjectTemplateDeque.push_back(objectTemplate);
					}
				}
				break;
			case DataLint::AT_portalProperty:
				{
					Object dummyObject;
					dummyObject.setNetworkId(NetworkId(NetworkId::NetworkIdType(1)));
					PortalProperty *portalProperty = new PortalProperty(dummyObject, filePath);

					DataLint::pushAsset(filePath);

						dummyObject.addProperty(*portalProperty);
						portalProperty->createAppearance();
						portalProperty->clientSinglePlayerInitializeFirstTimeObject();

					DataLint::popAsset();
				} //lint !e429 //portalProperty custodial
				break;
			case DataLint::AT_shaderTemplate:
				{
					ShaderTemplate const *shaderTemplate = ShaderTemplateList::fetch(filePath);
					if (shaderTemplate)
					{
						const Shader* const shader = shaderTemplate->fetchShader ();
						IGNORE_RETURN(shader->alter (0.0f));
						shader->release ();

						dataLintShaderTemplateDeque.push_back(shaderTemplate);
					}
				}
				break;
			case DataLint::AT_skyBox:
				{
				}
				break;
			case DataLint::AT_slotDescriptor:
				{
					SlotDescriptor const *slotDescriptor = SlotDescriptorList::fetch(filePath);
					if (slotDescriptor)
					{
						slotDescriptor->release();
					}
				}
				break;
			case DataLint::AT_soundTemplate:
				{
					SoundTemplate const *soundTemplate = SoundTemplateList::fetch(filePath);
					if (soundTemplate)
					{
						Sound2* const sound = soundTemplate->createSound ();
						delete sound;

						SoundTemplateList::release(soundTemplate);
					}
				}
				break;
			case DataLint::AT_terrain:
				{
					const AppearanceTemplate* const appearanceTemplate = AppearanceTemplateList::fetch(filePath);
					if (appearanceTemplate)
					{
						Object* const object = new Object ();
						object->setAppearance (appearanceTemplate->createAppearance ());
						delete object;

						AppearanceTemplateList::release (appearanceTemplate);
					}
				}
				break;
			case DataLint::AT_texture:
				{
					Texture const *texture = TextureList::fetch(filePath);
					if (texture)
					{
						texture->release();
					}
				}
				break;
			case DataLint::AT_textureRendererTemplate:
				{
					TextureRendererTemplate const *textureRenderer = TextureRendererList::fetch(filePath);
					if (textureRenderer)
					{
						textureRenderer->release();
					}
				}
				break;
			case DataLint::AT_unSupported:
			case DataLint::AT_count:
			case DataLint::AT_invalid:
				{
				}
				break;
		}

		DEBUG_REPORT_LOG (true, ("ok\n"));
	}
	catch (FatalException const &e)
	{
		DEBUG_REPORT_LOG (true, ("failed\n"));

		DataLint::logWarning(e.getMessage());
	}
	catch (...)
	{
		DEBUG_REPORT_LOG (true, ("failed: unhandled exception\n"));
		DataLint::logWarning("failed: unhandled exception");
	}
}

//-----------------------------------------------------------------------------
static void makeMicrosoftHappyAboutObjectUnWinding(char const *filePath, DataLint::AssetType const assetType)
{
	__try
	{
		lintType(filePath, assetType);
	}
	__except (ExceptionHandler(GetExceptionInformation())) //lint !e1924 //msdev bug
	{
	}
}

//-----------------------------------------------------------------------------

static void clearDeques(uint goalSize)
{
	while (dataLintAppearanceTemplateDeque.size() > goalSize)
	{
		AppearanceTemplate const * at = dataLintAppearanceTemplateDeque.front();
		dataLintAppearanceTemplateDeque.pop_front();
		AppearanceTemplateList::release(at);
	}

	while (dataLintObjectTemplateDeque.size() > goalSize)
	{
		ObjectTemplate const * ot = dataLintObjectTemplateDeque.front();
		dataLintObjectTemplateDeque.pop_front();
		ot->releaseReference();
	}

	while (dataLintShaderTemplateDeque.size() > goalSize)
	{
		ShaderTemplate const * st = dataLintShaderTemplateDeque.front();
		dataLintShaderTemplateDeque.pop_front();
		st->release();
	}
}

//-----------------------------------------------------------------------------
static void lint(char const *dataTypeString, DataLint::AssetType const assetType)
{
	DataLint::StringPairList stringPairList(DataLint::getList(assetType));
	DataLint::StringPairList::const_iterator dataLintStringListIter = stringPairList.begin();

	DEBUG_REPORT_LOG_PRINT(1, ("Linting %d %s assets\n", stringPairList.size(), dataTypeString));

	int const cacheSize = ConfigFile::getKeyInt("ClientGame/DataLint", "cacheSize", 10);

	int count = 0;
	for (; dataLintStringListIter != stringPairList.end(); ++dataLintStringListIter)
	{
		{
			if ((count++) % 10 == 0)
			{
				//-- see if a file exists to abort

				FILE *file = fopen("stoplint.dat", "rt");

				if (file)
				{
					fclose(file);
					DEBUG_REPORT_LOG_PRINT(1, ("Stopping all DataLint processing for %s.\n", dataTypeString));
					DataLint::logWarning("Forced DataLint stop. Delete \"stoplint.dat\" to DataLint all the assets.");
					break;
				}
			}
		}

		char const *filePath = dataLintStringListIter->first.c_str();
		currentLintedAsset = dataLintStringListIter->second.c_str();

		DataLint::pushAsset(filePath);
		makeMicrosoftHappyAboutObjectUnWinding(filePath, assetType);
		DataLint::popAsset();

		DataLint::clearAssetStack();

		clearDeques(static_cast<uint32>(cacheSize));
	}
}

//-----------------------------------------------------------------------------
void runDataLint(char const *responsePath)
{
	PerformanceTimer performanceTimer;
	performanceTimer.start();

	AsynchronousLoader::disable();

	FILE *fp = fopen(responsePath, "r");

	if (fp)
	{
		// Verify this is a valid resonse file

		char text[4096];
		IGNORE_RETURN(fgets(text, sizeof(text), fp));

		if (strstr(text, "Valid DataLint Rsp") == NULL)
		{
			DEBUG_REPORT_LOG_PRINT(1, ("DataLint: Invalid Rsp file: %s\n", responsePath));

#ifdef WIN32
			sprintf(text, "Invalid Rsp file specified: %s. Make sure to run DataLintRspBuilder before running DataLint.", responsePath);
			IGNORE_RETURN(MessageBox(NULL, text, "DataLint Error!", MB_OK | MB_ICONERROR));
#endif // WIN32
		}
		else
		{
			DataLint::install();

			typedef std::vector<std::string> StringList;
			StringList stringList;
			stringList.reserve(32768);

			DEBUG_REPORT_LOG_PRINT(1, ("Loading assets to lint from: %s\n", responsePath));

			while (fgets(text, sizeof(text), fp))
			{
				// Remove the newline character

				char *newLineTest = strchr(text, '\n');

				if (newLineTest)
				{
					*newLineTest = '\0';
				}

				// Add the files to the master file list

				stringList.push_back(text);
			}

			fclose(fp);

			// Add files to data lint

			DEBUG_REPORT_LOG_PRINT(1, ("Adding %d assets to DataLint to be categorized\n", stringList.size()));

			StringList::iterator stringListIter = stringList.begin();

			for (; stringListIter != stringList.end(); ++stringListIter)
			{
				DataLint::addFilePath((*stringListIter).c_str());
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "appearance", true))
			{
				lint("Appearance",                DataLint::AT_appearance);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "arrangementDescriptor", true))
			{
				lint("Arrangement Descriptor",    DataLint::AT_arrangementDescriptor);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "localizedStringTable", true))
			{
				lint("Localized String Table",    DataLint::AT_localizedStringTable);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "objectTemplate", true))
			{
				lint("Objects Template",          DataLint::AT_objectTemplate);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "portalProperty", true))
			{
				lint("Portal Property",           DataLint::AT_portalProperty);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "shaderTemplate", true))
			{
				lint("Shader Template",           DataLint::AT_shaderTemplate);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "skyBox", true))
			{
				lint("Sky Box",                   DataLint::AT_skyBox);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "slotDescriptor", true))
			{
				lint("Slot Descriptor",           DataLint::AT_slotDescriptor);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "soundTemplate", true))
			{
				lint("Sound Template",            DataLint::AT_soundTemplate);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool ("ClientGame/DataLint", "terrain", true))
			{
				const bool useMultiThreadedTerrainGeneration = ClientProceduralTerrainAppearance::getUseMultiThreadedTerrainGeneration ();
				ClientProceduralTerrainAppearance::setUseMultiThreadedTerrainGeneration (false);
				lint ("Terrain", DataLint::AT_terrain);
				ClientProceduralTerrainAppearance::setUseMultiThreadedTerrainGeneration (useMultiThreadedTerrainGeneration);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "texture", true))
			{
				lint("Texture",                   DataLint::AT_texture);
				clearDeques(0);
			}

			if (ConfigFile::getKeyBool("ClientGame/DataLint", "textureRendererTemplate", true))
			{
				lint("Texture Renderer Template", DataLint::AT_textureRendererTemplate);
				clearDeques(0);
			}

			DataLint::report();
		}
	}
	else
	{
		DEBUG_REPORT_LOG_PRINT(1, ("Bad response path specified: %s", responsePath));

#ifdef WIN32
		char text[4096];
		sprintf(text, "Bad response file specified: %s. Make sure to run DataLintRspBuilder before running DataLint.", responsePath);
		IGNORE_RETURN(MessageBox(NULL, text, "DataLint Error!", MB_OK | MB_ICONERROR));
#endif // WIN32
	}

	// Dump the time required to data lint

	performanceTimer.stop();
	float const dataLintTime = performanceTimer.getElapsedTime();
	int const seconds = static_cast<int>(dataLintTime) % 60;
	int const minutes = (static_cast<int>(dataLintTime) - seconds) / 60;
	DEBUG_REPORT_LOG_PRINT(1, ("DataLint took: %dm %ds\n", minutes, seconds));
}
#endif // _DEBUG

//-------------------------------------------------------------------

void Game::setGameOptionChangedCallback(GameOptionChangedCallback const callback)
{
	ms_gameOptionChangedCallback = callback;
}

//-------------------------------------------------------------------

void Game::gameOptionChanged()
{
	if (ms_gameOptionChangedCallback != NULL)
	{
		ms_gameOptionChangedCallback();
	}
}

//-------------------------------------------------------------------

int Game::getReceivedUncompressedBytesPerSecond()
{
	return s_receivedUncompressedBytesPerSecond;
}

//-------------------------------------------------------------------

int Game::getReceivedHistoryUncompressedBytesPerSecond()
{
	return s_receivedHistoryUncompressedBytesPerSecond;
}

//-------------------------------------------------------------------

int Game::getReceivedCompressedBytesPerSecond()
{
	return s_receivedCompressedBytesPerSecond;
}

//-------------------------------------------------------------------

int Game::getReceivedHistoryCompressedBytesPerSecond()
{
	return s_receivedHistoryCompressedBytesPerSecond;
}

//-------------------------------------------------------------------

int Game::getSentUncompressedBytesPerSecond()
{
	return s_sentUncompressedBytesPerSecond;
}

//-------------------------------------------------------------------

int Game::getSentHistoryUncompressedBytesPerSecond()
{
	return s_sentHistoryUncompressedBytesPerSecond;
}

//-------------------------------------------------------------------

int Game::getSentCompressedBytesPerSecond()
{
	return s_sentCompressedBytesPerSecond;
}

//-------------------------------------------------------------------

int Game::getSentHistoryCompressedBytesPerSecond()
{
	return s_sentHistoryCompressedBytesPerSecond;
}

//-------------------------------------------------------------------

void Game::setRadarRange(float range)
{
	ms_radarRange = range;
}

//-------------------------------------------------------------------

float Game::getRadarRange()
{
	return ms_radarRange;
}

//----------------------------------------------------------------------

Game::SceneType Game::getHudSceneType()
{
#if PRODUCTION == 0
	if (ms_useSpaceHudOnly)
	{
		return ST_space;
	}
#endif

	Game::SceneType currentSceneType = ST_ground;

	if (isSceneLoading())
	{
		currentSceneType = ms_isSpace ? ST_space : ST_ground;
	}
	else
	{
		CreatureObject * creature = getPlayerCreature();
		if (creature)
		{
			if (creature->getShipStation() != ShipStation::ShipStation_None)
			{
				// Assume space.
				currentSceneType = ST_space;

				// Override if necessary.
				GroundScene const * const gs = dynamic_cast<GroundScene const *>(Game::getScene());
				if (NULL != gs)
				{
					if (gs->getCurrentView() == GroundScene::CI_free ||
						gs->getCurrentView() == GroundScene::CI_debugPortal)
					{
						currentSceneType = ST_ground;
					}
				}
			}
		}
	}

	return currentSceneType;
}

//----------------------------------------------------------------------

Game::SceneType Game::getLastHudSceneType()
{
	return ms_lastHudSceneType;
}

//----------------------------------------------------------------------

bool Game::hudSceneTypeIsLastHudSceneType ()
{
	return getHudSceneType() == ms_lastHudSceneType;
}

//----------------------------------------------------------------------

bool Game::isSpace()
{
	return ms_isSpace;
}

//----------------------------------------------------------------------

bool Game::isSpaceSceneName (std::string const & sceneName)
{
	return _strnicmp ("space_", sceneName.c_str(), 6) == 0;
}

//----------------------------------------------------------------------

bool Game::isTutorial(void)
{
	std::string zoneName;

	if (getPlayer())
	{
		Vector location_w = getPlayer()->getPosition_w();

		GroundZoneManager::getZoneName(ms_sceneId.c_str(), location_w, zoneName);
	}

	if (isSpace())
	{
		return (_strnicmp( "space_ord_mantell", zoneName.c_str(), 17 ) == 0);
	}

	return (
	   (_strnicmp( "npe", zoneName.c_str(), 3 ) == 0)
	|| (_strnicmp( "tutorial", zoneName.c_str(), 8) == 0)
	|| (_strnicmp( "space_npe_falcon", zoneName.c_str(), 16) == 0)
	);
}

//----------------------------------------------------------------------

void Game::updateHudSceneType()
{
	ms_lastHudSceneType = getHudSceneType();

	if (ms_scene)
	{
		// This changes the maximum distance a sound is heard. In space we want
		// sounds audible further due to increased visiblity. In the ground
		// game we are pulling the sound range back in.

		Audio::setSoundFallOffPower((ms_lastHudSceneType == ST_ground) ? 3 : 5);
	}
}

//----------------------------------------------------------------------

void Game::emitSceneChange()
{
	NOT_NULL(ms_emitter);

	ms_emitter->emitMessage(MessageDispatch::MessageBase (Messages::SCENE_CHANGED));
	Transceivers::sceneChanged.emitMessage(true);

	updateHudSceneType();

	InputActivityManager::setInactive(IoWinManager::isInactive());
}

//----------------------------------------------------------------------

bool Game::isHudSceneTypeSpace()
{
	return getHudSceneType() == ST_space;
}

//----------------------------------------------------------------------

void Game::resetHud()
{
	ms_lastHudSceneType = ST_reset;
}

// ---------------------------------------------------------------------

float Game::getBrightness()
{
	return isSpace() ? ms_spaceBrightness : ms_groundBrightness;
}

// ---------------------------------------------------------------------

void Game::setBrightness(float const brightness)
{
	if (isSpace())
		ms_spaceBrightness = brightness;
	else
		ms_groundBrightness = brightness;

	Graphics::setBrightness(brightness);
}

// ---------------------------------------------------------------------

float Game::getDefaultBrightness()
{
	return 1.f;
}

// ---------------------------------------------------------------------

float Game::getContrast()
{
	return isSpace() ? ms_spaceContrast : ms_groundContrast;
}

// ---------------------------------------------------------------------

void Game::setContrast(float const contrast)
{
	if (isSpace())
		ms_spaceContrast = contrast;
	else
		ms_groundContrast = contrast;

	Graphics::setContrast(contrast);
}

// ---------------------------------------------------------------------

float Game::getDefaultContrast()
{
	return 1.f;
}

// ---------------------------------------------------------------------

float Game::getGamma()
{
	return isSpace() ? ms_spaceGamma : ms_groundGamma;
}

// ---------------------------------------------------------------------

void Game::setGamma(float const gamma)
{
	if (isSpace())
		ms_spaceGamma = gamma;
	else
		ms_groundGamma = gamma;

	Graphics::setGamma(gamma);
}

// ---------------------------------------------------------------------

float Game::getDefaultGamma()
{
	return 1.f;
}

// ---------------------------------------------------------------------

int Game::getGameFeatureBits()
{
	return ms_gameFeatureBits;
}

// ---------------------------------------------------------------------

int Game::getSubscriptionFeatureBits()
{
	return ms_subscriptionFeatureBits;
}

// ---------------------------------------------------------------------

void Game::setGameFeatureBits(int bits)
{
	char buffer[40];
	_itoa(bits, buffer, 2);
	CrashReportInformation::addStaticText("GameFeatureBits: %s\n", buffer);
	ms_gameFeatureBits = bits;
}

// ---------------------------------------------------------------------

void Game::setSubscriptionFeatureBits(int bits)
{
	char buffer[40];
	_itoa(bits, buffer, 2);
	CrashReportInformation::addStaticText("SubscriptionFeatureBits: %s\n", buffer);
	ms_subscriptionFeatureBits = bits;
}

// ---------------------------------------------------------------------

int Game::getServerSideGameFeatureBits()
{
	return ms_serverSideGameFeatureBits;
}

// ---------------------------------------------------------------------

int Game::getServerSideSubscriptionFeatureBits()
{
	return ms_serverSideSubscriptionFeatureBits;
}

// ---------------------------------------------------------------------

void Game::setServerSideFeatureBits(int gameFeatureBits, int subscriptionFeatureBits)
{
	ms_serverSideGameFeatureBits = gameFeatureBits;
	ms_serverSideSubscriptionFeatureBits = subscriptionFeatureBits;
}

// ---------------------------------------------------------------------

void Game::requestCollectionServerFirstList()
{
	GenericValueTypeMessage<std::string> const msg("CollectionServerFirstListRequest", s_collectionServerFirstListVersion);
	GameNetwork::send (msg, true);
}

// ---------------------------------------------------------------------

void Game::responseCollectionServerFirstList(const std::string & collectionServerFirstListVersion, const std::set<std::pair<std::pair<int32, std::string>, std::pair<NetworkId, Unicode::String> > > & collectionServerFirst)
{
	if (s_collectionServerFirstListVersion != collectionServerFirstListVersion)
	{
		s_collectionServerFirstListVersion = collectionServerFirstListVersion;
		CollectionsDataTable::setServerFirstData(collectionServerFirst);

		Transceivers::collectionServerFirstChanged.emitMessage(s_collectionServerFirstListVersion);
	}
}

// ---------------------------------------------------------------------

void Game::handleCollectionShowServerFirstOptionChanged(bool enabled)
{
	Transceivers::collectionShowServerFirstOptionChanged.emitMessage(enabled);
}

// ---------------------------------------------------------------------

void Game::setExternalCommandHandler(ExternalCommandHandler const handler)
{
	ms_externalCommandHandler = handler;
}

// ---------------------------------------------------------------------

void Game::externalCommand(const char* name)
{
	if (ms_externalCommandHandler)
		ms_externalCommandHandler(name);
}

// ---------------------------------------------------------------------

std::string Game::calculateNonInstanceSceneId(std::string const & scene)
{
	size_t const underscorePos = scene.rfind('_');

	if (underscorePos != std::string::npos && underscorePos < scene.size() - 1)
	{
		if (isdigit(scene.at(underscorePos + 1)))
		{
			return scene.substr(0, underscorePos);
		}
	}

	return scene;
}

// ---------------------------------------------------------------------

#if PRODUCTION == 0
void Game::videoCaptureConfig(int resolution, int seconds, int quality, const char* filename)
{
	VideoCapture::install(); // Installs SoeUtilMemoryAdapter on first call
	VideoCapture::SingleUse::config(resolution, seconds, quality, filename, &AudioCapture::SwgAudioCaptureManager::GetInstance());
}
#endif // PRODUCTION

// ---------------------------------------------------------------------

#if PRODUCTION == 0
class VideoCaptureCallback : public VideoCapture::SingleUse::ICallback
{
public:
	static VideoCaptureCallback& GetInstance();
	virtual ~VideoCaptureCallback(){}
	virtual void OnStart()
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("VideoCapture started"));
	}
	virtual void OnStop()
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("VideoCapture stopped"));
	}
private:
	VideoCaptureCallback(){}
	VideoCaptureCallback(const VideoCaptureCallback&);
	VideoCaptureCallback& operator=(const VideoCaptureCallback&);
};

VideoCaptureCallback& VideoCaptureCallback::GetInstance()
{
	static VideoCaptureCallback s_videoCaptureCallback;
	return s_videoCaptureCallback;
}

void Game::videoCaptureStart()
{
	VideoCapture::install(); // Installs SoeUtilMemoryAdapter on first call
	VideoCapture::SingleUse::start(&VideoCaptureCallback::GetInstance(), &AudioCapture::SwgAudioCaptureManager::GetInstance());
}
#endif // PRODUCTION

// ---------------------------------------------------------------------

#if PRODUCTION == 0
void Game::videoCaptureStop()
{
	VideoCapture::SingleUse::stop();
}
#endif // PRODUCTION

// ---------------------------------------------------------------------
