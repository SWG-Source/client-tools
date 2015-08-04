// CuiVoiceChatManager.cpp
// Steven Wyckoff

#include "clientUserInterface/FirstClientUserInterface.h"
#include "CuiVoiceChatManager.h"
#include "CuiVoiceChatGlue.h"
#include "CuiVoiceChatEventHandler.h"

#include "clientAudio/Audio.h"

#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/CreatureObject.h"

#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiConsentManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"

#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/VoiceChatChannelInfo.h"
#include "sharedNetworkMessages/VoiceChatOnGetAccount.h"
#include "sharedNetworkMessages/VoiceChatMiscMessages.h"
#include "sharedObject/NetworkIdManager.h"

#include <sstream>

namespace CuiVoiceChatManagerNamespace
{
	namespace Transceivers
	{
#define MESSAGE_EMITER(T,N)MessageDispatch::Transceiver<const CuiVoiceChatManager::Messages::##T##::Payload &, CuiVoiceChatManager::Messages::##T##>N

		MESSAGE_EMITER(VoiceChatEnabledChanged,enabledChanged);
		MESSAGE_EMITER(VoiceChatServerDisabledChanged, serverDisabledChanged);

		MESSAGE_EMITER(ShowFlybarChanged,showFlybarChanged);
		MESSAGE_EMITER(PushToTalkModeChanged,pushToTalkModeChanged);
		MESSAGE_EMITER(PushToTalkKeyPressed,pushToTalkKeyPressed);
		MESSAGE_EMITER(ChannelSelectionModeChanged,channelSelectionModeChanged);
		MESSAGE_EMITER(ChannelAddedToShortlist,channelAddedToShortlist);
		MESSAGE_EMITER(ChannelRemovedFromShortlist,channelRemovedFromShortlist);
		MESSAGE_EMITER(StartedListeningToChannel,startedListeningToChannel);
		MESSAGE_EMITER(StoppedListeningToChannel,stoppedListeningToChannel);
		MESSAGE_EMITER(SpeakingToChannelChanged,speakingToChannelChanged);
		MESSAGE_EMITER(CurrentSpeakerChanged,currentSpeakerChanged);

#undef MESSAGE_EMITER
	}

	static float const defaultSpeakerVolume = 0.5f;
	static float const defaultMicVolume = 0.5f;
	static bool const pushToTalkDefault = true;

	static char const * const vivoxApiConnectionSuffix = "/api2";

	static char const * const vivoxVoiceServiceExecutable = "SWGVoiceService.exe";
	static char const * const localHost = "127.0.0.1";

	static std::string const voiceStringTable("ui_voice");
	static StringId const checkMicAnd(voiceStringTable, "checkmic_and");
	static StringId const checkMicOk(voiceStringTable, "checkmic_ok");
	static StringId const checkMicMuted(voiceStringTable, "checkmic_muted");
	static StringId const checkMicLowVolume(voiceStringTable, "checkmic_low_volume");
	static StringId const checkMicNotSelected(voiceStringTable, "checkmic_not_selected");
	static StringId const checkMicDisconnected(voiceStringTable, "checkmic_disconnected");
	static StringId const checkMicErrorOccured(voiceStringTable, "checkmic_error");
	static StringId const inviteConsentMessage(voiceStringTable, "invite_consent_message");
	static StringId const kickedMessage(voiceStringTable, "kicked_message");
	static StringId const invitationSentMessage(voiceStringTable, "invitation_sent_message");
	static StringId const channelDoesNotExistMessage(voiceStringTable, "channel_doesnt_exist");
	static StringId const channelJoinErrorMessage(voiceStringTable, "channel_join_error");

	bool findVivoxSessionHandle(std::string const & channelURI, std::string & sessionHandle)
	{
		SwgVivox::Session const * const session = SwgVivox::getInstance().FindSessionByURI(channelURI);
		if(session != NULL)
		{
			sessionHandle = session->sHandle;
			return true;
		}
		return false;
	}

	void setVivoxSpeakingChannel(std::string const & uri)
	{
		SwgVivox::Session const * const currentSpeakingSession = SwgVivox::getInstance().GetTransmitSession();

		//we want to speak on a different channel
		if(currentSpeakingSession == NULL && !uri.empty()
			|| currentSpeakingSession && currentSpeakingSession->sURI != uri)
		{
			std::string sessionHandle;
			if(findVivoxSessionHandle(uri, sessionHandle))
			{
				SwgVivox::getInstance().SetTransmitSession(sessionHandle);
			}
		}
	}

	std::string toLower(const std::string & source)
	{
		std::string lower;
		size_t i;
		for(i = 0; i < source.length(); ++i)
		{
			char a = static_cast<char>(tolower(source[i]));
			lower += a;
		}
		return lower;
	}

}


using namespace CuiVoiceChatManagerNamespace;


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// CuiVoiceChatManager
// This is now finally stuff that should actually be in this file
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


bool CuiVoiceChatManager::ms_voiceChatManagerInstalled = false;
CuiVoiceChatManager::DebugOutputLevel CuiVoiceChatManager::ms_debugOutputLevel = CuiVoiceChatManager::DOL_Error;
int CuiVoiceChatManager::ms_vivoxLogLevel = 0;


