// ======================================================================
//
// ClientCommandQueue.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientCommandQueue_H
#define INCLUDED_ClientCommandQueue_H

// ======================================================================

#include "sharedGame/Command.h"
#include "clientGame/ClientCommandQueueEntry.h"
#include "sharedNetworkMessages/MessageQueueCommandTimer.h"

#include <queue>

// ======================================================================

class ClientCommandQueueEntry;
class NetworkId;
class Object;

//----------------------------------------------------------------------

class ClientCommandQueue // static class
{
public:
	
	typedef ClientCommandQueueEntry Entry;

	struct Messages
	{
		struct Added
		{
			typedef std::pair<uint32, const Entry *> Payload;
		};

		struct Removing
		{
			struct Payload
			{
				Payload(uint32 sequenceId_, float waitTime_, const Entry *commandEntry_, Command::ErrorCode status_, int _statusDetail) :
					sequenceId(sequenceId_),
					waitTime(waitTime_),
					commandEntry(commandEntry_),
					status(status_),
					statusDetail (_statusDetail)
				{
				}
				uint32         sequenceId;
				float          waitTime;
				const Entry *  commandEntry;
				Command::ErrorCode status;
				int                statusDetail;
			};
		};

		struct CommandTimerDataUpdated
		{
			typedef MessageQueueCommandTimer Payload;
		};
	};

	typedef stdmap<uint32, Entry>::fwd EntryMap;

	static void install();
	static void remove();

	static void commandsAreNowFromToolbar(bool const enabled);
	static void useCombatTargeting(bool const enabled);

	static uint32           enqueueCommand (Command const &command, NetworkId const &targetId, Unicode::String const &params);
	static uint32           enqueueCommand (uint32 commandHash, NetworkId const &targetId, Unicode::String const &params);
	static uint32           enqueueCommand (std::string const &commandName, NetworkId const &targetId, Unicode::String const &params);
	static void             clear          ();
	static EntryMap const & get            ();
	static const Entry *    findEntry      (uint32 sequenceId);
	static void             handleCommandRemoved (uint32 sequenceId, float waitTime, Command::ErrorCode status = Command::CEC_Success, int statusDetail = 0);
	static void             generateCommandRemovedMessage (Unicode::String & result, const Command & cmd, Command::ErrorCode status, int statusDetail);
	static void             executeClientCommands();
	static void             update         (float time);

	static bool             canEnqueueCombatCommand (bool bPrimaryCommand, Command const &command);
	static void             updateCombatCommandTime (bool bPrimaryCommand, Command const &command, uint32 sequenceId);
	static void				setIsSecondaryCommand (bool bIsSecondaryCommand );
	static double           getCooldownRemainingForCommand (Command const &command);

	static void             setCommandCooldown (uint32 cooldownGroupCrc, double currentCooldownTimeSecond, double maxCooldownTimeSecond, bool updateToolbar = true);

	static bool             allowMountCommand(Object const & rider, Object const & mount);

	static void             determineTarget(Unicode::String const & params, NetworkId &processedTarget, Unicode::String &processedParams, bool const includeStealthCreature);

private:
	                        ClientCommandQueue   (ClientCommandQueue const &);
	ClientCommandQueue&     operator=            (ClientCommandQueue const &);

	static uint32           nextSequenceId       ();
	static bool             isClientOnlyCommand  (Command const &command);
	static bool             isFull();

	static float            getCooldownTime(Command const& cmd);

	// so we can filter on displaying command remove messages
	static bool				shouldGenerateAndDisplayCommandRemovedMessage(Command const& cmd, Command::ErrorCode status, int statusDetail);

private:
	static bool                  ms_installed;
	static uint32                ms_nextSequenceId;
	static EntryMap              ms_commandQueue;
	static std::queue<Entry>     ms_clientOnlyQueue;
	static float                 ms_cutoffTime;
	static bool					 ms_isSecondaryCommand;

private:

	struct CooldownMapEntry
	{
		uint32 lastSequenceId;
		double endTime;
	};

	// map of cooldown group crc -> cooldown timer details
	typedef stdmap<std::pair<NetworkId, uint32>, CooldownMapEntry>::fwd CooldownMap;
	static double                ms_timeUntilPrimaryCommandExecuteCompletes;
	static double                ms_timeUntilSecondaryCommandExecuteCompletes;
	static CooldownMap           ms_cooldownMap;
};

//----------------------------------------------------------------------

#endif // INCLUDED_ClientCommandQueue_H

