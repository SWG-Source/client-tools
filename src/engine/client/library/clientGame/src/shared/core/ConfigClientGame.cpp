//===================================================================
//
// ConfigClientGame.cpp
// copyright 1999, bootprint entertainment
// copyright 2001 Sony Online Entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ConfigClientGame.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/Vector.h"
#include "sharedUtility/LocalMachineOptionManager.h"

//===================================================================

namespace ConfigClientGameNamespace
{
	const char*     ms_objectViewer;
	int             ms_viewerFlags;

	bool            ms_disableLensFlares;

	const char*     ms_groundScene;
	const char*     ms_particleEditorGroundScene;

	real            ms_musicVolume;

	bool            ms_fogDisable;
	bool            ms_debugPrint;

	real            ms_movementSpeed;

	float           ms_cameraNearPlane;
	float           ms_cameraFarPlane;
	float           ms_cameraFarPlaneSpace;
	float           ms_cameraFieldOfView;

	const char*     ms_playerName;

	const char*     ms_avatarSelection;
	float           ms_singlePlayerStartLocationX;
	float           ms_singlePlayerStartLocationY;
	float           ms_singlePlayerStartLocationZ;
	const char*     ms_particleEditorAvatarSelection;

	const char*     ms_blendingViewer;

	bool            ms_allowDefaultTemplateParams;

	// Network Services
	std::string     ms_loginServerAddress;
	int             ms_loginServerPort;

	const char*     ms_loginClientID;              //-- the login id to use to autoconnect to the login server
	const char*     ms_loginClientPassword;        //-- the login passwd to use to autoconnect to the login server
	bool            ms_autoConnectToLoginServer;

	std::string     ms_centralServerName;          //-- the central server name to autoconnect to
	bool            ms_autoConnectToCentralServer;

	std::string     ms_avatarName;                  // -- the name of the avatar to use to autoconnect to the game server
	bool            ms_autoConnectToGameServer;
	bool            ms_nextAutoConnectToGameServer;

	bool			ms_autoQuitAfterLoadScreen;

	int             ms_networkTrafficMetricsResetTimeSecond;
	int             ms_networkTrafficMetricsResetScaleFactor;

	// login sequence
	bool            ms_skipIntro;
	bool            ms_skipSplash;

	bool            ms_hudDisabled;

	const char *    ms_menuViewer;

	const char *    ms_defaultStartLocation;

	bool            ms_disableCutScenes;
	bool            ms_replayCutScenes;

	bool            ms_cuiTest;
	bool            ms_cuiStringList;
	const char*     ms_testIoWin;
	const char*     ms_testObject1;
	const char*     ms_testObject2;
	bool            ms_testFloor;

	float           ms_freeCameraSpeedSlow;
	float           ms_freeCameraSpeedFast;

	bool            ms_useCustomInputMaps;
	bool            ms_disableWorldSnapshot;
	bool            ms_worldSnapshotIgnorePobChanges;
	float           ms_worldSnapshotDetailLevelBias;

	bool            ms_disableRemoteObjectInterpolation;
	bool            ms_disableCreatureTurningAnimation;

	bool            ms_ambientLightInCells;

	float           ms_loadScreenTime;
	int             ms_loadObjectCount;

	float           ms_warpTolerance;

	float           ms_freeChaseCameraMaximumZoom;

	bool            ms_logCombatActionMessages;
	bool            ms_logCombatPlaybackSelection;
	bool            ms_logFireProjectileAction;
	bool            ms_logGrenadeLobAction;
	bool            ms_logTrailActions;
	bool            ms_logCloneWeaponAction;

	bool            ms_forceTrailsOnAllActions;
	bool            ms_disableCombatSpecialMoveEffects;

	int             ms_playerAttackerCombatPriorityBoost;
	int             ms_playerDefenderCombatPriorityBoost;

	bool            ms_logContainerProcessing;

	std::string     ms_gameChatCode;

	float           ms_debugPortalCameraPathConstantStepRate;
	bool            ms_debugPortalCameraPathDragPlayer;

	float           ms_targetingRangeGround;
	float           ms_targetingRangeSpace;

	float           ms_projectileFlyByRange;

	bool            ms_disableExitTimer;
	bool            ms_invertMouse;
	bool            ms_showAttachmentsInFirstPerson;

	bool            ms_characterCreationLoadoutsEnabled;

	bool            ms_showMatchMakingDebug;

	const char *    ms_taskConnectionAddress;
	int             ms_taskConnectionPort;

	bool            ms_autoInviteReject;

	bool            ms_preloadWorldSnapshot;

	float           ms_hackMovementSpeed;

	bool            ms_profanityFiltered;

	float           ms_freeChaseCameraZoomSpeed;

	float           ms_mouseSensitivity;

	bool            ms_runWhenMoving;
	bool            ms_preloadPlayerMusicManager;

	bool            ms_useInteriorLayoutFiles;

	std::string     ms_launcherAvatarName;
	uint32          ms_launcherClusterId;

	int             ms_lagRequestDelay;
	int             ms_lagReportThreshold;

