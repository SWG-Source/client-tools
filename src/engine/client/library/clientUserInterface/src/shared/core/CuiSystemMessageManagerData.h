//======================================================================
//
// CuiSystemMessageManagerData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiSystemMessageManagerData_H
#define INCLUDED_CuiSystemMessageManagerData_H

//======================================================================

#include "Unicode.h"

//----------------------------------------------------------------------

class CuiSystemMessageManagerData
{
public:

	//This should be kept in sync with ChatSystemMessage::Flags
	enum Flags
	{
		F_personal    = 0x00,
		F_broadcast   = 0x01,
		F_chatBoxOnly = 0x02,
		F_quest       = 0x04
	};

	uint32          flags;
	Unicode::String msg;
	Unicode::String oob;
	Unicode::String translated;

	CuiSystemMessageManagerData (uint32 _flags, const Unicode::String & _msg, const Unicode::String & _oob);
};


//======================================================================

#endif
