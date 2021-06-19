//======================================================================
//
// CuiChatRoomManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatRoomManager.h"

#include "UnicodeUtils.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatRoomDataNode.h"
#include "clientUserInterface/CuiChatRoomManagerHistory.h"
#include "clientUserInterface/CuiChatRoomManagerListener.h"
#include "clientUserInterface/CuiChatRoomManagerStatusMessage.h"
#include "clientUserInterface/CuiChatRoomMessage.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiPrelocalizedChatMessage.h"
#include "clientUserInterface/CuiStringIdsChatRoom.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedCommandParser/CommandParser.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ChatAddFriend.h"
#include "sharedNetworkMessages/ChatAddModeratorToRoom.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatBanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatCreateRoom.h"
#include "sharedNetworkMessages/ChatDestroyRoom.h"
#include "sharedNetworkMessages/ChatEnterRoom.h"
#include "sharedNetworkMessages/ChatEnterRoomById.h"
#include "sharedNetworkMessages/ChatEnum.h"
#include "sharedNetworkMessages/ChatInviteAvatarToRoom.h"
#include "sharedNetworkMessages/ChatInviteGroupToRoom.h"
#include "sharedNetworkMessages/ChatKickAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnAddModeratorToRoom.h"
#include "sharedNetworkMessages/ChatOnBanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnEnteredRoom.h"
#include "sharedNetworkMessages/ChatOnInviteGroupToRoom.h"
#include "sharedNetworkMessages/ChatOnInviteToRoom.h"
#include "sharedNetworkMessages/ChatOnKickAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnLeaveRoom.h"
#include "sharedNetworkMessages/ChatOnReceiveRoomInvitation.h"
#include "sharedNetworkMessages/ChatOnRemoveModeratorFromRoom.h"
#include "sharedNetworkMessages/ChatOnUnbanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatQueryRoom.h"
#include "sharedNetworkMessages/ChatQueryRoomResults.h"
#include "sharedNetworkMessages/ChatRemoveAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatRemoveModeratorFromRoom.h"
#include "sharedNetworkMessages/ChatRequestRoomList.h"
#include "sharedNetworkMessages/ChatRoomData.h"
#include "sharedNetworkMessages/ChatRoomDataArchive.h"
#include "sharedNetworkMessages/ChatRoomList.h"
#include "sharedNetworkMessages/ChatRoomMessage.h"
#include "sharedNetworkMessages/ChatSendToRoom.h"
#include "sharedNetworkMessages/ChatUnbanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatUninviteFromRoom.h"

//======================================================================

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::OtherEntered::Payload &,         CuiChatRoomManager::Messages::OtherEntered > 
			otherEntered;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::OtherLeft::Payload &,            CuiChatRoomManager::Messages::OtherLeft > 
			otherLeft;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::SelfEntered::Payload &,          CuiChatRoomManager::Messages::SelfEntered > 
			selfEntered;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::JoinFailed::Payload &,           CuiChatRoomManager::Messages::JoinFailed > 
			joinFailed;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::DestroyFailed::Payload &,        CuiChatRoomManager::Messages::DestroyFailed > 
			destroyFailed;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::SelfLeft::Payload &,             CuiChatRoomManager::Messages::SelfLeft > 
			selfLeft;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::Destroyed::Payload &,            CuiChatRoomManager::Messages::Destroyed > 
			destroyed;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::Created::Payload &,              CuiChatRoomManager::Messages::Created > 
			created;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::CreateFailed::Payload &,         CuiChatRoomManager::Messages::CreateFailed > 
			createFailed;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::Modified::Payload &,             CuiChatRoomManager::Messages::Modified > 
			modified;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::MessageReceived::Payload &,      CuiChatRoomManager::Messages::MessageReceived > 
			messageReceived;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::RoomMembersChanged::Payload &,   CuiChatRoomManager::Messages::RoomMembersChanged > 
			roomMembersChanged;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::StatusMessage::Payload &,        CuiChatRoomManager::Messages::StatusMessage> 
			statusMessage;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::ModeratorAdded::Payload &,       CuiChatRoomManager::Messages::ModeratorAdded> 
			moderatorAdded;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::ModeratorRemoved::Payload &,     CuiChatRoomManager::Messages::ModeratorRemoved> 
			moderatorRemoved;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::PlanetRoomIdChanged::Payload &,  CuiChatRoomManager::Messages::PlanetRoomIdChanged> 
			planetRoomIdChanged;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::GroupRoomIdChanged::Payload &,   CuiChatRoomManager::Messages::GroupRoomIdChanged> 
			groupRoomIdChanged;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::GuildRoomIdChanged::Payload &,   CuiChatRoomManager::Messages::GuildRoomIdChanged> 
			guildRoomIdChanged;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::CityRoomIdChanged::Payload &,    CuiChatRoomManager::Messages::CityRoomIdChanged> 
			cityRoomIdChanged;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::NamedRoomIdChanged::Payload &,   CuiChatRoomManager::Messages::NamedRoomIdChanged> 
			namedRoomIdChanged;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::InviteeAdded::Payload &,         CuiChatRoomManager::Messages::InviteeAdded> 
			inviteeAdded;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::InviteeRemoved::Payload &,       CuiChatRoomManager::Messages::InviteeRemoved> 
			inviteeRemoved;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::Banned::Payload &,               CuiChatRoomManager::Messages::Banned> 
			banned;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::Unbanned::Payload &,             CuiChatRoomManager::Messages::Unbanned> 
			unbanned;
		MessageDispatch::Transceiver<const CuiChatRoomManager::Messages::Prelocalized::Payload &,         CuiChatRoomManager::Messages::Prelocalized> 
			prelocalized;
	};

	//----------------------------------------------------------------------

	CuiChatRoomDataNode chatRoomTree;

	//----------------------------------------------------------------------

	typedef std::vector<uint32> IntVector;
	typedef IntVector ChatRoomsEnteredVector;
	typedef stdmap<std::string, uint32>::fwd StringIntMap;
	ChatRoomsEnteredVector chatRoomsEntered;
	StringIntMap           chatRoomsEnteredNames;

	//----------------------------------------------------------------------

	uint32     s_planetRoomId = 0;
	uint32     s_groupRoomId  = 0;
	uint32     s_guildRoomId  = 0;
	uint32     s_cityRoomId   = 0;
	bool       s_installed    = false;
	bool s_chatRequestRoomListPending = false;
	std::set<uint32>  s_namedRoomIds;
	std::map<std::string, int> s_pendingEnters;
}

//----------------------------------------------------------------------

const Unicode::String CuiChatRoomManager::ms_cmdGroup = Unicode::narrowToWide (ChatRoomTypes::ROOM_GROUP);

//----------------------------------------------------------------------

void CuiChatRoomManager::install ()				
{
	DEBUG_FATAL (s_installed, ("already installed"));

	CuiChatRoomManagerListener::install ();
	s_installed = true;
}

//----------------------------------------------------------------------

void CuiChatRoomManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	CuiChatRoomManagerListener::remove ();
	s_installed = false;
}

//----------------------------------------------------------------------

void CuiChatRoomManager::reset ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	chatRoomsEntered.clear ();
	chatRoomsEnteredNames.clear ();

	chatRoomTree.purge ();
}

//----------------------------------------------------------------------

static uint32 enter_room_sequence = 0;

