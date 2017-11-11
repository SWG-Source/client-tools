//======================================================================
//
// CuiChatRoomManagerStatusMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiChatRoomManagerStatusMessage_H
#define INCLUDED_CuiChatRoomManagerStatusMessage_H

//======================================================================

#include "Unicode.h"

//----------------------------------------------------------------------

class CuiChatRoomManagerStatusMessage
{
public:

	uint32          sequence;
	uint32          roomId;
	Unicode::String str;
	
	CuiChatRoomManagerStatusMessage (uint32 _sequence, uint32 _roomId, const Unicode::String _str);
	CuiChatRoomManagerStatusMessage ();
};

//======================================================================

#endif
