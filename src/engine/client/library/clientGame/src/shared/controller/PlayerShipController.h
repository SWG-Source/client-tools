// ======================================================================
//
// PlayerShipController.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_PlayerShipController_H
#define INCLUDED_PlayerShipController_H

// ======================================================================

#include "clientGame/ShipController.h"
#include "sharedObject/CachedNetworkId.h"

class MessageQueueDataTransform;
class MessageQueueDataTransformWithParent;
class ShipDynamicsModel;
class ShipObject;
class ShipUpdateTransformCollisionMessage;

// ======================================================================

class PlayerShipController : public ShipController
{
public:

	static void install ();

public:

	explicit PlayerShipController(ShipObject * newOwner);
	virtual ~PlayerShipController();

	virtual void conclude();
	void receiveTransform(ShipUpdateTransformCollisionMessage const & shipUpdateTransformCollisionMessage);
	virtual void receiveTransform(ShipUpdateTransformMessage const & shipUpdateTransformMessage);
	virtual void handleMessage(int message, float value, const MessageQueue::Data* data, uint32 flags);

	virtual PlayerShipController * asPlayerShipController();
	virtual PlayerShipController const * asPlayerShipController() const;

	bool isVirtualJoystickActive() const;

	float getThrottlePosition() const;
	void setThrottlePosition(float throttlePosition, bool stopAutopilot);

	void lockInputState(bool lock, bool allowTransforms = false);
	bool isInputStateLocked() const;

	void cancelAutopilot();
	void engageAutopilotRollLevel();
	void engageAutopilotFullyLevel();
	void engageAutopilotToLocation(Vector const & target);
	void engageAutopilotToDirection(Vector const & direction);
	void engageAutopilotFollow(ShipObject const & target);

#ifdef ENABLE_FORMATIONS
	bool getInFormation() const;
	void setInFormation(bool inFormation);
#endif

	void matchSpeed(ShipObject const & target);
	ShipObject const * getShipToFollow() const;
	bool isFollowing() const;
	bool isAutoPilotEngaged() const;

protected:

	virtual float realAlter(float time);

	virtual void handleNetUpdateTransform(MessageQueueDataTransform const & message);
	virtual void handleNetUpdateTransformWithParent(MessageQueueDataTransformWithParent const & message);

private:

	PlayerShipController();
	PlayerShipController(PlayerShipController const &);
	PlayerShipController & operator=(PlayerShipController const &);

private:
	enum AutopilotMode { AM_rollLevel, AM_fullyLevel, AM_toHeading, AM_toLocation, AM_follow };


private:
	void doAutopilot(float & yawPosition, float & pitchPosition, float & rollPosition, float & throttlePosition);
	float doAutopilotTurnToHeading(float & pitchPosition, float & yawPosition);
	bool doAutopilotRollLevel(float & rollPosition);
	void internalEngageAutopilot(AutopilotMode const newMode);
	void internalEngageAutopilotRollLevel();
	void internalEngageAutopilotFullyLevel();
	void internalEngageAutopilotToLocation(Vector const & target);
	void internalEngageAutopilotToDirection(Vector const & direction);
	void internalEngageAutopilotFollow(ShipObject const & target);
	void sendTransform(bool const reliable);
	void setThrottlePositionFromJoystick(float joystickThrottlePosition, bool forceOverride);
	void turnOffBooster() const;

private:

	ShipDynamicsModel * const m_serverShipDynamicsModel;

	//-- used for sending move packets
	Timer m_sendTransformTimer;
	bool m_sendTransformThisFrame;
	Timer m_sendReliableTransformTimer;
	bool m_sendReliableTransformThisFrame;
	Transform m_previousTransform_p;
	float m_throttlePosition;
	float m_throttleAcceleration;
	float m_throttleRepeatDelay;
	bool m_virtualJoystickActive;
	CachedNetworkId m_shipToFollow;	
	uint32 m_serverToClientLastSyncStamp;
	bool m_lockInputState;
	bool m_allowTransformsWhileLocked;
	bool m_sentFinalTransform;

	// Autopilot
	bool m_autopilotEngaged;
	AutopilotMode m_autopilotMode;
	Vector m_autopilotTargetHeading;
	Vector m_autopilotTargetLocation;
	Vector m_autopilotFollowLocation;
	Timer m_autoLevelTimer;
	
	bool m_inFormation;

	float m_lastJoystickThrottlePosition;

	NetworkId m_shipThatLastShotPlayer;
	bool m_swapRollYawAxes;
	bool m_nearZoneEdge;
	float m_throttleDeltaSliderPosition;
	float m_throttleDeltaAxisPosition;
	bool m_sentAutoPilotEngagedMessage;
	bool m_boosterWasActive;
};

//======================================================================

#ifdef ENABLE_FORMATIONS
inline bool PlayerShipController::getInFormation() const
{
	return m_inFormation;
}
#endif

//======================================================================

#endif
