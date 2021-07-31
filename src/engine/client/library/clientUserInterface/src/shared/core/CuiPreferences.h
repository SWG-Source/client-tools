//======================================================================
//
// CuiPreferences.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiPreferences_H
#define INCLUDED_CuiPreferences_H

//======================================================================

class Callback;

//----------------------------------------------------------------------

class CuiPreferences
{
public:
	static void                install     ();
	static void                remove      ();
	static bool                isInstalled ();

	static float               getHudOpacity ();
	static void                setHudOpacity (float f);

	static float               getTooltipDelaySecs ();
	static void                setTooltipDelaySecs (float f);

	static bool                getLocationDisplayEnabled();
	static void                setLocationDisplayEnabled(bool b);

	static bool                getDpsMeterEnabled();
	static void                setDpsMeterEnabled(bool b);

	static bool                getGroundRadarTerrainEnabled ();
	static void                setGroundRadarTerrainEnabled (bool b);

	static bool                getGroundRadarBlinkCombatEnabled();
	static void                setGroundRadarBlinkCombatEnabled(bool b);

	static bool                getShowLookAtTargetStatusWindowEnabled();
	static void                setShowLookAtTargetStatusWindowEnabled(bool b);

	static bool                getShowStatusOverIntendedTarget();
	static void                setShowStatusOverIntendedTarget(bool b);

	static bool                getUseDoubleToolbar ();
	static void                setUseDoubleToolbar (bool b);

	static bool                getShowToolbarCooldownTimer();
	static void                setShowToolbarCooldownTimer(bool b);

	static bool                getPointerModeMouseCameraEnabled ();
	static void                setPointerModeMouseCameraEnabled (bool b);

	static bool                getMouseModeDefault ();
	static void                setMouseModeDefault (bool b);

	static bool                getDragOntoContainers ();
	static void                setDragOntoContainers (bool b);

	static bool                getTurnStrafesDuringMouseModeToggle ();
	static void                setTurnStrafesDuringMouseModeToggle (bool b);
	static bool                isTurnStrafe                        ();

	static bool                getCanFireSecondariesFromToolbar ();
	static void                setCanFireSecondariesFromToolbar (bool b);

	static bool                getMiddleMouseDrivesMovementToggle();
	static void                setMiddleMouseDrivesMovementToggle(bool b);

	static bool                getMouseLeftAndRightDrivesMovementToggle();
	static void                setMouseLeftAndRightDrivesMovementToggle(bool b);

	static bool                getScrollThroughDefaultActions();
	static void                setScrollThroughDefaultActions(bool b);

	static bool                getUseNewbieTutorial ();
	static void                setUseNewbieTutorial (bool b);

	static bool                getUseExpMonitor ();
	static void                setUseExpMonitor (bool b);

	static bool                getUseWaypointMonitor ();
	static void                setUseWaypointMonitor (bool b);

	static bool                getShowGroupWaypoints ();
	static void                setShowGroupWaypoints (bool b);

	static bool                getTargetNothingUntargets ();
	static void                setTargetNothingUntargets (bool b);

	static bool                getModalChat ();
	static void                setModalChat (bool b);

	static bool                getNetStatusEnabled ();
	static void                setNetStatusEnabled (bool b);

	static void                setConfirmObjDelete (bool b);
	static bool                getConfirmObjDelete ();

	static void                setConfirmCrafting (bool b);
	static bool                getConfirmCrafting ();

	static float               getObjectNameRange ();
	static void                setObjectNameRange (float f);

	static bool                getAutoJoinChatRoomOnCreate ();
	static void                setAutoJoinChatRoomOnCreate (bool b);

	static float               getCameraInertia           ();
	static void                setCameraInertia           (float f);

	static float               getFirstPersonCameraInertia  ();
	static void                setFirstPersonCameraInertia  (float f);

	static void                setDrawNetworkIds (int drawNetworkIds);
	static int                 getDrawNetworkIds ();

	static void                setDrawObjectNames (bool drawObjectNames);
	static bool                getDrawObjectNames ();

