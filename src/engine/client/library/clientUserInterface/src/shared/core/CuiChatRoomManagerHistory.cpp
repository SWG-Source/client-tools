//======================================================================
//
// CuiChatRoomManagerHistory.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatRoomManagerHistory.h"

#include <map>

//======================================================================

namespace
{
	typedef CuiChatRoomManagerHistory::IntStringPair IntStringPair;

	typedef stdmap<uint32, std::string>::fwd   StringMap;
	typedef stdmap<uint32, IntStringPair>::fwd IntStringMap;
	
	namespace Maps
	{
		StringMap    roomCreate;
		IntStringMap roomJoin;
		StringMap    roomDestroy;
		StringMap    roomSend;
		IntStringMap uninvite;
		IntStringMap addModerator;
		IntStringMap removeModerator;
	}

	template <typename T> T removeFromMap (std::map<uint32, T> & theMap, uint32 sequenceId)
	{
		const std::map<uint32, T>::iterator it = theMap.find (sequenceId);
		
		if (it != theMap.end ())
		{
			const T ret = (*it).second;
			theMap.erase (it);
			return ret;
		}
		
		return T ();
	}
}

//----------------------------------------------------------------------

void CuiChatRoomManagerHistory::roomCreateAdd (uint32 sequenceId, const std::string & roomName)
{
	Maps::roomCreate [sequenceId] = roomName;
}

//----------------------------------------------------------------------

std::string CuiChatRoomManagerHistory::roomCreateRemove (uint32 sequenceId)
{
	return removeFromMap<std::string> (Maps::roomCreate, sequenceId);
}

//----------------------------------------------------------------------

void CuiChatRoomManagerHistory::roomJoinAdd      (uint32 sequenceId, const std::string & roomName, const uint32 & roomId)
{
	Maps::roomJoin [sequenceId] = IntStringPair (roomId, roomName);
}

//----------------------------------------------------------------------

IntStringPair CuiChatRoomManagerHistory::roomJoinRemove   (uint32 sequenceId)
{
	return removeFromMap<IntStringPair> (Maps::roomJoin, sequenceId);
}
 

//----------------------------------------------------------------------

void CuiChatRoomManagerHistory::roomDestroyAdd      (uint32 sequenceId, const std::string & roomName)
{
	Maps::roomDestroy [sequenceId] = roomName;
}

//----------------------------------------------------------------------

std::string CuiChatRoomManagerHistory::roomDestroyRemove   (uint32 sequenceId)
{
	return removeFromMap <std::string>(Maps::roomDestroy, sequenceId);
}

//----------------------------------------------------------------------

void CuiChatRoomManagerHistory::roomSendAdd      (uint32 sequenceId, const std::string & roomName)
{
	Maps::roomSend [sequenceId] = roomName;
}

//----------------------------------------------------------------------

std::string CuiChatRoomManagerHistory::roomSendRemove   (uint32 sequenceId)
{
	return removeFromMap <std::string>(Maps::roomSend, sequenceId);
}

//----------------------------------------------------------------------

void CuiChatRoomManagerHistory::uninviteAdd         (uint32 sequenceId, const uint32 & roomId, const std::string & avatarName)
{
	Maps::uninvite [sequenceId] = IntStringPair (roomId, avatarName);
}

//----------------------------------------------------------------------

IntStringPair CuiChatRoomManagerHistory::uninviteRemove      (uint32 sequenceId)
{
	return removeFromMap<IntStringPair> (Maps::uninvite, sequenceId);
}

//----------------------------------------------------------------------

void CuiChatRoomManagerHistory::addModeratorAdd           (uint32 sequenceId, const uint32 & roomId, const std::string & avatarName)
{
	Maps::addModerator [sequenceId] = IntStringPair (roomId, avatarName);
}

//----------------------------------------------------------------------

IntStringPair CuiChatRoomManagerHistory::addModeratorRemove        (uint32 sequenceId)
{
	return removeFromMap<IntStringPair> (Maps::addModerator, sequenceId);
}

//----------------------------------------------------------------------

void CuiChatRoomManagerHistory::removeModeratorAdd         (uint32 sequenceId, const uint32 & roomId, const std::string & avatarName)
{
	Maps::removeModerator [sequenceId] = IntStringPair (roomId, avatarName);
}

//----------------------------------------------------------------------

IntStringPair CuiChatRoomManagerHistory::removeModeratorRemove      (uint32 sequenceId)
{
	return removeFromMap<IntStringPair> (Maps::removeModerator, sequenceId);
}

//======================================================================
