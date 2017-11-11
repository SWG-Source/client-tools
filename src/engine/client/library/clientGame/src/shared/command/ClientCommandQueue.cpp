// ======================================================================
//
// ClientCommandQueue.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/GroundCombatActionManager.h"

#include "StringId.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/CombatDataTable.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/ProsePackage.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"
#include "clientGame/CellObject.h"
#include "clientGame/ClientCommandChecks.h"
#include "clientGame/ClientCommandQueueEntry.h"
#include "clientGame/ClientStringIds.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/WeaponObject.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiVoiceChatManager.h"
#include "clientGame/ProsePackageManagerClient.h"
#include <queue>

//!!---------------------
//!! @todo: these include paths are errors!
//!!---------------------
#include "swgSharedUtility/Locomotions.h"
#include "swgSharedUtility/States.h"

// ======================================================================

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const ClientCommandQueue::Messages::Added::Payload &, ClientCommandQueue::Messages::Added >
			added;
		MessageDispatch::Transceiver<const ClientCommandQueue::Messages::Removing::Payload &, ClientCommandQueue::Messages::Removing >
			removing;
		MessageDispatch::Transceiver<const ClientCommandQueue::Messages::CommandTimerDataUpdated::Payload &, ClientCommandQueue::Messages::CommandTimerDataUpdated >
			commandTimerDataUpdated;
	}

	const char * const ClientOnlyGroupName = "client_only";
	const unsigned int SEQUENCE_INCREMENT_AMOUNT = 32u;
	const unsigned int SEQUENCE_NORMAL_BIT = 0x40000000u;
	const unsigned int SEQUENCE_MASK = SEQUENCE_NORMAL_BIT-1;

	const float        MAX_QUEUED_COMMANDS = 16.0f;

	const double       TOO_MANY_CMDS_NOTIFICATION_FREQUENCY = 5; //seconds

	char               ms_removingCommandBuffer[256];

#ifdef _DEBUG
	const bool         cms_debug = false; // set to true to enable debug logging
#endif

	bool s_commandsAreNowFromToolbar = false;
	bool s_useCombatTargeting = false;

	const uint32 NPC_CONVERSTATION_START_COMMAND            = Crc::normalizeAndCalculate("npcConversationStart");
}

// ======================================================================

bool                                   ClientCommandQueue::ms_installed = false;
uint32                                 ClientCommandQueue::ms_nextSequenceId = 0;
ClientCommandQueue::EntryMap           ClientCommandQueue::ms_commandQueue;
std::queue<ClientCommandQueue::Entry>  ClientCommandQueue::ms_clientOnlyQueue;
float                                  ClientCommandQueue::ms_cutoffTime;
double                                 ClientCommandQueue::ms_timeUntilPrimaryCommandExecuteCompletes = 0;
double                                 ClientCommandQueue::ms_timeUntilSecondaryCommandExecuteCompletes = 0;
bool								   ClientCommandQueue::ms_isSecondaryCommand = false;
ClientCommandQueue::CooldownMap        ClientCommandQueue::ms_cooldownMap;

// ======================================================================


static void determineTargetLoc(Unicode::String &processedParams)
{
	Vector loc;
	NetworkId cellId;
	if (GroundCombatActionManager::getSecondaryAttackLocation(loc, cellId) || GroundCombatActionManager::getPrimaryAttackLocation(loc,cellId))
	{
		CellObject *cellObject = safe_cast<CellObject *>(NetworkIdManager::getObjectById(cellId));
		char tmp[512];
		Vector localPoint;
		if(cellObject)
			localPoint = cellObject->rotateTranslate_w2o(loc);
		sprintf(tmp, "%f %f %f %s %f %f %f ", loc.x, loc.y, loc.z, cellId.getValueString().c_str(),	localPoint.x, localPoint.y, localPoint.z);
		processedParams = Unicode::narrowToWide(tmp);			
	}
}