	static void                setDrawSelfName (bool drawSelfName);
	static bool                getDrawSelfName ();

	static void                setDrawObjectNamesPlayers (bool drawObjectNames);
	static bool                getDrawObjectNamesPlayers ();

	static void                setDrawObjectNamesGroup (bool drawObjectNames);
	static bool                getDrawObjectNamesGroup ();

	static void                setDrawObjectNamesNpcs (bool drawObjectNames);
	static bool                getDrawObjectNamesNpcs ();

	static void                setDrawObjectNamesGuild (bool drawObjectNames);
	static bool                getDrawObjectNamesGuild ();

	static void                setDrawObjectNamesSigns (bool drawObjectNames);
	static bool                getDrawObjectNamesSigns ();

	static void                setDrawObjectNamesMyName (bool drawObjectNames);
	static bool                getDrawObjectNamesMyName ();

	static void                setDrawObjectNamesBeasts (bool drawObjectNames);
	static bool                getDrawObjectNamesBeasts ();

	static bool                getAllowTargetAnything ();
	static void                setAllowTargetAnything (bool b);

	static bool                getDebugExamine ();
	static void                setDebugExamine (bool b);

	static bool                getDebugClipboardExamine ();
	static void                setDebugClipboardExamine (bool b);

	static float               getFlyTextSize ();
	static void                setFlyTextSize (float f);

	static bool                getAutoInviteReject ();
	static void                setAutoInviteReject (bool b);

	static float               getObjectNameFontSizeFactor ();
	static void                setObjectNameFontSizeFactor (float f);

	static bool                getShowSystemMessages ();
	static void                setShowSystemMessages (bool b);

	static bool                getShowPopupHelp ();
	static void                setShowPopupHelp (bool b);

	static const std::string & getPaletteName ();
	static bool                setPaletteName (const std::string & paletteName, bool testForce, bool setName = true);

	typedef stdvector<std::string>::fwd StringVector;
	static const StringVector & getPaletteNames (bool testForce);

	static bool                getChatAutoEmote ();
	static void                setChatAutoEmote (bool b);

	static bool                getShowIconNames ();
	static void                setShowIconNames (bool b);

	static void                setObjectIconSize (float f);
	static float               getObjectIconSize ();
	static float               getObjectIconMinSize ();
	static float               getObjectIconMaxSize ();

	static void                setCommandButtonOpacity (float f);
	static float               getCommandButtonOpacity ();

	static void                setSystemMessageDuration (float f);
	static float               getSystemMessageDuration ();

	static void                setShowWaypointArrowsOnscreen (bool b);
	static bool                getShowWaypointArrowsOnscreen ();

	static void                setDropShadowUiEnabled (bool b);
	static bool                getDropShadowUiEnabled ();

	static void                setDropShadowObjectNamesEnabled (bool b);
	static bool                getDropShadowObjectNamesEnabled ();

	static void                setRotateMap(bool b);
	static bool                getRotateMap ();

	static void                setRotateInventoryObjects(bool b);
	static bool                getRotateInventoryObjects ();

	static void                setShowInterestingAppearance (bool b);
	static bool                getShowInterestingAppearance ();

	static void                setShowAFKSpeech (bool b);
	static bool                getShowAFKSpeech ();

	static void                setShowNPCSpeech (bool b);
	static bool                getShowNPCSpeech ();

	static void                setScreenShake (bool b);
	static bool                getScreenShake ();

	static void                setShowGameObjectArrowsOnRadar(bool b);
	static bool                getShowGameObjectArrowsOnRadar();
 
	static void                setShowRadarNPCs(bool b);
	static bool                getShowRadarNPCs();

	static void                setUseModelessInterface( bool b );
	static bool                getUseModelessInterface();
	// the function above LIES!!!
	static bool                getActualUseModelessInterface();

	static void                setUseSwgMouseMap( bool b );
	static bool                getUseSwgMouseMap();

	static void                setCombatSpamBrief( bool b );
	static bool                getCombatSpamBrief();

