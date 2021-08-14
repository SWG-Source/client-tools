// ======================================================================
//
// CommandCppFuncs.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CommandCppFuncs.h"

#include "clientDirectInput/DirectInput.h"
#include "clientGame/ChatLogManager.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientCommandChecks.h"
#include "clientGame/ClientMacroManager.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientObjectUsabilityManager.h"
#include "clientGame/ClientStringIds.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GroupManager.h"
#include "clientGame/GroupObject.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/HardpointObject.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiConsoleHelper.h"
#include "clientUserInterface/CuiConversationManager.h"
#include "clientUserInterface/CuiInstantMessageManager.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiMessageQueueManager.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiRecipeManager.h"
#include "clientUserInterface/CuiSecureTradeManager.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsCommand.h"
#include "clientUserInterface/CuiStringIdsCommunity.h"
#include "clientUserInterface/CuiStringIdsGameLanguage.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiVoiceChatManager.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Footprint.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/GameLanguageManager.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"
#include "sharedNetworkMessages/AddMapLocationMessage.h"
#include "sharedNetworkMessages/AppendCommentMessage.h"
#include "sharedNetworkMessages/CancelTicketMessage.h"
#include "sharedNetworkMessages/ConnectPlayerMessage.h"
#include "sharedNetworkMessages/CreateTicketMessage.h"
#include "sharedNetworkMessages/DisconnectPlayerMessage.h"
#include "sharedNetworkMessages/GetArticleMessage.h"
#include "sharedNetworkMessages/GetCommentsMessage.h"
#include "sharedNetworkMessages/GetMapLocationsMessage.h"
#include "sharedNetworkMessages/GetTicketsMessage.h"
#include "sharedNetworkMessages/MessageQueuePosture.h"
#include "sharedNetworkMessages/MessageQueueSitOnObject.h"
#include "sharedNetworkMessages/MessageQueueString.h"
#include "sharedNetworkMessages/RequestCategoriesMessage.h"
#include "sharedNetworkMessages/SearchKnowledgeBaseMessage.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgSharedUtility/Postures.def"

#include "UnicodeUtils.h"
#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsCommandCpp
{
	MAKE_STRING_ID(ui_opt, controller_found);
	MAKE_STRING_ID(ui_opt, controller_notfound);
};

#undef MAKE_STRING_ID

//----------------------------------------------------------------------

