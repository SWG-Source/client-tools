//
// GameNetwork.cpp
// jrandall 8 September 2000
//
// copyright 2000, verant interactive, inc.
//

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/GameNetwork.h"

#include "clientGame/ClientController.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ConnectionServerConnection.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/LoginConnection.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientGame/TaskConnection.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiLoadingManager.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiLoginManagerAvatarInfo.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"
#include "sharedNetworkMessages/CommandChannelMessages.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/PlayCutSceneMessage.h"
#include "sharedNetworkMessages/ErrorMessage.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/ParametersMessage.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedRemoteDebugServer/SharedRemoteDebugServer.h"
#include "sharedUtility/FileName.h"

//----------------------------------------------------------------------

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const GameNetwork::Messages::LoginServerConnectionChanged::Payload &, GameNetwork::Messages::LoginServerConnectionChanged >
			loginServerConnectionChanged;

		MessageDispatch::Transceiver<const GameNetwork::Messages::ConnectionServerConnectionChanged::Payload &, GameNetwork::Messages::ConnectionServerConnectionChanged >
			connectionServerConnectionChanged;
	}

	GameNetwork * s_instance = 0;

	char ms_clusterName[128];
	char ms_userName[128];

	// client Epoch time = server Epoch time + s_serverEpochTimeDifference
	int s_serverEpochTimeDifference = 0;

	// std::map<planetName, std::map<gcwScoreCategory, std::pair<std::pair<regionCenterX, regionCenterZ>, regionRadius> > >
	std::map<std::string, std::map<std::string, std::pair<std::pair<float, float>, float> > > s_gcwScoreCategoryRegions;

	// std::map<gcwScoreGroup, std::map<gcwScoreCategory, % contribution> >
	// % contribution is expressed as a value out of 1,000,000,000
	std::map<std::string, std::map<std::string, int> > s_gcwScoreCategoryGroups;
}

//----------------------------------------------------------------------

class GameNetwork::Listener :
public MessageDispatch::Receiver
{
public:
	Listener ();
	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
};

//----------------------------------------------------------------------

GameNetwork::Listener::Listener () :
MessageDispatch::Receiver ()
{
	connectToMessage("LoginConnectionOpened");
	connectToMessage("LoginConnectionClosed");
	connectToMessage("ConnectionServerConnectionOpened");
	connectToMessage("ConnectionServerConnectionClosed");
	connectToMessage("LoginClientToken");
	connectToMessage("SetSessionKey");
	connectToMessage("LoginIncorrectClientId");
	connectToMessage("GameConnectionClosed");
	connectToMessage("CmdStartScene");
	connectToMessage("PlayCutSceneMessage");
	connectToMessage("ObjControllerMessage");
	connectToMessage("ErrorMessage");
	connectToMessage("ParametersMessage");
	connectToMessage("ServerNowEpochTime");
	connectToMessage("CollectionServerFirstListResponse");
	connectToMessage("CharacterChangedUnlocked");
	connectToMessage("CharacterChangedNormal");
	connectToMessage("CharacterNameChanged");
	connectToMessage("GcwRegionsRsp");
	connectToMessage("GcwGroupsRsp");
	connectToMessage("OpenExamineWindow");
}

//-----------------------------------------------------------------------

