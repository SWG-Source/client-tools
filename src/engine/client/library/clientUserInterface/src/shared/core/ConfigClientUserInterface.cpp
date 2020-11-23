//======================================================================
//
// ConfigClientUserInterface.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/ConfigClientUserInterface.h"

#include "UIManager.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/Callback.h"
#include <string>

//======================================================================


namespace
{
	bool              ms_groundRadarTerrainEnabled;
	bool              ms_modalChat;
	float             ms_chatBubbleRangeFactor;
	float             ms_chatBubbleFontSize;
	float             ms_chatBubbleDurationScale; // seconds per character chatbubble is visible
	float             ms_chatBubbleDurationMin;   // minimum time chatbubble is visible
	float             ms_chatBubbleDurationMax;   // minimum time chatbubble is visible
	bool              ms_chatBubblesEnabled;
	bool              ms_chatBubblesMinimized;
	bool              ms_chatBubbleEmotesEnabled;
	int               ms_chatBubblesMaxStackHeight;
	int               ms_reticleDeadZoneSizeX;
	int               ms_reticleDeadZoneSizeY;
	float             ms_cameraInertia;
	int               ms_drawNetworkIds;
	bool              ms_drawObjectNames;
	bool              ms_drawSelfName;
	bool              ms_allowRadialMenuPickup;
	std::string       ms_uiRootPath;
    std::string       ms_uiRootName;
	bool              ms_allowTargetAnything;
	bool              ms_debugExamine;
	int               ms_connectionServerPingPeriodMs;
	int               ms_connectionServerPingWindow;
	int               ms_splashTimeoutSeconds;
	std::string       ms_iconPathState;
	std::string       ms_iconPathPosture;
	std::string       ms_iconPathCommand;
	bool              ms_enableStartupBugReporting;
	ConfigClientUserInterface::IntVector ms_chatWindowFontSizes;
	int               ms_chatWindowFontSizeDefaultIndex;
	bool              ms_showDamageOverHeadOthers;
	bool              ms_showDamageOverHeadSelf;
	bool              ms_showDamageSnare;
	float             ms_damageOverHeadSizeModifer;
	bool              ms_autoJoinChatRoomOnCreate;
	float             ms_objectNameRange;
	bool              ms_alwaysSetMouseCursor;
	float             ms_damageDelay;
	bool              ms_chatTimestampEnabled;
	bool              ms_testGroupWithNpcs;
	bool              ms_testImageDesignWithNpcs;
	bool              ms_joystickMovesPointer;
	float             ms_hudOpacity;
	bool              ms_confirmObjDelete;
	float             ms_tooltipDelaySecs;
	bool              ms_settingsEnabled;
	bool              ms_netStatusEnabled;
	bool              ms_pointerModeMouseCameraEnabled;
	bool              ms_mouseModeDefault;
	bool              ms_turnStrafesDuringMouseModeToggle;
	bool              ms_canFireSecondariesFromToolbar;
	bool              ms_useNewbieTutorial;
	bool              ms_chatBoxKeyClick;
	bool              ms_useExpMonitor;
	bool              ms_targetNothingUntargets;
	float             ms_firstPersonameraInertia;
	float             ms_objectNameFontSizeFactor;
	std::string       ms_paletteName;
	bool              ms_drawObjectNamesPlayers;
	bool              ms_drawObjectNamesGroup;
	bool              ms_drawObjectNamesNpcs;
	bool              ms_drawObjectNamesGuild;
	bool              ms_drawObjectNamesSigns;
	bool              ms_drawObjectNamesMyName;
	bool              ms_drawObjectNamesBeasts;
	bool              ms_fatalOnBadKnowledgeBaseEntry;
	bool              ms_fatalOnBadPoiEntry;
	bool              ms_displayLoadingBoxes;
	std::string       ms_messageOfTheDayTable;
	float             ms_numberOfSecondsUntilCuiSave;
	bool              ms_drawRadarArrows;
	bool              ms_drawArrowToActiveWaypointOnly;
	bool              ms_drawNpcRadarArrows;
	bool			  ms_dontDrawNpcsOnRadar;
	bool			  ms_dontDrawCorpsesOnRadar;
	bool			  ms_dontDrawObjectsOnRadar;
	bool              ms_displayQualifiedForSkill;
	bool              ms_drawTargetArrow;
	bool              ms_middleMouseDrivesMovementToggle;
	bool              ms_mouseLeftAndRightDrivesMovementToggle;
	bool              ms_scrollThroughDefaultActions;	
	bool			  ms_offsetCamera;
	bool              ms_disableG15Lcd;
	bool              ms_showStatusOverIntendedTarget;
	bool			  ms_enableGimbal;
	int				  ms_currencyFormat;
	bool              ms_targetArrow;
	bool              ms_damagerArrow;
	bool              ms_visibleEnemyDamagerArrow;
	bool			  ms_checkNewVendorExamine;
	bool              ms_showQuestHelper;
	bool              ms_showEnteringCombatFlyText;
	bool              ms_showCompletedCollections;
}

