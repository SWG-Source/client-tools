//======================================================================
//
// CuiLoginManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"

#include "clientUserInterface/CuiLoginManager.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiCachedAvatarManager.h"
#include "clientUserInterface/CuiCharacterLoadoutManager.h"
#include "clientUserInterface/CuiLoginManagerAvatarInfo.h"
#include "clientUserInterface/CuiLoginManagerClusterInfo.h"
#include "clientUserInterface/CuiLoginManagerClusterPing.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Production.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/UdpSock.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedNetworkMessages/ClientPermissionsMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/LoginClusterStatus.h"
#include "sharedNetworkMessages/LoginClusterStatusEx.h"
#include "sharedNetworkMessages/LoginEnumCluster.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedRandom/Random.h"

#include <ctime>
#include <map>
#include <algorithm>

//======================================================================

namespace CuiLoginManagerNamespace
{
	typedef LoginEnumCluster::ClusterData   LoginEnumClusterData;
	typedef LoginClusterStatus::ClusterData LoginClusterStatusData;
	typedef EnumerateCharacterId::Chardata  EnumerateCharacterIdData;

	typedef CuiLoginManagerClusterInfo         ClusterInfo;
	typedef CuiLoginManager::ClusterInfoMap    ClusterInfoMap;
	typedef CuiLoginManager::AvatarInfoMap     AvatarInfoMap;

	typedef stdmap<Address, uint32>::fwd       AddressClusterMap;

	ClusterInfoMap    s_clusterInfo;
	AvatarInfoMap     s_avatarInfo;
	AddressClusterMap s_addressClusterMap;
	int               s_stationIdNumberJediSlot = 0;
	int               s_stationIdNumberJediSlotCharacter = 0;

#if PRODUCTION == 0
	// override the ConnectionServer host passed from the LoginServer with this host
	std::map<uint32, std::string> s_clusterOverrideHostInfo;

	// override the ConnectionServer port passed from the LoginServer with this port
	std::map<uint32, uint16> s_clusterOverridePortInfo;
#endif

	bool      s_installed = false;

	bool      s_canCreateRegularCharacter = false;
	bool      s_canCreateJedi = false;
	bool      s_canSkipTutorial = false;

	UdpSock * pingSocket = 0;

	uint32    s_pingPeriod = 0;
	uint32    s_lastPing   = 0;
	uint32    s_connectedCluster = 0;
	uint32    s_lastConnectedCluster = 0;

	std::string s_overrideSessionIdKey;

	//----------------------------------------------------------------------

	namespace Transceivers
	{		
		MessageDispatch::Transceiver<bool,    CuiLoginManager::Messages::ClusterConnection>       clusterConnection;
		MessageDispatch::Transceiver<bool,    CuiLoginManager::Messages::ClusterListChanged >     clusterListChanged;
		MessageDispatch::Transceiver<bool,    CuiLoginManager::Messages::ClusterStatusChanged >   clusterStatusChanged;
		MessageDispatch::Transceiver<bool,    CuiLoginManager::Messages::AvatarListChanged >      avatarListChanged;
	}

	//----------------------------------------------------------------------

	namespace MessageNames
	{
		const char * const LoginEnumCluster                 = "LoginEnumCluster";
		const char * const CharacterCreationDisabled        = "CharacterCreationDisabled";
		const char * const LoginClusterStatus               = "LoginClusterStatus";
		const char * const LoginClusterStatusEx             = "LoginClusterStatusEx";
		const char * const StationIdHasJediSlot             = "StationIdHasJediSlot";
		const char * const EnumerateCharacterId             = "EnumerateCharacterId";
		const char * const ClientPermissionsMessage         = "ClientPermissionsMessage";
		const char * const AccountFeatureBits               = "AccountFeatureBits";
		const char * const ConnectionServerConnectionOpened = "ConnectionServerConnectionOpened";
		const char * const ConnectionServerConnectionClosed = "ConnectionServerConnectionClosed";
	}

	//----------------------------------------------------------------------

	class Listener : public MessageDispatch::Receiver
	{
	public:

		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (MessageNames::LoginEnumCluster);
			connectToMessage (MessageNames::CharacterCreationDisabled);
			connectToMessage (MessageNames::LoginClusterStatus);
			connectToMessage (MessageNames::LoginClusterStatusEx);	
			connectToMessage (MessageNames::StationIdHasJediSlot);
			connectToMessage (MessageNames::EnumerateCharacterId);
			connectToMessage (MessageNames::ClientPermissionsMessage);
			connectToMessage (MessageNames::AccountFeatureBits);
			connectToMessage (MessageNames::ConnectionServerConnectionClosed);
		}

		void receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			//----------------------------------------------------------------------
			static LoginEnumCluster const * lec = NULL;

			if (message.isType (MessageNames::LoginEnumCluster))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();

				if (lec)
					delete lec;

