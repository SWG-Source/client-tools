//======================================================================
//
// CuiLoginManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiLoginManager_H
#define INCLUDED_CuiLoginManager_H

//======================================================================

class CuiLoginManagerClusterInfo;
class CuiLoginManagerAvatarInfo;
class LoginEnumCluster;
class LoginClusterStatus;
class LoginClusterStatusEx;
class EnumerateCharacterId;
class CreatureObject;
class ClientPermissionsMessage;
class CuiLoginManagerClusterPing;
class NetworkId;

//----------------------------------------------------------------------

class CuiLoginManager
{
public:

	struct Messages
	{
		struct ClusterConnection;
		struct ClusterListChanged;
		struct ClusterStatusChanged;
		struct AvatarListChanged;
	};

	typedef CuiLoginManagerClusterInfo                ClusterInfo;
	typedef CuiLoginManagerAvatarInfo                 AvatarInfo;
	typedef CuiLoginManagerClusterPing                ClusterPing;

	typedef stdmap<uint32, ClusterInfo>::fwd          ClusterInfoMap;
	typedef stdmap<std::pair<uint32, NetworkId>, std::pair<AvatarInfo, CreatureObject *> >::fwd AvatarInfoMap;

	typedef stdvector<ClusterInfo>::fwd               ClusterInfoVector;
	typedef stdvector<AvatarInfo>::fwd                AvatarInfoVector;


	static void                   install                         ();
	static void                   remove                          ();

	static void                   receiveLoginEnumCluster         (const LoginEnumCluster & lec, const stdset<std::string>::fwd & characterCreationDisabled);
	static void                   receiveLoginClusterStatus       (const LoginClusterStatus & lcs);
	static void                   receiveEnumerateCharacterId     (const EnumerateCharacterId & eci);
	static void                   receiveClientPermissionsMessage (const ClientPermissionsMessage & cpm);
	static void                   receiveLoginClusterStatusEx     (const LoginClusterStatusEx &msg );

	static uint32                 findClusterId                   (const std::string & name);
	static const ClusterInfo *    findClusterInfo                 (uint32 id);
	static const AvatarInfo *     findAvatarInfo                  (uint32 clusterId, const Unicode::String & name);
	static const AvatarInfo *     findAvatarInfo                  (uint32 clusterId, const NetworkId & id);
	static void                   setOverrideHostForCluster       (uint32 id, const std::string & hostName);
	static void                   setOverridePortForCluster       (uint32 id, uint16 port);

	static CreatureObject *       getAvatarCreature               (uint32 clusterId, const NetworkId & id);

	static void                   getClusterInfo                  (ClusterInfoVector & civ);
	static void                   getAvatarInfo                   (AvatarInfoVector  & aiv);
	static int                    getAvatarCount                  ();

	static bool                   canCreateCharacter              ();
	static bool                   canCreateRegularCharacter       ();
	static bool                   canCreateJedi                   ();
	static bool                   canSkipTutorial                 ();
	static bool                   hasUnoccupiedJediSlot           ();
	static void                   characterChangedUnlocked        (const std::string & clusterName, const NetworkId & id);
	static void                   characterChangedNormal          (const std::string & clusterName, const NetworkId & id);
	static void                   characterNameChanged            (const std::string & clusterName, const NetworkId & id, const Unicode::String & newName);

	static void                   connectToCluster                (const ClusterInfo & cinfo);
	static void                   disconnectFromCluster           ();

	static uint32                 getMaxAvatars                   ();

	static void                   update                          (float elapsedTime);

	static void                   setAllPingsDisabled             ();
	static void                   setPingEnabled                  (uint32 clusterId, bool b);

	static uint32                 getConnectedClusterId           ();
	static uint32                 getLastConnectedClusterId       ();
	static const std::string &    getConnectedClusterName         ();

	static const char *           getSessionIdKey                 (bool useOverrideSessionIdKeyIfAvailable = false);
	static void                   setOverrideSessionIdKey         (const std::string & sessionIdKey);

	static void                   addAvatarToList                 (const CreatureObject & avatar, const NetworkId & networkId);
	static void                   removeAvatarFromList            (const AvatarInfo & avatar);
		
	static void                   purgeCreatures                  ();

	static void                   simulateClusters                ();

private:

	static void                   setAllPingsEnabled              (bool b);
	static void                   pingServers                     ();
	static void                   pingServer                      (ClusterInfo & cinfo);

	// createCreature uses the name, template, networkId and clusterId to create the creature
	// and fills in the cached data fields if the data is found.
	static CreatureObject *       createCreature                  (AvatarInfo & avatarInfo);

	static ClusterInfo *          findClusterInfoInternal         (uint32 id);
};

//======================================================================

#endif
