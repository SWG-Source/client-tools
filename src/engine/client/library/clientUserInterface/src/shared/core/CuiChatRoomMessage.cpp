//======================================================================
//
// CuiChatRoomMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatRoomMessage.h"

//======================================================================

CuiChatRoomMessage::CuiChatRoomMessage () :
roomId  (0),
sender  (),
message (),
oob     ()
{

}

//----------------------------------------------------------------------

CuiChatRoomMessage::CuiChatRoomMessage (uint32 _roomId, const CuiChatAvatarId & _sender, const Unicode::String & _message, const Unicode::String & _oob) :
roomId  (_roomId),
sender  (_sender),
message (_message),
oob     (_oob)
{
}

//======================================================================