//----------------------------------------------------------------------

const char * const DefaultUIFile = "ui_root.ui";
const char * const KeyName = "ClientUserInterface";

#define KEY_INT(a,b)       (ms_ ## a = ConfigFile::getKeyInt    (KeyName, #a,  b))
#define KEY_BOOL(a,b)      (ms_ ## a = ConfigFile::getKeyBool   (KeyName, #a, b))
#define KEY_FLOAT(a,b)     (ms_ ## a = ConfigFile::getKeyFloat  (KeyName, #a, b))
#define KEY_STRING(a,b)    (ms_ ## a = ConfigFile::getKeyString (KeyName, #a, b))

//----------------------------------------------------------------------

void ConfigClientUserInterface::install ()
{
	ExitChain::add(ConfigClientUserInterface::remove, "ConfigClientUserInterface::remove");

	KEY_FLOAT  (chatBubbleRangeFactor,       1.0f);
	KEY_FLOAT  (chatBubbleFontSize,          1.0f);
	KEY_FLOAT  (chatBubbleDurationScale,     0.25f);
	KEY_FLOAT  (chatBubbleDurationMin,       2.00f);
	KEY_FLOAT  (chatBubbleDurationMax,      10.00f);
	KEY_BOOL   (chatBubblesEnabled,           true);
	KEY_BOOL   (chatBubblesMinimized,        false);
	KEY_BOOL   (chatBubbleEmotesEnabled,      true);
	KEY_INT    (chatBubblesMaxStackHeight,       6);
	KEY_INT    (reticleDeadZoneSizeX,          0);
	KEY_INT    (reticleDeadZoneSizeY,          0);
	KEY_FLOAT  (cameraInertia,               3.50f);
	KEY_INT    (drawNetworkIds,                  0);
	KEY_BOOL   (drawObjectNames,             false);
	KEY_BOOL   (drawSelfName,                false);
	KEY_BOOL   (allowRadialMenuPickup,       false);

	KEY_STRING(uiRootPath,					 "ui/");
	if (ConfigFile::getKeyBool("ClientUserInterface", "use4kResolutionInterface", false)) {
		ms_uiRootPath = "ui-4k/";
	}

	KEY_STRING (uiRootName,                  DefaultUIFile);
	if(ConfigFile::getKeyBool("ClientTools","loadHud",false))
	{
		ms_uiRootName = DefaultUIFile;
	}

	KEY_BOOL   (allowTargetAnything,         false);
	KEY_BOOL   (debugExamine,                false);
	KEY_INT    (connectionServerPingPeriodMs, 1000);
	KEY_INT    (connectionServerPingWindow,     20);
	KEY_INT    (splashTimeoutSeconds,            5);
	KEY_STRING (iconPathState,               "/styles.icon.state");
	KEY_STRING (iconPathPosture,             "/styles.icon.posture");
	KEY_STRING (iconPathCommand,             "/styles.icon.command");
	KEY_BOOL   (enableStartupBugReporting,   false);
	KEY_BOOL   (showDamageOverHeadOthers,    false);
	KEY_BOOL   (showDamageOverHeadSelf,      true);
	KEY_BOOL   (showDamageSnare,             true);
	KEY_FLOAT  (damageOverHeadSizeModifer,   1.5f);
	KEY_BOOL   (autoJoinChatRoomOnCreate,    true);
	KEY_FLOAT  (objectNameRange,             32.0f);

	std::string ms_tmpChatFontSizes;
	KEY_STRING (tmpChatFontSizes,             "12,13,14,16,18,20,22,24");
	KEY_INT    (chatWindowFontSizeDefaultIndex,  0);

	size_t endpos = 0;
	std::string token;
	const char tokensep [] = { ',', ' ', 0};

	while (endpos != std::string::npos && Unicode::getFirstToken (ms_tmpChatFontSizes, endpos, endpos, token, tokensep))
	{
		const int size = atoi (token.c_str ());
		ms_chatWindowFontSizes.push_back (size);
		if (endpos != std::string::npos)
		{
			++endpos;
		}
	}

	KEY_BOOL   (alwaysSetMouseCursor,  false);
	KEY_FLOAT  (damageDelay,           0.75f);
	KEY_BOOL   (chatTimestampEnabled,  false);
	KEY_BOOL   (testGroupWithNpcs,     false);
	KEY_BOOL   (testImageDesignWithNpcs, false);
	KEY_BOOL   (joystickMovesPointer,  false);
	KEY_FLOAT  (hudOpacity,            1.0f);
	KEY_BOOL   (confirmObjDelete,      true);
	KEY_FLOAT  (tooltipDelaySecs,      0.66f);
	KEY_BOOL   (groundRadarTerrainEnabled,        true);
	KEY_BOOL   (settingsEnabled,                  true);
	KEY_BOOL   (netStatusEnabled,                 false);
	KEY_BOOL   (pointerModeMouseCameraEnabled,    true);
	KEY_BOOL   (mouseModeDefault,                 false);
	KEY_BOOL   (turnStrafesDuringMouseModeToggle, false);
	KEY_BOOL   (canFireSecondariesFromToolbar, false);
	KEY_BOOL   (modalChat,                        false);
	KEY_BOOL   (useNewbieTutorial,                true);
	KEY_BOOL   (chatBoxKeyClick,                  true);
	KEY_BOOL   (useExpMonitor,                    true);
	KEY_BOOL   (targetNothingUntargets,           true);
	KEY_FLOAT  (firstPersonameraInertia,          5.0f);
	KEY_FLOAT  (objectNameFontSizeFactor,         0.8f);
	KEY_STRING (paletteName,                      "destroyer");
	KEY_BOOL   (drawObjectNamesPlayers,           true);
	KEY_BOOL   (drawObjectNamesGroup,             true);
	KEY_BOOL   (drawObjectNamesNpcs,              false);
	KEY_BOOL   (drawObjectNamesGuild,             true);
	KEY_BOOL   (drawObjectNamesSigns,             true);
	KEY_BOOL   (drawObjectNamesMyName,            false);
	KEY_BOOL   (drawObjectNamesBeasts,            false);
	KEY_BOOL   (fatalOnBadKnowledgeBaseEntry,     false);
	KEY_BOOL   (fatalOnBadPoiEntry,               false);
	KEY_BOOL   (displayLoadingBoxes,              false);
	KEY_STRING (messageOfTheDayTable,             "");
	KEY_FLOAT  (numberOfSecondsUntilCuiSave,      10.0f);
	KEY_BOOL   (drawRadarArrows,                  false);
	KEY_BOOL   (drawNpcRadarArrows,               true);
	KEY_BOOL   (dontDrawNpcsOnRadar,              false);
	KEY_BOOL   (dontDrawCorpsesOnRadar,           false);
	KEY_BOOL   (dontDrawObjectsOnRadar,           false);
	KEY_BOOL   (displayQualifiedForSkill,         false);
	KEY_BOOL   (drawArrowToActiveWaypointOnly,    false);
	KEY_BOOL   (drawTargetArrow,                  false);
	KEY_BOOL   (middleMouseDrivesMovementToggle,  false);
	KEY_BOOL   (mouseLeftAndRightDrivesMovementToggle, false);
	KEY_BOOL   (scrollThroughDefaultActions,      false);
	KEY_BOOL   (offsetCamera,					  true);
	KEY_BOOL   (disableG15Lcd,                    false);
	KEY_BOOL   (showStatusOverIntendedTarget,     true);
	KEY_BOOL   (enableGimbal,					  true);
	KEY_INT	   (currencyFormat,						0);
	KEY_BOOL   (targetArrow,					  true);
	KEY_BOOL   (damagerArrow,					  true);
	KEY_BOOL   (visibleEnemyDamagerArrow,		  true);
	KEY_BOOL   (checkNewVendorExamine,			  true);
	KEY_BOOL   (showQuestHelper,     			  true);
	KEY_BOOL   (showEnteringCombatFlyText,     	  true);
	KEY_BOOL   (showCompletedCollections,         true);
}

