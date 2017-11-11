//======================================================================
//
// CuiLoginManagerClusterInfo.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiLoginManagerClusterInfo_H
#define INCLUDED_CuiLoginManagerClusterInfo_H

//======================================================================

#include "sharedNetworkMessages/LoginClusterStatus.h"

#include <string>
class Address;
class CuiLoginManagerClusterPing;

//----------------------------------------------------------------------

class CuiLoginManagerClusterInfo
{
public:

	typedef CuiLoginManagerClusterPing Ping;

	uint32      id;
	std::string name;
	bool        disableCharacterCreation;
	bool        up;
	bool        loading;
	bool        locked;
	bool        restricted;
	bool        isFull;
	uint32      onlinePlayerLimit;
	uint32      onlineFreeTrialLimit;
	bool        notRecommended;
	int         population; // must be signed, -1 is a legitimate value meaning not available (for security reason)
	LoginClusterStatus::ClusterData::PopulationStatus populationStatus;
	uint32      maxAvatars;
	uint32      numAvatars;
	int         timeZone;
	std::string branch;
	uint32      version;
	bool        netVersionMatch;

	Ping &                       getPing ();
	const Ping &                 getPing () const;

	                             CuiLoginManagerClusterInfo  ();
	                             CuiLoginManagerClusterInfo  (uint32 _id, const std::string & _name, bool _disableCharacterCreation);
	                            ~CuiLoginManagerClusterInfo ();
	                             CuiLoginManagerClusterInfo  (const CuiLoginManagerClusterInfo & rhs);
	CuiLoginManagerClusterInfo & operator= (const CuiLoginManagerClusterInfo & rhs);

	void                         setPort              (uint16 port);
	void                         setPingPort          (uint16 port);
	void                         setHost              (const std::string& host);
	float                        getPacketSuccess     () const;
	uint32                       getLatencyMs         () const;
	float                        computePerformance   () const;
	const Address*               getAddress           ();
	const Address*               getPingAddress       ();
	const std::string&           getHost              () const;
	uint16                       getPort              () const;
	uint16                       getPingPort          () const;

private:
	std::string                  m_host;
	uint16                       m_port;
	uint16                       m_pingPort;
	Ping *                       m_ping;
	Address*                     m_address;
	Address*                     m_pingAddress;
};

//----------------------------------------------------------------------

inline CuiLoginManagerClusterPing & CuiLoginManagerClusterInfo::getPing ()
{
	return *NON_NULL (m_ping);
}

//----------------------------------------------------------------------

inline uint16 CuiLoginManagerClusterInfo::getPort() const
{
	return m_port;
}

//----------------------------------------------------------------------

inline uint16 CuiLoginManagerClusterInfo::getPingPort() const
{
	return m_pingPort;
}

//----------------------------------------------------------------------

inline const std::string& CuiLoginManagerClusterInfo::getHost() const
{
	return m_host;
}

//----------------------------------------------------------------------

inline const CuiLoginManagerClusterInfo::Ping & CuiLoginManagerClusterInfo::getPing () const
{
	return *NON_NULL (m_ping);
}

//======================================================================

#endif
