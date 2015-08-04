// ======================================================================
//
// SwgCuiMediatorFactorySetup.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiMediatorFactorySetup.h"

#include "clientUserInterface/CuiBackdrop.h"
#include "clientUserInterface/CuiBugSubmissionPage.h"
#include "clientUserInterface/CuiColorPicker.h"
#include "clientUserInterface/CuiDeleteSkillConfirmation.h"
#include "clientUserInterface/CuiHueObject.h"
#include "clientUserInterface/CuiInputBox.h"
#include "clientUserInterface/CuiKeypad.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorFactory_Constructor.h"
#include "clientUserInterface/CuiMinigameManager.h"
#include "clientUserInterface/CuiNotepad.h"
#include "clientUserInterface/CuiPopupHelp.h"
#include "clientUserInterface/CuiSetName.h"
#include "clientUserInterface/CuiShipSelfInfo.h"
#include "clientUserInterface/CuiShipTargetInfo.h"
#include "clientUserInterface/CuiTransfer.h"
#include "clientUserInterface/CuiTransition.h"
#include "clientUserInterface/CuiVehicleProto.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Production.h"
#include "swgClientUserInterface/SwgCuiAppearanceTab.h"
#include "swgClientUserInterface/SwgCuiAuctionBid.h"
#include "swgClientUserInterface/SwgCuiAuctionBuy.h"
#include "swgClientUserInterface/SwgCuiAuctionDetails.h"
#include "swgClientUserInterface/SwgCuiAuctionList.h"
#include "swgClientUserInterface/SwgCuiAuctionSell.h"
#include "swgClientUserInterface/SwgCuiAvatarCreation.h"
#include "swgClientUserInterface/SwgCuiAvatarCustomize2.h"
#include "swgClientUserInterface/SwgCuiAvatarLocation2.h"
#include "swgClientUserInterface/SwgCuiAvatarProfessionTemplateSelect.h"
#include "swgClientUserInterface/SwgCuiAvatarSelection.h"
#include "swgClientUserInterface/SwgCuiAvatarSetupProf.h"
#include "swgClientUserInterface/SwgCuiAvatarSimple.h"
#include "swgClientUserInterface/SwgCuiAvatarSummary.h"
#include "swgClientUserInterface/SwgCuiBiographyEdit.h"
#include "swgClientUserInterface/SwgCuiBuffBuilderBuffee.h"
#include "swgClientUserInterface/SwgCuiBuffBuilderBuffer.h"
#include "swgClientUserInterface/SwgCuiChatRooms.h"
#include "swgClientUserInterface/SwgCuiClusterSelection.h"
#include "swgClientUserInterface/SwgCuiCollections.h"
#include "swgClientUserInterface/SwgCuiCommandBrowser.h"
#include "swgClientUserInterface/SwgCuiConsole.h"
#include "swgClientUserInterface/SwgCuiCraftAssembly.h"
#include "swgClientUserInterface/SwgCuiCraftCustomize.h"
#include "swgClientUserInterface/SwgCuiCraftDraft.h"
#include "swgClientUserInterface/SwgCuiCraftExperiment.h"
#include "swgClientUserInterface/SwgCuiCraftOption.h"
#include "swgClientUserInterface/SwgCuiCraftSummary.h"
#include "swgClientUserInterface/SwgCuiCredits.h"
#include "swgClientUserInterface/SwgCuiCustomize.h"
#include "swgClientUserInterface/SwgCuiCybernetics.h"
#include "swgClientUserInterface/SwgCuiDataStorage.h"
#include "swgClientUserInterface/SwgCuiDebugInfoPage.h"
#include "swgClientUserInterface/SwgCuiDpsMeter.h"
#include "swgClientUserInterface/SwgCuiDroidCommand.h"
#include "swgClientUserInterface/SwgCuiExamine.h"
#include "swgClientUserInterface/SwgCuiExpertise.h"
#include "swgClientUserInterface/SwgCuiExpMonitorBox.h"
#include "swgClientUserInterface/SwgCuiGalacticCivilWar.h"
#include "swgClientUserInterface/SwgCuiGameMenu.h"
#include "swgClientUserInterface/SwgCuiHarassmentReport.h"
#include "swgClientUserInterface/SwgCuiHudGround.h"
#include "swgClientUserInterface/SwgCuiHudSpace.h"
#include "swgClientUserInterface/SwgCuiHyperspaceMap.h"
#include "swgClientUserInterface/SwgCuiImageDesignerDesigner.h"
#include "swgClientUserInterface/SwgCuiImageDesignerRecipient.h"
#include "swgClientUserInterface/SwgCuiIMEIndicator.h"
#include "swgClientUserInterface/SwgCuiIMEIndicatorSpace.h"
#include "swgClientUserInterface/SwgCuiIMEInput.h"
#include "swgClientUserInterface/SwgCuiIncubator.h"
#include "swgClientUserInterface/SwgCuiInventory.h"
#include "swgClientUserInterface/SwgCuiKillMeter.h"
#include "swgClientUserInterface/SwgCuiLoadingGround.h"
#include "swgClientUserInterface/SwgCuiLoadingSpace.h"
#include "swgClientUserInterface/SwgCuiLocationDisplay.h"
#include "swgClientUserInterface/SwgCuiLocations.h"
#include "swgClientUserInterface/SwgCuiLoginScreen.h"
#include "swgClientUserInterface/SwgCuiLootBox.h"
#include "swgClientUserInterface/SwgCuiMahjong.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiMissionBrowser.h"
#include "swgClientUserInterface/SwgCuiMissionDetails.h"
#include "swgClientUserInterface/SwgCuiNetStatus.h"
#include "swgClientUserInterface/SwgCuiNews.h"
#include "swgClientUserInterface/SwgCuiNpeContinuation.h"
#include "swgClientUserInterface/SwgCuiOpt.h"
#include "swgClientUserInterface/SwgCuiPersistentMessageBrowser.h"
#include "swgClientUserInterface/SwgCuiPersistentMessageComposer.h"
#include "swgClientUserInterface/SwgCuiPlanetMap.h"
#include "swgClientUserInterface/SwgCuiPlayerQuestConversation.h"
#include "swgClientUserInterface/SwgCuiProfessionTemplateSelect.h"
#include "swgClientUserInterface/SwgCuiQuestAcceptance.h"
#include "swgClientUserInterface/SwgCuiQuestBuilder.h"
#include "swgClientUserInterface/SwgCuiQuestJournal.h"
#include "swgClientUserInterface/SwgCuiQuestHelper.h"
#include "swgClientUserInterface/SwgCuiRating.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_Hopper.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_Quantity.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_SetResource.h"
#include "swgClientUserInterface/SwgCuiResourceSplitter.h"
#include "swgClientUserInterface/SwgCuiRoadmap.h"
#include "swgClientUserInterface/SwgCuiSceneSelection.h"
#include "swgClientUserInterface/SwgCuiServerDisconnected.h"
#include "swgClientUserInterface/SwgCuiService_TicketList_AddComment.h"
#include "swgClientUserInterface/SwgCuiShipChoose.h"
#include "swgClientUserInterface/SwgCuiShipComponentDetail.h"
#include "swgClientUserInterface/SwgCuiShipComponentManagement.h"
#include "swgClientUserInterface/SwgCuiShipView.h"
#include "swgClientUserInterface/SwgCuiSpaceAssignPlayerFormation.h"
#include "swgClientUserInterface/SwgCuiSpaceConversation.h"
#include "swgClientUserInterface/SwgCuiSpaceGroupLaunch.h"
#include "swgClientUserInterface/SwgCuiSpaceMiningSale.h"
#include "swgClientUserInterface/SwgCuiSpaceZoneMap.h"
#include "swgClientUserInterface/SwgCuiSplash.h"
#include "swgClientUserInterface/SwgCuiStructurePlacement.h"
#include "swgClientUserInterface/SwgCuiTcgWindow.h"
#include "swgClientUserInterface/SwgCuiTicketPurchase.h"
#include "swgClientUserInterface/SwgCuiTrade.h"
#include "swgClientUserInterface/SwgCuiVendor.h"
#include "swgClientUserInterface/SwgCuiVendorSetPrice.h"
#include "swgClientUserInterface/SwgCuiVoiceActiveSpeakers.h"
#include "swgClientUserInterface/SwgCuiVoiceFlyBar.h"
#include "swgClientUserInterface/SwgCuiWaypointMonitorBox.h"
#include "swgClientUserInterface/SwgCuiWeaponGroupAssignment.h"

