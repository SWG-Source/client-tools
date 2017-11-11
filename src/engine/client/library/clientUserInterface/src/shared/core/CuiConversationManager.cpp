//======================================================================
//
// CuiConversationManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiConversationManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientGame/ShipStation.h"
#include "clientUserInterface/CuiAction.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatBubbleManager.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/NpcConversationData.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedRandom/Random.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "UnicodeUtils.h"

//======================================================================

namespace CuiConversationManagerNamespace
{
	namespace Transceivers
	{
		CuiConversationManager::TransceiverTypes::ConsoleOutput    consoleOutput;
		CuiConversationManager::TransceiverTypes::ResponsesChanged responsesChanged;
		CuiConversationManager::TransceiverTypes::TargetChanged    targetChanged;
		CuiConversationManager::TransceiverTypes::ConversationEnded conversationEnded;
	}

	bool s_playerSelectedResponseSinceLastMessage = false;
	bool s_installed = false;
	MessageDispatch::Emitter * s_emitter;

	CachedNetworkId ms_lastTargetId;
	bool s_useDefaultResponse = false;
	bool s_clientOnlyMode = false;
	std::string s_soundEffect;
	float s_clientConvoWindowCloseTimer = 17.0f;
}

using namespace CuiConversationManagerNamespace;

//----------------------------------------------------------------------

class CuiConversationManager::CuiConversationManagerAction : public CuiAction
{
public:
	
	CuiConversationManagerAction () : CuiAction () {}
	
	bool  performAction (const std::string & id, const Unicode::String &) const
	{
		int index = 0;
			
		if (id == CuiActions::conversationResponse0)
			index = 0;
		else if (id == CuiActions::conversationResponse1)
			index = 1;
		else if (id == CuiActions::conversationResponse2)
			index = 2;
		else if (id == CuiActions::conversationResponse3)
			index = 3;
		else if (id == CuiActions::conversationResponse4)
			index = 4;
		else if (id == CuiActions::conversationResponse5)
			index = 5;
		else
			return false;

		CuiConversationManager::respond (CuiConversationManager::getTarget (), index);
			
		return true;
	}
};

//----------------------------------------------------------------------

CuiConversationManager::StringVector                    CuiConversationManager::ms_responses;
Unicode::String                                         CuiConversationManager::ms_lastMessage;
CachedNetworkId                                         CuiConversationManager::ms_targetId;
CuiConversationManager::CuiConversationManagerAction    CuiConversationManager::ms_action;
uint32                                                  CuiConversationManager::ms_appearanceOverrideSharedTemplateCrc;

const char * const CuiConversationManager::Messages::RESPONSES_CHANGED = "CuiConversationManager::RESPONSES_CHANGED";
const char * const CuiConversationManager::Messages::TARGET_CHANGED    = "CuiConversationManager::TARGET_CHANGED";

//----------------------------------------------------------------------

void CuiConversationManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed\n"));
	s_emitter         = new MessageDispatch::Emitter;

	CuiActionManager::addAction (CuiActions::conversationResponse0,      &ms_action, false);
	CuiActionManager::addAction (CuiActions::conversationResponse1,      &ms_action, false);
	CuiActionManager::addAction (CuiActions::conversationResponse2,      &ms_action, false);
	CuiActionManager::addAction (CuiActions::conversationResponse3,      &ms_action, false);
	CuiActionManager::addAction (CuiActions::conversationResponse4,      &ms_action, false);
	CuiActionManager::addAction (CuiActions::conversationResponse5,      &ms_action, false);

	ms_appearanceOverrideSharedTemplateCrc = 0;

	s_useDefaultResponse = false;
	s_clientOnlyMode = false;

	s_installed = true;
}

//----------------------------------------------------------------------

void CuiConversationManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));
	delete s_emitter;
	s_emitter        = 0;

	IGNORE_RETURN (CuiActionManager::removeAction (&ms_action));

	ms_responses.clear ();

	ms_lastMessage.clear ();

	s_installed      = false;
}
	
