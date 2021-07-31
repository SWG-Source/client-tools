//======================================================================
//
// CuiPreferences.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiPreferences.h"

#include "clientAnimation/PriorityPlaybackScriptManager.h"
#include "clientDirectInput/DirectInput.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/IMEManager.h"
#include "clientUserInterface/CuiVoiceChatManager.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/CombatDataTable.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "sharedUtility/LocalMachineOptionManager.h"


#include "UIManager.h"
#include "UINamespace.h"
#include "UIPage.h"
#include "UIPalette.h"
#include "UIUtils.h"



//======================================================================

namespace
{
	bool        ms_isInstalled                      = false;

	float       ms_cameraInertia                    = 0.0f;
	int         ms_drawNetworkIds                   = 0;
	bool        ms_drawObjectNames                  = true;
	bool        ms_locationDisplayEnabled           = true;
	bool        ms_dpsMeterEnabled                  = false;
	bool        ms_drawSelfName                     = false;
	bool        ms_dragOntoContainers               = false;
	bool        ms_allowTargetAnything              = false;
	bool        ms_debugExamine                     = false;
	bool        ms_debugClipboardExamine            = false;
	bool        ms_autoJoinChatRoomOnCreate         = true;
	float       ms_objectNameRange                  = 0.0f;
	float       ms_hudOpacity                       = 0.0f;
	float       ms_flyTextSize                      = 1.0f;
	bool        ms_confirmObjDelete                 = true;
	float       ms_tooltipDelaySecs                 = 0.0f;
	bool        ms_groundRadarTerrainEnabled        = true;
	bool        ms_groundRadarBlinkCombatEnabled    = false;
	bool        ms_showLookAtTargetStatusWindowEnabled = false;
	bool        ms_showStatusOverIntendedTarget     = true;
	bool        ms_useDoubleToolbar                 = false;
	bool		ms_showToolbarCooldownTimer			= false;
	bool        ms_netStatusEnabled                 = false;
	bool        ms_pointerModeMouseCameraEnabled    = true;
	bool        ms_mouseModeDefault[Game::ST_numTypes] = {false, false, false};
	bool        ms_turnStrafesDuringMouseModeToggle = false;
	bool        ms_autoAimToggle                     = false;
	bool        ms_canFireSecondariesFromToolbar    = true;
	bool        ms_modalChat[Game::ST_numTypes]     = {true, true, true};
	bool        ms_useNewbieTutorial                = true;
	bool        ms_useExpMonitor                    = true;
	bool        ms_useWaypointMonitor               = false;
	bool        ms_showGroupWaypoints               = false;
	bool        ms_targetNothingUntargets           = true;
	bool        ms_autoInviteReject                 = false;
	float       ms_firstPersonameraInertia          = 0.0f;
	float       ms_objectNameFontSizeFactor         = 1.0f;
	std::string ms_paletteName                      = "";

	bool        ms_drawObjectNamesPlayers           = false;
	bool        ms_drawObjectNamesGroup             = false;
	bool        ms_drawObjectNamesNpcs              = false;
	bool        ms_drawObjectNamesGuild             = false;
	bool        ms_drawObjectNamesSigns             = false;
	bool        ms_drawObjectNamesMyName            = false;
	bool        ms_drawObjectNamesBeasts            = false;

	bool        ms_showSystemMessages               = true;

	bool        ms_chatAutoEmote                    = true;
	bool        ms_confirmCrafting                  = true;
	bool        ms_showPopupHelp                    = false;

	bool        ms_showIconNames                    = true;
	float       ms_objectIconSize                   = 1.0f;
	float       ms_commandButtonOpacity             = 1.0f;
	float       ms_systemMessageDuration            = 1.0f;
	bool        ms_showWaypointArrowsOnscreen       = false;

	bool        ms_dropShadowUiEnabled              = true;
	bool        ms_dropShadowObjectNamesEnabled     = true;

	bool        ms_rotateMap                        = false;
	bool        ms_rotateInventoryObjects           = true;
	bool        ms_showInterestingAppearance        = true;
	bool        ms_showAFKSpeech                    = true;
	bool        ms_showNPCSpeech                    = true;
	
	bool        ms_screenShake                      = true;
	bool        ms_showObjectArrowsOnRadar          = true;
	bool        ms_showRadarNPCs                    = true;
	bool        ms_useModelessInterface             = false;
	bool        ms_useSwgMouseMap                   = false;

	// Increment defaultUiSettingsVersion to force a reset to latest defaults for all users.
	const int   cms_defaultUiSettingsVersion          = 1;
	int         ms_uiSettingsVersion                  = 0;

	bool        ms_combatSpamVerbose                = false;
	bool        ms_spamShowWeapon                   = true;
	bool		ms_spamShowDamageDetail				= true;
	bool		ms_spamShowArmorAbsorption			= true;
	bool        ms_characterSheetShowDetails        = false;
	int const   ms_combatSpamFilterDefault          = static_cast<int>(CombatDataTable::CSFT_Self);
	int         ms_combatSpamFilter                 = ms_combatSpamFilterDefault;
	int const   ms_combatSpamRangeFilterDefault     = 32;
	int         ms_combatSpamRangeFilter            = ms_combatSpamRangeFilterDefault;

	int const   ms_furnitureRotationDegreeDefault   = 90;
	int         ms_furnitureRotationDegree          = ms_furnitureRotationDegreeDefault;

	// commodities window column sizes
	// SwgCuiAuctionPaneTableModel::ColumnInfo::All and CuiPreferences::ms_cscsAll *MUST* be kept in sync
	int         ms_cscsAllPremium                   = 0;
	int         ms_cscsAllName                      = 0;
	int         ms_cscsAllType                      = 0;
	int         ms_cscsAllBid                       = 0;
	int         ms_cscsAllTime                      = 0;
	int         ms_cscsAllSaleType                  = 0;
	int         ms_cscsAllLocation                  = 0;
	int* const  ms_cscsAll[]                        = {&ms_cscsAllPremium,
	                                                   &ms_cscsAllName,
	                                                   &ms_cscsAllType,
	                                                   &ms_cscsAllBid,
	                                                   &ms_cscsAllTime,
	                                                   &ms_cscsAllSaleType,
	                                                   &ms_cscsAllLocation};

	// SwgCuiAuctionPaneTableModel::ColumnInfo::MySales and CuiPreferences::ms_cscsMySales *MUST* be kept in sync
	int         ms_cscsMySalesPremium               = 0;
	int         ms_cscsMySalesName                  = 0;
	int         ms_cscsMySalesType                  = 0;
	int         ms_cscsMySalesBid                   = 0;
	int         ms_cscsMySalesTime                  = 0;
	int         ms_cscsMySalesSaleType              = 0;
	int         ms_cscsMySalesLocation              = 0;
	int* const  ms_cscsMySales[]                    = {&ms_cscsMySalesPremium,
	                                                   &ms_cscsMySalesName,
	                                                   &ms_cscsMySalesType,
	                                                   &ms_cscsMySalesBid,
	                                                   &ms_cscsMySalesTime,
	                                                   &ms_cscsMySalesSaleType,
	                                                   &ms_cscsMySalesLocation};

	// SwgCuiAuctionPaneTableModel::ColumnInfo::MyBids and CuiPreferences::ms_cscsMyBids *MUST* be kept in sync
	int         ms_cscsMyBidsName                   = 0;
	int         ms_cscsMyBidsType                   = 0;
	int         ms_cscsMyBidsBid                    = 0;
	int         ms_cscsMyBidsTime                   = 0;
	int         ms_cscsMyBidsMyBid                  = 0;
	int         ms_cscsMyBidsMyProxy                = 0;
	int         ms_cscsMyBidsLocation               = 0;
	int* const  ms_cscsMyBids[]                     = {&ms_cscsMyBidsName,
	                                                   &ms_cscsMyBidsType,
	                                                   &ms_cscsMyBidsBid,
	                                                   &ms_cscsMyBidsTime,
	                                                   &ms_cscsMyBidsMyBid,
	                                                   &ms_cscsMyBidsMyProxy,
	                                                   &ms_cscsMyBidsLocation};

	// SwgCuiAuctionPaneTableModel::ColumnInfo::Available and CuiPreferences::ms_cscsAvailable *MUST* be kept in sync
	int         ms_cscsAvailableName                = 0;
	int         ms_cscsAvailableType                = 0;
	int         ms_cscsAvailableTime                = 0;
	int         ms_cscsAvailableLocation            = 0;
	int* const  ms_cscsAvailable[]                  = {&ms_cscsAvailableName,
	                                                   &ms_cscsAvailableType,
	                                                   &ms_cscsAvailableTime,
	                                                   &ms_cscsAvailableLocation};

	// SwgCuiAuctionPaneTableModel::ColumnInfo::VendorSellerSelling and CuiPreferences::ms_cscsVendorSellerSelling *MUST* be kept in sync
	int         ms_cscsVendorSellerSellingName      = 0;
	int         ms_cscsVendorSellerSellingType      = 0;
	int         ms_cscsVendorSellerSellingPrice     = 0;
	int         ms_cscsVendorSellerSellingTime      = 0;
	int         ms_cscsVendorSellerSellingLocation  = 0;
	int* const  ms_cscsVendorSellerSelling[]        = {&ms_cscsVendorSellerSellingName,
	                                                   &ms_cscsVendorSellerSellingType,
	                                                   &ms_cscsVendorSellerSellingPrice,
	                                                   &ms_cscsVendorSellerSellingTime,
	                                                   &ms_cscsVendorSellerSellingLocation};

	// SwgCuiAuctionPaneTableModel::ColumnInfo::VendorBuyerSelling and CuiPreferences::ms_cscsVendorBuyerSelling *MUST* be kept in sync
	int         ms_cscsVendorBuyerSellingName       = 0;
	int         ms_cscsVendorBuyerSellingType       = 0;
	int         ms_cscsVendorBuyerSellingPrice      = 0;
	int* const  ms_cscsVendorBuyerSelling[]         = {&ms_cscsVendorBuyerSellingName,
	                                                   &ms_cscsVendorBuyerSellingType,
	                                                   &ms_cscsVendorBuyerSellingPrice};

	// SwgCuiAuctionPaneTableModel::ColumnInfo::VendorSellerOffers and CuiPreferences::ms_cscsVendorSellerOffers *MUST* be kept in sync
	int         ms_cscsVendorSellerOffersName       = 0;
	int         ms_cscsVendorSellerOffersType       = 0;
	int         ms_cscsVendorSellerOffersPrice      = 0;
	int         ms_cscsVendorSellerOffersOwner      = 0;
	int         ms_cscsVendorSellerOffersLocation   = 0;
	int* const  ms_cscsVendorSellerOffers[]         = {&ms_cscsVendorSellerOffersName,
	                                                   &ms_cscsVendorSellerOffersType,
	                                                   &ms_cscsVendorSellerOffersPrice,
	                                                   &ms_cscsVendorSellerOffersOwner,
	                                                   &ms_cscsVendorSellerOffersLocation};

	// SwgCuiAuctionPaneTableModel::ColumnInfo::VendorBuyerOffers and CuiPreferences::ms_cscsVendorBuyerOffers *MUST* be kept in sync
	int         ms_cscsVendorBuyerOffersName        = 0;
	int         ms_cscsVendorBuyerOffersType        = 0;
	int         ms_cscsVendorBuyerOffersPrice       = 0;
	int         ms_cscsVendorBuyerOffersTime        = 0;
	int         ms_cscsVendorBuyerOffersOwner       = 0;
	int* const  ms_cscsVendorBuyerOffers[]          = {&ms_cscsVendorBuyerOffersName,
	                                                   &ms_cscsVendorBuyerOffersType,
	                                                   &ms_cscsVendorBuyerOffersPrice,
	                                                   &ms_cscsVendorBuyerOffersTime,
	                                                   &ms_cscsVendorBuyerOffersOwner};

	// SwgCuiAuctionPaneTableModel::ColumnInfo::VendorSellerStockroom and CuiPreferences::ms_cscsVendorSellerStockroom *MUST* be kept in sync
	int         ms_cscsVendorSellerStockroomName    = 0;
	int         ms_cscsVendorSellerStockroomType    = 0;
	int         ms_cscsVendorSellerStockroomPrice   = 0;
	int         ms_cscsVendorSellerStockroomTime    = 0;
	int         ms_cscsVendorSellerStockroomLocation= 0;
	int* const  ms_cscsVendorSellerStockroom[]      = {&ms_cscsVendorSellerStockroomName,
	                                                   &ms_cscsVendorSellerStockroomType,
													   &ms_cscsVendorSellerStockroomPrice,
	                                                   &ms_cscsVendorSellerStockroomTime,
	                                                   &ms_cscsVendorSellerStockroomLocation};

