//===================================================================
//
// ConfigClientGame.h
// copyright 1999 bootprint entertainment
// copyright 2001 Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_ConfigClientGame_H
#define INCLUDED_ConfigClientGame_H

//===================================================================

class Vector;

//===================================================================

class ConfigClientGame
{
public:

	static void                install();

	static const char*         getObjectViewer ();
	static int                 getViewerFlags ();
	static bool                getDisableLensFlares ();

	static const char*         getGroundScene ();
	static const char*         getParticleEditorGroundScene ();

	static real                getMusicVolume ();

	static bool                getFogDisable ();
	static bool                getDebugPrint ();

	static float               getCameraNearPlane ();
	static float               getCameraFarPlane ();
	static float               getCameraFarPlaneSpace();
	static float               getCameraFieldOfView();

	static const char*         getPlayerName();

	static const char*         getAvatarSelection ();
	static const char*         getParticleEditorAvatarSelection ();
	static Vector              getSinglePlayerStartLocation ();

	static const char*         getBlendingViewer ();

	static bool                getAllowDefaultTemplateParams();
	
	// Network Services
	static const std::string & getLoginServerAddress();
	static const uint16        getLoginServerPort();
	static const char*         getLoginClientID();
	static const char*         getLoginClientPassword();

	static void                setLoginServerAddress (const std::string & address);
	static void                setLoginServerPort    (uint16 port);
	
	static bool                getAutoConnectToLoginServer ();
	static const std::string & getCentralServerName ();
	static void                setCentralServerName (const std::string & name);
	static bool                getAutoConnectToCentralServer ();
	static const std::string & getAvatarName ();
	static void                setAvatarName (const std::string & name);

	static const std::string & getLauncherAvatarName ();
	static void                setLauncherAvatarName (const std::string & name);
	static uint32              getLauncherClusterId  ();
	static void                setLauncherClusterId  (uint32 id);

	static bool                getAutoConnectToGameServer ();
	static bool                getNextAutoConnectToGameServer ();
	static void                setNextAutoConnectToGameServer (bool b);
	static bool                getAutoQuitAfterLoadScreen ();

	static unsigned int        getNetworkTrafficMetricsResetTimeSecond ();
	static unsigned int        getNetworkTrafficMetricsResetScaleFactor ();
	
	//-----------------------------------------------------------------
	
	// login sequence
	static bool                getSkipIntro();
	static bool                getSkipSplash();

	static bool                getHudDisabled();
	static const char *        getMenuViewer ();
	static const char *        getDefaultStartLocation ();

	static bool                getDisableCutScenes();
	static bool                getReplayCutScenes();

	static bool                getCuiTest();
	static bool                getCuiStringList();
	static const char*         getTestIoWin ();
	static const char*         getTestObject1 ();
	static const char*         getTestObject2 ();
	static bool                getTestFloor ();

	static float               getFreeCameraSpeedSlow ();
	static float               getFreeCameraSpeedFast ();
	static void                setFreeCameraSpeedSlow (float f);
	static void                setFreeCameraSpeedFast (float f);

	static bool                getUseCustomInputMaps ();
	static bool                getDisableWorldSnapshot ();
	static bool                getWorldSnapshotIgnorePobChanges();
	static float               getWorldSnapshotDetailLevelBias ();

	static bool                getDisableRemoteObjectInterpolation ();
	static bool                getDisableCreatureTurningAnimation ();

	static bool                getAmbientLightInCells ();

	static float               getLoadScreenTime ();
	static int                 getLoadObjectCount ();

	static float               getWarpTolerance ();

	static float               getFreeChaseCameraMaximumZoom ();

	static bool                getLogCombatActionMessages ();
	static bool                getLogCombatPlaybackSelection ();
	static bool                getLogFireProjectileAction ();
	static bool                getLogGrenadeLobAction ();
	static bool                getLogTrailActions ();
	static bool                getLogCloneWeaponAction ();