	bool            ms_showClientRegionChanges;
	float           ms_connectionTimeout;

	bool            ms_allowShootingDeadTarget;
	bool            ms_enablePostIncapDeathPostureChangeLockoutPeriod;

	//the "don't hide" csr commands config switch.  This is merely for obfuscation purposes.
	bool            ms_0fd345d9;

	bool            ms_anonymousCrashReports;
	bool            ms_allowCustomerContact;

	bool            ms_enableChatLogging;

	bool            ms_disableVehicleRiderCombatAnimationState;
	int             ms_disableMovementInventoryOverload;
	bool            ms_shipAutolevelDefault;

	bool            ms_validateShipParkingLocation;

	int             ms_gameBitsToClear; //hack for sending down features to disable until JTL patching is supported through launchpad
	bool            ms_setJtlRetailIfBetaIsSet;

	int             ms_chatTabMaxTextLines;
	int             ms_chatTabMaxOutputTextLines;
	int             ms_chatTabOutputAverageCharactersPerLine;

	int             ms_hamBarType;

	float           ms_minimumShowExecuteTime;
	bool            ms_enableAdminLogin;

	float           ms_collisionExtentRatio;
	bool            ms_loadBuildoutOnly;

	float           ms_flyTextSpeedPixelsPerSecond;
	float           ms_flyTextSpeedModifier;

	bool            ms_debugStringIds;

	bool            ms_allowConnectWhenFull = false;

	bool            ms_logClientCommandChecks = false;

	float           ms_debounceLightsaberBlade = 2.0f;

	char const *    ms_csTrackingBaseUrl;
	bool            ms_useTcgRealmTypeStage = false;
	std::string     ms_tcgDirectory;

#if PRODUCTION == 0
	int ms_2fa8673b89f9443bb24e40c3d6127118 = 0; // additionalCharacterSelectionStationId
	std::string ms_e7e3221d29dd405dbfc3bd274b010854; // additionalCharacterSelectionNetworkId
	std::string ms_eb20e2896d8742b681b121d3fb390e2e; // additionalCharacterSelectionClusterName
	std::string ms_afa5930b6c2b42e6b6ec38954855abc2; // additionalCharacterSelectionCharacterName
#endif

	int             ms_maxWaypoints;
}

using namespace ConfigClientGameNamespace;

//===================================================================

const char* ConfigClientGame::getObjectViewer ()
{
	return ms_objectViewer;
}

//-------------------------------------------------------------------

float ConfigClientGame::getFlyTextSpeedPixelsPerSecond()
{
	return ms_flyTextSpeedPixelsPerSecond;
}

//-------------------------------------------------------------------

float ConfigClientGame::getFlyTextSpeedModifier()
{
	return ms_flyTextSpeedModifier;
}

//-------------------------------------------------------------------

bool ConfigClientGame::getLoadBuildoutOnly()
{
	return ms_loadBuildoutOnly;
}

//-------------------------------------------------------------------

int ConfigClientGame::getHamBarType ()
{
	return ms_hamBarType;
}

//-------------------------------------------------------------------

