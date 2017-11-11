//======================================================================
//
// CuiChatRoomManagerHistory.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiChatRoomManagerHistory_H
#define INCLUDED_CuiChatRoomManagerHistory_H

//======================================================================

class CuiChatRoomManagerHistory
{
public:

	typedef std::pair<uint32, std::string>   IntStringPair;

	static void                   roomCreateAdd       (uint32 sequenceId, const std::string & roomName);
	static std::string            roomCreateRemove    (uint32 sequenceId);

	static void                   roomJoinAdd         (uint32 sequenceId, const std::string & roomName, const uint32 & roomId);
	static IntStringPair          roomJoinRemove      (uint32 sequenceId);

	static void                   roomDestroyAdd      (uint32 sequenceId, const std::string & roomName);
	static std::string            roomDestroyRemove   (uint32 sequenceId);

	static void                   roomSendAdd         (uint32 sequenceId, const std::string & roomName);
	static std::string            roomSendRemove      (uint32 sequenceId);

	static void                   uninviteAdd         (uint32 sequenceId, const uint32 & roomId, const std::string & avatarName);
	static IntStringPair          uninviteRemove      (uint32 sequenceId);

	static void                   addModeratorAdd            (uint32 sequenceId, const uint32 & roomId, const std::string & avatarName);
	static IntStringPair          addModeratorRemove         (uint32 sequenceId);

	static void                   removeModeratorAdd         (uint32 sequenceId, const uint32 & roomId, const std::string & avatarName);
	static IntStringPair          removeModeratorRemove      (uint32 sequenceId);
};

//======================================================================

#endif