CuiVoiceChatManager::CuiVoiceChatManager()
: MessageDispatch::Receiver(),
  m_callback (new MessageDispatch::Callback),
  m_messageProcessors(),
  m_eventHandler(new SwgVivoxEventHandler()),
  m_userInfo (),
  m_knownChannelData (),
  m_shortlistedChannels (),
  m_listeningToChannels (),
  m_speakingToChannel (),
  m_pushToTalkKeyDown(true),
  m_theSessionGroupHandle(),
  m_someoneWasSpeakingLastUpdate(false),
  m_disabledByServer(false),
  m_preventAutoLogin(false),
  m_localLogoutRequest(false),
  m_otherPlayer(false)
{
	DEBUG_FATAL(!SwgVivox::isInstalled(), ("Vivox wrapper must be installed before an instance of CuiVoiceChatManager can be created"));

	connectToMessage(VoiceChatChannelInfo::cms_name);
	connectToMessage(VoiceChatOnGetAccount::cms_name);
	connectToMessage(VoiceChatOnGetChannel::cms_name);
	connectToMessage(VoiceChatInvite::cms_name);
	connectToMessage(VoiceChatKick::cms_name);
	connectToMessage(VoiceChatStatus::cms_name);
	connectToMessage("VCBroadcastMessage");

	//toggle the key to make sure everything is in a good state
	pushToTalkKeyPressed(true);
	pushToTalkKeyPressed(false);

	setHandsOff(false);
	setOtherPlayer(false);

	m_callback->connect(*this, &CuiVoiceChatManager::onConnectionServerConnectionChanged, static_cast<GameNetwork::Messages::ConnectionServerConnectionChanged*>(0));
	m_callback->connect(*this, &CuiVoiceChatManager::onIgnoreListChanged, static_cast<CommunityManager::Messages::IgnoreListChanged*>(0));

	setUpMessageProcessors();
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::reset()
{
	//set us to be disconnecting
	UserInfo info;
	setUserInfo(info);
	setChannelSimpleMode("");

	m_shortlistedChannels.clear();
	m_knownChannelData.clear();
	m_activeSpeakers.clear();
	m_currentActiveSpeaker.clear();
	m_pendingChannelAdds.clear();

	if(m_someoneWasSpeakingLastUpdate)
	{
		Audio::unfadeAll();
		m_someoneWasSpeakingLastUpdate = false;
	}
}

//----------------------------------------------------------------------------

CuiVoiceChatManager::~CuiVoiceChatManager()
{
	disconnectFromMessage("VCBroadcastMessage");
	disconnectFromMessage(VoiceChatStatus::cms_name);
	disconnectFromMessage(VoiceChatKick::cms_name);
	disconnectFromMessage(VoiceChatInvite::cms_name);
	disconnectFromMessage(VoiceChatOnGetChannel::cms_name);
	disconnectFromMessage(VoiceChatChannelInfo::cms_name);
	disconnectFromMessage(VoiceChatOnGetAccount::cms_name);

	delete m_eventHandler;
	m_eventHandler = 0;

	m_messageProcessors.clear();

	m_callback->disconnect(*this, &CuiVoiceChatManager::onIgnoreListChanged, static_cast<CommunityManager::Messages::IgnoreListChanged*>(0));
	m_callback->disconnect(*this, &CuiVoiceChatManager::onConnectionServerConnectionChanged, static_cast<GameNetwork::Messages::ConnectionServerConnectionChanged*>(0));

	delete m_callback;
	m_callback = 0;

	if(m_someoneWasSpeakingLastUpdate)
	{
		Audio::unfadeAll();
		m_someoneWasSpeakingLastUpdate = false;
	}

	//TODO: clean up invitations

}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setUpMessageProcessors()
{
#define PROCESSOR(name)m_messageProcessors[MessageDispatch::MessageBase::makeMessageTypeFromString(name##::cms_name)]=&CuiVoiceChatManager::process##name

	PROCESSOR(VoiceChatChannelInfo);
	PROCESSOR(VoiceChatOnGetAccount);
	PROCESSOR(VoiceChatOnGetChannel);
	PROCESSOR(VoiceChatInvite);
	PROCESSOR(VoiceChatKick);
	PROCESSOR(VoiceChatStatus);

	m_messageProcessors[MessageDispatch::MessageBase::makeMessageTypeFromString("VCBroadcastMessage")]=&CuiVoiceChatManager::processVCBroadcastMessage;

#undef PROCESSOR
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	MessageProcessorMap::const_iterator i = m_messageProcessors.find(message.getType());
	if(i != m_messageProcessors.end())
	{
		(this->*(i->second))(source,message);
	}
	else
	{
		DEBUG_WARNING(true, ("CuiVoiceChatManager::receiveMessage got a message that it does not know what to do with"));
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::processVoiceChatChannelInfo(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message)
{
	Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
	VoiceChatChannelInfo const channelInfo(ri);
	DEBUG_WARNING(true, ("Got a message about a chat room %s %s", channelInfo.getUri().c_str(),channelInfo.getPassword().c_str()));

	std::string const lowerChannelName = toLower(channelInfo.getName());

	bool canRemoveFromShortlist = !channelInfo.getForcedShortlist();

	//don't let us remove group, guild, or my own private channel
	if(toLower(getMyPrivateChannelName()) == lowerChannelName)
	{
		canRemoveFromShortlist = false;
	}


	VoiceChatChannelData data(channelInfo.getName(), channelInfo.getUri(), channelInfo.getPassword(), canRemoveFromShortlist, channelInfo.getTargetIsModerator());

	std::stringstream ss;
	ss << "Server sent channel information for " << channelInfo.getName();
	CuiVoiceChatManager::debugOuputString(DOL_Info, ss.str());

	if(channelInfo.getIsLeaveChannelCommand())
	{
		requestLeaveChannel(data.name);
		shortlistRemoveChannel(data.name);
		eraseChannelData(data.name);
	}
	else if (channelInfo.getChannelExists())
	{
		bool join = channelInfo.getIsAutoJoin() || (CuiPreferences::getVoiceAutoJoinChannels() && getSpeakingToChannel().empty());

		newChannelData(data, true, true, join);
	}
	else
	{
		Unicode::String str;
		CuiStringVariablesManager::process (CuiVoiceChatManagerNamespace::channelDoesNotExistMessage, Unicode::narrowToWide(data.name), Unicode::String (), Unicode::String (), str);
		CuiSystemMessageManager::sendFakeSystemMessage(str);
	}

	if(!channelInfo.getAnnounceText().empty())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide(channelInfo.getAnnounceText()));
	}
}

void CuiVoiceChatManager::processVCBroadcastMessage(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message)
{
	Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
	GenericValueTypeMessage<std::string> const msg(ri);

	std::string const & textMessage = msg.getValue();

	if(!textMessage.empty())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide(textMessage));
	}

#ifdef _DEBUG
	else
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("DebugOnly: received voice chat broadcast message with no text"));
	}
