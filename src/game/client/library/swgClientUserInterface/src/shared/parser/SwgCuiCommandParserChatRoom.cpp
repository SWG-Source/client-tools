//======================================================================
//
// SwgCuiCommandParserChatRoom.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserChatRoom.h"

#include "UIUtils.h"
#include "clientUserInterface/CuiChatRoomDataNode.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiStringIdsChatRoom.h"

#include <vector>

//======================================================================


namespace
{
	namespace Commands
	{
#define MAKE_COMMAND(a) const char * const a = #a
		MAKE_COMMAND (create);
		MAKE_COMMAND (destroy);
		MAKE_COMMAND (join);
		MAKE_COMMAND (leave);
		MAKE_COMMAND (list);
		MAKE_COMMAND (listAll);
		MAKE_COMMAND (refresh);
		MAKE_COMMAND (send);
		MAKE_COMMAND (who);
		MAKE_COMMAND (query);
		MAKE_COMMAND (invite);
		MAKE_COMMAND (inviteGroup);
		MAKE_COMMAND (uninvite);
		MAKE_COMMAND (invites);
		MAKE_COMMAND (info);
		MAKE_COMMAND (op);
		MAKE_COMMAND (deop);
		MAKE_COMMAND (ops);
		MAKE_COMMAND (kick);
		MAKE_COMMAND (ban);
		MAKE_COMMAND (unban);

#undef MAKE_COMMAND
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{ Commands::create,               4, "<room> <moderated> <public> <title>","Create a chat room"},
		{ Commands::destroy,              1, "<room>",                             "Destroy a chat room"},
		{ Commands::join,                 1, "<room>",                             "Enter a chat room"},
		{ Commands::leave,                0, "[room]",                             "Leave a chat room"},
		{ Commands::list,                 0, "",                                   "List joined chat rooms"},
		{ Commands::listAll,              0, "",                                   "List all chat rooms"},
		{ Commands::refresh,              0, "",                                   "Refresh chatroom list"},
		{ Commands::send,                 0, "<room> <msg>",                       "Send a chat message"},
		{ Commands::who,                  0, "[room]",                             "List ppl in a chat room"},
		{ Commands::query,                0, "[room]",                             "Request update for list of ppl in a chat room"},
		{ Commands::invite,               1, "<player> [room]",                    "Invite a player to the chat room" },
		{ Commands::inviteGroup,          1, "<group leader> [room]",              "Invite a group leader and his group to the chat room" },
		{ Commands::invites,              0, "[room]",                             "List the invitees for a chat room" },
		{ Commands::uninvite,             1, "<player> [room]",                    "Uninvite a player from the chat room" },
		{ Commands::info,                 0, "[room]",                             "Show some info about the chat room" },
		{ Commands::op,                   1, "<player> [room]",                    "Give a player moderator status in the chat room" },
		{ Commands::deop,                 1, "<player> [room]",                    "Revoke a player's moderator status" },
		{ Commands::ops,                  0, "[room]",                             "List the moderators of the room" },
		{ Commands::kick,                 1, "<player> [room]",                    "Kick a player from the room" },
		{ Commands::ban,                  1, "<player> [room]",                    "Ban a player from the room" },
		{ Commands::unban,                1, "<player> [room]",                    "Unban a player from the room" },
		{ "",                             0, "",                                   ""} // this must be last
	};

	uint32 s_currentRoomId = 0;
}

//----------------------------------------------------------------------


