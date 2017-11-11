//======================================================================
//
// CuiLoginManagerClusterPing.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLoginManagerClusterPing.h"

#include "sharedFoundation/Clock.h"
#include <map>

//======================================================================

CuiLoginManagerClusterPing::CuiLoginManagerClusterPing  (int windowSize) :
m_windowSize    (static_cast<uint32>(windowSize)),
m_pingMap       (new PingMap),
m_latency       (0),
m_packetSuccess (0.0f),
m_dirty         (false),
m_enabled       (true)
{
}

//----------------------------------------------------------------------

CuiLoginManagerClusterPing::CuiLoginManagerClusterPing (const CuiLoginManagerClusterPing & rhs) :
m_windowSize    (rhs.m_windowSize),
m_pingMap       (new PingMap (*rhs.m_pingMap)),
m_latency       (rhs.m_latency),
m_packetSuccess (rhs.m_packetSuccess),
m_dirty         (rhs.m_dirty),
m_enabled       (rhs.m_enabled)
{

}

//----------------------------------------------------------------------

CuiLoginManagerClusterPing &  CuiLoginManagerClusterPing::operator= (const CuiLoginManagerClusterPing & rhs)
{
	m_windowSize    = rhs.m_windowSize;
	*m_pingMap      = *rhs.m_pingMap;
	m_latency       = rhs.m_latency;
	m_packetSuccess = rhs.m_packetSuccess;
	m_dirty         = rhs.m_dirty;
	m_enabled       = rhs.m_enabled;

	return *this;
}

//----------------------------------------------------------------------

CuiLoginManagerClusterPing::~CuiLoginManagerClusterPing ()
{
	delete m_pingMap;
	m_pingMap = 0;
}

//----------------------------------------------------------------------

uint32 CuiLoginManagerClusterPing::onSendingPing    ()
{
	if (m_windowSize == 0)
		return 0;

	if (m_pingMap->size () == m_windowSize)
		m_pingMap->erase (m_pingMap->begin ());

	const uint32 currentTime = Clock::timeMs ();

	(*m_pingMap) [currentTime] = 0;

	m_dirty = true;

	return currentTime;
}

//----------------------------------------------------------------------

void   CuiLoginManagerClusterPing::onReceivedPing (uint32 timestamp)
{
	const PingMap::iterator it = m_pingMap->find (timestamp);

	if (it != m_pingMap->end ())
	{
		const uint32 currentTime = Clock::timeMs ();

		if (timestamp <= currentTime)
		{
			(*m_pingMap) [timestamp] = currentTime;
			m_dirty = true;
		}
		else
			WARNING (true, ("Time fold."));
	}
	else
		WARNING (true, ("Got a stale ping %d response", timestamp));
}

//----------------------------------------------------------------------

void    CuiLoginManagerClusterPing::recompute () const
{
	int received_count = 0;

	bool lastPacketOk = false;

	const PingMap::const_iterator end = m_pingMap->end ();
	for (PingMap::const_iterator it = m_pingMap->begin (); it != end; ++it)
	{
		const uint32 receiptTime = (*it).second;
		const uint32 timestamp   = (*it).first;

		if (receiptTime > 0)
		{
			++received_count;
			m_latency += (receiptTime - timestamp);
			lastPacketOk = true;
		}
		else
			lastPacketOk = false;
	}

	if (received_count)
		m_latency /= received_count;
	else
		m_latency = 0;

	if (m_pingMap->empty ())
		m_packetSuccess = 0.0f;
	else
	{
		const int size = static_cast<int>(m_pingMap->size ());

		//-- the last packet in flight should not be counted as packet loss
		if (!lastPacketOk)
			++received_count;

		m_packetSuccess = static_cast<float>((received_count * 1000) / size) * 0.001f;
	}

	m_dirty = false;
}

//======================================================================
