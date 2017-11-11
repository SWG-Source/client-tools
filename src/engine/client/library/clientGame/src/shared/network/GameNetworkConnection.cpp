//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/GameNetworkConnection.h"

#include "Archive/ByteStream.h"
#include "clientGame/ConfigClientGame.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <map>
#include <algorithm>

//-----------------------------------------------------------------------

// 'GameNetworkConnection::~GameNetworkConnection' : unreferenced local function has been removed
#ifdef PLATFORM_WIN32
#pragma warning(disable: 4505)
#endif

namespace GameNetworkConnectionNamespace
{
	bool s_installed = false;
	const unsigned long gs_defaultUnreliableSendRateMilliseconds = 150;
	const unsigned long gs_maxUnreliableSendRateMilliseconds = 10000;
	bool s_logNetworkTraffic = false;
}

using namespace GameNetworkConnectionNamespace;

//-----------------------------------------------------------------------

GameNetworkConnection::MessageConnectionCallback::MessageConnectionCallback(const char * const messageName) :
MessageDispatch::MessageBase(messageName)
{
}

//-----------------------------------------------------------------------

GameNetworkConnection::MessageConnectionCallback::~MessageConnectionCallback()
{
}

//-----------------------------------------------------------------------

GameNetworkConnection::MessageConnectionOverflowing::MessageConnectionOverflowing(const unsigned int newBytesPending) :
MessageDispatch::MessageBase("ConnectionOverflowing"),
bytesPending(newBytesPending)
{
}

//-----------------------------------------------------------------------

GameNetworkConnection::MessageConnectionOverflowing::~MessageConnectionOverflowing()
{
}

//-----------------------------------------------------------------------

const unsigned int GameNetworkConnection::MessageConnectionOverflowing::getBytesPending() const
{
	return bytesPending;
}

//-----------------------------------------------------------------------

GameNetworkConnection::GameNetworkConnection(const std::string & a, const unsigned short p, const NetworkSetupData & setupData) :
Connection(a, p, setupData),
MessageDispatch::Emitter(),
m_inboundTrafficBytes(0),
m_outboundTrafficBytes(0),
m_timeOfLastUnreliableSendMilliseconds(0),
m_timeOfLastReceiveMilliseconds(0),
m_unreliableSendRateMilliseconds(gs_defaultUnreliableSendRateMilliseconds),
m_sentThisFrame(false),
m_receiveThisFrame(false),
m_timeLastResetInbound  (Clock::timeMs ()),
m_timeLastResetOutbound (Clock::timeMs ()),
m_lastSentGameNetworkMessageName (0)
{
}

//-----------------------------------------------------------------------

GameNetworkConnection::~GameNetworkConnection()
{
	m_lastSentGameNetworkMessageName = 0;
}

//-----------------------------------------------------------------------
void GameNetworkConnection::install()
{
	InstallTimer const installTimer("GameNetworkConnection::install");

	DEBUG_FATAL(s_installed, ("Already installed"));

	DebugFlags::registerFlag(s_logNetworkTraffic, "ClientGame", "logNetworkTraffic");

	ExitChain::add(&remove, "ParticleManager::remove");

	s_installed = true;
}

//-----------------------------------------------------------------------
void GameNetworkConnection::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	DebugFlags::unregisterFlag(s_logNetworkTraffic);

	s_installed = false;
}

//-----------------------------------------------------------------------

