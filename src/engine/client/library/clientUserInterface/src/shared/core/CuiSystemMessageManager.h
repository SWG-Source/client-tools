
//======================================================================
//
// CuiSystemMessageManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiSystemMessageManager_H
#define INCLUDED_CuiSystemMessageManager_H

//======================================================================

class ChatSystemMessage;
class CuiSystemMessageManagerData;

//----------------------------------------------------------------------

class CuiSystemMessageManager
{
public:

	typedef CuiSystemMessageManagerData Data;

	struct Messages
	{
		struct Received
		{
			typedef Data Payload;
		};
	};

	static void                install ();
	static void                remove ();

	static void                receiveSystemMessage (const ChatSystemMessage & msg);

	static void                sendFakeSystemMessage (const Unicode::String & msg, bool chatBoxOnly = false);
};

//======================================================================

#endif


