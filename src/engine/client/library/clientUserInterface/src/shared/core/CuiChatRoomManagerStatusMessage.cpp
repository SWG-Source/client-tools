//======================================================================
//
// CuiChatRoomManagerStatusMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatRoomManagerStatusMessage.h"

//======================================================================

CuiChatRoomManagerStatusMessage::CuiChatRoomManagerStatusMessage (uint32 _sequence, uint32 _roomId, const Unicode::String _str) :
sequence (_sequence),
roomId   (_roomId),
str      (_str)
{
}

//----------------------------------------------------------------------

CuiChatRoomManagerStatusMessage::CuiChatRoomManagerStatusMessage () :
sequence (0),
roomId   (0),
str      ()
{
}

//======================================================================

