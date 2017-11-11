// ======================================================================
//
// GrenadeLobAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_GrenadeLobAction_H
#define INCLUDED_GrenadeLobAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/TemporaryCrcString.h"
#include "sharedMath/Vector.h"

class CallbackAnimationNotification;
class CellProperty;
class CrcLowerString;
class GrenadeLobActionTemplate;
class Object;

template <class T>
class Watcher;

// ======================================================================

class GrenadeLobAction: public PlaybackAction
{
public:

	enum State
	{
		S_setupNotification,   // initial state, setup animation notification.
		S_waitingForAnimation, // we're waiting for the throw animation to trigger the throw.
		S_initialize,          // startup mode.
		S_lobbing,             // the grenade is flying through the air.
		S_landed               // the grenade has hit the ground and is waiting to explode.
	};

public:

	GrenadeLobAction(const GrenadeLobActionTemplate &actionTemplate);
	virtual ~GrenadeLobAction();

	virtual void cleanup(PlaybackScript &playbackScript);
	virtual bool update(float deltaTime, PlaybackScript &script);

private:

	static float                    getObjectRadius(const Object &object);
	static Vector                   createRandomVector(float minDistance, float maxDistance);
	static Vector                   getTargetPosition(const Object &targetObject, bool isHit, bool throwAtCenter, bool randomOffset);

	static void                     staticNotificationCallback(void *context, int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger);

private:

	const GrenadeLobActionTemplate &getOurTemplate() const;
	bool                            initialize(PlaybackScript &script);
	bool                            isDefenderHit(const PlaybackScript &script) const;

	bool                            updateUninitialized(PlaybackScript &script);
	bool                            updateLobbing(float deltaTime, PlaybackScript &script);
	bool                            updateLanded(float deltaTime);

	void                            notificationCallback(int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger);

	// Disabled.
	GrenadeLobAction();
	GrenadeLobAction(const GrenadeLobAction&);
	GrenadeLobAction &operator =(const GrenadeLobAction&);

private:

	State               m_state;

	Vector              m_xzVelocity;

	float               m_yAcceleration;
	float               m_yStartVelocity;
	float               m_yStartPosition;

	float               m_accumulatedFlyTime;
	float               m_totalFlyTime;

	float               m_accumulatedSitTime;
	float               m_totalSitTime;

	bool                m_trackTarget;
	bool                m_showGrenadeDuringFlight;
	Watcher<Object>    *m_grenadeWatcher;

	Vector              m_grenadeDestination;
	const CellProperty *m_grenadeDestinationCell;
	CrcLowerString     *m_grenadeExplodeEffectName;

	float               m_grenadePitchSpeed;
	float               m_grenadeYawSpeed;

	bool                           m_grenadeDeleted;
	CallbackAnimationNotification *m_animationNotification;
	TemporaryCrcString             m_throwAnimationMessageName;
	bool                           m_receivedThrowMessage;
};

// ======================================================================

#endif