				// save off message, to be processed when we receive MessageNames::CharacterCreationDisabled or MessageNames::LoginClusterStatus, whichever comes first
				lec = new LoginEnumCluster (ri);
			}
			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::CharacterCreationDisabled))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();
				const GenericValueTypeMessage<std::set<std::string> > ccd (ri);

				if (lec)
				{
					CuiLoginManager::receiveLoginEnumCluster (*lec, ccd.getValue());

					delete lec;
					lec = NULL;
				}
			}
			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::LoginClusterStatus))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();
				const LoginClusterStatus lcs (ri);

				if (lec)
				{
					CuiLoginManager::receiveLoginEnumCluster (*lec, std::set<std::string>());

					delete lec;
					lec = NULL;
				}

				CuiLoginManager::receiveLoginClusterStatus (lcs);
			}

			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::StationIdHasJediSlot))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();
				const GenericValueTypeMessage<int> sihjs (ri);
				s_stationIdNumberJediSlot = sihjs.getValue();
			}

			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::EnumerateCharacterId))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();
				const EnumerateCharacterId eci (ri);

#if PRODUCTION == 0
				{
					StationId const sid = ConfigClientGame::get2fa8673b89f9443bb24e40c3d6127118(); // getAdditionalCharacterSelectionStationId()
					std::string const & chIdStr = ConfigClientGame::gete7e3221d29dd405dbfc3bd274b010854(); // getAdditionalCharacterSelectionNetworkId()
					std::string const & clName = ConfigClientGame::geteb20e2896d8742b681b121d3fb390e2e(); // getAdditionalCharacterSelectionClusterName()
					std::string const & chName = ConfigClientGame::getafa5930b6c2b42e6b6ec38954855abc2(); // getAdditionalCharacterSelectionCharacterName()

					bool added = false;
					if ((sid > 0) && (sid == GameNetwork::getStationId()) && !chIdStr.empty() && !clName.empty() && !chName.empty())
					{
						NetworkId const chId(chIdStr);
						if (chId.isValid())
						{
							uint32 const clId = CuiLoginManager::findClusterId(clName);
							if (clId > 0)
							{
								std::vector<EnumerateCharacterId::Chardata> data = eci.getData();
								data.push_back(EnumerateCharacterId::Chardata(Unicode::narrowToWide(chName), 101601749, chId, clId, EnumerateCharacterId::Chardata::CT_normal));

								added = true;
								const EnumerateCharacterId modifiedEci(data);
								CuiLoginManager::receiveEnumerateCharacterId(modifiedEci);
							}
						}
					}

					if (!added)
						CuiLoginManager::receiveEnumerateCharacterId(eci);
				}
#else
				CuiLoginManager::receiveEnumerateCharacterId (eci);
#endif
			}

			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ClientPermissionsMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();
				const ClientPermissionsMessage cpm (ri);
				CuiLoginManager::receiveClientPermissionsMessage (cpm);
			}

			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::AccountFeatureBits))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();
				const GenericValueTypeMessage<std::pair<std::pair<unsigned long, unsigned long>, std::pair<int, int32> > > msgFeatureBits(ri);
				Game::setServerSideFeatureBits(static_cast<int>(msgFeatureBits.getValue().first.first), static_cast<int>(msgFeatureBits.getValue().first.second));
				Game::setConnectionServerId(msgFeatureBits.getValue().second.first);
				GameNetwork::setServerEpochTimeDifference(static_cast<time_t>(msgFeatureBits.getValue().second.second));
			}

			//----------------------------------------------------------------------

			else if ( message.isType( MessageNames::LoginClusterStatusEx ) )
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();
				const LoginClusterStatusEx lcs( ri );
				CuiLoginManager::receiveLoginClusterStatusEx( lcs );
			}

		}
	};

	Listener * s_listener = 0;

	std::string convertServerToShared (int serverTemplateId)
	{
		std::string server;
		ConstCharCrcString cccs = ObjectTemplateList::lookUp(serverTemplateId);
		if (!cccs.isEmpty())
			server = cccs.getString();
		size_t sharedpos = server.rfind ('/');

		if (sharedpos != std::string::npos)
		{
			return server.substr (0, sharedpos + 1) + "shared_" + server.substr (sharedpos + 1);
		}

		WARNING (true, ("bad template name"));
		return std::string ();
	}


	class MyCallback : 
	public MessageDispatch::Callback
	{
	public:

		MyCallback () : MessageDispatch::Callback () {}	

		//----------------------------------------------------------------------

		void onConnectionServerConnectionChanged (const GameNetwork::Messages::ConnectionServerConnectionChanged::Payload &)
		{
			if (!GameNetwork::getConnectionServerConnection ())
				s_connectedCluster = 0;
		} //lint !e1762
	};
	
	MyCallback s_callback;

}

using namespace CuiLoginManagerNamespace;

//----------------------------------------------------------------------

void CuiLoginManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));
	DEBUG_FATAL (!s_clusterInfo.empty (), ("not empty"));
	DEBUG_FATAL (!s_addressClusterMap.empty (), ("not empty"));
	DEBUG_FATAL (!s_avatarInfo.empty (),  ("not empty"));
	s_listener = new Listener;
	pingSocket = new UdpSock;
	pingSocket->bind ();
	s_connectedCluster = 0;

	s_installed = true;

	s_pingPeriod = static_cast<uint32>(ConfigClientUserInterface::getConnectionServerPingPeriodMs ());

	s_callback.connect (s_callback, &MyCallback::onConnectionServerConnectionChanged,           static_cast<GameNetwork::Messages::ConnectionServerConnectionChanged *>     (0));

}

//----------------------------------------------------------------------

void CuiLoginManager::remove  ()
{
	s_callback.disconnect (s_callback, &MyCallback::onConnectionServerConnectionChanged,           static_cast<GameNetwork::Messages::ConnectionServerConnectionChanged *>     (0));

	DEBUG_FATAL (!s_installed, ("not installed"));
	delete s_listener;
	s_listener = 0;

	delete pingSocket;
	pingSocket = 0;

	s_clusterInfo.clear ();
	s_addressClusterMap.clear ();
	
	purgeCreatures();
	s_avatarInfo.clear();
	s_installed = false;
}

//----------------------------------------------------------------------

void CuiLoginManager::receiveLoginEnumCluster (const LoginEnumCluster & lec, const std::set<std::string> & characterCreationDisabled)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	s_clusterInfo.clear ();
	s_addressClusterMap.clear ();

	typedef stdvector<LoginEnumClusterData>::fwd ClusterDataVector;

	const ClusterDataVector & cdv = lec.getData ();

	for (ClusterDataVector::const_iterator it = cdv.begin (); it != cdv.end (); ++it)
	{
		const LoginEnumClusterData & data = *it;
		s_clusterInfo [data.m_clusterId] = ClusterInfo (data.m_clusterId, data.m_clusterName, (characterCreationDisabled.count(data.m_clusterName) >= 1));
	}

	Transceivers::clusterListChanged.emitMessage (true);
}

//----------------------------------------------------------------------

void CuiLoginManager::receiveLoginClusterStatusEx( const LoginClusterStatusEx &msg )
{
	typedef std::vector< LoginClusterStatusEx::ClusterData > ClusterDataVector;

	const ClusterDataVector & vec = msg.getData();

	for ( ClusterDataVector::const_iterator it = vec.begin(); it != vec.end(); ++it )
	{
		const LoginClusterStatusEx::ClusterData &data = *it;

		const ClusterInfoMap::iterator cit = s_clusterInfo.find( data.m_clusterId );

		if ( cit != s_clusterInfo.end() )
		{
			ClusterInfo &info   = cit->second;
			info.branch         = data.m_branch;
			info.version        = data.m_version;
			info.netVersionMatch = data.m_networkVersion == GameNetworkMessage::NetworkVersionId;
		}
	} 

	Transceivers::clusterStatusChanged.emitMessage (true);
}

//----------------------------------------------------------------------

