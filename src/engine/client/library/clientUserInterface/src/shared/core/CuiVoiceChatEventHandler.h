// CuiVoiceChatEventHandler.h
// Steven Wyckoff

#ifndef INCLUDED_CuiVoiceChatEventHandler_h
#define INCLUDED_CuiVoiceChatEventHandler_h

#include "CuiVoiceChatGlue.h"
#include "CuiVoiceChatManager.h"

class SwgVivoxEventHandler
	: public SwgVivox::VivoxEventHandler
{
	// Connection events
	virtual void VivoxOnLogin( bool bLoggedIn );
	virtual void VivoxOnLoginStateChange( const std::string& sState );

	// Session events
	virtual void VivoxOnSessionAdded( const std::string& sSessionGroup, const std::string& sSession, const SwgVivox::Session& s );
	virtual void VivoxOnSessionRemoved( const std::string& sSessionGroup, const std::string& sSession, const SwgVivox::Session& s );
	virtual void VivoxOnMediaStreamUpdated( const std::string& sSessionGroup, const std::string& sSession, int iStatusCode, const std::string& sStatusString, SwgVivox::Session::State oldState, const SwgVivox::Session& s );
	virtual void VivoxVoiceServiceStateChange(bool bConnected);

	// Participant events
	virtual void VivoxOnParticipantAdded( const std::string& sSessionGroup, const std::string& sSession, const SwgVivox::Participant& p );
	virtual void VivoxOnParticipantRemoved( const std::string& sSessionGroup, const std::string& sSession, const SwgVivox::Participant& p, SwgVivox::RemoveReason reason );
	// Note! The oldP only contains the non-string elements of the Vivox::Participant structure
	virtual void VivoxOnParticipantUpdated( const std::string& sSessionGroup, const std::string& sSession, const SwgVivox::Participant& oldP, const SwgVivox::Participant& newP );

	// Other events
	virtual void VivoxOnVUMonitorStart();
	virtual void VivoxVUMonitor( float fMicVolume, float fSpeakerVolume, float fEnergy, bool bSpeaking );
	virtual void VivoxOnVUMonitorEnd();
	virtual void VivoxOnEchoTestStart();
	virtual void VivoxOnEchoTestEnd( u64 uLengthMS );
	virtual void VivoxOnEchoTestPlaybackStart( u64 uLengthMS );
	virtual void VivoxOnEchoTestPlaybackEnd( u64 uLengthMS );
	virtual void VivoxOnEchoTestError();

	// Device events
	virtual void VivoxOnGetOutputDevices( const std::string& sCurrent, const std::vector<std::string>& aDevices );
	virtual void VivoxOnGetInputDevices( const std::string& sCurrent, const std::vector<std::string>& aDevices );
	virtual void VivoxOnSetOutputDevice( const std::string& sDevice, bool bSucceeded );
	virtual void VivoxOnSetInputDevice( const std::string& sDevice, bool bSucceeded );

	// Station Launcher session stuff.
	virtual void VivoxOnJoinExistingSessionBegin();
	virtual void VivoxOnJoinExistingSessionEnd();

};

//TODO: this should live in a better place
namespace VivoxUtils
{
	void ConvertParticipantToActiveSpeaker(SwgVivox::Participant const & participant, CuiVoiceChatManager::ActiveSpeaker & speaker);

}


#endif