	static void                setCombatSpamVerbose( bool b );
	static bool                getCombatSpamVerbose();
/*
	static void                setSpamShowAttacker( bool b );
	static bool                getSpamShowAttacker();

	static void                setSpamShowDefender( bool b );
	static bool                getSpamShowDefender();

	static void                setSpamShowAttack( bool b );
	static bool                getSpamShowAttack();
*/
	static void                setSpamShowWeapon( bool b );
	static bool                getSpamShowWeapon();
/*
	static void                setSpamShowTotalDamage( bool b );
	static bool                getSpamShowTotalDamage();

	static void                setSpamShowPrimaryDamage( bool b );
	static bool                getSpamShowPrimaryDamage();

	static void                setSpamShowPrimaryDamageType( bool b );
	static bool                getSpamShowPrimaryDamageType();

	static void                setSpamShowElementalDamage( bool b );
	static bool                getSpamShowElementalDamage();

	static void                setSpamShowElementalDamageType( bool b );
	static bool                getSpamShowElementalDamageType();

	static void                setSpamShowArmor( bool b );
	static bool                getSpamShowArmor();

	static void                setSpamShowArmorAmount( bool b );
	static bool                getSpamShowArmorAmount();

	static void                setSpamShowRawDamage( bool b );
	static bool                getSpamShowRawDamage();
*/
	static void                setSpamShowDamageDetail( bool b );
	static bool                getSpamShowDamageDetail();

	static void                setSpamShowArmorAbsorption( bool b );
	static bool                getSpamShowArmorAbsorption();

	static void                setCharacterSheetShowDetails( bool b );
	static bool                getCharacterSheetShowDetails();

	static void                setChatBarFadesOut( bool b );
	static bool                getChatBarFadesOut();
	
	static void                setUiSettingsVersion( int i );
	static int                 getUiSettingsVersion();

	static int                 getDefaultUiSettingsVersion();

	static void				   setNewVendorDoubleClickExamine(bool b);
	static bool				   getNewVendorDoubleClickExamine();

	static void				   setDisableAnimationPriorities(bool b);
	static bool				   getDisableAnimationPriorities();

	static void				   setShowBackpack(bool b);
	static bool				   getShowBackpack();

	static void				   setShowHelmet(bool b);
	static bool				   getShowHelmet();

	static void				   setShowQuestHelper(bool b);
	static bool				   getShowQuestHelper();

	static void				   setShowCompletedCollections(bool b);
	static bool				   getShowCompletedCollections();

	//----------------------------------------------------------------------
	//-- space stuff
	//----------------------------------------------------------------------

	static void setJoystickDeadZone(float f);
	static float getJoystickDeadZone();
	static bool isJoystickInverted();
	static void setJoystickInverted(bool b);
	static void setJoystickSensitivity(float f);
	static float getJoystickSensitivity();
	
	static void                setReticleSelect (int reticle);
	static int                 getReticleSelect ();
	
	static void                setRadarSelect (int radar);
	static int                 getRadarSelect ();

	static void                setSpaceCameraElasticity(float f);
	static float               getSpaceCameraElasticity();
	static float               getSpaceCameraElasticityMinSize ();
	static float               getSpaceCameraElasticityMaxSize ();
	static float               getSpaceCameraElasticityDefault();

	static void                setVariableTargetingReticlePercentage(float f);
	static float               getVariableTargetingReticlePercentage();
	static float               getVariableTargetingReticlePercentageMinimumSize ();
	static float               getVariableTargetingReticlePercentageMaximumSize ();

	static bool getRenderVariableTargetingReticle();
	static void setRenderVariableTargetingReticle(bool enabled);

	static int                 getShipDestroyDetailBias();
	static void                getShipDestroyDetailBias(int bias);
	static int                 getCapshipDestroyDetailBias();
	static void                getCapshipDestroyDetailBias(int bias);

	static float               getShipDebrisLifespan();
	static void                setShipDebrisLifespan(float lifeSpan);

	static bool getUseSpaceDebugWindows();
	static void setUseSpaceDebugWindows(bool useit);

	static float getGlobalNebulaDensity();
	static void setGlobalNebulaDensity(float density);
	static float getGlobalNebulaDensityDefault();

