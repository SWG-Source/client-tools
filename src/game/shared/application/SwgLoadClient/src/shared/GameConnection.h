// GameConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_GameConnection_H
#define	_INCLUDED_GameConnection_H

//-----------------------------------------------------------------------

#include "LoadConnection.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Timer.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

class Client;

//-----------------------------------------------------------------------

class GameConnection : public LoadConnection
{
public:
	GameConnection(Client * owner, const std::string & address, const unsigned short port, const NetworkId &characterId, const NetworkSetupData &setupData);
	~GameConnection();

	const Vector &   getPosition          () const;
	const Vector &   getVelocity          () const;
	const bool       getReadyToSimulate   () const;

	void             onConnectionClosed   ();
	void             onConnectionOpened   ();
	void             onReceive            (const Archive::ByteStream & data);

	const bool       tryToSendUnreliable  ();
	void             update               ();

private:
	GameConnection & operator = (const GameConnection & rhs);
	GameConnection(const GameConnection & source);

	void  chat    (char const *text);
	void  social  ();

private:
	NetworkId          m_characterObjectId;
	NetworkId          m_characterContainerId;
	bool               m_characterInShip;
	Client *           m_owner;
	Transform          m_transform;
	bool               m_readyToSimulate;
	int                m_sequenceNumber;
	Timer              m_updateTransformTimer;
	Vector             m_velocity;
	Timer              m_chatEventTimer;
	int                m_chatTextCount;
	Timer              m_socialEventTimer;
	unsigned long      m_timeOfLastUnreliableSendMilliseconds;
	unsigned long      m_timeOfLastReceiveMilliseconds;
	unsigned long      m_unreliableSendRateMilliseconds;
	bool               m_sentThisFrame;
	bool               m_receiveThisFrame;
	Timer              m_shipTransformUpdateTimer;
	Timer              m_shipTransformReliableUpdateTimer;
	Vector             m_shipGoalPosition;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GameConnection_H
