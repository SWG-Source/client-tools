// ======================================================================
//
// PlanetServerConnection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetWatcher.h"
#include "PlanetServerConnection.h"

#include "PlanetWatcher.h"
#include "PlanetWatcherUtility.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/FrameEndMessage.h"
#include "sharedNetworkMessages/GalaxyLoopTimesResponse.h"
#include "sharedNetworkMessages/GameServerStatus.h"
#include "sharedNetworkMessages/PlanetObjectStatusMessage.h"
#include "sharedNetworkMessages/PlanetNodeStatusMessage.h"

//-----------------------------------------------------------------------

PlanetServerConnection::PlanetServerConnection(const std::string & a, const unsigned short p,PlanetWatcher *watcher) :
		Connection(a, p, NetworkSetupData()),
		m_watcher(watcher),
		m_gotUpdates(false)
{
}

//-----------------------------------------------------------------------

PlanetServerConnection::~PlanetServerConnection()
{
}

//-----------------------------------------------------------------------

void PlanetServerConnection::onConnectionClosed()
{
	m_watcher->connectionLost();
}

//-----------------------------------------------------------------------

void PlanetServerConnection::onConnectionOpened()
{
	DEBUG_REPORT_LOG(true,("Connection to Planet opened.\n"));
	m_watcher->connectionSuccess();
}

//-----------------------------------------------------------------------

void PlanetServerConnection::onReceive(const Archive::ByteStream &message) 
{
	Archive::ReadIterator r = message.begin();
	GameNetworkMessage m(r);
	r = message.begin();
	
	if(m.isType("PlanetObjectStatusMessage"))
	{
		//DEBUG_REPORT_LOG(true,("Got PlanetObjectStatusMessage.\n"));
		PlanetObjectStatusMessage msg(r);
		std::vector<PlanetObjectStatusMessageData> data;
		msg.getData(data);

		for (std::vector<PlanetObjectStatusMessageData>::iterator i=data.begin(); i!=data.end(); ++i)
		{
			if (i->m_deleteObject)
			{
				m_watcher->removeObject(i->m_objectId);
			}
			else
			{
				m_watcher->updateObject(i->m_objectId, i->m_authoritativeServer, i->m_x, i->m_z, i->m_interestRadius, i->m_objectTypeTag, i->m_level, i->m_hibernating, i->m_templateCrc, i->m_aiActivity, i->m_creationType);
			}
		}
		m_gotUpdates = true;
	}
	else if(m.isType("GameServerStatus"))
	{
		GameServerStatus msg(r);
		if(msg.isOnline())
		{
			char pidBuf[128] = {"\0"};
			snprintf(pidBuf, sizeof(pidBuf), "%d", msg.getServerInfo().systemPid);
			m_watcher->addGameServer(msg.getServerInfo().serverId, msg.getServerInfo().ipAddress.c_str(), pidBuf, msg.getServerInfo().sceneId.c_str());
		}
		else
		{
			m_watcher->removeGameServer(msg.getServerInfo().serverId);
		}
	}
	else if(m.isType("PlanetNodeStatusMessage"))
	{
		//DEBUG_REPORT_LOG(true,("Got PlanetNodeStatusMessage.\n"));
		PlanetNodeStatusMessage msg(r);
		std::vector<PlanetNodeStatusMessageData> data;
		msg.getData(data);

		for (std::vector<PlanetNodeStatusMessageData>::iterator i=data.begin(); i!=data.end(); ++i)
		{
			const std::vector<uint32> &servers = i->m_servers;
			const std::vector<int> subscriptionCounts = i->m_subscriptionCounts;
			
			if (i->m_loaded)
			{
				m_watcher->setCellSubscription(i->m_x,i->m_z, servers, subscriptionCounts);
			}
			else
			{
				m_watcher->clearCellGameServersIds(i->m_x,i->m_z);
			}
		}
		m_gotUpdates = true;
	}
	else if(m.isType("GalaxyLoopTimesResponse"))
	{
		GalaxyLoopTimesResponse msg(r);
		if(m_watcher)
		{
			m_watcher->setServerLoopTime(msg.getCurrentFrameMilliseconds(), msg.getLastFrameMilliseconds());
		}
	}
	else if(m.isType("FrameEndMessage"))
	{
		FrameEndMessage msg(r);
		m_watcher->setServerFrameInfo(msg.getProcessId(), msg.getFrameTime(), msg.getProfilerData());
	}
}

// ======================================================================