void GameNetwork::Listener::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	NOT_NULL (s_instance);
	if(message.isType("LoginConnectionOpened"))
	{
		DEBUG_REPORT_LOG(true, ("Connection with Login Server established\n"));
		Transceivers::loginServerConnectionChanged.emitMessage (true);
	}
	else if(message.isType("LoginConnectionClosed"))
	{
		s_instance->receiveLoginServerClosed ();
	}
	else if(message.isType("ConnectionServerConnectionOpened"))
	{
		s_instance->receiveConnectionServerConnectionOpened ();
	}
	else if(message.isType("ConnectionServerConnectionClosed"))
	{
		s_instance->receiveConnectionServerConnectionClosed ();
	}
	else if(message.isType("LoginClientToken"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const LoginClientToken t(ri);
		s_instance->receiveLoginClientToken (t);
	}
	else if(message.isType("SetSessionKey"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const GenericValueTypeMessage<std::string> m(ri);
		CuiLoginManager::setOverrideSessionIdKey(m.getValue());
	}
	else if(message.isType("LoginIncorrectClientId"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const LoginIncorrectClientId msg(ri);
		DEBUG_REPORT_LOG(true,("Client / server network version mismatch, client: %s server: %s\n", (GameNetworkMessage::NetworkVersionId).c_str(), msg.getServerId().c_str()));
		s_instance->disconnectLoginServer ();
	}
	else if(message.isType("CmdStartScene"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const CmdStartScene s(ri);
		setServerEpochTimeDifference(static_cast<time_t>(s.getServerEpoch()));
		s_instance->receiveCmdStartScene (s);
	}
	else if(message.isType("PlayCutSceneMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const PlayCutSceneMessage s(ri);
		const std::string &cutScene = s.getCutSceneName();
		Game::playCutScene(cutScene.c_str(), true);
	}
	else if(message.isType("ObjControllerMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		ObjControllerMessage c(ri);

		s_instance->receiveObjControllerMessage (c);
	}

	else if (message.isType("ErrorMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const ErrorMessage e(ri);
		FATAL(e.getFatal(), ("Server Forced Fatal: %s : %s", e.getErrorName().c_str(), e.getDescription().c_str()));
		DEBUG_REPORT_LOG(true,("%s: %s\n", e.getErrorName().c_str(), e.getDescription().c_str()));
	}
	else if (message.isType("ParametersMessage"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const ParametersMessage msg(ri);

		s_instance->receiveParametersMessage (msg);
	}
	else if (message.isType("ServerNowEpochTime"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const GenericValueTypeMessage<int32> serverNowEpochTime(ri);

		setServerEpochTimeDifference(static_cast<time_t>(serverNowEpochTime.getValue()));
	}
	else if (message.isType("CollectionServerFirstListResponse"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const GenericValueTypeMessage<std::pair<std::string, std::set<std::pair<std::pair<int32, std::string>, std::pair<NetworkId, Unicode::String> > > > > collectionServerFirst(ri);
		Game::responseCollectionServerFirstList(collectionServerFirst.getValue().first, collectionServerFirst.getValue().second);
	}
	else if (message.isType("CharacterChangedUnlocked"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::string, NetworkId> > const characterChangedUnlocked(ri);
		CuiLoginManager::characterChangedUnlocked(characterChangedUnlocked.getValue().first, characterChangedUnlocked.getValue().second);
	}
	else if (message.isType("CharacterChangedNormal"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::string, NetworkId> > const characterChangedNormal(ri);
		CuiLoginManager::characterChangedNormal(characterChangedNormal.getValue().first, characterChangedNormal.getValue().second);
	}
	else if (message.isType("CharacterNameChanged"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::pair<std::string, NetworkId>, Unicode::String> > const characterNameChanged(ri);
		CuiLoginManager::characterNameChanged(characterNameChanged.getValue().first.first, characterNameChanged.getValue().first.second, characterNameChanged.getValue().second);
	}
	else if (message.isType("GcwRegionsRsp"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		GenericValueTypeMessage<std::map<std::string, std::map<std::string, std::pair<std::pair<float, float>, float> > > > const gcwRegionsRsp(ri);
		s_gcwScoreCategoryRegions = gcwRegionsRsp.getValue();
	}
	else if (message.isType("GcwGroupsRsp"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		GenericValueTypeMessage<std::map<std::string, std::map<std::string, int> > > const gcwGroupsRsp(ri);
		s_gcwScoreCategoryGroups = gcwGroupsRsp.getValue();
	}
	else if (message.isType("OpenExamineWindow"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		GenericValueTypeMessage<NetworkId> const openExamineWindowMsg(ri);
		CuiActionManager::performAction(CuiActions::examine, Unicode::narrowToWide(openExamineWindowMsg.getValue().getValueString()));
	}
}

//-----------------------------------------------------------------------

GameNetwork::GameNetwork     () :
m_listener                   (new Listener),
m_connectionServer           (0),
m_loginServer                (0),
m_loginToken                 (0),
m_userName                   (ConfigClientGame::getLoginClientID()),
m_userPassword               (ConfigClientGame::getLoginClientPassword()),
m_userIpAddress              (),
m_centralServerName          (),
m_pendingCentralServerName   (),
m_loginTokenLength           (0),
m_userPort                   (0),
m_acceptSceneCommand         (false),
m_stationId                  (static_cast<StationId>(ConfigFile::getKeyInt("Station", "stationId", 0))),
m_taskConnection             (0),
m_weatherUpdateInterval      (0)
{
	uint16 taskConnectionPort = ConfigClientGame::getTaskConnectionPort();
	if(taskConnectionPort)
	{
		std::string taskConnectionAddress = ConfigClientGame::getTaskConnectionAddress();
		if(taskConnectionAddress.length())
		{
			m_taskConnection = new TaskConnection(taskConnectionAddress, taskConnectionPort);
		}
	}

	SharedRemoteDebugServer::install();
	strcpy(ms_clusterName, "Cluster: unknown\n");
	CrashReportInformation::addDynamicText(ms_clusterName);

	snprintf(ms_userName, sizeof(ms_userName), "User: %d unknown\n", static_cast<int>(m_stationId));
	ms_userName[sizeof(ms_userName)-1] = '\0';
	if (!ConfigClientGame::getAnonymousCrashReports ())
		CrashReportInformation::addDynamicText(ms_userName);

	CrashReportInformation::addStaticText("Contact: %s\n", ConfigClientGame::getAllowCustomerContact() ? "1" : "0");
}

//-----------------------------------------------------------------------

GameNetwork::~GameNetwork()
{
	CrashReportInformation::removeDynamicText(ms_clusterName);
	if (!ConfigClientGame::getAnonymousCrashReports ())
		CrashReportInformation::removeDynamicText(ms_userName);

	delete m_taskConnection;

	delete m_listener;
	m_listener = 0;

	delete[] m_loginToken;
	m_loginToken = 0;

	delete m_connectionServer;
	m_connectionServer = 0;

	if (m_loginServer)
	{
		m_loginServer->disconnect ();
		m_loginServer = 0;
	}

	SharedRemoteDebugServer::remove();
}

//-----------------------------------------------------------------------

void GameNetwork::addObject(Object * object)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	DEBUG_FATAL(object->getObjectType() == 0, ("bad object type from network!\n"));
	ClientWorld::queueObject(object);
}

//-----------------------------------------------------------------------

void GameNetwork::connectConnectionServer(const std::string & address, const unsigned short port, const std::string & name)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	disconnectConnectionServer();
	NetworkSetupData setupData;
	setupData.useTcp = false;
	s_instance->m_connectionServer = new ConnectionServerConnection(address, port, setupData);
	s_instance->m_pendingCentralServerName = name;
}

//-----------------------------------------------------------------------

void GameNetwork::connectLoginServer(const std::string & address, const unsigned short port)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	disconnectLoginServer();
	NetworkSetupData setupData;
	setupData.useTcp = false;
	s_instance->m_loginServer = new LoginConnection(address, port, setupData);
}

//-----------------------------------------------------------------------

void GameNetwork::disconnectConnectionServer()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));

	if (s_instance->m_connectionServer)
	{
		s_instance->m_connectionServer->disconnect();
		s_instance->m_connectionServer = 0;
		Transceivers::connectionServerConnectionChanged.emitMessage (true);
	}
}

//-----------------------------------------------------------------------

void GameNetwork::disconnectLoginServer()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	if (s_instance->m_loginServer)
	{
		s_instance->m_loginServer->disconnect ();
		s_instance->m_loginServer = 0;
	}
}

//-----------------------------------------------------------------------

Object * GameNetwork::findObject(NetworkId id)
{
	return NetworkIdManager::getObjectById(id);
}

//-----------------------------------------------------------------------

void GameNetwork::removeObject(Object * object)
{
	object->removeFromWorld();
}

//-----------------------------------------------------------------------

void GameNetwork::send(const GameNetworkMessage & outboundMessage, bool reliable)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	if(s_instance->m_connectionServer)
	{
		static Archive::ByteStream out;
		out.clear();
		outboundMessage.pack(out);
		s_instance->m_connectionServer->send(out, reliable);
	}
	else
		WARNING (!Game::getSinglePlayer (), ("GameNetwork::send attempt to send message [%s] with no connection server connection.", outboundMessage.getCmdName ().c_str ()));
}

//----------------------------------------------------------------------

void GameNetwork::sendToLoginServer       (const GameNetworkMessage & outboundMessage, bool reliable)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	if (s_instance->m_loginServer)
	{
		static Archive::ByteStream out;
		out.clear();
		outboundMessage.pack(out);
		s_instance->m_loginServer->send(out, reliable);
	}
}

//-----------------------------------------------------------------------

void GameNetwork::setSceneChannel()
{
	if (s_instance->m_connectionServer)
	{
		const CmdSceneReady	s;
		GameNetwork::send(s, true);

		GenericValueTypeMessage<int> const msg1("SetCombatSpamFilter", CuiPreferences::getCombatSpamFilter());
		GameNetwork::send(msg1, true);

		GenericValueTypeMessage<int> const msg2("SetCombatSpamRangeFilter", CuiPreferences::getCombatSpamRangeFilter());
		GameNetwork::send(msg2, true);

		GenericValueTypeMessage<int> const msg3("SetFurnitureRotationDegree", CuiPreferences::getFurnitureRotationDegree());
		GameNetwork::send(msg3, true);

		if (s_gcwScoreCategoryRegions.empty())
		{
			GameNetworkMessage const msg4("GcwRegionsReq");
			GameNetwork::send(msg4, true);
		}

		CuiActionManager::performAction("sendSavedPlayerInterestsToServer", Unicode::String());		

		updateServerWithJediSlotInfo();
	}
}

//-----------------------------------------------------------------------

void GameNetwork::setUserName (const std::string & newUserName)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	s_instance->m_userName = newUserName;
}

//-------------------------------------------------------------------

void GameNetwork::setUserPassword (const std::string & newUserPassword)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	s_instance->m_userPassword = newUserPassword;
}

//-----------------------------------------------------------------------

void GameNetwork::startScene(const std::string & sceneName, const NetworkId & objectId, const std::string & templateName, const Vector & startPosition, const float startYaw, const float timeInSeconds, bool disableSnapshot)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	//First destroy the old scene if need be.
	{
		GroundScene * const gs = dynamic_cast<GroundScene*> (Game::getScene ());
		if (gs != 0)
		{
			gs->close ();
			delete gs;
			setAcceptSceneCommand(true);
		}
	}

	//@todo we ONLY support ground scenes now so let all messages regardless of type go through
	Game::setScene(
		false,
		sceneName.c_str(), 
		objectId, 
		templateName.c_str(), 
		startPosition, 
		startYaw, 
		timeInSeconds, 
		disableSnapshot
	);
}

//-----------------------------------------------------------------------

void GameNetwork::update(void)
{
	if (!s_instance)
		return;

	NetworkHandler::update();
	NetworkHandler::dispatch();
	if(s_instance->m_connectionServer)
	{
		s_instance->m_connectionServer->updateRates();
		s_instance->m_connectionServer->updateLagStats();
	}
	NetworkHandler::clearBytesThisFrame();
}

//----------------------------------------------------------------------

const ConnectionServerConnection* GameNetwork::getConnectionServerConnection()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_connectionServer;
}

//-------------------------------------------------------------------

const GameNetworkConnection* GameNetwork::getLoginServerConnection()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return dynamic_cast<GameNetworkConnection*>(s_instance->m_loginServer);
}

//----------------------------------------------------------------------

uint16 GameNetwork::getServerSyncStampShort()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	if (s_instance->m_connectionServer)
		return s_instance->m_connectionServer->getServerSyncStampShort();
	return 0;
}