SwgCuiCommandParserChatRoom::SwgCuiCommandParserChatRoom () :
CommandParser   ("chatRoom", 0, "...", "Chat Room commands", 0)
{
	createDelegateCommands (cmds);
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserChatRoom::performParsing  (const NetworkId & , const StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node)
{
	UNREF (originalCommand);

	const static Unicode::String tabStop  (Unicode::narrowToWide ("\\@01    "));
	const static Unicode::String tabStop2 (Unicode::narrowToWide ("\\@02    "));

	//----------------------------------------------------------------------
	
	if (isCommand (argv[0], Commands::join))
	{
		CuiChatRoomManager::enterRoom (Unicode::wideToNarrow (argv [1]), result);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::list))
	{
		CuiChatRoomManager::ChatRoomsEnteredVector rv;
		CuiChatRoomManager::getEnteredRooms (rv);
		for (CuiChatRoomManager::ChatRoomsEnteredVector::const_iterator it = rv.begin (); it != rv.end (); ++it)
		{
			const uint32 id = *it;
			const CuiChatRoomDataNode * const room = CuiChatRoomManager::findRoomNode (id);
			
			Unicode::String tmp;
			UIUtils::FormatInteger (tmp, id);
			
			Unicode::appendStringField (result, tmp, 5, Unicode::FA_RIGHT);

			result.append (tabStop);

			if (room)
			{
				Unicode::appendStringField (result, room->data.path, 20, Unicode::FA_LEFT);
				result += room->data.title;
			}
			else
				result += Unicode::narrowToWide ("ERROR!");

			result.append (1, '\n');

		}
		return true;
	}	

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::listAll))
	{
		const CuiChatRoomDataNode & tree = CuiChatRoomManager::getTree ();

		std::vector <const CuiChatRoomDataNode *> rooms;
		rooms.push_back (&tree);
		
		while (!rooms.empty ())
		{
			const CuiChatRoomDataNode * const room = rooms.back ();
			rooms.pop_back ();
			
			const uint32 id = room->data.id;
			
			if (id)
			{
				Unicode::String tmp;
				UIUtils::FormatInteger (tmp, id);
				
				Unicode::appendStringField (result, tmp, 5, Unicode::FA_RIGHT);
				
				result.append (tabStop);
				
				if (node)
				{
					Unicode::appendStringField (result, room->data.path, 20, Unicode::FA_LEFT);
					result.append (4, ' ');
					result += room->data.title;
				}
				else
					result += Unicode::narrowToWide ("ERROR!");
				
				result.append (1, '\n');
			}
			
			const CuiChatRoomDataNode::NodeVector & roomChildren = room->getChildren ();

			for (CuiChatRoomDataNode::NodeVector::const_iterator it = roomChildren.begin (); it != roomChildren.end (); ++it)
				rooms.push_back (*it);
		}

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::refresh))
	{
		CuiChatRoomManager::requestRoomsRefresh ();
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::leave))
	{
		if (argv.size () > 1)
			CuiChatRoomManager::leaveRoom (Unicode::wideToNarrow (argv [1]), result);
		else if (s_currentRoomId)
			CuiChatRoomManager::leaveRoom (s_currentRoomId, result);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
		}

		return true;
	}	

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::create))
	{
		CuiChatRoomManager::createRoom (Unicode::wideToNarrow (argv [1]), argv [2][0] == '1', argv [3][0] == '1', argv[4]);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::destroy))
	{
		CuiChatRoomManager::destroyRoom (Unicode::wideToNarrow (argv [1]), result);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::send))
	{
		Unicode::String cmd;

		reconstructString (argv, 1, argv.size (), true, cmd);

		if (CuiChatRoomManager::parse (cmd, result, true) == CommandParser::ERR_NONE)
		{
			result += Unicode::narrowToWide ("Bad parsing");
		}

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::query))
	{
		if (argv.size () > 1)
			CuiChatRoomManager::queryRoom (Unicode::wideToNarrow (argv [1]).c_str (), result);
		else if (s_currentRoomId)
			CuiChatRoomManager::queryRoom (s_currentRoomId, result);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
		}

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::who))
	{
		const CuiChatRoomDataNode * roomNode = 0;

		if (argv.size () > 1)
			roomNode = CuiChatRoomManager::findRoomNode (Unicode::wideToNarrow (argv [1]));
		else if (s_currentRoomId)
			roomNode = CuiChatRoomManager::findRoomNode (s_currentRoomId);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}

		if (roomNode)
		{
			result += CuiStringIdsChatRoom::who_header.localize ();
			result.append (1, '\n');

			const CuiChatRoomDataNode::AvatarVector & roomMembers = roomNode->getMembers ();
			for (CuiChatRoomDataNode::AvatarVector::const_iterator it = roomMembers.begin (); it != roomMembers.end (); ++it)
			{
				const CuiChatAvatarId & cca_id = *it;
				result.append (tabStop);
				result += Unicode::narrowToWide (cca_id.chatId.getFullName ());
				result.append (1, '\n');
			}
		}
		else
		{
			result += CuiStringIdsChatRoom::err_not_found.localize ();
		}

		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv[0], Commands::invite))
	{
		const std::string & playerName = Unicode::wideToNarrow (argv [1]);
		if (argv.size () > 2)
		{
			const std::string & roomName = Unicode::wideToNarrow (argv [2]);
			CuiChatRoomManager::setInvited (roomName, playerName, true, result);
		}
		else if (s_currentRoomId)
			CuiChatRoomManager::setInvited (s_currentRoomId, playerName, true, result);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv[0], Commands::inviteGroup))
	{
		const std::string & playerName = Unicode::wideToNarrow (argv [1]);
		if (argv.size () > 2)
		{
			const std::string & roomName = Unicode::wideToNarrow (argv [2]);
			CuiChatRoomManager::inviteGroup (roomName, playerName, result);
		}
		else if (s_currentRoomId)
		{
			CuiChatRoomManager::inviteGroup (s_currentRoomId, playerName, result);
		}
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv[0], Commands::uninvite))
	{
		const std::string & playerName = Unicode::wideToNarrow (argv [1]);
		if (argv.size () > 2)
		{
			const std::string & roomName = Unicode::wideToNarrow (argv [2]);
			CuiChatRoomManager::setInvited (roomName, playerName, false, result);
		}
		else if (s_currentRoomId)
			CuiChatRoomManager::setInvited (s_currentRoomId, playerName, false, result);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv[0], Commands::ban))
	{
		const std::string & playerName = Unicode::wideToNarrow (argv [1]);
		if (argv.size () > 2)
		{
			const std::string & roomName = Unicode::wideToNarrow (argv [2]);
			CuiChatRoomManager::setBanned (roomName, playerName, true, result);
		}
		else if (s_currentRoomId)
			CuiChatRoomManager::setBanned (s_currentRoomId, playerName, true, result);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv[0], Commands::unban))
	{
		const std::string & playerName = Unicode::wideToNarrow (argv [1]);
		if (argv.size () > 2)
		{
			const std::string & roomName = Unicode::wideToNarrow (argv [2]);
			CuiChatRoomManager::setBanned (roomName, playerName, false, result);
		}
		else if (s_currentRoomId)
			CuiChatRoomManager::setBanned (s_currentRoomId, playerName, false, result);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}
		return true;
	}


	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::invites))
	{
		const CuiChatRoomDataNode * roomNode = 0;

		if (argv.size () > 1)
			roomNode = CuiChatRoomManager::findRoomNode (Unicode::wideToNarrow (argv [1]));
		else if (s_currentRoomId)
			roomNode = CuiChatRoomManager::findRoomNode (s_currentRoomId);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}

		if (roomNode)
		{
			result += CuiStringIdsChatRoom::invitees_header.localize ();
			result.append (1, '\n');

			const CuiChatRoomDataNode::AvatarVector & roomInvitees = roomNode->getInvitees();
			for (CuiChatRoomDataNode::AvatarVector::const_iterator it = roomInvitees.begin (); it != roomInvitees.end (); ++it)
			{
				const CuiChatAvatarId & cca_id = *it;
				result.append (tabStop);
				result += Unicode::narrowToWide (cca_id.chatId.getFullName ());
				result.append (1, '\n');
			}
		}
		else
		{
			result += CuiStringIdsChatRoom::err_not_found.localize ();
		}
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::info))
	{
		const CuiChatRoomDataNode * roomNode = 0;

		static const Unicode::String color_contrast = Unicode::narrowToWide ("\\#pcontrast1 ");
		static const Unicode::String color_reset    = Unicode::narrowToWide ("\\#.");

		if (argv.size () > 1)
			roomNode = CuiChatRoomManager::findRoomNode (Unicode::wideToNarrow (argv [1]));
		else if (s_currentRoomId)
			roomNode = CuiChatRoomManager::findRoomNode (s_currentRoomId);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}

		if (roomNode)
		{
			Unicode::String tmpStr;

			result += color_contrast;
			result += CuiStringIdsChatRoom::info_header.localize ();
			result.append (1, '\n');

			result += CuiStringIdsChatRoom::info_owner.localize ();
			result += color_reset;
			result.append (tabStop2);
			result += Unicode::narrowToWide (roomNode->data.owner.getFullName ());
			result.append (1, '\n');

			result += color_contrast;
			result += CuiStringIdsChatRoom::info_creator.localize ();
			result += color_reset;
			result.append (tabStop2);
			result += Unicode::narrowToWide (roomNode->data.creator.getFullName ());
			result.append (1, '\n');

			result += color_contrast;
			result += CuiStringIdsChatRoom::info_private_prefix.localize ();
			result += color_reset;
			result.append (tabStop2);
			UIUtils::FormatBoolean (tmpStr, roomNode->data.roomType == CHAT_ROOM_PRIVATE);
			result += tmpStr;
			result.append (1, '\n');

			result += color_contrast;
			result += CuiStringIdsChatRoom::info_moderated_prefix.localize ();
			result += color_reset;
			result.append (tabStop2);
			UIUtils::FormatBoolean (tmpStr, roomNode->data.moderated != 0);
			result += tmpStr;
			result.append (1, '\n');


			const CuiChatRoomDataNode::AvatarVector & roomModerators = roomNode->getModerators ();
			const CuiChatRoomDataNode::AvatarVector & roomMembers    = roomNode->getMembers    ();
			const CuiChatRoomDataNode::AvatarVector & roomInvitees   = roomNode->getInvitees   ();

			if (!roomModerators.empty ())
			{
				result += color_contrast;
				result += CuiStringIdsChatRoom::info_moderators.localize ();
				result += color_reset;
				result.append (1, '\n');

				for (CuiChatRoomDataNode::AvatarVector::const_iterator it = roomModerators.begin (); it != roomModerators.end (); ++it)
				{
					const CuiChatAvatarId & avatar = *it;
					result.append (tabStop);
					result += Unicode::narrowToWide (avatar.chatId.getFullName ());
					result.append (1, '\n');
				}
			}

			if (!roomMembers.empty ())
			{
				result += color_contrast;
				result += CuiStringIdsChatRoom::info_members.localize ();
				result += color_reset;
				result.append (1, '\n');

				for (CuiChatRoomDataNode::AvatarVector::const_iterator it = roomMembers.begin (); it != roomMembers.end (); ++it)
				{
					const CuiChatAvatarId & avatar = *it;
					result.append (tabStop);
					result += Unicode::narrowToWide (avatar.chatId.getFullName ());
					result.append (1, '\n');
				}
			}

			if (!roomInvitees.empty ())
			{
				result += color_contrast;
				result += CuiStringIdsChatRoom::info_invitees.localize ();
				result += color_reset;
				result.append (1, '\n');

				for (CuiChatRoomDataNode::AvatarVector::const_iterator it = roomInvitees.begin (); it != roomInvitees.end (); ++it)
				{
					const CuiChatAvatarId & avatar = *it;
					result.append (tabStop);
					result += Unicode::narrowToWide (avatar.chatId.getFullName ());
					result.append (1, '\n');
				}
			}
		}
		else
		{
			result += CuiStringIdsChatRoom::err_not_found.localize ();
		}

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::op))
	{
		const std::string & playerName = Unicode::wideToNarrow (argv [1]);
		if (argv.size () > 2)
		{
			const std::string & roomName = Unicode::wideToNarrow (argv [2]);
			CuiChatRoomManager::setModerator (roomName, playerName, true, result);
		}
		else if (s_currentRoomId)
			CuiChatRoomManager::setModerator (s_currentRoomId, playerName, true, result);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::deop))
	{
		const std::string & playerName = Unicode::wideToNarrow (argv [1]);
		if (argv.size () > 2)
		{
			const std::string & roomName = Unicode::wideToNarrow (argv [2]);
			CuiChatRoomManager::setModerator (roomName, playerName, false, result);
		}
		else if (s_currentRoomId)
			CuiChatRoomManager::setModerator (s_currentRoomId, playerName, false, result);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::ops))
	{
		const CuiChatRoomDataNode * roomNode = 0;

		if (argv.size () > 1)
			roomNode = CuiChatRoomManager::findRoomNode (Unicode::wideToNarrow (argv [1]));
		else if (s_currentRoomId)
			roomNode = CuiChatRoomManager::findRoomNode (s_currentRoomId);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}

		if (roomNode)
		{
			result += CuiStringIdsChatRoom::ops_header.localize ();
			result.append (1, '\n');

			const CuiChatRoomDataNode::AvatarVector & roomModerators = roomNode->getModerators ();
			for (CuiChatRoomDataNode::AvatarVector::const_iterator it = roomModerators.begin (); it != roomModerators.end (); ++it)
			{
				const CuiChatAvatarId & cca_id = *it;
				result.append (tabStop);
				result += Unicode::narrowToWide (cca_id.chatId.getFullName ());
				result.append (1, '\n');
			}
		}
		else
		{
			result += CuiStringIdsChatRoom::err_not_found.localize ();
		}

		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand (argv[0], Commands::kick))
	{
		const std::string & playerName = Unicode::wideToNarrow (argv [1]);
		if (argv.size () > 2)
		{
			const std::string & roomName = Unicode::wideToNarrow (argv [2]);
			CuiChatRoomManager::kick (roomName, playerName, result);
		}
		else if (s_currentRoomId)
			CuiChatRoomManager::kick (s_currentRoomId, playerName, result);
		else
		{
			result += node->constructFullPath () + Unicode::narrowToWide (" : ") + CuiStringIdsChatRoom::err_parser_no_room_specified.localize ();
			result.append (1, '\n');
			return true;
		}
		return true;
	}

	//----------------------------------------------------------------------

	return false;
}

//----------------------------------------------------------------------

void SwgCuiCommandParserChatRoom::setCurrentChatRoom (uint32 id)
{
	s_currentRoomId = id;
}

//======================================================================