	// SwgCuiAuctionPaneTableModel::ColumnInfo::Location and CuiPreferences::ms_cscsLocation *MUST* be kept in sync
	int         ms_cscsLocationName                 = 0;
	int         ms_cscsLocationType                 = 0;
	int         ms_cscsLocationLocation             = 0;
	int         ms_cscsLocationDistanceTo           = 0;
	int         ms_cscsLocationEntranceFee          = 0;
	int         ms_cscsLocationBid                  = 0;
	int * const ms_cscsLocation[]                   = {&ms_cscsLocationName,
	                                                   &ms_cscsLocationType,
	                                                   &ms_cscsLocationLocation,
	                                                   &ms_cscsLocationDistanceTo,
	                                                   &ms_cscsLocationEntranceFee,
	                                                   &ms_cscsLocationBid};

	// SwgCuiAuctionPaneTypes::Type and CuiPreferences::ms_cscs *MUST* be kept in sync
	int * const * const ms_cscs[]                   = {ms_cscsAll,
	                                                   ms_cscsMySales,
	                                                   ms_cscsMyBids,
	                                                   ms_cscsAvailable,
	                                                   ms_cscsVendorSellerSelling,
	                                                   ms_cscsVendorSellerOffers,
	                                                   ms_cscsVendorSellerStockroom,
	                                                   ms_cscsVendorBuyerSelling,
	                                                   ms_cscsVendorBuyerOffers,
	                                                   ms_cscsLocation};

	int const ms_cscsNumColumns[]                   = {sizeof(ms_cscsAll)/sizeof(ms_cscsAll[0]),
	                                                   sizeof(ms_cscsMySales)/sizeof(ms_cscsMySales[0]),
	                                                   sizeof(ms_cscsMyBids)/sizeof(ms_cscsMyBids[0]),
	                                                   sizeof(ms_cscsAvailable)/sizeof(ms_cscsAvailable[0]),
	                                                   sizeof(ms_cscsVendorSellerSelling)/sizeof(ms_cscsVendorSellerSelling[0]),
	                                                   sizeof(ms_cscsVendorSellerOffers)/sizeof(ms_cscsVendorSellerOffers[0]),
	                                                   sizeof(ms_cscsVendorSellerStockroom)/sizeof(ms_cscsVendorSellerStockroom[0]),
	                                                   sizeof(ms_cscsVendorBuyerSelling)/sizeof(ms_cscsVendorBuyerSelling[0]),
	                                                   sizeof(ms_cscsVendorBuyerOffers)/sizeof(ms_cscsVendorBuyerOffers[0]),
	                                                   sizeof(ms_cscsLocation)/sizeof(ms_cscsLocation[0])};

	//-- space stuff
	float       ms_joystickDeadZone                 = 0.1f;
	int         ms_reticleSelect                    = 0;
	int         ms_radarSelect                      = 0;
	int         ms_shipDestroyDetailBias            = 2;
	int         ms_capshipDestroyDetailBias         = 1;
	float       ms_shipDebrisLifespan               = 10.0f;
	float       ms_spaceCameraElasticity = CuiPreferences::getSpaceCameraElasticityDefault();
	float       ms_variableTargetingReticlePercentage = 0.5f;
	bool ms_renderVariableTargetingReticle = false;
	bool ms_useSpaceDebugWindows = false;
	float ms_globalNebulaDensity = CuiPreferences::getGlobalNebulaDensityDefault();
	float ms_globalNebulaRange = 16384.0f;
	bool ms_shipAutolevel = false;
	bool ms_autoSortInventoryContents = true;
	bool ms_autoSortDataPadContents = true;
	int         ms_pilotMouseMode = static_cast<int>(CuiPreferences::PMM_automatic);
	bool ms_joystickInverted = false;
	float ms_joystickSensitivity = 1.0f;
	int ms_povHatMode = static_cast<int>(CuiPreferences::PHM_snap);

	float ms_povHatPanSpeed = 1.0f;
	float ms_cockpitCameraSnapSpeed = 1.0f;
	float ms_cockpitCameraYOffset = 0.0f;
	float ms_cockpitCameraZoomMultiplier = 1.5f;
	bool ms_cockpitCameraRecenterOnShipMovement = false;
	float ms_povHatSnapAngle = convertDegreesToRadians(60.0f);

	bool ms_middleMouseDrivesMovementToggle = false;
	bool ms_mouseLeftAndRightDrivesMovementToggle = false;
	bool ms_scrollThroughDefaultActions = false;

	std::string ms_overrideSpacePalette("Alpha_Blue");
	std::string ms_stylePathSpace = "/styles.space.palette";
	std::string ms_stylePathGround = "/styles.palette";

	Callback * ms_useExpMonitorCallback             = 0;
	Callback * ms_locationDisplayEnabledCallback    = 0;
	Callback * ms_useWaypointMonitorCallback        = 0;
	Callback * ms_showGroupWaypointsCallback        = 0;
	Callback * ms_keybindingsChangedCallback        = 0;
	Callback * ms_objectIconCallback                = 0;
	Callback * ms_commandButtonOpacityCallback      = 0;
	Callback * ms_showInterestingAppearanceCallback = 0;
	Callback * ms_reticleSelectCallback             = 0;
	Callback * ms_radarSelectCallback               = 0;
	Callback * ms_spaceCameraElasticityCallback     = 0;
	Callback * ms_variableTargetingReticlePercentageCallback = 0;
	Callback * ms_paletteChangedCallback            = 0;

	const char * const s_force_prefix       = "force_";
	const size_t       s_force_prefix_len   = strlen (s_force_prefix);

	bool ms_showNotifications = true;
	bool ms_chatBarFadesOut = true;

	bool		ms_offsetCamera						= true;

	bool        ms_collectionShowServerFirst        = false;

	int ms_secondaryTargetMode = CuiPreferences::STM_none;
	bool ms_alwaysShowRangeInGroundRadar = false;

	bool ms_enableGimbal						    = true;

	bool ms_newVendorDoubleClick					= true;

	bool ms_disableAnimationPriorities				= false;

	bool ms_targetArrow								= true;

	bool ms_damagerArrow						    = true;

	bool ms_visibleEnemyDamagerArrow				= true;

	int  ms_currencyFormat							= static_cast<int>(CuiPreferences::CF_none);

	bool ms_showCorpseLootIcon					    = true;
	bool ms_showBackpack							= true;
    bool ms_showHelmet								= true;
	bool ms_showQuestHelper							= true;
	bool ms_showCompletedCollections                = true;

	const static int ms_buffIconSizeSliderMin = 8;
	const static int ms_buffIconSizeSliderMax = 64;

	const static int ms_buffIconSizeStatusDefault          = 22;
	const static int ms_buffIconSizeTargetDefault          = 22;
	const static int ms_buffIconSizeSecondaryTargetDefault = 22;
	const static int ms_buffIconSizeGroupDefault           = 16;
	const static int ms_buffIconSizePetDefault             = 16;

	int         ms_buffIconSizeStatus               = ms_buffIconSizeStatusDefault;
	int         ms_buffIconSizeTarget               = ms_buffIconSizeTargetDefault;
	int         ms_buffIconSizeSecondaryTarget      = ms_buffIconSizeSecondaryTargetDefault;
	int         ms_buffIconSizeGroup                = ms_buffIconSizeGroupDefault;
	int         ms_buffIconSizePet                  = ms_buffIconSizePetDefault;

	float ms_buffIconWhirlygigOpacityDefault = 0.75f;
	float ms_buffIconWhirlygigOpacity = ms_buffIconWhirlygigOpacityDefault;

	Callback * ms_buffIconSettingsChangedCallback = 0;

	// all voice options disabled by default
	bool ms_voiceChatEnabled = false;
	bool ms_voiceUsePushToTalk = false;
	bool ms_voiceUseAdvancedChannelSelection = false;
	bool ms_voiceShowFlybar = false;
	bool ms_voiceAutoDeclineInvites = false;
	bool ms_voiceAutoJoinChannels = false;

	bool ms_defaultExamineHideAppearance = false;
	bool ms_doubleClickAppearanceUnequip = true;
	bool ms_showAppearanceInventory = true;

	float ms_playerCameraHeight = 1.4f;

	bool ms_autoLootCorpses = false;

	float ms_speakerVolume = 0.5f;
	float ms_micVolume = 0.5f;

	float ms_overheadMapOpacity = 1.0f;
	bool  ms_overheadMapShowWaypoints = true;
	bool  ms_overheadMapShowCreatures = true;
	bool  ms_overheadMapShowPlayer = true;
	bool  ms_overheadMapShowLabels = true;
	bool  ms_overheadMapShowBuildings = true;

	bool  ms_hideCharactersOnClosedGalaxies = false;
}

//----------------------------------------------------------------------

const char * const KeyName = "ClientUserInterface";

#define REGISTER_OPTION_USER(a)    (CurrentUserOptionManager::registerOption(ms_ ## a, KeyName, #a))
#define REGISTER_OPTION(a) (LocalMachineOptionManager::registerOption(ms_ ## a, KeyName, #a))


//----------------------------------------------------------------------