namespace CommandCppFuncsNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string const cs_sitServerCommandName("sitServer");
	std::string const cs_jumpServerCommandName("jumpServer");
	std::string const cs_logoutServerCommandName("logoutServer");
	std::string const cs_waypointServerCommandName("waypointServer");

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ClientObject * findClientObjectFromParam (const Unicode::String & param)
	{
		if (!param.empty () && param [0] == '@')
		{
			const uint32 uniqueId = static_cast<uint32>(atol (Unicode::wideToNarrow (param.substr (1)).c_str ()));
			return ClientObject::findClientObjectByUniqueId (uniqueId);
		}

		return 0;
	}

	CreatureObject const *getCreatureOrPilot(NetworkId const &id)
	{
		Object const * const o = NetworkIdManager::getObjectById(id);
		if (o)
		{
			ClientObject const * const co = o->asClientObject();
			if (co->asCreatureObject())
				return co->asCreatureObject();
			if (co->asShipObject())
				return co->asShipObject()->getPilot();
		}
		return 0;
	}

	//----------------------------------------------------------------------


	void  commandFuncTest                  (Command const &,             NetworkId const &,         NetworkId const &, Unicode::String const &);
	void  commandFuncPosture               (Command const & command,     NetworkId const & actor,   NetworkId const &, Unicode::String const &);
	void  commandFuncSitClient             (Command const &            , NetworkId const & actorId, NetworkId const &          , Unicode::String const &);
	void  commandFuncJumpClient            (Command const &            , NetworkId const & actorId, NetworkId const &          , Unicode::String const &);
	void  follow                           (Command const &            , NetworkId const & actorId, NetworkId const & targetId, Unicode::String const &);
	void  stopFollow                       (Command const &            , NetworkId const & actorId, NetworkId const &          , Unicode::String const &);
	void  commandFuncAddMapLocation        (Command const &            , NetworkId const & actorId, NetworkId const &          , Unicode::String const &);
	void  commandFuncGetMapLocations       (Command const &            , NetworkId const & actorId, NetworkId const &          , Unicode::String const &);

	void  commandFuncInstantMessageTell    (Command const &,             NetworkId const &,         NetworkId const &, Unicode::String const &);
	void  commandFuncInstantMessageTtell   (Command const &,             NetworkId const &,         NetworkId const &, Unicode::String const &);
	void  commandFuncInstantMessageRetell  (Command const &,             NetworkId const &,         NetworkId const &, Unicode::String const &);
	void  commandFuncInstantMessageReply   (Command const &,             NetworkId const &,         NetworkId const &, Unicode::String const &);

	void commandFuncAttack                 (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const &);
	void commandFuncExamine                (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncExamineCharacterSheet  (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncTrade                  (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncTradeAccept            (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncPickup                 (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncEquip                  (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncUnequip                (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncDrop                   (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncConversationStart      (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncConversationStop       (Command const & command, NetworkId const & , NetworkId const &, Unicode::String const &);
	void commandFuncFindControllers        (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncShowBackpack		   (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncShowHelmet			   (Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);

	void commandFuncActivate               (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncDeactivate             (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncOpen                   (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncOpenNewWindow          (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncItemMail               (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncAuctionUse             (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncSplit                  (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &);
	void commandFuncDestroyObject          (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &);

	void commandFuncJoin                   (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);
	void commandFuncDecline                (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);

	void commandFuncSurvey                 (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);
	void commandFuncGetSample              (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);
	void commandFuncConsider               (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);

	void commandFuncLanguage               (Command const & , NetworkId const & actorId, NetworkId const & targetId, Unicode::String const &);
	void commandFuncLogChat                (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);
	void commandFuncFriendComment          (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);
	void commandFuncFriendGroup            (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);
	void commandFuncFriend                 (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);
	void commandFuncFriendList             (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);
	void commandFuncIgnore                 (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);
	void commandFuncIgnoreList             (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);

	void commandFuncAssist                 (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);


	void commandFuncNotepad                (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &);

	void commandFuncSetScale               (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncSetYaw                 (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncHideMe                 (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncReloadAdminTable       (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncSkills                 (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncSpawn                  (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncDestroy                (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncSysgroup               (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncEditBank               (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncEditInventory          (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncEditDatapad            (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncExamineObjVars         (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncRenamePlayer           (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncDraw                   (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncUiDebugExamine         (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncUnload                 (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncEmptyMail              (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncTargetPilot            (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncEditBiography          (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);


	void commandFuncEcho                   (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncMacro                  (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncDumpPausedCommands     (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncMailSave               (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncTarget                 (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);

	//space
	void commandFuncShipManageComponents   (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncBoosterToggle(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncThrottle(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncMatchSpeed(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncWaypointAutopilot(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncHyperspaceMap(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
#ifdef ENABLE_FORMATIONS
	void commandFuncSpaceAssignPlayerFormation(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncEnterFormation(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncLeaveFormation(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
#endif
	void commandFuncOpenCloseWings(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncComponentDetail(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncZoneMap(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncWeaponGroupAssignment(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncQuestJournal(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncChatterSpeech(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncLogout(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncWaypoint(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	//endspace

	void commandFuncExternalCommand(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncCollections(Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);

	void commandFuncVoiceInvite(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncVoiceKick(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);

	//void commandFuncTcg(Command const & command, NetworkId const & , NetworkId const & target, Unicode::String const &);

	void commandFuncToggleOutOfCharacter(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const &);

	void commandFuncEquipAppearance(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncUnequipAppearance(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);

	void commandFuncAppearanceTab(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);

	void commandFuncQuestBuilder(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncOpenStorytellerRecipe(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
	void commandFuncGCWInfo(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);

	void commandFuncToggleLookingForWork(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params);
}

using namespace CommandCppFuncsNamespace;

// ======================================================================

void CommandCppFuncsNamespace::commandFuncTest (Command const &, NetworkId const &, NetworkId const &, Unicode::String const &)
{
	WARNING(true, ("client command table test function called"));
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncPosture (Command const & command, NetworkId const & actor, NetworkId const &, Unicode::String const &)
{
	if (Game::getSinglePlayer ())
	{
		static const uint32 hash_prone = Crc::normalizeAndCalculate ("prone");
		static const uint32 hash_kneel = Crc::normalizeAndCalculate ("kneel");
		static const uint32 hash_stand = Crc::normalizeAndCalculate ("stand");

		CreatureObject * const creature = dynamic_cast<CreatureObject *>(CachedNetworkId (actor).getObject ());

		if (creature)
		{
			if (command.m_commandHash == hash_prone)
			{
				creature->requestServerPostureChange (Postures::Prone);
			}
			else if (command.m_commandHash == hash_kneel)
			{
				creature->requestServerPostureChange (Postures::Crouched);
			}
			else if (command.m_commandHash == hash_stand)
			{
				creature->requestServerPostureChange (Postures::Upright);
			}
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncAttack (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	if (Game::getSinglePlayer ())
	{
		IGNORE_RETURN(CuiCombatManager::simulateSinglePlayerCombat(target));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncExamine (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & params)
{
	if (target.isValid ())
		CuiActionManager::performAction (CuiActions::examine, Unicode::narrowToWide (target.getValueString ()));
	else
		CuiActionManager::performAction (CuiActions::examine, params);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncExamineCharacterSheet (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & params)
{
	if (target.isValid ())
		CuiActionManager::performAction (CuiActions::examineCharacterSheet, Unicode::narrowToWide (target.getValueString ()));
	else
		CuiActionManager::performAction (CuiActions::examineCharacterSheet, params);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncSurvey (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	CuiActionManager::performAction (CuiActions::survey, Unicode::narrowToWide (""));
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncGetSample (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	CuiActionManager::performAction (CuiActions::getSample, Unicode::narrowToWide (""));
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncTrade (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	CuiSecureTradeManager::initiateTrade (target, NetworkId::cms_invalid);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncTradeAccept (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	CuiSecureTradeManager::initiateTrade (target, NetworkId::cms_invalid);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncPickup (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	CuiInventoryManager::pickupObject (target);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncEquip (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	CuiInventoryManager::equipObject (target);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncUnequip (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	CuiInventoryManager::unequipObject (target);
}

void CommandCppFuncsNamespace::commandFuncEquipAppearance(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	UNREF(command);
	UNREF(actor);
	UNREF(params);

	if(Game::getPlayerObject())
	{
		Object * const targetObject = NetworkIdManager::getObjectById(target);
		bool equipAllowed = false;
		if(targetObject && targetObject->asClientObject())
		{
			TangibleObject * const tangibleTarget = targetObject->asClientObject()->asTangibleObject();

			if(!tangibleTarget)
				return;

			const int got = tangibleTarget->getGameObjectType();

			if(( GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_armor) ||	GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_clothing)||
				GameObjectTypes::isTypeOf (got, SharedObjectTemplate::GOT_jewelry) || got == SharedObjectTemplate::GOT_misc_container_wearable) && ClientObjectUsabilityManager::canWear(Game::getPlayerCreature(), tangibleTarget))
			{
				equipAllowed = true;
			}
			else if(got == SharedObjectTemplate::GOT_misc_appearance_only || got == SharedObjectTemplate::GOT_misc_appearance_only_invisible)
			{
				equipAllowed = true;
			}
		}

		if(equipAllowed)
		{
			CuiInventoryManager::equipAppearanceItem(target);
			return;
		}
	}

	if(PlayerObject::isAdmin())
	{
		CuiInventoryManager::equipAppearanceItem(target);
	}
}

void CommandCppFuncsNamespace::commandFuncUnequipAppearance(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	UNREF(command);
	UNREF(actor);
	UNREF(params);
	CuiInventoryManager::unequipAppearanceItem(target);
}
//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncFindControllers(Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	int totalJoysticks = DirectInput::getNumberOfJoysticksAvailable();

	DirectInput::reaquireJoystick();

	if(totalJoysticks < DirectInput::getNumberOfJoysticksAvailable())
		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsCommandCpp::controller_found.localize());
	else
		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsCommandCpp::controller_notfound.localize());
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncShowBackpack(Command const & /*command*/, NetworkId const & , NetworkId const & /*target*/, Unicode::String const &)
{
	if(Game::getPlayerObject())
		Game::getPlayerObject()->sendBackpackMsg(!(Game::getPlayerObject()->getShowBackpack()));
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncShowHelmet(Command const & /*command*/, NetworkId const & , NetworkId const & /*target*/, Unicode::String const &)
{
	if(Game::getPlayerObject())
		Game::getPlayerObject()->sendHelmetMsg(!(Game::getPlayerObject()->getShowHelmet()));
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncDrop (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	CuiInventoryManager::dropObject (target);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncConversationStart (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	CuiConversationManager::start (target);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncConversationStop (Command const & , NetworkId const & , NetworkId const &, Unicode::String const &)
{
	CuiConversationManager::stop ();
}


//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncActivate (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	ClientWaypointObject * const waypoint = dynamic_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById (target));
	if (waypoint)
	{
		if (!waypoint->setWaypointActive (true))
		{
			if (!strncmp("space_",waypoint->getPlanetName().c_str(),6))
				CuiSystemMessageManager::sendFakeSystemMessage (SharedStringIds::waypoint_in_other_space_zone.localize());
			else
				CuiSystemMessageManager::sendFakeSystemMessage (SharedStringIds::waypoint_on_other_planet.localize());
		}
	}
	else
	{
		ClientMissionObject * mission = dynamic_cast<ClientMissionObject *>(NetworkIdManager::getObjectById (target));
		if (mission)
		{
			Container* c = ContainerInterface::getContainer(*mission);
			for(ContainerIterator i = c->begin(); i != c->end(); ++i)
			{
				Container::ContainedItem item = *i;
				Object* o = item.getObject();
				ClientWaypointObject * w = dynamic_cast<ClientWaypointObject *>(o);
				if(w)
				{
					if (!w->setWaypointActive (true))
						CuiSystemMessageManager::sendFakeSystemMessage (SharedStringIds::mission_on_other_planet.localize());
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncDeactivate (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	ClientWaypointObject * const waypoint = dynamic_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById (target));
	if (waypoint)
		IGNORE_RETURN(waypoint->setWaypointActive (false));
	else
	{
		ClientMissionObject * mission = dynamic_cast<ClientMissionObject *>(NetworkIdManager::getObjectById (target));
		if (mission)
		{
			Container* c = ContainerInterface::getContainer(*mission);
			for(ContainerIterator i = c->begin(); i != c->end(); ++i)
			{
				Container::ContainedItem item = *i;
				Object* o = item.getObject();
				ClientWaypointObject* w = dynamic_cast<ClientWaypointObject*>(o);
				if(w)
				{
					IGNORE_RETURN(w->setWaypointActive (false));
					break;
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncOpen (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	ClientObject * const clientObject = dynamic_cast<ClientObject *>(NetworkIdManager::getObjectById (target));
	if (clientObject)
		CuiInventoryManager::requestItemOpen (*clientObject, std::string (), 0, 0, false, false);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncOpenNewWindow (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	ClientObject * const clientObject = dynamic_cast<ClientObject *>(NetworkIdManager::getObjectById (target));
	if (clientObject)
		CuiInventoryManager::requestItemOpen (*clientObject, std::string (), 0, 0, true, false);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncItemMail               (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	ClientObject * const clientObject = dynamic_cast<ClientObject *>(NetworkIdManager::getObjectById (target));
	if (clientObject)
		CuiPersistentMessageManager::attachObjectToComposeWindow (*clientObject);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncAuctionUse             (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	ClientObject * const clientObject = dynamic_cast<ClientObject *>(NetworkIdManager::getObjectById (target));
	if (clientObject)
	{
		const int got = clientObject->getGameObjectType ();

		if (got == SharedObjectTemplate::GOT_terminal_bazaar)
			CuiActionManager::performAction  (CuiActions::terminalAuctionUse, Unicode::narrowToWide (clientObject->getNetworkId ().getValueString ()));
		else if (got == SharedObjectTemplate::GOT_vendor)
			CuiActionManager::performAction  (CuiActions::vendorUse, Unicode::narrowToWide (clientObject->getNetworkId ().getValueString ()));
		else
		{
			TangibleObject * const tangible = dynamic_cast<TangibleObject *>(clientObject);
			if (tangible && tangible->hasCondition (TangibleObject::C_vendor))
				CuiActionManager::performAction  (CuiActions::vendorUse, Unicode::narrowToWide (clientObject->getNetworkId ().getValueString ()));
		}
	}

}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncSplit                  (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	CuiInventoryManager::splitContainer (target);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncDestroyObject           (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	CuiInventoryManager::destroyObject (target, false);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncJoin           (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	GroupManager::handleInviteResponse (true);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncDecline          (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	GroupManager::handleInviteResponse (false);
}

// ----------------------------------------------------------------------
/**
 * Check if the player is requesting to sit in a chair, and forward
 * the request on to the server.
 *
 * The flow of the /sit command is a little confusing.  Simplify it if
 * you can!  Here's how it works.
 *
 * - The player executes /sit or presses the sit button.
 *
 * - The client checks if the player has a look-at target.  If there
 *   is a look-at target and if the target has the seating hardpoint,
 *   the player runs the /serverSit command with the client-side-only
 *   network id for the chair as the target.  Otherwise the /serverSit
 *   command is executed with no target.
 *
 * - The /serverSit command handler allows the "sit" script hook to be
 *   called, possibly preventing execution of the command.
 *
 * - If the "sit" script allows continuation of the command, the Cpp
 *   hook function will broadcast a CM_sitOnObject command to all clients.
 *   The Cpp hook sets the sit on chair state if the target network id
 *   is not the invalid value.
 *
 * - The CM_sitOnObject command handles doing a sit on a specified
 *   piece of furniture.  If the specified object id is invalid or does
 *   not yield an appearance with the proper hardpoint, the player sits
 *   on the ground; otherwise, the player sits on the chair.6
 */

void CommandCppFuncsNamespace::commandFuncSitClient (Command const &, NetworkId const & actorId, NetworkId const & targetId, Unicode::String const & params)
{
	//-- Get the look-at target for the actor.
	// Get the actor as CreatureObject.
	CreatureObject * const actorObject = dynamic_cast<CreatureObject *> (NetworkIdManager::getObjectById (actorId));
	if (!actorObject)
	{
		DEBUG_WARNING (true, ("commandFuncSitClient: cannot make a non-present or non-CreatureObject [id=%s] to sit.", actorId.getValueString ().c_str ()));
		return;
	}

	const TangibleObject * targetObject = 0;

	// Favoring the parameter string because it contains the client-side object which we want to have precedence over the server-side
	// because of NGE-related look-at target changes which were confusing this command. -- ARH
	if (!params.empty ())
	{
		targetObject = dynamic_cast<TangibleObject *>(findClientObjectFromParam (params));
	}
	else if (targetId.isValid ())
		targetObject = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById (targetId));

	// Get the look-at target.
	NetworkId  chairNetworkId;

	if (!targetObject)
		targetObject = dynamic_cast<TangibleObject *>(actorObject->getLookAtTarget ().getObject ());

	if (targetObject)
	{
		if (!targetObject->canSitOn())
		{
			targetObject = 0;
			DEBUG_REPORT_LOG (true, ("commandFuncSitClient: ignoring target object since it doesn't appear to be a chair.  Sitting on ground.\n"));
		}
		else if (static_cast<const ClientObject*>(targetObject) != actorObject->getClosestChair())
		{
			//-- @todo convert this to a sendFakeSystemMessage so player knows what's going on.
			CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIds::chair_out_of_range.localize ());
			DEBUG_REPORT_LOG(true, ("commandFuncSitClient: ignoring target chair because it is not the closest chair to the actor.\n"));
			return;
		}
		else if (targetObject->getPosition_w().magnitudeBetweenSquared(actorObject->getPosition_w()) > MessageQueueSitOnObject::cs_maximumChairRangeSquared)
		{
			CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIds::chair_out_of_range.localize ());
			return;
		}
		else
		{
			//-- Keep track of chair's network id.
			if (targetObject->getNetworkId ().isValid ())
				chairNetworkId = targetObject->getNetworkId ();
			else
				chairNetworkId = actorId;
		}
	}

	//-- Don't allow sitting on the ground while in water.
	if (!chairNetworkId.isValid ())
	{
		const TerrainObject* const     terrainObject     = TerrainObject::getConstInstance();
		const CollisionProperty* const collisionProperty = actorObject->getCollisionProperty ();
		const bool                     isOnSolidFloor    = collisionProperty && collisionProperty->getFootprint() && collisionProperty->getFootprint()->isOnSolidFloor();

		Vector normal = Vector::unitY;
		if (terrainObject && actorObject->isInWorldCell() && !isOnSolidFloor)
		{
			const Vector position = actorObject->getPosition_w ();

			float terrainHeight;
			if (terrainObject->getHeight(position, terrainHeight, normal))
			{
				//-- see if the object is swimming
				float waterHeight;
				if (terrainObject->getWaterHeight(position, waterHeight))
				{
					if (waterHeight >terrainHeight)
					{
						// Client is in the water, abort the command.
						CuiSystemMessageManager::sendFakeSystemMessage (SharedStringIds::no_sitting_in_water.localize ());
						return;
					}
				}
			}
		}
	}

	bool const sittingInChair = chairNetworkId.isValid ();

#if PRODUCTION == 0
	if (Game::getSinglePlayer())
	{
		actorObject->setState(States::SittingOnChair, sittingInChair);

		if (sittingInChair)
		{
			NOT_NULL(targetObject);
			MessageQueueSitOnObject* const message = new MessageQueueSitOnObject(targetObject->isInWorldCell() ? NetworkId::cms_invalid : targetObject->getParentCell()->getOwner().getNetworkId(), targetObject->getPosition_w());
			actorObject->getController()->appendMessage(static_cast<int>(CM_sitOnObject), 0.f, message, 0);
		}
		else
		{
			MessageQueuePosture* const message = new MessageQueuePosture(Postures::Sitting, true);
			actorObject->getController()->appendMessage(static_cast<int>(CM_setPosture), 0.f, message, 0);
		}
	}
	else
#endif
	{
		//@todo -TRF- I think this can be stripped out now.
		if (chairNetworkId == actorId && targetObject)
		{
			PlayerObject * const playerObject = actorObject->getPlayerObject ();
			if (playerObject)
				playerObject->setLastSitOnAttemptUniqueId (targetObject->getUniqueId ());
		}

		//@todo -TRF- I think we no longer need to differentiate based on target parameter filling: we no longer use target parameter.
		//            We do all differentiation based on parameter string.
		if (!sittingInChair)
		{
			//-- Handle sitting on the ground.
			IGNORE_RETURN(ClientCommandQueue::enqueueCommand(cs_sitServerCommandName, NetworkId::cms_invalid, Unicode::emptyString));
		}
		else if (NULL != targetObject)
		{
			//-- Handle sitting on a chair.
			IGNORE_RETURN(ClientCommandQueue::enqueueCommand(cs_sitServerCommandName, NetworkId::cms_invalid, targetObject->buildChairParameterString()));
		}
	}
}

// ----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncJumpClient (Command const & command, NetworkId const & actorId, NetworkId const &, Unicode::String const &)
{
	CreatureObject * actorObject = dynamic_cast<CreatureObject *> (NetworkIdManager::getObjectById (actorId));
	if (!actorObject)
	{
		DEBUG_WARNING (true, ("commandFuncJumpClient: cannot make a non-present or non-CreatureObject [id=%s] to jump.", actorId.getValueString ().c_str ()));
		return;
	}

	if(actorObject->getMountedCreature() != NULL)
	{
		Object const * hardPoint = actorObject->getAttachedTo();
		HardpointObject const * hardpointObj = dynamic_cast<HardpointObject const *>(hardPoint);
		std::string hardpointName = hardpointObj ? hardpointObj->getHardpointName().getString() : "none";

		// Only the driver can make a mount/vehicle jump.
		if(!hardpointName.empty() && hardpointName.find("passenger") != std::string::npos)
			return;

		actorObject = actorObject->getMountedCreature();
	}

	// @todo determine if these checks should be done in ClientCommandQueue for all client_only cppHook commands
	if (actorObject->isDead()
		|| actorObject->isIncapacitated()
		|| ClientCommandChecks::doesLocomotionInvalidateCommand(&command, actorObject)
		|| ClientCommandChecks::doesStateInvalidateCommand(&command, actorObject))
	{
		return;
	}

	Controller * const controller = actorObject->getController();
	if (controller != 0)
	{
		MessageQueueString * const message = new MessageQueueString ("jump");
		controller->appendMessage(
			CM_animationAction,
			0.0f,
			message,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_ALL_CLIENT
			);
	}

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(cs_jumpServerCommandName, NetworkId::cms_invalid, Unicode::emptyString));
}

// ----------------------------------------------------------------------

static std::string nextStringParm(Unicode::String const &str, size_t &curpos)
{
    size_t endpos = 0;
    Unicode::String token;
    if (!Unicode::getFirstToken(str, curpos, endpos, token))
        return std::string ();
    curpos = endpos;
    return Unicode::wideToUTF8(token);
}

// ----------------------------------------------------------------------

static int nextIntParm(Unicode::String const &str, size_t &curpos)
{
    std::string strParm = nextStringParm(str, curpos);
    if (strParm.empty())
        return -1;
    return atoi(strParm.c_str());
}


// ----------------------------------------------------------------------
/**
 * Instruct the actor creature to follow the specified target.
 *
 * This function ensures both the follower and the leader are creatures.
 */

void CommandCppFuncsNamespace::follow (Command const & , NetworkId const & actorId, NetworkId const & targetId, Unicode::String const &)
{
	if (!targetId.isValid ())
		return;

	ClientObject * followerObject = dynamic_cast<ClientObject*>(NetworkIdManager::getObjectById(actorId));
	if(followerObject)
	{
		//-- Get the follower creature object.
		CreatureObject * const follower = followerObject->asCreatureObject();
		if(follower)
		{
			//-- Check to see if the creature is in a ship.
			if(follower->getPilotedShip())
			{
				//-- Get the follower ship object.
				ShipObject * const followerShip = follower->getPilotedShip();
				if(followerShip)
				{
					//-- Check if the target (leader) is a ship object.
					ShipObject * const target = dynamic_cast<ShipObject*>(NetworkIdManager::getObjectById(targetId));
					if(!target)
					{
						DEBUG_REPORT_LOG(true, ("commandFuncFollow: failed because target id [%s] is not a ShipObject.\n", targetId.getValueString ().c_str ()));
						return;
					}

					//-- Set the follower's auto follow target.
					PlayerShipController * const controller = dynamic_cast<PlayerShipController*>(followerShip->getController());
					if(!controller)
					{
						DEBUG_REPORT_LOG(true, ("commandFuncFollow: failed because actor id [%s] doesn't have a PlayerShipController.\n", targetId.getValueString().c_str()));
						return;
					}

					controller->engageAutopilotFollow(*target);
				}
			}
			else
			{
				//-- Check if the target (leader) is a creature.
				CreatureObject *const target = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(targetId));
				if (!target)
				{
					DEBUG_REPORT_LOG(true, ("commandFuncFollow: failed because target id [%s] is not a CreatureObject.\n", targetId.getValueString ().c_str ()));
					return;
				}

				//-- Set the follower's auto follow target.
				PlayerCreatureController * const controller = dynamic_cast<PlayerCreatureController*>(follower->getController());
				if (!controller)
				{
					DEBUG_REPORT_LOG(true, ("commandFuncFollow: failed because actor id [%s] doesn't have a PlayerCreatureController.\n", targetId.getValueString().c_str()));
					return;
				}

/*
				//-- Can only follow another player character
				if ((follower == Game::getPlayer()) && (target->getPlayerObject() == NULL))
				{
					Unicode::String result;
					CuiStringVariablesManager::process(ClientStringIds::follow_not_pc_prose, Unicode::emptyString, target->getLocalizedName(), Unicode::emptyString, result);
					CuiSystemMessageManager::sendFakeSystemMessage(result);
					return;
				}
*/

				controller->setAutoFollowTarget(target);
			}
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Instruct the actor to stop following whatever creature he is following.
 *
 * This function ensures both the follower and the leader are creatures.
 */

void CommandCppFuncsNamespace::stopFollow (Command const &            , NetworkId const & actorId, NetworkId const &          , Unicode::String const &)
{
	if (!actorId.isValid())
		return;

	ClientObject * followerObject = dynamic_cast<ClientObject*>(NetworkIdManager::getObjectById(actorId));
	if(followerObject)
	{
		//-- Get the follower creature object.
		CreatureObject * const followerCreature = followerObject->asCreatureObject();
		if(followerCreature)
		{
			//-- Check to see if the creature is in a ship.
			if(followerCreature->getPilotedShip())
			{
				//-- Get the follower ship object.
				ShipObject * const followerShip = followerCreature->getPilotedShip();
				if(followerShip)
				{
					//-- Set the follower's auto follow target.
					PlayerShipController * const controller = dynamic_cast<PlayerShipController*>(followerShip->getController());
					if(controller)
					{
						controller->cancelAutopilot();
					}
					else
					{
						DEBUG_REPORT_LOG(true, ("commandStopFollow: failed because actor id [%s] doesn't have a PlayerShipController.\n", actorId.getValueString().c_str()));
					}
				}
				else
				{
					DEBUG_REPORT_LOG(true, ("commandStopFollow: failed because actor id [%s] doesn't have a PilotedShip.\n", actorId.getValueString().c_str()));
				}
			}
			else
			{
				//-- Set the follower's auto follow target.
				PlayerCreatureController * const controller = dynamic_cast<PlayerCreatureController*>(followerObject->getController());
				if(controller)
				{
					controller->setAutoFollowTarget(NULL);
				}
				else
				{
					DEBUG_REPORT_LOG(true, ("commandStopFollow: failed because actor id [%s] doesn't have a PlayerCreatureController.\n", actorId.getValueString().c_str()));
				}
			}
		}
	}
}

// ======================================================================

void  CommandCppFuncsNamespace::commandFuncAddMapLocation(Command const &            , NetworkId const & actorId, NetworkId const &          , Unicode::String const &params)
{
	UNREF(actorId);
	size_t pos = 0;
	const std::string & planetName     = nextStringParm(params, pos);
	const NetworkId locationId  (nextStringParm (params, pos));
	const std::string & locationName   = nextStringParm(params, pos);
	const float x                      = static_cast<float>(nextIntParm(params, pos));
	const float y                      = static_cast<float>(nextIntParm(params, pos));
	const uint8 category               = PlanetMapManager::findCategory (nextStringParm(params, pos));
	const uint8 subCategory            = PlanetMapManager::findCategory (nextStringParm(params, pos));

	const AddMapLocationMessage message(planetName, NetworkId(locationId), Unicode::narrowToWide(locationName), Vector2d(x, y), category, subCategory);
	GameNetwork::send (message, true);
}

// ======================================================================

void  CommandCppFuncsNamespace::commandFuncGetMapLocations(Command const &            , NetworkId const & actorId, NetworkId const &          , Unicode::String const &params)
{
	UNREF(actorId);
	size_t pos = 0;
	std::string planetName       = nextStringParm(params, pos);

	GetMapLocationsMessage message(planetName, 0, 0, 0);
	GameNetwork::send (message, true);
}

//----------------------------------------------------------------------

void  CommandCppFuncsNamespace::commandFuncInstantMessageTell    (Command const &,             NetworkId const & actorId,         NetworkId const &, Unicode::String const & params)
{
	Object * const player = Game::getPlayer ();
	if (!player || player->getNetworkId () != actorId)
		return;

	if(Game::isTutorial() && !Game::getPlayerObject()->isAdmin())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("You cannot initiate a tell while in the tutorial."));
		return;
	}

	Unicode::String result;
	IGNORE_RETURN(CuiInstantMessageManager::tell  (params, result));
	if (!result.empty ())
		CuiChatRoomManager::sendPrelocalizedChat(result);
}

//----------------------------------------------------------------------

void  CommandCppFuncsNamespace::commandFuncInstantMessageTtell   (Command const &,             NetworkId const & actorId,         NetworkId const &, Unicode::String const & params)
{
	Object * const player = Game::getPlayer ();
	if (!player || player->getNetworkId () != actorId)
		return;

	if (Game::isTutorial() && !Game::getPlayerObject()->isAdmin())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("You cannot initiate a tell while in the tutorial."));
		return;
	}
	
	Unicode::String result;
	IGNORE_RETURN(CuiInstantMessageManager::targetedTell  (params, result));
	if (!result.empty ())
		CuiChatRoomManager::sendPrelocalizedChat (result);
}

//----------------------------------------------------------------------

void  CommandCppFuncsNamespace::commandFuncInstantMessageRetell  (Command const &,             NetworkId const & actorId,         NetworkId const &, Unicode::String const & params)
{
	Object * const player = Game::getPlayer ();
	if (!player || player->getNetworkId () != actorId)
		return;

	if (Game::isTutorial() && !Game::getPlayerObject()->isAdmin())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("You cannot initiate a tell while in the tutorial."));
		return;
	}

	Unicode::String result;
	IGNORE_RETURN(CuiInstantMessageManager::retell  (params, result));
	if (!result.empty ())
		CuiChatRoomManager::sendPrelocalizedChat (result);
}

//----------------------------------------------------------------------

void  CommandCppFuncsNamespace::commandFuncInstantMessageReply   (Command const &,             NetworkId const & actorId,         NetworkId const &, Unicode::String const & params)
{
	Object * const player = Game::getPlayer ();
	if (!player || player->getNetworkId () != actorId)
		return;

	Unicode::String result;
	IGNORE_RETURN(CuiInstantMessageManager::reply  (params, result));
	if (!result.empty ())
		CuiChatRoomManager::sendPrelocalizedChat (result);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncConsider               (Command const & , NetworkId const & actorId, NetworkId const & targetId, Unicode::String const &)
{
	const CreatureObject * const player = Game::getPlayerCreature ();
	if (!player || player->getNetworkId () != actorId)
		return;

	const CreatureObject * target = 0;
	if (!targetId.isValid ())
		target = dynamic_cast<CreatureObject *>(player->getLookAtTarget ().getObject ());
	else
		target = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById (targetId));

	if (!target || target->isDead ())
	{
		CuiChatRoomManager::sendPrelocalizedChat (CuiStringIdsCommand::no_valid_target.localize());
		return;
	}

	Unicode::String message;
	IGNORE_RETURN(CuiCombatManager::getConMessage (*target, message));
	CuiChatRoomManager::sendPrelocalizedChat (message);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncLanguage (Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
{
	Unicode::UnicodeStringVector argv;

	Unicode::String result;

	IGNORE_RETURN(Unicode::tokenize (params, argv));

	if (argv.empty ())
	{
		result += CuiStringIdsGameLanguage::spoken_languages.localize();
		result += '\n';

		// Display the list of spoken languages

		typedef std::vector<std::string> StringVector;
		StringVector languages;

		GameLanguageManager::getLanguages(languages);

		StringVector::const_iterator iterLanguages = languages.begin();
		int speakLanguageCount = 0;
		int comprehendLanguageCount = 0;

		PlayerObject *playerObject = Game::getPlayerObject();

		if (playerObject != NULL)
		{
			for (; iterLanguages != languages.end(); ++iterLanguages)
			{
				std::string const &language = (*iterLanguages);

				bool const speaksLanguage = playerObject->speaksLanguage(GameLanguageManager::getLanguageId(language));
				bool const comprehendsLanguage = playerObject->comprehendsLanguage(GameLanguageManager::getLanguageId(language));

				if (speaksLanguage ||
				    comprehendsLanguage)
				{
					result += Unicode::narrowToWide(language);

					if (speaksLanguage)
					{
						result += ' ';
						result += ClientTextManager::getColorCode(PackedRgb::solidYellow);
						result += Unicode::narrowToWide("[");
						result += ClientTextManager::getColorCode(PackedRgb::solidGreen);
						result += CuiStringIdsGameLanguage::speak.localize();
						result += ClientTextManager::getColorCode(PackedRgb::solidYellow);
						result += Unicode::narrowToWide("]");
						++speakLanguageCount;
					}

					if (comprehendsLanguage)
					{
						result += ' ';
						result += ClientTextManager::getColorCode(PackedRgb::solidYellow);
						result += Unicode::narrowToWide("[");
						result += ClientTextManager::getColorCode(PackedRgb::solidGreen);
						result += CuiStringIdsGameLanguage::comprehend.localize();
						result += ClientTextManager::getColorCode(PackedRgb::solidYellow);
						result += Unicode::narrowToWide("]");
						++comprehendLanguageCount;
					}

					result += Unicode::narrowToWide("\\#.\n");
				}
			}
		}

		// Language count

		if (speakLanguageCount == comprehendLanguageCount)
		{
			if (speakLanguageCount == 1)
			{
				result += CuiStringIdsGameLanguage::fluent_one.localize();
			}
			else if (speakLanguageCount > 1)
			{
				CuiStringVariablesData data;
				data.digit_i = speakLanguageCount;

				Unicode::String fluentManyString;
				CuiStringVariablesManager::process(CuiStringIdsGameLanguage::fluent_many, data, fluentManyString);
				result += fluentManyString;
			}
		}
		else
		{
			CuiStringVariablesData data;

			data.digit_i = speakLanguageCount;
			Unicode::String proseSpeakString;
			CuiStringVariablesManager::process(CuiStringIdsGameLanguage::prose_speak, data, proseSpeakString);

			data.digit_i = comprehendLanguageCount;
			Unicode::String proseComprehendString;
			CuiStringVariablesManager::process(CuiStringIdsGameLanguage::prose_comprehend, data, proseComprehendString);

			result += proseSpeakString;
			result += ' ';
			result += proseComprehendString;
		}
		result += '\n';

		// Current spoken language

		if (playerObject != NULL)
		{
			Unicode::String localizedName;
			GameLanguageManager::getLocalizedLanguageName(playerObject->getSpokenLanguage(), localizedName);

			CuiStringVariablesData data;
			data.targetName = localizedName;

			Unicode::String currentLanguageString;
			CuiStringVariablesManager::process(CuiStringIdsGameLanguage::language_current, data, currentLanguageString);

			result += currentLanguageString;
		}
	}
	else if (argv.	size() == 1)
	{
		uint32 languageId = 0;
		Unicode::String const &argName = argv[0];

		if (GameLanguageManager::isLanguageAbbreviationValid(argName, languageId))
		{
			PlayerObject *playerObject = Game::getPlayerObject();

			std::string languageName;
			GameLanguageManager::getLanguageName(static_cast<int>(languageId), languageName);

			if ((playerObject != NULL) &&
				playerObject->speaksLanguage(GameLanguageManager::getLanguageId(languageName)))
			{
				if (!languageName.empty())
				{
					if (GameLanguageManager::getLanguageId(languageName) == playerObject->getSpokenLanguage())
					{
						// No change in language

						Unicode::String localizedName;
						GameLanguageManager::getLocalizedLanguageName(playerObject->getSpokenLanguage(), localizedName);

						CuiStringVariablesData data;
						data.targetName = localizedName;

						Unicode::String currentLanguageString;
						CuiStringVariablesManager::process(CuiStringIdsGameLanguage::language_current, data, currentLanguageString);

						result += currentLanguageString;
					}
					else
					{
						// Set the new language

						playerObject->setSpokenLanguage(GameLanguageManager::getLanguageId(languageName));
					}
				}
				else
				{
					DEBUG_WARNING(true, ("The game language is empty, something is wrong."));
				}
			}
			else
			{
				Unicode::String localizedName;
				GameLanguageManager::getLocalizedLanguageName(languageName, localizedName);

				CuiStringVariablesData data;
				data.targetName = localizedName;

				Unicode::String notFluentString;
				CuiStringVariablesManager::process(CuiStringIdsGameLanguage::not_fluent_in_language, data, notFluentString);

				result += notFluentString;
			}
		}
		else
		{
			// Invalid language

			CuiStringVariablesData data;
			data.targetName = argv[0];

			Unicode::String notSpokenLanguageString;
			CuiStringVariablesManager::process(CuiStringIdsGameLanguage::not_spoken_language, data, notSpokenLanguageString);
			result += notSpokenLanguageString;
		}
	}
	else
	{
		result += CuiStringIdsGameLanguage::usage.localize();
	}

	if (!result.empty())
	{
		CuiChatRoomManager::sendPrelocalizedChat (result);
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncLogChat(Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	// Toggle chat logging

	ChatLogManager::setEnabled(!ChatLogManager::isEnabled());

	if (ChatLogManager::isEnabled())
	{
		ChatLogManager::showEnabledText();
	}
	else
	{
		ChatLogManager::showDisabledText();
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncFriendComment(Command const & , NetworkId const & , NetworkId const & , Unicode::String const &params)
{
	Unicode::String result;
	Unicode::UnicodeStringVector argv;
	IGNORE_RETURN(Unicode::tokenize(params, argv));

	if (argv.size() < 2)
	{
		result = CuiStringIdsCommunity::friend_comment_usage.localize();
	}
	else
	{
		Unicode::String const &name = argv[0];

		if (CommunityManager::isFriend(name))
		{
			int const start = static_cast<int>(name.size()) + 1;
			unsigned int const end = static_cast<unsigned int>(Unicode::String::npos);

			Unicode::String comment(params.substr(static_cast<size_t>(start), end));

			if (!comment.empty())
			{
				CommunityManager::addFriend(name, Unicode::emptyString, comment, true);

				CuiStringVariablesData data;
				data.sourceName = name;

				Unicode::String friendCommentSuccessString;
				CuiStringVariablesManager::process(CuiStringIdsCommunity::friend_comment_success, data, friendCommentSuccessString);

				result = friendCommentSuccessString;
			}
		}
		else
		{
			CuiStringVariablesData data;
			data.sourceName = name;

			Unicode::String friendInvalidString;
			CuiStringVariablesManager::process(CuiStringIdsCommunity::friend_invalid, data, friendInvalidString);

			result += friendInvalidString;
		}
	}

	if (!result.empty())
	{
		CuiChatRoomManager::sendPrelocalizedChat(result);
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncFriendGroup(Command const & , NetworkId const & , NetworkId const & , Unicode::String const &params)
{
	Unicode::String result;
	Unicode::UnicodeStringVector argv;
	IGNORE_RETURN(Unicode::tokenize(params, argv));

	if (argv.size() < 2)
	{
		result = CuiStringIdsCommunity::friend_group_usage.localize();
	}
	else
	{
		Unicode::String const &name = argv[0];

		if (CommunityManager::isFriend(name))
		{
			int const start = static_cast<int>(name.size()) + 1;
			unsigned int const end = static_cast<unsigned int>(Unicode::String::npos);

			Unicode::String group(params.substr(static_cast<size_t>(start), end));

			if (!group.empty())
			{
				CommunityManager::addFriend(name, group, Unicode::emptyString, true);

				CuiStringVariablesData data;
				data.sourceName = name;

				Unicode::String friendGroupSuccessString;
				CuiStringVariablesManager::process(CuiStringIdsCommunity::friend_group_success, data, friendGroupSuccessString);

				result = friendGroupSuccessString;
			}
		}
		else
		{
			CuiStringVariablesData data;
			data.sourceName = name;

			Unicode::String friendInvalidString;
			CuiStringVariablesManager::process(CuiStringIdsCommunity::friend_invalid, data, friendInvalidString);

			result += friendInvalidString;
		}
	}

	if (!result.empty())
	{
		CuiChatRoomManager::sendPrelocalizedChat(result);
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncFriend(Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	CommunityManager::showFriendList();
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncFriendList(Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	CommunityManager::showFriendList();
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncIgnore(Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	CommunityManager::showIgnoreList();
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncIgnoreList(Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	CommunityManager::showIgnoreList();
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncAssist(Command const & , NetworkId const & actorId, NetworkId const & targetId, Unicode::String const & params)
{
	ClientObject * assisterObject = dynamic_cast<ClientObject*>(NetworkIdManager::getObjectById(actorId));
	if(!assisterObject)
		return;
	CreatureObject * const assister = assisterObject->asCreatureObject();
	if (!assister)
		return;

	NetworkId assisteeId = NetworkId::cms_invalid;

	// if /assist with no parameter, use intended target, then look at target
	if (!targetId.isValid() && params.empty())
	{
		if (assister->getIntendedTarget().isValid())
			assisteeId = assister->getIntendedTarget();
		else if (assister->getLookAtTarget().isValid())
			assisteeId = assister->getLookAtTarget();
	}
	// if /assist with parameter that doesn't specify a valid target, do nothing
	else if (!targetId.isValid() && !params.empty())
	{
		return;
	}
	// if /assist with a parameter that specifies a valid target, use specified target
	else
	{
		assisteeId = targetId;
	}

	ClientObject * assisteeObject = 0;
	if (assisteeId.isValid())
		assisteeObject = dynamic_cast<ClientObject*>(NetworkIdManager::getObjectById(assisteeId));

	if (assisteeObject)
	{
		CreatureObject * const assistee = assisteeObject->asCreatureObject();
		if (assistee && assistee->getIntendedTarget().isValid() && assistee->isPlayer())
		{
			assister->setIntendedTarget(assistee->getIntendedTarget());
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncNotepad(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
{
	CuiActionManager::performAction (CuiActions::notepad, params);
}

////////////////////////////
//BEGIN CSR COMMAND WRAPPERS
//These commands should gate functionality with ConfigClientGame::getCSR().  This is not full-proof protection, but provides an additional level of obfuscation

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncSetScale (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & params)
{
	if(Game::getPlayerObject()->isAdmin())
	{
		if(target == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true, ("no target passed to setScale"));
			return;
		}
		const std::string scaleCommand = "/remote object scalesize ";
		std::string targetId = target.getValueString();
		std::string command = scaleCommand + targetId + std::string(" ") + Unicode::wideToNarrow(params);

		IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(command, true));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncSetYaw (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & params)
{
	if(Game::getPlayerObject()->isAdmin())
	{
		if(target == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true, ("no target passed to setYaw"));
			return;
		}
		const std::string setyawCommand = "/remote object rotate ";
		std::string targetId = target.getValueString();
		std::string command = setyawCommand + targetId + std::string(" ") + Unicode::wideToNarrow(params) + " 0 0";

		IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(command, true));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncHideMe (Command const &, NetworkId const &, NetworkId const &, Unicode::String const &params)
{
	if (Game::getPlayerObject()->isAdmin())
	{
		bool hide = false;

		std::string const &narrowParams = Unicode::wideToNarrow(params);
		if (narrowParams == "true" || narrowParams == "1")
		{
			hide = true;
		}
		else if (narrowParams == "false" || narrowParams== "0")
		{
			hide = false;
		}
		else
		{
			CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Syntax: /hideMe <0 | 1> where 0 is visible and 1 is hidden."));
			return;
		}

		UNREF(hide);

		Object const *actor = Game::getConstPlayerPilotedShip();
		if (!actor)
			actor = Game::getConstPlayer();

		if (actor)
		{
			std::string const &command = std::string("/remote object hide ") + actor->getNetworkId().getValueString() + std::string(" ") + narrowParams;
			IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(command, true));
			std::string msg = std::string("hideMe: Your client and character are now ") + std::string(hide ? "hidden" : "visible") + std::string(" to other players.");
			CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide(msg));

		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncReloadAdminTable (Command const & , NetworkId const & , NetworkId const & , Unicode::String const & )
{
	if(Game::getPlayerObject()->isAdmin())
	{
		const std::string reloadCommand = "/remote server reloadAdminTable";
		IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(reloadCommand, true));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncSkills (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & )
{
	if(Game::getPlayerObject()->isAdmin())
	{
		if(target == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true, ("no target passed to commandFuncSkills"));
			return;
		}
		const std::string skillsCommand = "/remote skill getSkillList ";
		std::string targetId = target.getValueString();
		std::string command = skillsCommand + targetId;

		IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(command, true));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncSpawn (Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
{
	if(Game::getPlayerObject()->isAdmin())
	{
		const std::string spawnCommand = "/remote object spawn ";

		const std::string narrowParams = Unicode::wideToNarrow(params);

		std::string command = spawnCommand + narrowParams;

		IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(command, true));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncDestroy (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & )
{
	if(Game::getPlayerObject()->isAdmin())
	{
		if(target == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true, ("no target passed to commandFuncDestroy"));
			return;
		}
		const std::string destroyCommand = "/remote object destroy ";
		std::string targetId = target.getValueString();
		std::string command = destroyCommand + targetId;

		IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(command, true));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncEditBank (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & )
{
	if (Game::getPlayerObject()->isAdmin())
	{
		if (target == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true, ("no target passed to commandFuncEditBank"));
			return;
		}

		CreatureObject const * const co = getCreatureOrPilot(target);
		if (co)
		{
			ClientObject const * const bank = co->getBankObject();
			if (bank)
			{
				IGNORE_RETURN(ClientCommandQueue::enqueueCommand("openContainer", bank->getNetworkId(), Unicode::emptyString));
			}
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncEditInventory (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & )
{
	if (Game::getPlayerObject()->isAdmin())
	{
		if (target == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true, ("no target passed to commandFuncEditInventory"));
			return;
		}

		CreatureObject const * const co = getCreatureOrPilot(target);
		if (co)
		{
			ClientObject const * const inv = co->getInventoryObject();
			if (inv)
			{
				IGNORE_RETURN(ClientCommandQueue::enqueueCommand("openContainer", inv->getNetworkId(), Unicode::emptyString));
			}
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncEditDatapad (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & )
{
	if (Game::getPlayerObject()->isAdmin())
	{
		if (target == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true, ("no target passed to commandFuncEditDatapad"));
			return;
		}

		CreatureObject const * const co = getCreatureOrPilot(target);
		if (co)
		{
			ClientObject const * const inv = co->getDatapadObject();
			if (inv)
			{
				IGNORE_RETURN(ClientCommandQueue::enqueueCommand("openContainer", inv->getNetworkId(), Unicode::emptyString));
			}
		}
	}
}

//----------------------------------------------------------------------
void CommandCppFuncsNamespace::commandFuncExamineObjVars (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & params)
{
	if(Game::getPlayerObject()->isAdmin())
	{
		const std::string objvarCommand = "/remote objvar list ";

		std::string targetId = target.getValueString();
		std::string command = objvarCommand + targetId + std::string(" ") + Unicode::wideToUTF8(params);

		IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(command, true));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncRenamePlayer        (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & params)
{
	if(Game::getPlayerObject()->isAdmin())
	{
		if(target == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true, ("no target passed renamePlayer"));
			return;
		}

		const CreatureObject * const co = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById (target));
		if(co)
		{
			const std::string renamePlayerCommand = "/remote object renamePlayer ";
			std::string targetId = target.getValueString();
			std::string command = renamePlayerCommand + targetId + std::string(" ") + Unicode::wideToUTF8(params);

			IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(command, true));
		}
		else
		{
			DEBUG_WARNING(true, ("Target is not a creature in /renamePlayer"));
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncDraw (Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
{
	if(Game::getPlayerObject()->isAdmin())
	{
		size_t pos = 0;
		int value = nextIntParm (params, pos);
		CuiPreferences::setDrawNetworkIds(value);
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncUiDebugExamine (Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
{
	if(Game::getPlayerObject()->isAdmin())
	{
		const std::string debugExamineCommand = "/ui debugexamine ";
		std::string command = debugExamineCommand + Unicode::wideToUTF8(params);
		IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(command, true));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncUnload (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & )
{
	if(Game::getPlayerObject()->isAdmin())
	{
		if(target == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true, ("no target passed to unload"));
			return;
		}
		const std::string unloadCommand = "/remote object unload ";
		std::string targetId = target.getValueString();
		std::string command = unloadCommand + targetId;

		IGNORE_RETURN(CuiMessageQueueManager::executeCommandByString(command, true));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncEmptyMail(Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	CuiPersistentMessageManager::emptyMail(false);
}


//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncEditBiography(Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & str)
{
	if(Game::getPlayerObject()->isAdmin())
	{
		NetworkId editTarget(target);

		if(editTarget == NetworkId::cms_invalid)
		{
			editTarget = NetworkId(Unicode::wideToNarrow(str));

			if(editTarget == NetworkId::cms_invalid)
			{
				Game::debugPrintUi("A valid target was not passed to editBiography.");
				return;
			}
		}

		CreatureObject const * const creatureObject = getCreatureOrPilot(editTarget);
		PlayerObject const * const playerObject = creatureObject ? creatureObject->getPlayerObject() : NULL;
		if (playerObject)
		{
			Game::debugPrintUi("Requesting player biography...");
			CuiActionManager::performAction(CuiActions::editBiography, Unicode::narrowToWide(editTarget.getValueString()));
		}
		else
		{
			Game::debugPrintUi("A the target passed to editBiography is invalid.");
		}
	}
}

//END CSR COMMAND WRAPPERS
//////////////////////////

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncSysgroup (Command const & , NetworkId const & actorId, NetworkId const & targetId, Unicode::String const & params)
{
	const uint32 serverCommandHash = Crc::normalizeAndCalculate ("serversysgroup");

	//run text through the socials parser to convert "hi %TT" to "hi luke", etc.
	Unicode::String resultText;
	CuiStringVariablesData csvd;

	const CreatureObject * const player = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById (actorId));

	if (!player)
		return;

	NetworkId processedTarget = targetId;
	Unicode::String processedParams = params;

	if(CuiStringVariablesManager::stringHasCodeForObject(params, CuiStringVariablesManager::VO_other))
	{
		ClientCommandQueue::determineTarget(params, processedTarget, processedParams, false);
	}

	csvd.source = player;
	ClientObject* const t = dynamic_cast<ClientObject*>(NetworkIdManager::getObjectById(processedTarget));
	csvd.target = NULL;

	if(CuiPreferences::getAutoAimToggle())
		csvd.target = dynamic_cast<ClientObject *>(player->getIntendedTarget().getObject());
	else
		csvd.target = dynamic_cast<ClientObject *>(player->getLookAtTarget().getObject());

	csvd.other  = t;
	CuiStringVariablesManager::process(processedParams, csvd, resultText);

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(serverCommandHash, NetworkId::cms_invalid, resultText));
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncEcho                   (Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
{
	if (params.empty ())
		CuiChatRoomManager::sendPrelocalizedChat (CuiStringIdsCommand::echo.localize());
	else
		CuiChatRoomManager::sendPrelocalizedChat (params);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncMacro                  (Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	UNREF (actor);
	UNREF (target);
	UNREF (command);

	if (params.empty ())
		return;

	const std::string & narrowParams = Unicode::wideToUTF8 (params);
	if (!ClientMacroManager::executeMacroByUserDefinedName (narrowParams, true))
	{
		CuiStringVariablesData data;
		data.targetName = params;

		Unicode::String failedMacro;
		CuiStringVariablesManager::process(CuiStringIdsCommand::unable_to_execute, data, failedMacro);

		CuiChatRoomManager::sendPrelocalizedChat(failedMacro);
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncDumpPausedCommands                  (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	const int num = CuiConsoleHelper::dumpPausedCommands ();

	if (num > 0)
	{
		CuiStringVariablesData data;
		data.digit_i = num;

		Unicode::String dumpedResult;
		CuiStringVariablesManager::process(CuiStringIdsCommand::paused_commands_dumped, data, dumpedResult);

		CuiChatRoomManager::sendPrelocalizedChat(dumpedResult);
	}
	else
		CuiChatRoomManager::sendPrelocalizedChat (CuiStringIdsCommand::no_paused_commands.localize());
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncMailSave                  (Command const & , NetworkId const & , NetworkId const & , Unicode::String const &)
{
	CuiPersistentMessageManager::startWritingEmailsToDisk ();
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncTarget (Command const & , NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
	{
		Object         const * const o        = NetworkIdManager::getObjectById(target);
		ClientObject   const * const co       = o ? o->asClientObject()  : NULL;
		{
			if(co && co->isTargettable())
			{
				player->setIntendedTarget(target);
				player->setLookAtTarget(target);
			}
			else
			{
				player->setLookAtTarget (NetworkId::cms_invalid);
			}
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncTargetPilot(Command const &, NetworkId const &, NetworkId const &target, Unicode::String const &)
{
	if (Game::getPlayerObject()->isAdmin())
	{
		CreatureObject * const player = Game::getPlayerCreature();
		if (player)
		{
			Object const * const o = NetworkIdManager::getObjectById(target);
			if (o)
			{
				ClientObject const * const co = o->asClientObject();
				if (co)
				{
					ShipObject const * const ship = co->asShipObject();
					if (ship)
					{
						CreatureObject const * const pilot = ship->getPilot();
						if (pilot)
							player->setLookAtTarget(pilot->getNetworkId());
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncShipManageComponents(Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & )
{
	NetworkId localTarget = target;
	if(!localTarget.isValid())
	{
		ShipObject const * const ship = Game::getPlayerPilotedShip();
		if(ship)
			localTarget = ship->getNetworkId();
	}
	Object const * const o = NetworkIdManager::getObjectById(localTarget);
	ClientObject const * const co = o ? o->asClientObject() : NULL;
	ShipObject const * const ship = co ? co->asShipObject() : NULL;
	//see if the target is actually a ship
	if(ship)
	{
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::manageShipComponents, Unicode::narrowToWide (localTarget.getValueString ())));
	}
	else
	{
		//if it's not a ship, it could be the ship control device
		const int got =  co ? co->getGameObjectType () : SharedObjectTemplate::GOT_none;
		if(co && got == SharedObjectTemplate::GOT_data_ship_control_device)
		{
			Container const * const c = ContainerInterface::getContainer(*co);
			if(c)
			{
				for(ContainerConstIterator i = c->begin(); i != c->end(); ++i)
				{
					Container::ContainedItem const item = *i;
					Object const * const containedO = item.getObject();
					ClientObject const * const containedCO = containedO ? containedO->asClientObject() : NULL;
					ShipObject const * const containedShip = containedCO ? containedCO->asShipObject() : NULL;
					if(containedShip)
					{
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::manageShipComponents, Unicode::narrowToWide (containedShip->getNetworkId().getValueString ())));
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncComponentDetail(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & )
{
	ShipObject const * const ship = Game::getPlayerPilotedShip();
	if(ship)
	{
		CuiActionManager::performAction (CuiActions::shipComponentDetail, Unicode::narrowToWide (ship->getNetworkId().getValueString().c_str()));
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncBoosterToggle(Command const & , NetworkId const & , NetworkId const &, Unicode::String const & )
{
	ShipObject * const shipObject = Game::getPlayerPilotedShip();
	if (shipObject == NULL)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(SharedStringIds::booster_pilot_only.localize());
		return;
	}

	if (!shipObject->isSlotInstalled(ShipChassisSlotType::SCST_booster))
	{
		CuiSystemMessageManager::sendFakeSystemMessage(SharedStringIds::no_booster.localize());
		return;
	}

	bool const boosterActive = shipObject->isBoosterActive();

	if (Game::getSinglePlayer())
	{
		IGNORE_RETURN(shipObject->clientSetComponentActive(ShipChassisSlotType::SCST_booster, !boosterActive));
		return;
	}

	static uint32 const hash_boosterOn = Crc::normalizeAndCalculate("boosterOn");
	static uint32 const hash_boosterOff = Crc::normalizeAndCalculate("boosterOff");

	if (boosterActive)
		IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash_boosterOff, NetworkId::cms_invalid, Unicode::emptyString));
	else
		IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash_boosterOn, NetworkId::cms_invalid, Unicode::emptyString));
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncThrottle(Command const & , NetworkId const & , NetworkId const &, Unicode::String const & params)
{
	CreatureObject * const player = Game::getPlayerCreature();
	if (player == NULL)
		return;

	ShipObject * const playerShip = player->getPilotedShip();
	if (playerShip == NULL)
	{
		CuiSystemMessageManager::sendFakeSystemMessage (Unicode::narrowToWide("(debug) You must be piloting a ship to use the throttle."));
		return;
	}

	PlayerShipController * const playerShipController = dynamic_cast<PlayerShipController *>(playerShip->getController());
	if (playerShipController == NULL)
		return;

	if (!params.empty())
	{
		float const newThrottlePosition = static_cast<float>(atof(Unicode::wideToNarrow(params).c_str()));
		playerShipController->setThrottlePosition(newThrottlePosition, true);
	}

	float const throttlePosition = playerShipController->getThrottlePosition();
	char buf[128];
	size_t const buf_size = sizeof(buf);
	snprintf(buf, buf_size, "(debug) Throttle: [%5.2f]", throttlePosition);
	Game::debugPrintUi(buf);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncMatchSpeed(Command const & , NetworkId const & , NetworkId const & targetId, Unicode::String const &)
{
	CreatureObject * const player = Game::getPlayerCreature();
	if (player == NULL)
		return;

	ShipObject * const playerShip = player->getPilotedShip();
	if (playerShip == NULL)
	{
		CuiSystemMessageManager::sendFakeSystemMessage (Unicode::narrowToWide("(debug) You must be piloting a ship to use the throttle."));
		return;
	}

	PlayerShipController * const playerShipController = dynamic_cast<PlayerShipController *>(playerShip->getController());
	if (playerShipController == NULL)
		return;

	ShipObject * const targetShip = dynamic_cast<ShipObject *>(NetworkIdManager::getObjectById(targetId));

	if (targetShip == NULL)
	{
		CuiChatRoomManager::sendPrelocalizedChat (CuiStringIdsCommand::no_ship_targetted.localize());
		return;
	}

	playerShipController->matchSpeed(*targetShip);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncWaypointAutopilot(Command const & /*command*/, NetworkId const & /*actor*/, NetworkId const & target, Unicode::String const & /*params*/)
{
	ShipObject * const playerShip = Game::getPlayerPilotedShip();
	if (playerShip == NULL)
	{
		// this should not be reachable because the "autopilot" radial menu option disappears when not piloting a ship
		CuiSystemMessageManager::sendFakeSystemMessage (Unicode::narrowToWide("(debug) You must be piloting a ship to use the autopilot."));
		return;
	}

	PlayerShipController * const playerShipController = dynamic_cast<PlayerShipController *>(playerShip->getController());
	if (playerShipController == NULL)
		return;

	ClientWaypointObject * const waypoint = dynamic_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById (target));
	if (waypoint)
	{
		if (waypoint->getPlanetName() != Game::getNonInstanceSceneId())
		{
			// this should not be reachable because the "autopilot" radial menu option disappears when in the wrong scene.
			CuiSystemMessageManager::sendFakeSystemMessage (Unicode::narrowToWide("(debug) You cannot autopilot to a different space zone."));
			return;
		}
		playerShipController->engageAutopilotToLocation(waypoint->getLocation());
		waypoint->setWaypointActive(true);
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncHyperspaceMap(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & )
{
	CuiActionManager::performAction (CuiActions::hyperspaceMap, Unicode::emptyString);
}

//----------------------------------------------------------------------

#ifdef ENABLE_FORMATIONS
void CommandCppFuncsNamespace::commandFuncSpaceAssignPlayerFormation(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & )
{
	CuiActionManager::performAction (CuiActions::spaceAssignPlayerFormation, Unicode::emptyString);
}
#endif

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncZoneMap(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
{
	CuiActionManager::performAction (CuiActions::spaceZoneMap, params);
}

//----------------------------------------------------------------------

#ifdef ENABLE_FORMATIONS
void CommandCppFuncsNamespace::commandFuncEnterFormation(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & )
{
	CreatureObject * const player = Game::getPlayerCreature();
	if (!player)
		return;
	ShipObject * const ship = player->getPilotedShip();
	if(!ship)
		return;

	CachedNetworkId const id (player->getGroup ());
	GroupObject const * const group = safe_cast<const GroupObject *>(id.getObject());
	if(group)
	{
		if(group->getFormationNameCrc() != Crc::crcNull)
		{
			Controller * const c = ship->getController();
			PlayerShipController * const psc = dynamic_cast<PlayerShipController *>(c);
			if(psc)
			{
				psc->setInFormation(true);
			}
		}
	}
}
#endif

//----------------------------------------------------------------------

#ifdef ENABLE_FORMATIONS
void CommandCppFuncsNamespace::commandFuncLeaveFormation(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & )
{
	CreatureObject * const player = Game::getPlayerCreature();
	ShipObject * const ship = player ? player->getPilotedShip() : NULL;
	Controller * const c = ship ? ship->getController() : NULL;
	PlayerShipController * const psc = dynamic_cast<PlayerShipController *>(c);
	if(psc)
	{
		psc->setInFormation(false);
	}
}
#endif

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncOpenCloseWings(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & )
{
	CreatureObject * const player = Game::getPlayerCreature();
	ShipObject * const ship = player ? player->getPilotedShip() : NULL;

	if (ship && ship->hasWings())
	{
		if (ship->hasCondition(TangibleObject::C_wingsOpened))
		{
			if (Game::getSinglePlayer())
				ship->clearCondition(TangibleObject::C_wingsOpened);
			else
			{
				static uint32 const hash_closeWings = Crc::normalizeAndCalculate("closeWings");
				IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash_closeWings, NetworkId::cms_invalid, Unicode::emptyString));
			}
		}
		else
		{
			if (Game::getSinglePlayer())
				ship->setCondition(TangibleObject::C_wingsOpened);
			else
			{
				static uint32 const hash_openWings = Crc::normalizeAndCalculate("openWings");
				IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash_openWings, NetworkId::cms_invalid, Unicode::emptyString));
			}
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncWeaponGroupAssignment(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & )
{
	IGNORE_RETURN(CuiActionManager::performAction (CuiActions::weaponGroupAssignment, Unicode::emptyString));
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncQuestJournal(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & )
{
	IGNORE_RETURN(CuiActionManager::performAction (CuiActions::questJournal, Unicode::emptyString));
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncChatterSpeech(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
{
	StringId const s("chatter_speech", Unicode::wideToNarrow(params));
	Unicode::String const res = s.localize();

	uint32 const chatType = SpatialChatManager::getChatTypeByName ("say");
	CreatureObject * const playerCreature = Game::getPlayerCreature();
	uint32 const mood = playerCreature ? playerCreature->getMood () : 0;
	bool const isPrivate = SpatialChatManager::isPrivate (chatType);
	PlayerObject * const playerObject = Game::getPlayerObject();
	uint32 const language = playerObject ? playerObject->getSpokenLanguage() : 0;

	CuiSpatialChatManager::sendMessage (NetworkId::cms_invalid, chatType, mood, res, isPrivate, false, language);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncLogout(Command const & command, NetworkId const & actorId, NetworkId const & targetId, Unicode::String const & params)
{
	CreatureObject const * const player = Game::getPlayerCreature();

	if (player == 0)
	{
		return;
	}

	if (player->getState(States::Combat))
	{
		CuiSystemMessageManager::sendFakeSystemMessage(ClientStringIds::you_cannot_log_out_while_in_combat.localize());
		return;
	}

	if (!player->isSittingOnObject())
	{
		CommandCppFuncsNamespace::commandFuncSitClient(command, actorId, targetId, params);
	}

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(cs_logoutServerCommandName, NetworkId::cms_invalid, Unicode::emptyString));
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncWaypoint(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
{
	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(cs_waypointServerCommandName, NetworkId::cms_invalid, params));
}


void CommandCppFuncsNamespace::commandFuncExternalCommand(Command const & command, NetworkId const & , NetworkId const & , Unicode::String const & )
{
	Game::externalCommand(command.m_commandName.c_str());
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncCollections(Command const & , NetworkId const & , NetworkId const & target, Unicode::String const & params)
{
	if (target.isValid())
		CuiActionManager::performAction(CuiActions::collections, Unicode::narrowToWide(target.getValueString()));
	else
		CuiActionManager::performAction(CuiActions::collections, params);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncVoiceInvite(Command const &, NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	if(target.isValid() && CuiVoiceChatManager::isLoggedIn())
	{
		Object const * const o = NetworkIdManager::getObjectById(target);
		if(o)
		{
			ClientObject const * const co = o->asClientObject();
			if(co)
			{
				CreatureObject const * const creature = co->asCreatureObject();
				if(creature)
				{
					PlayerObject const * const player = creature->getPlayerObject();
					if(player)
					{
						CuiVoiceChatManager::channelInvite(target, Unicode::wideToNarrow(creature->getLocalizedFirstName()), CuiVoiceChatManager::getMyPrivateChannelName());
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncVoiceKick(Command const &, NetworkId const & , NetworkId const & target, Unicode::String const &)
{
	if(target.isValid() && CuiVoiceChatManager::isLoggedIn())
	{
		Object const * const o = NetworkIdManager::getObjectById(target);
		if(o)
		{
			ClientObject const * const co = o->asClientObject();
			if(co)
			{
				CreatureObject const * const creature = co->asCreatureObject();
				if(creature)
				{
					PlayerObject const * const player = creature->getPlayerObject();
					if(player)
					{
						CuiVoiceChatManager::channelKick(target, Unicode::wideToNarrow(creature->getLocalizedFirstName()), CuiVoiceChatManager::getMyPrivateChannelName());
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

//void CommandCppFuncsNamespace::commandFuncTcg(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
//{
//	CuiActionManager::performAction(CuiActions::tcg, params);
//}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncGCWInfo(Command const & , NetworkId const & , NetworkId const & , Unicode::String const & params)
{
	CuiActionManager::performAction(CuiActions::gcwInfo, params);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncAppearanceTab(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	UNREF(actor);
	UNREF(target);
	UNREF(command);

	CuiActionManager::performAction(CuiActions::appearanceTab, params);
}

void CommandCppFuncsNamespace::commandFuncQuestBuilder(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	UNREF(actor);
	UNREF(target);
	UNREF(command);

	CuiActionManager::performAction(CuiActions::questBuilder, params);
}

void CommandCppFuncsNamespace::commandFuncOpenStorytellerRecipe(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	UNREF(command);
	UNREF(actor);

	CuiRecipeManager::setRecipeObject(target);
	CuiActionManager::performAction(CuiActions::questBuilder, params);
}

//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncToggleOutOfCharacter(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const &)
{
	UNREF(actor);
	UNREF(target);
	UNREF(command);

	PlayerObject const * const player = Game::getPlayerObject();
	if(player)
		player->toggleOutOfCharacter();
}


//----------------------------------------------------------------------

void CommandCppFuncsNamespace::commandFuncToggleLookingForWork(Command const & command, NetworkId const & actor, NetworkId const & target, Unicode::String const &)
{
	UNREF(actor);
	UNREF(target);
	UNREF(command);

	PlayerObject const * const player = Game::getPlayerObject();
	if(player)
		player->toggleLookingForWork();
}
// ======================================================================

void CommandCppFuncs::install()
{
	InstallTimer const installTimer("CommandCppFuncs::install");

	CommandTable::addCppFunction("activate",               commandFuncActivate);
	CommandTable::addCppFunction("addMapLocation",         commandFuncAddMapLocation);
	CommandTable::addCppFunction("auctionUse",             commandFuncAuctionUse);
	CommandTable::addCppFunction("clientcommandtest",      commandFuncTest);
	CommandTable::addCppFunction("clientcommandtest",      commandFuncTest);
	CommandTable::addCppFunction("attack",                 commandFuncAttack);
	CommandTable::addCppFunction("conversationStart",      commandFuncConversationStart);
	CommandTable::addCppFunction("conversationStop",       commandFuncConversationStop);
	CommandTable::addCppFunction("deactivate",             commandFuncDeactivate);
	CommandTable::addCppFunction("decline",                commandFuncDecline);
	CommandTable::addCppFunction("destroyObject",          commandFuncDestroyObject);
	CommandTable::addCppFunction("drop",                   commandFuncDrop);
	CommandTable::addCppFunction("equip",                  commandFuncEquip);
	CommandTable::addCppFunction("examine",                commandFuncExamine);
	CommandTable::addCppFunction("examineCharacterSheet",  commandFuncExamineCharacterSheet);
	CommandTable::addCppFunction("follow",                 follow);
	CommandTable::addCppFunction("getMapLocations",        commandFuncGetMapLocations);
	CommandTable::addCppFunction("getSample",              commandFuncGetSample);
	CommandTable::addCppFunction("join",                   commandFuncJoin);
	CommandTable::addCppFunction("kneel",                  commandFuncPosture);
	CommandTable::addCppFunction("kneel",                  commandFuncPosture);
	CommandTable::addCppFunction("open",                   commandFuncOpen);
	CommandTable::addCppFunction("openNewWindow",          commandFuncOpenNewWindow);
	CommandTable::addCppFunction("pickup",                 commandFuncPickup);
	CommandTable::addCppFunction("prone",                  commandFuncPosture);
	CommandTable::addCppFunction("prone",                  commandFuncPosture);
	CommandTable::addCppFunction("sitClient",              commandFuncSitClient);
	CommandTable::addCppFunction("jumpClient",             commandFuncJumpClient);
	CommandTable::addCppFunction("split",                  commandFuncSplit);
	CommandTable::addCppFunction("stand",                  commandFuncPosture);
	CommandTable::addCppFunction("stand",                  commandFuncPosture);
	CommandTable::addCppFunction("stopFollow",             stopFollow);
	CommandTable::addCppFunction("survey",                 commandFuncSurvey);
	CommandTable::addCppFunction("trade",                  commandFuncTrade);
	CommandTable::addCppFunction("tradeAccept",            commandFuncTradeAccept);
	CommandTable::addCppFunction("unequip",                commandFuncUnequip);
	CommandTable::addCppFunction("findcontrollers",		   commandFuncFindControllers);
	CommandTable::addCppFunction("showBackpack",		   commandFuncShowBackpack);
	CommandTable::addCppFunction("showHelmet",			   commandFuncShowHelmet);

	CommandTable::addCppFunction("instantMessageReply",    commandFuncInstantMessageReply);
	CommandTable::addCppFunction("instantMessageRetell",   commandFuncInstantMessageRetell);
	CommandTable::addCppFunction("instantMessageTtell",    commandFuncInstantMessageTtell);
	CommandTable::addCppFunction("instantMessageTell",     commandFuncInstantMessageTell);

	CommandTable::addCppFunction("consider",               commandFuncConsider);
	CommandTable::addCppFunction("language",               commandFuncLanguage);
	CommandTable::addCppFunction("logChat",                commandFuncLogChat);
	CommandTable::addCppFunction("sysgroup",               commandFuncSysgroup);
	CommandTable::addCppFunction("friendComment",          commandFuncFriendComment);
	CommandTable::addCppFunction("friendGroup",            commandFuncFriendGroup);
	CommandTable::addCppFunction("friend",                 commandFuncFriend);
	CommandTable::addCppFunction("friendList",             commandFuncFriendList);
	CommandTable::addCppFunction("ignore",                 commandFuncIgnore);
	CommandTable::addCppFunction("ignoreList",             commandFuncIgnoreList);

	CommandTable::addCppFunction("assist",                 commandFuncAssist);

	//csr commands
	CommandTable::addCppFunction("setScale",               commandFuncSetScale);
	CommandTable::addCppFunction("setYaw",                 commandFuncSetYaw);
	CommandTable::addCppFunction("hideMe",                 commandFuncHideMe);
	CommandTable::addCppFunction("reloadAdminTable",       commandFuncReloadAdminTable);
	CommandTable::addCppFunction("skills",                 commandFuncSkills);
	CommandTable::addCppFunction("spawn",                  commandFuncSpawn);
	CommandTable::addCppFunction("destroy",                commandFuncDestroy);
	CommandTable::addCppFunction("editBank",               commandFuncEditBank);
	CommandTable::addCppFunction("editInventory",          commandFuncEditInventory);
	CommandTable::addCppFunction("editDatapad",            commandFuncEditDatapad);
	CommandTable::addCppFunction("examineObjVars",         commandFuncExamineObjVars);
	CommandTable::addCppFunction("renamePlayer",           commandFuncRenamePlayer);
	CommandTable::addCppFunction("shownetworkids",         commandFuncDraw);
	CommandTable::addCppFunction("debugexamine",           commandFuncUiDebugExamine);
	CommandTable::addCppFunction("unload",                 commandFuncUnload);
	CommandTable::addCppFunction("emptyMail",              commandFuncEmptyMail);
	CommandTable::addCppFunction("targetPilot",            commandFuncTargetPilot);
	CommandTable::addCppFunction("editBiography",          commandFuncEditBiography);
	//end csr commands

	CommandTable::addCppFunction("echo",                   commandFuncEcho);
	CommandTable::addCppFunction("macro",                  commandFuncMacro);
	CommandTable::addCppFunction("dumpPausedCommands",     commandFuncDumpPausedCommands);
	CommandTable::addCppFunction("mailSave",               commandFuncMailSave);
	CommandTable::addCppFunction("itemMail",               commandFuncItemMail);
	CommandTable::addCppFunction("friendGroup",            commandFuncFriendGroup);
	CommandTable::addCppFunction("notepad",                commandFuncNotepad);
	CommandTable::addCppFunction("target",                 commandFuncTarget);

	//space
	CommandTable::addCppFunction("manageShipComponents",   commandFuncShipManageComponents);
	CommandTable::addCppFunction("boosterToggle", commandFuncBoosterToggle);
	CommandTable::addCppFunction("throttle", commandFuncThrottle);
	CommandTable::addCppFunction("matchSpeed", commandFuncMatchSpeed);
	CommandTable::addCppFunction("waypointAutopilot", commandFuncWaypointAutopilot);
	CommandTable::addCppFunction("hyperspaceMap", commandFuncHyperspaceMap);
#ifdef ENABLE_FORMATIONS
	CommandTable::addCppFunction("assignPlayerFormation", commandFuncSpaceAssignPlayerFormation);
	CommandTable::addCppFunction("enterFormation", commandFuncEnterFormation);
	CommandTable::addCppFunction("leaveFormation", commandFuncLeaveFormation);
#endif
	CommandTable::addCppFunction("openCloseWings", commandFuncOpenCloseWings);
	CommandTable::addCppFunction("componentDetail",   commandFuncComponentDetail);
	CommandTable::addCppFunction("zoneMap",   commandFuncZoneMap);
	CommandTable::addCppFunction("weaponGroupAssignment",   commandFuncWeaponGroupAssignment);
	CommandTable::addCppFunction("questJournal",   commandFuncQuestJournal);
	CommandTable::addCppFunction("chatterSpeech",   commandFuncChatterSpeech);
	CommandTable::addCppFunction("logout", commandFuncLogout);
	CommandTable::addCppFunction("waypoint", commandFuncWaypoint);
	CommandTable::addCppFunction("externalCommand", commandFuncExternalCommand);
	CommandTable::addCppFunction("collections", commandFuncCollections);
	//endspace

	CommandTable::addCppFunction("voiceInvite", commandFuncVoiceInvite);
	CommandTable::addCppFunction("voiceKick", commandFuncVoiceKick);

	//CommandTable::addCppFunction("tcg", commandFuncTcg);

	CommandTable::addCppFunction("equipAppearance", commandFuncEquipAppearance);
	CommandTable::addCppFunction("unequipAppearance", commandFuncUnequipAppearance);
	CommandTable::addCppFunction("ooc", commandFuncToggleOutOfCharacter);

	CommandTable::addCppFunction("appearanceTab", commandFuncAppearanceTab);
	CommandTable::addCppFunction("chronicleQuestBuilder", commandFuncQuestBuilder);
	CommandTable::addCppFunction("openStorytellerRecipe", commandFuncOpenStorytellerRecipe);
	CommandTable::addCppFunction("gcwInfo", commandFuncGCWInfo);
	CommandTable::addCppFunction("lfw", commandFuncToggleLookingForWork);
}

// ----------------------------------------------------------------------

void CommandCppFuncs::remove()
{
}

// ======================================================================