void GameNetworkConnection::onConnectionClosed()
{
	static MessageConnectionCallback m("ConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void GameNetworkConnection::onConnectionOpened()
{
	static MessageConnectionCallback m("ConnectionOpened");
	emitMessage(m);
	setNoDataTimeout(600000);
}

//-----------------------------------------------------------------------

void GameNetworkConnection::onConnectionOverflowing(const unsigned int bytesPending)
{
	// not static -- bytesPending is different each time this is invoked
	MessageConnectionOverflowing m(bytesPending);
	emitMessage(m);
}

//-----------------------------------------------------------------------

void GameNetworkConnection::onReceive(const Archive::ByteStream & message)
{
	static Archive::ReadIterator ri;

	// adjust send rate if necessary
	if(! m_receiveThisFrame)
	{
		unsigned long timeMs = Clock::timeMs();
		if(timeMs < m_timeOfLastReceiveMilliseconds + m_unreliableSendRateMilliseconds)
		{
			if(m_unreliableSendRateMilliseconds / 2 >= gs_defaultUnreliableSendRateMilliseconds)
			{
				m_unreliableSendRateMilliseconds = m_unreliableSendRateMilliseconds / 2;
			}
			else
			{
				m_unreliableSendRateMilliseconds = gs_defaultUnreliableSendRateMilliseconds;
			}
		}
		m_receiveThisFrame = true;
		m_timeOfLastReceiveMilliseconds = timeMs;
	}

	ri = message.begin();
	GameNetworkMessage m(ri);
	emitMessage(m);
	m_inboundTrafficBytes += static_cast<long>(message.getSize());

	DEBUG_REPORT_LOG (s_logNetworkTraffic, ("Network: received %i bytes\n", message.getSize()));
}

//-----------------------------------------------------------------------

void GameNetworkConnection::send(const GameNetworkMessage & message, const bool reliable)
{
	if(! reliable)
	{
		if(! tryToSendUnreliable())
		{
			return;
		}
	}

	m_lastSentGameNetworkMessageName = message.getCmdName ().c_str ();

	Archive::ByteStream a;
	message.pack(a);
	Connection::send(a, reliable);
}

//-----------------------------------------------------------------------

void GameNetworkConnection::send(const Archive::ByteStream & bs, const bool r)
{
	if(! r)
	{
		if(! tryToSendUnreliable())
		{
			return;
		}
	}

	m_outboundTrafficBytes += static_cast<long>(bs.getSize());
	Connection::send(bs, r);

	DEBUG_REPORT_LOG (s_logNetworkTraffic, ("Network: [%s] sent %i bytes\n", m_lastSentGameNetworkMessageName ? m_lastSentGameNetworkMessageName : "unknown", bs.getSize()));

	m_lastSentGameNetworkMessageName = 0;
}

//-----------------------------------------------------------------------

const bool GameNetworkConnection::tryToSendUnreliable() const
{
	bool result = false;
	unsigned long timeMs = Clock::timeMs();
	if(m_sentThisFrame || timeMs > m_timeOfLastUnreliableSendMilliseconds + m_unreliableSendRateMilliseconds)
	{
		result = true;
		m_sentThisFrame = true;
		m_timeOfLastUnreliableSendMilliseconds = timeMs;
	}
	else
	{
		result = false;
	}
	return result;
}

//-----------------------------------------------------------------------

void GameNetworkConnection::updateRates() const
{
	if(m_sentThisFrame)
	{
		// adjust rate
		unsigned long timeMs = Clock::timeMs();
		if (timeMs > m_timeOfLastReceiveMilliseconds + m_unreliableSendRateMilliseconds && m_unreliableSendRateMilliseconds < gs_maxUnreliableSendRateMilliseconds)
		{
			m_unreliableSendRateMilliseconds = m_unreliableSendRateMilliseconds * 2;
		}
		m_sentThisFrame = false;
	}
	m_receiveThisFrame = false;
}

//----------------------------------------------------------------------

void GameNetworkConnection::resetInboundTraffic     (long maxTimeSinceLastUpdate) const
{
	const long curTime        = Clock::timeMs ();
	const long deltaTime      = curTime - m_timeLastResetInbound;

	if (deltaTime > maxTimeSinceLastUpdate)
	{
		m_inboundTrafficBytes      = 0;
		m_timeLastResetInbound = curTime;
	}
}

//-----------------------------------------------------------------------

void GameNetworkConnection::resetOutboundTraffic    (long maxTimeSinceLastUpdate) const
{
	const long curTime        = Clock::timeMs ();
	const long deltaTime      = curTime - m_timeLastResetOutbound;

	if (deltaTime > maxTimeSinceLastUpdate)
	{
		m_outboundTrafficBytes      = 0;
		m_timeLastResetOutbound = curTime;
	}
}

//----------------------------------------------------------------------

long GameNetworkConnection::getInboundTrafficPerSec () const
{
	const long curTime        = Clock::timeMs ();
	const long deltaTime      = curTime - m_timeLastResetInbound;

	if (deltaTime)
		return m_inboundTrafficBytes * 1000 / deltaTime;

	return 0;
}

//----------------------------------------------------------------------

long GameNetworkConnection::getOutboundTrafficPerSec () const
{
	const long curTime        = Clock::timeMs ();
	const long deltaTime      = curTime - m_timeLastResetOutbound;

	if (deltaTime)
		return m_outboundTrafficBytes * 1000 / deltaTime;

	return 0;
}

//-----------------------------------------------------------------------
