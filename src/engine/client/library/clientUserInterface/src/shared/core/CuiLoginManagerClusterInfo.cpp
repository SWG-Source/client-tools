//======================================================================
//
// CuiLoginManagerClusterInfo.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLoginManagerClusterInfo.h"

#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiLoginManagerClusterPing.h"
#include "sharedNetwork/Address.h"

//======================================================================

CuiLoginManagerClusterInfo::CuiLoginManagerClusterInfo () :
id             (0),
name           (),
disableCharacterCreation(false),
up             (false),
loading        (false),
locked         (false),
restricted     (false),
isFull         (false),
onlinePlayerLimit (0),
onlineFreeTrialLimit (0),
notRecommended (false),
population     (-1),
populationStatus(LoginClusterStatus::ClusterData::PS_very_light),
maxAvatars     (0),
numAvatars     (0),
timeZone       (0),
branch         (),
version        (0),
netVersionMatch(false),
m_host         (),
m_port         (0),
m_pingPort     (0),
m_ping         (new Ping (ConfigClientUserInterface::getConnectionServerPingWindow ())),
m_address      (0),
m_pingAddress  (0)
{
}

//----------------------------------------------------------------------

CuiLoginManagerClusterInfo::CuiLoginManagerClusterInfo (const CuiLoginManagerClusterInfo & rhs) :
id             (rhs.id),
name           (rhs.name),
disableCharacterCreation(rhs.disableCharacterCreation),
up             (rhs.up),
loading        (rhs.loading),
locked         (rhs.locked),
restricted     (rhs.restricted),
isFull         (rhs.isFull),
onlinePlayerLimit (rhs.onlinePlayerLimit),
onlineFreeTrialLimit (rhs.onlineFreeTrialLimit),
notRecommended (rhs.notRecommended),
population     (rhs.population),
populationStatus(rhs.populationStatus),
maxAvatars     (rhs.maxAvatars),
numAvatars     (rhs.numAvatars),
timeZone       (rhs.timeZone),
branch         (rhs.branch),
version        (rhs.version),
netVersionMatch(rhs.netVersionMatch),
m_host         (rhs.m_host),
m_port         (rhs.m_port),
m_pingPort     (rhs.m_pingPort),
m_ping         (new Ping (*rhs.m_ping)),
m_address      (new Address(rhs.m_host, rhs.m_port)),
m_pingAddress  (new Address(rhs.m_host, rhs.m_pingPort))
{
}

//----------------------------------------------------------------------

CuiLoginManagerClusterInfo::CuiLoginManagerClusterInfo (uint32 _id, const std::string & _name, bool _disableCharacterCreation) :
id             (_id),
name           (_name),
disableCharacterCreation(_disableCharacterCreation),
up             (false),
loading        (false),
locked         (false),
restricted     (false),
isFull         (false),
onlinePlayerLimit (0),
onlineFreeTrialLimit (0),
notRecommended (false),
population     (-1),
populationStatus(LoginClusterStatus::ClusterData::PS_very_light),
maxAvatars     (0),
numAvatars     (0),
timeZone       (0),
branch         (),
version        (0),
netVersionMatch(false),
m_host         (),
m_port         (0),
m_pingPort     (0),
m_ping         (new Ping (ConfigClientUserInterface::getConnectionServerPingWindow ())),
m_address      (0),
m_pingAddress  (0)
{
}

//----------------------------------------------------------------------

CuiLoginManagerClusterInfo::~CuiLoginManagerClusterInfo ()
{
	delete m_ping;
	m_ping = 0;
	delete m_address;
	m_address = 0;
	delete m_pingAddress;
	m_pingAddress = 0;
}


//----------------------------------------------------------------------

CuiLoginManagerClusterInfo & CuiLoginManagerClusterInfo::operator= (const CuiLoginManagerClusterInfo & rhs)
{
	if(&rhs == this)
		return *this;

	id             = rhs.id;
	name           = rhs.name;
	disableCharacterCreation = rhs.disableCharacterCreation;
	setHost    (rhs.m_host);
	setPort    (rhs.m_port);
	setPingPort(rhs.getPingPort());
	up             = rhs.up;
	loading        = rhs.loading;
	locked         = rhs.locked;
	restricted     = rhs.restricted;
	isFull         = rhs.isFull;
	onlinePlayerLimit = rhs.onlinePlayerLimit;
	onlineFreeTrialLimit = rhs.onlineFreeTrialLimit;
	notRecommended = rhs.notRecommended;
	population     = rhs.population;
	populationStatus = rhs.populationStatus;
	maxAvatars     = rhs.maxAvatars;
	numAvatars     = rhs.numAvatars;
	timeZone       = rhs.timeZone;
	*m_ping        = *rhs.m_ping;
	branch         = rhs.branch;
	version        = rhs.version;
	netVersionMatch = rhs.netVersionMatch;
	return *this;
}

//----------------------------------------------------------------------

float CuiLoginManagerClusterInfo::getPacketSuccess     () const
{
	return m_ping->getPacketSuccess ();
}

//----------------------------------------------------------------------

uint32 CuiLoginManagerClusterInfo::getLatencyMs         () const
{
	return m_ping->getLatency ();
}

//----------------------------------------------------------------------

const Address* CuiLoginManagerClusterInfo::getAddress()
{
	if(!m_address)
		m_address = new Address(m_host, m_port);
	return m_address;
}

//----------------------------------------------------------------------

const Address* CuiLoginManagerClusterInfo::getPingAddress()
{
	if(!m_pingAddress)
		m_pingAddress = new Address(m_host, m_pingPort);
	return m_pingAddress;
}

//----------------------------------------------------------------------

void CuiLoginManagerClusterInfo::setHost(const std::string& host)
{
	m_host = host;
	if(m_address)
		delete m_address;
	m_address = new Address(m_host, m_port);
}

//----------------------------------------------------------------------

void CuiLoginManagerClusterInfo::setPort(uint16 port)
{
	m_port = port;
	if(m_address)
		delete m_address;
	m_address = new Address(m_host, m_port);
}

//----------------------------------------------------------------------

void CuiLoginManagerClusterInfo::setPingPort(uint16 port)
{
	m_pingPort = port;
	if(m_pingAddress)
		delete m_pingAddress;
	m_pingAddress = new Address(m_host, m_pingPort);
}

//----------------------------------------------------------------------

float CuiLoginManagerClusterInfo::computePerformance   () const
{
	const uint32 latency       = m_ping->getLatency       ();
	const float  packetSuccess = m_ping->getPacketSuccess ();

	// rank latency on a scale from 0-2000 ms.  Anything greater than 2000 ms is considered shite

	return packetSuccess * static_cast<float>(2000 - std::min (uint32(2000), latency)) / 2000.f;
}

//======================================================================