bool CuiChatRoomManager::enterRoom (uint32 id, const CuiChatRoomDataNode * room, Unicode::String & result)
{
	if (!room)
		room = findRoomNode (id);
	
	if (room)
	{
		ChatEnterRoomById msg (++enter_room_sequence, id, room->data.path);
		GameNetwork::send (msg, true);
		CuiChatRoomManagerHistory::roomJoinAdd (enter_room_sequence, room->data.path, id);
		return true;
	}
	else
	{
		result += CuiStringIdsChatRoom::not_found.localize ();
		return false;
	}
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::enterRoom (const std::string & name, Unicode::String & result)
{
	if (name.empty ())
		return false;

	const CuiChatRoomDataNode * const room = findRoomNode (name);
	if (room && room->data.id)
		return enterRoom (room->data.id, room, result);
	else if (isdigit (name [0]))
		return enterRoom (atoi (name.c_str ()), 0, result);
	else
	{
		// queue an enter room request
		createRoom(name, false, true, Unicode::emptyString);
		s_pendingEnters[name] = 0;
		return false;
	}
}

//----------------------------------------------------------------------

const CuiChatRoomDataNode & CuiChatRoomManager::getTree ()
{
	return chatRoomTree;
}

//----------------------------------------------------------------------

void CuiChatRoomManager::requestRoomsRefresh ()
{
	if (!s_chatRequestRoomListPending)
	{
		s_chatRequestRoomListPending = true;
		ChatRequestRoomList msg;
		GameNetwork::send (msg, true);
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::getChatRoomSearchPathsLower   (StringVector & sv, const std::string & path)
{
	sv.clear ();
	sv.reserve (6);

	const ChatAvatarId & selfId = CuiChatManager::getSelfAvatarId ();
	const std::string & planetName = Game::getSceneId ();
	
	static const std::string dot (1, '.');

	const std::string & lowerPath = Unicode::toLower (path);

	static const std::string gamePrefix    = Unicode::toLower (selfId.gameCode + dot);
	const std::string clusterPrefix = gamePrefix + Unicode::toLower (selfId.cluster) + dot;
	const std::string planetPrefix  = clusterPrefix + Unicode::toLower (planetName) + dot;

	static const std::string chatPrefixSubstring = Unicode::toLower (ChatRoomTypes::ROOM_CHAT) + dot;

	sv.push_back (lowerPath);
	sv.push_back (planetPrefix  + lowerPath);
	sv.push_back (planetPrefix  + chatPrefixSubstring + lowerPath);
	sv.push_back (clusterPrefix + lowerPath);
	sv.push_back (clusterPrefix + chatPrefixSubstring + lowerPath);
	sv.push_back (gamePrefix    + lowerPath);
	sv.push_back (gamePrefix    + chatPrefixSubstring + lowerPath);
}


//----------------------------------------------------------------------

const CuiChatRoomDataNode * CuiChatRoomManager::findRoomNode            (const std::string & path)
{
	return findRoomNodeInternal (path);
}

//----------------------------------------------------------------------

const CuiChatRoomDataNode * CuiChatRoomManager::findRoomNode (uint32 id)
{
	return findRoomNodeInternal (id);
}

//----------------------------------------------------------------------

CuiChatRoomDataNode * CuiChatRoomManager::findRoomNodeInternal (uint32 id)
{
	if (id == 0)
		return 0;

	std::vector <CuiChatRoomDataNode *> rooms;
	rooms.push_back (&chatRoomTree);

	while (!rooms.empty ())
	{
		CuiChatRoomDataNode * const room = rooms.back ();
		rooms.pop_back ();

		if (room->data.id == id)
			return room;

		const CuiChatRoomDataNode::NodeVector & roomChildren = room->getChildren ();

		for (CuiChatRoomDataNode::NodeVector::const_iterator it = roomChildren.begin (); it != roomChildren.end (); ++it)
			rooms.push_back (*it);
	}

	return 0;
}

//----------------------------------------------------------------------

CuiChatRoomDataNode * CuiChatRoomManager::findRoomNodeInternal            (const std::string & path)
{
	StringVector sv;	
	getChatRoomSearchPathsLower (sv, path);
	
	for (StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
	{
		const std::string & pathToTest = *it;
		CuiChatRoomDataNode * const roomNode = chatRoomTree.getChild (pathToTest);
		if (roomNode)
			return roomNode;
	}
	
	return 0;
}

//----------------------------------------------------------------------

int CuiChatRoomManager::parse (const Unicode::String & str, Unicode::String & result, bool anyRoom)
{
	Unicode::String token;
	size_t endpos = 0;
	
	if (!Unicode::getFirstToken  (str, 0, endpos, token))
		return CommandParser::ERR_NONE;

	const std::string & path = Unicode::wideToNarrow (token);
	const CuiChatRoomDataNode * node = findEnteredRoomNode (path);
	
	if (!node && anyRoom)
		node = findRoomNode (path);

	if (node)
	{
		if (endpos == str.npos)
		{
			result += CuiStringIdsChatRoom::send_err_no_message.localize ();
			return CommandParser::ERR_FAIL;
		}
		
		const Unicode::String message (str.substr (endpos + 1));
		
		if (message.empty ())
		{
			result += CuiStringIdsChatRoom::send_err_no_message.localize ();
			return CommandParser::ERR_FAIL;
		}
		
		sendToRoom (node->data.id, message, Unicode::String());
		return CommandParser::ERR_SUCCESS;
	}

	return CommandParser::ERR_NONE;
}

//----------------------------------------------------------------------

void CuiChatRoomManager::sendToRoom (uint32 id, const Unicode::String & message, const Unicode::String & outOfBand)
{
	static uint32 sequence = 0;

	const CuiChatRoomDataNode * const room = findRoomNode (id);

	if (!room)
		WARNING (true, ("null room in sendToRoom"));
	else if (!Game::isPlayerSquelched())
	{
		CuiChatRoomManagerHistory::roomSendAdd (++sequence, room->data.path);
		const ChatSendToRoom msg (sequence, id, message, outOfBand);
		GameNetwork::send (msg, true);
	}
}

//----------------------------------------------------------------------
	
void CuiChatRoomManager::getEnteredRooms (ChatRoomsEnteredVector & rv)
{
	rv = chatRoomsEntered;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::isRoomEntered           (uint32 id)
{
	return std::find (chatRoomsEntered.begin (), chatRoomsEntered.end (), id) != chatRoomsEntered.end ();
}

//----------------------------------------------------------------------

void CuiChatRoomManager::createRoom (const std::string & name, bool isModerated, bool isPublic, const Unicode::String & title)
{
	static uint32 sequence = 0;
	const ChatCreateRoom msg (++sequence, std::string (), name, isModerated, isPublic, Unicode::wideToNarrow(title));
	GameNetwork::send (msg, true);

	CuiChatRoomManagerHistory::roomCreateAdd (sequence, name);
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::destroyRoom (const std::string & name, Unicode::String & result)
{
	const CuiChatRoomDataNode * const room = findRoomNode (name);
	if (room)
	{
		if (!room->data.id)
		{
			result += CuiStringIdsChatRoom::not_a_room.localize ();
			return false;
		}

		return destroyRoom (room->data.id, result);
	}

	result += CuiStringIdsChatRoom::not_found.localize ();
	return false;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::destroyRoom (uint32 id, Unicode::String & result)
{
	if (!id)
	{
		result += CuiStringIdsChatRoom::not_a_room.localize ();
		return false;
	}

	const CuiChatRoomDataNode * const room = findRoomNodeInternal     (id);
	
	if (!room)
	{
		result += CuiStringIdsChatRoom::not_found.localize ();
		return false;
	}
		
	static uint32 sequence = 0;
	const ChatDestroyRoom msg (++sequence, id);
	GameNetwork::send (msg, true);

	CuiChatRoomManagerHistory::roomDestroyAdd (sequence, room->data.path);

	return true;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::leaveRoom   (const uint32 id, Unicode::String & result)
{
	if (!id)
	{
		result += CuiStringIdsChatRoom::not_a_room.localize ();
		return false;
	}

	if (std::find (chatRoomsEntered.begin (),chatRoomsEntered.end (), id) == chatRoomsEntered.end ())
	{
		result += CuiStringIdsChatRoom::err_entered_room_not_found.localize ();
		return false;
	}
	
	const CuiChatRoomDataNode * const roomNode = findRoomNode (id);
	if (roomNode)
	{
		const ChatAvatarId & self = CuiChatManager::getSelfAvatarId ();
		CuiChatAvatarId cuiAvatar = CuiChatAvatarId (self);

		if (roomNode->findAvatarInRoom (self.getFullName (), cuiAvatar, CuiChatRoomDataNode::ALT_members))
		{
			const ChatRemoveAvatarFromRoom msg (cuiAvatar.chatId, roomNode->data.path);
			GameNetwork::send (msg, true);
			return true;
		}

		result += CuiStringIdsChatRoom::err_no_such_member.localize ();
		return false;
	}

	result += CuiStringIdsChatRoom::not_found.localize ();
	return false;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::leaveRoom   (const std::string & name, Unicode::String & result)
{
	const CuiChatRoomDataNode * const node = findEnteredRoomNode (name);
	if (node)
	{
		leaveRoom (node->data.id, result);
		return true;
	}

	result += CuiStringIdsChatRoom::err_entered_room_not_found.localize ();
	return false;
}

//----------------------------------------------------------------------

const CuiChatRoomDataNode * CuiChatRoomManager::findEnteredRoomNode (const std::string & path)
{
	StringVector sv;	
	getChatRoomSearchPathsLower (sv, path);
	
	for (StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
	{
		const std::string & pathToTest = *it;
		const StringIntMap::const_iterator chit = chatRoomsEnteredNames.find (pathToTest);
		if (chit != chatRoomsEnteredNames.end ())
		{
			return findRoomNode ((*chit).second);
		}
	}

	return 0;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::queryRoom (uint32 id, Unicode::String & result)
{
	static uint32 sequence = 0;

	const CuiChatRoomDataNode * const room = findRoomNode (id);
	if (room)
	{
		const ChatQueryRoom msg (++sequence, room->data.path);
		GameNetwork::send (msg, true);
		return true;
	}

	result += CuiStringIdsChatRoom::not_found.localize ();
	return false;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::queryRoom (const std::string & path, Unicode::String & result)
{
	static uint32 sequence = 0;

	const CuiChatRoomDataNode * const room = findRoomNode (path);
	if (room)
	{
		if (!room->data.id)
		{
			result += CuiStringIdsChatRoom::not_a_room.localize ();
			return false;
		}
		queryRoom (room->data.id, result);
		return true;
	}

	result += CuiStringIdsChatRoom::not_found.localize ();
	return false;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::setInvited    (uint32 id, const std::string & avatarName, bool invited, Unicode::String & result)
{
	if (!id)
	{
		result += CuiStringIdsChatRoom::not_a_room.localize ();
		return false;
	}

	const CuiChatRoomDataNode * const roomNode = findRoomNode (id);
	if (!roomNode)
	{
		result += CuiStringIdsChatRoom::not_found.localize ();
		return false;
	}
	
	ChatAvatarId avatar;
	if (!CuiChatManager::constructChatAvatarId (avatarName, avatar))
	{
		result += CuiStringIdsChatRoom::err_bad_avatar_name.localize ();
		return false;
	}
	
	if (invited)
	{
		const ChatInviteAvatarToRoom msg (avatar, roomNode->data.path);
		GameNetwork::send (msg, true);
	}
	else
	{
		static uint32 sequence_uninvite = 0;
		CuiChatRoomManagerHistory::uninviteAdd (++sequence_uninvite, id, avatarName);	

		// -- 
		if (isRoomEntered (roomNode->data.id))
		{
			//-- construct a possibly invalid avatar id
			CuiChatAvatarId cuiAvatar = CuiChatAvatarId (ChatAvatarId (avatarName));

			if (!roomNode->findAvatarInRoom (avatarName, cuiAvatar, CuiChatRoomDataNode::ALT_invitees))
			{
				result += CuiStringIdsChatRoom::err_no_such_invitee.localize ();
				return false;
			}

			avatar = cuiAvatar.chatId;
		}

		const ChatUninviteFromRoom msg (sequence_uninvite, avatar, roomNode->data.path);
		GameNetwork::send (msg, true);
	}
	
	return true;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::setInvited    (const std::string & roomName, const std::string & avatarName, bool invited, Unicode::String & result)
{
	const CuiChatRoomDataNode * const room = findRoomNode (roomName);
	if (!room)
	{
		result += CuiStringIdsChatRoom::not_found.localize ();
		return false;
	}
	
	if (!room->data.id)
	{
		result += CuiStringIdsChatRoom::not_a_room.localize ();
		return false;
	}

	return setInvited (room->data.id, avatarName, invited, result);
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::inviteGroup (uint32 id, const std::string & avatarName, Unicode::String & result)
{
	if (!id)
	{
		result += CuiStringIdsChatRoom::not_a_room.localize ();
		return false;
	}

	const CuiChatRoomDataNode * const roomNode = findRoomNode (id);
	if (!roomNode)
	{
		result += CuiStringIdsChatRoom::not_found.localize ();
		return false;
	}
	
	ChatAvatarId avatar;
	if (!CuiChatManager::constructChatAvatarId (avatarName, avatar))
	{
		result += CuiStringIdsChatRoom::err_bad_avatar_name.localize ();
		return false;
	}
	
	const ChatInviteGroupToRoom msg (avatar, roomNode->data.path);
	GameNetwork::send (msg, true);
	
	return true;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::inviteGroup (const std::string & roomName, const std::string & avatarName, Unicode::String & result)
{
	const CuiChatRoomDataNode * const room = findRoomNode (roomName);
	if (!room)
	{
		result += CuiStringIdsChatRoom::not_found.localize ();
		return false;
	}
	
	if (!room->data.id)
	{
		result += CuiStringIdsChatRoom::not_a_room.localize ();
		return false;
	}

	return inviteGroup (room->data.id, avatarName, result);
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::setBanned    (uint32 id, const std::string & avatarName, bool invited, Unicode::String & result)
{
	if (!id)
	{
		result += CuiStringIdsChatRoom::not_a_room.localize ();
		return false;
	}

	const CuiChatRoomDataNode * const roomNode = findRoomNode (id);
	if (!roomNode)
	{
		result += CuiStringIdsChatRoom::not_found.localize ();
		return false;
	}
	
	ChatAvatarId avatar;
	if (!CuiChatManager::constructChatAvatarId (avatarName, avatar))
	{
		result += CuiStringIdsChatRoom::err_bad_avatar_name.localize ();
		return false;
	}
	
	if (invited)
	{
		static uint32 sequence_ban = 0;
		++sequence_ban;
		const ChatBanAvatarFromRoom msg (sequence_ban, avatar, roomNode->data.path);
		GameNetwork::send (msg, true);
	}
	else
	{
		// -- 
		if (isRoomEntered (roomNode->data.id))
		{
			//-- construct a possibly invalid avatar id
			CuiChatAvatarId cuiAvatar = CuiChatAvatarId (ChatAvatarId (avatarName));

			/*if (!roomNode->findAvatarInRoom (avatarName, cuiAvatar, CuiChatRoomDataNode::ALT_invitees))
			{
				result += CuiStringIdsChatRoom::err_no_such_invitee.localize ();
				return false;
			}*/

			avatar = cuiAvatar.chatId;
		}

		static uint32 sequence_unban = 0;
		++sequence_unban;
		const ChatUnbanAvatarFromRoom msg (sequence_unban, avatar, roomNode->data.path);
		GameNetwork::send (msg, true);
	}
	
	return true;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::setBanned    (const std::string & roomName, const std::string & avatarName, bool invited, Unicode::String & result)
{
	const CuiChatRoomDataNode * const room = findRoomNode (roomName);
	if (!room)
	{
		result += CuiStringIdsChatRoom::not_found.localize ();
		return false;
	}
	
	if (!room->data.id)
	{
		result += CuiStringIdsChatRoom::not_a_room.localize ();
		return false;
	}

	return setBanned (room->data.id, avatarName, invited, result);
}


//----------------------------------------------------------------------

bool CuiChatRoomManager::setModerator  (uint32 id, const std::string & avatarName, bool moderator, Unicode::String & result)
{
	if (!id)
	{
		result += CuiStringIdsChatRoom::not_a_room.localize ();
		return false;
	}

	const CuiChatRoomDataNode * const roomNode = findRoomNode (id);
	if (!roomNode)
	{
		result += CuiStringIdsChatRoom::not_found.localize ();
		return false;
	}
	
	ChatAvatarId avatar;
	if (!CuiChatManager::constructChatAvatarId (avatarName, avatar))
	{
		result += CuiStringIdsChatRoom::err_bad_avatar_name.localize ();
		return false;
	}
	
	if (moderator)
	{
		static uint32 sequence_addModerator = 0;
		CuiChatRoomManagerHistory::addModeratorAdd (++sequence_addModerator, id, avatarName);
		const ChatAddModeratorToRoom msg (sequence_addModerator, avatar, roomNode->data.path);
		GameNetwork::send (msg, true);
	}
	else
	{
		//-- construct a possibly invalid avatar id
		CuiChatAvatarId cuiAvatar = CuiChatAvatarId (ChatAvatarId (avatarName));
		
		// -- 
		if (isRoomEntered (roomNode->data.id))
		{
			if (!roomNode->findAvatarInRoom (avatarName, cuiAvatar, CuiChatRoomDataNode::ALT_moderators))
			{
				result += CuiStringIdsChatRoom::err_no_such_moderator.localize ();
				return false;
			}
		}
		
		static uint32 sequence_removeModerator = 0;
		CuiChatRoomManagerHistory::removeModeratorAdd (++sequence_removeModerator, id, avatarName);

		const ChatRemoveModeratorFromRoom msg (sequence_removeModerator, avatar, roomNode->data.path);
		GameNetwork::send (msg, true);

		return true;
	}
	
	return true;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::setModerator  (const std::string & roomName, const std::string & avatarName, bool moderator, Unicode::String & result)
{
	const CuiChatRoomDataNode * const roomNode = findRoomNode (roomName);
	if (roomNode)
	{
		return setModerator (roomNode->data.id, avatarName, moderator, result);
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::kick          (uint32 id , const std::string & avatarName, Unicode::String & result)
{
	if (!id)
	{
		result += CuiStringIdsChatRoom::not_a_room.localize ();
		return false;
	}

	const CuiChatRoomDataNode * const roomNode = findRoomNode (id);
	if (roomNode)
	{
		//-- construct a possibly invalid avatar id
		CuiChatAvatarId cuiAvatar = CuiChatAvatarId (ChatAvatarId (avatarName));

		if (roomNode->findAvatarInRoom (avatarName, cuiAvatar, CuiChatRoomDataNode::ALT_members))
		{
			const ChatKickAvatarFromRoom msg (cuiAvatar.chatId, roomNode->data.path);
			GameNetwork::send (msg, true);
			return true;
		}

		result += CuiStringIdsChatRoom::err_no_such_member.localize ();
		return false;
	}

	result += CuiStringIdsChatRoom::not_found.localize ();
	return false;
}

//----------------------------------------------------------------------

bool CuiChatRoomManager::kick          (const std::string & roomName, const std::string & avatarName, Unicode::String & result)
{
	const CuiChatRoomDataNode * const roomNode = findRoomNode (roomName);
	if (roomNode)
	{
		return kick (roomNode->data.id, avatarName, result);
	}

	result += CuiStringIdsChatRoom::not_found.localize ();
	return false;
}

//----------------------------------------------------------------------

void CuiChatRoomManager::getRoomShortPath        (const CuiChatRoomDataNode & roomNode, std::string & shortPath)
{
	if (roomNode.data.id == getGroupRoomId () || roomNode.data.id == getGuildRoomId () || roomNode.data.id == getCityRoomId () )
	{
		shortPath = roomNode.name;
		return;
	}

	shortPath = roomNode.data.path;

	const ChatAvatarId & selfId = CuiChatManager::getSelfAvatarId ();

	const std::string & planetName = Game::getSceneId ();

	static const std::string dot (1, '.');

	const std::string prefix_planet   = planetName + dot;
	const std::string prefix_cluster  = selfId.cluster + dot;
	static const std::string prefix_game     = selfId.gameCode + dot;
	static const std::string prefix_chat     = ChatRoomTypes::ROOM_CHAT + dot;

	if (!_strnicmp (shortPath.c_str (), prefix_game.c_str (), prefix_game.size ()))
		shortPath.erase (0, prefix_game.size ());
	if (!_strnicmp (shortPath.c_str (), prefix_cluster.c_str (), prefix_cluster.size ()))
		shortPath.erase (0, prefix_cluster.size ());
	if (!_strnicmp (shortPath.c_str (), prefix_planet.c_str (), prefix_planet.size ()))
		shortPath.erase (0, prefix_planet.size ());

	if (!_strnicmp (shortPath.c_str (), prefix_chat.c_str (), prefix_chat.size ()))
		shortPath.erase (0, prefix_chat.size ());
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveRoomList (const RoomDataVector & v)
{
	s_chatRequestRoomListPending = false;

	//chatRoomTree.purge ();
	for (RoomDataVector::const_iterator it = v.begin (); it != v.end (); ++it)
	{
		const ChatRoomData & data = *it;

		const CuiChatRoomDataNode * const node = chatRoomTree.insertData (data);
		NOT_NULL (node);
		UNREF (node);
		DEBUG_WARNING (node->data.id == 0, ("Inserted a room with zero id: %d, %s", data.id, data.path.c_str ()));
	}

	Transceivers::modified.emitMessage (0);

	// Only display the "Channel Listing Complete" message if this is a bulk list update
	if (v.size() > 1)
	{
		Transceivers::statusMessage.emitMessage (StatusMessage (0, 0, CuiStringIdsChatRoom::list_received.localize ()));
	}
	
	// attempt to enter rooms that were unknown to the client
	std::map<std::string, int>::iterator i;
	for(i = s_pendingEnters.begin(); i != s_pendingEnters.end();)
	{
		// stop trying after a few attempts
		if (i->second >= 5)
		{
			s_pendingEnters.erase(i++);
		}
		else
		{
			Unicode::String none;
			enterRoom(i->first, none);
			++(i->second);
			++i;
		}
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::sendPrelocalizedChat(const Unicode::String & value)
{
	if (value.length() > 0)
		Transceivers::prelocalized.emitMessage (CuiPrelocalizedChatMessage (0, 0, value));
}

//----------------------------------------------------------------------

void CuiChatRoomManager::internalDestroyRoom (CuiChatRoomDataNode *& roomNode)
{
	NOT_NULL (roomNode);

	//-- remove from entered room list
	const ChatRoomsEnteredVector::iterator it = std::find (chatRoomsEntered.begin (), chatRoomsEntered.end (), roomNode->data.id);
	
	if (it != chatRoomsEntered.end ())
		chatRoomsEntered.erase (it);

	chatRoomsEnteredNames.erase (Unicode::toLower (roomNode->data.path));

	const CuiChatRoomDataNode::NodeVector & roomChildren = roomNode->getChildren ();

	//-- if the room has no children, remove it from the tree
	if (roomChildren.empty ())
	{
		CuiChatRoomDataNode * child  = roomNode;
		CuiChatRoomDataNode * parent = child->parent;
		
		while (child && parent)
		{
			parent->removeNode (*child);
			delete child;

			child = parent;
			parent = child->parent;

			//-- valid rooms should not get removed
			//-- rooms with children should not get removed
			if (!child || child->data.id || !child->getChildren ().empty ())
				break;
		}

		roomNode = 0;
	}

	//-- otherwise just gray it out
	else
		roomNode->invalidateRoom ();
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnDestroyRoom    (const ChatAvatarId & destroyer, uint32 result, uint32 roomId, uint32 sequence)
{
	CuiChatRoomDataNode * roomNode = findRoomNodeInternal (roomId);
		
	if (!roomNode)
	{
		if (sequence != 0)
		{
			char buf [256];
			snprintf (buf, sizeof (buf), "OnDestroyRoom for non-existant room [%d], destroyer [%s], result [%d], sequence [%d]", roomId, destroyer.getFullName ().c_str (), result, sequence);
			WARNING (true, (buf, roomId));
			Transceivers::statusMessage.emitMessage (StatusMessage (0, 0, Unicode::narrowToWide (buf)));
		}
		return;
	}

	if (result == CHATRESULT_SUCCESS)
	{
		if (roomNode)
		{
			//-- after destroying a room, we will receive a redundant notification of the room destory with a zero sequence id
			if (sequence == 0)
			{
				CuiChatAvatarId cuiDestroyer        (destroyer);	
				Transceivers::destroyed.emitMessage (Messages::Destroyed::Payload (roomNode, &cuiDestroyer));			
				internalDestroyRoom                 (roomNode);
				Transceivers::modified.emitMessage  (0);
			}
		}
	}
	else
	{
		if (sequence == 0)
		{
			WARNING (true, ("CuiChatRoomManager received a failed OnDestroyRoom with zero sequenceId"));
			return;
		}

		const std::string & attemptedRoomName = CuiChatRoomManagerHistory::roomDestroyRemove (sequence);

		Unicode::String str;

		const StringId * stringId = 0;
		stringId = &CuiStringIdsChatRoom::destroy_fail_prose;

		CuiStringVariablesManager::process (*stringId, attemptedRoomName, std::string (), str);
		Transceivers::destroyFailed.emitMessage (Messages::DestroyFailed::Payload (roomNode, str));
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveRoomQueryResults (const ChatQueryRoomResults & msg)
{
	//DEBUG_REPORT_LOG(true, ("CuiChatRoomManager::receiveRoomQueryResults() - room(%s)\n", Unicode::wideToNarrow(msg.getRoomData().title).c_str()));

	bool wasInTree = findRoomNodeInternal (msg.getRoomData ().id) != 0;

	CuiChatRoomDataNode * const roomNode = chatRoomTree.insertData (msg.getRoomData ());

	if (roomNode)
	{
		const AvatarVector & avatars    = msg.getAvatars    ();
		const AvatarVector & moderators = msg.getModerators ();
		const AvatarVector & invitees   = msg.getInvitees   ();
		const AvatarVector & banned     = msg.getBanned     ();

		roomNode->setMembers    (avatars);
		roomNode->setModerators (moderators);
		roomNode->setInvitees   (invitees);
		roomNode->setBanned     (banned);
	}
	
	if (!wasInTree)
		Transceivers::modified.emitMessage (0);
	else
		Transceivers::modified.emitMessage (roomNode->data.id);

	// Only tell the client they received channel information if they actually requested it
	// NOTE: The server will send the updated room information but the sequence ID will be zero
	if (msg.getSequence() != 0)
	{
		Transceivers::statusMessage.emitMessage (StatusMessage (0, 0, CuiStringIdsChatRoom::query_received.localize ()));
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::sendGenericStatusMessage (uint32 sequenceId, const StringId & prefixStringId, CuiChatRoomDataNode * roomNode, const CuiChatAvatarId * avatarId)
{
	Unicode::String str;
	
	if (!prefixStringId.isInvalid())
	{
		prefixStringId.localize (str);
	}
	
	if (roomNode)
	{
		if (!str.empty ())
			str.append (1, ' ');

		std::string shortPath;
		CuiChatRoomManager::getRoomShortPath (*roomNode, shortPath);
		str += Unicode::narrowToWide (shortPath);
	}
	
	if (avatarId)
	{
		if (!str.empty ())
			str.append (1, ' ');

		Unicode::String shortName;
		CuiChatManager::getShortName (avatarId->chatId, shortName);
		str += shortName;
	}
	
	Transceivers::statusMessage.emitMessage (StatusMessage (sequenceId, roomNode ? roomNode->data.id : 0, str));
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnEnteredRoom (const ChatOnEnteredRoom & msg)
{
	const uint32 result                  = msg.getResult ();
	const uint32 roomId                  = msg.getRoomId ();
	CuiChatRoomDataNode * const roomNode = CuiChatRoomManager::findRoomNodeInternal (roomId);
		
	if (!roomNode)
	{
		WARNING (true, ("received ChatOnEnteredRoom but room [%d] doesn't exist on client.", msg.getRoomId ())); 
		return;
	}

	if (result == CHATRESULT_SUCCESS)
	{
		s_pendingEnters.erase(roomNode->name);

		//-- ignore the system room
		if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_SYSTEM.c_str ()))
			return;

		//-- squirrel away the planet room id
		if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_PLANET.c_str ()))
		{
			setPlanetRoomId (roomId);
			return;
		}

		//-- squirrel away the group room id
		if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_GROUP.c_str ()))
		{
			setGroupRoomId (roomId);
			return;
		}

		//-- squirrel away the guild room id
		if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_GUILD.c_str ()))
		{
			setGuildRoomId (roomId);
			return;
		}

		//-- squirrel away the city room id
		if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_CITY.c_str ()))
		{
			setCityRoomId (roomId);
			return;
		}

		//-- the server put the client into a "named" room. A named room
		//   works much like the group and guild rooms, without the hackishness
		if(roomNode->parent)
		{
			if (roomNode->parent->name == "named")
			{
				setNamedRoomId(roomId);
				return;
			}
		}

		const ChatAvatarId & selfAvatarId = CuiChatManager::getSelfAvatarId ();
		const CuiChatAvatarId roomMember (msg.getCharacterName ());

		roomNode->setMember (roomMember, true);

		if (selfAvatarId == msg.getCharacterName ())
		{
			const uint32 sequence = msg.getSequence ();
			if (sequence)
				CuiChatRoomManagerHistory::roomJoinRemove (sequence);

			if (std::find (chatRoomsEntered.begin (), chatRoomsEntered.end (), roomId) == chatRoomsEntered.end ())
			{
				chatRoomsEnteredNames [Unicode::toLower (roomNode->data.path)] = roomId;
				chatRoomsEntered.push_back              (roomId);
				Transceivers::selfEntered.emitMessage   (*roomNode);
				Unicode::String dummyResult;
				CuiChatRoomManager::queryRoom (roomNode->data.path, dummyResult);
			}
			else
				WARNING (true, ("received self ChatOnEnteredRoom for room already entered (%s).", roomNode->data.path.c_str ()));
		}
		else
		{
			Transceivers::otherEntered.emitMessage  (Messages::OtherEntered::Payload (roomNode, &roomMember));
		}

		Transceivers::modified.emitMessage        (roomId);
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnLeaveRoom      (const ChatOnLeaveRoom & msg)
{
	const uint32 result                  = msg.getResultCode ();
	const uint32 roomId                  = msg.getRoomId ();
	CuiChatRoomDataNode * const roomNode = CuiChatRoomManager::findRoomNodeInternal (roomId);

	if (!roomNode)
	{
		WARNING (true, ("received receiveOnLeaveRoom but room [%d] doesn't exist on client.", roomId)); 
		return;
	}

	if (result == CHATRESULT_SUCCESS)
	{
		const ChatAvatarId & other = msg.getCharacterName ();

		// Process the chat avatar leaving the room
		processAvatarLeavingRoom(roomId, other);

			// Remove the client for the "entered" list if necessary
		const ChatAvatarId & selfAvatarId = CuiChatManager::getSelfAvatarId ();
		if (selfAvatarId == other)
		{	
			const ChatRoomsEnteredVector::iterator it = std::find (chatRoomsEntered.begin (), chatRoomsEntered.end (), roomId);

			if (it != chatRoomsEntered.end ())
			{
				chatRoomsEnteredNames.erase (Unicode::toLower (roomNode->data.path));
				chatRoomsEntered.erase (it);
				Transceivers::selfLeft.emitMessage(*roomNode);
			}
			else
				WARNING (true, ("received self ChatOnLeaveRoom for room not entered."));
		}
		else
		{
			const CuiChatAvatarId cuiId (other);
			Transceivers::otherLeft.emitMessage(Messages::OtherLeft::Payload (roomNode, &cuiId));
		}

		Transceivers::modified.emitMessage(roomId);
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnKickAvatarFromRoom      (const ChatOnKickAvatarFromRoom & msg)
{
	const uint32 result = msg.getResultCode ();

	// Output a pretty message to the client
	{
		std::string attemptedAvatarName = msg.getAvatarId().getFullName();
		std::string attemptedRoomName   = msg.getRoomName();
		
		Unicode::String str;
		
		const StringId * stringId = 0;
		
		switch (result)
		{
		case CHATRESULT_SUCCESS:
			stringId = &CuiStringIdsChatRoom::kick_success_prose;
			break;
			
		case ERR_SRCAVATARDOESNTEXIST:
		case ERR_DESTAVATARDOESNTEXIST:
			stringId = &CuiStringIdsChatRoom::kick_fail_avatar_not_found_prose;
			break;
		case ERR_ADDRESSDOESNTEXIST:
		case ERR_ADDRESSNOTROOM:
		case ERR_ADDRESSNOTAID:
			stringId = &CuiStringIdsChatRoom::kick_fail_room_not_exist_prose;
			break;
		case ERR_ROOM_NOPRIVILEGES:
			stringId = &CuiStringIdsChatRoom::kick_fail_no_privs_prose;
			break;
		default:
			stringId = &CuiStringIdsChatRoom::kick_fail_unknown_prose;
			break;
		}
		
		NOT_NULL (stringId);
		CuiStringVariablesManager::process (*stringId, attemptedRoomName, attemptedAvatarName, str);
		Transceivers::statusMessage.emitMessage (CuiChatRoomManagerStatusMessage (0, 0, str));
	}

	// If an avatar really was kicked, then we have some processing to do
	if (result == CHATRESULT_SUCCESS)
	{
		CuiChatRoomDataNode const * const roomNode = findRoomNode (msg.getRoomName());
		if (!roomNode)
		{
			WARNING (true, ("received receiveOnKickAvatarFromRoom but room [%s] doesn't exist on client.", msg.getRoomName())); 
			return;
		}

		const uint32         roomId = roomNode->data.id;
		const ChatAvatarId & other  = msg.getAvatarId();

		// Process the chat avatar leaving the room
		processAvatarLeavingRoom(roomId, other);

		// Remove the client for the "entered" list if necessary
		const ChatAvatarId & selfAvatarId = CuiChatManager::getSelfAvatarId ();
		if (selfAvatarId == other)
		{	
			const ChatRoomsEnteredVector::iterator it = std::find (chatRoomsEntered.begin (), chatRoomsEntered.end (), roomId);

			if (it != chatRoomsEntered.end ())
			{
				chatRoomsEnteredNames.erase (Unicode::toLower (roomNode->data.path));
				chatRoomsEntered.erase (it);
				Transceivers::selfLeft.emitMessage   (*roomNode);
			}
			else
				WARNING (true, ("received self ChatOnLeaveRoom for room not entered."));
		}
		else
		{
			const CuiChatAvatarId cuiId (other);
			Transceivers::otherLeft.emitMessage  (Messages::OtherLeft::Payload (roomNode, &cuiId));
		}

		Transceivers::modified.emitMessage   (roomId);
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnAddModeratorToRoom      (const ChatOnAddModeratorToRoom & msg)
{
	const uint32 result   = msg.getResultCode ();
	const uint32 sequence = msg.getSequenceId ();
	const CuiChatRoomManagerHistory::IntStringPair & attempt = CuiChatRoomManagerHistory::addModeratorRemove (sequence);

	// Output a pretty message to the client
	if (result != CHATRESULT_SUCCESS)
	{
		std::string attemptedAvatarName = attempt.second;
		std::string attemptedRoomName   = "UNKNOWN";
		const uint32 roomId             = attempt.first;
		
		if (roomId)
		{
			const CuiChatRoomDataNode * const roomNode = findRoomNode (roomId);
			if (roomNode)
				attemptedRoomName = roomNode->data.path;
		}
		
		Unicode::String str;
		
		const StringId * stringId = 0;
		
		switch (result)
		{
		case CHATRESULT_SUCCESS:
			stringId = &CuiStringIdsChatRoom::mod_add_success_prose;
			break;
			
		case ERR_SRCAVATARDOESNTEXIST:
		case ERR_DESTAVATARDOESNTEXIST:
			stringId = &CuiStringIdsChatRoom::mod_add_fail_avatar_not_found_prose;
			break;
		case ERR_ADDRESSDOESNTEXIST:
		case ERR_ADDRESSNOTROOM:
		case ERR_ADDRESSNOTAID:
			stringId = &CuiStringIdsChatRoom::mod_add_fail_room_not_exist_prose;
			break;
		case ERR_ROOM_UNKNOWNFAILURE:
			stringId = &CuiStringIdsChatRoom::mod_add_fail_room_not_moderated_prose;
			break;
		case ERR_ROOM_NOPRIVILEGES:
			stringId = &CuiStringIdsChatRoom::mod_add_fail_not_moderator_prose;
			break;
		default:
			stringId = &CuiStringIdsChatRoom::mod_add_fail_unknown_prose;
			break;
		}
		
		NOT_NULL (stringId);
		CuiStringVariablesManager::process (*stringId, attemptedRoomName, attemptedAvatarName, str);
		Transceivers::statusMessage.emitMessage (CuiChatRoomManagerStatusMessage (0, 0, str));
	}

	if (result == CHATRESULT_SUCCESS)
	{
		CuiChatRoomDataNode * const roomNode = const_cast<CuiChatRoomDataNode * >(findRoomNode (msg.getRoomName ()));

		if (!roomNode)
		{
			WARNING (true, ("received ChatOnAddModeratorToRoom but room doesn't exist on client.")); 
			return;
		}

		const CuiChatAvatarId avatarId (msg.getAvatarId ());
		roomNode->setModerator (avatarId, true);

		Transceivers::moderatorAdded.emitMessage  (Messages::ModeratorAdded::Payload (roomNode, &avatarId));
		Transceivers::modified.emitMessage        (roomNode->data.id);
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnRemoveModeratorFromRoom (const ChatOnRemoveModeratorFromRoom & msg)
{
	const uint32 result   = msg.getResultCode ();
	const uint32 sequence = msg.getSequenceId ();
	const CuiChatRoomManagerHistory::IntStringPair & attempt = CuiChatRoomManagerHistory::removeModeratorRemove (sequence);

	// Output a pretty message to the client
	if (result != CHATRESULT_SUCCESS)
	{
		std::string attemptedAvatarName = attempt.second;
		std::string attemptedRoomName   = "UNKNOWN";
		const uint32 roomId             = attempt.first;
		
		if (roomId)
		{
			const CuiChatRoomDataNode * const roomNode = findRoomNode (roomId);
			if (roomNode)
				attemptedRoomName = roomNode->data.path;
		}
		
		Unicode::String str;
		
		const StringId * stringId = 0;
		
		switch (result)
		{
		case CHATRESULT_SUCCESS:
			stringId = &CuiStringIdsChatRoom::mod_remove_success_prose;
			break;
			
		case ERR_SRCAVATARDOESNTEXIST:
		case ERR_DESTAVATARDOESNTEXIST:
			stringId = &CuiStringIdsChatRoom::mod_remove_fail_avatar_not_found_prose;
			break;
		case ERR_ADDRESSDOESNTEXIST:
		case ERR_ADDRESSNOTROOM:
		case ERR_ADDRESSNOTAID:
			stringId = &CuiStringIdsChatRoom::mod_remove_fail_room_not_exist_prose;
			break;
		case ERR_ROOM_UNKNOWNFAILURE:
			stringId = &CuiStringIdsChatRoom::mod_remove_fail_room_not_moderated_prose;
			break;
		case ERR_ROOM_NOPRIVILEGES:
			stringId = &CuiStringIdsChatRoom::mod_remove_fail_not_moderator_prose;
			break;
		default:
			stringId = &CuiStringIdsChatRoom::mod_remove_fail_unknown_prose;
			break;
		}
		
		NOT_NULL (stringId);
		CuiStringVariablesManager::process (*stringId, attemptedRoomName, attemptedAvatarName, str);
		Transceivers::statusMessage.emitMessage (CuiChatRoomManagerStatusMessage (0, 0, str));
	}

	if (result == CHATRESULT_SUCCESS)
	{
		CuiChatRoomDataNode * const roomNode = const_cast<CuiChatRoomDataNode * >(findRoomNode (msg.getRoomName ()));

		if (!roomNode)
		{
			WARNING (true, ("received ChatOnAddModeratorToRoom but room doesn't exist on client.")); 
			return;
		}

		const CuiChatAvatarId avatarId (msg.getAvatarId ());
		roomNode->setModerator (avatarId, false);

		Transceivers::moderatorRemoved.emitMessage(Messages::ModeratorRemoved::Payload (roomNode, &avatarId));
		Transceivers::modified.emitMessage        (roomNode->data.id);
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnBanAvatarFromRoom            (const ChatOnBanAvatarFromRoom & msg)
{
	const uint32 result            = msg.getResult ();
	const std::string & roomName   = msg.getRoomName ();
	const CuiChatAvatarId bannee   (msg.getBannee ());
	const CuiChatAvatarId banner   (msg.getBanner ());
		
	CuiChatRoomDataNode * const roomNode = findRoomNodeInternal (roomName);
	
	if (!roomNode)
	{
		WARNING (true, ("received ChatOnBanAvatarFromRoom but room [%s] doesn't exist on client.", roomName.c_str ()));
		return;
	}

	// Output a pretty message to client
	{
		Unicode::String str;
		
		const StringId * stringId = 0;
		
		switch (result)
		{
		case CHATRESULT_SUCCESS:
			stringId = &CuiStringIdsChatRoom::ban_success_prose;
			break;
			
		case ERR_SRCAVATARDOESNTEXIST:
		case ERR_DESTAVATARDOESNTEXIST:
			stringId = &CuiStringIdsChatRoom::ban_fail_avatar_not_found_prose;
			break;
		case ERR_ADDRESSDOESNTEXIST:
		case ERR_ADDRESSNOTROOM:
		case ERR_ADDRESSNOTAID:
			stringId = &CuiStringIdsChatRoom::ban_fail_room_not_exist_prose;
			break;
		case ERR_ROOM_BANNEDAVATAR:
			stringId = &CuiStringIdsChatRoom::ban_fail_banned_prose;
			break;
		case ERR_ROOM_NOPRIVILEGES:
			stringId = &CuiStringIdsChatRoom::ban_fail_no_privs_prose;
			break;
		default:
			stringId = &CuiStringIdsChatRoom::ban_fail_unknown_prose;
			break;
		}

		NOT_NULL (stringId);
		std::string shortPath;
		getRoomShortPath (*roomNode, shortPath);
		Unicode::String shortName;
		CuiChatManager::getShortName (bannee.chatId, shortName);
		CuiStringVariablesManager::process (*stringId, shortPath, Unicode::wideToNarrow (shortName), str);
		Transceivers::statusMessage.emitMessage (CuiChatRoomManagerStatusMessage (0, 0, str));
	}

	if (result == CHATRESULT_SUCCESS)
	{
		const uint32         roomId = roomNode->data.id;
		const ChatAvatarId & other  = msg.getBannee();

		// Process the chat avatar leaving the room
		processAvatarLeavingRoom(roomId, other);

		// Remove the client for the "entered" list if necessary
		const ChatAvatarId & selfAvatarId = CuiChatManager::getSelfAvatarId ();
		if (selfAvatarId == other)
		{	
			const ChatRoomsEnteredVector::iterator it = std::find (chatRoomsEntered.begin (), chatRoomsEntered.end (), roomId);

			if (it != chatRoomsEntered.end ())
			{
				chatRoomsEnteredNames.erase (Unicode::toLower (roomNode->data.path));
				chatRoomsEntered.erase (it);
				Transceivers::selfLeft.emitMessage   (*roomNode);
			}
			else
				WARNING (true, ("received self ChatOnLeaveRoom for room not entered."));
		}
		else
		{
			const CuiChatAvatarId cuiId (other);
			Transceivers::otherLeft.emitMessage  (Messages::OtherLeft::Payload (roomNode, &cuiId));
		}

		roomNode->setBanned (bannee, true);

		Transceivers::banned.emitMessage   (Messages::Banned::Payload (roomNode, Messages::AvatarPair (&bannee, &banner)));
		Transceivers::modified.emitMessage (roomId);
	}
}

//----------------------------------------------------------------------

void  CuiChatRoomManager::receiveOnUnbanAvatarFromRoom          (const ChatOnUnbanAvatarFromRoom & msg)
{
	const uint32 result            = msg.getResult ();
	const std::string & roomName   = msg.getRoomName ();
	const CuiChatAvatarId unbannee (msg.getUnbannee ());
	const CuiChatAvatarId unbanner (msg.getUnbanner ());

	CuiChatRoomDataNode * const roomNode = findRoomNodeInternal (roomName);
	
	if (!roomNode)
	{
		WARNING (true, ("received ChatOnBanAvatarFromRoom but room [%s] doesn't exist on client.", roomName.c_str ()));
		return;
	}

	// Output a pretty message to client
	{
		Unicode::String str;
		
		const StringId * stringId = 0;
		
		switch (result)
		{
		case CHATRESULT_SUCCESS:
			stringId = &CuiStringIdsChatRoom::unban_success_prose;
			break;
			
		case ERR_SRCAVATARDOESNTEXIST:
		case ERR_DESTAVATARDOESNTEXIST:
			stringId = &CuiStringIdsChatRoom::unban_fail_avatar_not_found_prose;
			break;
		case ERR_ADDRESSDOESNTEXIST:
		case ERR_ADDRESSNOTROOM:
		case ERR_ADDRESSNOTAID:
			stringId = &CuiStringIdsChatRoom::unban_fail_room_not_exist_prose;
			break;
		case ERR_ROOM_BANNEDAVATAR:
			stringId = &CuiStringIdsChatRoom::unban_fail_banned_prose;
			break;
		case ERR_ROOM_NOPRIVILEGES:
			stringId = &CuiStringIdsChatRoom::unban_fail_no_privs_prose;
			break;
		default:
			stringId = &CuiStringIdsChatRoom::unban_fail_unknown_prose;
			break;
		}

		NOT_NULL (stringId);
		std::string shortPath;
		getRoomShortPath (*roomNode, shortPath);
		Unicode::String shortName;
		CuiChatManager::getShortName (unbannee.chatId, shortName);
		CuiStringVariablesManager::process (*stringId, shortPath, Unicode::wideToNarrow (shortName), str);
		Transceivers::statusMessage.emitMessage (CuiChatRoomManagerStatusMessage (0, 0, str));
	}

	if (result == CHATRESULT_SUCCESS)
	{
		roomNode->setBanned (unbannee, false);

		Transceivers::unbanned.emitMessage (Messages::Unbanned::Payload (roomNode, Messages::AvatarPair (&unbannee, &unbanner)));
		Transceivers::modified.emitMessage (roomNode->data.id);
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnCreateRoom     (const uint32 sequence, const uint32 result, const ChatRoomData & roomData)
{	
	const std::string & attemptedRoomName = CuiChatRoomManagerHistory::roomCreateRemove (sequence);

	if (attemptedRoomName.empty ())
	{
		WARNING (true, ("CuiChatRoomManager receiveOnCreateRoom for invalid sequence=%d (%s, result=%d)", sequence, roomData.path.c_str (), result));
		return;
	}

	const CuiChatRoomDataNode * const oldRoomNode = findRoomNode (roomData.id);
	if (oldRoomNode)
	{
		WARNING (true, ("CuiChatRoomManager receiveOnCreateRoom for pre-existing room [%s] sequence=%d (%s, id=%d, result=%d)", oldRoomNode->data.path.c_str (), sequence, roomData.path.c_str (), roomData.id, result));
		return;
	}

	if (result == CHATRESULT_SUCCESS)
	{
		const ChatRoomData & crd = roomData;

		if (crd.path.empty () || crd.id == 0)
		{
			WARNING (true, ("CuiChatRoomManager received invalid ChatRoomData sequence=%d (%s, id=%d, result=%d)", sequence, roomData.path.c_str (), crd.id, result));
			return;
		}

		const CuiChatRoomDataNode * const roomNode = chatRoomTree.insertData (crd);
		NOT_NULL (roomNode);

		if (roomNode)
		{
			if (CuiPreferences::getAutoJoinChatRoomOnCreate ())
			{
				Unicode::String tmp;
				enterRoom (roomNode->data.id, roomNode, tmp);
 			}

			Transceivers::created.emitMessage   (*roomNode);
			Transceivers::modified.emitMessage  (0);
		}
	}
	else
	{
		Unicode::String str;
		
		const StringId * stringId = 0;
		
		switch (result)
		{
		case ERR_ROOMALREADYEXISTS:
			break;
		case ERR_ADDRESSNOTROOM:
			stringId = &CuiStringIdsChatRoom::create_create_fail_invalid_name_prose;
			break;
		default:
			stringId = &CuiStringIdsChatRoom::create_create_fail_unknown_prose;
			break;
		}

		if(stringId)
		{
			CuiStringVariablesManager::process (*stringId, attemptedRoomName, std::string (), str);
			Transceivers::createFailed.emitMessage (str);
		}
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveFailedEnterRoom             (const uint32 sequence, const uint32 result)
{
	if (!sequence)
		return;

	const CuiChatRoomManagerHistory::IntStringPair & attempt = CuiChatRoomManagerHistory::roomJoinRemove (sequence);

	std::string attemptedRoomName = attempt.second;

	const uint32 roomId = attempt.first;

	if (roomId)
	{
		const CuiChatRoomDataNode * const roomNode = findRoomNode (roomId);
		if (roomNode)
			attemptedRoomName = roomNode->data.path;
	}

	Unicode::String str;

	const StringId * stringId = 0;
	
	switch (result)
	{
	case ERR_ROOM_BANNEDAVATAR:
		stringId = &CuiStringIdsChatRoom::join_fail_banned_prose;
		break;
	case ERR_ROOM_PRIVATEROOM:
	case ERR_ROOM_NOPRIVILEGES:
		stringId = &CuiStringIdsChatRoom::join_fail_not_invited_prose;
		break;
	case ERR_ADDRESSDOESNTEXIST:
		stringId = &CuiStringIdsChatRoom::join_fail_invalid_room_id_prose;
		break;
	case ERR_ADDRESSNOTROOM:
		stringId = &CuiStringIdsChatRoom::join_fail_invalid_room_name_prose;
		break;
	case SWG_CHAT_ERR_CHAT_SERVER_UNAVAILABLE:
		stringId = &CuiStringIdsChatRoom::join_fail_swg_chat_server_unavailable_prose;
		break;
	case SWG_CHAT_ERR_WRONG_FACTION:
		stringId = &CuiStringIdsChatRoom::join_fail_wrong_faction_prose;
		break;
	case SWG_CHAT_ERR_WRONG_GCW_REGION_DEFENDER_FACTION:
		stringId = &CuiStringIdsChatRoom::join_fail_wrong_gcw_region_defender_faction_prose;
		break;
	case SWG_CHAT_ERR_NOT_WARDEN:
		stringId = &CuiStringIdsChatRoom::join_fail_not_warden_prose;
		break;
	case SWG_CHAT_ERR_NOT_GUILD_LEADER:
		stringId = &CuiStringIdsChatRoom::join_fail_not_guild_leader_prose;
		break;
	case SWG_CHAT_ERR_NOT_CITY_MAYOR:
		stringId = &CuiStringIdsChatRoom::join_fail_not_mayor_prose;
		break;
	case SWG_CHAT_ERR_NOT_WAR_PLANNER:
		stringId = &CuiStringIdsChatRoom::join_fail_cannot_enter_war_room_prose;
		break;
	case SWG_CHAT_ERR_INVALID_OBJECT:
		stringId = &CuiStringIdsChatRoom::join_fail_invalid_object_prose;
		break;
	case SWG_CHAT_ERR_NO_GAME_SERVER:
		stringId = &CuiStringIdsChatRoom::join_fail_no_game_server_prose;
		break;
	case ERR_ROOM_ALREADYINROOM:
		break;
	default:
		stringId = &CuiStringIdsChatRoom::join_fail_unknown_prose;
		break;
	}

	if(stringId)
	{
		CuiStringVariablesManager::process (*stringId, attemptedRoomName, std::string (), str);
		Transceivers::joinFailed.emitMessage (str);
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveChatOnReceiveRoomInvitation (const ChatOnReceiveRoomInvitation & msg)
{
	CuiChatRoomDataNode * const roomNode = const_cast<CuiChatRoomDataNode * >(findRoomNode (msg.getRoomName ()));

	if (!roomNode)
	{
		WARNING (true, ("received receiveChatOnReceiveRoomInvitation but room doesn't exist on client.")); 
		return;
	}

	//-- squirrel away the group room id
	if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_GROUP.c_str ()))
	{
		setGroupRoomId (roomNode->data.id);
		return;
	}

	//-- squirrel away the guild room id
	if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_GUILD.c_str ()))
	{
		setGuildRoomId (roomNode->data.id);
		return;
	}

	//-- squirrel away the city room id
	if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_CITY.c_str ()))
	{
		setCityRoomId (roomNode->data.id);
		return;
	}

	const CuiChatAvatarId invitee  (CuiChatManager::getSelfAvatarId ());
	const CuiChatAvatarId avatarId (msg.getInvitorAvatar ());
	roomNode->setInvitee (invitee, true);

	Transceivers::inviteeAdded.emitMessage  (Messages::InviteeAdded::Payload (roomNode, Messages::AvatarPair (&invitee, &avatarId)));
	Transceivers::modified.emitMessage      (roomNode->data.id);

	//-- notify user of invitation
	std::string shortPath;
	CuiChatRoomManager::getRoomShortPath (*roomNode, shortPath);
	const Unicode::String & shortName = CuiChatManager::getShortName (avatarId.chatId);
	Unicode::String result;
	CuiStringVariablesManager::process (CuiStringIdsChatRoom::invited_prose, Unicode::emptyString, shortName, Unicode::narrowToWide (shortPath), result);
	CuiSystemMessageManager::sendFakeSystemMessage (result);
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveChatRoomMessage (const ChatRoomMessage & msg)
{
	const uint32 roomId = msg.getFromRoom ();
	
	if (roomId == CuiChatRoomManager::getPlanetRoomId () || 
		roomId == CuiChatRoomManager::getGroupRoomId () ||
		roomId == CuiChatRoomManager::getGuildRoomId () ||
		roomId == CuiChatRoomManager::getCityRoomId () ||
		std::find (chatRoomsEntered.begin (), chatRoomsEntered.end (), roomId) != chatRoomsEntered.end () ||
		CuiChatRoomManager::hasNamedRoom(roomId))
	{
		const CuiChatRoomMessage chatRoomMsg (roomId, CuiChatAvatarId (msg.getFromName (), NetworkId::cms_invalid), msg.getMessage (), msg.getOutOfBand ());
		Transceivers::messageReceived.emitMessage (chatRoomMsg);
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveChatOnSendRoomMessage       (const uint32 result, const uint32 sequence)
{
	const std::string & attemptedRoomName = CuiChatRoomManagerHistory::roomSendRemove (sequence);

	if (result != CHATRESULT_SUCCESS)
	{
		const StringId * sid = 0;

		if (result == ERR_ROOM_NOPRIVILEGES)
			sid = &CuiStringIdsChatRoom::send_err_insufficient_privs_prose;
		else if (result == ERR_ROOM_UNKNOWNFAILURE)
			sid = &CuiStringIdsChatRoom::send_err_not_a_moderator_prose;
		else
			sid = &CuiStringIdsChatRoom::send_err_unknown_prose;
		
		Unicode::String str;
		CuiStringVariablesManager::process (*sid, attemptedRoomName, std::string (), str);
		Transceivers::statusMessage.emitMessage (CuiChatRoomManagerStatusMessage (0, 0, str));
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnInviteGroupToRoom (const ChatOnInviteGroupToRoom & msg)
{
	const uint32 result          = msg.getResult ();
	const std::string & roomName = msg.getRoomName ();
	const ChatAvatarId invitor   = msg.getInvitor ();
	const ChatAvatarId invitee   = msg.getInvitee ();

	CuiChatRoomDataNode * const roomNode = findRoomNodeInternal (roomName);
	
	if (!roomNode)
	{
		WARNING (true, ("received ChatOnInviteGroupToRoom but room [%s] doesn't exist on client.", roomName.c_str ()));
		return;
	}

	// Output a pretty message to the client
	{
		Unicode::String str;

		const StringId * stringId = 0;

		switch (result)
		{
		case CHATRESULT_SUCCESS:
			stringId = &CuiStringIdsChatRoom::invite_group_success_prose;
			break;
			
		case ERR_DESTAVATARDOESNTEXIST:
			stringId = &CuiStringIdsChatRoom::invite_group_fail_avatar_not_found_prose;
			break;

		default:
			stringId = &CuiStringIdsChatRoom::invite_group_fail_unknown_prose;
			break;
		}

		NOT_NULL (stringId);

		// Get the short version of the room name
		std::string shortPath;
		getRoomShortPath (*roomNode, shortPath);

		// Get the short version of the character name
		Unicode::String shortName;
		CuiChatManager::getShortName (invitee, shortName);

		CuiStringVariablesManager::process (*stringId, shortPath, Unicode::wideToNarrow (shortName), str);
		Transceivers::statusMessage.emitMessage (CuiChatRoomManagerStatusMessage (0, 0, str));
	}

	if (result == CHATRESULT_SUCCESS)
	{
		// This message only reports whether we could find the group members
		// and invite them individually.  Successful invites for group members
		// will be received in receiveOnInviteToRoom(...)
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnInviteToRoom (const ChatOnInviteToRoom & msg)
{
	const uint32 result          = msg.getResult ();
	const std::string & roomName = msg.getRoomName ();
	const ChatAvatarId invitor   = msg.getInvitor ();
	const ChatAvatarId invitee   = msg.getInvitee ();

	CuiChatRoomDataNode * const roomNode = findRoomNodeInternal (roomName);
	
	if (!roomNode)
	{
		WARNING (true, ("received receiveOnInviteToRoom but room [%s] doesn't exist on client.", roomName.c_str ()));
		return;
	}

	// Output a pretty message to the client
	{
		Unicode::String str;

		const StringId * stringId = 0;

		switch (result)
		{
		case CHATRESULT_SUCCESS:
			stringId = &CuiStringIdsChatRoom::invite_success_prose;
			break;
			
		case ERR_SRCAVATARDOESNTEXIST:
		case ERR_DESTAVATARDOESNTEXIST:
			stringId = &CuiStringIdsChatRoom::invite_fail_avatar_not_found_prose;
			break;
		case ERR_ADDRESSNOTAID:
		case ERR_ADDRESSDOESNTEXIST:
		case ERR_ADDRESSNOTROOM:
			stringId = &CuiStringIdsChatRoom::invite_fail_room_not_exist_prose;
			break;
		case ERR_ROOM_UNKNOWNFAILURE:
			stringId = &CuiStringIdsChatRoom::invite_fail_room_not_private_prose;
			break;
		case ERR_ROOM_NOPRIVILEGES:
			stringId = &CuiStringIdsChatRoom::invite_fail_not_moderator_prose;
			break;
		default:
			stringId = &CuiStringIdsChatRoom::invite_fail_unknown_prose;
			break;
		}

		NOT_NULL (stringId);

		// Get the short version of the room name
		std::string shortPath;
		getRoomShortPath (*roomNode, shortPath);

		// Get the short version of the character name
		Unicode::String shortName;
		CuiChatManager::getShortName (invitee, shortName);

		CuiStringVariablesManager::process (*stringId, shortPath, Unicode::wideToNarrow (shortName), str);
		Transceivers::statusMessage.emitMessage (CuiChatRoomManagerStatusMessage (0, 0, str));
	}

	if (roomNode && result == CHATRESULT_SUCCESS)
	{
		const CuiChatAvatarId inviteeId (invitee);
		const CuiChatAvatarId avatarId = CuiChatAvatarId (CuiChatManager::getSelfAvatarId ());

		roomNode->setInvitee (inviteeId, true);

		Transceivers::inviteeAdded.emitMessage (Messages::InviteeAdded::Payload (roomNode, Messages::AvatarPair (&inviteeId, &avatarId)));
		Transceivers::modified.emitMessage     (roomNode->data.id);
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManager::receiveOnUninviteFromRoom        (const uint32 result, const uint32 sequence)
{
	const CuiChatRoomManagerHistory::IntStringPair & attempt = CuiChatRoomManagerHistory::uninviteRemove (sequence);

	std::string attemptedAvatarName = attempt.second;
	std::string attemptedRoomName   = "UNKNOWN";
	const uint32 roomId             = attempt.first;
	CuiChatRoomDataNode * roomNode = 0;

	if (roomId)
	{
		roomNode = findRoomNodeInternal (roomId);
		if (roomNode)
			attemptedRoomName = roomNode->data.path;
	}

	// Output a pretty message to the client
	{
		Unicode::String str;

		const StringId * stringId = 0;

		switch (result)
		{
		case CHATRESULT_SUCCESS:
			stringId = &CuiStringIdsChatRoom::uninvite_success_prose;
			break;
			
		case ERR_ROOM_PRIVATEROOM:
			stringId = &CuiStringIdsChatRoom::uninvite_fail_avatar_not_invited_prose;
			break;

		case ERR_SRCAVATARDOESNTEXIST:
		case ERR_DESTAVATARDOESNTEXIST:
			stringId = &CuiStringIdsChatRoom::uninvite_fail_avatar_not_found_prose;
			break;
		case ERR_ADDRESSNOTAID:
		case ERR_ADDRESSNOTROOM:
		case ERR_ADDRESSDOESNTEXIST:
			stringId = &CuiStringIdsChatRoom::uninvite_fail_room_not_exist_prose;
			break;
		case ERR_ROOM_UNKNOWNFAILURE:
			stringId = &CuiStringIdsChatRoom::uninvite_fail_room_not_private_prose;
			break;
		case ERR_ROOM_NOPRIVILEGES:
			stringId = &CuiStringIdsChatRoom::uninvite_fail_not_moderator_prose;
			break;
		default:
			stringId = &CuiStringIdsChatRoom::uninvite_fail_unknown_prose;
			break;
		}

		NOT_NULL (stringId);
		CuiStringVariablesManager::process (*stringId, attemptedRoomName, attemptedAvatarName, str);
		Transceivers::statusMessage.emitMessage (CuiChatRoomManagerStatusMessage (0, 0, str));
	}

	if (roomNode && result == CHATRESULT_SUCCESS)
	{
		ChatAvatarId inviteeId;
		CuiChatManager::constructChatAvatarId (attemptedAvatarName, inviteeId);
		const CuiChatAvatarId invitee (inviteeId);
		const CuiChatAvatarId avatarId = CuiChatAvatarId (CuiChatManager::getSelfAvatarId ());

		roomNode->setInvitee (invitee, false);

		Transceivers::inviteeRemoved.emitMessage  (Messages::InviteeRemoved::Payload (roomNode, Messages::AvatarPair (&invitee, &avatarId)));
		Transceivers::modified.emitMessage (roomId);
	}
}

//----------------------------------------------------------------------

uint32  CuiChatRoomManager::getPlanetRoomId         ()
{
	return s_planetRoomId;
}

//----------------------------------------------------------------------

uint32  CuiChatRoomManager::getGroupRoomId         ()	
{
	return s_groupRoomId;
}

//----------------------------------------------------------------------

uint32  CuiChatRoomManager::getGuildRoomId         ()	
{
	return s_guildRoomId;
}

//----------------------------------------------------------------------

uint32  CuiChatRoomManager::getCityRoomId         ()	
{
	return s_cityRoomId;
}

//----------------------------------------------------------------------

void CuiChatRoomManager::setPlanetRoomId (uint32 roomId)
{
	s_planetRoomId = roomId;
	Transceivers::planetRoomIdChanged.emitMessage (roomId);
}

//----------------------------------------------------------------------

void CuiChatRoomManager::setGroupRoomId (uint32 roomId)
{
	s_groupRoomId = roomId;
	Transceivers::groupRoomIdChanged.emitMessage (roomId);
}

//-----------------------------------------------------------------------

void CuiChatRoomManager::setGuildRoomId (uint32 roomId)
{
	s_guildRoomId = roomId;
	Transceivers::guildRoomIdChanged.emitMessage (roomId);
}

//-----------------------------------------------------------------------

void CuiChatRoomManager::setCityRoomId (uint32 roomId)
{
	s_cityRoomId = roomId;
	Transceivers::cityRoomIdChanged.emitMessage (roomId);
}

//-----------------------------------------------------------------------

void CuiChatRoomManager::setNamedRoomId (uint32 roomId)
{
	IGNORE_RETURN(s_namedRoomIds.insert(roomId));
	Transceivers::namedRoomIdChanged.emitMessage(roomId);
}

//----------------------------------------------------------------------

void CuiChatRoomManager::processAvatarLeavingRoom (uint32 roomId, const ChatAvatarId & avatarId)
{
	CuiChatRoomDataNode * const roomNode = CuiChatRoomManager::findRoomNodeInternal (roomId);

	if (!roomNode)
	{
		WARNING (true, ("CuiChatRoomManager::processAvatarLeaveRoom(): room [%d] doesn't exist on client.", roomId)); 
		return;
	}

	const ChatAvatarId & selfAvatarId = CuiChatManager::getSelfAvatarId ();

	//-- ignore the system room
	if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_SYSTEM.c_str ()))
		return;

	//-- unset the planet room id
	if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_PLANET.c_str ()))
	{
		if (s_planetRoomId == roomId && selfAvatarId == avatarId)
			setPlanetRoomId (0);
		return;
	}

	//-- unset the group room id
	if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_GROUP.c_str ()))
	{
		if (s_groupRoomId == roomId && selfAvatarId == avatarId)
			setGroupRoomId (0);
		return;
	}

	//-- unset the guild room id
	if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_GUILD.c_str ()))
	{
		if (s_guildRoomId == roomId && selfAvatarId == avatarId)
			setGuildRoomId (0);
		return;
	}

	//-- unset the city room id
	if (!_stricmp (roomNode->name.c_str (), ChatRoomTypes::ROOM_CITY.c_str ()))
	{
		if (s_cityRoomId == roomId && selfAvatarId == avatarId)
			setCityRoomId (0);
		return;
	}

	if(hasNamedRoom(roomId) && selfAvatarId == avatarId)
	{
		IGNORE_RETURN(s_namedRoomIds.erase(roomId));
	}

	roomNode->setMember (CuiChatAvatarId (avatarId), false);
}

//-----------------------------------------------------------------------

void CuiChatRoomManager::addFriend(const ChatAvatarId & id)
{
	static uint32 sequence = 0;
	ChatAddFriend chat(++sequence, id);
	GameNetwork::send(chat, true);
}

//-----------------------------------------------------------------------

bool CuiChatRoomManager::hasNamedRoom(uint32 roomId)
{
	return (s_namedRoomIds.find(roomId) != s_namedRoomIds.end());
}

//======================================================================
