// GameNetworkConnection.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_GameNetworkConnection_H
#define	_INCLUDED_GameNetworkConnection_H

//-----------------------------------------------------------------------

#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedNetwork/Connection.h"

class GameNetworkMessage;

//-----------------------------------------------------------------------

class GameNetworkConnection : public Connection, public MessageDispatch::Emitter
{
public:

	static void install();

public:
	              GameNetworkConnection   (const std::string & remoteAddress, const unsigned short remotePort, const NetworkSetupData &);
	virtual       ~GameNetworkConnection  () = 0;
	const bool    tryToSendUnreliable     () const;
	virtual void  onConnectionClosed      ();
	virtual void  onConnectionOpened      ();
	virtual void  onConnectionOverflowing (const unsigned int bytesPending);
	virtual void  onReceive               (const Archive::ByteStream & message);
	virtual void  send                    (const Archive::ByteStream & bs, const bool r);
	void          send                    (const GameNetworkMessage & message, const bool reliable);
	long          getInboundTraffic       () const;
	long          getOutboundTraffic      () const;
	void          resetInboundTraffic     (long maxTimeSinceUpdate = 0) const;
	void          resetOutboundTraffic    (long maxTimeSinceUpdate = 0) const;
	long          getInboundTrafficPerSec () const;
	long          getOutboundTrafficPerSec() const;
	void          updateRates             () const;

public:
	class MessageConnectionCallback: public MessageDispatch::MessageBase
	{
	public:
		MessageConnectionCallback(const char * const messageName);
		~MessageConnectionCallback();
	};

	class MessageConnectionOverflowing : public MessageDispatch::MessageBase
	{
	public:
		MessageConnectionOverflowing(const unsigned int bytesPending);
		~MessageConnectionOverflowing();

		const unsigned int getBytesPending() const;
	private:
		unsigned int bytesPending;
	};

private:
	GameNetworkConnection & operator = (const GameNetworkConnection & rhs);
	GameNetworkConnection(const GameNetworkConnection & source);

private:
	mutable long             m_inboundTrafficBytes;
	mutable long             m_outboundTrafficBytes;
	mutable unsigned long    m_timeOfLastUnreliableSendMilliseconds;
	mutable unsigned long    m_timeOfLastReceiveMilliseconds;
	mutable unsigned long    m_unreliableSendRateMilliseconds;
	mutable bool             m_sentThisFrame;
	mutable bool             m_receiveThisFrame;
	mutable long             m_timeLastResetInbound;
	mutable long             m_timeLastResetOutbound;
	mutable const char*      m_lastSentGameNetworkMessageName;

	static void remove();
};

//-----------------------------------------------------------------------

inline long GameNetworkConnection::getInboundTraffic       () const
{
	return m_inboundTrafficBytes;
}

//-----------------------------------------------------------------------

inline long GameNetworkConnection::getOutboundTraffic      () const
{
	return m_outboundTrafficBytes;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GameNetworkConnection_H
