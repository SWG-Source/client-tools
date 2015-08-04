//======================================================================
//
// CuiPersistentMessageManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"

#include "UnicodeUtils.h"
#include "UIUtils.h"
#include "clientGame/AttachmentDAta.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GroupManager.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiStringIdsPersistentMessage.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "fileInterface/StdioFile.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/GuildRankDataTable.h"
#include "sharedGame/OutOfBand.h"
#include "sharedGame/OutOfBandBase.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/TextManager.h"
#include "sharedGame/Waypoint.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ChatDeleteAllPersistentMessages.h"
#include "sharedNetworkMessages/ChatDeletePersistentMessage.h"
#include "sharedNetworkMessages/ChatEnum.h"
#include "sharedNetworkMessages/ChatOnDeleteAllPersistentMessages.h"
#include "sharedNetworkMessages/ChatOnSendPersistentMessage.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClient.h"
#include "sharedNetworkMessages/ChatPersistentMessageToServer.h"
#include "sharedNetworkMessages/ChatRequestPersistentMessage.h"
#include "sharedObject/Container.h"
#include "unicodeArchive/UnicodeArchive.h"

#include <map>

//======================================================================

namespace CuiPersistentMessageManagerNamespace
{
	CuiMessageBox *s_emptyMailBox = 0;

	typedef CuiPersistentMessageManager::Data Data;

	typedef CuiPersistentMessageManager::Attachments    Attachments;
	typedef CuiPersistentMessageManager::AttachmentMap  AttachmentMap;

	bool s_debugOutput = false;

	//----------------------------------------------------------------------

	struct MessageInfo
	{
		std::string     name;
		Unicode::String subject;
		Unicode::String body;
		Unicode::String oob;
		float       expire_time;

		static const float EXPIRE_TIME_LIMIT;
	};

	//-- one minute expire time
	const float MessageInfo::EXPIRE_TIME_LIMIT = 1000.0f * 60.0f;

	typedef std::map<uint32, MessageInfo> MessageInfoMap;
	MessageInfoMap s_messageInfoMap;

	//----------------------------------------------------------------------

	typedef std::map<uint32, Data> DataMap;
	DataMap s_dataMap;

	AttachmentMap s_attachmentMap;

	bool s_waitingForBodiesToWriteEmailsToDisk = false;