void CuiPreferences::install ()
{
	ms_useExpMonitorCallback = new Callback;
	ms_useExpMonitorCallback->fetch ();

	ms_useWaypointMonitorCallback = new Callback;
	ms_useWaypointMonitorCallback->fetch ();

	ms_locationDisplayEnabledCallback = new Callback;
	ms_locationDisplayEnabledCallback->fetch ();

	ms_showGroupWaypointsCallback = new Callback;
	ms_showGroupWaypointsCallback->fetch ();

	ms_keybindingsChangedCallback = new Callback;
	ms_keybindingsChangedCallback->fetch ();

	ms_objectIconCallback = new Callback;
	ms_objectIconCallback->fetch ();

	ms_commandButtonOpacityCallback = new Callback;
	ms_commandButtonOpacityCallback->fetch ();

	ms_showInterestingAppearanceCallback = new Callback;
	ms_showInterestingAppearanceCallback->fetch();

	ms_reticleSelectCallback = new Callback;
	ms_reticleSelectCallback->fetch ();

	ms_radarSelectCallback = new Callback;
	ms_radarSelectCallback->fetch ();

	ms_spaceCameraElasticityCallback = new Callback;
	ms_spaceCameraElasticityCallback->fetch();

	ms_variableTargetingReticlePercentageCallback = new Callback;
	ms_variableTargetingReticlePercentageCallback->fetch();

	ms_paletteChangedCallback = new Callback;
	ms_paletteChangedCallback->fetch();

	ms_buffIconSettingsChangedCallback = new Callback;
	ms_buffIconSettingsChangedCallback->fetch();

	ms_cameraInertia                    = ConfigClientUserInterface::getCameraInertia ();
	ms_firstPersonameraInertia          = ConfigClientUserInterface::getFirstPersonCameraInertia ();
	ms_drawNetworkIds                   = ConfigClientUserInterface::getDrawNetworkIds ();
	ms_drawObjectNames                  = ConfigClientUserInterface::getDrawObjectNames ();
	ms_drawSelfName                     = ConfigClientUserInterface::getDrawSelfName ();
	ms_dragOntoContainers               = false;
	ms_allowTargetAnything              = ConfigClientUserInterface::getAllowTargetAnything ();
	ms_debugExamine                     = ConfigClientUserInterface::getDebugExamine ();
	ms_debugClipboardExamine            = false;
	ms_autoJoinChatRoomOnCreate         = ConfigClientUserInterface::getAutoJoinChatRoomOnCreate ();
	ms_objectNameRange                  = ConfigClientUserInterface::getObjectNameRange ();
	ms_hudOpacity                       = ConfigClientUserInterface::getHudOpacity ();
	ms_flyTextSize                      = 1.0f;
	ms_confirmObjDelete                 = ConfigClientUserInterface::getConfirmObjDelete ();
	ms_tooltipDelaySecs                 = ConfigClientUserInterface::getTooltipDelaySecs ();
	ms_groundRadarTerrainEnabled        = ConfigClientUserInterface::getGroundRadarTerrainEnabled ();
	ms_groundRadarBlinkCombatEnabled    = false;
	ms_showLookAtTargetStatusWindowEnabled = false;
	ms_showStatusOverIntendedTarget     = ConfigClientUserInterface::getShowStatusOverIntendedTarget ();
	ms_useDoubleToolbar                 = false;
	ms_showToolbarCooldownTimer			= false;
	ms_netStatusEnabled                 = ConfigClientUserInterface::getNetStatusEnabled ();
	ms_pointerModeMouseCameraEnabled    = ConfigClientUserInterface::getPointerModeMouseCameraEnabled ();
	ms_mouseModeDefault[Game::ST_ground] = ConfigClientUserInterface::getMouseModeDefault ();
	ms_mouseModeDefault[Game::ST_space] = false;
	ms_turnStrafesDuringMouseModeToggle = ConfigClientUserInterface::getTurnStrafesDuringMouseModeToggle ();
	ms_autoAimToggle                     = false;
	ms_enableGimbal						 = true;
	ms_newVendorDoubleClick				 = true;
	ms_disableAnimationPriorities	     = false;
	ms_targetArrow						 = true;
	ms_damagerArrow						 = true;
	ms_visibleEnemyDamagerArrow          = true;
	
	ms_canFireSecondariesFromToolbar     = ConfigClientUserInterface::getCanFireSecondariesFromToolbar ();
	ms_middleMouseDrivesMovementToggle = ConfigClientUserInterface::getMiddleMouseDrivesMovementToggle();
	ms_mouseLeftAndRightDrivesMovementToggle = ConfigClientUserInterface::getMouseLeftAndRightDrivesMovementToggle();
	ms_scrollThroughDefaultActions = ConfigClientUserInterface::getScrollThroughDefaultActions();

	ms_useNewbieTutorial                = ConfigClientUserInterface::getUseNewbieTutorial ();
	ms_useExpMonitor                    = true; // ConfigClientUserInterface::getUseExpMonitor ();
	ms_locationDisplayEnabled           = true;
	ms_dpsMeterEnabled                  = false;
	ms_useWaypointMonitor               = false;
	ms_showGroupWaypoints               = false;
	ms_targetNothingUntargets           = ConfigClientUserInterface::getTargetNothingUntargets ();
	ms_autoInviteReject                 = ConfigClientGame::getAutoInviteReject ();
	ms_objectNameFontSizeFactor         = ConfigClientUserInterface::getObjectNameFontSizeFactor ();
	ms_paletteName                      = ConfigClientUserInterface::getPaletteName ();

	ms_drawObjectNamesPlayers           = ConfigClientUserInterface::getDrawObjectNamesPlayers ();
	ms_drawObjectNamesGroup             = ConfigClientUserInterface::getDrawObjectNamesGroup ();
	ms_drawObjectNamesNpcs              = ConfigClientUserInterface::getDrawObjectNamesNpcs ();
	ms_drawObjectNamesGuild             = ConfigClientUserInterface::getDrawObjectNamesGuild ();
	ms_drawObjectNamesSigns             = ConfigClientUserInterface::getDrawObjectNamesSigns ();
	ms_drawObjectNamesMyName            = ConfigClientUserInterface::getDrawObjectNamesMyName ();
	ms_drawObjectNamesBeasts            = ConfigClientUserInterface::getDrawObjectNamesBeasts ();
	ms_showSystemMessages               = true;
	ms_chatAutoEmote                    = true;
	ms_confirmCrafting                  = true;
	ms_showIconNames                    = true;
	ms_objectIconSize                   = 1.0f;
	ms_systemMessageDuration            = 1.0f;
	ms_showWaypointArrowsOnscreen       = false;
	ms_dropShadowUiEnabled              = true;
	ms_dropShadowObjectNamesEnabled     = true;
	ms_rotateMap                        = false;
	ms_rotateInventoryObjects           = true;
	ms_showInterestingAppearance        = true;
	ms_showAFKSpeech                    = true;
	ms_showNPCSpeech                    = true;
	ms_screenShake                      = true;
	ms_reticleSelect                    = 0;
	ms_radarSelect                      = 0;
	ms_showObjectArrowsOnRadar          = true;
	ms_showRadarNPCs                    = true;

	ms_combatSpamVerbose                = false;
	ms_spamShowWeapon                   = true;
	ms_spamShowDamageDetail             = true;
	ms_spamShowArmorAbsorption          = true;
	ms_combatSpamFilter                 = ms_combatSpamFilterDefault;
	ms_combatSpamRangeFilter            = ms_combatSpamRangeFilterDefault;

	ms_furnitureRotationDegree          = ms_furnitureRotationDegreeDefault;

	ms_characterSheetShowDetails        = false;
	ms_showNotifications                = true;
	ms_chatBarFadesOut                  = true;

	// commodities window column sizes
	ms_cscsAllPremium                   = 0;
	ms_cscsAllName                      = 0;
	ms_cscsAllType                      = 0;
	ms_cscsAllBid                       = 0;
	ms_cscsAllTime                      = 0;
	ms_cscsAllSaleType                  = 0;
	ms_cscsAllLocation                  = 0;
	ms_cscsMySalesPremium               = 0;
	ms_cscsMySalesName                  = 0;
	ms_cscsMySalesType                  = 0;
	ms_cscsMySalesBid                   = 0;
	ms_cscsMySalesTime                  = 0;
	ms_cscsMySalesSaleType              = 0;
	ms_cscsMySalesLocation              = 0;
	ms_cscsMyBidsName                   = 0;
	ms_cscsMyBidsType                   = 0;
	ms_cscsMyBidsBid                    = 0;
	ms_cscsMyBidsTime                   = 0;
	ms_cscsMyBidsMyBid                  = 0;
	ms_cscsMyBidsMyProxy                = 0;
	ms_cscsMyBidsLocation               = 0;
	ms_cscsAvailableName                = 0;
	ms_cscsAvailableType                = 0;
	ms_cscsAvailableTime                = 0;
	ms_cscsAvailableLocation            = 0;
	ms_cscsVendorSellerSellingName      = 0;
	ms_cscsVendorSellerSellingType      = 0;
	ms_cscsVendorSellerSellingPrice     = 0;
	ms_cscsVendorSellerSellingTime      = 0;
	ms_cscsVendorSellerSellingLocation  = 0;
	ms_cscsVendorBuyerSellingName       = 0;
	ms_cscsVendorBuyerSellingType       = 0;
	ms_cscsVendorBuyerSellingPrice      = 0;
	ms_cscsVendorSellerOffersName       = 0;
	ms_cscsVendorSellerOffersType       = 0;
	ms_cscsVendorSellerOffersPrice      = 0;
	ms_cscsVendorSellerOffersOwner      = 0;
	ms_cscsVendorSellerOffersLocation   = 0;
	ms_cscsVendorBuyerOffersName        = 0;
	ms_cscsVendorBuyerOffersType        = 0;
	ms_cscsVendorBuyerOffersPrice       = 0;
	ms_cscsVendorBuyerOffersTime        = 0;
	ms_cscsVendorBuyerOffersOwner       = 0;
	ms_cscsVendorSellerStockroomName    = 0;
	ms_cscsVendorSellerStockroomType    = 0;
	ms_cscsVendorSellerStockroomPrice   = 0;
	ms_cscsVendorSellerStockroomTime    = 0;
	ms_cscsVendorSellerStockroomLocation= 0;
	ms_cscsLocationName                 = 0;
	ms_cscsLocationType                 = 0;
	ms_cscsLocationLocation             = 0;
	ms_cscsLocationDistanceTo           = 0;
	ms_cscsLocationEntranceFee          = 0;
	ms_cscsLocationBid                  = 0;

	ms_spaceCameraElasticity = 0.3f;
	ms_shipAutolevel = ConfigClientGame::getShipAutolevelDefault();

	ms_offsetCamera						= ConfigClientUserInterface::getOffsetCamera();

	ms_collectionShowServerFirst        = false;

	ms_secondaryTargetMode = STM_none;
	ms_alwaysShowRangeInGroundRadar = false;

	ms_showCorpseLootIcon = true;

	ms_showBackpack = true;
	ms_showHelmet = true;
	ms_buffIconWhirlygigOpacity = getBuffIconWhirlygigOpacityDefault();

	REGISTER_OPTION(cameraInertia);
	REGISTER_OPTION(firstPersonameraInertia);
	REGISTER_OPTION(drawNetworkIds);
	REGISTER_OPTION(drawObjectNames);
	REGISTER_OPTION(drawSelfName);
	REGISTER_OPTION(dragOntoContainers);
	REGISTER_OPTION(allowTargetAnything);
	REGISTER_OPTION(debugExamine);
	REGISTER_OPTION(debugClipboardExamine);
	REGISTER_OPTION(autoJoinChatRoomOnCreate);
	REGISTER_OPTION(objectNameRange);
	REGISTER_OPTION(confirmObjDelete);
	REGISTER_OPTION(tooltipDelaySecs);
	REGISTER_OPTION(groundRadarTerrainEnabled);
	REGISTER_OPTION(groundRadarBlinkCombatEnabled);
	REGISTER_OPTION(showLookAtTargetStatusWindowEnabled);

	if (ms_showLookAtTargetStatusWindowEnabled)
	{
		ms_secondaryTargetMode = STM_lookAtTarget;
		ms_showLookAtTargetStatusWindowEnabled = false;
	}

	REGISTER_OPTION(showStatusOverIntendedTarget);
	REGISTER_OPTION(useDoubleToolbar);
	REGISTER_OPTION(showToolbarCooldownTimer);
	REGISTER_OPTION(netStatusEnabled);
	REGISTER_OPTION(pointerModeMouseCameraEnabled);
	REGISTER_OPTION(useNewbieTutorial);
	REGISTER_OPTION(targetNothingUntargets);
	REGISTER_OPTION(autoInviteReject);
	REGISTER_OPTION(objectNameFontSizeFactor);
	REGISTER_OPTION(drawObjectNamesPlayers);
	REGISTER_OPTION(drawObjectNamesGroup);
	REGISTER_OPTION(drawObjectNamesNpcs);
	REGISTER_OPTION(drawObjectNamesGuild);
	REGISTER_OPTION(drawObjectNamesSigns);
	REGISTER_OPTION(drawObjectNamesMyName);
	REGISTER_OPTION(drawObjectNamesBeasts);
	REGISTER_OPTION(showSystemMessages);
	REGISTER_OPTION(chatAutoEmote);
	REGISTER_OPTION(confirmCrafting);
	//REGISTER_OPTION(showPopupHelp);
	REGISTER_OPTION(showIconNames);
	REGISTER_OPTION(objectIconSize);
	REGISTER_OPTION(systemMessageDuration);
	LocalMachineOptionManager::registerOption(ms_useWaypointMonitor, KeyName, "useWaypointMonitor", 1);
	REGISTER_OPTION(locationDisplayEnabled);
	REGISTER_OPTION(dpsMeterEnabled);
	REGISTER_OPTION(combatSpamVerbose);
	REGISTER_OPTION(spamShowWeapon);
	REGISTER_OPTION(spamShowDamageDetail);
	REGISTER_OPTION(spamShowArmorAbsorption);
	REGISTER_OPTION(combatSpamFilter);
	REGISTER_OPTION(combatSpamRangeFilter);
	REGISTER_OPTION(furnitureRotationDegree);

	// commodities window column sizes
	REGISTER_OPTION(cscsAllPremium);
	REGISTER_OPTION(cscsAllName);
	REGISTER_OPTION(cscsAllType);
	REGISTER_OPTION(cscsAllBid);
	REGISTER_OPTION(cscsAllTime);
	REGISTER_OPTION(cscsAllSaleType);
	REGISTER_OPTION(cscsAllLocation);
	REGISTER_OPTION(cscsMySalesPremium);
	REGISTER_OPTION(cscsMySalesName);
	REGISTER_OPTION(cscsMySalesType);
	REGISTER_OPTION(cscsMySalesBid);
	REGISTER_OPTION(cscsMySalesTime);
	REGISTER_OPTION(cscsMySalesSaleType);
	REGISTER_OPTION(cscsMySalesLocation);
	REGISTER_OPTION(cscsMyBidsName);
	REGISTER_OPTION(cscsMyBidsType);
	REGISTER_OPTION(cscsMyBidsBid);
	REGISTER_OPTION(cscsMyBidsTime);
	REGISTER_OPTION(cscsMyBidsMyBid);
	REGISTER_OPTION(cscsMyBidsMyProxy);
	REGISTER_OPTION(cscsMyBidsLocation);
	REGISTER_OPTION(cscsAvailableName);
	REGISTER_OPTION(cscsAvailableType);
	REGISTER_OPTION(cscsAvailableTime);
	REGISTER_OPTION(cscsAvailableLocation);
	REGISTER_OPTION(cscsVendorSellerSellingName);
	REGISTER_OPTION(cscsVendorSellerSellingType);
	REGISTER_OPTION(cscsVendorSellerSellingPrice);
	REGISTER_OPTION(cscsVendorSellerSellingTime);
	REGISTER_OPTION(cscsVendorSellerSellingLocation);
	REGISTER_OPTION(cscsVendorBuyerSellingName);
	REGISTER_OPTION(cscsVendorBuyerSellingType);
	REGISTER_OPTION(cscsVendorBuyerSellingPrice);
	REGISTER_OPTION(cscsVendorSellerOffersName);
	REGISTER_OPTION(cscsVendorSellerOffersType);
	REGISTER_OPTION(cscsVendorSellerOffersPrice);
	REGISTER_OPTION(cscsVendorSellerOffersOwner);
	REGISTER_OPTION(cscsVendorSellerOffersLocation);
	REGISTER_OPTION(cscsVendorBuyerOffersName);
	REGISTER_OPTION(cscsVendorBuyerOffersType);
	REGISTER_OPTION(cscsVendorBuyerOffersPrice);
	REGISTER_OPTION(cscsVendorBuyerOffersTime);
	REGISTER_OPTION(cscsVendorBuyerOffersOwner);
	REGISTER_OPTION(cscsVendorSellerStockroomName);
	REGISTER_OPTION(cscsVendorSellerStockroomType);
	REGISTER_OPTION(cscsVendorSellerStockroomPrice);
	REGISTER_OPTION(cscsVendorSellerStockroomTime);
	REGISTER_OPTION(cscsVendorSellerStockroomLocation);
	REGISTER_OPTION(cscsLocationName);
	REGISTER_OPTION(cscsLocationType);
	REGISTER_OPTION(cscsLocationLocation);
	REGISTER_OPTION(cscsLocationDistanceTo);
	REGISTER_OPTION(cscsLocationEntranceFee);
	REGISTER_OPTION(cscsLocationBid);

#if 0
	REGISTER_OPTION(showGroupWaypoints);
#endif
	REGISTER_OPTION(showWaypointArrowsOnscreen);
	REGISTER_OPTION(dropShadowUiEnabled);
	REGISTER_OPTION(dropShadowObjectNamesEnabled);
	REGISTER_OPTION(rotateMap);
	REGISTER_OPTION(rotateInventoryObjects);
	REGISTER_OPTION(showInterestingAppearance);
	REGISTER_OPTION(showAFKSpeech);
	REGISTER_OPTION(showNPCSpeech);
	REGISTER_OPTION(screenShake);
	REGISTER_OPTION(reticleSelect);
	REGISTER_OPTION(radarSelect);
	REGISTER_OPTION(spaceCameraElasticity);
	REGISTER_OPTION(shipDestroyDetailBias);
	REGISTER_OPTION(shipDebrisLifespan);
	REGISTER_OPTION(globalNebulaDensity);
	REGISTER_OPTION(globalNebulaRange);
	REGISTER_OPTION(shipAutolevel);
	REGISTER_OPTION(autoSortInventoryContents);
	REGISTER_OPTION(autoSortDataPadContents);
	REGISTER_OPTION(pilotMouseMode);
	REGISTER_OPTION(joystickInverted);
	REGISTER_OPTION(joystickSensitivity);
	REGISTER_OPTION(joystickDeadZone);
	REGISTER_OPTION(povHatMode);
	REGISTER_OPTION(povHatPanSpeed);
	REGISTER_OPTION(cockpitCameraSnapSpeed);
	REGISTER_OPTION(cockpitCameraYOffset);
	REGISTER_OPTION(cockpitCameraZoomMultiplier);
	REGISTER_OPTION(cockpitCameraRecenterOnShipMovement);
	REGISTER_OPTION(povHatSnapAngle);

	REGISTER_OPTION_USER(useModelessInterface);
	REGISTER_OPTION_USER(showObjectArrowsOnRadar);
	REGISTER_OPTION_USER(showRadarNPCs);
	REGISTER_OPTION_USER(useSwgMouseMap);

	REGISTER_OPTION_USER(paletteName);
	REGISTER_OPTION_USER(hudOpacity);
	REGISTER_OPTION_USER(flyTextSize);
	REGISTER_OPTION_USER(modalChat[Game::ST_ground]);
	REGISTER_OPTION_USER(modalChat[Game::ST_space]);
	REGISTER_OPTION_USER(mouseModeDefault[Game::ST_ground]);
	REGISTER_OPTION_USER(mouseModeDefault[Game::ST_space]);
	REGISTER_OPTION_USER(turnStrafesDuringMouseModeToggle);
	REGISTER_OPTION_USER(autoAimToggle);
	REGISTER_OPTION_USER(canFireSecondariesFromToolbar);
	REGISTER_OPTION_USER(middleMouseDrivesMovementToggle);
	REGISTER_OPTION_USER(mouseLeftAndRightDrivesMovementToggle);
	REGISTER_OPTION_USER(scrollThroughDefaultActions);
	CurrentUserOptionManager::registerOption(ms_useExpMonitor, KeyName, "useExpMonitor", 1);

	REGISTER_OPTION(overrideSpacePalette);

	REGISTER_OPTION_USER(spaceCameraElasticity);
	REGISTER_OPTION_USER(variableTargetingReticlePercentage);
	REGISTER_OPTION_USER(renderVariableTargetingReticle);

	REGISTER_OPTION_USER(uiSettingsVersion);

	REGISTER_OPTION(showNotifications);
	REGISTER_OPTION(chatBarFadesOut);

	REGISTER_OPTION_USER(offsetCamera);

	REGISTER_OPTION(collectionShowServerFirst);

	REGISTER_OPTION(secondaryTargetMode);
	REGISTER_OPTION(alwaysShowRangeInGroundRadar);
	REGISTER_OPTION(enableGimbal);
	REGISTER_OPTION(targetArrow);
	REGISTER_OPTION(damagerArrow);
	REGISTER_OPTION(visibleEnemyDamagerArrow);

	REGISTER_OPTION(currencyFormat);

	REGISTER_OPTION(newVendorDoubleClick);
	REGISTER_OPTION(disableAnimationPriorities);
	REGISTER_OPTION(showCorpseLootIcon);
	REGISTER_OPTION(showBackpack);
	REGISTER_OPTION(showHelmet);
	REGISTER_OPTION(showQuestHelper);
	REGISTER_OPTION(showCompletedCollections);

	REGISTER_OPTION(buffIconSizeStatus);
	REGISTER_OPTION(buffIconSizeTarget);
	REGISTER_OPTION(buffIconSizeSecondaryTarget);
	REGISTER_OPTION(buffIconSizePet);
	REGISTER_OPTION(buffIconSizeGroup);

	REGISTER_OPTION(buffIconWhirlygigOpacity);

	REGISTER_OPTION_USER(voiceChatEnabled);
	REGISTER_OPTION_USER(voiceUsePushToTalk);
	REGISTER_OPTION_USER(voiceShowFlybar);
	REGISTER_OPTION_USER(voiceUseAdvancedChannelSelection);
	REGISTER_OPTION_USER(voiceAutoDeclineInvites);
	REGISTER_OPTION_USER(voiceAutoJoinChannels);

	REGISTER_OPTION(defaultExamineHideAppearance);
	REGISTER_OPTION(doubleClickAppearanceUnequip);
	REGISTER_OPTION(showAppearanceInventory);

	REGISTER_OPTION(playerCameraHeight);

	REGISTER_OPTION(autoLootCorpses);

	REGISTER_OPTION(speakerVolume);
	REGISTER_OPTION(micVolume);

	REGISTER_OPTION(overheadMapOpacity);
	REGISTER_OPTION(overheadMapShowWaypoints);
	REGISTER_OPTION(overheadMapShowCreatures);
	REGISTER_OPTION(overheadMapShowPlayer);
	REGISTER_OPTION(overheadMapShowLabels);
	REGISTER_OPTION(overheadMapShowBuildings);

	REGISTER_OPTION(hideCharactersOnClosedGalaxies);

	// disable all voice preferences by default
	CuiVoiceChatManager::setVoiceChatEnabled(false);
	CuiVoiceChatManager::setUsePushToTalkForceUpdate(false);
	CuiVoiceChatManager::setShowFlybar(false);
	CuiVoiceChatManager::setUseAdvancedChannelSelection(false);

	// Update our utils class with our loaded option.
	setCurrencyFormat(static_cast<CuiPreferences::CurrencyFormat>(ms_currencyFormat));
	
	// Update the config setting
	setDisableAnimationPriorities(ms_disableAnimationPriorities);

	// The preferences are installed
	ms_isInstalled = true;
}