	static float getGlobalNebulaRange();
	static void setGlobalNebulaRange(float range);

	static bool getShipAutolevel();
	static void setShipAutolevel(bool enabled);

	static bool getAutoSortInventoryContents();
	static void setAutoSortInventoryContents(bool enabled);

	static bool getAutoSortDataPadContents();
	static void setAutoSortDataPadContents(bool enabled);

	static bool getShowNotifications();
	static void setShowNotifications(bool b);

	static bool getAutoAimToggle();
	static void setAutoAimToggle(bool b);

	static bool getEnableGimbal();
	static void setEnableGimbal(bool b);

	static bool getTargetArrow();
	static void setTargetArrow(bool b);

	static bool getDamagerArrow();
	static void setDamagerArrow(bool b);

	static bool getVisibleEnemyDamagerArrow();
	static void setVisibleEnemyDamagerArrow(bool b);

	enum PilotMouseMode
	{
		PMM_none,
		PMM_automatic,
		PMM_virtualJoystick,
		PMM_cockpitCamera
	};

	static PilotMouseMode getPilotMouseMode();
	static void setPilotMouseMode(PilotMouseMode pmm);
	static PilotMouseMode getPilotMouseModeAutomatic();

	enum CurrencyFormat
	{
		CF_none,
		CF_comma,
		CF_period,
		CF_space
	};

	static int getCurrencyFormat();
	static void	setCurrencyFormat(CurrencyFormat cf);

	enum PovHatMode
	{
		PHM_none,
		PHM_pan,
		PHM_snap
	};

	static void setPovHatMode(PovHatMode phm);
	static PovHatMode getPovHatMode();

	static void setPovHatPanSpeed(float speed);
	static float getPovHatPanSpeed();

	static void setCockpitCameraSnapSpeed(float speed);
	static float getCockpitCameraSnapSpeed();

	static void setCockpitCameraYOffset(float offset);
	static float getCockpitCameraYOffset();

	static void setCockpitCameraZoomMultiplier(float zoomMultiplier);
	static float getCockpitCameraZoomMultiplier();

	static void setCockpitCameraRecenterOnShipMovement(bool b);
	static bool getCockpitCameraRecenterOnShipMovement();

	static void setPovHatSnapAngle(float angle);
	static float getPovHatSnapAngle();
	static void setPovHatSnapAngleDegrees(float angle);
	static float getPovHatSnapAngleDegrees();

	//----------------------------------------------------------------------

	static Callback & getUseExpMonitorCallback();
	static Callback & getUseWaypointMonitorCallback();
	static Callback & getLocationDisplayEnabledCallback();
	static Callback & getShowGroupWaypointsCallback();
	static Callback & getKeybindingsChangedCallback();
	static Callback & getObjectIconCallback();
	static Callback & getCommandButtonOpacityCallback();
	static Callback & getShowInterestingAppearanceCallback();
	static Callback & getReticleSelectCallback();
	static Callback & getRadarSelectCallback();
	static Callback & getSpaceCameraElasticityCallback();
	static Callback & getVariableTargetingReticlePercentageCallback();
	static Callback & getPaletteChangedCallback();

	static void                refreshValues ();
	static void                signalKeybindingsChanged ();

	static void		setOffsetCamera(bool b);
	static bool		getOffsetCamera();

	static void     setCombatSpamFilter(int combatSpamFilter);
	static int      getCombatSpamFilter();
	static int      getCombatSpamFilterDefault();

	static void     setCombatSpamRangeFilter(int combatSpamRangeFilter);
	static int      getCombatSpamRangeFilter();
	static int      getCombatSpamRangeFilterDefault();

	static void     setCommoditiesWindowColumnSize(int type, int columnIndex, int value);
	static int      getCommoditiesWindowColumnSize(int type, int columnIndex);

	static bool     getCollectionShowServerFirst();
	static void     setCollectionShowServerFirst(bool enabled);

	enum SecondaryTargetMode
	{
		STM_none,
		STM_lookAtTarget,
		STM_targetOfTarget
	};

