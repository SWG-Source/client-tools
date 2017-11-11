//======================================================================
//
// CuiPrelocalizedChatMessage.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiPrelocalizedChatMessage_H
#define INCLUDED_CuiPrelocalizedChatMessage_H

//======================================================================

#include "Unicode.h"

//----------------------------------------------------------------------

class CuiPrelocalizedChatMessage
{
public:

	uint32          sequence;
	uint32          roomId;
	Unicode::String str;
	
	CuiPrelocalizedChatMessage (uint32 _sequence, uint32 _roomId, const Unicode::String _str);
	CuiPrelocalizedChatMessage ();
};

//======================================================================

#endif