//======================================================================
//
// SwgCuiHudActionSpace.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHudActionSpace.h"

#include "UnicodeUtils.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientShipTargeting.h"
#include "clientGame/CockpitCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/ShipObject.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "swgClientUserInterface/SwgCuiHudWindowManagerSpace.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiSpaceMiningSale.h"

//======================================================================

namespace SwgCuiHudActionSpaceNamespace
{
	int s_toggleDebounceMsBooster = 250;
	int s_toggleDebounceMsTargetLock = 250;
}

using namespace SwgCuiHudActionSpaceNamespace;

//----------------------------------------------------------------------

SwgCuiHudActionSpace::SwgCuiHudActionSpace (SwgCuiHud & mediator) :
SwgCuiHudAction (mediator),
m_toggleDebounceCounterMsBooster(0),
m_toggleDebounceCounterMsTargetLock(0)
{
	CuiActionManager::addAction (CuiActions::autopilotLevelShip,         this, false);
	CuiActionManager::addAction (CuiActions::onPointerToggled,           this, false);
	CuiActionManager::addAction (CuiActions::boosterToggleDown,          this, false);
	CuiActionManager::addAction (CuiActions::boosterToggleUp,            this, false);
#ifdef ENABLE_FORMATIONS
	CuiActionManager::addAction (CuiActions::spaceAssignPlayerFormation, this, false);
#endif
	CuiActionManager::addAction (CuiActions::weaponGroupAssignment,      this, false);
	CuiActionManager::addAction (CuiActions::shipComponentDetail,        this, false);
	CuiActionManager::addAction (CuiActions::hyperspaceMap,              this, false);
	CuiActionManager::addAction (CuiActions::targetLockDown,             this, false);
	CuiActionManager::addAction (CuiActions::targetLockUp,               this, false);
	CuiActionManager::addAction (CuiActions::spaceTargetStationAndComm,  this, false);
	CuiActionManager::addAction (CuiActions::spaceMiningSale,  this, false);
}

//----------------------------------------------------------------------

SwgCuiHudActionSpace::~SwgCuiHudActionSpace ()
{
}

//----------------------------------------------------------------------

SwgCuiHudWindowManagerSpace & SwgCuiHudActionSpace::getWindowManagerSpace () const
{
	return *NON_NULL(safe_cast<SwgCuiHudWindowManagerSpace *>(&getWindowManager()));
}

//----------------------------------------------------------------------

