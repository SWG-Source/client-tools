//======================================================================
//
// CuiInstantMessageManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiInstantMessageManager.h"

#include "UnicodeUtils.h"
#include "clientGame/AwayFromKeyBoardManager.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ShipObject.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiStringIdsAwayFromKeyBoard.h"
#include "clientUserInterface/CuiStringIdsInstantMessage.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ChatEnum.h"
#include "sharedNetworkMessages/ChatInstantMessageToCharacter.h"
#include "sharedNetworkMessages/ChatInstantMessageToClient.h"
#include "sharedNetworkMessages/ChatOnSendInstantMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

#include <map>

//======================================================================

const Unicode::String CuiInstantMessageManager::ms_cmdTell           = Unicode::narrowToWide ("tell");
const Unicode::String CuiInstantMessageManager::ms_cmdTargetedTell   = Unicode::narrowToWide ("ttell");
const Unicode::String CuiInstantMessageManager::ms_cmdSend           = Unicode::narrowToWide ("send");
const Unicode::String CuiInstantMessageManager::ms_cmdRetell         = Unicode::narrowToWide ("retell");
const Unicode::String CuiInstantMessageManager::ms_cmdReply          = Unicode::narrowToWide ("reply");

//----------------------------------------------------------------------

namespace
{
	struct MessageInfo
	{
		ChatAvatarId    avatarId;
		float           expire_time;
		Unicode::String msg;
		bool            wasReply;

		static const float EXPIRE_TIME_LIMIT;
	};

	//-- one minute expire time
	const float MessageInfo::EXPIRE_TIME_LIMIT = 1000.0f * 60.0f;

	typedef std::map<uint32, MessageInfo> MessageInfoMap;
	MessageInfoMap s_messageInfoMap;

	//----------------------------------------------------------------------

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiInstantMessageManager::Messages::MessageReceived::Payload &, CuiInstantMessageManager::Messages::MessageReceived > 
			s_messageReceived;
		MessageDispatch::Transceiver<const CuiInstantMessageManager::Messages::MessageFailed::Payload &, CuiInstantMessageManager::Messages::MessageFailed > 
			s_messageFailed;
	};

	//----------------------------------------------------------------------

	namespace MessageNames
	{
		const char * const ChatInstantMessageToClient = "ChatInstantMessageToClient";
		const char * const ChatOnSendInstantMessage   = "ChatOnSendInstantMessage";
	};

	//----------------------------------------------------------------------

	class Listener : public MessageDispatch::Receiver
	{
	public:

		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (MessageNames::ChatInstantMessageToClient);
			connectToMessage (MessageNames::ChatOnSendInstantMessage);
		}

		virtual void receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			if (message.isType (MessageNames::ChatInstantMessageToClient))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();

				const ChatInstantMessageToClient msg (ri);
				CuiInstantMessageManager::receiveMessage (msg.getFromName (), msg.getMessage ());
			}
			else if (message.isType (MessageNames::ChatOnSendInstantMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnSendInstantMessage msg (ri);

				const uint32 sequenceId = msg.getSequence ();
				int chatResult   = static_cast<int>(msg.getResult());

				const MessageInfoMap::iterator it = s_messageInfoMap.find (sequenceId);

				Unicode::String result;

				if (it != s_messageInfoMap.end ())
				{
					const MessageInfo & info = (*it).second;

					if (chatResult == ERR_DESTAVATARDOESNTEXIST)
						result += CuiChatManager::prosify (info.avatarId, CuiStringIdsInstantMessage::im_recipient_offline_prose);
					else if (chatResult == ERR_ROOM_UNKNOWNFAILURE)
						result += CuiChatManager::prosify (info.avatarId, CuiStringIdsInstantMessage::im_recipient_ignored_prose);
					else if (chatResult == ERR_DESTAVATARDOESNTEXIST)
						result += CuiChatManager::prosify (info.avatarId, CuiStringIdsInstantMessage::im_recipient_invalid_prose);
					else if (chatResult == ERR_ROOM_NOPRIVILEGES)
						result += CuiChatManager::prosify (info.avatarId, CuiStringIdsInstantMessage::im_failed_insufficient_privs_prose);
					else if (chatResult == CHATRESULT_SUCCESS)
					{
						if (info.wasReply)
							result += CuiChatManager::prosify (info.avatarId, info.msg, CuiStringIdsInstantMessage::im_success_reply_prose);
						else
							result += CuiChatManager::prosify (info.avatarId, info.msg, CuiStringIdsInstantMessage::im_success_prose);
					}
					else
						result += CuiChatManager::prosify (info.avatarId, CuiStringIdsInstantMessage::im_failed_unknown_prose);

					s_messageInfoMap.erase (it);
				}
				else
				{
					result += CuiChatManager::prosify (ChatAvatarId ("UKNOWN"), CuiStringIdsInstantMessage::im_failed_unknown_prose);
				}

				Transceivers::s_messageFailed.emitMessage (result);
			}
		}
	};

	Listener * s_listener = 0;

	//----------------------------------------------------------------------

	namespace LastMessage
	{
		CuiInstantMessageManagerElement incoming;
		CuiInstantMessageManagerElement outgoing;
	}

	//----------------------------------------------------------------------

	namespace TargetDelimiters
	{
		const Unicode::unicode_char_t targetSeparator [2] = { ',', 0};
	};

	//----------------------------------------------------------------------

	typedef std::vector<ChatAvatarId> TargetVector;
	bool getTargets (const Unicode::String & str, TargetVector & targets)
	{

		Unicode::String token;
		size_t endpos = 0;
		size_t startpos = 0;
	
		while (Unicode::getFirstToken  (str, startpos, endpos, token, TargetDelimiters::targetSeparator))
		{			
			ChatAvatarId avatarId;
			if (!CuiChatManager::constructChatAvatarId (Unicode::wideToNarrow (token), avatarId))
				return false;

			targets.push_back (avatarId);

			if (endpos == Unicode::String::npos)
				break;

			startpos = endpos + 1;
		}

		return true;
	}

	bool sendReplyRetell (const CuiInstantMessageManagerElement & elem, const Unicode::String & str, Unicode::String & result)
	{
		const Unicode::String & message = Unicode::getTrim (str);

		if (message.empty ())
		{
			result += CuiStringIdsInstantMessage::im_no_message.localize ();
			return false;
		}
				
		Unicode::String oob;
		CuiInstantMessageManager::sendMessageToCharacter (elem.avatarId, message, oob, true);		

		return true;
	}

	bool s_installed = false;
}

