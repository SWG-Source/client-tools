// ======================================================================
//
// SwgCuiMediatorTypes.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiMediatorTypes_H
#define INCLUDED_SwgCuiMediatorTypes_H

#include "clientUserInterface/CuiMediatorTypes.h"

// ======================================================================
#define MAKE_MEDIATOR_TYPE(a) Type const a = #a

namespace CuiMediatorTypes
{
	MAKE_MEDIATOR_TYPE (WS_AuctionBid);
	MAKE_MEDIATOR_TYPE (WS_AuctionBuy);
	MAKE_MEDIATOR_TYPE (WS_AuctionDetails);
	MAKE_MEDIATOR_TYPE (WS_AuctionList);
	MAKE_MEDIATOR_TYPE (WS_AuctionSell);
	MAKE_MEDIATOR_TYPE (AvatarCreation);
	MAKE_MEDIATOR_TYPE (AvatarCustomize);
	MAKE_MEDIATOR_TYPE (AvatarLocation);
	MAKE_MEDIATOR_TYPE (AvatarLocation2);
	MAKE_MEDIATOR_TYPE (AvatarSelection);
	MAKE_MEDIATOR_TYPE (AvatarSetupProf);
	MAKE_MEDIATOR_TYPE (AvatarProfessionTemplateSelect);
	MAKE_MEDIATOR_TYPE (AvatarSummary);
	MAKE_MEDIATOR_TYPE (AvatarSimple);
	MAKE_MEDIATOR_TYPE (CharacterSheet);
	MAKE_MEDIATOR_TYPE (ClusterSelection);
	MAKE_MEDIATOR_TYPE (Community);
	MAKE_MEDIATOR_TYPE (ControlsMenu);
	MAKE_MEDIATOR_TYPE (DebugInfoPage);
	MAKE_MEDIATOR_TYPE (GameMenu);
	MAKE_MEDIATOR_TYPE (HudGround);
	MAKE_MEDIATOR_TYPE (HudSpace);
	MAKE_MEDIATOR_TYPE (Inventory);
	MAKE_MEDIATOR_TYPE (LoadingGround);
	MAKE_MEDIATOR_TYPE (Locations);
	MAKE_MEDIATOR_TYPE (LoginScreen);
	MAKE_MEDIATOR_TYPE (MissionBrowser);
	MAKE_MEDIATOR_TYPE (MissionDetails);
	MAKE_MEDIATOR_TYPE (News);
	MAKE_MEDIATOR_TYPE (SceneSelection);
	MAKE_MEDIATOR_TYPE (ServerDisconnected);
	MAKE_MEDIATOR_TYPE (Service);
	MAKE_MEDIATOR_TYPE (ShipSelection);
	MAKE_MEDIATOR_TYPE (StructurePlacement);
	MAKE_MEDIATOR_TYPE (WS_Vendor);
	MAKE_MEDIATOR_TYPE (WS_VendorSetPrice);
	MAKE_MEDIATOR_TYPE (WS_ChatRooms);
	MAKE_MEDIATOR_TYPE (WS_CommandBrowser);
	MAKE_MEDIATOR_TYPE (WS_CraftAssembly);
	MAKE_MEDIATOR_TYPE (WS_CraftCustomize);
	MAKE_MEDIATOR_TYPE (WS_CraftDraft);
	MAKE_MEDIATOR_TYPE (WS_CraftExperiment);
	MAKE_MEDIATOR_TYPE (WS_CraftOption);
	MAKE_MEDIATOR_TYPE (WS_CraftSummary);
	MAKE_MEDIATOR_TYPE (WS_Customize);
	MAKE_MEDIATOR_TYPE (WS_DataStorage);
	MAKE_MEDIATOR_TYPE (WS_Examine);
	MAKE_MEDIATOR_TYPE (WS_Expertise);
	MAKE_MEDIATOR_TYPE (WS_ExpMonitorBox);
	MAKE_MEDIATOR_TYPE (WS_NetStatus);
	MAKE_MEDIATOR_TYPE (WS_Opt);
	MAKE_MEDIATOR_TYPE (WS_PersistentMessageBrowser);
	MAKE_MEDIATOR_TYPE (WS_PersistentMessageComposer);
	MAKE_MEDIATOR_TYPE (WS_PlanetMap);
	MAKE_MEDIATOR_TYPE (WS_ResourceExtraction);
	MAKE_MEDIATOR_TYPE (WS_ResourceExtraction_Hopper);
	MAKE_MEDIATOR_TYPE (WS_ResourceExtraction_Quantity);
	MAKE_MEDIATOR_TYPE (WS_ResourceExtraction_SetResource);
	MAKE_MEDIATOR_TYPE (WS_TicketListAddComment);
	MAKE_MEDIATOR_TYPE (WS_TicketPurchase);
	MAKE_MEDIATOR_TYPE (WS_WaypointMonitorBox);
	MAKE_MEDIATOR_TYPE (WS_QuestJournal);
	MAKE_MEDIATOR_TYPE (WS_QuestAcceptance);
	MAKE_MEDIATOR_TYPE (WS_QuestHelper);
	MAKE_MEDIATOR_TYPE (WS_Roadmap);
	MAKE_MEDIATOR_TYPE (WS_Trade);
	MAKE_MEDIATOR_TYPE (Credits);
	MAKE_MEDIATOR_TYPE (WS_HarassmentReport);
	MAKE_MEDIATOR_TYPE (WS_ImageDesignerDesigner);
	MAKE_MEDIATOR_TYPE (WS_ImageDesignerRecipient);
	MAKE_MEDIATOR_TYPE (WS_StatMigration);
	MAKE_MEDIATOR_TYPE (WS_DroidCommand);
	MAKE_MEDIATOR_TYPE (WS_SpaceGroupLaunch);
	MAKE_MEDIATOR_TYPE (WS_BiographyEdit);
	MAKE_MEDIATOR_TYPE (WS_GroupLootLottery);
	MAKE_MEDIATOR_TYPE (WS_Cybernetics);
	MAKE_MEDIATOR_TYPE (WS_ProfessionTemplateSelect);
	MAKE_MEDIATOR_TYPE (WS_LootBox);
	MAKE_MEDIATOR_TYPE (WS_NpeContinuation);
	MAKE_MEDIATOR_TYPE (WS_LocationDisplay);
	MAKE_MEDIATOR_TYPE (WS_DpsMeter);
	MAKE_MEDIATOR_TYPE (WS_KillMeter);
	MAKE_MEDIATOR_TYPE (WS_BuffBuilderBuffer);
	MAKE_MEDIATOR_TYPE (WS_BuffBuilderBuffee);
	MAKE_MEDIATOR_TYPE (WS_Incubator);
	MAKE_MEDIATOR_TYPE (WS_Collections);
	MAKE_MEDIATOR_TYPE (WS_VoiceFlyBar);
	MAKE_MEDIATOR_TYPE (WS_VoiceActiveSpeakers);
	MAKE_MEDIATOR_TYPE (WS_TcgWindow);
	MAKE_MEDIATOR_TYPE (WS_Mahjong);
	MAKE_MEDIATOR_TYPE (WS_AppearanceTab);
	MAKE_MEDIATOR_TYPE (WS_QuestBuilder);
	MAKE_MEDIATOR_TYPE (WS_Rating);
	MAKE_MEDIATOR_TYPE (WS_PlayerQuestConversation);
	MAKE_MEDIATOR_TYPE (WS_GalacticCivilWar);

