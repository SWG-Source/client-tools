//======================================================================
//
// SwgCuiHudWindowManagerGround.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHudWindowManagerGround_H
#define INCLUDED_SwgCuiHudWindowManagerGround_H

#include "swgClientUserInterface/SwgCuiHudWindowManager.h"

#include "sharedNetworkMessages/MessageQueueCyberneticsOpen.h"

class CreatureObject;
class PlayerObject;
class SwgCuiGroundRadar;
class SwgCuiStatusGround;

//======================================================================

class SwgCuiHudWindowManagerGround : public SwgCuiHudWindowManager
{
public:
	                         SwgCuiHudWindowManagerGround (const SwgCuiHud & hud, CuiWorkspace & workspace);
	                         ~SwgCuiHudWindowManagerGround ();

	virtual void             receiveMessage(const MessageDispatch::Emitter & emitter, const MessageDispatch::MessageBase & message);
	virtual void             update();

	void                     spawnSurvey                           ();
	void                     spawnShipChoose                       (NetworkId const & terminalId) const;
	void                     spawnShipView                         (NetworkId const & shipId, NetworkId const & terminalId) const;
	void                     spawnDroidCommand                     (NetworkId const & droidControlDeviceId) const;
	void                     spawnGroupLootLottery                 (NetworkId const & container);
	void                     closeGroupLootLottery                 (NetworkId const & container);
	void                     spawnCybernetics                      (NetworkId const & npc, MessageQueueCyberneticsOpen::OpenType openType) const;


	void clearLookAtTarget();
	bool isShowingLookAtTarget() const;

	void onTargetChanged(const CreatureObject & payload); // Do not move the const.
	void onPlayerSetup(const CreatureObject & payload);
	void onJediStateChanged(const PlayerObject & creature);
	void onPetChanged(const PlayerObject & creature);

	virtual void handlePerformActivate();
	virtual void handlePerformDeactivate();

	void updateTargetStatusPages();

private:
	                         SwgCuiHudWindowManagerGround ();
	                         SwgCuiHudWindowManagerGround (const SwgCuiHudWindowManagerGround & rhs);
	SwgCuiHudWindowManagerGround & operator= (const SwgCuiHudWindowManagerGround & rhs);

private:
	void createPlayerStatusPage();
	void removePlayerStatusPage();

	SwgCuiGroundRadar * m_groundRadarMediator;
	SwgCuiStatusGround *  m_targetStatusPage;
	SwgCuiStatusGround *  m_secondaryTargetStatusPage;
	SwgCuiStatusGround *  m_playerStatusPage;
	SwgCuiStatusGround *  m_petStatusPage;
};

//======================================================================

#endif