	//----------------------------------------------------------------------

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiPersistentMessageManager::Messages::HeaderReceived::Payload &,  CuiPersistentMessageManager::Messages::HeaderReceived > 
			s_headerReceived;
		MessageDispatch::Transceiver<const CuiPersistentMessageManager::Messages::BodyReceived::Payload &,    CuiPersistentMessageManager::Messages::BodyReceived > 
			s_bodyReceived;
		MessageDispatch::Transceiver<const CuiPersistentMessageManager::Messages::MessageFailed::Payload &,   CuiPersistentMessageManager::Messages::MessageFailed > 
			s_messageFailed;
		MessageDispatch::Transceiver<const CuiPersistentMessageManager::Messages::AttachObject::Payload &,    CuiPersistentMessageManager::Messages::AttachObject > 
			s_attachObject;
		MessageDispatch::Transceiver<const CuiPersistentMessageManager::Messages::StartComposing::Payload &,  CuiPersistentMessageManager::Messages::StartComposing > 
			startComposing;
		MessageDispatch::Transceiver<const CuiPersistentMessageManager::Messages::EmailListChanged::Payload &,  CuiPersistentMessageManager::Messages::EmailListChanged > 
			s_emailListChanged;
	};

	//----------------------------------------------------------------------

	namespace MessageNames
	{
		const char * const ChatPersistentMessageToClient = "ChatPersistentMessageToClient";
		const char * const ChatOnSendPersistentMessage = "ChatOnSendPersistentMessage";
		const char * const ChatOnDeleteAllPersistentMessages = "ChatOnDeleteAllPersistentMessages";
	};

	//----------------------------------------------------------------------

	class Listener : public MessageDispatch::Receiver
	{
	public:

		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (MessageNames::ChatPersistentMessageToClient);
			connectToMessage (MessageNames::ChatPersistentMessageToClient);
			connectToMessage (MessageNames::ChatOnDeleteAllPersistentMessages);
		}

		virtual void receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			if (message.isType (MessageNames::ChatOnDeleteAllPersistentMessages))
			{
				// This is a hack since I get 3 response messages from the ChatServer for this. When
				// the Chat API allows a single call to delete all mail, we can fix this.

				static int count = 0;
				if (count % 3 == 0)
				{
					Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();

					const ChatOnDeleteAllPersistentMessages msg (ri);

					CuiStringVariablesData data;
					data.targetName = Unicode::narrowToWide(msg.getTargetName());
					Unicode::String resultMessage;

					if (msg.isSuccess())
					{
						CuiStringVariablesManager::process(CuiStringIdsPersistentMessage::delete_all_mail_success, data, resultMessage);
					}
					else
					{
						CuiStringVariablesManager::process(CuiStringIdsPersistentMessage::delete_all_mail_fail, data, resultMessage);
					}

					CuiSystemMessageManager::sendFakeSystemMessage(resultMessage);
				}
				++count;
			}
			if (message.isType (MessageNames::ChatPersistentMessageToClient))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();

				const ChatPersistentMessageToClient msg (ri);
				CuiPersistentMessageManager::receiveMessage (msg.getData ());
			}
			else if (message.isType (MessageNames::ChatOnSendPersistentMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnSendPersistentMessage msg (ri);

				int chatResult   = static_cast<int>(msg.getResult());

				if ((chatResult == CHATRESULT_SUCCESS) || (chatResult == ERR_IGNORING))
				{
					return;
				}

				const uint32 sequenceId = msg.getSequence ();
				const MessageInfoMap::const_iterator it = s_messageInfoMap.find (sequenceId);

				Unicode::String result;

				if (it != s_messageInfoMap.end ())
				{
					const MessageInfo & info = (*it).second;

					if (chatResult == ERR_DESTAVATARDOESNTEXIST)
					{
						CuiStringVariablesManager::process (CuiStringIdsPersistentMessage::recipient_invalid_prose, Unicode::emptyString, Unicode::narrowToWide (info.name), Unicode::emptyString, result);

						Unicode::String subject = StringId::decodeString (info.subject);
						const Unicode::String & prefix = CuiStringIdsPersistentMessage::returned_mail_prefix.localize ();

						//-- add the prefix if needed
						if (subject.compare (0, prefix.size (), prefix) != 0)
							subject = prefix + subject;

						CuiPersistentMessageManager::startComposingNewMessage (info.name, subject, info.body, info.oob);
					}
					else
						CuiStringVariablesManager::process (CuiStringIdsPersistentMessage::failed_unknown_prose,    Unicode::emptyString, Unicode::narrowToWide (info.name), Unicode::emptyString, result);
				}
				else
				{
					CuiStringIdsPersistentMessage::failed_unknown.localize (result);
				}

				Transceivers::s_messageFailed.emitMessage (result);

				CuiMessageBox::createInfoBox (result);
			}
		}
	};

	void emptyMail();
	void deleteAttachmentsForId(int32 id);
	void deleteAttachments(Attachments & attachments);

	class MyCallback : public MessageDispatch::Callback
	{
	public:

		MyCallback () :
		MessageDispatch::Callback ()
		{
		}

		void onMessageBoxClosed (const CuiMessageBox & box)
		{
			if (&box == s_emptyMailBox)
			{
				if (s_emptyMailBox->completedAffirmative())
				{
					emptyMail();
				}

				s_emptyMailBox = 0;
			}
		}
	};

	MyCallback *s_callback = 0;
	Listener * s_listener  = 0;
	bool       s_installed = false;

	typedef std::vector<uint32> MessageList;
	MessageList s_requestMessageBodyList;
	MessageList s_requestMessageDeleteList;

	float s_requestMessageBodyTimer = 0.0f;
	float s_requestMessageDeleteTimer = 0.0f;
	float s_emailSoundTimer = 0.0f;

	// list of predefined recipients
	std::set<std::string> s_predefinedRecipients;
}