void ClientCommandQueue::determineTarget(Unicode::String const & params, NetworkId &processedTarget, Unicode::String &processedParams, bool const includeStealthCreature)
{
	const CreatureObject * const player = Game::getPlayerCreature ();

	// see if the first argument looks like a target
	size_t curPos = 0, endPos = 0;
	Unicode::String token;
	if (Unicode::getFirstToken(params, curPos, endPos, token))
	{
		const Unicode::NarrowString narrowLowerToken(Unicode::toLower(Unicode::wideToNarrow(token)));
		const Object * target = 0;
		if (narrowLowerToken == "self" || narrowLowerToken == "me" || narrowLowerToken == "this")
			target = player;
		else if (narrowLowerToken == "none")
		{
			target = NULL;
			processedTarget = NetworkId::cms_invalid;
			processedParams.clear();
			return;
		}
		else if (player)
			target = ClientWorld::findObjectByLocalizedAbbrev(player->getPosition_w(), token);
		if (target)
		{
			bool validTarget = true;
			const ClientObject* const clientObject = target ? target->asClientObject () : 0;
			const CreatureObject* const creatureObject = clientObject ? clientObject->asCreatureObject () : 0;			
			if(creatureObject && !creatureObject->getCoverVisibility() && !includeStealthCreature && !PlayerObject::isAdmin() && !creatureObject->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
				validTarget = false;			
			if(validTarget)
			{			
				curPos = Unicode::skipWhitespace(params, endPos);
				if (curPos == Unicode::String::npos)
					processedParams.clear();
				else
					processedParams = params.substr(curPos);
				processedTarget = target->getNetworkId();
				return;
			}
		}
#if PRODUCTION == 0
		if (narrowLowerToken[0] == '>' && isdigit(narrowLowerToken[1]))
		{
			curPos = Unicode::skipWhitespace(params, endPos);
			if (curPos == Unicode::String::npos)
				processedParams.clear();
			else
				processedParams = params.substr(curPos);
			processedTarget = NetworkId(narrowLowerToken.c_str()+1);
		}
#endif
	}

	processedParams = params;
	if (player && !processedTarget.isValid ())
	{
		if (!Game::isSpace() && s_useCombatTargeting && CuiPreferences::getAutoAimToggle())
			processedTarget = player->getIntendedTarget();
		else
			processedTarget = player->getLookAtTarget();
	}
}

// ======================================================================

void ClientCommandQueue::install()
{
	InstallTimer const installTimer("ClientCommandQueue::install");

	DEBUG_FATAL(ms_installed, ("already installed\n"));

	sprintf(ms_removingCommandBuffer, "RemovingCommand:\n");
	CrashReportInformation::addDynamicText(ms_removingCommandBuffer);

	ms_cutoffTime = 0.0f;
	ms_installed = true;
}

// ----------------------------------------------------------------------

void ClientCommandQueue::remove()
{
	DEBUG_FATAL(!ms_installed, ("not installed\n"));

	CrashReportInformation::removeDynamicText(ms_removingCommandBuffer);

	ms_installed = false;
}

// ----------------------------------------------------------------------

bool ClientCommandQueue::isFull()
{
	return ms_cutoffTime >= 1.0f;
}

// ----------------------------------------------------------------------

// this is a very ugly hacky way to do this -- TMF
// I blame the SwgCuiToolbar and clicky combat for making me do this
void ClientCommandQueue::commandsAreNowFromToolbar(bool const enabled)
{
	s_commandsAreNowFromToolbar = enabled;
}

void ClientCommandQueue::useCombatTargeting(bool const enabled)
{
	s_useCombatTargeting = enabled;
}


// ----------------------------------------------------------------------

uint32 ClientCommandQueue::enqueueCommand(Command const &command, NetworkId const &targetId, Unicode::String const &params)
{
	DEBUG_REPORT_LOG(cms_debug, ("ClientCommandQueue::enqueueCommand(%s, %s, %s)\n", command.m_commandName.c_str(), targetId.getValueString().c_str(), Unicode::wideToNarrow(params).c_str()));

	//this function can safely be called when the system is not installed (it will just return trivially)
	if(!ms_installed)
		return 0;

	CreatureObject * const player = Game::getPlayerCreature ();
	
	if (player == NULL)
	{
		return 0;
	}

	if ((command.m_toolbarOnly) && (!s_commandsAreNowFromToolbar))
	{
		return 0;	
	}


	if (isFull())
	{
#if PRODUCTION == 0
		//only show the "too many commands queued" messages once every X seconds - don't want to spam the user with this
		double static nextMsgTime = 0;
		double const currentTime = Clock::getCurrentTime();

		if (currentTime >= nextMsgTime)
		{
			nextMsgTime = currentTime + TOO_MANY_CMDS_NOTIFICATION_FREQUENCY;

			ProsePackage p;
			p.stringId = ClientStringIds::too_many_commands_queued;
			std::string lowerCommand = Unicode::toLower(command.m_commandName);
			p.target.stringId = StringId("cmd_n", lowerCommand);
			Unicode::String result;
			ProsePackageManagerClient::appendTranslation(p, result);
			CuiSystemMessageManager::sendFakeSystemMessage (result);
		}
#endif
		return 0;
	}

	NetworkId processedTarget;
	Unicode::String processedParams;

	// for Vivox integration phase 1 we need to do special handling of
	// report to send the client connection info across
	static uint32 const hash_report = Crc::normalizeAndCalculate("report");

	// need to do custom handling for the some group commands
	static uint32 const hash_groupInvite = Crc::normalizeAndCalculate("invite");
	static uint32 const hash_groupUninvite = Crc::normalizeAndCalculate("uninvite");
	static uint32 const hash_groupMakeLeader = Crc::normalizeAndCalculate("makeLeader");
	static uint32 const hash_groupMakeMasterLooter = Crc::normalizeAndCalculate("makeMasterLooter");
	static uint32 const hash_groupDismissGroupMember = Crc::normalizeAndCalculate("dismissGroupMember");
	static uint32 const hash_groupLeaveGroup = Crc::normalizeAndCalculate("leaveGroup");

	// need to do custom handling for the "assist" command
	static uint32 const hash_assist = Crc::normalizeAndCalculate("assist");
	if (command.m_commandHash == hash_assist)
	{
		if (params.empty())
		{
			// if no parameters specified, pass processedTarget=0
			// and processedParams="" to command handler
			processedTarget = NetworkId::cms_invalid;
			processedParams.clear();
		}
		else
		{
			determineTarget(params, processedTarget, processedParams, false);

			if (params == processedParams)
			{
				// if parameters did not specify a valid object, pass processedTarget=0
				// and processedParams=params to command handler
				processedTarget = NetworkId::cms_invalid;
			}
			else if (!processedTarget.isValid())
			{
				// if parameters did not specify a valid object, pass processedTarget=0
				// and processedParams=params to command handler
				processedParams = params;
			}
			else
			{
				// if parameters did specify a valid object, pass processedTarget=oid
				// and processedParams="" to command handler
				processedParams.clear();
			}
		}
	}
	else
	{
		// do target substitution as necessary
		if (command.m_targetType == Command::CTT_None || targetId != NetworkId::cms_invalid)
		{
			processedTarget = targetId;
			processedParams = params;
		}
		else if (command.m_targetType == Command::CTT_Location)
		{
			determineTargetLoc(processedParams);
		}
		else
		{
			// waypoint and wp commands in space need special processing
			static uint32 const hash_waypoint = Crc::normalizeAndCalculate("waypoint");
			static uint32 const hash_wp = Crc::normalizeAndCalculate("wp");

			// rotateFurniture and moveFurniture commands need special processing
			static uint32 const hash_rotateFurniture = Crc::normalizeAndCalculate("rotateFurniture");
			static uint32 const hash_moveFurniture = Crc::normalizeAndCalculate("moveFurniture");

			// spammer and unspammer commands can be used on a stealth target
			static uint32 const hash_spammer = Crc::normalizeAndCalculate("spammer");
			static uint32 const hash_unspammer = Crc::normalizeAndCalculate("unspammer");

			// deputizeWarden and undeputizeWarden commands need special processing
			static uint32 const hash_deputizeWarden = Crc::normalizeAndCalculate("deputizeWarden");
			static uint32 const hash_undeputizeWarden = Crc::normalizeAndCalculate("undeputizeWarden");

			// need to do custom handling for the "spammer" and "unspammer" command
			if ((command.m_commandHash == hash_spammer) || (command.m_commandHash == hash_unspammer))
			{
				// if the player specified a command line target, always pass the
				// command line target to the server for target resolution there;
				// we don't want to use the client's partial name resolution as that
				// may resolve into an incorrect/unintended target, as the player may
				// intend as the command line target someone on a different planet
				if (!params.empty())
				{
					processedTarget = NetworkId::cms_invalid;
					processedParams = params;
				}
				else
				{
					determineTarget(params, processedTarget, processedParams, true);
				}
			}
			else if ((command.m_commandHash == hash_groupInvite) || (command.m_commandHash == hash_groupUninvite) || (command.m_commandHash == hash_deputizeWarden) || (command.m_commandHash == hash_undeputizeWarden))
			{
				determineTarget(params, processedTarget, processedParams, false);

				// if processedTarget is not a player character, then pass
				// params to the server for the server to attempt to resolve
				// it to a player character; that is, if processedTarget is
				// an object that is not a player character, it is definitely
				// an incorrect target, so we will try to have the server resolve
				// params to see if the server can resolve it to a player character
				if (!params.empty())
				{
					if (!processedTarget.isValid())
					{
						processedParams = params;
					}
					else
					{
						const Object * const object = NetworkIdManager::getObjectById(processedTarget);
						const ClientObject * const clientObject = (object ? object->asClientObject() : NULL);
						const CreatureObject * const creatureObject = (clientObject ? clientObject->asCreatureObject() : NULL);
						const PlayerObject * const playerObject = (creatureObject ? creatureObject->getPlayerObject() : NULL);
						if (!playerObject)
						{
							processedTarget = NetworkId::cms_invalid;
							processedParams = params;
						}
					}
				}
			}
			else if (command.m_commandHash == hash_groupMakeLeader)
			{
				// don't bother if player is not in a group or not group leader
				GroupObject const * const group = safe_cast<GroupObject const *>(player->getGroup().getObject());
				if (!group)
					return 0;

				if (group->getLeader() != player->getNetworkId())
				{
					CuiSystemMessageManager::sendFakeSystemMessage(StringId("group", "must_be_leader").localize());
					return 0;
				}

				determineTarget(params, processedTarget, processedParams, false);

				// if processedTarget is a group member, then we are done,
				// otherwise, and if params is specified, try to match
				// params with a group member
				if (!group->isGroupMember(processedTarget))
				{
					if (params.empty())
						return 0;

					bool groupLeader = false;
					GroupObject::GroupMember const * const gm = group->findMember(Unicode::wideToNarrow(params), groupLeader);
					if (!gm || groupLeader)
						return 0;

					processedTarget = gm->first;
				}
				else if (processedTarget == player->getNetworkId())
				{
					// player is already group leader
					return 0;
				}
			}
			else if (command.m_commandHash == hash_groupMakeMasterLooter)
			{
				// don't bother if player is not in a group or not group leader
				GroupObject const * const group = safe_cast<GroupObject const *>(player->getGroup().getObject());
				if (!group)
					return 0;

				if (group->getLeader() != player->getNetworkId())
				{
					CuiSystemMessageManager::sendFakeSystemMessage(StringId("group", "must_be_leader").localize());
					return 0;
				}

				determineTarget(params, processedTarget, processedParams, false);

				// if processedTarget is a group member, then we are done,
				// otherwise, and if params is specified, try to match
				// params with a group member
				if (!group->isGroupMember(processedTarget))
				{
					if (!params.empty())
					{
						bool groupLeader = false;
						GroupObject::GroupMember const * const gm = group->findMember(Unicode::wideToNarrow(params), groupLeader);
						if (gm)
						{
							if (gm->first == group->getLootMaster())
							{
								// target is already the loot master
								return 0;
							}

							processedTarget = gm->first;
						}
						else
						{
							// specified a non group member as target
							return 0;
						}
					}
					else if (processedTarget.isValid())
					{
						// specified a non group member as target
						return 0;
					}
				}
				else if (processedTarget == group->getLootMaster())
				{
					// target is already the loot master
					return 0;
				}
			}
			else if ((command.m_commandHash == hash_groupDismissGroupMember) || (command.m_commandHash == hash_groupLeaveGroup))
			{
				// don't bother if player is not in a group or not group leader
				GroupObject const * const group = safe_cast<GroupObject const *>(player->getGroup().getObject());
				if (!group)
					return 0;

				determineTarget(params, processedTarget, processedParams, false);

				// if processedTarget is a group member, then we are done,
				// otherwise, and if params is specified, try to match
				// params with a group member
				if (!group->isGroupMember(processedTarget))
				{
					if (!params.empty())
					{
						bool groupLeader = false;
						GroupObject::GroupMember const * const gm = group->findMember(Unicode::wideToNarrow(params), groupLeader);
						if (gm)
						{
							processedTarget = gm->first;
						}
					}
				}

				// if group leader, don't allow command if a param is specified
				// and processedTarget is cms_invalid, because that will cause
				// the server to disband the group, and if a param is specified,
				// it could just mean a typo, which shouldn't disband the group
				if (!processedTarget.isValid() && !params.empty() && (group->getLeader() == player->getNetworkId()))
				{
					return 0;
				}
			}
			else if ((command.m_commandHash == hash_waypoint) || (command.m_commandHash == hash_wp))
			{
				// if the first param matches a planet/space zone name, don't try to
				// resolve that to a target, instead send it to the server to be
				// interpreted as the planet/space zone on which to create the waypoint
				static std::set<std::string> planetOrZoneNames;
				if (planetOrZoneNames.empty())
				{
					// this list ***MUST*** be kept in sync with WAYPOINT_GROUND_PLANETS_EXTERNAL
					// and WAYPOINT_SPACE_ZONES_EXTERNAL in base_player.script
					IGNORE_RETURN(planetOrZoneNames.insert("tatooine"));
					IGNORE_RETURN(planetOrZoneNames.insert("naboo"));
					IGNORE_RETURN(planetOrZoneNames.insert("corellia"));
					IGNORE_RETURN(planetOrZoneNames.insert("rori"));
					IGNORE_RETURN(planetOrZoneNames.insert("talus"));
					IGNORE_RETURN(planetOrZoneNames.insert("yavin4"));
					IGNORE_RETURN(planetOrZoneNames.insert("endor"));
					IGNORE_RETURN(planetOrZoneNames.insert("lok"));
					IGNORE_RETURN(planetOrZoneNames.insert("dantooine"));
					IGNORE_RETURN(planetOrZoneNames.insert("dathomir"));
					IGNORE_RETURN(planetOrZoneNames.insert("kachirho"));
					IGNORE_RETURN(planetOrZoneNames.insert("etyyy"));
					IGNORE_RETURN(planetOrZoneNames.insert("khowir"));
					IGNORE_RETURN(planetOrZoneNames.insert("mustafar"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_tatooine"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_naboo"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_corellia"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_deep"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_kessel"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_yavin4"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_endor"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_lok"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_dantooine"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_dathomir"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_kashyyyk"));
					IGNORE_RETURN(planetOrZoneNames.insert("space_nova_orion"));
				}

				bool planetOrZoneNameSpecified = false;
				if (!params.empty())
				{
					size_t curPos = 0, endPos = 0;
					Unicode::String token;
					if (Unicode::getFirstToken(params, curPos, endPos, token))
					{
						const Unicode::NarrowString narrowLowerToken(Unicode::toLower(Unicode::wideToNarrow(token)));
						if (planetOrZoneNames.count(narrowLowerToken) > 0)
						{
							planetOrZoneNameSpecified = true;
							processedTarget = NetworkId::cms_invalid;
							processedParams = params;
						}
					}
				}

				if (!planetOrZoneNameSpecified)
				{
					determineTarget(params, processedTarget, processedParams, false);
				}
			}
			else if (command.m_commandHash == hash_rotateFurniture)
			{
				// if the first param matches a valid sub-command, don't try to
				// resolve that to a target, instead send it to the server to be
				// interpreted as the sub-command
				static std::set<std::string> rotateFurnitureSubCommands;
				if (rotateFurnitureSubCommands.empty())
				{
					// this list ***MUST*** be kept in sync with the possible
					// sub-commands for /rotateFurniture in rotateFurniture()
					// in player_building.script
					IGNORE_RETURN(rotateFurnitureSubCommands.insert("yaw"));
					IGNORE_RETURN(rotateFurnitureSubCommands.insert("pitch"));
					IGNORE_RETURN(rotateFurnitureSubCommands.insert("roll"));
					IGNORE_RETURN(rotateFurnitureSubCommands.insert("random"));
					IGNORE_RETURN(rotateFurnitureSubCommands.insert("reset"));
					IGNORE_RETURN(rotateFurnitureSubCommands.insert("copy"));
					IGNORE_RETURN(rotateFurnitureSubCommands.insert("save"));
					IGNORE_RETURN(rotateFurnitureSubCommands.insert("restore"));
				}

				bool subCommandSpecified = false;
				if (!params.empty())
				{
					size_t curPos = 0, endPos = 0;
					Unicode::String token;
					if (Unicode::getFirstToken(params, curPos, endPos, token))
					{
						const Unicode::NarrowString narrowLowerToken(Unicode::toLower(Unicode::wideToNarrow(token)));
						if (rotateFurnitureSubCommands.count(narrowLowerToken) > 0)
						{
							subCommandSpecified = true;
							processedTarget = NetworkId::cms_invalid;
							processedParams = params;
						}
					}
				}

				if (!subCommandSpecified)
				{
					determineTarget(params, processedTarget, processedParams, false);
				}
			}
			else if (command.m_commandHash == hash_moveFurniture)
			{
				// if the first param matches a valid sub-command, don't try to
				// resolve that to a target, instead send it to the server to be
				// interpreted as the sub-command
				static std::set<std::string> moveFurnitureSubCommands;
				if (moveFurnitureSubCommands.empty())
				{
					// this list ***MUST*** be kept in sync with the possible
					// sub-commands for /moveFurniture in moveFurniture()
					// in player_building.script
					IGNORE_RETURN(moveFurnitureSubCommands.insert("forward"));
					IGNORE_RETURN(moveFurnitureSubCommands.insert("back"));
					IGNORE_RETURN(moveFurnitureSubCommands.insert("left"));
					IGNORE_RETURN(moveFurnitureSubCommands.insert("right"));
					IGNORE_RETURN(moveFurnitureSubCommands.insert("up"));
					IGNORE_RETURN(moveFurnitureSubCommands.insert("down"));
					IGNORE_RETURN(moveFurnitureSubCommands.insert("copy"));
				}

				bool subCommandSpecified = false;
				if (!params.empty())
				{
					size_t curPos = 0, endPos = 0;
					Unicode::String token;
					if (Unicode::getFirstToken(params, curPos, endPos, token))
					{
						const Unicode::NarrowString narrowLowerToken(Unicode::toLower(Unicode::wideToNarrow(token)));
						if (moveFurnitureSubCommands.count(narrowLowerToken) > 0)
						{
							subCommandSpecified = true;
							processedTarget = NetworkId::cms_invalid;
							processedParams = params;
						}
					}
				}

				if (!subCommandSpecified)
				{
					determineTarget(params, processedTarget, processedParams, false);
				}
			}
			else
			{
				determineTarget(params, processedTarget, processedParams, false);
			}
		}
	}

	// don't allow player to mount a vehicle/pet
	// if the vehicle/pet is behind a wall
	static uint32 const hash_mount = Crc::normalizeAndCalculate("mount");
	if (command.m_commandHash == hash_mount)
	{
		if (processedTarget.isValid())
		{
			const Object * const mountObj = NetworkIdManager::getObjectById(processedTarget);
			if (mountObj)
			{
				if (!allowMountCommand(*player, *mountObj))
					return 0;
			}
		}
	}
	else if (command.m_commandHash == hash_report)
	{
		processedParams.append(Unicode::narrowToWide("|" + CuiVoiceChatManager::getCsReportString()));
	}

	const Entry * entryPtr = 0;

	if (command.m_cppHook)
	{
		ms_clientOnlyQueue.push(Entry(command, processedTarget, processedParams));

		if (isClientOnlyCommand(command) || Game::getSinglePlayer ())
			return 0;
	}

	uint32 sequenceId = ms_nextSequenceId;
	if (!command.isNull())
	{
		// Assign next sequenceId for client-visible commands.
		if (command.m_visibleToClients)
		{
			sequenceId = nextSequenceId();
			if (command.m_defaultPriority == Command::CP_Normal)
				sequenceId |= SEQUENCE_NORMAL_BIT;
			entryPtr = &(ms_commandQueue[sequenceId] = Entry(command, processedTarget, processedParams));
		}

		// Update cooldown timers for combat commands.
		if (command.m_addToCombatQueue)
		{
			const bool bIsPrimaryCommand = !ClientCommandQueue::ms_isSecondaryCommand;
			updateCombatCommandTime(bIsPrimaryCommand, command, sequenceId);
		}
	}

	if(processedParams.empty() && command.m_failScriptHook.find("failSpecialAttack") != Unicode::String::npos)
	{
		// Special attack params should NEVER be empty.

		if (CuiPreferences::getAutoAimToggle())
			processedParams += Unicode::narrowToWide("a");
		else
			processedParams += Unicode::narrowToWide("c");
	}

	//-- Only enqueue the command if we're multi-player.  Tools crash here if we're single player.
	if (!Game::getSinglePlayer())
		safe_cast<PlayerCreatureController*>(player->getController())->sendCommandQueueEnqueue(sequenceId, command.m_commandHash, processedTarget, processedParams);

	if(command.m_defaultPriority == Command::CP_Immediate)
		ms_cutoffTime += 1/MAX_QUEUED_COMMANDS;

	if (entryPtr)
		Transceivers::added.emitMessage (Messages::Added::Payload (sequenceId, entryPtr));

	return sequenceId;
}

// ----------------------------------------------------------------------

uint32 ClientCommandQueue::enqueueCommand(uint32 commandHash, NetworkId const &targetId, Unicode::String const &params)
{
	return enqueueCommand(CommandTable::getCommand(commandHash), targetId, params);
}

// ----------------------------------------------------------------------

uint32 ClientCommandQueue::enqueueCommand(std::string const &commandName, NetworkId const &targetId, Unicode::String const &params)
{
	return enqueueCommand(CommandTable::getCommand(Crc::normalizeAndCalculate(commandName.c_str())), targetId, params);
}

// ----------------------------------------------------------------------

void ClientCommandQueue::clear()
{
	DEBUG_REPORT_LOG(cms_debug, ("ClientCommandQueue::clear()\n"));

	Object * const player = Game::getPlayer ();
	if (player)
	{
		// player wants to remove all commands, so notify the server
		// this is done by telling it to remove sequenceId 0, which means everything
		PlayerCreatureController * const playerController = safe_cast<PlayerCreatureController*>(player->getController());
		playerController->sendCommandQueueRemove(0);
		// clear the queue locally.  note that you shouldn't clear the current command - nothing can stop that.
		ClientCommandQueue::EntryMap::const_iterator it = ms_commandQueue.begin ();
		if(it != ms_commandQueue.end())
		{
			std::pair<uint32, Entry> firstValue = *it;
			ms_commandQueue.clear();

			double const currentTime = Clock::getCurrentTime();
			double timeUntilExecuteCompletes = (firstValue.second.m_command->isPrimaryCommand()) 
				? ms_timeUntilPrimaryCommandExecuteCompletes 
				: ms_timeUntilSecondaryCommandExecuteCompletes;

			// Determine if we are in the middle of executing it; if so, can't remove it
			if (currentTime < timeUntilExecuteCompletes)
			{
				ms_commandQueue.insert(firstValue);
			}
		}		
	}
}

// ----------------------------------------------------------------------

ClientCommandQueue::EntryMap const &ClientCommandQueue::get()
{
	return ms_commandQueue;
}

// ----------------------------------------------------------------------

void ClientCommandQueue::handleCommandRemoved(uint32 sequenceId, float waitTime, Command::ErrorCode status, int statusDetail)
{
	DEBUG_REPORT_LOG(cms_debug, ("ClientCommandQueue::handleCommandRemoved(%d, %f, %d, %d)\n", static_cast<int>(sequenceId), waitTime, status, statusDetail));

	UNREF(statusDetail);
	// immediately remove command from the local queue
	const ClientCommandQueue::EntryMap::iterator i = ms_commandQueue.find(sequenceId);
	if (i != ms_commandQueue.end())
	{
		const Entry & entry = (*i).second;

		sprintf(ms_removingCommandBuffer, "RemovingCommand: name=[%s] status=[%d] statusDetail=[%d]\n", 
			entry.m_command ? entry.m_command->m_commandName.c_str() : "???",
			status,
			statusDetail);

		Transceivers::removing.emitMessage (Messages::Removing::Payload (sequenceId, waitTime, &entry, status, statusDetail));

		if (status != Command::CEC_Success)
		{
			const Command * const cmd = entry.m_command;
			if (cmd)
			{
				// close the initial booting crafting station screen if it's up
				if(CuiCraftManager::isCraftingStartCommand(cmd->m_commandHash))
				{
					CuiCraftManager::abortCraftingStart();
				}

				// reset the cooldown for failed commands
				{
					ms_timeUntilPrimaryCommandExecuteCompletes = 0;
					ms_timeUntilSecondaryCommandExecuteCompletes = 0;

					// Update cooldown timer component #2
					CooldownMap::iterator i = ms_cooldownMap.find(std::make_pair(Game::getPlayerNetworkId(), cmd->m_coolGroup));
					if (i != ms_cooldownMap.end())
					{
						CooldownMapEntry & cooldown = i->second;
						if (sequenceId == cooldown.lastSequenceId)
						{
							cooldown.endTime = 0;
						}
					}

					// Inform UI that command failed and cooldown timer has reset.
					MessageQueueCommandTimer msg(sequenceId, cmd->m_coolGroup, NULL_COOLDOWN_GROUP, Crc::normalizeAndCalculate(cmd->m_commandName.c_str()));
					msg.setCurrentTime(MessageQueueCommandTimer::F_execute, 0.0f);
					msg.setMaxTime    (MessageQueueCommandTimer::F_execute, 0.0f);
					msg.setCurrentTime(MessageQueueCommandTimer::F_cooldown, 0.0f);
					msg.setMaxTime    (MessageQueueCommandTimer::F_cooldown, 0.0f);
					msg.setFailed();
					Transceivers::commandTimerDataUpdated.emitMessage(msg);
				}
				
				if (shouldGenerateAndDisplayCommandRemovedMessage(*cmd, status, statusDetail))
				{
					static Unicode::String result;
					result.clear ();
					ClientCommandQueue::generateCommandRemovedMessage (result, *cmd, status, statusDetail);
					CuiSystemMessageManager::sendFakeSystemMessage (result);
				}
			}
		}

		ms_commandQueue.erase(i);
	}
}

//----------------------------------------------------------------------

void ClientCommandQueue::generateCommandRemovedMessage (Unicode::String & result, const Command & cmd, Command::ErrorCode status, int statusDetail)
{
	static Unicode::String cmdName;
	cmdName.clear ();

	CuiSkillManager::localizeCmdName (Unicode::toLower (cmd.m_commandName), cmdName);
	const StringId & stringId = Command::getStringIdForErrorCode (status);

	Unicode::String arg;
	if (status == Command::CEC_Locomotion)
		arg = Locomotions::getLocomotionStringId (statusDetail).localize ();
	else if ((status == Command::CEC_StateMustNotHave) || (status == Command::CEC_StateMustHave))
		arg = States::getStateStringId (statusDetail).localize ();

	CuiStringVariablesManager::process (stringId, arg, Unicode::emptyString, cmdName, result);
}

// ----------------------------------------------------------------------

uint32 ClientCommandQueue::nextSequenceId()
{
	ms_nextSequenceId = (ms_nextSequenceId + SEQUENCE_INCREMENT_AMOUNT) & SEQUENCE_MASK;
	return ms_nextSequenceId;
}

// ----------------------------------------------------------------------

const ClientCommandQueue::Entry * ClientCommandQueue::findEntry(uint32 sequenceId)
{
	const ClientCommandQueue::EntryMap::const_iterator i = ms_commandQueue.find(sequenceId);
	if (i != ms_commandQueue.end())
		return &(*i).second;

	return 0;
}

// ----------------------------------------------------------------------

bool ClientCommandQueue::isClientOnlyCommand(Command const &command)
{
	static const uint32 hash_clientOnlyGroup = Crc::normalizeAndCalculate(ClientOnlyGroupName);
	return (command.m_commandGroup == hash_clientOnlyGroup);
}

// ----------------------------------------------------------------------

void ClientCommandQueue::executeClientCommands()
{
	PROFILER_AUTO_BLOCK_DEFINE("ClientCommandQueue::executeClientCommands");

	Object *player = Game::getPlayer();
	if (player)
	{
		NetworkId playerId = player->getNetworkId();
		while (!ms_clientOnlyQueue.empty())
		{
			Entry &e = ms_clientOnlyQueue.front();
			if (e.m_command && e.m_command->m_cppHook)
				(*e.m_command->m_cppHook)(*e.m_command, playerId, e.m_targetId, e.m_params);
			ms_clientOnlyQueue.pop();
		}
	}
}

// ----------------------------------------------------------------------

void ClientCommandQueue::update(float time)
{
	//subtract from the cutoff value, allowing more commands to go through
	ms_cutoffTime -= time;
	ms_cutoffTime = clamp(0.0f, ms_cutoffTime, 1.0f);
}

// ----------------------------------------------------------------------

bool ClientCommandQueue::canEnqueueCombatCommand(bool bPrimaryCommand,Command const &command)
{
	bool result = true;

	// Right now, we will only care about combat commands
	if (command.m_addToCombatQueue)
	{
		double const currentTime = Clock::getCurrentTime();
		double timeUntilExecuteCompletes = (bPrimaryCommand) 
			? ms_timeUntilPrimaryCommandExecuteCompletes 
			: ms_timeUntilSecondaryCommandExecuteCompletes;


		// Determine if we are in the middle of another combat action
		if (currentTime < timeUntilExecuteCompletes)
		{
			// Busy executing another command
			result = false;
		}
		else
		{
			// Check the cooldown timers
			CooldownMap::const_iterator i = ms_cooldownMap.find(std::make_pair(Game::getPlayerNetworkId(), command.m_coolGroup));
			if (i != ms_cooldownMap.end())
			{
				// An entry exists in the map, so check the end-time for the cooldown
				if (currentTime < i->second.endTime)
				{
					// We are busy cooling down for that group
					result = false;
				}
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------

double ClientCommandQueue::getCooldownRemainingForCommand(Command const &command)
{
	double result = 0.0f;

	// Right now, we will only care about combat commands
	if (command.m_addToCombatQueue)
	{
		double const currentTime = Clock::getCurrentTime();

		// Check the cooldown timers
		CooldownMap::const_iterator i = ms_cooldownMap.find(std::make_pair(Game::getPlayerNetworkId(), command.m_coolGroup));
		if (i != ms_cooldownMap.end())
		{
			result = std::max(0.0, i->second.endTime - currentTime);
		}
	}

	return result;
}

// ----------------------------------------------------------------------

void ClientCommandQueue::updateCombatCommandTime(bool bPrimaryCommand, Command const &command, uint32 sequenceId)
{
	double const currentTime = Clock::getCurrentTime();

	// Store the time that this command will finish executing
	double timeUntilExecuteCompletes;
	if(bPrimaryCommand)
	{
		ms_timeUntilPrimaryCommandExecuteCompletes = currentTime + command.m_execTime;
		timeUntilExecuteCompletes = ms_timeUntilPrimaryCommandExecuteCompletes;
	}
	else
	{
		ms_timeUntilSecondaryCommandExecuteCompletes = currentTime + command.m_execTime;
		timeUntilExecuteCompletes = ms_timeUntilSecondaryCommandExecuteCompletes;
	}

	// Store the time that this command will finish cooling down
	if (command.m_coolGroup != 0)
	{
		CooldownMapEntry & cooldown = ms_cooldownMap[std::make_pair(Game::getPlayerNetworkId(), command.m_coolGroup)];
		cooldown.lastSequenceId = sequenceId;
		cooldown.endTime = timeUntilExecuteCompletes + getCooldownTime(command);
	}

	// This message is a temporary replacement for the message that comes from the server
	MessageQueueCommandTimer msg(
		sequenceId,
		command.m_coolGroup,
		NULL_COOLDOWN_GROUP,   // We should not be using cooldown group 2 anymore!
		Crc::normalizeAndCalculate(command.m_commandName.c_str()) );

	msg.setCurrentTime( MessageQueueCommandTimer::F_execute, 0.0f );
	msg.setMaxTime    ( MessageQueueCommandTimer::F_execute, command.m_execTime );
	msg.setCurrentTime( MessageQueueCommandTimer::F_cooldown, 0.0f );
	msg.setMaxTime    ( MessageQueueCommandTimer::F_cooldown, getCooldownTime(command) );
	// We should not be using cooldown group 2 anymore!
	//msg.setCurrentTime( MessageQueueCommandTimer::F_cooldown2, 0.0f );
	//msg.setMaxTime    ( MessageQueueCommandTimer::F_cooldown2, 0 );

	Transceivers::commandTimerDataUpdated.emitMessage(msg);
}

// ----------------------------------------------------------------------

void ClientCommandQueue::setIsSecondaryCommand(bool bIsSecondaryCommand)
{
	ClientCommandQueue::ms_isSecondaryCommand = bIsSecondaryCommand;
}

// ----------------------------------------------------------------------

void ClientCommandQueue::setCommandCooldown(uint32 cooldownGroupCrc, double currentCooldownTimeSecond, double maxCooldownTimeSecond, bool updateToolbar)
{
	double const currentTime = Clock::getCurrentTime();

	CooldownMap::iterator i = ms_cooldownMap.find(std::make_pair(Game::getPlayerNetworkId(), cooldownGroupCrc));
	if (i != ms_cooldownMap.end())
	{
		i->second.endTime = currentTime + static_cast<double>(maxCooldownTimeSecond - currentCooldownTimeSecond);
	}
	else
	{
		CooldownMapEntry & cooldown = ms_cooldownMap[std::make_pair(Game::getPlayerNetworkId(), cooldownGroupCrc)];
		cooldown.lastSequenceId = 0;
		cooldown.endTime = currentTime + static_cast<double>(maxCooldownTimeSecond - currentCooldownTimeSecond);
	}

	if(updateToolbar)
	{
		// Inform UI of the cooldown timer value
		MessageQueueCommandTimer msg(
			0,
			cooldownGroupCrc,
			-1,
			0);
		msg.setCurrentTime(MessageQueueCommandTimer::F_cooldown, static_cast<float>(currentCooldownTimeSecond));
		msg.setMaxTime    (MessageQueueCommandTimer::F_cooldown, static_cast<float>(maxCooldownTimeSecond));

		Transceivers::commandTimerDataUpdated.emitMessage(msg);
	}
}

// ----------------------------------------------------------------------

float ClientCommandQueue::getCooldownTime(Command const& cmd)
{
	if(cmd.isPrimaryCommand())
	{
		CreatureObject *obj = Game::getPlayerCreature();
		if(obj)
		{
			WeaponObject *weapon = obj->getCurrentWeapon();
			if(weapon)
				return weapon->getAttackSpeed();
		}
	}

	return cmd.m_coolTime;
}

// ----------------------------------------------------------------------

bool ClientCommandQueue::shouldGenerateAndDisplayCommandRemovedMessage(Command const& cmd, Command::ErrorCode status, int statusDetail)
{
	bool ret = true;

	// CEC_Cancelled comes from things that fail in script. We do not display a message  
	// because script displays a custom message in that case.
	if (status == Command::CEC_Cancelled)
	{
		ret = false;
	}
	else if(status == Command::CEC_Locomotion)
	{
		if(statusDetail == Locomotions::Standing && cmd.m_commandHash == NPC_CONVERSTATION_START_COMMAND)
		{
			ret = false;
		}
		else if(statusDetail == Locomotions::Incapacitated || statusDetail == Locomotions::Dead)
		{
			ret = false;
		}
	}
	return ret;
}

// ----------------------------------------------------------------------

bool ClientCommandQueue::allowMountCommand(Object const & rider, Object const & mount)
{
	// don't allow player to mount a vehicle/pet if the vehicle/pet is behind a wall
	if (!Game::isSpace() && rider.isInWorldCell() && mount.isInWorldCell())
	{
		const CellProperty *parentCell = rider.getParentCell();
		ClientWorld::CollisionInfoVector collisionInfo;
		if (ClientWorld::collide(parentCell, rider.getPosition_w(), mount.getPosition_w(), CollideParameters::cms_default, collisionInfo, ClientWorld::CF_tangible|ClientWorld::CF_tangibleNotTargetable))
		{
			Object const * collidedObj;
			ClientObject const * clientObject;
			const CollisionProperty *playerCollisionProperty = rider.getCollisionProperty();
			for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
			{
				collidedObj = collisionInfoIter->getObject();
				clientObject = (collidedObj ? collidedObj->asClientObject() : NULL);
				if (collidedObj && collidedObj->getNetworkId().isValid() &&
					clientObject && !clientObject->asCreatureObject() && !clientObject->asWeaponObject() &&
					playerCollisionProperty && playerCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()))
				{
					return false;
				}
			}
		}
	}

	return true;
}

// ======================================================================