int ConfigClientGame::getViewerFlags ()
{
	return ms_viewerFlags;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getDisableLensFlares ()
{
	return ms_disableLensFlares;
}

//-------------------------------------------------------------------

const char* ConfigClientGame::getGroundScene ()
{
	return ms_groundScene;
}

//-------------------------------------------------------------------

const char* ConfigClientGame::getParticleEditorGroundScene ()
{
	return ms_particleEditorGroundScene;
}

// ----------------------------------------------------------------------

real ConfigClientGame::getMusicVolume ()
{
	return ms_musicVolume;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getFogDisable ()
{
	return ms_fogDisable;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getDebugPrint ()
{
	return ms_debugPrint;
}

// ----------------------------------------------------------------------

const char* ConfigClientGame::getPlayerName ()
{
	return ms_playerName;
}

// ----------------------------------------------------------------------

const char* ConfigClientGame::getAvatarSelection ()
{
	return ms_avatarSelection;
}

// ----------------------------------------------------------------------

Vector ConfigClientGame::getSinglePlayerStartLocation ()
{
	return Vector(ms_singlePlayerStartLocationX, ms_singlePlayerStartLocationY, ms_singlePlayerStartLocationZ);
}

// ----------------------------------------------------------------------

const char* ConfigClientGame::getParticleEditorAvatarSelection ()
{
	return ms_particleEditorAvatarSelection;
}

// ----------------------------------------------------------------------

const char* ConfigClientGame::getBlendingViewer ()
{
	return ms_blendingViewer;
}

//-------------------------------------------------------------------

bool ConfigClientGame::getAllowDefaultTemplateParams()
{
	return ms_allowDefaultTemplateParams;
}

//-------------------------------------------------------------------

const std::string & ConfigClientGame::getLoginServerAddress ()
{
	return ms_loginServerAddress;
}

//-------------------------------------------------------------------

const uint16 ConfigClientGame::getLoginServerPort ()
{
	return static_cast<const uint16>(ms_loginServerPort);
}

//-------------------------------------------------------------------

const char* ConfigClientGame::getLoginClientID()
{
	return ms_loginClientID;
}

//-------------------------------------------------------------------

const char* ConfigClientGame::getLoginClientPassword()
{
	return ms_loginClientPassword;
}

//-----------------------------------------------------------------

bool ConfigClientGame::getSkipIntro()
{
	return ms_skipIntro;
}
//-----------------------------------------------------------------

bool ConfigClientGame::getSkipSplash()
{
	return ms_skipSplash;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getHudDisabled()
{
	return ms_hudDisabled;
}

//-------------------------------------------------------------------

const char * ConfigClientGame::getMenuViewer()
{
	return ms_menuViewer;
}

//----------------------------------------------------------------------

const char * ConfigClientGame::getDefaultStartLocation ()
{
	return ms_defaultStartLocation;
}

//-----------------------------------------------------------------

bool ConfigClientGame::getDisableCutScenes()
{
	return ms_disableCutScenes;
}

//-----------------------------------------------------------------

bool ConfigClientGame::getReplayCutScenes()
{
	return ms_replayCutScenes;
}

//-----------------------------------------------------------------

bool ConfigClientGame::getAutoConnectToLoginServer ()
{
	return ms_autoConnectToLoginServer;
}

//-----------------------------------------------------------------

const std::string & ConfigClientGame::getCentralServerName ()
{
	return ms_centralServerName;
}

//-----------------------------------------------------------------

void ConfigClientGame::setCentralServerName (const std::string & name)
{
	ms_centralServerName = name;
}

//-----------------------------------------------------------------

bool ConfigClientGame::getAutoConnectToCentralServer ()
{
	return ms_autoConnectToCentralServer;
}

//-----------------------------------------------------------------

const std::string & ConfigClientGame::getAvatarName ()
{
	return ms_avatarName;
}

//-----------------------------------------------------------------

void ConfigClientGame::setAvatarName (const std::string & name)
{
	ms_avatarName = name;
}

//-----------------------------------------------------------------

bool ConfigClientGame::getAutoConnectToGameServer ()
{
	return ms_autoConnectToGameServer;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getNextAutoConnectToGameServer ()
{
	return ms_nextAutoConnectToGameServer;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getAutoQuitAfterLoadScreen ()
{
	return ms_autoQuitAfterLoadScreen;
}

//----------------------------------------------------------------------

unsigned int ConfigClientGame::getNetworkTrafficMetricsResetTimeSecond ()
{
	return static_cast<unsigned int>(ms_networkTrafficMetricsResetTimeSecond);
}

//----------------------------------------------------------------------

unsigned int ConfigClientGame::getNetworkTrafficMetricsResetScaleFactor ()
{
	return static_cast<unsigned int>(ms_networkTrafficMetricsResetScaleFactor);
}

//----------------------------------------------------------------------

void ConfigClientGame::setNextAutoConnectToGameServer (bool b)
{
	ms_nextAutoConnectToGameServer = b;
}

//-----------------------------------------------------------------

bool ConfigClientGame::getCuiTest()
{
	return ms_cuiTest;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getCuiStringList ()
{
	return ms_cuiStringList;
}

//-----------------------------------------------------------------

const char* ConfigClientGame::getTestIoWin ()
{
	return ms_testIoWin;
}

//-----------------------------------------------------------------

const char* ConfigClientGame::getTestObject1 ()
{
	return ms_testObject1;
}

//-----------------------------------------------------------------

const char* ConfigClientGame::getTestObject2 ()
{
	return ms_testObject2;
}

//-----------------------------------------------------------------

bool ConfigClientGame::getTestFloor ()
{
	return ms_testFloor;
}

//-----------------------------------------------------------------

float ConfigClientGame::getFreeCameraSpeedSlow()
{
	return ms_freeCameraSpeedSlow;
}

//-----------------------------------------------------------------

float ConfigClientGame::getFreeCameraSpeedFast()
{
	return ms_freeCameraSpeedFast;
}

//----------------------------------------------------------------------

void ConfigClientGame::setFreeCameraSpeedSlow (float f)
{
	ms_freeCameraSpeedSlow = f;
}

//----------------------------------------------------------------------

void ConfigClientGame::setFreeCameraSpeedFast (float f)
{
	ms_freeCameraSpeedFast = f;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getUseCustomInputMaps()
{
	return ms_useCustomInputMaps;
}

//-------------------------------------------------------------------

float ConfigClientGame::getCameraNearPlane ()
{
	return ms_cameraNearPlane;
}

//----------------------------------------------------------------------

float ConfigClientGame::getCameraFarPlane ()
{
	return ms_cameraFarPlane;
}

//-------------------------------------------------------------------

float ConfigClientGame::getCameraFarPlaneSpace()
{
	return ms_cameraFarPlaneSpace;
}

//-------------------------------------------------------------------

float ConfigClientGame::getCameraFieldOfView ()
{
	return ms_cameraFieldOfView;
}

//-------------------------------------------------------------------

bool ConfigClientGame::getDisableWorldSnapshot ()
{
	return ms_disableWorldSnapshot;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getWorldSnapshotIgnorePobChanges()
{
	return ms_worldSnapshotIgnorePobChanges;
}

// ----------------------------------------------------------------------

float ConfigClientGame::getWorldSnapshotDetailLevelBias ()
{
	return ms_worldSnapshotDetailLevelBias;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getDisableRemoteObjectInterpolation ()
{
	return ms_disableRemoteObjectInterpolation;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getDisableCreatureTurningAnimation ()
{
	return ms_disableCreatureTurningAnimation;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getAmbientLightInCells()
{
	return ms_ambientLightInCells;
}

// ----------------------------------------------------------------------

float ConfigClientGame::getLoadScreenTime ()
{
	return ms_loadScreenTime;
}

// ----------------------------------------------------------------------

int ConfigClientGame::getLoadObjectCount ()
{
	return ms_loadObjectCount;
}

// ----------------------------------------------------------------------

float ConfigClientGame::getWarpTolerance ()
{
	return ms_warpTolerance;
}

// ----------------------------------------------------------------------

float ConfigClientGame::getFreeChaseCameraMaximumZoom ()
{
	return ms_freeChaseCameraMaximumZoom;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getLogCombatActionMessages ()
{
	return ms_logCombatActionMessages;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getLogCombatPlaybackSelection ()
{
	return ms_logCombatPlaybackSelection;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getLogFireProjectileAction ()
{
	return ms_logFireProjectileAction;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getLogGrenadeLobAction ()
{
	return ms_logGrenadeLobAction;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getLogTrailActions ()
{
	return ms_logTrailActions;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getLogCloneWeaponAction ()
{
	return ms_logCloneWeaponAction;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getForceTrailsOnAllActions ()
{
	return ms_forceTrailsOnAllActions;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getDisableCombatSpecialMoveEffects ()
{
	return ms_disableCombatSpecialMoveEffects;
}

// ----------------------------------------------------------------------

int ConfigClientGame::getPlayerAttackerCombatPriorityBoost ()
{
	return ms_playerAttackerCombatPriorityBoost;
}

// ----------------------------------------------------------------------

int ConfigClientGame::getPlayerDefenderCombatPriorityBoost ()
{
	return ms_playerDefenderCombatPriorityBoost;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getLogContainerProcessing ()
{
	return ms_logContainerProcessing;
}

//----------------------------------------------------------------------

const std::string & ConfigClientGame::getGameChatCode ()
{
	return ms_gameChatCode;
}

// ----------------------------------------------------------------------

float ConfigClientGame::getDebugPortalCameraPathConstantStepRate()
{
	return ms_debugPortalCameraPathConstantStepRate;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getDebugPortalCameraPathDragPlayer()
{
	return ms_debugPortalCameraPathDragPlayer;
}

//----------------------------------------------------------------------

float ConfigClientGame::getTargetingRange()
{
	return Game::isHudSceneTypeSpace() ? getTargetingRangeSpace() : getTargetingRangeGround();
}

//----------------------------------------------------------------------

float ConfigClientGame::getTargetingRangeGround()
{
	return ms_targetingRangeGround;
}

//----------------------------------------------------------------------

float ConfigClientGame::getTargetingRangeSpace()
{
	return ms_targetingRangeSpace;
}

//----------------------------------------------------------------------

float ConfigClientGame::getProjectileFlyByRange()
{
	return ms_projectileFlyByRange;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getDisableExitTimer ()
{
	return ms_disableExitTimer;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getInvertMouse ()
{
	return ms_invertMouse;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getShowAttachmentsInFirstPerson()
{
	return ms_showAttachmentsInFirstPerson;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getCharacterCreationLoadoutsEnabled ()
{
	return ms_characterCreationLoadoutsEnabled;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getShowMatchMakingDebug()
{
	return ms_showMatchMakingDebug;
}

//-----------------------------------------------------------------------

float ConfigClientGame::getConnectionTimeout()
{
	return ms_connectionTimeout;
}

//-----------------------------------------------------------------------

/**
 *>>>>>>>> WARNING READ THIS <<<<<<<<<<
 *
 * This is not a secure or reliable way to validate who is/is not a CSR.
 * This only checks for the obfuscation code in the client-side config
 * which anyone can add. This should ONLY be used to flag for the command
 * parser that admin commands are available.
 *
 * If you need to validate if someone is/is not a CSR (isGod) then you
 * should use Game::getPlayerObject->isAdmin() only!! But MORE importantly,
 * anything being done should also be validated server-side!!!
 */
bool ConfigClientGame::getCSR()
{
	return ms_0fd345d9;
}

//-----------------------------------------------------------------------

bool ConfigClientGame::getValidateShipParkingLocation()
{
	return ms_validateShipParkingLocation;
}

//-----------------------------------------------------------------------

uint32 ConfigClientGame::getGameBitsToClear()
{
	return static_cast<uint32>(ms_gameBitsToClear);
}

//-----------------------------------------------------------------------

bool ConfigClientGame::getSetJtlRetailIfBetaIsSet()
{
	return ms_setJtlRetailIfBetaIsSet;
}

//-----------------------------------------------------------------------

int ConfigClientGame::getChatTabMaxTextLines()
{
	return ms_chatTabMaxTextLines;
}

//-----------------------------------------------------------------------

int ConfigClientGame::getChatTabMaxOutputTextLines()
{
	return ms_chatTabMaxOutputTextLines;
}

//-----------------------------------------------------------------------

int ConfigClientGame::getChatTabOutputAverageCharactersPerLine()
{
	return ms_chatTabOutputAverageCharactersPerLine;
}

bool ConfigClientGame::getDebugStringIds()
{
	return ms_debugStringIds;
}

bool ConfigClientGame::getAllowConnectWhenFull()
{
	return ms_allowConnectWhenFull;
}

bool ConfigClientGame::getLogClientCommandChecks()
{
	return ms_logClientCommandChecks;
}

//===================================================================

#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("ClientGame", #a,  b))
#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("ClientGame", #a, b))
#define KEY_FLOAT(a,b)   (ms_ ## a = ConfigFile::getKeyFloat("ClientGame", #a, b))
#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("ClientGame", #a, b))

#define REGISTER_FLAG(a) (DebugFlags::registerFlag(ms_ ## a, "ClientGame", #a))
#define REGISTER_OPTION(a) (LocalMachineOptionManager::registerOption(ms_ ## a, "ClientGame", #a))

//===================================================================

void ConfigClientGame::install(void)
{
	InstallTimer const installTimer("ConfigClientGame::install");

	KEY_BOOL   (disableLensFlares,             false);

	KEY_STRING (groundScene,                   0);

	KEY_FLOAT  (musicVolume,                   1.0f);

	KEY_BOOL   (fogDisable,                    false);
	KEY_BOOL   (debugPrint,                    false);

	KEY_FLOAT  (movementSpeed,                 2.f);

	KEY_FLOAT  (cameraNearPlane,               0.25f);
	KEY_FLOAT  (cameraFarPlane,                8192.f);
	KEY_FLOAT  (cameraFarPlaneSpace,           16384.f);
	KEY_FLOAT  (cameraFieldOfView,             60.f);

	KEY_STRING (playerName,                    "player");
	KEY_STRING (avatarSelection,               0);

	KEY_FLOAT  (singlePlayerStartLocationX,    0.0f);
	KEY_FLOAT  (singlePlayerStartLocationY,    0.0f);
	KEY_FLOAT  (singlePlayerStartLocationZ,    0.0f);

	KEY_STRING (blendingViewer,                0);
	KEY_STRING (objectViewer,                  0);
	KEY_INT    (viewerFlags,                   0);

	KEY_BOOL   (allowDefaultTemplateParams,    false);

	KEY_STRING (loginServerAddress,            "localhost");
	KEY_INT    (loginServerPort,               44453);
	KEY_STRING (loginClientID,                 "");
	KEY_STRING (loginClientPassword,           "");

	KEY_BOOL   (autoConnectToLoginServer,      false);
	KEY_STRING (centralServerName,             "devCluster");
	KEY_BOOL   (autoConnectToCentralServer,    false);
	KEY_STRING (avatarName,                    "");
	KEY_BOOL   (autoConnectToGameServer,        false);
	KEY_BOOL   (nextAutoConnectToGameServer,    false);
	KEY_BOOL   (autoQuitAfterLoadScreen,        false);
	KEY_INT    (networkTrafficMetricsResetTimeSecond, 60);
	KEY_INT    (networkTrafficMetricsResetScaleFactor, 8);

	KEY_BOOL   (skipIntro,                     false);
	KEY_BOOL   (skipSplash,                    false);

	KEY_BOOL   (hudDisabled,                   false);

	KEY_STRING (menuViewer,                    "");

#if PRODUCTION
	ms_defaultStartLocation = "mos_eisley";
#else
	KEY_STRING (defaultStartLocation,          "mos_eisley");
#endif

	KEY_BOOL   (disableCutScenes,              false);
	KEY_BOOL   (replayCutScenes,               true);

	KEY_BOOL   (cuiTest,                       false);
	KEY_BOOL   (cuiStringList,                 false);
	KEY_STRING (testIoWin,                     0);
	KEY_STRING (testObject1,                   "appearance/shadowtest1.msh");
	KEY_STRING (testObject2,                   0);
	KEY_BOOL   (testFloor,                     true);

	KEY_FLOAT  (freeCameraSpeedFast,           30.0f);
	KEY_FLOAT  (freeCameraSpeedSlow,           7.0f);

	KEY_BOOL   (useCustomInputMaps,            false);

#if PRODUCTION
	ms_disableWorldSnapshot = false;
#else
	KEY_BOOL   (disableWorldSnapshot,          true);
#endif

	KEY_BOOL   (worldSnapshotIgnorePobChanges,    false);
	KEY_FLOAT (worldSnapshotDetailLevelBias, 0.f);

	KEY_BOOL      (disableRemoteObjectInterpolation,    false);
	KEY_BOOL      (disableCreatureTurningAnimation,     false);
	REGISTER_FLAG (disableCreatureTurningAnimation);

	KEY_BOOL   (ambientLightInCells,           false);
	KEY_FLOAT  (loadScreenTime,                0.f);
	KEY_INT    (loadObjectCount,               0);

	KEY_FLOAT  (warpTolerance,                 15.f);

	KEY_FLOAT  (freeChaseCameraMaximumZoom,    1.f);

	KEY_BOOL      (logCombatActionMessages,    false);
	REGISTER_FLAG (logCombatActionMessages);

	KEY_BOOL      (logCombatPlaybackSelection,    false);
	REGISTER_FLAG (logCombatPlaybackSelection);

	KEY_BOOL      (logFireProjectileAction,       false);
	REGISTER_FLAG (logFireProjectileAction);

	KEY_BOOL      (logGrenadeLobAction,           false);
	REGISTER_FLAG (logGrenadeLobAction);

	KEY_BOOL      (logTrailActions, false);
	REGISTER_FLAG (logTrailActions);

	KEY_BOOL      (logCloneWeaponAction,          false);
	REGISTER_FLAG (logCloneWeaponAction);

	KEY_BOOL      (forceTrailsOnAllActions, false);
	REGISTER_FLAG (forceTrailsOnAllActions);

	KEY_BOOL      (disableCombatSpecialMoveEffects, true);
	REGISTER_FLAG (disableCombatSpecialMoveEffects);

	KEY_INT       (playerAttackerCombatPriorityBoost,    50);
	KEY_INT       (playerDefenderCombatPriorityBoost,    49);

	KEY_BOOL      (logContainerProcessing,        false);
	REGISTER_FLAG (logContainerProcessing);

	KEY_STRING (gameChatCode,    "SWG");

	// Particle Editor mode settings
	ms_particleEditorGroundScene = ConfigFile::getKeyString("ParticleEditor",    "groundScene", "terrain/simple.trn");
	ms_particleEditorAvatarSelection = ConfigFile::getKeyString("ParticleEditor",    "avatarSelection", "object/creature/player/shared_human_male.iff");

	KEY_FLOAT  (debugPortalCameraPathConstantStepRate,    0.0f);
	KEY_BOOL   (debugPortalCameraPathDragPlayer,          false);

	KEY_FLOAT  (targetingRangeGround,                     160.0f);
	KEY_FLOAT  (targetingRangeSpace,                     2048.0f);

	KEY_FLOAT  (projectileFlyByRange,    3.0f);

#if PRODUCTION
	ms_disableExitTimer = false;
#else
	KEY_BOOL   (disableExitTimer,    true);
#endif

	KEY_BOOL   (invertMouse,    false);

	KEY_BOOL   (showAttachmentsInFirstPerson,    false);

	KEY_BOOL   (characterCreationLoadoutsEnabled,    true);

	KEY_STRING (taskConnectionAddress, "");
	KEY_INT    (taskConnectionPort,    0);
	KEY_BOOL   (autoInviteReject,      false);

	bool const shouldPreloadWorldSnapshot = MemoryManager::getLimit() >= 375;

	KEY_BOOL(preloadWorldSnapshot, shouldPreloadWorldSnapshot);

	KEY_FLOAT  (hackMovementSpeed,         1.0f);

	KEY_BOOL   (profanityFiltered,         true);

	KEY_FLOAT  (freeChaseCameraZoomSpeed,  0.2f);

	KEY_FLOAT  (mouseSensitivity,          1.0f);

	KEY_BOOL   (runWhenMoving,             true);

	KEY_STRING (launcherAvatarName,        "");
	KEY_INT    (launcherClusterId,         0);
	KEY_INT    (lagRequestDelay, 600000);
	KEY_INT    (lagReportThreshold, 10000);

	KEY_BOOL   (showClientRegionChanges, true);
	KEY_FLOAT  (connectionTimeout, 600.0f);

	//----------------------------------------------------------------------

	REGISTER_OPTION  (showAttachmentsInFirstPerson);
	REGISTER_OPTION  (avatarName);
	REGISTER_OPTION  (centralServerName);
	REGISTER_OPTION  (freeCameraSpeedSlow);
	REGISTER_OPTION  (freeCameraSpeedFast);

#if PRODUCTION
	KEY_BOOL (preloadPlayerMusicManager, true);
#else
	KEY_BOOL (preloadPlayerMusicManager, false);
#endif

	KEY_BOOL (useInteriorLayoutFiles,    true);

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_showMatchMakingDebug, "ClientGame", "showMatchMakingDebug", PlayerObject::showMatchMakingStatistics);
#endif // _DEBUG

	KEY_BOOL (allowShootingDeadTarget, true);
	KEY_BOOL (enablePostIncapDeathPostureChangeLockoutPeriod, false);

	//the csr config switch (only obfuscation, not true security).
	//The functionaly provided by hacking this config switch is functionally that is already technically available,
	// the ocnfig switch makes them easier to leverage.
	KEY_BOOL  (0fd345d9,      false);

	KEY_BOOL  (anonymousCrashReports,  true);
	KEY_BOOL  (allowCustomerContact,   false);

	KEY_BOOL  (enableChatLogging, false);

	KEY_BOOL  (disableVehicleRiderCombatAnimationState, true);
	KEY_BOOL  (shipAutolevelDefault, false);

	// should be kept in sync with ConfigServerGame's moveValidationMaxInventoryOverload
	KEY_INT   (disableMovementInventoryOverload, 121);

	KEY_BOOL  (validateShipParkingLocation, true);

	KEY_INT   (gameBitsToClear, 0);
	KEY_BOOL  (setJtlRetailIfBetaIsSet, false);

	KEY_INT   (chatTabMaxTextLines, 1000);
	KEY_INT   (chatTabMaxOutputTextLines, 500);
	KEY_INT   (chatTabOutputAverageCharactersPerLine, 80);

	KEY_INT   (hamBarType, 0);

	KEY_FLOAT (minimumShowExecuteTime, 0);
	KEY_BOOL  (enableAdminLogin, false);

#if PRODUCTION
	ms_collisionExtentRatio = 0.05f;
#else
	KEY_FLOAT (collisionExtentRatio, 0.05f);
#endif

	KEY_BOOL  (loadBuildoutOnly, false);

	KEY_FLOAT (flyTextSpeedPixelsPerSecond, -500.f);
	KEY_FLOAT (flyTextSpeedModifier, 0.9f);

	KEY_BOOL (debugStringIds, false);

#if PRODUCTION
	ms_allowConnectWhenFull = false;
#else
	KEY_BOOL (allowConnectWhenFull, false);
#endif

	KEY_BOOL (logClientCommandChecks, false);

	KEY_FLOAT(debounceLightsaberBlade, 2.0f);

#define CS_TRACKING_BASE_URL "https://soe-ing.custhelp.com/cgi-bin/soe_ing.cfg/php/enduser/std_alp.php"
#if PRODUCTION
	ms_csTrackingBaseUrl = CS_TRACKING_BASE_URL;
#else
	KEY_STRING(csTrackingBaseUrl, CS_TRACKING_BASE_URL);
#endif

#if PRODUCTION
	ms_useTcgRealmTypeStage = false;
#else
	KEY_BOOL(useTcgRealmTypeStage, false);
#endif

	KEY_STRING(tcgDirectory, "TradingCardGame");

#if PRODUCTION == 0
	KEY_INT(2fa8673b89f9443bb24e40c3d6127118, 0); // additionalCharacterSelectionStationId
	KEY_STRING(e7e3221d29dd405dbfc3bd274b010854, ""); // additionalCharacterSelectionNetworkId
	KEY_STRING(eb20e2896d8742b681b121d3fb390e2e, ""); // additionalCharacterSelectionClusterName
	KEY_STRING(afa5930b6c2b42e6b6ec38954855abc2, ""); // additionalCharacterSelectionCharacterName
#endif

	KEY_INT(maxWaypoints, 100); // Should be in sync with ConfigServerGames's maxWaypointsPerCharacter 

	ExitChain::add(ConfigClientGame::remove, "ConfigClientGame::remove", 0, false);
}

//-----------------------------------------------------------------------------
void ConfigClientGame::remove(void)
{
	DebugFlags::unregisterFlag(ms_showMatchMakingDebug);
}

//-----------------------------------------------------------------------

const char * ConfigClientGame::getTaskConnectionAddress()
{
	return ms_taskConnectionAddress;
}

//-----------------------------------------------------------------------

const uint16 ConfigClientGame::getTaskConnectionPort()
{
	return static_cast<uint16>(ms_taskConnectionPort);
}

//----------------------------------------------------------------------

bool ConfigClientGame::getAutoInviteReject ()
{
	return ms_autoInviteReject;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getPreloadWorldSnapshot()
{
	return ms_preloadWorldSnapshot;
}

//----------------------------------------------------------------------

float ConfigClientGame::getHackMovementSpeed()
{
	return ms_hackMovementSpeed;
}

//----------------------------------------------------------------------

bool ConfigClientGame::isProfanityFiltered  ()
{
	return ms_profanityFiltered;
}

//----------------------------------------------------------------------

float ConfigClientGame::getFreeChaseCameraZoomSpeed ()
{
	return ms_freeChaseCameraZoomSpeed;
}

//----------------------------------------------------------------------

float ConfigClientGame::getMouseSensitivity         ()
{
	return ms_mouseSensitivity;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getRunWhenMoving            ()
{
	return ms_runWhenMoving;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getPreloadPlayerMusicManager ()
{
	return ms_preloadPlayerMusicManager;
}

//----------------------------------------------------------------------

bool ConfigClientGame::getUseInteriorLayoutFiles()
{
	return ms_useInteriorLayoutFiles;
}

//----------------------------------------------------------------------

const std::string & ConfigClientGame::getLauncherAvatarName ()
{
	return ms_launcherAvatarName;
}

//----------------------------------------------------------------------

void ConfigClientGame::setLauncherAvatarName (const std::string & name)
{
	ms_launcherAvatarName = name;
}

//----------------------------------------------------------------------

uint32 ConfigClientGame::getLauncherClusterId  ()
{
	return ms_launcherClusterId;
}

//----------------------------------------------------------------------

void ConfigClientGame::setLauncherClusterId  (uint32 id)
{
	ms_launcherClusterId = id;
}

//---------------------------------------------------------------------

int ConfigClientGame::getLagRequestDelay()
{
	return ms_lagRequestDelay;
}

//---------------------------------------------------------------------

int ConfigClientGame::getLagReportThreshold()
{
	return ms_lagReportThreshold;
}

//---------------------------------------------------------------------

bool ConfigClientGame::getShowClientRegionChanges()
{
	return ms_showClientRegionChanges;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getAllowShootingDeadTarget()
{
	return ms_allowShootingDeadTarget;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getEnablePostIncapDeathPostureChangeLockoutPeriod()
{
	return ms_enablePostIncapDeathPostureChangeLockoutPeriod;
}

//----------------------------------------------------------------------

void ConfigClientGame::setLoginServerAddress (const std::string & address)
{
	ms_loginServerAddress = address;
}

//----------------------------------------------------------------------

void ConfigClientGame::setLoginServerPort    (uint16 port)
{
	ms_loginServerPort = port;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getAnonymousCrashReports ()
{
	return ms_anonymousCrashReports;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getAllowCustomerContact ()
{
	return ms_allowCustomerContact;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getEnableChatLogging ()
{
	return ms_enableChatLogging;
}

// ----------------------------------------------------------------------
/**
 * Disable a non-mount vehicle rider (e.g. a speeder bike rider) from entering the combat portion
 * of the animation state hierarchy when this value is true.
 *
 * @return  true if the rider of a land-based vehicle (mounts system, non-mount) rider should not
 *          go into the combat portion of the animation hierarchy; false if the rider should be
 *          allowed into the combat portion of the animation state hierarchy.
 */

bool ConfigClientGame::getDisableVehicleRiderCombatAnimationState ()
{
	return ms_disableVehicleRiderCombatAnimationState;
}

// ----------------------------------------------------------------------

int ConfigClientGame::getDisableMovementInventoryOverload ()
{
	return ms_disableMovementInventoryOverload;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getShipAutolevelDefault()
{
	return ms_shipAutolevelDefault;
}

// ----------------------------------------------------------------------

float ConfigClientGame::getMinimumShowExecuteTime()
{
	return ms_minimumShowExecuteTime;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getEnableAdminLogin()
{
	return ms_enableAdminLogin;
}

// ----------------------------------------------------------------------

float ConfigClientGame::getCollisionExtentRatio()
{
	return ms_collisionExtentRatio;
}

// ----------------------------------------------------------------------

float ConfigClientGame::getDebounceLightsaberBlade()
{
	return ms_debounceLightsaberBlade;
}

// ----------------------------------------------------------------------

char const * ConfigClientGame::getCsTrackingBaseUrl()
{
	return ms_csTrackingBaseUrl;
}

// ----------------------------------------------------------------------

bool ConfigClientGame::getUseTcgRealmTypeStage()
{
	return ms_useTcgRealmTypeStage;
}

// ----------------------------------------------------------------------

std::string const & ConfigClientGame::getTcgDirectory()
{
	return ms_tcgDirectory;
}

// ----------------------------------------------------------------------
#if PRODUCTION == 0
uint32 ConfigClientGame::get2fa8673b89f9443bb24e40c3d6127118() // getAdditionalCharacterSelectionStationId()
{
	return static_cast<uint32>(ms_2fa8673b89f9443bb24e40c3d6127118); // additionalCharacterSelectionStationId
}

// ----------------------------------------------------------------------

std::string const & ConfigClientGame::gete7e3221d29dd405dbfc3bd274b010854() // getAdditionalCharacterSelectionNetworkId()
{
	return ms_e7e3221d29dd405dbfc3bd274b010854; // additionalCharacterSelectionNetworkId
}

// ----------------------------------------------------------------------

std::string const & ConfigClientGame::geteb20e2896d8742b681b121d3fb390e2e() // getAdditionalCharacterSelectionClusterName()
{
	return ms_eb20e2896d8742b681b121d3fb390e2e; // additionalCharacterSelectionClusterName
}

// ----------------------------------------------------------------------

std::string const & ConfigClientGame::getafa5930b6c2b42e6b6ec38954855abc2() // getAdditionalCharacterSelectionCharacterName()
{
	return ms_afa5930b6c2b42e6b6ec38954855abc2; // additionalCharacterSelectionCharacterName
}
#endif

// ----------------------------------------------------------------------

int ConfigClientGame::getMaxWaypoints()
{
   return ms_maxWaypoints;
}

//===================================================================