using namespace CuiPersistentMessageManagerNamespace;

//======================================================================
//
// CuiPersistentMessageManagerNamespace
//
//======================================================================

//----------------------------------------------------------------------

void CuiPersistentMessageManagerNamespace::emptyMail()
{
	if (Game::getPlayer() != NULL)
	{
		ChatDeleteAllPersistentMessages msg(Game::getPlayer()->getNetworkId(), Game::getPlayer()->getNetworkId());
		GameNetwork::send (msg, true);

		// Clear all email and attachments

		AttachmentMap::iterator iterAttachmentMap = s_attachmentMap.begin();

		for (; iterAttachmentMap != s_attachmentMap.end(); ++iterAttachmentMap)
		{
			const int32 id = iterAttachmentMap->first;
			deleteAttachmentsForId(id);
		}

		s_dataMap.clear();
		s_attachmentMap.clear();

		Transceivers::s_emailListChanged.emitMessage(true);
	}
}

//----------------------------------------------------------------------

void CuiPersistentMessageManagerNamespace::deleteAttachmentsForId (int32 id)
{
	AttachmentMap::iterator it = s_attachmentMap.find (id);
	if (it != s_attachmentMap.end ())
	{
		Attachments & oldAttachments = (*it).second;
		deleteAttachments (oldAttachments);
		s_attachmentMap.erase (it);
	}
}

//----------------------------------------------------------------------

void CuiPersistentMessageManagerNamespace::deleteAttachments (Attachments & attachments)
{
	std::for_each (attachments.begin (), attachments.end (), PointerDeleter ());
	attachments.clear ();
}

//======================================================================
//
// CuiPersistentMessageManager
//
//======================================================================

//----------------------------------------------------------------------

bool CuiPersistentMessageManager::ms_hasNewMail = false;
int  CuiPersistentMessageManager::ms_compositionWindowCount = 0;

//----------------------------------------------------------------------

void CuiPersistentMessageManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));
	s_listener  = new Listener;
	s_callback = new MyCallback;

	// build list of predefined recipients
	s_predefinedRecipients.insert(std::string("citizens"));
	s_predefinedRecipients.insert(std::string("group"));
	s_predefinedRecipients.insert(std::string("guild"));

	std::vector<GuildRankDataTable::GuildRank const *> const & allGuildRanks = GuildRankDataTable::getAllRanks();
	for (std::vector<GuildRankDataTable::GuildRank const *>::const_iterator iter = allGuildRanks.begin(); iter != allGuildRanks.end(); ++iter)
		s_predefinedRecipients.insert(Unicode::toLower((*iter)->displayName));

	s_installed = true;

	DebugFlags::registerFlag(s_debugOutput,       "ClientUserInterface", "pmDebugOutput");
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	clear ();

	delete s_listener;
	s_listener  = 0;
	delete s_callback;
	s_callback = 0;
	s_installed = false;
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::clear ()
{
	REPORT_LOG_PRINT (s_debugOutput, ("---> CuiPersistentMessageManager::clear () %d messages.\n", s_dataMap.size ()));

	for (AttachmentMap::iterator it = s_attachmentMap.begin (); it != s_attachmentMap.end (); ++it)
	{
		Attachments & oldAttachments = (*it).second;
		deleteAttachments (oldAttachments);
	}

	s_attachmentMap.clear ();
	s_dataMap.clear ();

	s_waitingForBodiesToWriteEmailsToDisk = false;

	checkNewMailStatus (false);
}

//----------------------------------------------------------------------