//----------------------------------------------------------------------

bool CuiConversationManager::respond (const NetworkId & id, int responseNum)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	// if client only, do not send a response.
	if (s_clientOnlyMode) 
	{
		stopClientOnlyConversation();
		return true;
	}

	//-- must not act unless the correct target has been set
	if (id != ms_targetId)
		return false;

	if (!ms_targetId.isValid ())
		return false;

	if (responseNum < 0)
		return false;

	if (responseNum >= static_cast<int>(ms_responses.size ()))
		return false;

	// get the player
	Object *const player = NON_NULL (Game::getPlayer ());
	
	if (!player)
		return false;

	// get the controller
	Controller * const controller = NON_NULL (player->getController());
	
	if (!controller)
		return false;
	
	s_playerSelectedResponseSinceLastMessage = true;

	if (Game::getSinglePlayer ())
	{
		static bool done = false;

		if (done)
		{
			onServerStopConversing(id, StringId("This is the end"), Unicode::emptyString,Unicode::narrowToWide("fin"));
		}
		else
		{
			issueMessage (Unicode::narrowToWide ("This is a response"));
			makeSinglePlayerResponses ();
		}

		done = !done;
	}
	else
	{
		//-- enqueue message
		static const uint32 hash_req = Crc::normalizeAndCalculate("npcConversationSelect");

		char buffer[32];
		sprintf(buffer, "%i", responseNum);
		ClientCommandQueue::enqueueCommand (hash_req, NetworkId::cms_invalid, Unicode::narrowToWide (buffer));
		
		//-- wipe out the responses so the player can't respond multiple ees
		ms_responses.clear ();
		
		s_emitter->emitMessage (MessageDispatch::MessageBase (Messages::RESPONSES_CHANGED));
		Transceivers::responsesChanged.emitMessage (true);
	}

	return true;
}

//----------------------------------------------------------------------

void CuiConversationManager::setResponses (const StringVector & responses, bool useDefaultResponse)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	ms_responses = responses;
	s_useDefaultResponse = useDefaultResponse;

	Unicode::String str;

	int i = 0;

	for (StringVector::iterator it = ms_responses.begin (); it != ms_responses.end (); ++it, ++i)
	{
		Unicode::String & msg = *it;
		const size_t nullpos = msg.find (static_cast<unsigned short>(0));
		
		if (nullpos != Unicode::String::npos)
		{
			Unicode::String tmp = msg.substr (0, nullpos);
			const Unicode::String & oob = msg.substr (nullpos + 1);
			ProsePackageManagerClient::appendAllProsePackages (oob, tmp);
			msg = tmp;
		}

		char buf [64];
		IGNORE_RETURN (_itoa (i, buf, 10));
		str += Unicode::narrowToWide (buf);
		IGNORE_RETURN (str.append (4, ' '));
		str += msg;
		IGNORE_RETURN (str.append (1, '\n'));
	}

	s_emitter->emitMessage (MessageDispatch::MessageBase (Messages::RESPONSES_CHANGED));
	Transceivers::responsesChanged.emitMessage (true);
	Transceivers::consoleOutput.emitMessage (str);
}

//----------------------------------------------------------------------

void CuiConversationManager::getResponses (StringVector & responses)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));
	responses = ms_responses;
}

//----------------------------------------------------------------------

