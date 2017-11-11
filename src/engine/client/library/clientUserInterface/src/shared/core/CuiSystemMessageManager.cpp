//======================================================================
//
// CuiSystemMessageManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSystemMessageManager.h"

#include "StringId.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientUserInterface/CuiSystemMessageManagerData.h"
#include "sharedGame/OutOfBand.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ChatSystemMessage.h"
#include "sharedObject/NetworkIdManager.h"

#include <vector>

//======================================================================

namespace
{
	namespace Transceivers
	{		
		MessageDispatch::Transceiver<const CuiSystemMessageManager::Messages::Received::Payload &,    CuiSystemMessageManager::Messages::Received>       received;
	}
	
	namespace MessageNames
	{
		const char * const ChatSystemMessage                 = "ChatSystemMessage";
	}

	//----------------------------------------------------------------------

	class Listener : public MessageDispatch::Receiver
	{
	public:

		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (MessageNames::ChatSystemMessage);
		}

		void receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			//----------------------------------------------------------------------

			if (message.isType (MessageNames::ChatSystemMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();
				const ChatSystemMessage msg (ri);
				CuiSystemMessageManager::receiveSystemMessage (msg);	
			}
		}
	};

	//----------------------------------------------------------------------

	Listener * s_listener = 0;

	bool s_installed = false;
}

//----------------------------------------------------------------------

void CuiSystemMessageManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed"));

	s_listener = new Listener;
	s_installed = true;
}

//----------------------------------------------------------------------

void CuiSystemMessageManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	delete s_listener;
	s_listener = 0;

	s_installed = false;
}

//----------------------------------------------------------------------

void CuiSystemMessageManager::receiveSystemMessage (const ChatSystemMessage & msg)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	
	Data data (msg.getFlags (), msg.getMessage (), msg.getOutOfBand ());

	if (!msg.getOutOfBand ().empty ())
		ProsePackageManagerClient::appendAllProsePackages (msg.getOutOfBand (), data.translated);

	Transceivers::received.emitMessage (data);
}

//----------------------------------------------------------------------

void CuiSystemMessageManager::sendFakeSystemMessage (const Unicode::String & msg, bool chatBoxOnly)
{
	//this function can safely be called when the system is not installed (it will just return trivially)
	if(!s_installed)
		return;

	int flags = 0;
	if (chatBoxOnly)
		flags |= ChatSystemMessage::F_chatBoxOnly;

	receiveSystemMessage (ChatSystemMessage (static_cast<uint8>(flags), msg, Unicode::emptyString));
}

//======================================================================