	static bool                getForceTrailsOnAllActions ();
	static bool                getDisableCombatSpecialMoveEffects ();

	static int                 getPlayerAttackerCombatPriorityBoost ();
	static int                 getPlayerDefenderCombatPriorityBoost ();

	static bool                getLogContainerProcessing ();

	static const std::string & getGameChatCode ();

	static float               getDebugPortalCameraPathConstantStepRate();
	static bool                getDebugPortalCameraPathDragPlayer();

	static float               getTargetingRange ();
	static float               getTargetingRangeGround ();
	static float               getTargetingRangeSpace ();

	static float               getProjectileFlyByRange ();

	static bool                getDisableExitTimer ();
	static bool                getInvertMouse ();
	static bool                getShowAttachmentsInFirstPerson();

	static bool                getCharacterCreationLoadoutsEnabled ();
	static const char*         getTaskConnectionAddress ();
	static const uint16        getTaskConnectionPort ();

	static bool                getAutoInviteReject ();

	static bool                getPreloadWorldSnapshot();
	static bool                getShowMatchMakingDebug();

	static float               getHackMovementSpeed();

	static bool                isProfanityFiltered  ();

	static float               getFreeChaseCameraZoomSpeed ();

	static float               getMouseSensitivity         ();

	static bool                getRunWhenMoving            ();
	static bool                getPreloadPlayerMusicManager ();

	static bool                getUseInteriorLayoutFiles();
	static int                 getLagRequestDelay();
	static int                 getLagReportThreshold();

	static bool                getShowClientRegionChanges ();

	static float               getConnectionTimeout();

	static bool                getAllowShootingDeadTarget();
	static bool                getEnablePostIncapDeathPostureChangeLockoutPeriod();

	static bool                getCSR ();

	static bool                getAnonymousCrashReports ();
	static bool                getAllowCustomerContact ();

	static bool                getEnableChatLogging ();

	static bool                getDisableVehicleRiderCombatAnimationState ();
	static bool                getShipAutolevelDefault();

	static int                 getDisableMovementInventoryOverload ();

	static bool                getValidateShipParkingLocation ();
	static uint32              getGameBitsToClear();
	static bool                getSetJtlRetailIfBetaIsSet();

	static int                 getChatTabMaxTextLines();
	static int                 getChatTabMaxOutputTextLines();
	static int                 getChatTabOutputAverageCharactersPerLine();

	static int                 getHamBarType();

	static float               getMinimumShowExecuteTime();
	static bool                getEnableAdminLogin();

	// this a ratio of (object size / distance to object), when the ratio is < this value, an appearance extent test counts as a collision
	static float               getCollisionExtentRatio();

	static bool                getLoadBuildoutOnly();
	static float               getFlyTextSpeedPixelsPerSecond();
	static float               getFlyTextSpeedModifier();
	static bool                getDebugStringIds();
	static bool                getAllowConnectWhenFull();
	static bool                getLogClientCommandChecks();
	static float               getDebounceLightsaberBlade();
	static char const *        getCsTrackingBaseUrl();
	static bool                getUseTcgRealmTypeStage();
	static std::string const & getTcgDirectory();

	// to display additional character on the character selection window
	// (like a character that is disabled in the login DB, but we want
	// to be able to login that character without having to enable the
	// character, because of the risk that the player may log in the character)
	static uint32              get2fa8673b89f9443bb24e40c3d6127118(); // getAdditionalCharacterSelectionStationId()
	static std::string const & gete7e3221d29dd405dbfc3bd274b010854(); // getAdditionalCharacterSelectionNetworkId()
	static std::string const & geteb20e2896d8742b681b121d3fb390e2e(); // getAdditionalCharacterSelectionClusterName()
	static std::string const & getafa5930b6c2b42e6b6ec38954855abc2(); // getAdditionalCharacterSelectionCharacterName()

	static int                 getMaxWaypoints();

private:

	static void                remove();
};

//===================================================================

#endif