void CuiConversationManager::issueMessage (const Unicode::String & msg)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	WARNING (!ms_targetId.isValid (), ("invalid target in issueMessage\n"));

	const size_t nullpos = msg.find (static_cast<unsigned short>(0));

	if (nullpos != Unicode::String::npos)
	{
		ms_lastMessage = msg.substr (0, nullpos);
		const Unicode::String oob = msg.substr (nullpos + 1);
		ProsePackageManagerClient::appendAllProsePackages (oob, ms_lastMessage);
	}
	else
		ms_lastMessage = msg;

	Object * const player = Game::getPlayer ();
	if (player)
		CuiSpatialChatManager::processMessage (MessageQueueSpatialChat (ms_targetId, player->getNetworkId (), ms_lastMessage, 255, 0, 0, 0, 0, Unicode::emptyString), true, false);

	s_emitter->emitMessage (MessageDispatch::MessageBase (Messages::RESPONSES_CHANGED));
	Transceivers::responsesChanged.emitMessage (true);

	s_playerSelectedResponseSinceLastMessage = false;
}

//----------------------------------------------------------------------

const CachedNetworkId & CuiConversationManager::getTarget ()
{
	return ms_targetId;
}

//----------------------------------------------------------------------

void CuiConversationManager::setTarget (NetworkId const & id, uint32 const appearanceOverrideSharedTemplateCrc, std::string const & soundEffect)
{
	CuiChatBubbleManager::clearBubblesFor(id);

	ms_targetId = id;
	ms_appearanceOverrideSharedTemplateCrc = appearanceOverrideSharedTemplateCrc;
	ms_responses.clear ();
	ms_lastMessage.clear ();
	s_soundEffect = soundEffect;

	s_emitter->emitMessage(MessageDispatch::MessageBase(Messages::TARGET_CHANGED));
	Transceivers::targetChanged.emitMessage (true);
	
	if (Game::isHudSceneTypeSpace()) 
	{
		CuiActionManager::performAction(CuiActions::spaceConversation, Unicode::emptyString);
	}
}

//----------------------------------------------------------------------

uint32 CuiConversationManager::getAppearanceOverrideTemplateCrc ()
{
	return ms_appearanceOverrideSharedTemplateCrc;
}

//----------------------------------------------------------------------

void CuiConversationManager::onServerStopConversing(NetworkId const & id, StringId const & finalText, Unicode::String const & finalProse, Unicode::String const & finalResponse)
{
	//-- must not act unless the correct target has been set
	if (id != ms_targetId && id != ms_lastTargetId)
		return;	

	if (!s_playerSelectedResponseSinceLastMessage)
	{
		Object * const player = Game::getPlayer ();
		if (player)
			CuiSpatialChatManager::processMessage(MessageQueueSpatialChat (ms_targetId, CachedNetworkId (*player), ms_lastMessage, 255, 0, 0, 0, 0, Unicode::emptyString), false, true);
	}

	ms_lastTargetId = NetworkId::cms_invalid;
	s_soundEffect.clear();

	setTarget(NetworkId::cms_invalid, 0, s_soundEffect);

	Transceivers::conversationEnded.emitMessage(true);

	startClientOnlyConversation(id, finalText, finalProse, finalResponse);
}

//----------------------------------------------------------------------

void  CuiConversationManager::start (const NetworkId & id)
{
	stopClientOnlyConversation();

	Object * const player = Game::getPlayer ();

	if (!player)
		return;

	//-- we must force the server to stop conversing
	if (id != ms_targetId && ms_targetId != NetworkId::cms_invalid)
		stop ();
	
	if (Game::getSinglePlayer ())
	{
		setTarget(id, 0, s_soundEffect);
		issueMessage (Unicode::narrowToWide ("This is my message"));
		makeSinglePlayerResponses();
	}
	else
	{
		static const uint32 hash_req = Crc::normalizeAndCalculate("npcConversationStart");

		char buffer[32];
		sprintf(buffer, "%i %s", static_cast<int>(NpcConversationData::CS_Player), "");
		ClientCommandQueue::enqueueCommand (hash_req, id, Unicode::narrowToWide (buffer));
	}
}

//----------------------------------------------------------------------