bool  SwgCuiHudActionSpace::performAction (const std::string & id, const Unicode::String & params) const
{
	//----------------------------------------------------------------------

	if (id == CuiActions::autopilotLevelShip)
	{
		ShipObject * const playerShip = Game::getPlayerPilotedShip();
		if (playerShip)
		{
			PlayerShipController * const controller = safe_cast<PlayerShipController*>(playerShip->getController());
			if (controller)
				controller->engageAutopilotFullyLevel();
		}
	}

	//----------------------------------------------------------------------

	else if (id == CuiActions::spaceTargetStationAndComm)
	{
		ShipObject * const playerShip = Game::getPlayerContainingShip();
		if (playerShip)
		{
			Object const * const target = ClientShipTargeting::closestCapShip(playerShip);
			if (target != 0)
			{
				Game::getPlayerCreature()->setLookAtTarget(target->getNetworkId());
				ClientCommandQueue::enqueueCommand("comm",target->getNetworkId(),Unicode::String());
			}
		}
	}

	//----------------------------------------------------------------------

	else if (id == CuiActions::onPointerToggled)
	{
		if (!CuiManager::getPointerToggledOn())
		{
			// warp cursor to the center of the screen so that closing menus doesn't cause a deflection on
			// the virtual joystick			

			UIPoint screenCenter;
			CuiManager::getIoWin().getScreenCenter(screenCenter);
			CuiManager::getIoWin().warpCursor(screenCenter.x, screenCenter.y);
		}
	}

 	//----------------------------------------------------------------------

	else if (id == CuiActions::boosterToggleDown)
	{
		int const timeMs = static_cast<int>(Clock::timeMs());
		if (timeMs > m_toggleDebounceCounterMsBooster)
		{
			ShipObject * const shipObject = Game::getPlayerPilotedShip();
			if (shipObject == NULL)
				CuiSystemMessageManager::sendFakeSystemMessage(SharedStringIds::booster_pilot_only.localize());
			else if (!shipObject->isSlotInstalled(ShipChassisSlotType::SCST_booster))
				CuiSystemMessageManager::sendFakeSystemMessage(SharedStringIds::no_booster.localize());
			else
			{
				bool const boosterActive = shipObject->isBoosterActive();
				
				if (!boosterActive)
				{
					if (Game::getSinglePlayer())
						IGNORE_RETURN(shipObject->clientSetComponentActive(ShipChassisSlotType::SCST_booster, !boosterActive));
					else
					{
						static uint32 const hash_boosterOn = Crc::normalizeAndCalculate("boosterOn");
						IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash_boosterOn, NetworkId::cms_invalid, Unicode::emptyString));
					}
					m_toggleDebounceCounterMsBooster = timeMs + s_toggleDebounceMsBooster;
				}
			}
		}
	}
	
	//----------------------------------------------------------------------
	
	else if (id == CuiActions::boosterToggleUp)
	{	
		int const timeMs = static_cast<int>(Clock::timeMs());
		if (timeMs > m_toggleDebounceCounterMsBooster)
		{
			ShipObject * const shipObject = Game::getPlayerPilotedShip();
			if (shipObject == NULL)
				CuiSystemMessageManager::sendFakeSystemMessage(SharedStringIds::booster_pilot_only.localize());
			else
			{
				bool const boosterActive = shipObject->isBoosterActive();
				
				if (boosterActive)
				{
					m_toggleDebounceCounterMsBooster = timeMs;
				
					if (Game::getSinglePlayer())
						IGNORE_RETURN(shipObject->clientSetComponentActive(ShipChassisSlotType::SCST_booster, false));
					else
					{
						static uint32 const hash_boosterOff = Crc::normalizeAndCalculate("boosterOff");
						IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash_boosterOff, NetworkId::cms_invalid, Unicode::emptyString));
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------

	else if(id == CuiActions::weaponGroupAssignment)
	{
		//this UI is only useful if you are the ship's pilot
		if(Game::getPlayerPilotedShip() != NULL)
			CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_WeaponGroupAssignment);
		else
		{
			CuiSystemMessageManager::sendFakeSystemMessage(SharedStringIds::not_a_pilot.localize());
		}
	}

	//----------------------------------------------------------------------

#ifdef ENABLE_FORMATIONS
	else if (id == CuiActions::spaceAssignPlayerFormation)
	{	
		CreatureObject const * const playerCreatureObject = Game::getPlayerCreature();
		if (playerCreatureObject)
		{
			Object const * const groupAsObject = playerCreatureObject->getGroup().getObject();
			GroupObject const * const group = dynamic_cast<GroupObject const *>(groupAsObject);
			if (group && group->getLeader() == playerCreatureObject->getNetworkId())
				getWindowManagerSpace().spawnSpaceAssignPlayerFormation();
			else
				CuiSystemMessageManager::sendFakeSystemMessage(SharedStringIds::not_group_leader_for_formation.localize());
		}
	}
#endif

	//----------------------------------------------------------------------

	else if (id == CuiActions::shipComponentDetail)
	{
		std::string const & shipIdStr = Unicode::wideToNarrow(params);
		NetworkId const shipId(shipIdStr);
		getWindowManagerSpace().spawnShipComponentDetail(shipId);
	}

	//----------------------------------------------------------------------

	else if(id == CuiActions::hyperspaceMap)
	{
		if (Game::getPlayerPilotedShip())
			getWindowManagerSpace().spawnHyperspaceMap();
	}
	
	//----------------------------------------------------------------------
	
	else if (id == CuiActions::targetLockDown)
	{
		int const timeMs = static_cast<int>(Clock::timeMs());
		if (timeMs > m_toggleDebounceCounterMsTargetLock)
		{
			CockpitCamera * const cockpitCamera = dynamic_cast<CockpitCamera *>(Game::getCamera());
			
			if (NULL != cockpitCamera)
			{
				if (cockpitCamera->getCameraLockTarget())
					cockpitCamera->setCameraLockTarget(false);
				else
				{
					cockpitCamera->setCameraLockTarget(true);
					m_toggleDebounceCounterMsTargetLock = timeMs + s_toggleDebounceMsTargetLock;
				}			
			}
		}
	}
	
	//----------------------------------------------------------------------
	
	else if (id == CuiActions::targetLockUp)
	{	
		int const timeMs = static_cast<int>(Clock::timeMs());
		if (timeMs > m_toggleDebounceCounterMsTargetLock)
		{
			CockpitCamera * const cockpitCamera = dynamic_cast<CockpitCamera *>(Game::getCamera());
			
			if (NULL != cockpitCamera)
			{
				m_toggleDebounceCounterMsTargetLock = timeMs;
				cockpitCamera->setCameraLockTarget(false);
			}
		}
	}


	//----------------------------------------------------------------------

	else if (id == CuiActions::spaceMiningSale)
	{
		size_t const spacepos = params.find(' ');

		if (spacepos == Unicode::String::npos)
		{
			WARNING(true, ("SwgCuiHudActionSpace malformed spaceMiningSale"));
			return false;
		}

		NetworkId const spaceStationId(Unicode::wideToNarrow(params.substr(0, spacepos)));
		std::string const & spaceStationName = Unicode::wideToNarrow(params.substr(spacepos + 1));
	
		SwgCuiSpaceMiningSale * const mediator = safe_cast<SwgCuiSpaceMiningSale *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_SpaceMiningSale));
		if (NULL != mediator)
		{
			mediator->setStation(spaceStationId, spaceStationName);
		}
	}

	//----------------------------------------------------------------------

	return SwgCuiHudAction::performAction (id, params);
}

//======================================================================
