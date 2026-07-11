// ======================================================================
//
// LoginClusterStatus.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LoginClusterStatus_H
#define INCLUDED_LoginClusterStatus_H

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"

// ======================================================================

struct LoginClusterStatus_ClusterData
{
	enum Status {S_down, S_loading, S_up, S_locked, S_restricted, S_full};

	// enum value must be ordered in ascending population density
	enum PopulationStatus {PS_LOWEST = 0, PS_very_light = PS_LOWEST, PS_light, PS_medium, PS_heavy, PS_very_heavy, PS_extremely_heavy, PS_full, PS_HIGHEST = PS_full};

	uint32      m_clusterId;
	std::string m_connectionServerAddress;
	uint16      m_connectionServerPort;
	uint16      m_connectionServerPingPort;
	int         m_populationOnline; // must be signed, -1 is a legitimate value meaning not available (for security reason)
	PopulationStatus m_populationOnlineStatus;
	int         m_maxCharactersPerAccount;
	int         m_timeZone;
	Status      m_status;
	bool        m_dontRecommend;
	uint32      m_onlinePlayerLimit;
	uint32      m_onlineFreeTrialLimit;

	// This flag tells the client that the account which requested the cluster information
	// is considered an admin by the server (in Admin Account Table). This enables connection
	// to locked clusters and viewing secret clusters. We don't need to worry about this flag
	// being manipulated because the Connection Server verifies admin permissions after the fact.
	// This flag merely unblocks client-side user interface elements for locked/secret clusters.
	// We also send this flag in the ClientPermissionsMessage header for character creation.
	bool        m_isAdmin;
	// Flag so we can append (Secret) in Client (for admins)
	bool        m_isSecret;
	
};

namespace Archive
{
	inline void get(ReadIterator& source, LoginClusterStatus_ClusterData& c)
	{
		get(source, c.m_clusterId);
		get(source, c.m_connectionServerAddress);
		get(source, c.m_connectionServerPort);
		get(source, c.m_connectionServerPingPort);
		get(source, c.m_populationOnline);
		int tempStatus;
		get(source, tempStatus);
		c.m_populationOnlineStatus = static_cast<LoginClusterStatus_ClusterData::PopulationStatus>(tempStatus);
		get(source, c.m_maxCharactersPerAccount);
		get(source, c.m_timeZone);
		get(source, tempStatus);
		c.m_status = static_cast<LoginClusterStatus_ClusterData::Status>(tempStatus);
		get(source, c.m_dontRecommend);
		get(source, c.m_onlinePlayerLimit);
		get(source, c.m_onlineFreeTrialLimit);
		// m_isAdmin and m_isSecret were added to LoginClusterStatus_ClusterData
		// in a newer protocol revision than the upstream server we run against.
		// The server's struct ends at m_onlineFreeTrialLimit and serializes 12
		// fields; the client expects 14. Without this guard, the unpack runs off
		// the buffer end, throws Archive::ReadException, and the dispatch handler
		// silently drops the entire LoginClusterStatus message - leaving every
		// cluster stuck at up=false with empty host/port and producing the
		// "You cannot connect to that Galaxy at this time. The connection
		// server is unavailable." popup. Default the trailing fields when the
		// server's payload doesn't include them.
		if (source.getSize() >= sizeof(bool))
			get(source, c.m_isAdmin);
		else
			c.m_isAdmin = false;
		if (source.getSize() >= sizeof(bool))
			get(source, c.m_isSecret);
		else
			c.m_isSecret = false;
	}

	inline void put(ByteStream& target, const LoginClusterStatus_ClusterData& c)
	{
		put(target, c.m_clusterId);
		put(target, c.m_connectionServerAddress);
		put(target, c.m_connectionServerPort);
		put(target, c.m_connectionServerPingPort);
		put(target, c.m_populationOnline);
		put(target, static_cast<int>(c.m_populationOnlineStatus));
		put(target, c.m_maxCharactersPerAccount);
		put(target, c.m_timeZone);
		put(target, static_cast<int>(c.m_status));
		put(target, c.m_dontRecommend);
		put(target, c.m_onlinePlayerLimit);
		put(target, c.m_onlineFreeTrialLimit);
		put(target, c.m_isAdmin);
		put(target, c.m_isSecret);
	}
}

#include "sharedNetworkMessages/GameNetworkMessage.h"

/**
 * Sent From:  LoginServer
 * Sent To:    Client
 * Action:     Lists all the clusters that are currently available for login.
*/
class LoginClusterStatus : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	typedef LoginClusterStatus_ClusterData ClusterData;
	
public:
	const std::vector<ClusterData> &getData() const;
	//	void addCluster(uint32 clusterId, const std::string &m_clusterName);
	
public:
	LoginClusterStatus(const std::vector<ClusterData> &data);
	LoginClusterStatus(Archive::ReadIterator & source);
	~LoginClusterStatus();
	
private:
	Archive::AutoArray<ClusterData> m_data;
	
private: //disable
	LoginClusterStatus();
	LoginClusterStatus(const LoginClusterStatus&);
	LoginClusterStatus& operator= (const LoginClusterStatus&);
};

// ======================================================================



// ======================================================================

inline const std::vector<LoginClusterStatus::ClusterData> &LoginClusterStatus::getData() const
{
	return m_data.get();
}

// ======================================================================

#endif