void CuiLoginManager::receiveLoginClusterStatus (const LoginClusterStatus & lcs)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	typedef stdvector<LoginClusterStatusData>::fwd StatusVector;
	const StatusVector &  sv = lcs.getData ();
	
	if (sv.empty ())
	{
//		DEBUG_WARNING (true, ("CuiLoginManager received an empty LoginClusterStatus vector"));
		return;
	}

	{
		for (ClusterInfoMap::iterator it = s_clusterInfo.begin (); it != s_clusterInfo.end (); ++it)
		{
			ClusterInfo & info = it->second;
			info.up = false;
		}
	}
	
	//-- clear out the cached ping addresses
	s_addressClusterMap.clear ();

	{
		for (StatusVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
		{
			const LoginClusterStatusData & data = *it;
			
			if (data.m_status != LoginClusterStatusData::S_down)
			{
				const ClusterInfoMap::iterator cit = s_clusterInfo.find (data.m_clusterId);
				if (cit != s_clusterInfo.end ())
				{
					ClusterInfo & info  = cit->second;
					info.up             = data.m_status != LoginClusterStatusData::S_down;
					info.loading        = data.m_status == LoginClusterStatusData::S_loading;
					info.locked         = data.m_status == LoginClusterStatusData::S_locked;
					info.restricted     = data.m_status == LoginClusterStatusData::S_restricted;
					info.isFull         = data.m_status == LoginClusterStatusData::S_full;
					info.onlinePlayerLimit = data.m_onlinePlayerLimit;
					info.onlineFreeTrialLimit = data.m_onlineFreeTrialLimit;
					info.notRecommended = data.m_dontRecommend;						

#if PRODUCTION == 0
					std::map<uint32, std::string>::const_iterator iterFind = s_clusterOverrideHostInfo.find(data.m_clusterId);
					if (iterFind != s_clusterOverrideHostInfo.end())
					{
						info.setHost(iterFind->second);
					}
					else
					{
						info.setHost(data.m_connectionServerAddress);
					}

					std::map<uint32, uint16>::const_iterator iterFind2 = s_clusterOverridePortInfo.find(data.m_clusterId);
					if (iterFind2 != s_clusterOverridePortInfo.end())
					{
						info.setPort(iterFind2->second);
					}
					else
					{
						info.setPort(data.m_connectionServerPort);
					}
#else
					info.setHost    (data.m_connectionServerAddress);
					info.setPort    (data.m_connectionServerPort);
#endif

					info.setPingPort(data.m_connectionServerPingPort);
					info.population    = data.m_populationOnline;
					info.populationStatus = data.m_populationOnlineStatus;
					info.maxAvatars    = 0;//this data is invalid
					info.timeZone      = data.m_timeZone;
					const Address tmpAddr (info.getHost(), info.getPingPort());
					const Address addr (tmpAddr.getSockAddr4 ());
					s_addressClusterMap [addr] = data.m_clusterId;
				}
				else
					WARNING (true, ("Received cluster status for unknown cluster %d, [%s:%d]", data.m_clusterId, data.m_connectionServerAddress, data.m_connectionServerPort));
			}
		}
	}

	for (ClusterInfoMap::iterator cit = s_clusterInfo.begin (); cit != s_clusterInfo.end (); ++cit)
	{
		const uint32 id = (*cit).first;

		bool found = false;

		for (StatusVector::const_iterator sit = sv.begin (); sit != sv.end (); ++sit)
		{
			const LoginClusterStatusData & data = *sit;
			if (data.m_clusterId == id)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			ClusterInfo & info  = (*cit).second;	
			info.up             = false;
			info.loading        = false;
			info.locked         = false;
			info.restricted     = false;
			info.isFull         = false;
			info.onlinePlayerLimit = 0;
			info.onlineFreeTrialLimit = 0;
			info.notRecommended = false;
			info.setHost     ("");
			info.setPort     (0);
			info.setPingPort (0);
			info.population     = -1;
			info.populationStatus = LoginClusterStatus::ClusterData::PS_very_light;
		}
	}

	Transceivers::clusterStatusChanged.emitMessage (true);
}

//----------------------------------------------------------------------

void CuiLoginManager::receiveEnumerateCharacterId (const EnumerateCharacterId & eci)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	purgeCreatures ();
	s_avatarInfo.clear ();
	s_stationIdNumberJediSlotCharacter = 0;

	typedef stdvector<EnumerateCharacterIdData>::fwd CharacterVector;
	const CharacterVector & cv = eci.getData ();

	for (CharacterVector::const_iterator it = cv.begin (); it != cv.end (); ++it)
	{
		const EnumerateCharacterIdData & data = *it;

		AvatarInfo info;
		info.name           = data.m_name;
		info.objectTemplate = convertServerToShared (data.m_objectTemplateId);
		info.networkId      = data.m_networkId;
		info.clusterId      = data.m_clusterId;
		info.characterType  = data.m_characterType;

		if (info.characterType == static_cast<int>(EnumerateCharacterId_Chardata::CT_jedi))
			++s_stationIdNumberJediSlotCharacter;

		CuiLoginManagerClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfoInternal (info.clusterId);

		WARNING (!clusterInfo, ("CuiLoginManager Unable to load cluster info for cluster %d, requested by avatar [%s], %s", info.clusterId, Unicode::wideToNarrow (info.name).c_str (), info.networkId.getValueString ().c_str ()));

		if (clusterInfo)
		{
			CuiCachedAvatarManager::CachedAvatarInfo cachedInfo;
			CuiCachedAvatarManager::getCachedAvatarInfo(info.networkId, GameNetwork::getUserName (), clusterInfo->name, cachedInfo);
			info.planetName = cachedInfo.m_planetName;
			info.characterLevel = cachedInfo.m_level;
			info.characterSkillTemplate = cachedInfo.m_skillTemplate;

			++clusterInfo->numAvatars;
		}

		CreatureObject * co = NULL;
		s_avatarInfo [std::make_pair(info.clusterId, info.networkId)] = std::make_pair(info, co);

		CuiCachedAvatarManager::addToCharacterList (info.clusterId, info.networkId, info.name, info.planetName);
	}

	Transceivers::avatarListChanged.emitMessage (true);

	// just do this check once per account each time the client is first started
	static std::set<StationId> alreadyCheckedForUnoccupiedJediSlot;
	StationId const stationId = GameNetwork::getStationId();
	if (alreadyCheckedForUnoccupiedJediSlot.count(stationId) <= 0)
	{
		IGNORE_RETURN(alreadyCheckedForUnoccupiedJediSlot.insert(stationId));

		if (hasUnoccupiedJediSlot())
			CuiMessageBox::createInfoBox (StringId("unlocked_slot", "has_unoccupied_unlock_slot_notification").localize());
	}
}