	//space
	MAKE_MEDIATOR_TYPE (WS_ShipTargetInfo);
	MAKE_MEDIATOR_TYPE (WS_ShipSelfInfo);
	MAKE_MEDIATOR_TYPE (WS_ShipComponentManagement);
	MAKE_MEDIATOR_TYPE (WS_SpaceConversation);
	MAKE_MEDIATOR_TYPE (WS_HyperspaceMap);
	MAKE_MEDIATOR_TYPE (WS_ShipComponentDetail);
	MAKE_MEDIATOR_TYPE (WS_SpaceAssignPlayerFormation);
	MAKE_MEDIATOR_TYPE (WS_SpaceZoneMap);
	MAKE_MEDIATOR_TYPE (WS_WeaponGroupAssignment);
	MAKE_MEDIATOR_TYPE (LoadingSpace);
	MAKE_MEDIATOR_TYPE (LoadingMustafar);
	MAKE_MEDIATOR_TYPE (LoadingHoth);
	MAKE_MEDIATOR_TYPE (WS_ShipChoose);
	MAKE_MEDIATOR_TYPE (WS_ShipView);
	MAKE_MEDIATOR_TYPE (WS_IMEIndicator);
	MAKE_MEDIATOR_TYPE (WS_IMEIndicatorSpace);
	MAKE_MEDIATOR_TYPE (WS_SpaceMiningSale);
	//endspace
};

#undef MAKE_MEDIATOR_TYPE

// ======================================================================

#endif