//----------------------------------------------------------------------

uint32 GameNetwork::getServerSyncStampLong()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	if (s_instance->m_connectionServer)
		return s_instance->m_connectionServer->getServerSyncStampLong();
	return 0;
}

//----------------------------------------------------------------------

int GameNetwork::syncStampShortDeltaTime(uint16 const time1, uint16 const time2)
{
	uint16 const deltaTimeMilliseconds = static_cast<uint16>(time2-time1);

	if (deltaTimeMilliseconds > 0x7fff)
		return static_cast<int>(0xffff - deltaTimeMilliseconds);
	else
		return static_cast<int>(deltaTimeMilliseconds);
}


//----------------------------------------------------------------------

int GameNetwork::syncStampLongDeltaTime(uint32 const time1, uint32 const time2)
{
	uint32 const deltaTimeMilliseconds = time2-time1;

	if (deltaTimeMilliseconds > 0x7fffffff)
		return static_cast<int>(0xffffffff - deltaTimeMilliseconds);
	else
		return static_cast<int>(deltaTimeMilliseconds);
}

//----------------------------------------------------------------------

float GameNetwork::computeDeltaTimeInSeconds(uint32 const syncStampLong)
{
	return static_cast<float>(syncStampLongDeltaTime(getServerSyncStampLong(), syncStampLong)) / 1000.f;
}