//----------------------------------------------------------------------

void CuiPreferences::remove ()
{
	// The preferences can no longer be considered installed...
	// (this gives us a quick test as to whether the callbacks still exist)
	ms_isInstalled = false;

	ms_useExpMonitorCallback->release ();
	ms_useExpMonitorCallback = 0;

	ms_locationDisplayEnabledCallback->release ();
	ms_locationDisplayEnabledCallback = 0;

	ms_useWaypointMonitorCallback->release ();
	ms_useWaypointMonitorCallback = 0;

	ms_showGroupWaypointsCallback->release ();
	ms_showGroupWaypointsCallback = 0;

	ms_keybindingsChangedCallback->release ();
	ms_keybindingsChangedCallback = 0;

	ms_objectIconCallback->release ();
	ms_objectIconCallback = 0;

	ms_commandButtonOpacityCallback->release ();
	ms_commandButtonOpacityCallback = 0;

	ms_showInterestingAppearanceCallback->release();
	ms_showInterestingAppearanceCallback = 0;

	ms_reticleSelectCallback->release();
	ms_reticleSelectCallback = 0;

	ms_radarSelectCallback->release();
	ms_radarSelectCallback = 0;

	ms_spaceCameraElasticityCallback->release();
	ms_spaceCameraElasticityCallback = 0;

	ms_variableTargetingReticlePercentageCallback->release();
	ms_variableTargetingReticlePercentageCallback = 0;

	ms_paletteChangedCallback->release();
	ms_paletteChangedCallback = 0;

	ms_buffIconSettingsChangedCallback->release ();
	ms_buffIconSettingsChangedCallback = 0;
}

//----------------------------------------------------------------------

bool CuiPreferences::isInstalled ()
{
	return ms_isInstalled;
}

//----------------------------------------------------------------------

void CuiPreferences::setCameraInertia           (float f)
{
	ms_cameraInertia = f;
	CuiManager::setCameraInertia (ms_cameraInertia);
}

//----------------------------------------------------------------------

void CuiPreferences::setDrawNetworkIds (int drawNetworkIds)
{
	ms_drawNetworkIds = drawNetworkIds;
}

//----------------------------------------------------------------------

void CuiPreferences::setDrawObjectNames (bool drawObjectNames)
{
	ms_drawObjectNames = drawObjectNames;
}

//----------------------------------------------------------------------

void CuiPreferences::setDrawSelfName (bool drawSelfName)
{
	ms_drawSelfName = drawSelfName;
}

//----------------------------------------------------------------------