	static void setSecondaryTargetMode(int mode);
	static int getSecondaryTargetMode();

	static void setAlwaysShowRangeInGroundRadar(bool alwaysShow);
	static bool getAlwaysShowRangeInGroundRadar();

	static void     setFurnitureRotationDegree(int degree);
	static int      getFurnitureRotationDegree();
	static int      getFurnitureRotationDegreeDefault();

	static bool getShowCorpseLootIcon();
	static void setShowCorpseLootIcon(bool show);

	enum BuffIconType
	{
		BIT_status,
		BIT_target,
		BIT_secondaryTarget,
		BIT_group,
		BIT_pet,
		BIT_numTypes,
	};

	static int  getBuffIconSize(int type = BIT_status);
	static void setBuffIconSize(int type, int iconSize);

	static int getBuffIconSizeSliderMin();
	static int getBuffIconSizeSliderMax();

	static int getBuffIconSizeStatus();
	static int getBuffIconSizeTarget();
	static int getBuffIconSizeSecondaryTarget();
	static int getBuffIconSizePet();
	static int getBuffIconSizeGroup();

	static void setBuffIconSizeStatus(int iconSize);
	static void setBuffIconSizeTarget(int iconSize);
	static void setBuffIconSizeSecondaryTarget(int iconSize);
	static void setBuffIconSizePet(int iconSize);
	static void setBuffIconSizeGroup(int iconSize);

	static int getBuffIconSizeStatusDefault();
	static int getBuffIconSizeTargetDefault();
	static int getBuffIconSizeSecondaryTargetDefault();
	static int getBuffIconSizePetDefault();
	static int getBuffIconSizeGroupDefault();

	static float getBuffIconWhirlygigOpacity();
	static void setBuffIconWhirlygigOpacity(float opacityPercent);
	static float getBuffIconWhirlygigOpacityDefault();

	static Callback& getBuffIconSettingsChangedCallback();

	static bool getVoiceChatEnabled();
	static void setVoiceChatEnabled(bool enabled);

	static bool getVoiceUsePushToTalk();
	static void setVoiceUsePushToTalk(bool useit);

	static bool getVoiceShowFlybar();
	static void setVoiceShowFlybar(bool showit);

	static bool getVoiceUseAdvancedChannelSelection();
	static void setVoiceUseAdvancedChannelSelection(bool useit);

	static bool getVoiceAutoDeclineInvites();
	static void setVoiceAutoDeclineInvites(bool declineThemAll);

	static bool getVoiceAutoJoinChannels();
	static void setVoiceAutoJoinChannels(bool joinAway);

	static bool getDefaultExamineHideAppearance();
	static void setDefaultExamineHideAppearance(bool hide);

	static bool getDoubleClickUnequipAppearance();
	static void setDoubleClickUnequipAppearance(bool unequip);

	static float getPlayerCameraHeight();
	static void  setPlayerCameraHeight(float height);

	static bool getShowAppearanceInventory();
	static void setShowAppearanceInventory(bool show);
	
	static bool getAutoLootCorpses();
	static void setAutoLootCorpses(bool loot);

	static float getSpeakerVolume();
	static void  setSpeakerVolume(float volume);

	static float getMicVolume();
	static void  setMicVolume(float volume);

	static float getOverheadMapOpacity();
	static void  setOverheadMapOpacity(float opacity);

	static bool  getOverheadMapShowWaypoints();
	static void  setOverheadMapShowWaypoints(bool show);

	static bool  getOverheadMapShowCreatures();
	static void  setOverheadMapShowCreatures(bool show);

	static bool  getOverheadMapShowPlayer();
	static void  setOverheadMapShowPlayer(bool show);

	static bool  getOverheadMapShowLabels();
	static void  setOverheadMapShowLabels(bool show);

	static bool  getOverheadMapShowBuildings();
	static void  setOverheadMapShowBuildings(bool show);

	static bool  getHideCharactersOnClosedGalaxies();
	static void  setHideCharactersOnClosedGalaxies(bool hide);
};

//======================================================================

#endif
