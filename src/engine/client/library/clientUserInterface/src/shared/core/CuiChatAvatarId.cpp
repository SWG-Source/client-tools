//======================================================================
//
// CuiChatAvatarId.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatAvatarId.h"

//======================================================================

CuiChatAvatarId::CuiChatAvatarId () :
chatId (),
networkId ()
{
}

//----------------------------------------------------------------------

CuiChatAvatarId::CuiChatAvatarId (const ChatAvatarId & _chatId, const NetworkId & _networkId) :
chatId    (_chatId),
networkId (_networkId)
{
}

//----------------------------------------------------------------------

CuiChatAvatarId::CuiChatAvatarId (const ChatAvatarId & _chatId) :
chatId    (_chatId),
networkId ()
{
}

//----------------------------------------------------------------------

bool CuiChatAvatarId::operator== (const CuiChatAvatarId & rhs) const
{
	return chatId == rhs.chatId;
}

//======================================================================