//----------------------------------------------------------------------

void CuiInstantMessageManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));
	s_listener = new Listener;
	s_installed = true;
}

//----------------------------------------------------------------------

void CuiInstantMessageManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	delete s_listener;
	s_listener = 0;
	s_installed = false;
}

//----------------------------------------------------------------------

bool CuiInstantMessageManager::sendMessageToCharacter (const std::string & targetName, const Unicode::String & message, const Unicode::String & outOfBand, bool isReply)
{
	ChatAvatarId avatarId;
	if (CuiChatManager::constructChatAvatarId   (targetName, avatarId))
	{
		sendMessageToCharacter (avatarId, message, outOfBand, isReply);
		return true;
	}
	
	return false;
}

//----------------------------------------------------------------------

void CuiInstantMessageManager::sendMessageToCharacter (const ChatAvatarId & avatarId, const Unicode::String & message, const Unicode::String & outOfBand, bool isReply)
{
	static uint32 last_sequence_id = 0;

	if (Game::isPlayerSquelched())
		return;

	const ChatInstantMessageToCharacter msg (++last_sequence_id, avatarId, message, outOfBand);
	GameNetwork::send (msg, true);
	
	if (!isReply)
	{
		LastMessage::outgoing.avatarId = avatarId;
		LastMessage::outgoing.message  = message;
	}

	MessageInfo info;
	info.avatarId = avatarId;
	info.msg      = message;
	info.wasReply = isReply;

	const float currentTime            = Game::getElapsedTime ();
	info.expire_time                   = currentTime + MessageInfo::EXPIRE_TIME_LIMIT;
	s_messageInfoMap[last_sequence_id] = info;

	for (MessageInfoMap::iterator it = s_messageInfoMap.begin (); it != s_messageInfoMap.end (); )
	{
		const MessageInfo & old_info = (*it).second;
		if (old_info.expire_time < currentTime)
			s_messageInfoMap.erase (it++);
		else
			++it;
	}
	
	if (Game::getSinglePlayer ())
	{
		CuiInstantMessageManager::receiveMessage (avatarId, message);
		Unicode::String result;

		if (isReply)
			result = CuiChatManager::prosify (avatarId, message, CuiStringIdsInstantMessage::im_success_reply_prose);
		else
			result = CuiChatManager::prosify (avatarId, message, CuiStringIdsInstantMessage::im_success_prose);

		Transceivers::s_messageFailed.emitMessage (result);
	}
}

//----------------------------------------------------------------------