//----------------------------------------------------------------------

void GameNetwork::setAcceptSceneCommand   (bool b)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	s_instance->m_acceptSceneCommand = b;
}

//----------------------------------------------------------------------

void GameNetwork::receiveConnectionServerConnectionOpened ()
{
	m_centralServerName = m_pendingCentralServerName;
	snprintf(ms_clusterName, sizeof(ms_clusterName), "Cluster: %s\n", m_centralServerName.c_str());
	m_pendingCentralServerName.clear ();
	ResourceTypeManager::reset    ();
	ObjectAttributeManager::reset ();
	DEBUG_REPORT_LOG(true, ("Connection with the User Server established\n"));
	Transceivers::connectionServerConnectionChanged.emitMessage (true);
}

//----------------------------------------------------------------------

void GameNetwork::receiveLoginClientToken (const LoginClientToken & t)
{
	delete[] m_loginToken;
	m_loginTokenLength = t.getTokenSize();
	m_loginToken       = new unsigned char[m_loginTokenLength];
	memcpy(m_loginToken, t.getToken(), m_loginTokenLength);
	m_stationId        = static_cast<StationId>(t.getStationId());
	setUserName         (t.getUsername ());

	snprintf(ms_userName, sizeof(ms_userName), "User: %d %s\n", static_cast<int>(m_stationId), t.getUsername().c_str());
	ms_userName[sizeof(ms_userName)-1] = '\0';
}