#endif

}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::processVoiceChatOnGetAccount(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message)
{
	Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
	VoiceChatOnGetAccount const msg(ri);
	DEBUG_WARNING(true, ("Got an OnGetAccount message success(%s) login(%s) password(%s)", 
		msg.getResult() == VoiceChatOnGetAccount::GAR_SUCCESS ? "true" : "false", msg.getName().c_str(), msg.getPassword().c_str()));

	std::stringstream ss;
	ss << "Server sent login information for " << msg.getName() << " result(" << msg.getResult() << ")";
	CuiVoiceChatManager::debugOuputString(DOL_Info, ss.str());

	if(msg.getResult() == VoiceChatOnGetAccount::GAR_SUCCESS)
	{
		std::string const & connector = msg.getConnectionServerAddress();
		std::string const fullConnectorAddress(connector + vivoxApiConnectionSuffix);
		
		UserInfo info;
		info.m_userName = msg.getName();
		info.m_userNameLower = toLower(info.m_userName);
		info.m_password = msg.getPassword();
		info.m_connectionAddress = fullConnectorAddress;
		
		setUserInfo(info);
	}
	else
	{
		UserInfo info;
		setUserInfo(info);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::processVoiceChatOnGetChannel(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message)
{
	Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
	VoiceChatOnGetChannel const msg(ri);
	DEBUG_WARNING(true, ("Got an OnGetChannel message success(%s) name(%s) uri(%s)", 
		msg.getWasSuccessful() ? "true" : "false", msg.getRoomName().c_str(), msg.getUri().c_str()));

	std::stringstream ss;
	ss << "Server created channel " << msg.getRoomName() << " and sent the information";
	CuiVoiceChatManager::debugOuputString(DOL_Info, ss.str());

	if(toLower(getMyPrivateChannelName()) == toLower(msg.getRoomName()))
	{
		for(PendingChannelAddMap::const_iterator i = m_pendingChannelAdds.begin(); i != m_pendingChannelAdds.end(); ++i)
		{
			VoiceChatInvite const invitemsg(Game::getPlayerNetworkId(), msg.getRoomName(), i->second, i->first);
			GameNetwork::send (invitemsg, true);

			Unicode::String str;
			CuiStringVariablesManager::process (CuiVoiceChatManagerNamespace::invitationSentMessage, Unicode::narrowToWide(i->first), Unicode::String (), Unicode::String (), str);
			CuiSystemMessageManager::sendFakeSystemMessage(str);
		}
		m_pendingChannelAdds.clear();

		VoiceChatChannelData data(msg.getRoomName(), msg.getUri(), msg.getPassword(), false, true);

		newChannelData(data, true, true, true);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::processVoiceChatInvite(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message)
{
	Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
	VoiceChatInvite const msg(ri);
	DEBUG_WARNING(true, ("Got an invite message sourceId(%s) name(%s) uri(%s)", 
		msg.getRequester().getValueString().c_str(), msg.getChannelName().c_str(), msg.getChannelUri().c_str()));
	
	
	VoiceChatChannelData channelData(msg.getChannelName(), msg.getChannelUri(), "", true, false);

	std::stringstream ss;
	ss << "Got a channel invite for " << channelData.name;
	CuiVoiceChatManager::debugOuputString(DOL_Info, ss.str());

	bool showConsent = !CuiPreferences::getVoiceAutoDeclineInvites() && isLoggedIn();
	
	VoiceChatChannelData foundData;
	if(findChannelData(channelData.name, foundData))
	{
		showConsent = false;
	}
	
	//if we have an outstanding invitation for this channel, overwrite the data with the latest info
	for(std::map<int, VoiceChatChannelData>::iterator inviteItr = m_invitations.begin(); inviteItr != m_invitations.end(); ++inviteItr)
	{
		VoiceChatChannelData & oldData = inviteItr->second;
		if(toLower(channelData.name) == toLower(oldData.name))
		{
			oldData = channelData;
			showConsent = false;
		}
	}

	if(showConsent)
	{
		Unicode::String str;
		CuiStringVariablesManager::process (CuiVoiceChatManagerNamespace::inviteConsentMessage, Unicode::narrowToWide(channelData.displayName), Unicode::String (), Unicode::String (), str);

		int consentId = CuiConsentManager::askLocalConsent(str, &CuiVoiceChatManager::handleConsentWindow);
		m_invitations[consentId] = channelData;
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::handleConsentWindow(int id, bool response)
{
	std::map<int, VoiceChatChannelData>::iterator i = getInstance().m_invitations.find(id);
	if(i != getInstance().m_invitations.end())
	{
		if (response == true)
		{
			VoiceChatChannelData & data = i->second;

			// only auto-join if not in a channel
			//getInstance().newChannelData(data, true, true, CuiPreferences::getVoiceAutoJoinChannels() && getInstance().getSpeakingToChannel().empty());

			// auto-join if you have the option turned on
			getInstance().newChannelData(data, true, true, CuiPreferences::getVoiceAutoJoinChannels());

			// always join because you accepted the invite
			//getInstance().newChannelData(data, true, true, true);
		}

		getInstance().m_invitations.erase(i);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::processVoiceChatKick(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message)
{
	Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
	VoiceChatKick const msg(ri);
	DEBUG_WARNING(true, ("Got a kick message sourceId(%s) name(%s)", 
		msg.getRequester().getValueString().c_str(), msg.getChannelName().c_str()));

	std::stringstream ss;
	ss << "Got kicked out of channel " << msg.getChannelName();
	CuiVoiceChatManager::debugOuputString(DOL_Info, ss.str());


	bool showSystemMessage = false;

	std::string const & channelName = msg.getChannelName();
	requestLeaveChannel(channelName);
	shortlistRemoveChannel(channelName);
	showSystemMessage = eraseChannelData(channelName);

	std::string const lowerChannelName = toLower(channelName);
	for(std::map<int,VoiceChatChannelData>::iterator i = m_invitations.begin(); i != m_invitations.end(); ++i)
	{
		if(toLower(i->second.name) == lowerChannelName)
		{
			CuiConsentManager::dismissLocalConsentRequest(i->first);
			m_invitations.erase(i);
			showSystemMessage = true;
		}
	}

	showSystemMessage = showSystemMessage && getVoiceChatEnabled();

	if(showSystemMessage)
	{
		VoiceChatChannelData data(msg.getChannelName(),"", "", true, false);
		Unicode::String str;
		CuiStringVariablesManager::process (CuiVoiceChatManagerNamespace::kickedMessage, Unicode::narrowToWide(data.displayName), Unicode::String (), Unicode::String (), str);
		CuiSystemMessageManager::sendFakeSystemMessage(str);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::processVoiceChatStatus(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message)
{
	Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
	VoiceChatStatus const msg(ri);
	DEBUG_WARNING(true, ("Got a voice status message code(%u)", msg.getStatus()));

	bool nowDisabled = m_disabledByServer;

	switch(msg.getStatus())
	{
	case VoiceChatStatus::SC_VoiceEnabled:
		{
			nowDisabled = false;
			break;
		}
	case VoiceChatStatus::SC_VoiceClientIsTrial:
	case VoiceChatStatus::SC_VoiceServerDisabled:
		{
			nowDisabled = true;
			break;
		}
	default:
		break;
	}

	if(nowDisabled != m_disabledByServer)
	{
		m_disabledByServer = nowDisabled;
		CuiVoiceChatManagerNamespace::Transceivers::serverDisabledChanged.emitMessage(m_disabledByServer);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::onConnectionServerConnectionChanged(bool const & )
{
	if(!GameNetwork::isConnectedToConnectionServer())
	{
		//set us to be disconnecting
		UserInfo info;
		setUserInfo(info);
		setChannelSimpleMode("");

		//clear our state
		getInstance().reset();
	}
	else
	{
		//check our channel data to see if anything we got before is our private channel
		for(VoiceChatChannelDataList::iterator i = getInstance().m_knownChannelData.begin(); i != getInstance().m_knownChannelData.end(); ++i)
		{
			VoiceChatChannelData & data = i->second;
			data.hasModeratorPowers = (toLower(data.name) == toLower(getMyPrivateChannelName()));
		}
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::onIgnoreListChanged(bool const &)
{
	//make sure that all of the ignored peeps are muted
	SwgVivox::SessionMap const & sessionMap = SwgVivox::getInstance().GetSessions();
	for(SwgVivox::SessionMap::const_iterator sessionItr = sessionMap.begin(); sessionItr != sessionMap.end(); ++sessionItr)
	{
		SwgVivox::Session const & currentSession = sessionItr->second;
		for(SwgVivox::Session::ParticipantMap::const_iterator i = currentSession.mParticipants.begin();
			i != currentSession.mParticipants.end(); ++i)
		{
			//@TODO: we may not need to create a whole active speaker and instead
			//get away with simply creating the name from the participant.
			ActiveSpeaker temp;
			VivoxUtils::ConvertParticipantToActiveSpeaker(i->second,temp);
			if(CommunityManager::isIgnored(Unicode::narrowToWide(temp.displayName)))
			{
				setLocalParticipantMute(temp.id, true);
			}
		}
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::install()
{
	DEBUG_FATAL(ms_voiceChatManagerInstalled,("Already installed"));

	SwgVivox::install();

	IGNORE_RETURN(getInstance ());

	ms_voiceChatManagerInstalled = true;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::remove()
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));

	SwgVivox::remove();

	ms_voiceChatManagerInstalled = false;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setApiLogLevel(int level)
{
	ms_vivoxLogLevel = level;
}

//----------------------------------------------------------------------------

int CuiVoiceChatManager::getApiLogLevel()
{
	return ms_vivoxLogLevel;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setDebugOutputLevel(DebugOutputLevel level)
{
	ms_debugOutputLevel = level;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::debugOuputString(DebugOutputLevel level, std::string const & out)
{
	if(ms_debugOutputLevel >= level)
	{
		CuiChatRoomManager::sendPrelocalizedChat(Unicode::narrowToWide(out));
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::dumpDebugInfo(std::string & outString)
{
	std::set<std::string> const & listeningTo = getListeningToChannels();
	std::string const & speakingTo = getSpeakingToChannel();
	VoiceChatChannelDataList const & knownChannels = getKnownChannelData();
	std::set<std::string> const & shortlist = getShortlistedChannels();

#define BTOS(b) ((b)?"true":"false")

	outString += "Voice chat enabled: ";
	outString += BTOS(getVoiceChatEnabled());

	outString += "\nPTT mode: ";
	outString += BTOS(getUsePushToTalk());
	outString += " PTT key down: ";
	outString += BTOS(getPushToTalkKeyPressed());

	outString += "\nKnown Channel Data:";
	for(VoiceChatChannelDataList::const_iterator i = knownChannels.begin(); i != knownChannels.end(); ++i)
	{
		outString += "\n\t" + i->second.toString();
	}

	outString += "\nShortlist:\n";
	for(std::set<std::string>::const_iterator i = shortlist.begin(); i != shortlist.end(); ++i)
	{
		outString += (*i) + " ";
	}

	outString += "\nDesired Speaking to Channel: " + speakingTo;
	outString += "\nDesired Listening To Channels:";
	if(!listeningTo.empty())
	{
		outString += "\n";
	}
	for(std::set<std::string>::const_iterator i = listeningTo.begin(); i != listeningTo.end(); ++i)
	{
		outString += (*i) + " ";
	}

	outString += "\n" + getCsReportString();

#undef BTOS
}

//----------------------------------------------------------------------------

std::string CuiVoiceChatManager::getCsReportString()
{
	std::string outString;

	FormattedString<1024> fs;

	outString += fs.sprintf("Vivox Information: account: %s %s\n", getUserInfo().m_userName.c_str(), SwgVivox::getInstance().isLoggedIn()?"logged in":"not logged in");

	SwgVivox::SessionMap const & sessions = SwgVivox::getInstance().GetSessions();

	if(sessions.empty())
	{
		outString += "no connected sessions";
	}

	for(SwgVivox::SessionMap::const_iterator i = sessions.begin(); i != sessions.end(); ++i)
	{
		SwgVivox::Session const & thisSession = i->second;

		outString += fs.sprintf("%s \"%s\" [%s]%s transmit %s\n", thisSession.sChannelName.c_str(), thisSession.sDisplayName.c_str(), thisSession.sURI.c_str(),
			thisSession.bMuted?" muted":"",thisSession.bTransmitEnabled?"enabled":"disabled");

		outString += "Participants:";

		FormattedString<1024> fs;
		for(SwgVivox::Session::ParticipantMap::const_iterator j = thisSession.mParticipants.begin(); j != thisSession.mParticipants.end(); ++j)
		{
			SwgVivox::Participant const & part = j->second;
#define BTOS(b) ((b)?"true":"false")
			outString += fs.sprintf(" {%s \"%s\" [%s] speaking(%s) localMute(%s) localVol(%f) energy(%f)}",
				part.sAccount.c_str(), part.sDisplayName.c_str(), part.sURI.c_str(), BTOS(part.bSpeaking),
				BTOS(part.GetLocalMute()),part.GetLocalVolume(),part.fEnergy);
#undef BTOS
		}
	}

	return outString;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setVoiceChatEnabled(bool enabled)
{
	if(enabled != getVoiceChatEnabled())
	{
		std::stringstream ss;
		ss << "Voice chat ";
		ss << (enabled ? "enabled" : "disabled");
		CuiVoiceChatManager::debugOuputString(DOL_Info, ss.str());

		CuiPreferences::setVoiceChatEnabled(enabled);
		CuiVoiceChatManagerNamespace::Transceivers::enabledChanged.emitMessage(enabled);
	}
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getVoiceChatEnabled()
{
	return CuiPreferences::getVoiceChatEnabled();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getVoiceChatDisabledByServer()
{
	return getInstance().m_disabledByServer;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getUseAdvancedChannelSelection()
{
	return CuiPreferences::getVoiceUseAdvancedChannelSelection();
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setUseAdvancedChannelSelection(bool useit)
{
	bool currentMode = CuiPreferences::getVoiceUseAdvancedChannelSelection();
	if(useit != currentMode)
	{
		CuiPreferences::setVoiceUseAdvancedChannelSelection(useit);
		CuiVoiceChatManagerNamespace::Transceivers::channelSelectionModeChanged.emitMessage(useit);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::update(float elapsedTime)
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));

	SwgVivox::getInstance().ProcessEvents();

	static bool updateEvenWhenDisabled = false;
	if(!updateEvenWhenDisabled && !getVoiceChatEnabled())
	{
		if(SwgVivox::getInstance().isConnected())
		{
			SwgVivox::getInstance().Shutdown();
		}
		return;
	}

	getInstance().updateCurrentSpeaker();

	static float timeTillUpdate = 0.0f;
	timeTillUpdate -= elapsedTime;
	if(timeTillUpdate <= 0.0f)
	{
		timeTillUpdate = 1.0f;

		if(getInstance().doLoginStateUpdate())
		{
			getInstance().doChannelStateUpdate();
		}
	}

	getInstance().m_preventAutoLogin = false;
	getInstance().m_localLogoutRequest = false;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setUserInfo(CuiVoiceChatManager::UserInfo const & info)
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));
	DEBUG_WARNING(SwgVivox::getInstance().isLoggedIn(), ("trying to set user info while logged in"));

	if(getInstance().m_userInfo.m_userName != info.m_userName)
	{
		getInstance ().m_userInfo = info;
		//TODO: trigger something
	}
}

//----------------------------------------------------------------------------

CuiVoiceChatManager::UserInfo const & CuiVoiceChatManager::getUserInfo()
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));
	return getInstance ().m_userInfo;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::isLoggedIn()
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));
	return SwgVivox::getInstance().isLoggedIn();
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::requestJoinChannel(std::string const & name)
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));

	VoiceChatChannelData data;
	
	if(getInstance().findChannelData(name, data))
	{
		getInstance().setChannelShortlistedState(data, true);
		setChannelSimpleMode(name);
	}
	else
	{
		requestChannelInfoFromServer(name);
	}
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::BeginSession(VoiceChatChannelData const & data)
{
	return SwgVivox::getInstance().BeginSession(data.uri, data.name, data.password, getInstance().m_theSessionGroupHandle);
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::requestLeaveChannel(std::string const &name)
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));

	VoiceChatChannelData data;

	getInstance().findChannelData(name, data);

	TerminateSession(data);

	getInstance().setChannelListeningState(data, false);
	
	if(getSpeakingToChannel() == name)
	{
		getInstance().setSpeakingToChannel("");
	}
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::TerminateSession(VoiceChatChannelData const & data)
{
	std::string sessionHandle;
	if(CuiVoiceChatManagerNamespace::findVivoxSessionHandle (data.uri, sessionHandle))
	{
		return SwgVivox::getInstance().TerminateSession (sessionHandle);
	}
	return false;
}

//---------------------------------------------------------------------------

bool CuiVoiceChatManager::isInChannel(std::string const & name)
{	
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));

	VoiceChatChannelData data;

	if(getInstance().findChannelData(name, data))
	{
		std::string sessionHandle;
		return CuiVoiceChatManagerNamespace::findVivoxSessionHandle (data.uri, sessionHandle);
	}

	return false;
}

//---------------------------------------------------------------------------

bool CuiVoiceChatManager::getChannelDisplayName(std::string const & name, std::string & displayName)
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));

	VoiceChatChannelData data;
	if(getInstance().findChannelData(name,data))
	{
		displayName = data.displayName;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setListeningToChannel(std::string const & name, bool listening)
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));

	VoiceChatChannelData data;
	bool const foundData = getInstance().findChannelData(name, data);

	std::stringstream ss;
	ss << "Set listening channel:";
	ss << listening ? " true " : " false ";
	ss << data.toString();
	CuiVoiceChatManager::debugOuputString(DOL_Info, ss.str());

	//enforce the rule that you can not speak on a channel you dont listen to
	if(foundData && (listening || data.name != getInstance().getSpeakingToChannel()))
	{
		//@TODO fire off a channel join here?

		getInstance().setChannelListeningState(data, listening);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setSpeakingToChannel(std::string const & name)
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));

	VoiceChatChannelData data;
	getInstance().findChannelData(name, data);

	std::stringstream ss;
	ss << "Set speaking channel: " << data.toString();
	CuiVoiceChatManager::debugOuputString(DOL_Info, ss.str());

	CuiVoiceChatManagerNamespace::setVivoxSpeakingChannel(data.uri);

	//enforce the rule that you can not speak on a channel you dont listen to
	setListeningToChannel(name, true);

	//temporarily short circut the callback assuming that the changes worked
	getInstance().setChannelSpeakingToState(data);
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setChannelSimpleMode(std::string const & name)
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));

	VoiceChatChannelData data;
	getInstance().findChannelData(name, data);

	std::stringstream ss;
	ss << "Set speaking channel simple: " << data.toString();
	CuiVoiceChatManager::debugOuputString(DOL_Info, ss.str());

	//make sure that we are only in the one channel
	std::list<std::string> listeningToRemove;
	std::set<std::string> const & currentlyListening = getListeningToChannels();
	for(std::set<std::string>::const_iterator i = currentlyListening.begin(); i != currentlyListening.end(); ++i)
	{
		if(*i != name)
		{
			listeningToRemove.push_back(*i);
		}
	}

	for(std::list<std::string>::const_iterator j = listeningToRemove.begin(); j != listeningToRemove.end(); ++j)
	{
		VoiceChatChannelData tmpData;
		if(getInstance().findChannelData(*j, tmpData))
		{
			getInstance().setChannelListeningState(tmpData, false);
		}
	}

	CuiVoiceChatManagerNamespace::setVivoxSpeakingChannel(data.uri);

	//enforce the rule that you can not speak on a channel you dont listen to
	setListeningToChannel(name, true);
	getInstance().setChannelSpeakingToState(data);

}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::findChannelData(std::string const & name, CuiVoiceChatManager::VoiceChatChannelData & data)
{
	VoiceChatChannelDataList::const_iterator i = m_knownChannelData.find(toLower(name));
	if(i != m_knownChannelData.end())
	{
		data = i->second;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::findChannelDataByUri(std::string const & uri, CuiVoiceChatManager::VoiceChatChannelData & data)
{
	for(VoiceChatChannelDataList::const_iterator i = m_knownChannelData.begin(); i != m_knownChannelData.end(); ++i)
	{
		if(i->second.uri == uri)
		{
			data = i->second;
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::enterChannelData(CuiVoiceChatManager::VoiceChatChannelData const & data, bool overwrite)
{
	VoiceChatChannelDataList::iterator i = m_knownChannelData.find(toLower(data.name));
	if(i != m_knownChannelData.end())
	{
		if(overwrite)
		{
			i->second = data;
		}
	}
	else
	{
		m_knownChannelData.insert(std::make_pair(toLower(data.name),data));
	}
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::eraseChannelData(std::string const & name)
{
	VoiceChatChannelDataList::iterator i = m_knownChannelData.find(toLower(name));
	if(i != m_knownChannelData.end())
	{
		m_knownChannelData.erase(i);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::newChannelData(VoiceChatChannelData const & data, bool clobber, bool shortlist, bool join)
{
	enterChannelData(data, clobber);

	if(isChannelShortlisted(data.name) != shortlist)
	{
		setChannelShortlistedState(data, shortlist);
	}

	//TODO: this does not support advanced mode
	if(join)
	{
		setChannelSimpleMode(data.name);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::shortlistAddChannel(std::string const & name)
{
	VoiceChatChannelData data;

	if(getInstance().findChannelData(name, data))
	{
		getInstance().setChannelShortlistedState(data, true);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::shortlistRemoveChannel(std::string const & name)
{
	VoiceChatChannelData data;

	if(getInstance().findChannelData(name, data))
	{
		getInstance().setChannelShortlistedState(data, false);
	}
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::isChannelShortlisted(std::string const & name)
{
	return getInstance().m_shortlistedChannels.find(name) != getInstance().m_shortlistedChannels.end();
}

//----------------------------------------------------------------------------

std::set<std::string> const & CuiVoiceChatManager::getShortlistedChannels ()
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));
	return getInstance().m_shortlistedChannels;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::canChannelBeRemovedFromShortlist(std::string const & name)
{
	VoiceChatChannelData data;
	if(getInstance().findChannelData(name, data))
	{
		return data.shortlistRemovable;
	}
	return false;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::doIHaveModeratorPowersInChannel(std::string const & name)
{
	VoiceChatChannelData data;
	if(getInstance().findChannelData(name, data))
	{
		return data.hasModeratorPowers;
	}
	return false;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::errorConnectingToChannel(std::string const & channelUri, std::string const & statusString)
{
	for(VoiceChatChannelDataList::iterator i = getInstance().m_knownChannelData.begin(); i != getInstance().m_knownChannelData.end(); ++i)
	{
		if(i->second.uri == channelUri)
		{
			Unicode::String str;
			CuiStringVariablesManager::process (CuiVoiceChatManagerNamespace::channelJoinErrorMessage, Unicode::narrowToWide(i->second.name), Unicode::String (), Unicode::narrowToWide(statusString), str);
			CuiSystemMessageManager::sendFakeSystemMessage(str);

			if(i->second.name == getSpeakingToChannel())
			{
				setSpeakingToChannel("");
			}

			setListeningToChannel(i->second.name,false);
		}
	}
}

//----------------------------------------------------------------------------

std::set<std::string> const & CuiVoiceChatManager::getListeningToChannels()
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));
	return getInstance().m_listeningToChannels;
}

//----------------------------------------------------------------------------

std::string const & CuiVoiceChatManager::getSpeakingToChannel()
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));
	return getInstance().m_speakingToChannel;
}

//----------------------------------------------------------------------------

CuiVoiceChatManager::VoiceChatChannelDataList const & CuiVoiceChatManager::getKnownChannelData()
{
	DEBUG_FATAL(!ms_voiceChatManagerInstalled,("Not installed"));
	return getInstance().m_knownChannelData;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::isParticipantMe(ParticipantId const & speaker)
{
	static const std::string prefix = "sip:";
	UserInfo const & info = getUserInfo();
	return(0 == toLower(speaker.m_uri).compare(prefix.length(),info.m_userName.length(),toLower(info.m_userName)));
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::isParticipantThisObject(ParticipantId const & speaker, NetworkId const & id)
{
	Object const * const obj = NetworkIdManager::getObjectById(id);
	if(obj)
	{
		ClientObject const * const clientObj = obj->asClientObject();
		if(clientObj)
		{
			static const std::string prefix = "sip:";

			std::string const firstName = Unicode::wideToNarrow (clientObj->getLocalizedFirstName());
			std::string const clusterName = GameNetwork::getCentralServerName();
			std::string const constructedUsername(clusterName + "." + firstName);

			return(0 == toLower(speaker.m_uri).compare(prefix.length(),constructedUsername.length(),toLower(constructedUsername)));
		}
	}
	return false;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::isThisObjectSpeaking(NetworkId const & id)
{
	if(!getVoiceChatEnabled() || !SwgVivox::getInstance().isLoggedIn())
	{
		return false;
	}

	typedef std::list<CuiVoiceChatManager::ActiveSpeaker> SpeakerList;
	SpeakerList allParticipants;
	getAllParticipants (allParticipants);
	for(SpeakerList::const_iterator i = allParticipants.begin(); i != allParticipants.end(); ++i)
	{
		ActiveSpeaker const & speaker = *i;
		if(isParticipantThisObject(speaker.id, id) && speaker.speaking)
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setLocalParticipantVolume(CuiVoiceChatManager::ParticipantId const & speaker, float gain)
{
	SwgVivox::Participant const * participant = SwgVivox::getInstance().FindParticipant(speaker.m_uri, speaker.m_sessionHandle);
	if(participant)
	{
		participant->SetLocalVolume(gain);
	}
}

//----------------------------------------------------------------------------

float CuiVoiceChatManager::getLocalParticipantVolume(CuiVoiceChatManager::ParticipantId const & speaker)
{
	SwgVivox::Participant const * participant = SwgVivox::getInstance().FindParticipant(speaker.m_uri, speaker.m_sessionHandle);
	if(participant)
	{
		return participant->GetLocalVolume();
	}
	return -1.0f;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::setLocalParticipantMute(CuiVoiceChatManager::ParticipantId const & speaker, bool muted)
{
	SwgVivox::Participant const * participant = SwgVivox::getInstance().FindParticipant(speaker.m_uri, speaker.m_sessionHandle);
	if(participant)
	{
		//@TODO: we may not need to create the whole active speaker just to get the name
		ActiveSpeaker temp;
		VivoxUtils::ConvertParticipantToActiveSpeaker(*participant,temp);
		bool isIgnored = CommunityManager::isIgnored(Unicode::narrowToWide(temp.displayName));

		// prevent unmuting ignored people
		if( !(isIgnored && !muted) )
		{
			participant->SetLocalMute(muted);
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getLocalParticipantMute(CuiVoiceChatManager::ParticipantId const & speaker)
{
	SwgVivox::Participant const * participant = SwgVivox::getInstance().FindParticipant(speaker.m_uri, speaker.m_sessionHandle);
	if(participant)
	{
		return participant->GetLocalMute();
	}
	//error gives a possible valid output...ew
	return false;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setShowFlybar(bool yesno)
{
	if(getShowFlybar() != yesno)
	{
		CuiPreferences::setVoiceShowFlybar(yesno);
		CuiVoiceChatManagerNamespace::Transceivers::showFlybarChanged.emitMessage(yesno);
	}
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getShowFlybar()
{
	return CuiPreferences::getVoiceShowFlybar();
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setUsePushToTalk(bool pushToTalkOn)
{
	if(getUsePushToTalk() != pushToTalkOn)
	{
		setUsePushToTalkForceUpdate(pushToTalkOn);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setUsePushToTalkForceUpdate(bool pushToTalkOn)
{
	CuiPreferences::setVoiceUsePushToTalk(pushToTalkOn);
	getInstance().m_pushToTalkKeyDown = false;
	SwgVivox::getInstance().SetLocalMicMute(pushToTalkOn);
	CuiVoiceChatManagerNamespace::Transceivers::pushToTalkModeChanged.emitMessage(pushToTalkOn);
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getUsePushToTalk()
{
	return CuiPreferences::getVoiceUsePushToTalk();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getUsePushToTalkDefault()
{
	return pushToTalkDefault;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::pushToTalkKeyPressed(bool down)
{
	if(getUsePushToTalk())
	{
		bool wasdown = getInstance().m_pushToTalkKeyDown;
		getInstance().m_pushToTalkKeyDown = down;
		setLocalMicMute(!down);
		if(wasdown != down)
		{
			CuiVoiceChatManagerNamespace::Transceivers::pushToTalkKeyPressed.emitMessage(down);
		}
	}
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getPushToTalkKeyPressed()
{
	return getInstance().m_pushToTalkKeyDown;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setLocalMicMute(bool mute)
{
	SwgVivox::getInstance().SetLocalMicMute(mute);
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setLocalMicVolume(float gain)
{
	SwgVivox::getInstance().SetLocalMicVolume(gain);
}

//----------------------------------------------------------------------------

float CuiVoiceChatManager::getLocalMicVolume()
{
	return SwgVivox::getInstance().GetLocalMicVolume();
}

//----------------------------------------------------------------------------

float CuiVoiceChatManager::getLocalMicVolumeDefault()
{
	return defaultMicVolume;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setLocalSpeakerMute(bool mute)
{
	SwgVivox::getInstance().SetLocalSpeakerMute(mute);
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setLocalSpeakerVolume(float gain)
{
	SwgVivox::getInstance().SetLocalSpeakerVolume(gain);
}

//----------------------------------------------------------------------------

float CuiVoiceChatManager::getLocalSpeakerVolume()
{
	return SwgVivox::getInstance().GetLocalSpeakerVolume();
}

//----------------------------------------------------------------------------

float CuiVoiceChatManager::getLocalSpeakerVolumeDefault()
{
	return defaultSpeakerVolume;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::requestOutputDeviceList()
{
	return SwgVivox::getInstance().GetOutputDevices();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::setOutputDevice(std::string const & device)
{
	if(device != getCurrentOutputDevice())
	{
		return SwgVivox::getInstance().SetOutputDevice(device);
	}
	return false;
}

//---------------------------------------------------------------------------

std::string const & CuiVoiceChatManager::getDefaultOutputDevice()
{
	return SwgVivox::getInstance().GetDefaultOutputDevice();
}

//----------------------------------------------------------------------------

std::string const & CuiVoiceChatManager::getCurrentOutputDevice()
{
	return SwgVivox::getInstance().GetCurrentOutputDevice();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::requestInputDeviceList()
{
	return SwgVivox::getInstance().GetInputDevices();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::setInputDevice(std::string const & device)
{
	if(device != getCurrentInputDevice())
	{
		return SwgVivox::getInstance().SetInputDevice(device);
	}
	return false;
}

//----------------------------------------------------------------------------

std::string const & CuiVoiceChatManager::getDefaultInputDevice()
{
	return SwgVivox::getInstance().GetDefaultInputDevice();
}

//----------------------------------------------------------------------------

std::string const & CuiVoiceChatManager::getCurrentInputDevice()
{
	return SwgVivox::getInstance().GetCurrentInputDevice();
}

//----------------------------------------------------------------------------

uint32 CuiVoiceChatManager::checkWindowsMicSettings(std::string const & device, bool fix)
{
	VivoxCheckMic result = sCheckMic(device, fix);
	return static_cast<uint32>(result);
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::canWindowsSettingsBeCorrected(uint32 checkMicResult)
{
	static const uint32 fixableProblems = VCM_NOT_SELECTED | VCM_VOLUME_TOO_LOW | VCM_MUTED;
	return (checkMicResult & fixableProblems);
}

//----------------------------------------------------------------------------

Unicode::String CuiVoiceChatManager::getCheckMicString(uint32 checkMicResult)
{
	std::list<Unicode::String> codeList;

	if(checkMicResult == VCM_OK)
		return checkMicOk.localize();

	if(checkMicResult & VCM_ERROR)
		codeList.push_back(checkMicErrorOccured.localize());

	if(checkMicResult & VCM_DISCONNECTED)
		codeList.push_back(checkMicDisconnected.localize());

	if(checkMicResult & VCM_NOT_SELECTED)
		codeList.push_back(checkMicNotSelected.localize());

	if(checkMicResult & VCM_VOLUME_TOO_LOW)
		codeList.push_back(checkMicLowVolume.localize());

	if(checkMicResult & VCM_MUTED)
		codeList.push_back(checkMicMuted.localize());

	Unicode::String outstr;

	for(std::list<Unicode::String>::const_iterator i = codeList.begin(); i != codeList.end(); ++i)
	{
		if(i != codeList.begin())
		{
			outstr = checkMicAnd.localize() + (*i) + outstr;
		}
		else
		{
			outstr = (*i) + outstr;
		}
	}

	return outstr;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::beginEchoTest()
{
	return SwgVivox::getInstance().BeginEchoTest();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::stopEchoTest()
{
	return SwgVivox::getInstance().StopEchoTest();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::startEchoTestPlayback()
{
	return SwgVivox::getInstance().PlayEchoTest();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::isPerformingEchoTest()
{
	return SwgVivox::getInstance().IsPerformingEchoTest();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::isPlayingEchoTest()
{
	return SwgVivox::getInstance().IsPlayingEchoTest();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::isEchoTestPlaybackAvailable()
{
	return SwgVivox::getInstance().IsEchoTestPlaybackAvailable();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::beginVUMonitor()
{
	return SwgVivox::getInstance().BeginVUMonitor();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::endVUMonitor()
{
	return SwgVivox::getInstance().EndVUMonitor();
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::isVUMonitorActive()
{
	return SwgVivox::getInstance().IsVUMonitorActive();
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setChannelShortlistedState(VoiceChatChannelData const & data, bool shortlisted)
{
	std::set<std::string>::iterator i = m_shortlistedChannels.find(data.name);
	bool alreadyShortlisted = i != m_shortlistedChannels.end();
	if(shortlisted != alreadyShortlisted)
	{
		if(shortlisted)
		{
			m_shortlistedChannels.insert(data.name);
			CuiVoiceChatManagerNamespace::Transceivers::channelAddedToShortlist.emitMessage(data);
		}
		else
		{
			m_shortlistedChannels.erase(i);
			CuiVoiceChatManagerNamespace::Transceivers::channelRemovedFromShortlist.emitMessage(data);
		}
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setChannelListeningState(CuiVoiceChatManager::VoiceChatChannelData const & data, bool listening)
{
	std::set<std::string>::iterator i = m_listeningToChannels.find(data.name);
	bool alreadListening = i != m_listeningToChannels.end();
	if(listening != alreadListening)
	{
		if(listening)
		{
			m_listeningToChannels.insert(data.name);
			CuiVoiceChatManagerNamespace::Transceivers::startedListeningToChannel.emitMessage(data);
		}
		else
		{
			m_listeningToChannels.erase(i);
			CuiVoiceChatManagerNamespace::Transceivers::stoppedListeningToChannel.emitMessage(data);
		}
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setChannelSpeakingToState(CuiVoiceChatManager::VoiceChatChannelData & data)
{
	if(m_speakingToChannel != data.name)
	{
		m_speakingToChannel = data.name;
		CuiVoiceChatManagerNamespace::Transceivers::speakingToChannelChanged.emitMessage(data);
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::getAllParticipants(std::list<ActiveSpeaker> & allParticipants)
{
	allParticipants.clear();

	SwgVivox::SessionMap const & sessionMap = SwgVivox::getInstance().GetSessions();
	for(SwgVivox::SessionMap::const_iterator sessionItr = sessionMap.begin(); sessionItr != sessionMap.end(); ++sessionItr)
	{
		SwgVivox::Session const & currentSession = sessionItr->second;
		for(SwgVivox::Session::ParticipantMap::const_iterator i = currentSession.mParticipants.begin();
			i != currentSession.mParticipants.end(); ++i)
		{
			ActiveSpeaker temp;
			VivoxUtils::ConvertParticipantToActiveSpeaker(i->second,temp);
			allParticipants.push_back(temp);
		}
	}
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getCurrentSpeakerInfo(ActiveSpeaker & currentSpeaker)
{
	return getInstance().getCurrentSpeakerInfoInternal(currentSpeaker);
}

//----------------------------------------------------------------------------

CuiVoiceChatManager::ParticipantId const & CuiVoiceChatManager::getCurrentActiveSpeakerId()
{
	return getInstance().m_currentActiveSpeaker;
}

//----------------------------------------------------------------------------

CuiVoiceChatManager::ActiveSpeakerMap& CuiVoiceChatManager::getCurrentSpeakerList()
{
	return m_activeSpeakers;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getParticipantData(ParticipantId const & id, ActiveSpeaker & data)
{
	SwgVivox::Participant const * const part = SwgVivox::getInstance().FindParticipant(id.m_uri, id.m_sessionHandle);
	if(part)
	{
		VivoxUtils::ConvertParticipantToActiveSpeaker(*part, data);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getCurrentSpeakerInfoInternal(ActiveSpeaker & currentSpeaker)
{
	if(m_currentActiveSpeaker.isValid())
	{
		ActiveSpeakerMap::iterator i = m_activeSpeakers.find(m_currentActiveSpeaker);
		if(i != m_activeSpeakers.end())
		{
			currentSpeaker = i->second;
			return true;
		}
		else
		{
			m_currentActiveSpeaker.clear();
		}
	}

	return false;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::updateCurrentSpeaker()
{
	static uint32 const removeFromActiveSilentTime = 2;
	static uint32 const stillSpeakingPadTime = 1;
	
	// loop through all of the active speakers and remove any that have timed out
	// keep track of the one that has been speaking the longest
	uint32 now = ::time(0);

	bool firstSpeakerIsMe = true;
	bool someoneIsSpeaking = false;
	CuiVoiceChatManager::ParticipantId lastSpeaker;
	uint32 lastSpeakingTime = 0;

	CuiVoiceChatManager::ParticipantId firstSpeaker;
	uint32 firstTalkingTime = now + 1;

	for(ActiveSpeakerMap::const_iterator i = m_activeSpeakers.begin(); i != m_activeSpeakers.end(); ++i)
	{
		ActiveSpeaker const& thisSpeaker = i->second;

		//this speaker is considered currently speaking
		if(thisSpeaker.lastHeardFrom + stillSpeakingPadTime >= now)
		{
			bool thisSpeakerIsMe = isParticipantMe(i->first);

			//make the most recent speaker the active speaker
			//but give priority to speakers other than myself
			if((!someoneIsSpeaking)
				|| (firstSpeakerIsMe && !thisSpeakerIsMe)
				|| (firstSpeakerIsMe == thisSpeakerIsMe && (thisSpeaker.startedSpeaking < firstTalkingTime)))
			{
				firstTalkingTime = thisSpeaker.startedSpeaking;
				firstSpeaker = i->first;
				someoneIsSpeaking = true;
				firstSpeakerIsMe = isParticipantMe(firstSpeaker);
			}
		}

		if(thisSpeaker.lastHeardFrom > lastSpeakingTime
			&& thisSpeaker.lastHeardFrom + removeFromActiveSilentTime >= now)
		{
			lastSpeakingTime = thisSpeaker.lastHeardFrom;
			lastSpeaker = i->first;
		}
	}


	if(someoneIsSpeaking && !m_someoneWasSpeakingLastUpdate)
	{
		Audio::fadeAll();
		m_someoneWasSpeakingLastUpdate = true;
	}
	else if(m_someoneWasSpeakingLastUpdate && !someoneIsSpeaking)
	{
		Audio::unfadeAll();
		m_someoneWasSpeakingLastUpdate = false;
	}

	CuiVoiceChatManager::ParticipantId newActiveSpeaker = someoneIsSpeaking ? firstSpeaker : lastSpeaker;

	if(newActiveSpeaker != m_currentActiveSpeaker)
	{
		m_currentActiveSpeaker = newActiveSpeaker;

		CuiVoiceChatManagerNamespace::Transceivers::currentSpeakerChanged.emitMessage(0);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::doLoginStateUpdate()
{

	bool shouldBeConnected = GameNetwork::isConnectedToConnectionServer() && !m_userInfo.m_userName.empty() && !m_preventAutoLogin && !SwgVivox::getInstance().IsHandsOff();

	if(m_otherPlayer)
		return true;

	if(shouldBeConnected)
	{
		if(SwgVivox::getInstance().isLoggedIn() && SwgVivox::getInstance().GetAccountName() == m_userInfo.m_userNameLower) // Lowercase when dealing with the shared wrapper.
		{
			return true;
		}

		if(!SwgVivox::getInstance().isConnected())
		{
			SwgVivox::getInstance().BeginConnect(m_userInfo.m_connectionAddress, 0, 0, vivoxVoiceServiceExecutable, localHost);
			return false;
		}
		else if(!SwgVivox::getInstance().isConnecting())
		{
			if(!SwgVivox::getInstance().isLoggingIn())
			{
				SwgVivox::getInstance().BeginLogin(m_userInfo.m_userName, m_userInfo.m_password);
			}
		}
	}
	else
	{
		if(SwgVivox::getInstance().isLoggedIn())
		{
			SwgVivox::getInstance().Logout();
		}
		else if(SwgVivox::getInstance().isConnected())
		{
			SwgVivox::getInstance().Shutdown();
		}
		return false;
	}

	return false;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::doChannelStateUpdate()
{
	bool noChanges = true;

	if(SwgVivox::getInstance().IsHandsOff())
		return true;

	typedef std::map<std::string, VoiceChatChannelData> UriToDataMap;

	UriToDataMap needToAdd;
	std::list<std::string> needToRemove;

	VoiceChatChannelData data;

	// get our desired state
	std::set<std::string> const & names = getListeningToChannels();

	for(std::set<std::string>::const_iterator i = names.begin(); i != names.end(); ++i)
	{
		if(findChannelData(*i, data))
		{
			needToAdd.insert(std::make_pair(data.uri, data));
		}
	}

	// compare to the actual state
	SwgVivox::SessionMap const & currentSessions = SwgVivox::getInstance().GetSessions();

	for(SwgVivox::SessionMap::const_iterator sItr = currentSessions.begin(); sItr != currentSessions.end(); ++sItr)
	{
		SwgVivox::Session const & currentSession = sItr->second;

		UriToDataMap::iterator addItr = needToAdd.find(currentSession.sURI);
		if(addItr != needToAdd.end())
		{
			needToAdd.erase(addItr);
		}
		else
		{
			needToRemove.push_back(currentSession.sHandle);
		}
	}

	//disconnect from any channels we should not be in
	for (std::list<std::string>::const_iterator removeItr = needToRemove.begin(); removeItr != needToRemove.end(); ++removeItr)
	{
		SwgVivox::getInstance().TerminateSession(*removeItr);
	}

	//connect to any channels we are missing
	for(UriToDataMap::const_iterator addItr = needToAdd.begin(); addItr != needToAdd.end(); ++addItr)
	{
		VoiceChatChannelData const & thisData = addItr->second;
		BeginSession(thisData);
	}

	// set our speaking channel
	if(findChannelData(getSpeakingToChannel(), data))
	{
		CuiVoiceChatManagerNamespace::setVivoxSpeakingChannel(data.uri);
	}
	else
	{
		CuiVoiceChatManagerNamespace::setVivoxSpeakingChannel("");
	}

	return noChanges;
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::channelInvite(NetworkId const & invitee, std::string const & inviteeName, std::string const & channelName)
{
	if(channelName == getMyPrivateChannelName())
	{
		VoiceChatChannelData data;
		if(getInstance().findChannelData(channelName, data))
		{
			setChannelSimpleMode(channelName);

			VoiceChatInvite const msg(Game::getPlayerNetworkId(), channelName, invitee, inviteeName);
			GameNetwork::send (msg, true);

			Unicode::String str;
			CuiStringVariablesManager::process (CuiVoiceChatManagerNamespace::invitationSentMessage, Unicode::narrowToWide(inviteeName), Unicode::String (), Unicode::String (), str);
			CuiSystemMessageManager::sendFakeSystemMessage(str);
		}
		else
		{
			getInstance().m_pendingChannelAdds[inviteeName] = invitee;
			VoiceChatRequestPersonalChannel const msg(Game::getPlayerNetworkId(), true);
			GameNetwork::send (msg, true);
		}
	}
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::channelKick(NetworkId const & kickee, std::string const & kickeeName, std::string const & channelName)
{
	PendingChannelAddMap::iterator i = getInstance().m_pendingChannelAdds.find(kickeeName);
	if(i != getInstance().m_pendingChannelAdds.end())
	{
		getInstance().m_pendingChannelAdds.erase(i);
	}
	
	VoiceChatKick const msg(Game::getPlayerNetworkId(), channelName, kickee, kickeeName);
	GameNetwork::send (msg, true);
}

//----------------------------------------------------------------------------

std::string CuiVoiceChatManager::getMyPrivateChannelName()
{
	static std::string const game = "SWG";
	static std::string const channelType = "private";
	std::string loginId, cluster;
	Unicode::String playerName;
	NetworkId id;

	if(Game::getPlayerPath(loginId, cluster, playerName, id))
	{
		//we use first names 'round here
		std::string::size_type spacePos = playerName.find(' ');
		if (spacePos != std::string::npos)
		{
			playerName = playerName.substr(0, spacePos);
		}

		FormattedString<1024> buff;
		return buff.sprintf("%s.%s.%s.%s", game.c_str(), cluster.c_str(), channelType.c_str(), Unicode::wideToNarrow(playerName).c_str());
	}
	return "";
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::requestChannelInfoFromServer(std::string const & channelName)
{
	VoiceChatRequestChannelInfo msg(Game::getPlayerNetworkId(), channelName);
	GameNetwork::send (msg, true);
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setExistingSession(bool /*existingSession*/)
{
	/*if(existingSession)
		SwgVivox::getInstance().SetHandsOff(true);*/
	//getInstance().m_existingSession = existingSession;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::getExistingSession()
{
	return SwgVivox::getInstance().IsJoiningExistingSession();
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::forceLogin()
{
	if(SwgVivox::getInstance().isConnected())
	{
		getInstance().m_preventAutoLogin = false;
		getInstance().m_localLogoutRequest = true;
		
		SwgVivox::getInstance().SetHandsOff(false);

		SwgVivox::getInstance().Shutdown(true);
	}

}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setHandsOff(bool handsOff)
{
	SwgVivox::getInstance().SetHandsOff(handsOff);
}

//----------------------------------------------------------------------------

void CuiVoiceChatManager::setOtherPlayer(bool other)
{
	getInstance().m_otherPlayer = other;
}

//----------------------------------------------------------------------------

bool CuiVoiceChatManager::isOtherPlayer()
{
	return getInstance().m_otherPlayer;
}