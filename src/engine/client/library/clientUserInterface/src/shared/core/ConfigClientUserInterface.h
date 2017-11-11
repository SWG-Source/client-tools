//======================================================================
//
// ConfigClientUserInterface.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ConfigClientUserInterface_H
#define INCLUDED_ConfigClientUserInterface_H

//======================================================================

class ConfigClientUserInterface
{
public:

	typedef stdvector<int>::fwd IntVector;

	static void                install ();
	static void                remove  ();

	static float               getChatBubbleFontSize        ();
	static float               getChatBubbleRangeFactor     ();
	static float               getChatBubbleDurationScale   ();
	static float               getChatBubbleDurationMax     ();
	static float               getChatBubbleDurationMin     ();
	static bool                getChatBubblesEnabled        ();
	static bool                getChatBubbleEmotesEnabled   ();
	static bool                getChatBubblesMinimized      ();
	static int                 getChatBubblesMaxStackHeight ();
	static void                getReticleDeadZoneSize       (int & x, int & y);
	static int                 getReticleDeadZoneSizeX      ();
	static float               getCameraInertia             ();
	static float               getFirstPersonCameraInertia  ();
	static int                 getDrawNetworkIds ();
	static bool                getDrawObjectNames ();
	static bool                getDrawSelfName ();
	static bool                getAllowRadialMenuPickup ();
	static const std::string & getUiRootName ();
	static const std::string & getUiRootPath ();
	static bool                getAllowTargetAnything ();
	static bool                getDebugExamine ();
	static int                 getConnectionServerPingPeriodMs ();
	static int                 getConnectionServerPingWindow ();
	static int                 getSplashTimeoutSeconds ();
	static const std::string & getIconPathState            ();
	static const std::string & getIconPathPosture          ();
	static const std::string & getIconPathCommand          ();
	static bool                getEnableStartupBugReporting ();
	static const IntVector &   getChatWindowFontSizes            ();
	static int                 getChatWindowFontSizesCount       ();
	static int                 getChatWindowFontSizeDefaultIndex ();
	static int                 getChatWindowFontSizeDefaultSize  ();
	static bool                getShowDamageOverHeadOthers   ();
	static bool                getShowDamageOverHeadSelf     ();
	static bool                getShowDamageSnare            ();
	static bool                getAutoJoinChatRoomOnCreate ();
	static float               getObjectNameRange ();
	static bool                getAlwaysSetMouseCursor ();
	static float               getDamageDelay ();
	static bool                getChatTimestampEnabled ();
	static bool                getTestGroupWithNpcs ();
	static bool                getTestImageDesignWithNpcs ();
	static bool                getJoystickMovesPointer ();
	static float               getHudOpacity ();
	static bool                getConfirmObjDelete ();
	static float               getTooltipDelaySecs ();
	static bool                getGroundRadarTerrainEnabled ();
	static bool                getSettingsEnabled ();
	static bool                getNetStatusEnabled ();
	static bool                getPointerModeMouseCameraEnabled ();
	static bool                getMouseModeDefault ();
	static bool                getTurnStrafesDuringMouseModeToggle ();
	static bool                getCanFireSecondariesFromToolbar ();
	static bool                getModalChat ();
	static bool                getUseNewbieTutorial ();
	static bool                getChatBoxKeyClick ();
	static bool                getUseExpMonitor ();
	static bool                getTargetNothingUntargets ();
	static float               getObjectNameFontSizeFactor ();
	static const std::string & getPaletteName ();
	static bool                getDrawObjectNamesPlayers ();
	static bool                getDrawObjectNamesGroup   ();
	static bool                getDrawObjectNamesNpcs    ();
	static bool                getDrawObjectNamesGuild   ();
	static bool                getDrawObjectNamesSigns   ();
	static bool                getDrawObjectNamesMyName  ();
	static bool                getDrawObjectNamesBeasts  ();
	static bool                getFatalOnBadKnowldgeBaseEntry ();
	static bool                getFatalOnBadPoiEntry ();
	static bool                getDisplayLoadingBoxes ();
	static std::string const & getMessageOfTheDayTable();
	static float               getNumberOfSecondsUntilCuiSave();
	static bool                getDrawRadarArrows();
	static bool                getDrawArrowToActiveWaypointOnly();
	static bool                getDrawNpcRadarArrows();
	static bool                getDontDrawNpcsOnRadar();	
	static bool                getDontDrawCorpsesOnRadar();	
	static bool                getDontDrawObjectsOnRadar();
	static bool                getDisplayQualifiedForSkill();
	static bool                getDrawTargetArrow();
	static bool                getMiddleMouseDrivesMovementToggle();	
	static bool                getMouseLeftAndRightDrivesMovementToggle();	
	static bool                getScrollThroughDefaultActions();	
	static bool				   getOffsetCamera();
	static bool                getDisableG15Lcd();
	static bool                getShowStatusOverIntendedTarget();
	static bool				   getEnableGimbal();
	static bool                getEnableTargetArrow();
	static bool                getEnableDamagerArrow();
	static bool                getEnableVisibleEnemyDamagerArrow();
	static bool				   GetEnableNewVendorExamine();
	static bool                getShowQuestHelper();
	static bool                getShowEnteringCombatFlyText();
	static bool                getShowCompletedCollections();
};

//======================================================================

#endif