//----------------------------------------------------------------------

void GameNetwork::receiveCmdStartScene (const CmdStartScene &s)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	if (m_acceptSceneCommand)
	{
		//@todo for now we only support ground scenes.
		startScene(s.getSceneName(), s.getObjectId(), s.getTemplateName(), s.getStartPosition(), s.getStartYaw(), static_cast<float> (s.getTimeSeconds()), s.getDisableWorldSnapshot());
	}
	else
		WARNING (true, ("Discarding CmdStartScene message because we don't want to enter a scene right now."));
}

//----------------------------------------------------------------------

void GameNetwork::receiveObjControllerMessage (ObjControllerMessage & c)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	bool  appended = false;

	Object * const object = NetworkIdManager::getObjectById(c.getNetworkId());
	if(object)
	{
		if(object->asClientObject() && (! object->asClientObject()->getController()))
		{
			object->asClientObject()->createDefaultController();
			object->scheduleForAlter();
		}

		// get the controller
		NetworkController * const networkController = dynamic_cast<NetworkController *>(object->getController());
		if(networkController)
		{
			//	This is a valid controller, message is either allocated or 0.
			//	Append it to the controller's message queue so that it may be
			//	processed during alter.

			ClientController * const clientController = dynamic_cast<ClientController*>(networkController);

			if (clientController)
			{
				uint32 flags = c.getFlags();
				flags &= ~(GameControllerMessageFlags::SEND); //Clear the send flag.  We shouldn't broadcast anything we receive from the network.
				flags |= GameControllerMessageFlags::SOURCE_REMOTE_SERVER;

				networkController->appendMessage(c.getMessage(), c.getValue(), c.getData(), flags);
				appended = true;
			}
		}
	}
	if (!appended)
	{
		delete c.getData();
	}
}

//----------------------------------------------------------------------

void GameNetwork::receiveParametersMessage (const ParametersMessage & msg)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	m_weatherUpdateInterval = msg.getWeatherUpdateInterval();
}

//----------------------------------------------------------------------

void GameNetwork::receiveConnectionServerConnectionClosed ()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	m_centralServerName.clear ();
	m_connectionServer = 0;
	DEBUG_REPORT_LOG(true, ("Connection with User server closed\n"));
	Transceivers::connectionServerConnectionChanged.emitMessage (true);
}

//-----------------------------------------------------------------------

void GameNetwork::sendTaskCommand(const ConGenericMessage & command)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	if(s_instance->m_taskConnection)
	{
		s_instance->m_taskConnection->send(command);
	}
}

//----------------------------------------------------------------------

