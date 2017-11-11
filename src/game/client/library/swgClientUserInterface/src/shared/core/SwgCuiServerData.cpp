//======================================================================
//
// SwgCuiServerData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiServerData.h"

#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedNetworkMessages/WhoListMessage.h"

#include <string>
#include <vector>

//======================================================================

const char * const SwgCuiServerData::Messages::WHO_LIST_UPDATED = "SwgCuiServerData::WHO_LIST_UPDATED";

//----------------------------------------------------------------------

SwgCuiServerData::SwgCuiServerData () :
Singleton<SwgCuiServerData> (),
m_whoList                   (new StringVector),
m_emitter                   (new MessageDispatch::Emitter)
{
	connectToMessage ("ConGenericMessage");

	connectToMessage (WhoListMessage::MESSAGE_TYPE);
}

//----------------------------------------------------------------------

SwgCuiServerData::~SwgCuiServerData ()
{
	delete m_whoList;
	m_whoList = 0;
	delete m_emitter;
	m_emitter = 0;
}

//----------------------------------------------------------------------

void  SwgCuiServerData::receiveMessage (const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	if (message.isType ("ConGenericMessage"))
	{
	}
	
	else if (message.isType (WhoListMessage::MESSAGE_TYPE))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const WhoListMessage whoMsg (ri);
		*m_whoList = whoMsg.getDataArray ();
		m_emitter->emitMessage (MessageDispatch::MessageBase (Messages::WHO_LIST_UPDATED));
	}
}

//======================================================================