//----------------------------------------------------------------------

void CuiLoginManager::receiveClientPermissionsMessage (const ClientPermissionsMessage & cpm)
{
	s_canCreateRegularCharacter = cpm.getCanCreateRegularCharacter();
	s_canCreateJedi = cpm.getCanCreateJediCharacter();
	s_canSkipTutorial = cpm.getCanSkipTutorial();
	
	Transceivers::clusterConnection.emitMessage (cpm.getCanLogin ());
	if (!cpm.getCanLogin ())
		s_connectedCluster = 0;
}

//----------------------------------------------------------------------

ClusterInfo * CuiLoginManager::findClusterInfoInternal         (uint32 id)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const ClusterInfoMap::iterator it = s_clusterInfo.find (id);
	if (it != s_clusterInfo.end ())
		return &it->second;

	return 0;
}

//----------------------------------------------------------------------

uint32 CuiLoginManager::findClusterId (const std::string & name)
{
	if (name.empty ())
		return 0;

	for (ClusterInfoMap::const_iterator it = s_clusterInfo.begin (); it != s_clusterInfo.end (); ++it)
	{
		const ClusterInfo & ci = (*it).second;
		if (!_stricmp (ci.name.c_str (), name.c_str ()))
			return ci.id;
	}

	return 0;
}

//----------------------------------------------------------------------

const CuiLoginManagerClusterInfo * CuiLoginManager::findClusterInfo (uint32 id)
{
	return findClusterInfoInternal (id);
}

//----------------------------------------------------------------------

const CuiLoginManagerAvatarInfo *    CuiLoginManager::findAvatarInfo   (uint32 clusterId, const Unicode::String & name)
{
	for (AvatarInfoMap::const_iterator it = s_avatarInfo.begin (); it != s_avatarInfo.end (); ++it)
	{
		const AvatarInfo & info = (*it).second.first;

		if (info.clusterId == clusterId && Unicode::caseInsensitiveCompare (info.name, name))
			return &info;
	}

	return 0;
}

//----------------------------------------------------------------------

const CuiLoginManagerAvatarInfo *    CuiLoginManager::findAvatarInfo   (uint32 clusterId, const NetworkId & id)
{
	for (AvatarInfoMap::const_iterator it = s_avatarInfo.begin (); it != s_avatarInfo.end (); ++it)
	{
		const AvatarInfo & info = (*it).second.first;

		if ((info.clusterId == clusterId) && (info.networkId == id))
			return &info;
	}

	return 0;
}

//----------------------------------------------------------------------

void CuiLoginManager::setOverrideHostForCluster(uint32 id, const std::string & hostName)
{
#if PRODUCTION == 0
	const ClusterInfoMap::const_iterator it = s_clusterInfo.find(id);
	if (it != s_clusterInfo.end())
	{
		s_clusterOverrideHostInfo[id] = hostName;
	}
#else
	UNREF(id);
	UNREF(hostName);
#endif
}

//----------------------------------------------------------------------

void CuiLoginManager::setOverridePortForCluster(uint32 id, uint16 port)
{
#if PRODUCTION == 0
	const ClusterInfoMap::const_iterator it = s_clusterInfo.find(id);
	if (it != s_clusterInfo.end())
	{
		s_clusterOverridePortInfo[id] = port;
	}
#else
	UNREF(id);
	UNREF(port);
#endif
}

//----------------------------------------------------------------------

void CuiLoginManager::addAvatarToList (const CreatureObject & avatar, const NetworkId & networkId)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	AvatarInfo info (s_connectedCluster, networkId);
	info.name           = avatar.getLocalizedName ();
	info.objectTemplate = avatar.getObjectTemplateName ();
	
	CreatureObject * co = NULL;
	s_avatarInfo [std::make_pair(info.clusterId, info.networkId)] = std::make_pair(info, co);
	Transceivers::avatarListChanged.emitMessage (true);
	
	CuiLoginManagerClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfoInternal (info.clusterId);
	if (clusterInfo)
		++clusterInfo->numAvatars;
	
	CuiCachedAvatarManager::addToCharacterList (info.clusterId, info.networkId, info.name, std::string ());
}

//----------------------------------------------------------------------

void CuiLoginManager::removeAvatarFromList            (const AvatarInfo & avatar)
{
	CuiCachedAvatarManager::removeAvatar (avatar.networkId, avatar.clusterId);

	const AvatarInfoMap::iterator ait = s_avatarInfo.find (std::make_pair(avatar.clusterId, avatar.networkId));
	
	if (ait != s_avatarInfo.end ())
	{		
		CuiLoginManagerClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfoInternal (avatar.clusterId);

		if (clusterInfo)
			--clusterInfo->numAvatars;
	
		if (ait->second.second)
		{
			AlterScheduler::removeObject(*ait->second.second);
			delete ait->second.second;
		}
		s_avatarInfo.erase (ait);
		
		Transceivers::avatarListChanged.emitMessage (true);
	}
	else
	{
		DEBUG_FATAL (true, ("CuiLoginManager::removeAvatarFromList   failed"));
	}
}