bool  CuiConversationManager::stop ()
{
	CuiConversationManager::stopClientOnlyConversation();

	Object * const player = Game::getPlayer ();
	
	if (!player)
		return false;
	
	if (!ms_targetId.isValid ())
		return false;

	s_playerSelectedResponseSinceLastMessage = true;

	if (Game::getSinglePlayer ())
	{
		s_soundEffect.clear();
		setTarget (NetworkId::cms_invalid, 0, s_soundEffect);
	}
	else
	{
		ms_lastTargetId = ms_targetId;
		s_soundEffect.clear();
		setTarget (CachedNetworkId::cms_invalid, 0, s_soundEffect);
		static const uint32 hash_req = Crc::normalizeAndCalculate("npcConversationStop");
		ClientCommandQueue::enqueueCommand (hash_req, NetworkId::cms_invalid, Unicode::emptyString);
	}

	Transceivers::conversationEnded.emitMessage(true);

	return true;
}

//----------------------------------------------------------------------

void CuiConversationManager::makeSinglePlayerResponses ()
{
	StringVector sv;

	static int testnum = 0;
	++testnum;
	const int ran = Random::random (1, 5);

	for (int i = 0; i < ran; ++i)
	{
		char buf [64];
		snprintf (buf, sizeof (buf), "test %d response %d", testnum, i);
		sv.push_back (Unicode::narrowToWide (buf));
	}

	setResponses (sv);
}

//----------------------------------------------------------------------

void CuiConversationManager::startClientOnlyConversation(NetworkId const & target, StringId const & message, Unicode::String const & messageoob, Unicode::String const & response)
{
	UNREF(response);

	stopClientOnlyConversation();

	Unicode::String finalMessage;

	if (!messageoob.empty())
	{
		ProsePackageManagerClient::appendAllProsePackages(messageoob, finalMessage);
	}
	else if (message.isValid())
	{
		finalMessage = message.localize();
	}


	if (!finalMessage.empty()) 
	{
#if END_CHAT_IN_DIALOG
		s_clientOnlyMode = true;

		// Make the convo window in single player mode.
		setTarget(target, 0, s_soundEffect);

		// issue the message.
		issueMessage(finalMessage);
		
#if END_CHAT_WITH_RESPONSE
		// Make the response. 
		StringVector localResponse;
		if (!response.empty()) 
		{
			Unicode::String pp;
			ProsePackageManagerClient::appendAllProsePackages(response, pp);
			localResponse.push_back(pp);
		}
		
		// Set response.
		setResponses(localResponse, response.empty());
#endif

#else
		CuiChatBubbleManager::enqueueChat(CachedNetworkId(target), finalMessage.c_str(), 0, 0, 32);
#endif
	}

	// false conversation end = a client only conversation.
	Transceivers::conversationEnded.emitMessage(false);
}

//----------------------------------------------------------------------

void CuiConversationManager::stopClientOnlyConversation()
{
	if (s_clientOnlyMode) 
	{
		// clear out responses.
		ms_responses.clear();
		Transceivers::conversationEnded.emitMessage(true);

		// set target.
		setTarget (CachedNetworkId::cms_invalid, 0, s_soundEffect);

		// emit message.
		static const uint32 hash_req = Crc::normalizeAndCalculate("npcConversationStop");
		ClientCommandQueue::enqueueCommand(hash_req, NetworkId::cms_invalid, Unicode::emptyString);

		// reset flag.
		s_clientOnlyMode = false;
	}
}

//----------------------------------------------------------------------

bool CuiConversationManager::getUseDefaultResponse()
{
	return s_useDefaultResponse;
}

//----------------------------------------------------------------------

bool CuiConversationManager::isClientOnlyMode()
{
	return s_clientOnlyMode;
}

//----------------------------------------------------------------------

const std::string & CuiConversationManager::getSoundEffect()
{
	return s_soundEffect;
}

//----------------------------------------------------------------------

float CuiConversationManager::getClientOnlyWindowCloseTime()
{
	return s_clientConvoWindowCloseTimer;
}

//======================================================================
