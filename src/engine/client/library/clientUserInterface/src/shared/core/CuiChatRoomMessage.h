//======================================================================
//
// CuiChatRoomMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiChatRoomMessage_H
#define INCLUDED_CuiChatRoomMessage_H

#include "clientUserInterface/CuiChatAvatarId.h"

//======================================================================

class CuiChatRoomMessage
{
public:
	uint32           roomId;
	CuiChatAvatarId  sender;
	Unicode::String  message;
	Unicode::String  oob;

	CuiChatRoomMessage ();
	CuiChatRoomMessage (uint32 _roomId, const CuiChatAvatarId & _sender, const Unicode::String & message, const Unicode::String & _oob);
};

//======================================================================

#endif