//----------------------------------------------------------------------

CreatureObject * CuiLoginManager::createCreature (AvatarInfo & avatarInfo)
{	
	DEBUG_FATAL (!s_installed, ("not installed"));

	ClientObject * const object = safe_cast<ClientObject *>(ObjectTemplate::createObject (avatarInfo.objectTemplate.c_str ()));
	
	if (!object)
		WARNING (true, ("Unable to create desired creature template: %s", avatarInfo.objectTemplate.c_str ()));
	else
	{
		CreatureObject * const creature = dynamic_cast<CreatureObject *>(object);
		
		if (!creature)
		{
			WARNING (true, ("Specified template is not a creature: %s", avatarInfo.objectTemplate.c_str ()));
			if (object)
				AlterScheduler::removeObject(*object);
			delete object;
		}
		else
		{
			creature->createDefaultController ();
			object->endBaselines ();
		
			creature->setObjectName (avatarInfo.name);
			
			Appearance * const app = creature->getAppearance ();
			if (app)
			{
				SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
				if (skelApp)
				{
					//-- preload all lod assets
					const int count = skelApp->getDetailLevelCount ();
					for (int i = 0; i < count; ++i)
						skelApp->isDetailLevelAvailable (i);
				}
			}

			const ClusterInfo * const clusterInfo = findClusterInfo (avatarInfo.clusterId);
			if (clusterInfo)
			{
				CuiCachedAvatarManager::CachedAvatarInfo cachedInfo;
				if (!CuiCachedAvatarManager::loadAvatar (*creature, cachedInfo, avatarInfo.networkId, GameNetwork::getUserName (), clusterInfo->name))
				{
					IGNORE_RETURN (CuiCharacterLoadoutManager::setupLoadout (*creature));
					IGNORE_RETURN (CuiCachedAvatarManager::saveAvatar       (*creature, avatarInfo.networkId, GameNetwork::getUserName (), clusterInfo->name));
				}
				else
				{
					avatarInfo.planetName = cachedInfo.m_planetName;
					avatarInfo.characterLevel = cachedInfo.m_level;
					avatarInfo.characterSkillTemplate = cachedInfo.m_skillTemplate;
				}

			}
			else
				WARNING (true, ("Attempt to create avatar with no valid cluster %d", avatarInfo.clusterId));

			return creature;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

CreatureObject * CuiLoginManager::getAvatarCreature (uint32 clusterId, const NetworkId & id)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const AvatarInfoMap::iterator ait = s_avatarInfo.find (std::make_pair(clusterId, id));
	if (ait != s_avatarInfo.end ())
	{
		if (ait->second.second)
		{
			return ait->second.second;
		}
		else
		{
			CreatureObject* newCreature = createCreature (ait->second.first);
			ait->second.second = newCreature;
			return newCreature;
		}
	}
	else
		WARNING (true, ("Request for invalid avatar id %s", id.getValueString ()));

	return 0;
}

//----------------------------------------------------------------------

void CuiLoginManager::purgeCreatures              ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	for (AvatarInfoMap::iterator it = s_avatarInfo.begin (); it != s_avatarInfo.end (); ++it)
	{
		if (it->second.second)
			AlterScheduler::removeObject(*it->second.second);

		delete it->second.second;
		it->second.second = 0;
	}
}

//----------------------------------------------------------------------

void CuiLoginManager::simulateClusters ()
{
	if (Game::getSinglePlayer () && s_clusterInfo.empty ())
	{
		for (int i = 0; i < 15; ++i)
		{
			ClusterInfo info;
			info.id = i;

			char buf [128];
			const size_t buf_size = sizeof (buf);

			snprintf (buf, buf_size, "simulated %d", i);
			info.name           = buf;
			info.up             = (i % 3) != 0;
			info.loading        = info.up && (i % 2) != 0;
			info.locked         = info.up && !info.loading && !Random::random (0, 2) == 0;
			info.restricted     = info.up && !info.loading && !info.locked && !Random::random (0, 2) == 0;
			info.isFull         = info.up && !info.loading && !info.locked && !info.restricted && !Random::random (0, 2) == 0;
			info.onlinePlayerLimit = 3000;
			info.onlineFreeTrialLimit = 250;
			info.notRecommended = info.up && Random::random (0, 1) == 0;
			info.population     = -1;
			info.populationStatus = static_cast<LoginClusterStatus::ClusterData::PopulationStatus>(Random::random(static_cast<int32>(LoginClusterStatus::ClusterData::PS_LOWEST), static_cast<int32>(LoginClusterStatus::ClusterData::PS_HIGHEST)));
			info.maxAvatars     = 0;
			info.numAvatars     = 0;
			info.setPort       (static_cast<uint16>(Random::random (1000, 50000)));

			snprintf (buf, buf_size, "%d.%d.%d.%d", i, i * 2, i * 3, i);
			info.setHost       (buf);
			info.setPingPort   (static_cast<uint16>(info.getPort () + i * 3));

			s_clusterInfo.insert (std::make_pair (info.id, info));
		}
	}
}

//----------------------------------------------------------------------

void CuiLoginManager::getClusterInfo              (ClusterInfoVector & civ)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	for (ClusterInfoMap::iterator it = s_clusterInfo.begin (); it != s_clusterInfo.end (); ++it)
	{
		const ClusterInfo & info = it->second;
		civ.push_back (info);
	}
}

