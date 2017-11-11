//======================================================================
//
// CuiChatAvatarId.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiChatAvatarId_H
#define INCLUDED_CuiChatAvatarId_H

#include "sharedNetworkMessages/ChatAvatarId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

//======================================================================

class CuiChatAvatarId
{
public:
	ChatAvatarId chatId;
	NetworkId    networkId;

	CuiChatAvatarId ();
	CuiChatAvatarId (const ChatAvatarId & chatId, const NetworkId & networkId);
	explicit CuiChatAvatarId (const ChatAvatarId & chatId);

	bool operator== (const CuiChatAvatarId & rhs) const;
	bool operator<  (const CuiChatAvatarId & rhs) const
	{
		return chatId < rhs.chatId;
	}
};

//======================================================================

#endif
