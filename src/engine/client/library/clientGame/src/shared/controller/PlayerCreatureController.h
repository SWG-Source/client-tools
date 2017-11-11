// ======================================================================
//
// PlayerCreatureController.h
// Portions Copyright 1999 Bootprint Entertainment Inc.
// Portions Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_PlayerCreatureController_H
#define INCLUDED_PlayerCreatureController_H

// ======================================================================

#include "clientGame/CreatureController.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"

struct MatchMakingCharacterResult;
class MessageQueueSecureTrade;
class Light;
class MessageQueueCommandTimer;
class NetworkId;
class PlayerObject;
class SharedBuffBuilderManagerSession;
class SharedImageDesignerManagerSession;
class StartingLocationData;

// ======================================================================

class PlayerCreatureController : public CreatureController
{
public:

	typedef void (*ModeCallback) (void* context);

	struct Messages
	{
		struct BiographyRetrieved
		{
			typedef std::pair<NetworkId, PlayerObject const * const> BiographyOwner;
		};

		struct BuffBuilderChangeReceived
		{
			typedef SharedBuffBuilderManagerSession Payload;
		};

		struct BuffBuilderCancelReceived
		{
			typedef SharedBuffBuilderManagerSession Payload;
		};

		struct CharacterMatchRetrieved
		{
			typedef MatchMakingCharacterResult MatchResults;
		};

		struct StartingLocationsReceived
		{
			typedef stdvector<std::pair <StartingLocationData, bool> >::fwd Payload;
		};

		struct StartingLocationSelectionResult
		{
			typedef std::pair<std::string, bool> Payload;
		};

		struct ImageDesignerChangeReceived
		{
			typedef SharedImageDesignerManagerSession Payload;
		};

		struct ImageDesignerCancelReceived
		{
			typedef SharedImageDesignerManagerSession Payload;
		};

		struct ShipParkingDataReceived
		{
			typedef stdvector<std::pair<NetworkId, std::string> >::fwd Payload;
		};

		struct GroupMemberInvitationToLaunchReceived
		{
			typedef std::pair<NetworkId /* invitee */, bool /* accepted answer */> Payload;
		};
		
		struct CommandTimerDataReceived
		{
			typedef MessageQueueCommandTimer Payload;
		};

		struct AutoAimToggled
		{
			typedef CreatureObject Payload;
		};
	};

public:

	static void install ();

	static void setRunWhenMoving (bool b);
	static bool getRunWhenMoving ();

public:

	explicit PlayerCreatureController (CreatureObject* newOwner);
	virtual ~PlayerCreatureController (void);

	virtual void          conclude ();
	virtual float         getCurrentSpeed () const;
	virtual void          setCurrentSpeed (float currentSpeed);
	virtual bool          shouldApplyAnimationDrivenLocomotion () const;

	void                  setDesiredYaw_w (float desiredYaw_w, bool shouldFaceDesiredYaw);
	void                  warpClient (const Transform& transform_p);
	void                  sendCommandQueueEnqueue(uint32 sequenceId, uint32 commandHash, NetworkId const &targetId, Unicode::String const &params);
	void                  sendCommandQueueRemove(uint32 sequenceId);

	void                  setModeCallback (ModeCallback modeCallback, void* context);

	void                  setAutoFollowTarget(CreatureObject *target);
	CreatureObject       *getAutoFollowTarget();
	const CreatureObject *getAutoFollowTarget() const;

	MessageQueue         *getMessageQueue();
	const MessageQueue   *getMessageQueue() const;

	void                  setServerCellObject (Object const * serverCellObject);
	void                  setServerTransform  (const Transform& serverTransform);

	void                  activatePlayerLight ();

	float                 getDesiredSpeed () const;

	uint32 getContainingBuildingCrc() const;
	uint32 getContainingCellCrc() const;

	void allowMovement(bool const allow);

protected:

	virtual float         realAlter (float time);

private:

	static void remove ();

private:

	virtual void          handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);
	virtual void          handleNetUpdateTransform (const MessageQueueDataTransform& message);
	virtual void          handleNetUpdateTransformWithParent (const MessageQueueDataTransformWithParent& message);

	void                  handleSecureTradeMessage(const MessageQueueSecureTrade &);
	void                  ackTeleport(int sequenceId);
	void                  checkPlayerMusic(const float elapsedTime);
	void updateLookAtTargetSlot(CreatureObject & owner) const;
	bool shouldSendUpdatedTransform() const;
	bool shouldProcessMovement() const;
	
	void updateBuildingAndCellInformation(CreatureObject const * const owner);

	bool hasLookAtYawChanged(bool reliable);

private:

	PlayerCreatureController (void);
	PlayerCreatureController (const PlayerCreatureController&);
	PlayerCreatureController& operator= (const PlayerCreatureController&);

private:

	int32                     m_serverSequenceNumber;

	float                     m_desiredYaw_w;
	bool                      m_shouldFaceDesiredYaw;
	float                     m_currentSpeed;

	//-- used for sending move packets
	Timer                     m_sendTransformTimer;
	bool                      m_sendTransformThisFrame;
	Transform                 m_previousTransform_p;
	float                     m_previousLookAtYaw;
	Timer                     m_sendReliableTransformTimer;
	bool                      m_sendReliableTransformThisFrame;
	Transform                 m_previousReliableTransform_p;
	float                     m_previousReliableLookAtYaw;
	
	ModeCallback              m_modeCallback;
	void*                     m_context;
	NetworkId                 m_currentMode;
	Watcher<CreatureObject> * m_autoFollowTarget;
	Vector                    m_autoFollowTargetOffset;
	bool                      m_autoRun;

	enum MouseWalkMode
	{
		MWM_none,
		MWM_ahead,
		MWM_followMouse
	};

	MouseWalkMode            m_mouseWalkMode;

	Light*                   m_light;

	ConstWatcher<Object>     m_serverCellObject;
	Transform                m_serverTransform;

	Unicode::String          m_autoFollowTargetName;
	float                    m_playerMovementTimer;

	float                    m_desiredSpeed;

	uint32 m_buildingSharedTemplateNameCrc;
	uint32 m_cellNameCrc;
	bool m_allowMovement;
};         

// ----------------------------------------------------------------------

inline void PlayerCreatureController::setServerCellObject(Object const * serverCellObject)
{
	m_serverCellObject = serverCellObject;
}

//----------------------------------------------------------------------

inline void PlayerCreatureController::setServerTransform(const Transform& serverTransform)
{
	m_serverTransform = serverTransform;
}

//----------------------------------------------------------------------

inline uint32 PlayerCreatureController::getContainingBuildingCrc() const
{
	return m_buildingSharedTemplateNameCrc;
}

//----------------------------------------------------------------------

inline uint32 PlayerCreatureController::getContainingCellCrc() const
{
	return m_cellNameCrc;
}

// ======================================================================

#endif
