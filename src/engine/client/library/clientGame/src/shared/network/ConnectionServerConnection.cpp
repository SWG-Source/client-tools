// ConnectionServerConnection.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ConnectionServerConnection.h"
#include "clientGame/ConfigClientGame.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "clientGame/GameNetwork.h"
#include "sharedFoundation/Clock.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

//-----------------------------------------------------------------------

ConnectionServerConnection::ConnectionServerConnection(const std::string & a, const unsigned short p, const NetworkSetupData & setupData) :
GameNetworkConnection(a, p, setupData),
m_lagRequestTime(0),
m_connectionServerLag(0),
m_gameServerLag(0),
m_lagRequestDelay(ConfigClientGame::getLagRequestDelay()),
m_pendingConnectionServerLagResponse(false),
m_pendingGameServerLagResponse(false),
m_deferredPackets(0)
{
}

//-----------------------------------------------------------------------

ConnectionServerConnection::~ConnectionServerConnection()
{
	if (isDeferringIncomingPackets())
	{
		endDeferringIncomingPackets(false);
	}
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onConnectionClosed()
{
	if (isDeferringIncomingPackets())
	{
		endDeferringIncomingPackets(false);
	}

	// ------------------------------------------------

	GameNetworkConnection::onConnectionClosed();

	// emit a message for Receiver objects specifically listening for
	// a ConnectionServerConnection to close
	static MessageConnectionCallback m("ConnectionServerConnectionClosed"); 
	emitMessage(m);
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onConnectionOpened()
{
	GameNetworkConnection::onConnectionOpened();

	// send credentials
	ClientIdMsg l(GameNetwork::getLoginToken(), GameNetwork::getLoginTokenLength(), ConfigClientGame::getGameBitsToClear());
	send(l, true);

	// emit a message for Receiver objects specifically listening for
	// a ConnectionServerConnection to open
	static MessageConnectionCallback m("ConnectionServerConnectionOpened"); 
	emitMessage(m);

	updateLagStats();
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onConnectionOverflowing(const unsigned int bytesPending)
{
	GameNetworkConnection::onConnectionOverflowing(bytesPending);
	DEBUG_REPORT_LOG(true, ("User Connection is overflowing! Stop sending so much data!!!\n"));
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onReceive(const Archive::ByteStream & message)
{
	if (isDeferringIncomingPackets())
	{
		_deferIncomingPacket(message);
	}
	else
	{
		_onReceive(message);
	}
}

//-----------------------------------------------------------------------

void  ConnectionServerConnection::_deferIncomingPacket(const Archive::ByteStream & message)
{
	DEBUG_FATAL(!m_deferredPackets, ("Trying to defer packet when not in packet deferral mode.\n"));

	Archive::ByteStream *savedMessage = new Archive::ByteStream(message);
	m_deferredPackets->push_back(savedMessage);
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::_onReceive(const Archive::ByteStream & message)
{
	static Archive::ReadIterator ri;
	ri = message.begin();
	GameNetworkMessage m(ri);
	ri = message.begin();

	if(m.isType("ConnectionServerLagResponse"))
	{
		m_pendingConnectionServerLagResponse = false;
		m_connectionServerLag = Clock::timeMs() - m_lagRequestTime;
		sendLagReport();
	}
	else if(m.isType("GameServerLagResponse"))
	{
		m_pendingGameServerLagResponse = false;
		m_gameServerLag = Clock::timeMs() - m_lagRequestTime;
		sendLagReport();
	}
	else if(m.isType("LagRequest"))
	{
		if(ConfigClientGame::getLagRequestDelay() == 0)
		{
			// lag requests are disabled
			if((! m_pendingConnectionServerLagResponse) && (! m_pendingGameServerLagResponse))
			{
				// and there aren't any outstanding requests
				m_lagRequestDelay = 1;
			}
		}
	}
	GameNetworkConnection::onReceive(message);
}

//---------------------------------------------------------------------

void ConnectionServerConnection::sendLagReport()
{
	if(! m_pendingGameServerLagResponse && ! m_pendingConnectionServerLagResponse)
	{
		GenericValueTypeMessage<std::pair<int, int> > lagReport("LagReport", std::make_pair(m_connectionServerLag, m_gameServerLag));
		send(lagReport, true);
		m_lagRequestDelay = ConfigClientGame::getLagRequestDelay();

		static const int lagReportThreshold = ConfigClientGame::getLagReportThreshold();
		if(lagReportThreshold > 0)
		{
			if(m_gameServerLag > lagReportThreshold || m_connectionServerLag > lagReportThreshold)
			{
				LOG("Network", ("Lag Report: ConnectionServer(%s)=%d, GameServer=%d", getRemoteAddress().c_str(), m_connectionServerLag, m_gameServerLag));
			}
		}
	}
}

//---------------------------------------------------------------------

int ConnectionServerConnection::getConnectionServerLag() const
{
	return m_connectionServerLag;
}

//---------------------------------------------------------------------

int ConnectionServerConnection::getGameServerLag() const
{
	return m_gameServerLag;
}

//---------------------------------------------------------------------

void ConnectionServerConnection::updateLagStats()
{
	if( (!m_pendingConnectionServerLagResponse) && (!m_pendingGameServerLagResponse))
	{
		if(m_lagRequestDelay > 0 && (m_lagRequestDelay < static_cast<int>(Clock::timeMs() - m_lagRequestTime)))
		{
			m_lagRequestTime = Clock::timeMs();
			GameNetworkMessage lag("LagRequest");
			m_lagRequestDelay = ConfigClientGame::getLagRequestDelay();
			send(lag, true);
		}
	}
}

//---------------------------------------------------------------------

void ConnectionServerConnection::beginDeferringIncomingPackets()
{
	if (isDeferringIncomingPackets())
	{
		return;
	}

	m_deferredPackets = new PacketList;
}

//---------------------------------------------------------------------

void ConnectionServerConnection::endDeferringIncomingPackets(bool emitThem)
{
	if (m_deferredPackets)
	{
		PacketList *deferredPackets = m_deferredPackets;
		m_deferredPackets=0;

		while (!deferredPackets->empty())
		{
			Archive::ByteStream *packet = deferredPackets->front();
			deferredPackets->pop_front();
			if (emitThem)
			{
				onReceive(*packet);
			}
			delete packet;
		}

		delete  deferredPackets;
	}
}