void CuiInstantMessageManager::receiveMessage(const ChatAvatarId & avatarId, const Unicode::String & message)
{
	if (CommunityManager::isIgnored(Unicode::narrowToWide(avatarId.name)))
	{
		// The player sending the message is being ignored

		return;
	}

	LastMessage::incoming.avatarId = avatarId;
	LastMessage::incoming.message  = message;

	Transceivers::s_messageReceived.emitMessage (LastMessage::incoming);
	CuiSoundManager::play (CuiSounds::incoming_im);

	// If the player is away from the keyboard, kick a message back to
	// the player that messaged, unless this message is from an "afk
	// auto response".

	if (AwayFromKeyBoardManager::isAwayFromKeyBoard() &&
	    (message.find(CuiStringIdsAwayFromKeyBoard::away_from_keyboard_automatic_response_prefix.localize()) == -1))
	{
		sendMessageToCharacter(avatarId,  AwayFromKeyBoardManager::getAutomaticResponseMessagePrefix() + AwayFromKeyBoardManager::getAutomaticResponseMessage(), Unicode::String());
	}
}

//----------------------------------------------------------------------

bool CuiInstantMessageManager::tell (const Unicode::String & str, Unicode::String & result)
{
	Unicode::String token;
	size_t endpos = 0;
	
	if (!Unicode::getFirstToken  (str, 0, endpos, token))
	{
		result += CuiStringIdsInstantMessage::im_tell_usage.localize ();
		return false;
	}
	
	TargetVector tv;
	if (!getTargets (token, tv))
	{
		result += CuiStringIdsInstantMessage::im_bad_target_format.localize ();
		return false;
	}
	
	if (tv.empty ())
	{
		result += CuiStringIdsInstantMessage::im_no_targets.localize ();
		return false;
	}
	
	if (endpos == str.npos)
	{
		result += CuiStringIdsInstantMessage::im_no_message.localize ();
		return false;
	}
	
	const Unicode::String message (str.substr (endpos + 1));
	
	if (message.empty ())
	{
		result += CuiStringIdsInstantMessage::im_no_message.localize ();
		return false;
	}
	
	std::set<ChatAvatarId> sentTo;
	
	for (TargetVector::const_iterator it = tv.begin (); it != tv.end (); ++it)
	{
		const ChatAvatarId & avatarId = *it;
		
		if (sentTo.find (avatarId) != sentTo.end ())
		{
			result += CuiChatManager::prosify (avatarId, CuiStringIdsInstantMessage::im_message_dupe_discarded_prose);
			result.append (1, '\n');
			continue;
		}
		
		Unicode::String oob;
		sendMessageToCharacter (avatarId, message, oob);			
		sentTo.insert (avatarId);
	}
	
	if (tv.size () == 1)
	{
//		result += CuiChatManager::prosify (tv.front (), CuiStringIdsInstantMessage::im_message_sent_prose);
	}
	else
		result += CuiStringIdsInstantMessage::im_messages_sent.localize ();
	
	return true;
}

//----------------------------------------------------------------------

bool CuiInstantMessageManager::reply (const Unicode::String & str, Unicode::String & result)
{
	if (LastMessage::incoming.avatarId.name.empty ())
	{
		result += CuiStringIdsInstantMessage::im_no_reply_target.localize ();
		return false;
	}
	
	return sendReplyRetell (LastMessage::incoming, str, result);
}

//----------------------------------------------------------------------

bool CuiInstantMessageManager::retell (const Unicode::String & str, Unicode::String & result)
{
	if (LastMessage::outgoing.avatarId.name.empty ())
	{
		result += CuiStringIdsInstantMessage::im_no_retell_target.localize ();
		return false;
	}
	
	return sendReplyRetell (LastMessage::outgoing, str, result);
//	CuiInstantMessageManager::sendMessageToCharacter (LastMessage::outgoing.avatarId, str, Unicode::emptyString);
//	return true;
}

//----------------------------------------------------------------------

bool CuiInstantMessageManager::targetedTell (const Unicode::String & str, Unicode::String & result)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return false;
	
	const CachedNetworkId & target = player->getLookAtTarget ();
	
	const TangibleObject * const targetObject = dynamic_cast<const TangibleObject *>(target.getObject ());

	bool const isPlayer = (targetObject != 0) && (targetObject->isPlayer() || (targetObject->asShipObject() && targetObject->asShipObject()->isPlayerControlled()));
	
	if (!isPlayer)
	{
		result += CuiStringIdsInstantMessage::im_ttell_no_target.localize ();
		return false;
	}
	
	const Unicode::String & message = Unicode::getTrim (str);
	
	if (message.empty ())
	{
		result += CuiStringIdsInstantMessage::im_no_message.localize ();
		return false;
	}
				
	const std::string & name = Unicode::wideToNarrow (targetObject->getLocalizedName ());
	
	std::string firstName;
	size_t dummy = 0;
	if (!Unicode::getFirstToken  (name, 0, dummy, firstName))
		return false;
	
	CuiInstantMessageManager::sendMessageToCharacter (firstName, message, Unicode::emptyString, false);		
	
	return true;
}

//----------------------------------------------------------------------

ChatAvatarId const & CuiInstantMessageManager::getLastChatId()
{
	return LastMessage::incoming.avatarId;
}

//======================================================================