//----------------------------------------------------------------------

void CuiLoginManager::getAvatarInfo               (AvatarInfoVector  & aiv)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	for (AvatarInfoMap::iterator it = s_avatarInfo.begin (); it != s_avatarInfo.end (); ++it)
	{
		aiv.push_back (it->second.first);
	}
}

//----------------------------------------------------------------------

int CuiLoginManager::getAvatarCount                  ()
{
	return static_cast<int>(s_avatarInfo.size ());
}

//----------------------------------------------------------------------

bool CuiLoginManager::canCreateCharacter()
{
	return canCreateRegularCharacter () || s_canCreateJedi;
}

//----------------------------------------------------------------------

bool CuiLoginManager::canCreateRegularCharacter()
{
	return s_canCreateRegularCharacter || Game::getSinglePlayer ();
}

//----------------------------------------------------------------------

bool CuiLoginManager::canCreateJedi()
{
	return s_canCreateJedi;
}

//----------------------------------------------------------------------

bool CuiLoginManager::canSkipTutorial()
{
	return s_canSkipTutorial;
}

//----------------------------------------------------------------------

bool CuiLoginManager::hasUnoccupiedJediSlot()
{
	return (s_stationIdNumberJediSlot > s_stationIdNumberJediSlotCharacter);
}

//----------------------------------------------------------------------

void CuiLoginManager::characterChangedUnlocked(const std::string & clusterName, const NetworkId & id)
{
	uint32 const clusterId = findClusterId(clusterName);
	if (clusterId)
	{
		AvatarInfoMap::iterator it = s_avatarInfo.find(std::make_pair(clusterId, id));
		if (it != s_avatarInfo.end())
		{
			it->second.first.characterType = static_cast<int>(EnumerateCharacterId_Chardata::CT_jedi);

			s_stationIdNumberJediSlotCharacter = 0;
			for (AvatarInfoMap::const_iterator it2 = s_avatarInfo.begin(); it2 != s_avatarInfo.end(); ++it2)
			{
				if (it2->second.first.characterType == static_cast<int>(EnumerateCharacterId_Chardata::CT_jedi))
					++s_stationIdNumberJediSlotCharacter;
			}

			GameNetwork::updateServerWithJediSlotInfo();
		}
	}
}

//----------------------------------------------------------------------

void CuiLoginManager::characterChangedNormal(const std::string & clusterName, const NetworkId & id)
{
	uint32 const clusterId = findClusterId(clusterName);
	if (clusterId)
	{
		AvatarInfoMap::iterator it = s_avatarInfo.find(std::make_pair(clusterId, id));
		if (it != s_avatarInfo.end())
		{
			it->second.first.characterType = static_cast<int>(EnumerateCharacterId_Chardata::CT_normal);

			s_stationIdNumberJediSlotCharacter = 0;
			for (AvatarInfoMap::const_iterator it2 = s_avatarInfo.begin(); it2 != s_avatarInfo.end(); ++it2)
			{
				if (it2->second.first.characterType == static_cast<int>(EnumerateCharacterId_Chardata::CT_jedi))
					++s_stationIdNumberJediSlotCharacter;
			}

			GameNetwork::updateServerWithJediSlotInfo();
		}
	}
}

//----------------------------------------------------------------------

void CuiLoginManager::characterNameChanged(const std::string & clusterName, const NetworkId & id, const Unicode::String & newName)
{
	uint32 const clusterId = findClusterId(clusterName);
	if (clusterId)
	{
		AvatarInfoMap::iterator it = s_avatarInfo.find(std::make_pair(clusterId, id));
		if (it != s_avatarInfo.end())
		{
			it->second.first.name = newName;
		}
	}
}

//----------------------------------------------------------------------

void CuiLoginManager::connectToCluster            (const ClusterInfo & cinfo)
{
	GameNetwork::connectConnectionServer (cinfo.getHost(), cinfo.getPort(), cinfo.name);
	s_connectedCluster     = cinfo.id;
	s_lastConnectedCluster = cinfo.id;
}

//----------------------------------------------------------------------

void CuiLoginManager::disconnectFromCluster ()
{
	GameNetwork::disconnectConnectionServer ();
	s_connectedCluster = 0;
}

//----------------------------------------------------------------------