#include "UIManager.h"
#include "UIPage.h"

#include <vector>

// ======================================================================

namespace
{
	bool s_installed;
}

//-----------------------------------------------------------------

#define MAKE_SWG_CTOR(type,path) CuiMediatorFactory::addConstructor (CuiMediatorTypes::##type , new CuiMediatorFactory::Constructor <SwgCui##type        > (##path));
#define MAKE_SWG_CTOR_WS(type,path) CuiMediatorFactory::addConstructor (CuiMediatorTypes::WS_##type , new CuiMediatorFactory::Constructor <SwgCui##type        > (##path, true));

void SwgCuiMediatorFactorySetup::install () 
{
	DEBUG_FATAL (s_installed, ("already installed\n"));

	CuiMediatorFactory::addConstructor (CuiMediatorTypes::Backdrop,      new CuiMediatorFactory::Constructor <CuiBackdrop>          ("/Back"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::ColorPicker,   new CuiMediatorFactory::Constructor <CuiColorPicker>       ("DO NOT USE"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::Keypad,        new CuiMediatorFactory::Constructor <CuiKeypad>            ("DO NOT USE"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::HueObject,     new CuiMediatorFactory::Constructor <CuiHueObject>         ("DO NOT USE"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::Transfer,      new CuiMediatorFactory::Constructor <CuiTransfer>          ("DO NOT USE"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::BugSubmission, new CuiMediatorFactory::Constructor <CuiBugSubmissionPage> ("/Debug.Bug"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::PopupHelp,     new CuiMediatorFactory::Constructor <CuiPopupHelp>         ("/craft.help", true));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::Transition,    new CuiMediatorFactory::Constructor <CuiTransition>        ("/trans"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::InputBox,      new CuiMediatorFactory::Constructor <CuiInputBox>          ("/pda.inputbox", true));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::SetName,       new CuiMediatorFactory::Constructor <CuiSetName>           ("/pda.inputbox", true));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::Notepad,       new CuiMediatorFactory::Constructor <CuiNotepad>           ("/pda.notepad", true));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::DeleteSkillConfirmation,      new CuiMediatorFactory::Constructor <CuiDeleteSkillConfirmation>("/confirm"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::VehicleProto,  new CuiMediatorFactory::Constructor <CuiVehicleProto>      ("/debug.vproto"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::WS_ShipTargetInfo, new CuiMediatorFactory::Constructor <CuiShipTargetInfo>   ("/pda.ShipTargetInfo", true));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::WS_ShipSelfInfo, new CuiMediatorFactory::Constructor <CuiShipSelfInfo>   ("/pda.ShipSelfInfo", true));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::WS_BiographyEdit, new CuiMediatorFactory::Constructor<SwgCuiBiographyEdit>("/pda.EditBiography", true));

	MAKE_SWG_CTOR (AvatarCreation,        "/AvCre");

	bool progressive = false;
	DebugFlags::registerFlag(progressive,       "SwgClientUserInterface", "progressive");

	CuiMediatorFactory::addConstructor (CuiMediatorTypes::AvatarCustomize, new CuiMediatorFactory::Constructor <SwgCuiAvatarCustomize2>   ("/AvCust"));

	MAKE_SWG_CTOR (Credits,                           "/Credits");

	MAKE_SWG_CTOR_WS (AuctionBid,                     "/Auction.AuctionBid");
	MAKE_SWG_CTOR_WS (AuctionBuy,                     "/Auction.AuctionBuy");
	MAKE_SWG_CTOR_WS (AuctionDetails,                 "/Auction.AuctionDetails");
	MAKE_SWG_CTOR_WS (AuctionList,                    "/Auction.AuctionList");
	MAKE_SWG_CTOR_WS (AuctionSell,                    "/Auction.AuctionSell");
	MAKE_SWG_CTOR (AvatarLocation2,                   "/AvLoc2");
	MAKE_SWG_CTOR (AvatarSelection,                   "/AvSel");
	MAKE_SWG_CTOR (AvatarSetupProf,                   "/AvSetupProf");
	MAKE_SWG_CTOR (AvatarSummary,                     "/AvSum");
	MAKE_SWG_CTOR (AvatarSimple,                      "/AvSimple");
	MAKE_SWG_CTOR (AvatarProfessionTemplateSelect,    "/AvProfessionTemplateSelect");
	MAKE_SWG_CTOR (ClusterSelection,                  "/ClusterSel");

#if PRODUCTION==0
	MAKE_SWG_CTOR (Console,                           "/Console");
#endif

	MAKE_SWG_CTOR (DebugInfoPage,                     "/Debug.DebugInfo");
	MAKE_SWG_CTOR (GameMenu,                          "/GameMenu");
	MAKE_SWG_CTOR (HudGround,                         "/GroundHUD");
	MAKE_SWG_CTOR (HudSpace,                          "/HudSpace");
	MAKE_SWG_CTOR (Inventory,                         "/Inv");
	MAKE_SWG_CTOR (LoadingGround,                     "/Load2");
	MAKE_SWG_CTOR (Locations,                         "/Loc");
	MAKE_SWG_CTOR (LoginScreen,                       "/Login");
	MAKE_SWG_CTOR (MissionBrowser,                    "/Mission.Browser");
	MAKE_SWG_CTOR (MissionDetails,                    "/Mission.Details");
	MAKE_SWG_CTOR (News,                              "/News");
	MAKE_SWG_CTOR (SceneSelection,                    "/SceneSel");
	MAKE_SWG_CTOR (ServerDisconnected,                "/Debug.ServerDisconnected");
	MAKE_SWG_CTOR (IMEInput,                          "/Debug.IME");
	MAKE_SWG_CTOR (Splash,                            "/Splash");
	MAKE_SWG_CTOR (StructurePlacement,                "/Placement");
	MAKE_SWG_CTOR_WS (Vendor,                         "/Auction.AuctionList");
	MAKE_SWG_CTOR_WS (VendorSetPrice,                 "/Auction.AuctionsetPrice");
	MAKE_SWG_CTOR_WS (ChatRooms,                      "/pda.ChatRooms");
	MAKE_SWG_CTOR_WS (CommandBrowser,                 "/skill.commandbrowser");
	MAKE_SWG_CTOR_WS (CraftAssembly,                  "/Craft.Assembly");
	MAKE_SWG_CTOR_WS (CraftCustomize,                 "/Craft.Customize");
	MAKE_SWG_CTOR_WS (CraftDraft,                     "/Craft.Draft");
	MAKE_SWG_CTOR_WS (CraftExperiment,                "/Craft.Experiment");
	MAKE_SWG_CTOR_WS (CraftOption,                    "/Craft.Option");
	MAKE_SWG_CTOR_WS (CraftSummary,                   "/Craft.Summary");
	MAKE_SWG_CTOR_WS (Customize,                      "/pda.Customize");
	MAKE_SWG_CTOR_WS (DataStorage,                    "/pda.datapad");
	MAKE_SWG_CTOR_WS (Examine,                        "/pda.examine");
	MAKE_SWG_CTOR_WS (Expertise,                      "/pda.Expertise");
	MAKE_SWG_CTOR_WS (ExpMonitorBox,                  "/pda.expMon");
	MAKE_SWG_CTOR_WS (WaypointMonitorBox,             "/pda.waypointMon");
	MAKE_SWG_CTOR_WS (NetStatus,                      "/pda.netStatus");
	MAKE_SWG_CTOR_WS (Opt,                            "/Opt.OptMain");
	MAKE_SWG_CTOR_WS (PersistentMessageBrowser,       "/pda.pmBrowser");
	MAKE_SWG_CTOR_WS (PersistentMessageComposer,      "/pda.pmComposer");
	MAKE_SWG_CTOR_WS (PlanetMap,                      "/PlanetMap.planetMap");
	MAKE_SWG_CTOR_WS (ResourceExtraction,             "/res.HarvesterPage");
	MAKE_SWG_CTOR_WS (ResourceExtraction_Hopper,      "/res.HopperPage");
	MAKE_SWG_CTOR_WS (ResourceExtraction_Quantity,    "/res.QuantityPage");
	MAKE_SWG_CTOR_WS (ResourceExtraction_SetResource, "/res.SetResourcePage");
	MAKE_SWG_CTOR_WS (TicketPurchase,                 "/TicketPurchase.ticketPurchase");
	MAKE_SWG_CTOR_WS (Trade,                          "/pda.trade");
	MAKE_SWG_CTOR_WS (ImageDesignerDesigner,          "/pda.imagedesigner.designer");
	MAKE_SWG_CTOR_WS (ImageDesignerRecipient,         "/pda.imagedesigner.recipient");
	MAKE_SWG_CTOR_WS (QuestJournal,                   "/pda.Quest");
	MAKE_SWG_CTOR_WS (QuestAcceptance,                "/pda.QuestAcceptance");
	MAKE_SWG_CTOR_WS (QuestHelper,                    "/GroundHUD.QuestHelper");
	MAKE_SWG_CTOR_WS (LocationDisplay,                "/pda.LocationDisplay");
	MAKE_SWG_CTOR_WS (DpsMeter,                       "/pda.dpsMeter");
	MAKE_SWG_CTOR_WS (Roadmap,                        "/pda.Roadmap");
	MAKE_SWG_CTOR_WS (LootBox,                        "/pda.lootbox");
	MAKE_SWG_CTOR_WS (NpeContinuation,                "/pda.ShellBeach");
	MAKE_SWG_CTOR_WS (KillMeter,                      "/pda.KillMeter");
	MAKE_SWG_CTOR_WS (BuffBuilderBuffer,              "/pda.BuffBuilder.buffer");
	MAKE_SWG_CTOR_WS (BuffBuilderBuffee,              "/pda.BuffBuilder.buffee");
	MAKE_SWG_CTOR_WS (Incubator,                      "/pda.Incubator");
	MAKE_SWG_CTOR_WS (Collections,                    "/pda.Collections");
	MAKE_SWG_CTOR_WS (VoiceFlyBar,                    "/Voice.VoiceFlyBar");
	MAKE_SWG_CTOR_WS (VoiceActiveSpeakers,            "/Voice.VoiceActiveSpeakers");
	MAKE_SWG_CTOR_WS (TcgWindow,                      "/pda.Tcg");
	MAKE_SWG_CTOR_WS (Mahjong,                        "/pda.Mahjong");
	MAKE_SWG_CTOR_WS (AppearanceTab,                  "/pda.AppearanceTab");
	MAKE_SWG_CTOR_WS (QuestBuilder,					  "/pda.QuestBuilder");
	MAKE_SWG_CTOR_WS (Rating,                         "/pda.ratingScreen");
	MAKE_SWG_CTOR_WS (GalacticCivilWar,               "/pda.GCWInformation");

	//space
	MAKE_SWG_CTOR_WS (SpaceConversation,              "/space.convo");
	MAKE_SWG_CTOR_WS (HyperspaceMap,                  "/space.hyperspace");
	MAKE_SWG_CTOR_WS (ShipComponentDetail,            "/space.componentdetails");
	MAKE_SWG_CTOR_WS (SpaceAssignPlayerFormation,     "/space.formation");
	MAKE_SWG_CTOR_WS (SpaceZoneMap,                   "/space.ZoneMap");
	MAKE_SWG_CTOR_WS (WeaponGroupAssignment,          "/space.weapongroupassignment");
	MAKE_SWG_CTOR_WS (PlayerQuestConversation,        "/space.playerQuest");

	CuiMediatorFactory::addConstructor (CuiMediatorTypes::LoadingSpace, new CuiMediatorFactory::Constructor <SwgCuiLoadingSpace> ("/SpaceLoading"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::LoadingMustafar , new CuiMediatorFactory::Constructor <SwgCuiLoadingSpace> ("/MustaLoading"));
	CuiMediatorFactory::addConstructor (CuiMediatorTypes::LoadingHoth, new CuiMediatorFactory::Constructor <SwgCuiLoadingSpace> ("/HothLoading"));

	MAKE_SWG_CTOR_WS (ShipComponentManagement,        "/space.shipcomponents");
	MAKE_SWG_CTOR_WS (ShipChoose,                     "/space.shipchoose");
	MAKE_SWG_CTOR_WS (ShipView,                       "/space.shipview");
	MAKE_SWG_CTOR_WS (DroidCommand,                   "/PDA.droidcommand");
	MAKE_SWG_CTOR_WS (SpaceGroupLaunch,               "/space.spaceGroupLaunch");
	MAKE_SWG_CTOR_WS (Cybernetics,                    "/PDA.cybernetics");
	MAKE_SWG_CTOR_WS (SpaceMiningSale,                "/space.mining_sale");
	//end space

	CuiMediatorFactory::addConstructor (CuiMediatorTypes::WS_TicketListAddComment, new CuiMediatorFactory::Constructor <SwgCuiService::TicketList::AddComment> ("/PDA.Service.edit", true));
	MAKE_SWG_CTOR (ResourceSplitter,                  "/pda.SplitResource.main");
	MAKE_SWG_CTOR_WS (HarassmentReport,               "/pda.Service.harassment");
	
	MAKE_SWG_CTOR_WS (IMEIndicator,                   "/pda.ime");
	MAKE_SWG_CTOR_WS (IMEIndicatorSpace,              "/pda.imeSpace");

	MAKE_SWG_CTOR_WS (ProfessionTemplateSelect,	      "/Skill.professionTemplate");

	//register the mediators for ui minigames
	CuiMinigameManager::registerMediatorForGame(SwgCuiMahjong::cms_gameName, CuiMediatorTypes::WS_Mahjong);

	s_installed = true;
}

//-----------------------------------------------------------------

void SwgCuiMediatorFactorySetup::remove ()
{
	CuiMinigameManager::unregisterMediatorForGame(SwgCuiMahjong::cms_gameName);

	DEBUG_FATAL (!s_installed, ("not installed\n"));
	s_installed = false;
}

// ======================================================================