const CuiPersistentMessageManager::Data * CuiPersistentMessageManager::getData  (uint32 id)
{
	DataMap::const_iterator it = s_dataMap.find (id);

	if (it != s_dataMap.end ())
		return &(*it).second;

	return 0;
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::getDataVector (DataVector & dv)
{
	dv.clear ();
	dv.reserve (s_dataMap.size ());

	for (DataMap::const_iterator it = s_dataMap.begin (); it != s_dataMap.end (); ++it)
		dv.push_back ((*it).second);
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::sendMessage (const std::string & targetName, const Unicode::String & subject, const Unicode::String & body, const Unicode::String & outOfBand)
{
	static uint32 sequence = 0;

	if (Game::getSinglePlayer ())
	{
		Data data;
		const ChatAvatarId & selfId = CuiChatManager::getSelfAvatarId ();

		data.fromCharacterName = selfId.name;
		data.fromGameCode      = selfId.gameCode;
		data.fromServerCode    = selfId.cluster;
		data.id                = ++sequence;
		data.isHeader          = true;
		data.subject           = subject;
		data.status            = 'N';
		data.timeStamp         = sequence * 10;

		CuiPersistentMessageManager::receiveMessage (data);

		data.isHeader          = false;
		data.message           = body;
		data.outOfBand         = outOfBand;
		data.subject           = subject;
		data.status            = 'R';
		
		CuiPersistentMessageManager::receiveMessage (data); // receive data twice, first for header, then for body/attachments
	}
	
	else
	{
		if (Game::isPlayerSquelched())
			return;

		ChatPersistentMessageToServer msg (++sequence, ChatAvatarId (targetName), subject, body, outOfBand);
		GameNetwork::send (msg, true);
		
		MessageInfo info;
		info.name                  = targetName;
		info.subject               = subject;
		info.body                  = body;
		info.oob                   = outOfBand;
		const float currentTime    = Game::getElapsedTime ();
		info.expire_time           = currentTime + MessageInfo::EXPIRE_TIME_LIMIT;
		s_messageInfoMap[sequence] = info;
		
		for (MessageInfoMap::iterator it = s_messageInfoMap.begin (); it != s_messageInfoMap.end (); )
		{
			const MessageInfo & old_info = (*it).second;
			if (old_info.expire_time < currentTime)
				s_messageInfoMap.erase (it++);
			else
				++it;
		}
	}
}

//----------------------------------------------------------------------

std::set<std::string> const & CuiPersistentMessageManager::getPredefinedRecipients()
{
	return s_predefinedRecipients;
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::sendMessageToMultiple (const std::string & recepients,  const Unicode::String & subject, const Unicode::String & body, const Unicode::String & outOfBand)
{
	StringVector sv;
	if (parseRecepientsString (recepients, sv))
		sendMessage (sv, subject, body, outOfBand);
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::sendMessage (const StringVector & recepients, const Unicode::String & subject, const Unicode::String & body, const Unicode::String & outOfBand)
{
	for (StringVector::const_iterator it = recepients.begin (); it != recepients.end (); ++it)
		sendMessage (*it, subject, body, outOfBand);
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::receiveMessage (const Data & data)
{
	const std::pair<DataMap::iterator,bool> retval = s_dataMap.insert (std::make_pair (data.id, data));
	
	Data & localData = (*(retval.first)).second;
	
	if (retval.second)
	{
		//-- we received a body with no header.  cancel
		if (!data.isHeader)
		{
			s_dataMap.erase (retval.first);
			return;
		}

		processData (localData);

		if (Game::isProfanityFiltered())
		{
			localData.subject = TextManager::filterText(localData.subject);
			localData.message = TextManager::filterText(localData.message);
		}

		REPORT_LOG_PRINT (s_debugOutput, ("---> CuiPersistentMessageManager received new message [%6d] status [%c,%d] isHeader [%d] [%s]\n", data.id, data.status, data.status, data.isHeader ? 1 : 0, Unicode::wideToNarrow (localData.subject).c_str ()));
		Transceivers::s_headerReceived.emitMessage (localData);

		if (localData.status == 'N')
		{
			setHasNewMail (true);

			if (s_emailSoundTimer <= 0.0f)
			{
				s_emailSoundTimer = 5.0f;
				CuiSoundManager::play (CuiSounds::incoming_mail);
			}
		}

		if (s_waitingForBodiesToWriteEmailsToDisk)
			requestMessageBody (localData.id);
	}
	else if (!data.isHeader)
	{
		localData.isHeader  = data.isHeader;
		localData.message   = data.message;
		localData.outOfBand = data.outOfBand;
		localData.status    = data.status;

		//-- hack until we figure out why the server is sending us 'N' and 'U' when we read new messages
		if (localData.status == 'N' || localData.status == 'U')
			localData.status = 'R';

		processData (localData);

		if (Game::isProfanityFiltered())
		{
			localData.subject = TextManager::filterText(localData.subject);
			localData.message = TextManager::filterText(localData.message);
		}

		REPORT_LOG_PRINT (s_debugOutput, ("---> CuiPersistentMessageManager received        BODY [%6d] status [%c,%d] [%s]\n", data.id, data.status, data.status, Unicode::wideToNarrow (localData.subject).c_str ()));

		Transceivers::s_bodyReceived.emitMessage (localData);
		checkNewMailStatus (false);

		// Flag that we are ready to receive another message body

		s_requestMessageBodyTimer = 0.0f;
	}
	else
		WARNING (true, ("CuiPersistentMessageManager received old message [%6d] status [%c,%d] isHeader [%d] [%s]", data.id, data.status, data.status, data.isHeader ? 1 : 0, Unicode::wideToNarrow (localData.subject).c_str ()));
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::processData (Data & data)
{
	if (data.isHeader)
	{
		const size_t nullpos = data.subject.find (static_cast<unsigned short>(0), size_t (0));

		if (nullpos != Unicode::String::npos)
		{
			const Unicode::String oob = data.subject.substr (nullpos + 1);
			data.subject = data.subject.substr (0, nullpos);

			//-- subject prose package for pm's is position -2.
			ProsePackageManagerClient::appendAllProsePackages (oob, data.subject, -2, -1);

			Unicode::String sender;
			//-- sender prose package for pm's is position -3.
			ProsePackageManagerClient::appendAllProsePackages (oob, sender, -3, -2);
			
			if (!sender.empty ())
			{
				//-- @todo: localize this properly
				data.fromCharacterName = Unicode::wideToNarrow (sender);
				data.fromGameCode.clear   ();
				data.fromServerCode.clear ();
			}
		}

		return;
	}

	data.message = StringId::decodeString(data.message);

	if (data.outOfBand.empty ())
		return;

	ProsePackageManagerClient::appendAllProsePackages (data.outOfBand, data.message, -1, -1);

	//-- subject prose package for pm's is position -2.
	ProsePackageManagerClient::appendAllProsePackages (data.outOfBand, data.subject, -2, -1);

	const OutOfBandPackager::OutOfBandBaseVector & oobv = OutOfBandPackager::unpack (data.outOfBand);

	Attachments attachments;

	if (!oobv.empty ())
	{
		for (OutOfBandPackager::OutOfBandBaseVector::const_iterator it = oobv.begin (); it != oobv.end (); ++it)
		{
			OutOfBandBase * const oobBase = *it;
			const int position = oobBase->getPosition ();
			const int typeId = oobBase->getTypeId ();
			
			//-- negative 3 is the index for attachments
			if (position == -3)
			{
				if (typeId == OutOfBandPackager::OT_waypointData)
				{
					AttachmentData * const ad = new AttachmentData;
					ad->set (*oobBase);
					attachments.push_back (ad);
				}
			}

			if(typeId == OutOfBandPackager::OT_prosePackage)
			{
				OutOfBand<ProsePackage> * const entry = dynamic_cast<OutOfBand<ProsePackage> * const>(oobBase);
				if(entry)
				{
					delete entry->getObject();
				}
			}

			delete oobBase;
		}
	}

	if (!attachments.empty ())
		setAttachments (data.id, attachments);
	else
		deleteAttachmentsForId (data.id);
}

//----------------------------------------------------------------------

const CuiPersistentMessageManager::Attachments * CuiPersistentMessageManager::getAttachments           (uint32 id)
{
	AttachmentMap::iterator it = s_attachmentMap.find (id);
	if (it != s_attachmentMap.end ())
		return &(*it).second;

	return 0;
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::setAttachments       (int32 id, Attachments & attachments)
{
	AttachmentMap::iterator it = s_attachmentMap.find (id);
	if (it != s_attachmentMap.end ())
	{
		Attachments & oldAttachments = (*it).second;
		deleteAttachments (oldAttachments);
		
		if (!attachments.empty ())
			(*it).second = attachments;
		else
			s_attachmentMap.erase (it);
	}

	if (!attachments.empty ())
		s_attachmentMap [id] = attachments;
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::emptyMail(const bool confirmed)
{
	if (!confirmed)
	{
		if (s_emptyMailBox)
{
			s_emptyMailBox->closeMessageBox();
}

		s_emptyMailBox = CuiMessageBox::createYesNoBox(CuiStringIdsPersistentMessage::confirm_empty_mail.localize());
		NOT_NULL (s_emptyMailBox);

		s_callback->connect(s_emptyMailBox->getTransceiverClosed(), *s_callback, &MyCallback::onMessageBoxClosed);
	}
	else
{
		CuiPersistentMessageManagerNamespace::emptyMail();
	}
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::deleteMessage  (uint32 id)
{
	s_dataMap.erase(id);
	deleteAttachmentsForId(id);

	checkNewMailStatus (false);

	// Queue the message for deletion

	s_requestMessageDeleteList.push_back(id);
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::checkNewMailStatus (bool status)
{
	if (status && !ms_hasNewMail)
	{
		for (DataMap::const_iterator it = s_dataMap.begin (); it != s_dataMap.end (); ++it)
		{
			const Data & data = (*it).second;

			if (data.status == 'N')
			{
				setHasNewMail (true);
				return;
			}				
		}
	}
	else if (!status && ms_hasNewMail)
	{
		for (DataMap::const_iterator it = s_dataMap.begin (); it != s_dataMap.end (); ++it)
		{
			const Data & data = (*it).second;

			if (data.status == 'N')
				return;
		}

		setHasNewMail (false);
	}

	if (s_waitingForBodiesToWriteEmailsToDisk)
	{
		bool readyToFinish = true;

		for (DataMap::const_iterator it = s_dataMap.begin (); it != s_dataMap.end (); ++it)
		{
			const Data & data = (*it).second;
			if (data.isHeader)
			{
				readyToFinish = false;
				break;
			}
		}

		if (readyToFinish)
			finishWritingEmailsToDisk ();
	}
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::requestMessageBody  (uint32 id)
{
	s_requestMessageBodyList.push_back(id);
}

//----------------------------------------------------------------------

bool CuiPersistentMessageManager::validateRecepientsString (const std::string & recepients)
{
	StringVector sv;
	return parseRecepientsString (recepients, sv);
}

//----------------------------------------------------------------------

bool CuiPersistentMessageManager::parseRecepientsString (const std::string & recepients, StringVector & sv)
{
	sv.clear ();

	static const char breakingChars [] = { ';', 0 };

	typedef std::set<std::string> StringSet;
	StringSet stringSet;

	size_t endpos = 0;
	std::string fulltoken;
	std::string token;
	while (Unicode::getFirstToken (recepients, endpos, endpos, fulltoken, breakingChars))
	{
		Unicode::trim(fulltoken);

		// if it's a predefined recipient, use the entire token
		if (s_predefinedRecipients.count(Unicode::toLower(fulltoken)) > 0)
		{
			token = Unicode::toLower(fulltoken);
		}
		else
		{
			size_t full_endpos = 0;
			if (!Unicode::getFirstToken (fulltoken, 0, full_endpos, token))
			{
				WARNING (true, ("CuiPersistentMessageManager bad recipient string [%s]", fulltoken.c_str ()));
				continue;
			}
		}

		if (!_stricmp(token.c_str(), "group"))
		{
			CreatureObject * const player = Game::getPlayerCreature ();
			if (player)
			{
				GroupManager::CreatureSet groupMembers;
				GroupManager::getGroupMembers(*player, groupMembers);
				for (GroupManager::CreatureSet::const_iterator i = groupMembers.begin(); i != groupMembers.end(); ++i)
				{
					CreatureObject * const member = *i;

					if (member->isPlayer())
					{
						size_t pos = 0;
						std::string firstName;
						Unicode::getFirstToken(Unicode::wideToNarrow(member->getObjectName()), pos, pos, firstName);

						const std::string & lowerFirstName = Unicode::toLower (firstName);

						//-- don't allow duplicates in mail
						if (stringSet.find (lowerFirstName) == stringSet.end ())
						{
							stringSet.insert (lowerFirstName);
							sv.push_back(firstName);
						}
					}
				}
			}
		}
		else
		{
			const std::string & lowerFirstName = Unicode::toLower (token);
			//-- don't allow duplicates in mail
			if (stringSet.find (lowerFirstName) == stringSet.end ())
			{
				stringSet.insert (lowerFirstName);				
				sv.push_back (token);
			}
		}

		if (endpos == recepients.npos)
			break;

		++endpos;
	}

	return !sv.empty ();
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::setHasNewMail (bool b)
{
	ms_hasNewMail = b;
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::saveToDatapad   (AttachmentData & ad)
{
	const int type = ad.getType ();
	switch (type)
	{
	case AttachmentData::T_waypoint:
		{
			WaypointDataBase const * wd = ad.getWaypointData ();
			NOT_NULL (wd);
			if (CuiPersistentMessageManager::checkDatapadSaveOk (*wd))
			{
				ClientWaypointObject::requestWaypoint (*wd);
			}
		}
		break;
	}
}

//----------------------------------------------------------------------

bool CuiPersistentMessageManager::checkDatapadSaveOk (const WaypointDataBase & wd)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return false;
	
	ClientObject * const datapad = player->getDatapadObject ();
	if (!datapad)
		return false;
	
	Container * const container = ContainerInterface::getContainer (*datapad);
	
	if (!container)
		return false;
	
	for (ContainerIterator it = container->begin (); it != container->end (); ++it)
	{
		const CachedNetworkId & id = *it;
		const ClientWaypointObject * const waypoint = dynamic_cast<const ClientWaypointObject *>(id.getObject ());
		
		if (waypoint)
		{
			if (waypoint->equals (wd))
			{
				CuiMessageBox::createInfoBox (CuiStringIdsPersistentMessage::err_waypoint_matches.localize ());
				return false;
			}
		}
	}
	
	return true;
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::setCompositionWindowCount   (int count)
{
	ms_compositionWindowCount = count;
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::attachObjectToComposeWindow (ClientObject & obj)
{
	Transceivers::s_attachObject.emitMessage (obj);
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::startComposingNewMessage    (const std::string & recipients, const Unicode::String & subject, const Unicode::String & body, const Unicode::String & oob)
{
	Data data;
	data.fromCharacterName = recipients;
	data.subject           = subject;
	data.message           = body;
	data.outOfBand         = oob;

	Transceivers::startComposing.emitMessage (data);
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::startWritingEmailsToDisk ()
{
	s_waitingForBodiesToWriteEmailsToDisk = false;
	int requestCount = 0;

	for (DataMap::const_iterator it = s_dataMap.begin (); it != s_dataMap.end (); ++it)
	{
		const uint32 id   = (*it).first;
		const Data & data = (*it).second;

		if (data.isHeader)
		{
			requestMessageBody (id);
			++requestCount;
		}
	}

	if (requestCount)
	{
		s_waitingForBodiesToWriteEmailsToDisk = true;

		CuiStringVariablesData data;
		data.digit_i = requestCount;
		Unicode::String messageBodiesRequested;
		CuiStringVariablesManager::process(CuiStringIdsPersistentMessage::message_bodies_requested, data, messageBodiesRequested);
		CuiChatRoomManager::sendPrelocalizedChat(messageBodiesRequested);
	}
	else 
		finishWritingEmailsToDisk ();
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::finishWritingEmailsToDisk ()
{
	s_waitingForBodiesToWriteEmailsToDisk = false;

	std::string str;
	char buf [128];
	const size_t buf_size = sizeof (buf);
	
	std::string fileNamePrefix;

	StdioFileFactory sff;

	{
		std::string loginId;
		std::string clusterName;
		Unicode::String playerName;
		NetworkId networkId;

		if (!Game::getPlayerPath (loginId, clusterName, playerName, networkId))
			return;

		fileNamePrefix = std::string ("profiles/") + loginId + "/" + clusterName + "/" + "mail_" + Unicode::wideToNarrow (playerName) + "/";

		Os::createDirectories (fileNamePrefix.c_str ());
	}

	int count = 0;
	int errors = 0;

	for (DataMap::const_iterator it = s_dataMap.begin (); it != s_dataMap.end (); ++it)
	{
		const uint32 id   = (*it).first;
		const Data & data = (*it).second;

		str.clear ();

		snprintf (buf, buf_size, "%08d", id);
		str += buf;
		str.push_back ('\n');

		const std::string & filename = fileNamePrefix + buf + "." + "mail";

		str += data.fromGameCode;
		str.push_back ('.');
		str += data.fromServerCode;
		str.push_back ('.');
		str += data.fromCharacterName;
		str.push_back ('\n');

		str += Unicode::wideToNarrow (StringId::decodeString (data.subject));
		str.push_back ('\n');

		snprintf (buf, buf_size, "TIMESTAMP: %d", data.timeStamp);
		str += buf;
		str.push_back ('\n');

		str += Unicode::wideToNarrow (data.message);

		AbstractFile * const f = sff.createFile (filename.c_str (), "w");
		if (f)
		{
			if (f->isOpen ())
			{
				f->write (str.size (), str.data ());
				++count;
			}
			else
				++errors;
			delete f;
		}
		else
			++errors;
	}
	
	CuiStringVariablesData data;
	data.targetName = Unicode::narrowToWide(fileNamePrefix);
	data.digit_i = count;
	UIUtils::FormatInteger (data.sourceName, errors);

	Unicode::String saveSucceeded;
	CuiStringVariablesManager::process(CuiStringIdsPersistentMessage::save_succeeded, data, saveSucceeded);

	CuiChatRoomManager::sendPrelocalizedChat(saveSucceeded);
}

//----------------------------------------------------------------------

void CuiPersistentMessageManager::update(float const deltaTime)
{
	s_requestMessageBodyTimer -= deltaTime;
	s_requestMessageDeleteTimer -= deltaTime;
	s_emailSoundTimer -= deltaTime;

	// See if we need to request any message bodies

	if (!s_requestMessageBodyList.empty())
	{
		if (s_requestMessageBodyTimer <= 0.0f)
		{
			s_requestMessageBodyTimer = 5.0f;

			uint32 const id = s_requestMessageBodyList.back();
			s_requestMessageBodyList.pop_back();

			// Make sure this email is still outstanding

			DataMap::const_iterator iterDataMap = s_dataMap.find(id);

			if (iterDataMap != s_dataMap.end())
			{
				if (iterDataMap->second.isHeader)
				{
					const ChatRequestPersistentMessage msg (0, id);
					GameNetwork::send (msg, true);
				}
			}
		}
	}

	// See if we need to delete any messages

	if (!s_requestMessageDeleteList.empty())
	{
		if (s_requestMessageDeleteTimer <= 0.0f)
		{
			s_requestMessageDeleteTimer = 0.1f;
			uint32 const id = s_requestMessageDeleteList.back();
			s_requestMessageDeleteList.pop_back();

			const ChatDeletePersistentMessage msg(id);
			GameNetwork::send(msg, true);
	
			REPORT_LOG_PRINT(s_debugOutput, ("---> CuiPersistentMessageManager::PM deleted(%d)\n", id));
		}
	}
}

//======================================================================