void CuiLoginManager::pingServers ()
{
	for (ClusterInfoMap::iterator it = s_clusterInfo.begin (); it != s_clusterInfo.end (); ++it)
	{
		ClusterInfo & cinfo = (*it).second;
		
		if (cinfo.up && cinfo.getPingPort())
		{
			pingServer (cinfo);
		}
	}
}

//----------------------------------------------------------------------

void CuiLoginManager::pingServer (ClusterInfo & cinfo)
{
	if (!cinfo.getPing ().isEnabled ())
		return;
	const uint32 currentTime = cinfo.getPing ().onSendingPing ();
	pingSocket->sendTo (*cinfo.getPingAddress(), &currentTime, 4);
}

//----------------------------------------------------------------------

void CuiLoginManager::update (float elapsedTime)
{
	static uint32 timestamp  = 0;

	while (pingSocket->canRecv ())
	{
		Address addr;
		const int count = pingSocket->recvFrom  (addr, &timestamp, 4);
		if(count > 0)
		{
			if (count != 4)
				WARNING (true, ("Received broken ping response of size [%d] from [%s:%d].", count, addr.getHostAddress ().c_str (), addr.getHostPort ()));

			const AddressClusterMap::const_iterator ait = s_addressClusterMap.find (addr);

			if (ait != s_addressClusterMap.end ())
			{
				const uint32 id = (*ait).second;

				const ClusterInfoMap::iterator cit = s_clusterInfo.find (id);
				if (cit != s_clusterInfo.end ())
				{
					ClusterInfo & cinfo = (*cit).second;
					cinfo.up = true;
					cinfo.getPing ().onReceivedPing (timestamp);
				}
			}
		}
		else if (count == -1)
		{
			const AddressClusterMap::const_iterator ait = s_addressClusterMap.find (addr);

			if (ait != s_addressClusterMap.end ())
			{
				const uint32 id = (*ait).second;

				const ClusterInfoMap::iterator cit = s_clusterInfo.find (id);
				if (cit != s_clusterInfo.end ())
				{
					ClusterInfo & cinfo = (*cit).second;
					if (cinfo.up)
					{
						DEBUG_WARNING(count == -1, ("The connection server is down. Stop sending ping packets\n"));
						cinfo.up = false;
					}
				}
			}
		}
	}

	UNREF (elapsedTime);

	const uint32 currentTime = Clock::timeMs ();

	if ((s_lastPing + s_pingPeriod) < currentTime)
	{
		pingServers ();
		s_lastPing = currentTime;
	}
}

//----------------------------------------------------------------------

void CuiLoginManager::setAllPingsEnabled          (bool b)
{
	for (ClusterInfoMap::iterator it = s_clusterInfo.begin (); it != s_clusterInfo.end (); ++it)
	{
		ClusterInfo & cinfo = (*it).second;
		cinfo.getPing ().setEnabled (b);
	}
}

//----------------------------------------------------------------------

void CuiLoginManager::setPingEnabled              (uint32 clusterId, bool b)
{
	const ClusterInfoMap::iterator cit = s_clusterInfo.find (clusterId);
	if (cit != s_clusterInfo.end ())
	{
		ClusterInfo & cinfo = (*cit).second;
		cinfo.getPing ().setEnabled (b);
	}
}

//----------------------------------------------------------------------

uint32 CuiLoginManager::getConnectedClusterId         ()
{
	return s_connectedCluster;
}

//----------------------------------------------------------------------

const std::string & CuiLoginManager::getConnectedClusterName         ()
{
	if (s_connectedCluster)
	{
		const ClusterInfo * const ci = findClusterInfo (s_connectedCluster);
		if (ci)
			return ci->name;
	}

	static const std::string empty;

	return empty;
}

//----------------------------------------------------------------------

uint32 CuiLoginManager::getLastConnectedClusterId         ()
{
	return s_lastConnectedCluster;
}

//----------------------------------------------------------------------

const char * CuiLoginManager::getSessionIdKey (bool useOverrideSessionIdKeyIfAvailable /*= false*/)
{
	static const char * const SessionIdSection = "Station";
	static const char * const SessionIdKey     = "sessionId";

	const char * sessionIdKey = ConfigFile::getKeyString (SessionIdSection, SessionIdKey, NULL);
	if (!sessionIdKey && useOverrideSessionIdKeyIfAvailable && !s_overrideSessionIdKey.empty())
		sessionIdKey = s_overrideSessionIdKey.c_str();

	return sessionIdKey;
}

//----------------------------------------------------------------------

void CuiLoginManager::setOverrideSessionIdKey(const std::string & sessionIdKey)
{
	if (sessionIdKey.empty())
		s_overrideSessionIdKey.clear();
	else
		s_overrideSessionIdKey = sessionIdKey;
}

//----------------------------------------------------------------------

void CuiLoginManager::setAllPingsDisabled             ()
{
	CuiLoginManager::setAllPingsEnabled (false);
}

//======================================================================
