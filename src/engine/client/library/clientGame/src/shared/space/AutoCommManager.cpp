// ======================================================================
//
// AutoCommManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AutoCommManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "sharedCollision/Extent.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Appearance.h"
#include "swgSharedUtility/States.def"
#include <vector>

// ======================================================================

namespace AutoCommManagerNamespace
{
	bool s_inTrigger = false;
	Timer s_checkTimer(1.0f);
	float const cs_triggerRadius = 100.0f;
	uint32 const s_commCommand = Crc::calculate("comm");

	//----------------------------------------------------------------------

	/*
	 * Class to track a sphere that will trigger the comm when the player
	 * flies into it.
	 */
	class CommTrigger
	{
	public:
		CommTrigger(ClientObject const & object, Vector const & location_w, float radius = cs_triggerRadius);
		CommTrigger();
		bool isWithin(Vector const & location) const;
		bool isNear(Vector const & location) const;
		NetworkId const & getNetworkId() const;

	private:
		Vector m_location_w;
		float m_radiusSquared;
		NetworkId m_networkId;
	};

	//----------------------------------------------------------------------
	
	// STL sure requires a lot of text for something simple:
	class IsTriggerForObject : public std::unary_function<CommTrigger, bool>
	{
	public:
		explicit IsTriggerForObject(const NetworkId & networkId) :			
			m_networkId(networkId)
		{ //lint !e1928 // base class has no constructor
		}

		bool operator()(CommTrigger const & trigger) const
		{
			return trigger.getNetworkId() == m_networkId;
		}
	private:
		NetworkId m_networkId;
	}; //lint !e1509 !e1712 // no virtual destructor, default constructor

	//----------------------------------------------------------------------

	typedef std::vector<CommTrigger> CommTriggersType;
	typedef std::set<NetworkId> TrackedObjectsType;

	CommTriggersType s_commTriggers;
	CommTrigger s_currentTrigger;
	TrackedObjectsType s_trackedObjects;

	//----------------------------------------------------------------------
}

using namespace AutoCommManagerNamespace;

// ======================================================================

/**
 * Create comm triggers for a new ship, if appropriate
 */
void AutoCommManager::registerShip(ShipObject const & ship)
{
	static bool installed = false;
	static int stationType = 0;

	if (!installed)
	{
		if (!GameObjectTypes::getTypeByName("ship_station",stationType))
			DEBUG_FATAL(true,("Programmer bug:  ship_station was removed as a valid GOT type.  AutoCommManager needs to be updated"));
		installed = true;
	}
	
	if (GameObjectTypes::isTypeOf(ship.getGameObjectType(),stationType))
	{			
		Appearance const * const appearance = ship.getAppearance();
		Transform hardpointTransform;
		int hardpointNumber = 1;
		char hardpointName[20];
		std::vector<Transform> hardpoints;

		sprintf(hardpointName,"autocomm%02i",hardpointNumber);
		while (appearance->findHardpoint(ConstCharCrcString(hardpointName), hardpointTransform))
		{
			hardpoints.push_back(hardpointTransform);
			sprintf(hardpointName,"autocomm%02i",++hardpointNumber);
		}

		if (hardpoints.size() != 0)
		{
			// special behavior if only one hardpoint is found 
			if (hardpoints.size() == 1)
			{
				// use the appearance extent radius if it is greater than the default
				float extentRadius = (appearance->getExtent()) ? (appearance->getExtent())->getRadius() : 0;
				if (extentRadius > cs_triggerRadius)
					s_commTriggers.push_back(CommTrigger(ship, ship.rotateTranslate_o2w(hardpoints[0].getPosition_p()), extentRadius));
				else
					s_commTriggers.push_back(CommTrigger(ship, ship.rotateTranslate_o2w(hardpoints[0].getPosition_p())));
			}
			else
				for (std::vector<Transform>::iterator i = hardpoints.begin(); i != hardpoints.end(); ++i)
					s_commTriggers.push_back(CommTrigger(ship, ship.rotateTranslate_o2w(i->getPosition_p())));

			IGNORE_RETURN(s_trackedObjects.insert(ship.getNetworkId()));
		}
	}	
}

//----------------------------------------------------------------------

void AutoCommManager::removeShip(ShipObject const & ship)
{	
	if (s_trackedObjects.erase(ship.getNetworkId()) != 0)
	{
		IGNORE_RETURN(s_commTriggers.erase(std::remove_if(s_commTriggers.begin(), s_commTriggers.end(), IsTriggerForObject(ship.getNetworkId())), s_commTriggers.end()));
	}
}

//----------------------------------------------------------------------

/**
 * Check whether the player is in one of the trigger spheres.  Since there
 * aren't very many and we don't check very often, this just does a linear
 * search through all of them.
 */
void AutoCommManager::update(float const elapsedTime)
{
	if (s_checkTimer.updateZero(elapsedTime))
	{
		CreatureObject * const playerCreature = Game::getPlayerCreature();
		if(!playerCreature ||
			!(playerCreature->getState(States::PilotingShip) || playerCreature->getState(States::PilotingPobShip)))
		{
			s_inTrigger = false;
			return;
		}

		Vector const location_w = playerCreature->getPosition_w(); 

		bool const wasInTrigger = s_inTrigger;
		if (s_inTrigger)
		{
			if (!s_currentTrigger.isNear(location_w))
				s_inTrigger = false;
		}
		
		for (CommTriggersType::const_iterator i=s_commTriggers.begin(); i!=s_commTriggers.end(); ++i)
		{
			if (i->isWithin(location_w))
			{
				if (!wasInTrigger)
					IGNORE_RETURN(ClientCommandQueue::enqueueCommand(s_commCommand, i->getNetworkId(), Unicode::String()));
				s_inTrigger = true;
				s_currentTrigger = *i;
				break;
			}
		}		
	}
}

// ======================================================================

CommTrigger::CommTrigger(ClientObject const & object, Vector const & location_w, float radius) :
	m_location_w(location_w),
	m_radiusSquared(sqr(radius)),
	m_networkId(object.getNetworkId())
{
}
	
//----------------------------------------------------------------------

bool CommTrigger::isWithin(Vector const & location) const
{
	return (m_location_w.magnitudeBetweenSquared(location) <= m_radiusSquared);
}

//----------------------------------------------------------------------

bool CommTrigger::isNear(Vector const & location) const
{
	return (m_location_w.magnitudeBetweenSquared(location) <= (m_radiusSquared * 1.5f));
}

//----------------------------------------------------------------------

NetworkId const & CommTrigger::getNetworkId() const
{
	return m_networkId;
}

//----------------------------------------------------------------------

CommTrigger::CommTrigger() :
	m_location_w(),
	m_radiusSquared(0),
	m_networkId(NetworkId::cms_invalid)
{
}

// ======================================================================
