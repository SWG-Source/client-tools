// CuiVoiceChatEventHandler.cpp
// Steven Wyckoff

#include "clientUserInterface/FirstClientUserInterface.h"
#include "CuiVoiceChatEventHandler.h"

#include "CuiVoiceChatManager.h"

#include "clientGame/CommunityManager.h"
#include "sharedMessageDispatch/Transceiver.h"

#include <sstream>


namespace CuiVoiceChatEventHandlerNamespace
{
	namespace Transceivers
	{
#define MESSAGE_EMITER(T,N)MessageDispatch::Transceiver<const CuiVoiceChatManager::Messages::##T##::Payload &, CuiVoiceChatManager::Messages::##T##>N

		MESSAGE_EMITER(LoginStateChanged, loginStateChanged);

		MESSAGE_EMITER(ParticipantAdded,participantAdded);
		MESSAGE_EMITER(ParticipantUpdated,participantUpdated);
		MESSAGE_EMITER(ParticipantRemoved,participantRemoved);

		MESSAGE_EMITER(LocalSpeakerVolumeChanged,localVolumeChanged);
		MESSAGE_EMITER(LocalSpeakerMuteChanged,localSpeakerMuteChanged);
		MESSAGE_EMITER(LocalMicVolumeChanged,localMicVolumeChanged);
		MESSAGE_EMITER(LocalMicMuteChanged,localMicMuteChanged);
		MESSAGE_EMITER(SessionStarted,sessionStarted);
		MESSAGE_EMITER(SessionEnded,sessionEnded);

		MESSAGE_EMITER(OutputDeviceList,outputDeviceList);
		MESSAGE_EMITER(InputDeviceList,inputDeviceList);
		MESSAGE_EMITER(OutputDeviceSet,outputDeviceSet);
		MESSAGE_EMITER(InputDeviceSet,inputDeviceSet);

		MESSAGE_EMITER(EchoTestRecordStart, echoTestRecordStart);
		MESSAGE_EMITER(EchoTestRecordStop, echoTestRecordStop);
		MESSAGE_EMITER(EchoTestPlaybackStart, echoTestPlaybackStart);
		MESSAGE_EMITER(EchoTestPlaybackStop, echoTestPlaybackStop);
		MESSAGE_EMITER(OnVUMonitorUpdate, onVUMonitorUpdate);
		MESSAGE_EMITER(OnVUMonitorModeChange, onVUMonitorModeChange);

		MESSAGE_EMITER(ExistingSessionChanged, existingSessionChange);


#undef MESSAGE_EMITER
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

namespace VivoxUtils
{
	void ConvertParticipantToActiveSpeaker(SwgVivox::Participant const & participant, CuiVoiceChatManager::ActiveSpeaker & speaker)
	{
		speaker.id = CuiVoiceChatManager::ParticipantId(participant.sURI,participant.pSession->sHandle);

		//@TODO: this should look up the name based on the vivox account
		// and ask the server for the proper name if we don't know it already
		speaker.displayName = participant.sAccount;
		uint32 pos = speaker.displayName.find_last_of('.');
		if(pos != speaker.displayName.npos && pos + 1 < speaker.displayName.size())
		{
			speaker.displayName = speaker.displayName.substr(pos + 1);
		}

		speaker.startedSpeaking = 0;
		speaker.lastHeardFrom = 0;

		CuiVoiceChatManager::VoiceChatChannelData data;
		if(CuiVoiceChatManager::getInstance().findChannelDataByUri(participant.pSession->sURI,data))
		{
			speaker.channelName = data.name;
		}
		else
		{
			speaker.channelName = participant.pSession->sURI;
		}

		speaker.energy = participant.fEnergy;
		speaker.volume = participant.GetLocalVolume();
		speaker.speaking = participant.bSpeaking;
		speaker.muted = participant.GetLocalMute();
	}
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnSessionAdded(std::string const & sSessionGroup, std::string const & sSession, SwgVivox::Session const & s )
{
	UNREF(sSession);
	UNREF(sSessionGroup);
	CuiVoiceChatManager::VoiceChatChannelData data;

	if(CuiVoiceChatManager::getInstance().findChannelDataByUri(s.sURI,data))
	{
		//already have the info, sweet

	}
	else
	{
		data.uri = s.sURI;
		data.name = s.sDisplayName;
	}

	std::stringstream ss;
	ss << "Added session: " << data.name << " " << data.displayName << " " << data.uri;
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());