void GameNetwork::receiveLoginServerClosed ()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	DEBUG_REPORT_LOG(true, ("Connection with login server closed\n"));
	m_loginServer = 0;
	Transceivers::loginServerConnectionChanged.emitMessage (false);
}

//----------------------------------------------------------------------

void GameNetwork::install()
{
	InstallTimer const installTimer("GameNetwork::install");

	DEBUG_FATAL(s_instance != 0, ("GameNetwork is already installed"));
	s_instance = new GameNetwork;
	ExitChain::add(remove, "GameNetwork::remove");
}

//---------------------------------------------------------------------


void GameNetwork::remove()
{
	delete s_instance;
}

//----------------------------------------------------------------------

bool GameNetwork::isConnectedToLoginServer ()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_loginServer != 0;
}

//----------------------------------------------------------------------

bool GameNetwork::isConnectedToConnectionServer()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_connectionServer != 0;
}

//-----------------------------------------------------------------------

const NetworkScene * GameNetwork::getCurrentScene(void)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_currentScene;
}

//-----------------------------------------------------------------------

const unsigned char * const GameNetwork::getLoginToken(void)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_loginToken;
}

//-----------------------------------------------------------------------

const uint16 GameNetwork::getLoginTokenLength(void)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_loginTokenLength;
}

//-----------------------------------------------------------------------

const std::string & GameNetwork::getUserName (void)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_userName;
}

//-------------------------------------------------------------------

const std::string & GameNetwork::getUserPassword (void)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_userPassword;
}

//-------------------------------------------------------------------

const std::string & GameNetwork::getUserIpAddress (void)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_userIpAddress;
}

//-------------------------------------------------------------------

uint16 GameNetwork::getUserPort (void)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_userPort;
}

//----------------------------------------------------------------------

const std::string & GameNetwork::getCentralServerName    ()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_centralServerName;
}

//-------------------------------------------------------------------

StationId const &GameNetwork::getStationId()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_stationId;
}

//-------------------------------------------------------------------

int GameNetwork::getServerEpochTimeDifference()
{
	return s_serverEpochTimeDifference;
}

//-------------------------------------------------------------------

void GameNetwork::setServerEpochTimeDifference(time_t serverEpoch)
{
	s_serverEpochTimeDifference = static_cast<int>(::time(NULL) - serverEpoch);
}

//-------------------------------------------------------------------

int GameNetwork::getWeatherUpdateInterval()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	return s_instance->m_weatherUpdateInterval;
}

//-------------------------------------------------------------------

void GameNetwork::updateServerWithJediSlotInfo()
{
	const CuiLoginManagerAvatarInfo * const avatarInfo = CuiLoginManager::findAvatarInfo(CuiLoginManager::getConnectedClusterId(), Game::getPlayerNetworkId());
	bool const playerIsJediSlotCharacter = (avatarInfo && (avatarInfo->characterType == static_cast<int>(EnumerateCharacterId_Chardata::CT_jedi)));

	GenericValueTypeMessage<std::pair<bool, bool> > const msg("SetJediSlotInfo", std::make_pair(CuiLoginManager::hasUnoccupiedJediSlot(), playerIsJediSlotCharacter));
	GameNetwork::send(msg, true);
}

//-------------------------------------------------------------------

std::map<std::string, std::map<std::string, std::pair<std::pair<float, float>, float> > > const & GameNetwork::getGcwScoreCategoryRegions()
{
	return s_gcwScoreCategoryRegions;
}

//-------------------------------------------------------------------

std::map<std::string, std::map<std::string, int> > const & GameNetwork::getGcwScoreCategoryGroups()
{
	return s_gcwScoreCategoryGroups;
}

//-------------------------------------------------------------------

void GameNetwork::beginDeferringConnectionServerMessages()
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	if (s_instance->m_connectionServer)
	{
		s_instance->m_connectionServer->beginDeferringIncomingPackets();
	}
}

//-------------------------------------------------------------------

void GameNetwork::endDeferringConnectionServerMessages(bool emitDeferredPackets)
{
	DEBUG_FATAL(s_instance == 0, ("GameNetwork not installed"));
	if (s_instance->m_connectionServer)
	{
		s_instance->m_connectionServer->endDeferringIncomingPackets(emitDeferredPackets);
	}
}

//-------------------------------------------------------------------
