//======================================================================
//
// CuiPrelocalizedChatMessage.cpp
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiPrelocalizedChatMessage.h"

//======================================================================

CuiPrelocalizedChatMessage::CuiPrelocalizedChatMessage (uint32 _sequence, uint32 _roomId, const Unicode::String _str) :
sequence (_sequence),
roomId   (_roomId),
str      (_str)
{
}

//----------------------------------------------------------------------

CuiPrelocalizedChatMessage::CuiPrelocalizedChatMessage () :
sequence (0),
roomId   (0),
str      ()
{
}

//======================================================================