	CuiVoiceChatEventHandlerNamespace::Transceivers::sessionStarted.emitMessage(data);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnSessionRemoved(std::string const & sSessionGroup,std::string const & sSession, SwgVivox::Session const & s )
{
	UNREF(sSession);
	UNREF(sSessionGroup);
	CuiVoiceChatManager::VoiceChatChannelData data;

	if(CuiVoiceChatManager::getInstance().findChannelDataByUri(s.sURI,data))
	{
		//already have the info, sweet

	}
	else
	{
		data.uri = s.sURI;
		data.name = s.sDisplayName;
	}

	std::stringstream ss;
	ss << "Removed session: " << data.name << " " << data.displayName << " " << data.uri;
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());

	CuiVoiceChatEventHandlerNamespace::Transceivers::sessionEnded.emitMessage(data);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnMediaStreamUpdated( const std::string& sSessionGroup, const std::string& sSession, int iStatusCode, const std::string& sStatusString, SwgVivox::Session::State oldState, const SwgVivox::Session& s )
{
	UNREF(sSessionGroup);
	UNREF(sSession);
	UNREF(oldState);

	// check to see if this update indicates that we should not continue to auto connect to this channel in the future
	// assume that if the state now is disconnected and the status code is a 4xx code (client error) that we should
	// not retry automatically.

	if(s.eState == SwgVivox::Session::MS_DISCONNECTED)
	{
		std::stringstream ss;
		ss << "VivoxOnMediaStreamUpdated: " << sSession << " " << iStatusCode << " " << sStatusString << " " << oldState;
		CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Debug, ss.str());

		if(iStatusCode >= 400 && iStatusCode < 500)
		{
			CuiVoiceChatManager::errorConnectingToChannel(s.sURI, sStatusString);
		}
	}
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnParticipantAdded( const std::string& sSessionGroup, const std::string& sSession, const SwgVivox::Participant& p )
{
	UNREF(sSessionGroup);
	UNREF(sSession);
	UNREF(p);
	// oy, I need to think through the ownership of all of this information
	// I may just trigger a transceiver in here to pass the information along
	// in SWG land letting the wrapper manage all of the information
	CuiVoiceChatManager::ActiveSpeaker speaker;
	VivoxUtils::ConvertParticipantToActiveSpeaker(p, speaker);
	if(CommunityManager::isIgnored(Unicode::narrowToWide(speaker.displayName)))
	{
		CuiVoiceChatManager::setLocalParticipantMute(speaker.id, true);
	}
	CuiVoiceChatEventHandlerNamespace::Transceivers::participantAdded.emitMessage(speaker);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnParticipantRemoved( const std::string& sSessionGroup, const std::string& sSession, const SwgVivox::Participant& p, SwgVivox::RemoveReason reason )
{
	UNREF(sSessionGroup);
	UNREF(sSession);
	UNREF(reason);
	CuiVoiceChatManager::ActiveSpeaker speaker;
	VivoxUtils::ConvertParticipantToActiveSpeaker(p, speaker);

	CuiVoiceChatManager::ActiveSpeakerMap& activeSpeakers = CuiVoiceChatManager::getInstance().getCurrentSpeakerList();

	//see if we know about this speaker or not
	CuiVoiceChatManager::ParticipantId id(p.sURI,sSession);
	CuiVoiceChatManager::ActiveSpeakerMap::iterator i = activeSpeakers.find(id);
	if(i != activeSpeakers.end())
	{
		speaker = i->second;
		activeSpeakers.erase(i);
	}

	CuiVoiceChatEventHandlerNamespace::Transceivers::participantRemoved.emitMessage(speaker);
}

//----------------------------------------------------------------------------

// Note! The oldP only contains the non-string elements of the Vivox::Participant structure
void SwgVivoxEventHandler::VivoxOnParticipantUpdated( const std::string& sSessionGroup, const std::string& sSession, const SwgVivox::Participant& oldP, const SwgVivox::Participant& newP )
{
	UNREF(sSessionGroup);
	UNREF(sSession);
	UNREF(oldP);
	UNREF(newP);

	uint32 now = ::time(0);

	CuiVoiceChatManager::ActiveSpeakerMap& activeSpeakers = CuiVoiceChatManager::getInstance().getCurrentSpeakerList();

	//see if we already know about this speaker or not
	CuiVoiceChatManager::ParticipantId id(newP.sURI,sSession);
	CuiVoiceChatManager::ActiveSpeakerMap::iterator i = activeSpeakers.find(id);
	if(i == activeSpeakers.end())
	{
		//new entry
		CuiVoiceChatManager::ActiveSpeaker thisSpeaker;
		VivoxUtils::ConvertParticipantToActiveSpeaker(newP, thisSpeaker);

		if(thisSpeaker.speaking)
		{
			thisSpeaker.startedSpeaking = now;
			thisSpeaker.lastHeardFrom = now;

			activeSpeakers[id] = thisSpeaker;
		}

		CuiVoiceChatEventHandlerNamespace::Transceivers::participantUpdated.emitMessage(thisSpeaker);
	}
	else
	{
		CuiVoiceChatManager::ActiveSpeaker& thisSpeaker = i->second;

		if(newP.bSpeaking)
		{
			if(!thisSpeaker.speaking)
			{
				thisSpeaker.startedSpeaking = now;
			}			
			thisSpeaker.lastHeardFrom = now;
		}
		thisSpeaker.energy = newP.fEnergy;
		thisSpeaker.volume = newP.fVolume;
		thisSpeaker.speaking = newP.bSpeaking;
		thisSpeaker.muted = newP.GetLocalMute();

		CuiVoiceChatEventHandlerNamespace::Transceivers::participantUpdated.emitMessage(thisSpeaker);
	}

}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnLogin( bool bLoggedIn )
{
	if(!bLoggedIn)
	{
		std::stringstream ss;
		ss << "Failed to log in to voice chat server as " << CuiVoiceChatManager::getUserInfo().m_userName;
		CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());
	}
	else
	{
		std::string accountName = CuiVoiceChatEventHandlerNamespace::toLower(SwgVivox::getInstance().GetAccountName());
		CuiVoiceChatManager::UserInfo userInfo = CuiVoiceChatManager::getUserInfo();

		CuiVoiceChatManager::setOtherPlayer(false);

		if(strcmp(accountName.c_str(), userInfo.m_userNameLower.c_str()) != 0)
		{
			//Find out if this is another SWG client.
			// The URI should always be something like https://www.<swd/swp>.vivox.com
			bool otherGame = true;

			std::string::size_type firstPeriod = userInfo.m_connectionAddress.find('.');
			std::string::size_type secondPeriod = userInfo.m_connectionAddress.find('.', firstPeriod + 1);

			if(firstPeriod == std::string::npos || secondPeriod == std::string::npos)
				otherGame = true; // Malformed URI, this is bad.

			std::string gameCode = userInfo.m_connectionAddress.substr(firstPeriod + 1, (secondPeriod - firstPeriod - 1));
			std::string const SWD = "swd";
			std::string const SWP = "swp";
			
			if(gameCode == SWD || gameCode == SWP)
				otherGame = false;

			CuiVoiceChatManager::setHandsOff(otherGame);
			CuiVoiceChatManager::setOtherPlayer(!otherGame);
		}

		CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, std::string("Logged in to voice chat as ") + CuiVoiceChatManager::getUserInfo().m_userName);
		CuiVoiceChatManager::setLocalMicMute(CuiVoiceChatManager::getUsePushToTalk());
	}

}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnLoginStateChange( const std::string& sState )
{
	//for some reason this is a string, the possible values are:
	// "logged_in" "logged_out" "logging_in" "logging_out" "resetting" "error"

	CuiVoiceChatEventHandlerNamespace::Transceivers::loginStateChanged.emitMessage (sState == "logged_in");
}

//----------------------------------------------------------------------------

// Device events
void SwgVivoxEventHandler::VivoxOnGetOutputDevices( const std::string& sCurrent, const std::vector<std::string>& aDevices )
{
	std::stringstream ss;
	ss << "Get output devices current(" << sCurrent << ") list:";
	for(std::vector<std::string>::const_iterator i = aDevices.begin(); i != aDevices.end(); ++i)
	{
		ss << " " << *i;
	}
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());