void CuiPreferences::setDragOntoContainers (bool b)
{
	ms_dragOntoContainers = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setDrawObjectNamesPlayers (bool drawObjectNames)
{
	ms_drawObjectNamesPlayers = drawObjectNames;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDrawObjectNamesPlayers ()
{
	return ms_drawObjectNamesPlayers;
}

//----------------------------------------------------------------------

void CuiPreferences::setDrawObjectNamesGroup (bool drawObjectNames)
{
	ms_drawObjectNamesGroup = drawObjectNames;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDrawObjectNamesGroup ()
{
	return ms_drawObjectNamesGroup;
}

//----------------------------------------------------------------------

void CuiPreferences::setDrawObjectNamesNpcs (bool drawObjectNames)
{
	ms_drawObjectNamesNpcs = drawObjectNames;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDrawObjectNamesNpcs ()
{
	return ms_drawObjectNamesNpcs;
}

//----------------------------------------------------------------------

void CuiPreferences::setDrawObjectNamesGuild (bool drawObjectNames)
{
	ms_drawObjectNamesGuild = drawObjectNames;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDrawObjectNamesGuild ()
{
	return ms_drawObjectNamesGuild;
}

//----------------------------------------------------------------------

void CuiPreferences::setDrawObjectNamesSigns (bool drawObjectNames)
{
	ms_drawObjectNamesSigns = drawObjectNames;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDrawObjectNamesSigns ()
{
	return ms_drawObjectNamesSigns;
}

//----------------------------------------------------------------------

void CuiPreferences::setDrawObjectNamesMyName (bool drawObjectNames)
{
	ms_drawObjectNamesMyName = drawObjectNames;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDrawObjectNamesMyName ()
{
	return ms_drawObjectNamesMyName;
}

//----------------------------------------------------------------------

void CuiPreferences::setDrawObjectNamesBeasts (bool drawObjectNames)
{
	ms_drawObjectNamesBeasts = drawObjectNames;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDrawObjectNamesBeasts ()
{
	return ms_drawObjectNamesBeasts;
}

//----------------------------------------------------------------------

void CuiPreferences::setAllowTargetAnything (bool b)
{
	ms_allowTargetAnything = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setDebugExamine (bool b)
{
	ms_debugExamine = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setDebugClipboardExamine (bool b)
{
	ms_debugClipboardExamine = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setAutoJoinChatRoomOnCreate (bool b)
{
	ms_autoJoinChatRoomOnCreate = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setObjectNameRange (float f)
{
	ms_objectNameRange = f;
}

//----------------------------------------------------------------------

void CuiPreferences::setHudOpacity (float f)
{
	ms_hudOpacity = f;
}

//----------------------------------------------------------------------

void CuiPreferences::setFlyTextSize (float f)
{
	ms_flyTextSize = f;
}

//----------------------------------------------------------------------

void CuiPreferences::setConfirmObjDelete (bool b)
{
	ms_confirmObjDelete = b;
}


//----------------------------------------------------------------------

void  CuiPreferences::setTooltipDelaySecs (float f)
{
	ms_tooltipDelaySecs = f;
	UIManager::gUIManager ().SetTooltipDelaySecs (f);
}

//----------------------------------------------------------------------

void CuiPreferences::setGroundRadarTerrainEnabled (bool b)
{
	ms_groundRadarTerrainEnabled = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setGroundRadarBlinkCombatEnabled (bool b)
{
	ms_groundRadarBlinkCombatEnabled = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowLookAtTargetStatusWindowEnabled (bool b)
{
	ms_showLookAtTargetStatusWindowEnabled = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowStatusOverIntendedTarget (bool b)
{
	ms_showStatusOverIntendedTarget = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setUseDoubleToolbar (bool b)
{
	ms_useDoubleToolbar = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowToolbarCooldownTimer (bool b)
{
	ms_showToolbarCooldownTimer = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setNetStatusEnabled (bool b)
{
	if ((ms_netStatusEnabled && !b) || (!ms_netStatusEnabled && b))
	{
		ms_netStatusEnabled = b;
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::netStatus, Unicode::narrowToWide (b ? "1" : "0")));
	}
}

//----------------------------------------------------------------------

void CuiPreferences::setPointerModeMouseCameraEnabled (bool b)
{
	ms_pointerModeMouseCameraEnabled = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setMouseModeDefault (bool b)
{
	int const sceneType = Game::getHudSceneType();
	
	if (sceneType < 0 || sceneType >= Game::ST_numTypes)
	{
		WARNING(true, ("CuiPreferences::setMouseModeDefault invalid scene type [%d]", sceneType));
		return;
	}

	ms_mouseModeDefault[sceneType] = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setTurnStrafesDuringMouseModeToggle (bool b)
{
	ms_turnStrafesDuringMouseModeToggle = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setAutoAimToggle (bool b)
{
	ms_autoAimToggle = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setEnableGimbal (bool b)
{
	ms_enableGimbal = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setTargetArrow (bool b)
{
	ms_targetArrow = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setDamagerArrow (bool b)
{
	ms_damagerArrow = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setVisibleEnemyDamagerArrow (bool b)
{
	ms_visibleEnemyDamagerArrow = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setCurrencyFormat(CurrencyFormat cf)
{
	ms_currencyFormat = cf;

	// Update the UI Utils
	switch(cf)
	{
	case CuiPreferences::CF_none:
		UIUtils::SetDigitGroupingSymbol('\0');
		break;
	case CuiPreferences::CF_comma:
		UIUtils::SetDigitGroupingSymbol(',');
		break;
	case CuiPreferences::CF_period:
		UIUtils::SetDigitGroupingSymbol('.');
		break;
	case CuiPreferences::CF_space:
		UIUtils::SetDigitGroupingSymbol(' ');
		break;
	default:
		UIUtils::SetDigitGroupingSymbol('\0');
		break;
	}
}

//----------------------------------------------------------------------

void CuiPreferences::setCanFireSecondariesFromToolbar (bool b)
{
	ms_canFireSecondariesFromToolbar = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setModalChat (bool b)
{
	int const sceneType = Game::getHudSceneType();
	
	if (sceneType < 0 || sceneType >= Game::ST_numTypes)
	{
		WARNING(true, ("CuiPreferences::setModalChat invalid scene type [%d]", sceneType));
		return;
	}

	ms_modalChat[sceneType] = b;
	IMEManager::ClearCompositionString();
}

//----------------------------------------------------------------------

void CuiPreferences::setUseNewbieTutorial (bool b)
{
	ms_useNewbieTutorial = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setUseExpMonitor (bool b)
{
	const bool changed = (ms_useExpMonitor && !b) || (!ms_useExpMonitor && b);

	if (changed)
	{
		ms_useExpMonitor = b;
		ms_useExpMonitorCallback->performCallback ();
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::expMonitor, Unicode::emptyString));
		CurrentUserOptionManager::save ();
		LocalMachineOptionManager::save ();
	}
}

//----------------------------------------------------------------------

void CuiPreferences::setUseWaypointMonitor (bool b)
{
	const bool changed = (ms_useWaypointMonitor && !b) || (!ms_useWaypointMonitor && b);

	if (changed)
	{
		ms_useWaypointMonitor = b;
		ms_useWaypointMonitorCallback->performCallback ();
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::waypointMonitor, Unicode::emptyString));
		CurrentUserOptionManager::save ();
		LocalMachineOptionManager::save ();
	}
}

//----------------------------------------------------------------------

void CuiPreferences::setLocationDisplayEnabled(bool b)
{
	const bool changed = (ms_locationDisplayEnabled && !b) || (!ms_locationDisplayEnabled && b);

	if (changed)
	{
		ms_locationDisplayEnabled = b;
		ms_locationDisplayEnabledCallback->performCallback ();
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::locationDisplay, Unicode::emptyString));
		CurrentUserOptionManager::save ();
		LocalMachineOptionManager::save ();
	}
}

//----------------------------------------------------------------------

void CuiPreferences::setDpsMeterEnabled(bool b)
{
	const bool changed = (ms_dpsMeterEnabled && !b) || (!ms_dpsMeterEnabled && b);

	if (changed)
	{
		ms_dpsMeterEnabled = b;
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::dpsMeter, Unicode::emptyString));
		CurrentUserOptionManager::save ();
		LocalMachineOptionManager::save ();
	}
}

//----------------------------------------------------------------------

void CuiPreferences::setShowGroupWaypoints(bool b)
{
	const bool changed = (ms_showGroupWaypoints && !b) || (!ms_showGroupWaypoints && b);

	if (changed)
	{
		ms_showGroupWaypoints = b;
		ms_showGroupWaypointsCallback->performCallback ();
		CurrentUserOptionManager::save ();
		LocalMachineOptionManager::save ();
	}
}

//----------------------------------------------------------------------

void CuiPreferences::setTargetNothingUntargets (bool b)
{
	ms_targetNothingUntargets = b;
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getUseExpMonitorCallback   ()
{
	return *NON_NULL (ms_useExpMonitorCallback);
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getUseWaypointMonitorCallback   ()
{
	return *NON_NULL (ms_useWaypointMonitorCallback);
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getLocationDisplayEnabledCallback   ()
{
	return *NON_NULL (ms_locationDisplayEnabledCallback);
}
//----------------------------------------------------------------------

Callback & CuiPreferences::getShowGroupWaypointsCallback   ()
{
	return *NON_NULL (ms_showGroupWaypointsCallback);
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getKeybindingsChangedCallback   ()
{
	return *NON_NULL (ms_keybindingsChangedCallback);
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getShowInterestingAppearanceCallback()
{
	return *NON_NULL(ms_showInterestingAppearanceCallback);
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getReticleSelectCallback   ()
{
	return *NON_NULL (ms_reticleSelectCallback);
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getRadarSelectCallback   ()
{
	return *NON_NULL (ms_radarSelectCallback);
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getSpaceCameraElasticityCallback()
{
	return *NON_NULL (ms_spaceCameraElasticityCallback);
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getVariableTargetingReticlePercentageCallback()
{
	return *NON_NULL(ms_variableTargetingReticlePercentageCallback);
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getPaletteChangedCallback()
{
	return *NON_NULL(ms_paletteChangedCallback);
}

//----------------------------------------------------------------------

void CuiPreferences::signalKeybindingsChanged ()
{
	ms_useExpMonitorCallback->performCallback ();
	ms_keybindingsChangedCallback->performCallback();
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------


float CuiPreferences::getHudOpacity ()
{
	return ms_hudOpacity;
}

//----------------------------------------------------------------------

float CuiPreferences::getFlyTextSize ()
{
	return ms_flyTextSize;
}

//----------------------------------------------------------------------

float  CuiPreferences::getTooltipDelaySecs ()
{
	return ms_tooltipDelaySecs;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getGroundRadarTerrainEnabled ()
{
	return ms_groundRadarTerrainEnabled;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getGroundRadarBlinkCombatEnabled ()
{
	return ms_groundRadarBlinkCombatEnabled;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getShowLookAtTargetStatusWindowEnabled ()
{
	return ms_showLookAtTargetStatusWindowEnabled;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getShowStatusOverIntendedTarget ()
{
	return ms_showStatusOverIntendedTarget;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getUseDoubleToolbar ()
{
	return ms_useDoubleToolbar;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getShowToolbarCooldownTimer()
{
	return ms_showToolbarCooldownTimer;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getPointerModeMouseCameraEnabled ()
{
	return ms_pointerModeMouseCameraEnabled;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getMouseModeDefault ()
{
	int const sceneType = Game::getHudSceneType();
	
	if (sceneType < 0 || sceneType >= Game::ST_numTypes)
	{
		WARNING(true, ("CuiPreferences::getMouseModeDefault invalid scene type [%d]", sceneType));
		return false;
	}

	return ms_mouseModeDefault[sceneType];
}

//----------------------------------------------------------------------

bool   CuiPreferences::getTurnStrafesDuringMouseModeToggle ()
{
	return ms_turnStrafesDuringMouseModeToggle;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getAutoAimToggle ()
{
	return ms_autoAimToggle;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getEnableGimbal ()
{
	return ms_enableGimbal;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getTargetArrow ()
{
	return ms_targetArrow;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getDamagerArrow ()
{
	return ms_damagerArrow;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getVisibleEnemyDamagerArrow ()
{
	return ms_visibleEnemyDamagerArrow;
}

//----------------------------------------------------------------------

int CuiPreferences::getCurrencyFormat()
{
	return ms_currencyFormat;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getCanFireSecondariesFromToolbar ()
{
	return ms_canFireSecondariesFromToolbar;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getUseNewbieTutorial ()
{
	return ms_useNewbieTutorial;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getUseExpMonitor ()
{
	return ms_useExpMonitor;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getUseWaypointMonitor ()
{
	return ms_useWaypointMonitor;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getLocationDisplayEnabled ()
{
	return ms_locationDisplayEnabled;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getDpsMeterEnabled ()
{
	return ms_dpsMeterEnabled;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getShowGroupWaypoints ()
{
	return ms_showGroupWaypoints;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getTargetNothingUntargets ()
{
	return ms_targetNothingUntargets;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getModalChat ()
{
	int const sceneType = Game::getHudSceneType();
	
	if (sceneType < 0 || sceneType >= Game::ST_numTypes)
	{
		WARNING(true, ("CuiPreferences::getModalChat invalid scene type [%d]", sceneType));
		return false;
	}

	return ms_modalChat[sceneType];
}

//----------------------------------------------------------------------

bool   CuiPreferences::getNetStatusEnabled ()
{
	return ms_netStatusEnabled;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getConfirmObjDelete ()
{
	return ms_confirmObjDelete;
}

//----------------------------------------------------------------------

float  CuiPreferences::getObjectNameRange ()
{
	return ms_objectNameRange;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getAutoJoinChatRoomOnCreate ()
{
	return ms_autoJoinChatRoomOnCreate;
}

//----------------------------------------------------------------------

float  CuiPreferences::getCameraInertia           ()
{
	return ms_cameraInertia;
}

//----------------------------------------------------------------------

int    CuiPreferences::getDrawNetworkIds ()
{
	return ms_drawNetworkIds;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getDrawObjectNames ()
{
	return ms_drawObjectNames;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getDrawSelfName ()
{
	return ms_drawSelfName;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getDragOntoContainers ()
{
	return ms_dragOntoContainers;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getAllowTargetAnything ()
{
	return ms_allowTargetAnything && PlayerObject::isAdmin();
}

//----------------------------------------------------------------------

bool   CuiPreferences::getDebugExamine ()
{
	return ms_debugExamine;
}

//----------------------------------------------------------------------

bool   CuiPreferences::getDebugClipboardExamine ()
{
	return ms_debugClipboardExamine;
}

//----------------------------------------------------------------------

bool CuiPreferences::isTurnStrafe ()
{
	const bool pointerActive = CuiManager::getPointerInputActive ();
	bool result;

	if (!ms_turnStrafesDuringMouseModeToggle) 
	{
		result = true;
	}
	else
	{
		const bool isMouseLookStateAvatar = (CuiIoWin::getMouseLookState() == CuiIoWin::MouseLookState_Avatar);
		result = (pointerActive && isMouseLookStateAvatar);
	}
	return result;
}

//----------------------------------------------------------------------

bool CuiPreferences::getAutoInviteReject ()
{
	return ms_autoInviteReject;
}

//----------------------------------------------------------------------

void CuiPreferences::setAutoInviteReject (bool b)
{
	ms_autoInviteReject = b;
}

//----------------------------------------------------------------------

float CuiPreferences::getFirstPersonCameraInertia  ()
{
	return ms_firstPersonameraInertia;
}

//----------------------------------------------------------------------

void CuiPreferences::setFirstPersonCameraInertia  (float f)
{
	ms_firstPersonameraInertia = f;
}

//----------------------------------------------------------------------

float CuiPreferences::getObjectNameFontSizeFactor ()
{
	return ms_objectNameFontSizeFactor;
}

//----------------------------------------------------------------------

void CuiPreferences::setObjectNameFontSizeFactor (float f)
{
	ms_objectNameFontSizeFactor = f;
}

//----------------------------------------------------------------------

const std::string & CuiPreferences::getPaletteName ()
{
	return ms_paletteName;
}

//----------------------------------------------------------------------

bool CuiPreferences::setPaletteName (const std::string & paletteName, bool testForce, bool setName)
{
	if (paletteName.empty())
		return false;

	bool const isSpaceHud = Game::isHudSceneTypeSpace();
	const std::string & palettePath = isSpaceHud ? ms_stylePathSpace : ms_stylePathGround;

	UINamespace const * paletteFolder = UI_ASOBJECT(UINamespace, UIManager::gUIManager().GetObjectFromPath(palettePath.c_str()));
	if(!paletteFolder)
	{
		WARNING(true,("No palette folder found at [%s]", palettePath.c_str()));
		return false;
	}

	UIPalette * const palette = UI_ASOBJECT(UIPalette, paletteFolder->GetChild(paletteName.c_str()));
	if (!palette)
	{
		WARNING (true, ("No such palette [%s].", paletteName.c_str ()));
		return false;
	}

	if (isSpaceHud && !palette->IsSpaceEnabled())
	{
		return setPaletteName(ms_overrideSpacePalette, testForce, false);
	}

	bool production = false;
#if PRODUCTION
	production = true;
#endif

	if (testForce)
	{
		if (!_strnicmp (paletteName.c_str (), s_force_prefix, s_force_prefix_len))
		{
			const PlayerObject * const player = Game::getPlayerObject ();
			if (!player || player->getMaxForcePower () <= 0)
			{
				//-- allow non production clients to see force palettes when there is no player (out-of-game)
				if (player || production) //lint !e774 // right side always evaluates to true
				{
					WARNING (true, ("No such palette [%s].", paletteName.c_str ()));
					return false;
				}
			}
		}
	}

	UIPage * const root = UIManager::gUIManager ().GetRootPage ();
	NOT_NULL (root);
	
	root->SetPropertyNarrow (UILowerString ("palette"), palette->GetFullPath ());
	palette->Reset ();

	if (setName)
	{
		ms_paletteChangedCallback->performCallback();
		ms_paletteName = paletteName;
	}


	return true;
}

//----------------------------------------------------------------------

const CuiPreferences::StringVector & CuiPreferences::getPaletteNames (bool testForce)
{
	static StringVector sv;
	sv.clear ();

	//-- Set the space styles, but default to ground if it fails.
	bool const isSpaceHud = Game::isHudSceneTypeSpace();
	const std::string & palettePath = isSpaceHud ? ms_stylePathSpace : ms_stylePathGround;
	
	UINamespace const * paletteFolder = UI_ASOBJECT(UINamespace, UIManager::gUIManager().GetObjectFromPath(palettePath.c_str()));
	if(!paletteFolder)
	{
		WARNING(true,("No palette folder found at [%s]", palettePath.c_str()));
		return sv;
	}
	
	bool production = false;
#if PRODUCTION
	production = true;
#endif

	bool showForce = false;

	if (!testForce)
		showForce = true;
	else 
	{
		const PlayerObject * const player = Game::getPlayerObject ();
		if (player)
		{
			showForce = player->getMaxForcePower () > 0;
		}
		//-- allow non production clients to see force palettes when there is no player (out-of-game)
		else if (!production) //lint !e774 //always true
		{
			showForce = true;
		}
	}

	const UIBaseObject::UIObjectList & olist = paletteFolder->GetChildrenRef ();
	
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIPalette const * const pal = UI_ASOBJECT(UIPalette, *it);
		
		if (pal && (!isSpaceHud || pal->IsSpaceEnabled()))
		{
			std::string const & name = pal->GetName();

			if (!showForce && !_strnicmp (name.c_str (), s_force_prefix, s_force_prefix_len))
				continue;

			sv.push_back(pal->GetName());
		}
	}
	
	return sv;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowSystemMessages ()
{
	return ms_showSystemMessages;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowSystemMessages (bool b)
{
	ms_showSystemMessages = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getChatAutoEmote ()
{
	return ms_chatAutoEmote;
}

//----------------------------------------------------------------------

void CuiPreferences::setChatAutoEmote (bool b)
{
	ms_chatAutoEmote = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setConfirmCrafting (bool b)
{
	ms_confirmCrafting = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getConfirmCrafting ()
{
	return ms_confirmCrafting;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowPopupHelp ()
{
	return ms_showPopupHelp;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowPopupHelp (bool b)
{
	ms_showPopupHelp = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setCombatSpamBrief( bool b )
{
	ms_combatSpamVerbose = !b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getCombatSpamBrief()
{
	return !ms_combatSpamVerbose;
}

//----------------------------------------------------------------------

void CuiPreferences::setCombatSpamVerbose( bool b )
{
	ms_combatSpamVerbose = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getCombatSpamVerbose()
{
	return ms_combatSpamVerbose;
}

//----------------------------------------------------------------------

void CuiPreferences::setSpamShowWeapon( bool b )
{
	ms_spamShowWeapon = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getSpamShowWeapon()
{
	return ms_spamShowWeapon;
}

//----------------------------------------------------------------------

void CuiPreferences::setSpamShowDamageDetail( bool b )
{
	ms_spamShowDamageDetail = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getSpamShowDamageDetail()
{
	return ms_spamShowDamageDetail;
}

//----------------------------------------------------------------------

void CuiPreferences::setSpamShowArmorAbsorption( bool b )
{
	ms_spamShowArmorAbsorption = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getSpamShowArmorAbsorption()
{
	return ms_spamShowArmorAbsorption;
}

//----------------------------------------------------------------------

void CuiPreferences::refreshValues ()
{
	setTooltipDelaySecs (ms_tooltipDelaySecs);
	setMouseModeDefault (getMouseModeDefault());
	setCameraInertia    (ms_cameraInertia);
	setVariableTargetingReticlePercentage(ms_variableTargetingReticlePercentage);
	IGNORE_RETURN(setPaletteName(ms_paletteName, false));

	//-- if there is no joystick we must be able to fly the ship from the mouse
	if (DirectInput::getNumberOfJoysticksAvailable() <= 0)
	{
		if (ms_pilotMouseMode == static_cast<PilotMouseMode>(PMM_cockpitCamera))
			ms_pilotMouseMode = static_cast<int>(PMM_virtualJoystick);
	}
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowIconNames ()
{
	return ms_showIconNames;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowIconNames (bool b)
{
	ms_showIconNames = b;
	ms_objectIconCallback->performCallback ();
}

//----------------------------------------------------------------------

void CuiPreferences::setObjectIconSize (float f)
{
	ms_objectIconSize = std::max (std::min (f, 2.0f), 0.5f);
	ms_objectIconCallback->performCallback ();
}

//----------------------------------------------------------------------

float CuiPreferences::getObjectIconSize ()
{
	return ms_objectIconSize;
}

//----------------------------------------------------------------------

void CuiPreferences::setCommandButtonOpacity (float f)
{
	ms_commandButtonOpacity = std::max (std::min (f, 1.0f), 0.0f);
	ms_commandButtonOpacityCallback->performCallback ();
}

//----------------------------------------------------------------------

float CuiPreferences::getCommandButtonOpacity ()
{
	return ms_commandButtonOpacity;
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getObjectIconCallback ()
{
	return *ms_objectIconCallback;
}

//----------------------------------------------------------------------

Callback & CuiPreferences::getCommandButtonOpacityCallback ()
{
	return *ms_commandButtonOpacityCallback;
}

//----------------------------------------------------------------------

void CuiPreferences::setSystemMessageDuration (float f)
{
	ms_systemMessageDuration = f;
}

//----------------------------------------------------------------------

float CuiPreferences::getSystemMessageDuration ()
{
	return ms_systemMessageDuration;
}

//----------------------------------------------------------------------

float CuiPreferences::getObjectIconMinSize ()
{
	return 0.5f;
}

//----------------------------------------------------------------------

float CuiPreferences::getObjectIconMaxSize ()
{
	return 2.0f;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowWaypointArrowsOnscreen (bool b)
{
	ms_showWaypointArrowsOnscreen = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowWaypointArrowsOnscreen ()
{
	return ms_showWaypointArrowsOnscreen;
}

//----------------------------------------------------------------------

void CuiPreferences::setDropShadowUiEnabled (bool b)
{
	ms_dropShadowUiEnabled = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDropShadowUiEnabled ()
{
	return ms_dropShadowUiEnabled;
}

//----------------------------------------------------------------------

void CuiPreferences::setDropShadowObjectNamesEnabled (bool b)
{
	ms_dropShadowObjectNamesEnabled = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDropShadowObjectNamesEnabled ()
{
	return ms_dropShadowObjectNamesEnabled;
}

//----------------------------------------------------------------------

void CuiPreferences::setRotateMap(bool b)
{
	ms_rotateMap = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getRotateMap ()
{
	return ms_rotateMap;
}

//----------------------------------------------------------------------

void CuiPreferences::setRotateInventoryObjects(bool b)
{
	ms_rotateInventoryObjects = b;
	ms_objectIconCallback->performCallback ();
}

//----------------------------------------------------------------------

bool CuiPreferences::getRotateInventoryObjects ()
{
	return ms_rotateInventoryObjects;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowInterestingAppearance (bool b)
{
	if (ms_showInterestingAppearance != b)
	{
		ms_showInterestingAppearance = b;
		ms_showInterestingAppearanceCallback->performCallback();
	}
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowInterestingAppearance ()
{
	return ms_showInterestingAppearance;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowAFKSpeech (bool b)
{
	ms_showAFKSpeech = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowAFKSpeech ()
{
	return ms_showAFKSpeech;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowNPCSpeech (bool b)
{
	ms_showNPCSpeech = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowNPCSpeech ()
{
	return ms_showNPCSpeech;
}

//----------------------------------------------------------------------

void CuiPreferences::setJoystickDeadZone(float f)
{
	ms_joystickDeadZone = f;
}

//----------------------------------------------------------------------

float CuiPreferences::getJoystickDeadZone()
{
	return ms_joystickDeadZone;
}

//----------------------------------------------------------------------

void CuiPreferences::setReticleSelect (int reticle)
{
	ms_reticleSelect = reticle;
	ms_reticleSelectCallback->performCallback ();
}

//----------------------------------------------------------------------

int CuiPreferences::getReticleSelect ()
{
	return ms_reticleSelect;
}

//----------------------------------------------------------------------

void CuiPreferences::setRadarSelect (int radar)
{
	ms_radarSelect = radar;
	ms_radarSelectCallback->performCallback ();
}

//----------------------------------------------------------------------

int CuiPreferences::getRadarSelect ()
{
	return ms_radarSelect;
}

//----------------------------------------------------------------------

void CuiPreferences::setSpaceCameraElasticity(float f)
{
	if (f != ms_spaceCameraElasticity)  //lint !e777 // Testing floats for equality
	{
		ms_spaceCameraElasticity = std::max(getSpaceCameraElasticityMinSize(), std::min(f, getSpaceCameraElasticityMaxSize()));
		ms_spaceCameraElasticityCallback->performCallback();
	}
}

//----------------------------------------------------------------------

float CuiPreferences::getSpaceCameraElasticity()
{
	return ms_spaceCameraElasticity;
}

//----------------------------------------------------------------------

float CuiPreferences::getSpaceCameraElasticityMinSize()
{
	return 0.0f;
}

//----------------------------------------------------------------------

float CuiPreferences::getSpaceCameraElasticityMaxSize()
{
	return 0.5f;
}

//----------------------------------------------------------------------

float CuiPreferences::getSpaceCameraElasticityDefault()
{
	return 0.3f;
}

//----------------------------------------------------------------------

void CuiPreferences::setVariableTargetingReticlePercentage(float f)
{
	float const minimum = getVariableTargetingReticlePercentageMinimumSize();
	float const maximum = getVariableTargetingReticlePercentageMaximumSize();
	ms_variableTargetingReticlePercentage = clamp(minimum, f, maximum);
	ms_variableTargetingReticlePercentageCallback->performCallback();
}

//----------------------------------------------------------------------

float CuiPreferences::getVariableTargetingReticlePercentage()
{
	return ms_variableTargetingReticlePercentage;
}

//----------------------------------------------------------------------

float CuiPreferences::getVariableTargetingReticlePercentageMinimumSize()
{
	return 0.05f;
}

//----------------------------------------------------------------------

float CuiPreferences::getVariableTargetingReticlePercentageMaximumSize()
{
	return 1.0f;
}

//----------------------------------------------------------------------

bool CuiPreferences::getRenderVariableTargetingReticle()
{
	return ms_renderVariableTargetingReticle;
}

//----------------------------------------------------------------------

void CuiPreferences::setRenderVariableTargetingReticle(bool enabled)
{
	ms_renderVariableTargetingReticle = enabled;
}

//----------------------------------------------------------------------

int CuiPreferences::getShipDestroyDetailBias()
{
	return ms_shipDestroyDetailBias;
}

//----------------------------------------------------------------------

void CuiPreferences::getShipDestroyDetailBias(int const bias)
{
	ms_shipDestroyDetailBias = bias;
}

//----------------------------------------------------------------------

int CuiPreferences::getCapshipDestroyDetailBias()
{
	return ms_capshipDestroyDetailBias;
}

//----------------------------------------------------------------------

void CuiPreferences::getCapshipDestroyDetailBias(int const bias)
{
	ms_capshipDestroyDetailBias = bias;
}

//----------------------------------------------------------------------

void CuiPreferences::setScreenShake (bool b)
{
	ms_screenShake = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getScreenShake ()
{
	return ms_screenShake;
}

//----------------------------------------------------------------------

float CuiPreferences::getShipDebrisLifespan()
{
	return ms_shipDebrisLifespan;
}

//----------------------------------------------------------------------

void CuiPreferences::setShipDebrisLifespan(float const lifeSpan)
{
	ms_shipDebrisLifespan = lifeSpan;
}

//----------------------------------------------------------------------

bool CuiPreferences::getUseSpaceDebugWindows()
{
	return ms_useSpaceDebugWindows;
}

//----------------------------------------------------------------------

void CuiPreferences::setUseSpaceDebugWindows(bool const useit)
{
	ms_useSpaceDebugWindows = useit;
}

//----------------------------------------------------------------------

float CuiPreferences::getGlobalNebulaDensity()
{
	return ms_globalNebulaDensity;
}

//----------------------------------------------------------------------

void CuiPreferences::setGlobalNebulaDensity(float density)
{
	ms_globalNebulaDensity = density;
}

//----------------------------------------------------------------------

float CuiPreferences::getGlobalNebulaDensityDefault()
{
	return 60.0f;
}

//----------------------------------------------------------------------

float CuiPreferences::getGlobalNebulaRange()
{
	return ms_globalNebulaRange;
}

//----------------------------------------------------------------------

void CuiPreferences::setGlobalNebulaRange(float range)
{
	ms_globalNebulaRange = range;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShipAutolevel()
{
	return ms_shipAutolevel;
}

//----------------------------------------------------------------------

void CuiPreferences::setShipAutolevel(bool enabled)
{
	ms_shipAutolevel = enabled;
}

//----------------------------------------------------------------------

bool CuiPreferences::getAutoSortInventoryContents()
{
	return ms_autoSortInventoryContents;
}

//----------------------------------------------------------------------

void CuiPreferences::setAutoSortInventoryContents(bool enabled)
{
	ms_autoSortInventoryContents = enabled;
}

//----------------------------------------------------------------------

bool CuiPreferences::getAutoSortDataPadContents()
{
	return ms_autoSortDataPadContents;
}

//----------------------------------------------------------------------

void CuiPreferences::setAutoSortDataPadContents(bool enabled)
{
	ms_autoSortDataPadContents = enabled;
}

//----------------------------------------------------------------------

CuiPreferences::PilotMouseMode CuiPreferences::getPilotMouseMode()
{
	return static_cast<PilotMouseMode>(ms_pilotMouseMode);
}

//----------------------------------------------------------------------

CuiPreferences::PilotMouseMode CuiPreferences::getPilotMouseModeAutomatic()
{
	if (PMM_automatic == static_cast<PilotMouseMode>(ms_pilotMouseMode))
	{
		if (DirectInput::getNumberOfJoysticksAvailable() <= 0)
			return PMM_virtualJoystick;
		else
			return PMM_cockpitCamera;
	}

	return static_cast<PilotMouseMode>(ms_pilotMouseMode);
}

//----------------------------------------------------------------------

void CuiPreferences::setPilotMouseMode(PilotMouseMode pmm)
{
	ms_pilotMouseMode = static_cast<int>(pmm);
}

//----------------------------------------------------------------------

bool CuiPreferences::isJoystickInverted()
{
	return ms_joystickInverted;
}

//----------------------------------------------------------------------

void CuiPreferences::setJoystickInverted(bool b)
{
	ms_joystickInverted = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setJoystickSensitivity(float f)
{
	ms_joystickSensitivity = f;
}

//----------------------------------------------------------------------

float CuiPreferences::getJoystickSensitivity()
{
	return ms_joystickSensitivity;
}

//----------------------------------------------------------------------

CuiPreferences::PovHatMode CuiPreferences::getPovHatMode()
{
	return static_cast<CuiPreferences::PovHatMode>(ms_povHatMode);
}

//----------------------------------------------------------------------

void CuiPreferences::setPovHatMode(PovHatMode phm)
{
	ms_povHatMode = static_cast<int>(phm);
}

//----------------------------------------------------------------------

void CuiPreferences::setPovHatPanSpeed(float speed)
{
	ms_povHatPanSpeed = speed;
}

//----------------------------------------------------------------------

float CuiPreferences::getPovHatPanSpeed()
{
	return ms_povHatPanSpeed;
}

//----------------------------------------------------------------------

void CuiPreferences::setCockpitCameraSnapSpeed(float speed)
{
	ms_cockpitCameraSnapSpeed = speed;
}

//----------------------------------------------------------------------

float CuiPreferences::getCockpitCameraSnapSpeed()
{
	return ms_cockpitCameraSnapSpeed;
}

//----------------------------------------------------------------------

void CuiPreferences::setCockpitCameraYOffset(float offset)
{
	ms_cockpitCameraYOffset = offset;
}

//----------------------------------------------------------------------

float CuiPreferences::getCockpitCameraYOffset()
{
	return ms_cockpitCameraYOffset;
}

//----------------------------------------------------------------------

void CuiPreferences::setCockpitCameraZoomMultiplier(float zoomMultiplier)
{
	ms_cockpitCameraZoomMultiplier = zoomMultiplier;
}

//----------------------------------------------------------------------

float CuiPreferences::getCockpitCameraZoomMultiplier()
{
	return ms_cockpitCameraZoomMultiplier;
}

//----------------------------------------------------------------------

void CuiPreferences::setCockpitCameraRecenterOnShipMovement(bool b)
{
	ms_cockpitCameraRecenterOnShipMovement = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getCockpitCameraRecenterOnShipMovement()
{
	return ms_cockpitCameraRecenterOnShipMovement;
}

//----------------------------------------------------------------------

void CuiPreferences::setPovHatSnapAngle(float angle)
{
	ms_povHatSnapAngle = angle;
}

//----------------------------------------------------------------------

float CuiPreferences::getPovHatSnapAngle()
{
	return ms_povHatSnapAngle;
}

//----------------------------------------------------------------------

void CuiPreferences::setPovHatSnapAngleDegrees(float angle)
{
	CuiPreferences::setPovHatSnapAngle(convertDegreesToRadians(angle));
}

//----------------------------------------------------------------------

float CuiPreferences::getPovHatSnapAngleDegrees()
{
	return convertRadiansToDegrees(ms_povHatSnapAngle);
}

//----------------------------------------------------------------------

void CuiPreferences::setShowGameObjectArrowsOnRadar(bool b)
{
	ms_showObjectArrowsOnRadar = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowGameObjectArrowsOnRadar()
{
	return ms_showObjectArrowsOnRadar;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowRadarNPCs(bool b)
{
	ms_showRadarNPCs = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowRadarNPCs()
{
	return ms_showRadarNPCs;
}

//----------------------------------------------------------------------

void CuiPreferences::setUseModelessInterface(bool b)
{
	ms_useModelessInterface = b;

	setModalChat( true );
}

//----------------------------------------------------------------------

bool CuiPreferences::getUseModelessInterface()
{
	if ( Game::getHudSceneType() == Game::ST_space )
	{
		return false;
	}

	return ms_useModelessInterface;
}

//----------------------------------------------------------------------

bool CuiPreferences::getActualUseModelessInterface()
{
	return ms_useModelessInterface;
}

//----------------------------------------------------------------------

void CuiPreferences::setUseSwgMouseMap(bool b)
{
	ms_useSwgMouseMap = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getUseSwgMouseMap()
{
	return ms_useSwgMouseMap;
}

//----------------------------------------------------------------------

void CuiPreferences::setCharacterSheetShowDetails(bool b)
{
	ms_characterSheetShowDetails = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getCharacterSheetShowDetails()
{
	return ms_characterSheetShowDetails;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowNotifications(bool b)
{
	ms_showNotifications = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowNotifications()
{
	return ms_showNotifications;
}

//----------------------------------------------------------------------

void CuiPreferences::setChatBarFadesOut(bool b)
{
	ms_chatBarFadesOut = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getChatBarFadesOut()
{
	return ms_chatBarFadesOut;
}

//----------------------------------------------------------------------

void CuiPreferences::setUiSettingsVersion(int i)
{
	ms_uiSettingsVersion = i;
}

//----------------------------------------------------------------------

int CuiPreferences::getUiSettingsVersion()
{
	return ms_uiSettingsVersion;
}

//----------------------------------------------------------------------

int CuiPreferences::getDefaultUiSettingsVersion()
{
	return cms_defaultUiSettingsVersion;
}

//----------------------------------------------------------------------

bool CuiPreferences::getMiddleMouseDrivesMovementToggle()
{
	return ms_middleMouseDrivesMovementToggle;
}

//----------------------------------------------------------------------

void CuiPreferences::setMiddleMouseDrivesMovementToggle(bool b)
{
	ms_middleMouseDrivesMovementToggle = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getMouseLeftAndRightDrivesMovementToggle()
{
	return ms_mouseLeftAndRightDrivesMovementToggle;
}

//----------------------------------------------------------------------

void CuiPreferences::setMouseLeftAndRightDrivesMovementToggle(bool b)
{
	ms_mouseLeftAndRightDrivesMovementToggle = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getScrollThroughDefaultActions()
{
	return ms_scrollThroughDefaultActions;
}

//----------------------------------------------------------------------

void CuiPreferences::setScrollThroughDefaultActions(bool b)
{
	ms_scrollThroughDefaultActions = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setOffsetCamera(bool b)
{
	ms_offsetCamera = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getOffsetCamera()
{
	return ms_offsetCamera;
}

//----------------------------------------------------------------------

void CuiPreferences::setCombatSpamFilter(int combatSpamFilter)
{
	ms_combatSpamFilter = combatSpamFilter;
}

//----------------------------------------------------------------------

int CuiPreferences::getCombatSpamFilter()
{
	return ms_combatSpamFilter;
}

//----------------------------------------------------------------------

int CuiPreferences::getCombatSpamFilterDefault()
{
	return ms_combatSpamFilterDefault;
}

//----------------------------------------------------------------------

void CuiPreferences::setCombatSpamRangeFilter(int combatSpamRangeFilter)
{
	ms_combatSpamRangeFilter = combatSpamRangeFilter;
}

//----------------------------------------------------------------------

int CuiPreferences::getCombatSpamRangeFilter()
{
	return ms_combatSpamRangeFilter;
}

//----------------------------------------------------------------------

int CuiPreferences::getCombatSpamRangeFilterDefault()
{
	return ms_combatSpamRangeFilterDefault;
}

//----------------------------------------------------------------------

void CuiPreferences::setFurnitureRotationDegree(int degree)
{
	ms_furnitureRotationDegree = degree;
}

//----------------------------------------------------------------------

int CuiPreferences::getFurnitureRotationDegree()
{
	return ms_furnitureRotationDegree;
}

//----------------------------------------------------------------------

int CuiPreferences::getFurnitureRotationDegreeDefault()
{
	return ms_furnitureRotationDegreeDefault;
}

//----------------------------------------------------------------------

void CuiPreferences::setCommoditiesWindowColumnSize(int type, int columnIndex, int value)
{
	if ((type < 0) || (type >= (sizeof(ms_cscs)/sizeof(ms_cscs[0]))))
		return;

	if ((columnIndex < 0) || (columnIndex >= ms_cscsNumColumns[type]))
		return;

	*((ms_cscs[type])[columnIndex]) = value;
}

//----------------------------------------------------------------------

int CuiPreferences::getCommoditiesWindowColumnSize(int type, int columnIndex)
{
	if ((type < 0) || (type >= (sizeof(ms_cscs)/sizeof(ms_cscs[0]))))
		return 0;

	if ((columnIndex < 0) || (columnIndex >= ms_cscsNumColumns[type]))
		return 0;

	return *((ms_cscs[type])[columnIndex]);
}

//----------------------------------------------------------------------

bool CuiPreferences::getCollectionShowServerFirst()
{
	return ms_collectionShowServerFirst;
}

//----------------------------------------------------------------------

void CuiPreferences::setCollectionShowServerFirst(bool enabled)
{
	const bool changed = (ms_collectionShowServerFirst && !enabled) || (!ms_collectionShowServerFirst && enabled);
	ms_collectionShowServerFirst = enabled;
	if (changed)
		Game::handleCollectionShowServerFirstOptionChanged(enabled);
}

//----------------------------------------------------------------------

int CuiPreferences::getSecondaryTargetMode()
{
	return ms_secondaryTargetMode;
}

//----------------------------------------------------------------------

void CuiPreferences::setSecondaryTargetMode(int mode)
{
	ms_secondaryTargetMode = mode;
}

//----------------------------------------------------------------------

bool CuiPreferences::getAlwaysShowRangeInGroundRadar()
{
	return ms_alwaysShowRangeInGroundRadar;
}

//----------------------------------------------------------------------

void CuiPreferences::setAlwaysShowRangeInGroundRadar(bool alwaysShow)
{
	ms_alwaysShowRangeInGroundRadar = alwaysShow;
	CuiActionManager::performAction("radarAlwaysShowRange", alwaysShow ? Unicode::narrowToWide("1") : Unicode::narrowToWide("0"));
}

//----------------------------------------------------------------------

bool CuiPreferences::getNewVendorDoubleClickExamine()
{
	return ms_newVendorDoubleClick;
}

//----------------------------------------------------------------------

void CuiPreferences::setNewVendorDoubleClickExamine(bool b)
{
	ms_newVendorDoubleClick = b;
}

//----------------------------------------------------------------------

void CuiPreferences::setDisableAnimationPriorities(bool b)
{
	ms_disableAnimationPriorities = b;
	PriorityPlaybackScriptManager::setPlaybackPriorityIgnore(ms_disableAnimationPriorities);
}

//----------------------------------------------------------------------

bool CuiPreferences::getDisableAnimationPriorities()
{
	return ms_disableAnimationPriorities;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowCorpseLootIcon()
{
	return ms_showCorpseLootIcon;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowCorpseLootIcon(bool show)
{
	ms_showCorpseLootIcon = show;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowBackpack()
{
	return ms_showBackpack;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowBackpack(bool b)
{
	if(ms_showBackpack != b)
	{
		ms_showBackpack = b;
		if(Game::getPlayerObject())
			Game::getPlayerObject()->sendBackpackMsg(ms_showBackpack);
	}
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowHelmet()
{
	return ms_showHelmet;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowQuestHelper(bool b)
{
	ms_showQuestHelper = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowQuestHelper()
{
	return ms_showQuestHelper;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowHelmet(bool b)
{
	if(ms_showHelmet != b)
	{
		ms_showHelmet = b;
		if(Game::getPlayerObject())
			Game::getPlayerObject()->sendHelmetMsg(ms_showHelmet);
	}
}

//----------------------------------------------------------------------

void CuiPreferences::setShowCompletedCollections(bool b)
{
	ms_showCompletedCollections = b;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowCompletedCollections()
{
	return ms_showCompletedCollections;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSize(int type)
{
	switch (type)
	{
	case BIT_status:
		return getBuffIconSizeStatus();
		break;
	case BIT_target:
		return getBuffIconSizeTarget();
		break;
	case BIT_secondaryTarget:
		return getBuffIconSizeSecondaryTarget();
		break;
	case BIT_pet:
		return getBuffIconSizePet();
		break;
	case BIT_group:
		return getBuffIconSizeGroup();
		break;
	default:
		DEBUG_WARNING(true, ("CuiPreferences::getBuffIconSize: request for icon size of invalid type"));
		return 0;
		break;
	}
}

//----------------------------------------------------------------------

void CuiPreferences::setBuffIconSize(int type, int iconSize)
{
	switch (type)
	{
	case BIT_status:
		return setBuffIconSizeStatus(iconSize);
		break;
	case BIT_target:
		return setBuffIconSizeTarget(iconSize);
		break;
	case BIT_secondaryTarget:
		return setBuffIconSizeSecondaryTarget(iconSize);
		break;
	case BIT_pet:
		return setBuffIconSizePet(iconSize);
		break;
	case BIT_group:
		return setBuffIconSizeGroup(iconSize);
		break;
	default:
		DEBUG_WARNING(true, ("CuiPreferences::setBuffIconSize: request to set icon size of invalid type"));
		return;
		break;
	}
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizeSliderMin()
{
	return ms_buffIconSizeSliderMin;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizeSliderMax()
{
	return ms_buffIconSizeSliderMax;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizeStatus()
{
	return ms_buffIconSizeStatus;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizeTarget()
{
	return ms_buffIconSizeTarget;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizeSecondaryTarget()
{
	return ms_buffIconSizeSecondaryTarget;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizePet()
{
	return ms_buffIconSizePet;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizeGroup()
{
	return ms_buffIconSizeGroup;
}

//----------------------------------------------------------------------

void CuiPreferences::setBuffIconSizeStatus(int iconSize)
{
	ms_buffIconSizeStatus = iconSize;
	ms_buffIconSettingsChangedCallback->performCallback();
}

//----------------------------------------------------------------------

void CuiPreferences::setBuffIconSizeTarget(int iconSize)
{
	ms_buffIconSizeTarget = iconSize;
	ms_buffIconSettingsChangedCallback->performCallback();
}

//----------------------------------------------------------------------

void CuiPreferences::setBuffIconSizeSecondaryTarget(int iconSize)
{
	ms_buffIconSizeSecondaryTarget = iconSize;
	ms_buffIconSettingsChangedCallback->performCallback();
}

//----------------------------------------------------------------------

void CuiPreferences::setBuffIconSizePet(int iconSize)
{
	ms_buffIconSizePet = iconSize;
	ms_buffIconSettingsChangedCallback->performCallback();
}

//----------------------------------------------------------------------

void CuiPreferences::setBuffIconSizeGroup(int iconSize)
{
	ms_buffIconSizeGroup = iconSize;
	ms_buffIconSettingsChangedCallback->performCallback();
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizeStatusDefault()
{
	return ms_buffIconSizeStatusDefault;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizeTargetDefault()
{
	return ms_buffIconSizeTargetDefault;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizeSecondaryTargetDefault()
{
	return ms_buffIconSizeSecondaryTargetDefault;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizePetDefault()
{
	return ms_buffIconSizePetDefault;
}

//----------------------------------------------------------------------

int CuiPreferences::getBuffIconSizeGroupDefault()
{
	return ms_buffIconSizePetDefault;
}

//----------------------------------------------------------------------

float CuiPreferences::getBuffIconWhirlygigOpacity()
{
	return ms_buffIconWhirlygigOpacity;
}

//----------------------------------------------------------------------

void CuiPreferences::setBuffIconWhirlygigOpacity(float opacityPercent)
{
	ms_buffIconWhirlygigOpacity = opacityPercent;
	ms_buffIconSettingsChangedCallback->performCallback();
}

//----------------------------------------------------------------------

float CuiPreferences::getBuffIconWhirlygigOpacityDefault()
{
	return ms_buffIconWhirlygigOpacityDefault;
}

//----------------------------------------------------------------------

Callback& CuiPreferences::getBuffIconSettingsChangedCallback()
{
	return *ms_buffIconSettingsChangedCallback;
}

//----------------------------------------------------------------------

bool CuiPreferences::getVoiceChatEnabled()
{
	return ms_voiceChatEnabled;
}

//----------------------------------------------------------------------

void CuiPreferences::setVoiceChatEnabled(bool enabled)
{
	ms_voiceChatEnabled = enabled;
}

//----------------------------------------------------------------------

bool CuiPreferences::getVoiceUsePushToTalk()
{
	return ms_voiceUsePushToTalk;
}

//----------------------------------------------------------------------

void CuiPreferences::setVoiceUsePushToTalk(bool useit)
{
	ms_voiceUsePushToTalk = useit;
}

//----------------------------------------------------------------------

bool CuiPreferences::getVoiceShowFlybar()
{
	return ms_voiceShowFlybar;
}

//----------------------------------------------------------------------

void CuiPreferences::setVoiceShowFlybar(bool showit)
{
	ms_voiceShowFlybar = showit;
}

//----------------------------------------------------------------------

bool CuiPreferences::getVoiceUseAdvancedChannelSelection()
{
	return ms_voiceUseAdvancedChannelSelection;
}

//----------------------------------------------------------------------

void CuiPreferences::setVoiceUseAdvancedChannelSelection(bool useit)
{
	ms_voiceUseAdvancedChannelSelection = useit;
}

//----------------------------------------------------------------------

bool CuiPreferences::getVoiceAutoDeclineInvites()
{
	return ms_voiceAutoDeclineInvites;
}

//----------------------------------------------------------------------

void CuiPreferences::setVoiceAutoDeclineInvites(bool declineThemAll)
{
	ms_voiceAutoDeclineInvites = declineThemAll;
}

//----------------------------------------------------------------------

bool CuiPreferences::getVoiceAutoJoinChannels()
{
	return ms_voiceAutoJoinChannels;
}

//----------------------------------------------------------------------

void CuiPreferences::setVoiceAutoJoinChannels(bool joinAway)
{
	ms_voiceAutoJoinChannels = joinAway;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDefaultExamineHideAppearance()
{
	return ms_defaultExamineHideAppearance;
}

//----------------------------------------------------------------------

void CuiPreferences::setDefaultExamineHideAppearance(bool hide)
{
	ms_defaultExamineHideAppearance = hide;
}

//----------------------------------------------------------------------

bool CuiPreferences::getDoubleClickUnequipAppearance()
{
	return ms_doubleClickAppearanceUnequip;
}

//----------------------------------------------------------------------

void CuiPreferences::setDoubleClickUnequipAppearance(bool unequip)
{
	ms_doubleClickAppearanceUnequip = unequip;
}

//----------------------------------------------------------------------

float CuiPreferences::getPlayerCameraHeight()
{
	return ms_playerCameraHeight;
}

//----------------------------------------------------------------------

void CuiPreferences::setPlayerCameraHeight(float height)
{
	ms_playerCameraHeight = height;
}

//----------------------------------------------------------------------

void CuiPreferences::setShowAppearanceInventory(bool show)
{
	ms_showAppearanceInventory = show;
}

//----------------------------------------------------------------------

bool CuiPreferences::getShowAppearanceInventory()
{
	return ms_showAppearanceInventory;
}

//----------------------------------------------------------------------

void CuiPreferences::setAutoLootCorpses(bool loot)
{
	ms_autoLootCorpses = loot;
}

//----------------------------------------------------------------------

bool CuiPreferences::getAutoLootCorpses()
{
	return ms_autoLootCorpses;
}

//----------------------------------------------------------------------

void CuiPreferences::setSpeakerVolume(float volume)
{
	ms_speakerVolume = volume;
}

//----------------------------------------------------------------------

float CuiPreferences::getSpeakerVolume()
{
	return ms_speakerVolume;
}

//----------------------------------------------------------------------

void CuiPreferences::setMicVolume(float volume)
{
	ms_micVolume = volume;
}

//----------------------------------------------------------------------

float CuiPreferences::getMicVolume()
{
	return ms_micVolume;
}

//----------------------------------------------------------------------

void CuiPreferences::setOverheadMapOpacity(float opacity)
{
	ms_overheadMapOpacity = opacity;
}

//----------------------------------------------------------------------

float CuiPreferences::getOverheadMapOpacity()
{
	return ms_overheadMapOpacity;
}

//----------------------------------------------------------------------

void CuiPreferences::setOverheadMapShowWaypoints(bool show)
{
	ms_overheadMapShowWaypoints = show;
}

//----------------------------------------------------------------------

bool CuiPreferences::getOverheadMapShowWaypoints()
{
	return ms_overheadMapShowWaypoints;
}

//----------------------------------------------------------------------

void CuiPreferences::setOverheadMapShowCreatures(bool show)
{
	ms_overheadMapShowCreatures = show;
}

//----------------------------------------------------------------------

bool CuiPreferences::getOverheadMapShowCreatures()
{
	return ms_overheadMapShowCreatures;
}

//----------------------------------------------------------------------

void CuiPreferences::setOverheadMapShowPlayer(bool show)
{
	ms_overheadMapShowPlayer = show;
}

//----------------------------------------------------------------------

bool CuiPreferences::getOverheadMapShowPlayer()
{
	return ms_overheadMapShowPlayer;
}

//----------------------------------------------------------------------

void CuiPreferences::setOverheadMapShowLabels(bool show)
{
	ms_overheadMapShowLabels = show;
}

//----------------------------------------------------------------------

bool CuiPreferences::getOverheadMapShowLabels()
{
	return ms_overheadMapShowLabels;
}

//----------------------------------------------------------------------

void CuiPreferences::setOverheadMapShowBuildings(bool show)
{
	ms_overheadMapShowBuildings = show;
}

//----------------------------------------------------------------------

bool CuiPreferences::getOverheadMapShowBuildings()
{
	return ms_overheadMapShowBuildings;
}

//----------------------------------------------------------------------

void CuiPreferences::setHideCharactersOnClosedGalaxies(bool hide)
{
	ms_hideCharactersOnClosedGalaxies = hide;
}

//----------------------------------------------------------------------

bool CuiPreferences::getHideCharactersOnClosedGalaxies()
{
	return ms_hideCharactersOnClosedGalaxies;
}

//======================================================================
