// CuiVoiceChatManager.h
// Steven Wyckoff

#ifndef _INCLUDED_CuiVoiceChatManager_h
#define _INCLUDED_CuiVoiceChatManager_h

#ifndef VIVOX_VERSION
#define VIVOX_VERSION 3
#endif

#include "sharedMessageDispatch/Receiver.h"
#include "Singleton/Singleton.h"

#include <string>
#include <set>
#include <map>
#include <list>
#include <vector>

class CreatureObject;

class SwgVivoxEventHandler;
class NetworkId;

namespace MessageDispatch
{
	class Callback;
}

class CuiVoiceChatManager 
	: public MessageDispatch::Receiver,
	  public Singleton<CuiVoiceChatManager>
{
public:

	//this will prolly need to get adjusted once we firm up what the server will
	//be sending us, basic idea is that channel data is everything we need to know
	//to connect to a channel. We may want another structure to represent a connected
	//channel (session) that couples this information to the channel handle used
	//by the vivox wrapper
	struct VoiceChatChannelData
	{
		VoiceChatChannelData() : name(), displayName(), uri(), password(), shortlistRemovable(false), hasModeratorPowers(false) {}
		VoiceChatChannelData(std::string const & _name, std::string const & _uri, std::string const & _password, bool _shortlistRemovable = false, bool _hasModeratorPowers = false)
			:name(_name), displayName(_name), uri(_uri), password(_password), shortlistRemovable(_shortlistRemovable), hasModeratorPowers(_hasModeratorPowers)
		{
			//@TODO: the display name really aught to be passed down from the server properly
			uint32 pos = displayName.find_last_of('.');
			if(pos != displayName.npos && pos + 1 < displayName.size())
			{
				displayName = displayName.substr(pos + 1);
			}
		}
		std::string name;
		std::string displayName;
		std::string uri;
		std::string password;
		bool shortlistRemovable;
		bool hasModeratorPowers;
		std::string toString() const {return displayName + ";" + name + "[" + uri + "]";}
	};

	typedef std::map<std::string, VoiceChatChannelData> VoiceChatChannelDataList;

	// this class wraps up all the information needed to make calls into the underlying
	// vivox wraper. CuiVoiceChatManager needs access to the contents, but nothing else
	// should care about it
	struct ParticipantId
	{
	public:
		friend class CuiVoiceChatManager;

		ParticipantId():m_uri(),m_sessionHandle(){}
		ParticipantId(ParticipantId const& p):m_uri(p.m_uri),m_sessionHandle(p.m_sessionHandle){}
		ParticipantId(std::string const& uri, std::string const &sessionHandle):m_uri(uri),m_sessionHandle(sessionHandle){}
		
		void clear() {m_uri.clear(); m_sessionHandle.clear();}
		bool isValid() const {return m_uri.size() > 0 && m_sessionHandle.size() > 0;}
		bool operator==(ParticipantId const& other)const {return m_uri == other.m_uri && m_sessionHandle == other.m_sessionHandle;}
		bool operator!=(ParticipantId const& other)const {return !(*this == other);}
		bool operator<(ParticipantId const&other)const {return m_sessionHandle < other.m_sessionHandle || m_uri < other.m_uri;}

	private:
		std::string m_uri;
		std::string m_sessionHandle;
	};

	// this struct packages the user's login information together, not sure if it is needed
	struct UserInfo
	{
		std::string m_userName;
		std::string m_userNameLower; // Internally the shared wrapper uses tolower on all user name strings, use this variable for comparisons to internal wrapper strings.
		std::string m_password;
		std::string m_connectionAddress;
	};

	// this struct represents all of the information relating to a channel participant
	// the vivox wrapper has its own internal representation but we don't want to force
	// all of swg to include the vivox headers
	struct ActiveSpeaker
	{
		ActiveSpeaker():id(),startedSpeaking(0),lastHeardFrom(0),energy(0.0f),volume(0.0f),speaking(false),muted(false){};

		bool isSameSpeaker(ActiveSpeaker const &o)const {return id == o.id;}

		ParticipantId id;
		std::string displayName;
		std::string channelName;
		unsigned startedSpeaking;
		unsigned lastHeardFrom;
		float energy;
		float volume;
		bool speaking;
		bool muted;
	};
	typedef std::map<ParticipantId, ActiveSpeaker> ActiveSpeakerMap;


public:

	struct Messages
	{
#define DECLARE_MESSAGE(T,P) struct T { typedef P Payload; }

		DECLARE_MESSAGE(VoiceChatEnabledChanged, bool);
		DECLARE_MESSAGE(VoiceChatServerDisabledChanged, bool);
		DECLARE_MESSAGE(LoginStateChanged, bool);
		DECLARE_MESSAGE(LocalSpeakerVolumeChanged, float);
		DECLARE_MESSAGE(LocalMicVolumeChanged, float);
		DECLARE_MESSAGE(LocalMicMuteChanged, bool);
		DECLARE_MESSAGE(LocalSpeakerMuteChanged, bool);
		DECLARE_MESSAGE(ShowFlybarChanged, bool);
		DECLARE_MESSAGE(PushToTalkModeChanged, bool);
		DECLARE_MESSAGE(PushToTalkKeyPressed, bool);
		DECLARE_MESSAGE(ChannelSelectionModeChanged, bool);
		DECLARE_MESSAGE(ChannelAddedToShortlist, VoiceChatChannelData);
		DECLARE_MESSAGE(ChannelRemovedFromShortlist, VoiceChatChannelData);
		DECLARE_MESSAGE(SessionStarted, VoiceChatChannelData);
		DECLARE_MESSAGE(SessionEnded, VoiceChatChannelData);
		DECLARE_MESSAGE(StartedListeningToChannel, VoiceChatChannelData);
		DECLARE_MESSAGE(StoppedListeningToChannel, VoiceChatChannelData);
		DECLARE_MESSAGE(SpeakingToChannelChanged, VoiceChatChannelData);
		DECLARE_MESSAGE(CurrentSpeakerChanged, int);//temp payload
		DECLARE_MESSAGE(ParticipantAdded, ActiveSpeaker);
		DECLARE_MESSAGE(ParticipantUpdated, ActiveSpeaker);
		DECLARE_MESSAGE(ParticipantRemoved, ActiveSpeaker);

		//TMYK: a pair is parsed as two comma separated arguments by the preprocessor
		typedef std::pair<std::string,std::vector<std::string> > DeviceListResult;
		DECLARE_MESSAGE(OutputDeviceList, DeviceListResult);
		DECLARE_MESSAGE(InputDeviceList, DeviceListResult);

		typedef std::pair<bool,std::string> DeviceSetResult;
		DECLARE_MESSAGE(OutputDeviceSet, DeviceSetResult);
		DECLARE_MESSAGE(InputDeviceSet, DeviceSetResult);

		DECLARE_MESSAGE(EchoTestRecordStart, bool); //no argument
		DECLARE_MESSAGE(EchoTestRecordStop, uint64);
		DECLARE_MESSAGE(EchoTestPlaybackStart, uint64);
		DECLARE_MESSAGE(EchoTestPlaybackStop, uint64);
		DECLARE_MESSAGE(OnVUMonitorUpdate, float);
		DECLARE_MESSAGE(OnVUMonitorModeChange, bool); //true for a start, false for an end

		DECLARE_MESSAGE(ExistingSessionChanged, bool); // True if we have an existing session from station launcher.


#undef DECLARE_MESSAGE
	};

public:
	CuiVoiceChatManager();
	~CuiVoiceChatManager();

	// Receiver interface
	virtual void receiveMessage(MessageDispatch::Emitter const & source, MessageDispatch::MessageBase const & message);


	static void install();
	static void remove();
	static void setVoiceChatEnabled(bool enabled);
	static bool getVoiceChatEnabled();

	static bool getVoiceChatDisabledByServer();

	static void update(float elapsedTime);

	static bool getUseAdvancedChannelSelection();
	static void setUseAdvancedChannelSelection(bool useit);

	// user info management
	static void setUserInfo(UserInfo const & info);
	static UserInfo const &  getUserInfo();
	static bool isLoggedIn();

	//channel management
	//shortlist-this is the list of channels that will appear on the fly bar list
	static void shortlistAddChannel(std::string const & name);
	static void shortlistRemoveChannel(std::string const & name);
	static bool isChannelShortlisted(std::string const & name);
	static std::set<std::string> const & getShortlistedChannels();

	static bool canChannelBeRemovedFromShortlist(std::string const & name);
	static bool doIHaveModeratorPowersInChannel(std::string const & name);
	static void errorConnectingToChannel(std::string const & channelUri, std::string const & statusString);

	static void requestJoinChannel(std::string const & name);
	static void requestLeaveChannel(std::string const &name);

	static bool isInChannel(std::string const & name);
	static bool getChannelDisplayName(std::string const & name, std::string & displayName);

	static void setListeningToChannel(std::string const & name, bool listening);
	static std::set<std::string> const & getListeningToChannels();
	
	static void setSpeakingToChannel(std::string const & name);
	static std::string const & getSpeakingToChannel();

	static void setChannelSimpleMode(std::string const & name);

	bool findChannelData(std::string const & name, VoiceChatChannelData & data);
	bool findChannelDataByUri(std::string const & uri, VoiceChatChannelData & data);
	void enterChannelData(VoiceChatChannelData const & data, bool overwrite = false);
	bool eraseChannelData(std::string const & name);
	static VoiceChatChannelDataList const & getKnownChannelData();

	static bool isParticipantMe(ParticipantId const & speaker);
	static bool isParticipantThisObject(ParticipantId const & speaker, NetworkId const & id);
	static bool isThisObjectSpeaking(NetworkId const & id);

	static void channelInvite(NetworkId const & invitee, std::string const & inviteeName, std::string const & channelName);
	static void channelKick(NetworkId const & kickee, std::string const & kickeeName, std::string const & channelName);
	static std::string getMyPrivateChannelName();

	static void requestChannelInfoFromServer(std::string const & channelName);

	// controls for remote speakers
	static void setLocalParticipantVolume(ParticipantId const & speaker, float gain);
	static float getLocalParticipantVolume(ParticipantId const & speaker);
	static bool setLocalParticipantMute(ParticipantId const & speaker, bool muted);
	static bool getLocalParticipantMute(ParticipantId const & speaker);

	static void setShowFlybar(bool yesno);
	static bool getShowFlybar();

	// controls for my devices
	static void setUsePushToTalk(bool yesno);
	static void setUsePushToTalkForceUpdate(bool yesno);
	static bool getUsePushToTalk();
	static bool getUsePushToTalkDefault();
	static void pushToTalkKeyPressed(bool down);
	static bool getPushToTalkKeyPressed();

	static void setLocalMicMute(bool muted);
	static void setLocalMicVolume(float gain);
	static float getLocalMicVolume();
	static float getLocalMicVolumeDefault();

	static void setLocalSpeakerMute(bool muted);
	static void setLocalSpeakerVolume(float gain);
	static float getLocalSpeakerVolume();
	static float getLocalSpeakerVolumeDefault();

	//device management
	static bool requestOutputDeviceList();
	static bool setOutputDevice(std::string const & device);
	static std::string const & getDefaultOutputDevice();
	static std::string const & getCurrentOutputDevice();
	static bool requestInputDeviceList();
	static bool setInputDevice(std::string const & device);
	static std::string const & getDefaultInputDevice();
	static std::string const & getCurrentInputDevice();

	static uint32 checkWindowsMicSettings(std::string const & device, bool fix = false);
	static bool canWindowsSettingsBeCorrected(uint32 checkMicResult);
	static Unicode::String getCheckMicString(uint32 checkMicResult);

	//Echo test
	static bool beginEchoTest();
	static bool stopEchoTest();
	static bool startEchoTestPlayback();
	static bool isPerformingEchoTest();
	static bool isPlayingEchoTest();
	static bool isEchoTestPlaybackAvailable();
	static bool beginVUMonitor();
	static bool endVUMonitor();
	static bool isVUMonitorActive();

	static void getAllParticipants(std::list<ActiveSpeaker> & allParticipants);
	static bool getParticipantData(ParticipantId const & id, ActiveSpeaker & data);
	static bool getCurrentSpeakerInfo(ActiveSpeaker & currentSpeaker);
	static ParticipantId const & getCurrentActiveSpeakerId();

	ActiveSpeakerMap& getCurrentSpeakerList();

	enum DebugOutputLevel
	{
		DOL_None = 0,
		DOL_Error = 1,
		DOL_Warning = 2,
		DOL_Info = 3,
		DOL_Debug = 4,
		DOL_Spam = 5,
	};

	static void setApiLogLevel(int level);
	static int getApiLogLevel();	
	static void setDebugOutputLevel(DebugOutputLevel level);
	static void debugOuputString(DebugOutputLevel level, std::string const & out);
	static void dumpDebugInfo(std::string & outStr);

	static std::string getCsReportString();

	static void setExistingSession(bool existingSession);
	static bool getExistingSession();

	static void forceLogin();
	static void setHandsOff(bool handsOff);

	static bool isOtherPlayer();
	static void setOtherPlayer(bool other);

private:

	void reset();

	bool doLoginStateUpdate();
	bool doChannelStateUpdate();

	bool updateCurrentSpeaker();
	bool getCurrentSpeakerInfoInternal(ActiveSpeaker & currentSpeaker);

	static bool BeginSession(VoiceChatChannelData const & data);
	static bool TerminateSession(VoiceChatChannelData const & data);

	void newChannelData(VoiceChatChannelData const & data, bool clobber, bool shortlist, bool join);

	void setChannelShortlistedState(VoiceChatChannelData const & data, bool shortlisted);
	void setChannelListeningState(VoiceChatChannelData const & data, bool listening);
	void setChannelSpeakingToState(VoiceChatChannelData & data);

	//message and transceiver processing functions
	typedef void(CuiVoiceChatManager::*MessageProcessorFunc)(MessageDispatch::Emitter const & source, MessageDispatch::MessageBase const & message);
	typedef std::map<uint32, MessageProcessorFunc> MessageProcessorMap;

	void setUpMessageProcessors();
	void processVoiceChatOnGetAccount(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message);
	void processVoiceChatChannelInfo(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message);
	void processVoiceChatOnGetChannel(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message);
	void processVoiceChatInvite(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message);
	void processVoiceChatKick(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message);
	void processVoiceChatStatus(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message);
	void processVCBroadcastMessage(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message);

	static void CuiVoiceChatManager::handleConsentWindow(int id, bool response);

	void onConnectionServerConnectionChanged(bool const &);
	void onIgnoreListChanged(bool const & status);

	MessageDispatch::Callback * m_callback;
	MessageProcessorMap m_messageProcessors;

	SwgVivoxEventHandler* m_eventHandler;
	ActiveSpeakerMap m_activeSpeakers;
	ParticipantId m_currentActiveSpeaker;

	static bool ms_voiceChatManagerInstalled;

	UserInfo m_userInfo;


	VoiceChatChannelDataList m_knownChannelData;

	std::set<std::string> m_shortlistedChannels;

	std::set<std::string> m_listeningToChannels;
	std::string m_speakingToChannel;

	//using our own push to talk so we can go through
	//our own keymap
	bool m_pushToTalkKeyDown;

	static DebugOutputLevel ms_debugOutputLevel;
	static int ms_vivoxLogLevel;

	std::string m_theSessionGroupHandle;

	typedef std::map<std::string, NetworkId> PendingChannelAddMap;
	PendingChannelAddMap m_pendingChannelAdds;

	std::map<int, VoiceChatChannelData> m_invitations;

	bool m_someoneWasSpeakingLastUpdate;

	bool m_disabledByServer;

	bool m_preventAutoLogin;
	bool m_localLogoutRequest;

	bool m_otherPlayer;

	//disable copy and assignment
	CuiVoiceChatManager(const CuiVoiceChatManager&);
	CuiVoiceChatManager &operator =(CuiVoiceChatManager const &);
};

#endif //_INCLUDED_CuiVoiceChatManager_h