	CuiVoiceChatEventHandlerNamespace::Transceivers::outputDeviceList.emitMessage(std::make_pair(sCurrent,aDevices));
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnGetInputDevices( const std::string& sCurrent, const std::vector<std::string>& aDevices )
{
	std::stringstream ss;
	ss << "Get input devices current(" << sCurrent << ") list:";
	for(std::vector<std::string>::const_iterator i = aDevices.begin(); i != aDevices.end(); ++i)
	{
		ss << " " << *i;
	}
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());

	CuiVoiceChatEventHandlerNamespace::Transceivers::inputDeviceList.emitMessage(std::make_pair(sCurrent,aDevices));
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnSetOutputDevice( const std::string& sDevice, bool bSucceeded ) 
{
	std::stringstream ss;
	ss << "Set output device " << sDevice << " result " << bSucceeded?"success":"failure";
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());

	CuiVoiceChatEventHandlerNamespace::Transceivers::outputDeviceSet.emitMessage(std::make_pair(bSucceeded,sDevice));
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnSetInputDevice( const std::string& sDevice, bool bSucceeded )
{
	std::stringstream ss;
	ss << "Set input device " << sDevice << " result " << bSucceeded?"success":"failure";
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());

	CuiVoiceChatEventHandlerNamespace::Transceivers::inputDeviceSet.emitMessage(std::make_pair(bSucceeded,sDevice));
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnVUMonitorStart()
{
	std::stringstream ss;
	ss << "VivoxOnVUMonitorStart";
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());
	CuiVoiceChatEventHandlerNamespace::Transceivers::onVUMonitorModeChange.emitMessage(true);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxVUMonitor( float fMicVolume, float fSpeakerVolume, float fEnergy, bool bSpeaking )
{
	std::stringstream ss;
	ss << "VivoxVUMonitor micVol(" << fMicVolume << ") speakerVol(" << fSpeakerVolume << ") energy(" << fEnergy <<") " << (bSpeaking?"speaking":"not speaking");
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Spam, ss.str());
	CuiVoiceChatEventHandlerNamespace::Transceivers::onVUMonitorUpdate.emitMessage(fEnergy);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnVUMonitorEnd()
{
	std::stringstream ss;
	ss << "VivoxOnVUMonitorEnd";
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());
	CuiVoiceChatEventHandlerNamespace::Transceivers::onVUMonitorModeChange.emitMessage(false);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnEchoTestStart()
{
	std::stringstream ss;
	ss << "VivoxOnEchoTestStart";
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());
	CuiVoiceChatEventHandlerNamespace::Transceivers::echoTestRecordStart.emitMessage(false);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnEchoTestEnd( u64 uLengthMS )
{
	std::stringstream ss;
	ss << "VivoxOnEchoTestEnd " << uLengthMS;
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());
	CuiVoiceChatEventHandlerNamespace::Transceivers::echoTestRecordStop.emitMessage(uLengthMS);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnEchoTestPlaybackStart( u64 uLengthMS )
{
	std::stringstream ss;
	ss << "VivoxOnEchoTestPlaybackStart " << uLengthMS;
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());
	CuiVoiceChatEventHandlerNamespace::Transceivers::echoTestPlaybackStart.emitMessage(uLengthMS);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnEchoTestPlaybackEnd( u64 uLengthMS )
{
	std::stringstream ss;
	ss << "VivoxOnEchoTestPlaybackEnd " << uLengthMS;
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Info, ss.str());
	CuiVoiceChatEventHandlerNamespace::Transceivers::echoTestPlaybackStop.emitMessage(uLengthMS);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnEchoTestError()
{
	std::stringstream ss;
	ss << "An error has occurred while attempting to perform an echo test.";
	CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Warning, ss.str());
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnJoinExistingSessionBegin()
{
	CuiVoiceChatManager::setExistingSession(true);
	CuiVoiceChatEventHandlerNamespace::Transceivers::existingSessionChange.emitMessage (true);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxOnJoinExistingSessionEnd()
{
	CuiVoiceChatManager::setExistingSession(false);
	CuiVoiceChatEventHandlerNamespace::Transceivers::existingSessionChange.emitMessage (false);
}

//----------------------------------------------------------------------------

void SwgVivoxEventHandler::VivoxVoiceServiceStateChange(bool bConnected)
{
	if(!bConnected)
		SwgVivox::getInstance().SetHandsOff(false);
}

//----------------------------------------------------------------------------