//----------------------------------------------------------------------

void ConfigClientUserInterface::remove ()
{
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getChatBubbleFontSize        ()
{
	return ms_chatBubbleFontSize;
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getChatBubbleRangeFactor     ()
{
	return ms_chatBubbleRangeFactor;
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getChatBubbleDurationScale ()
{
	return ms_chatBubbleDurationScale;
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getChatBubbleDurationMin ()
{
	return ms_chatBubbleDurationMin;
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getChatBubbleDurationMax ()
{
	return ms_chatBubbleDurationMax;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getChatBubblesEnabled ()
{
	return ms_chatBubblesEnabled;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getChatBubblesMinimized ()
{
	return ms_chatBubblesMinimized;
}

//-----------------------------------------------------------------

void ConfigClientUserInterface::getReticleDeadZoneSize (int & x, int & y)
{
	x = ms_reticleDeadZoneSizeX;
	y = ms_reticleDeadZoneSizeY;
}

//----------------------------------------------------------------------

int ConfigClientUserInterface::getReticleDeadZoneSizeX     ()
{
	return ms_reticleDeadZoneSizeX;
}

//-----------------------------------------------------------------

float  ConfigClientUserInterface::getCameraInertia ()
{
	return ms_cameraInertia;
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getFirstPersonCameraInertia  ()
{
	return ms_firstPersonameraInertia;
}

//----------------------------------------------------------------------

int ConfigClientUserInterface::getDrawNetworkIds ()
{
	return ms_drawNetworkIds;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawObjectNames ()
{
	return ms_drawObjectNames;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawSelfName ()
{
	return ms_drawSelfName;
}

//-----------------------------------------------------------------

bool ConfigClientUserInterface::getAllowRadialMenuPickup ()
{
	return ms_allowRadialMenuPickup;
}

//----------------------------------------------------------------------

const std::string & ConfigClientUserInterface::getUiRootName ()
{
	return ms_uiRootName;
}

//----------------------------------------------------------------------

const std::string & ConfigClientUserInterface::getUiRootPath ()
{
	return ms_uiRootPath;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getAllowTargetAnything ()
{
	return ms_allowTargetAnything;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDebugExamine ()
{
	return ms_debugExamine;
}

//----------------------------------------------------------------------

int  ConfigClientUserInterface::getConnectionServerPingPeriodMs ()
{
	return ms_connectionServerPingPeriodMs;
}

//----------------------------------------------------------------------

int ConfigClientUserInterface::getConnectionServerPingWindow ()
{
	return ms_connectionServerPingWindow;
}

//----------------------------------------------------------------------

int ConfigClientUserInterface::getSplashTimeoutSeconds ()
{
	return ms_splashTimeoutSeconds;
}

//----------------------------------------------------------------------

const std::string & ConfigClientUserInterface::getIconPathState            ()
{
	return ms_iconPathState;
}

//----------------------------------------------------------------------

const std::string & ConfigClientUserInterface::getIconPathPosture          ()
{
	return ms_iconPathPosture;
}

//----------------------------------------------------------------------

const std::string & ConfigClientUserInterface::getIconPathCommand          ()
{
	return ms_iconPathCommand;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getEnableStartupBugReporting ()
{
	return ms_enableStartupBugReporting;
}

//----------------------------------------------------------------------

int ConfigClientUserInterface::getChatBubblesMaxStackHeight ()
{
	return ms_chatBubblesMaxStackHeight;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getChatBubbleEmotesEnabled ()
{
	return ms_chatBubbleEmotesEnabled;
}

//----------------------------------------------------------------------

const ConfigClientUserInterface::IntVector & ConfigClientUserInterface::getChatWindowFontSizes ()
{
	return ms_chatWindowFontSizes;
}

//----------------------------------------------------------------------

int ConfigClientUserInterface::getChatWindowFontSizeDefaultIndex ()
{
	return ms_chatWindowFontSizeDefaultIndex;
}

//----------------------------------------------------------------------

int ConfigClientUserInterface::getChatWindowFontSizesCount ()
{
	return static_cast<int>(ms_chatWindowFontSizes.size ());
}

//----------------------------------------------------------------------

int ConfigClientUserInterface::getChatWindowFontSizeDefaultSize ()
{
	if (ms_chatWindowFontSizes.empty () || ms_chatWindowFontSizeDefaultIndex < 0 || ms_chatWindowFontSizeDefaultIndex >= getChatWindowFontSizesCount ())
		return 0;

	return ms_chatWindowFontSizes [ms_chatWindowFontSizeDefaultIndex];
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getShowDamageOverHeadOthers ()
{
	return ms_showDamageOverHeadOthers;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getShowDamageOverHeadSelf ()
{
	return ms_showDamageOverHeadSelf;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getShowDamageSnare ()
{
	return ms_showDamageSnare;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getAutoJoinChatRoomOnCreate ()
{
	return ms_autoJoinChatRoomOnCreate;
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getObjectNameRange ()
{
	return ms_objectNameRange;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getAlwaysSetMouseCursor ()
{
	return ms_alwaysSetMouseCursor;
}

// ----------------------------------------------------------------------

float ConfigClientUserInterface::getDamageDelay ()
{
	return ms_damageDelay;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getChatTimestampEnabled ()
{
	return ms_chatTimestampEnabled;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getTestGroupWithNpcs ()
{
	return ms_testGroupWithNpcs;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getTestImageDesignWithNpcs ()
{
	return ms_testImageDesignWithNpcs;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getJoystickMovesPointer ()
{
	return ms_joystickMovesPointer;
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getHudOpacity ()
{
	return ms_hudOpacity;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getConfirmObjDelete ()
{
	return ms_confirmObjDelete;
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getTooltipDelaySecs ()
{
	return ms_tooltipDelaySecs;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getSettingsEnabled ()
{
	return ms_settingsEnabled;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getNetStatusEnabled ()
{
	return ms_netStatusEnabled;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getPointerModeMouseCameraEnabled ()
{
	return ms_pointerModeMouseCameraEnabled;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getMouseModeDefault ()
{
	return ms_mouseModeDefault;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getTurnStrafesDuringMouseModeToggle ()
{
	return ms_turnStrafesDuringMouseModeToggle;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getCanFireSecondariesFromToolbar ()
{
	return ms_canFireSecondariesFromToolbar;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getUseNewbieTutorial ()
{
	return ms_useNewbieTutorial;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getChatBoxKeyClick ()
{
	return ms_chatBoxKeyClick;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getUseExpMonitor ()
{
	return ms_useExpMonitor;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getTargetNothingUntargets ()
{
	return ms_targetNothingUntargets;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getModalChat ()
{
	return ms_modalChat;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getGroundRadarTerrainEnabled ()
{
	return ms_groundRadarTerrainEnabled;
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getObjectNameFontSizeFactor ()
{
	return ms_objectNameFontSizeFactor;
}

//----------------------------------------------------------------------

const std::string & ConfigClientUserInterface::getPaletteName ()
{
	return ms_paletteName;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawObjectNamesPlayers ()
{
	return ms_drawObjectNamesPlayers;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawObjectNamesGroup   ()
{
	return ms_drawObjectNamesGroup;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawObjectNamesNpcs    ()
{
	return ms_drawObjectNamesNpcs;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawObjectNamesGuild   ()
{
	return ms_drawObjectNamesGuild;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawObjectNamesSigns   ()
{
	return ms_drawObjectNamesSigns;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawObjectNamesMyName  ()
{
	return ms_drawObjectNamesMyName;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawObjectNamesBeasts  ()
{
	return ms_drawObjectNamesBeasts;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getFatalOnBadKnowldgeBaseEntry   ()
{
	return ms_fatalOnBadKnowledgeBaseEntry;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getFatalOnBadPoiEntry   ()
{
	return ms_fatalOnBadPoiEntry;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDisplayLoadingBoxes ()
{
	return ms_displayLoadingBoxes;
}

//----------------------------------------------------------------------

std::string const &ConfigClientUserInterface::getMessageOfTheDayTable()
{
	return ms_messageOfTheDayTable;
}

//----------------------------------------------------------------------

float ConfigClientUserInterface::getNumberOfSecondsUntilCuiSave()
{
	return ms_numberOfSecondsUntilCuiSave;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawRadarArrows()
{
	return ms_drawRadarArrows;
}
//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawNpcRadarArrows()
{
	return ms_drawNpcRadarArrows;
}
//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDontDrawNpcsOnRadar()
{
	return ms_dontDrawNpcsOnRadar;
}
//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDontDrawCorpsesOnRadar()
{
	return ms_dontDrawCorpsesOnRadar;
}
//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDontDrawObjectsOnRadar()
{
	return ms_dontDrawObjectsOnRadar;
}
//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDisplayQualifiedForSkill()
{
	return ms_displayQualifiedForSkill;
}
//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawArrowToActiveWaypointOnly()
{
	return ms_drawArrowToActiveWaypointOnly;
}
//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDrawTargetArrow()
{
	return ms_drawTargetArrow;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getMiddleMouseDrivesMovementToggle()
{
	return ms_middleMouseDrivesMovementToggle;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getMouseLeftAndRightDrivesMovementToggle()
{
	return ms_mouseLeftAndRightDrivesMovementToggle;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getScrollThroughDefaultActions()
{
	return ms_scrollThroughDefaultActions;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getOffsetCamera()
{
	return ms_offsetCamera;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getDisableG15Lcd()
{
	return ms_disableG15Lcd;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getShowStatusOverIntendedTarget ()
{
	return ms_showStatusOverIntendedTarget;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getEnableGimbal()
{
	return ms_enableGimbal;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getEnableTargetArrow()
{
	return ms_targetArrow;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getEnableDamagerArrow()
{
	return ms_damagerArrow;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getEnableVisibleEnemyDamagerArrow()
{
	return ms_visibleEnemyDamagerArrow;
}

bool ConfigClientUserInterface::GetEnableNewVendorExamine()
{
	return ms_checkNewVendorExamine;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getShowQuestHelper()
{
	return ms_showQuestHelper;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getShowEnteringCombatFlyText()
{
	return ms_showEnteringCombatFlyText;
}

//----------------------------------------------------------------------

bool ConfigClientUserInterface::getShowCompletedCollections()
{
	return ms_showCompletedCollections;
}

//======================================================================
