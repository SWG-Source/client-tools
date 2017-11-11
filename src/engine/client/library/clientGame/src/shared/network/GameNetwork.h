// GameNetwork.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_GAME_NETWORK_H
#define	_GAME_NETWORK_H

//-----------------------------------------------------------------------

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/StationId.h"
#include <string>

namespace MessageDispatch
{
	class Emitter;
	class MessageBase;
}

class CmdStartScene;
class Connection;
class ConnectionServerConnection;
class GameNetworkConnection;
class GameNetworkMessage;
class LoginClientToken;
class NetworkScene;
class ObjControllerMessage;
class Object;
class ParametersMessage;
class TaskConnection;
class ConGenericMessage;
class Vector;

//-----------------------------------------------------------------------

class GameNetwork
{
public:

	struct Messages
	{
		struct LoginServerConnectionChanged
		{
			typedef bool Payload;
		};

		struct ConnectionServerConnectionChanged
		{
			typedef bool Payload;
		};

	};
	
	static void                        addObject               (Object * newObject);
	static void                        connectLoginServer      (const std::string & address, const unsigned short port);
	static void                        connectConnectionServer (const std::string & address, const unsigned short port, const std::string & name);

	static void                        createCharacter         (const Unicode::String & characterName, 
	                                                       const std::string & templateName, 
	                                                       const std::string & startingLocation);
	static void                        disconnectLoginServer   ();
	static void                        disconnectConnectionServer ();

	static bool                        isConnectedToLoginServer ();
	static bool                        isConnectedToConnectionServer();

	static Object *                    findObject              (NetworkId id);
	static const NetworkScene *        getCurrentScene         ();
	static const unsigned char * const getLoginToken           ();
	static const uint16                getLoginTokenLength     ();
	static const std::string &         getUserName             ();
	static const std::string &         getUserPassword         ();
	static const std::string &         getUserIpAddress        ();
	static uint16                      getUserPort             ();
	static const ConnectionServerConnection* getConnectionServerConnection();
	static const GameNetworkConnection* getLoginServerConnection();
	static uint16                      getServerSyncStampShort ();
	static uint32                      getServerSyncStampLong  ();
	static int                         syncStampShortDeltaTime (uint16 time1, uint16 time2);
	static int                         syncStampLongDeltaTime  (uint32 time1, uint32 time2);
	static float computeDeltaTimeInSeconds(uint32 syncStampLong);
	static StationId const &           getStationId            ();
	static int                         getServerEpochTimeDifference();
	static void                        setServerEpochTimeDifference(time_t serverEpoch);

	static void                        removeObject            (Object * object);
	static void                        update                  ();

	static void                        beginDeferringConnectionServerMessages();
	static void                        endDeferringConnectionServerMessages(bool emitDeferredPackets=true);

	static void                        send                    (const GameNetworkMessage & outboundMessage, bool reliable);
	static void                        sendToLoginServer       (const GameNetworkMessage & outboundMessage, bool reliable);
	static void                        sendTaskCommand         (const ConGenericMessage & command);
	static void                        setSceneChannel         ();
	static void                        setUserName             (const std::string & newUserName);
	static void                        setUserPassword         (const std::string & newUserPassword);
	static void                        startScene              (const std::string & sceneName,
	                                                     const NetworkId & characterObjectId,
	                                                     const std::string & characterTemplateName,
	                                                     const Vector & startPosition,
	                                                     const float startYaw,
														 float timeInSeconds,
														 bool disableSnapshot);

	static const std::string &         getCentralServerName    ();

	static void                        setAcceptSceneCommand   (bool b);

	static int                         getWeatherUpdateInterval();

	static void                        updateServerWithJediSlotInfo();

	static stdmap<std::string, stdmap<std::string, std::pair<std::pair<float, float>, float> >::fwd >::fwd const & getGcwScoreCategoryRegions();
	static stdmap<std::string, stdmap<std::string, int>::fwd >::fwd const & getGcwScoreCategoryGroups();

	static void                 install                 ();
	static void                 remove                  ();

	// Protect the constructor and destructor so that they
	// may only be instantiated via the singleton interface

private:
	GameNetwork   ();
	~GameNetwork  ();
	void                     receiveConnectionServerConnectionOpened ();
	void                     receiveLoginClientToken                 (const LoginClientToken & t);
	void                     receiveCmdStartScene                    (const CmdStartScene &s);
	void                     receiveObjControllerMessage             (ObjControllerMessage & c);
	void                     receiveParametersMessage                (const ParametersMessage & msg);
	void                     receiveConnectionServerConnectionClosed ();
	void                     receiveLoginServerClosed                ();

	class Listener;
	friend Listener;

	Listener *               m_listener;
	ConnectionServerConnection *  m_connectionServer;
	NetworkScene *           m_currentScene;
	Connection *             m_loginServer;
	unsigned char *          m_loginToken;
	std::string              m_userName;
	std::string              m_userPassword;
	std::string              m_userIpAddress;
	std::string              m_centralServerName;
	std::string              m_pendingCentralServerName;
	uint16                   m_loginTokenLength;
	uint16                   m_userPort;
	bool                     m_acceptSceneCommand;
	StationId                m_stationId;
	TaskConnection *         m_taskConnection;
	int                      m_weatherUpdateInterval;
};

//-------------------------------------------------------------------

#endif	// _GAME_NETWORK_H
