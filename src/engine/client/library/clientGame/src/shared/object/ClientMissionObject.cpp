// ClientMissionObject.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientMissionObject.h"

#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/SharedMissionObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiStringIdsMission.h"

//-----------------------------------------------------------------------

namespace ClientMissionObjectNamespace
{
	const unsigned int destroy     = CrcLowerString::calculateCrc("destroy");
	const unsigned int recon       = CrcLowerString::calculateCrc("recon");
	const unsigned int deliver     = CrcLowerString::calculateCrc("deliver");
	const unsigned int escorttocreator = CrcLowerString::calculateCrc("escorttocreator");
	const unsigned int escort      = CrcLowerString::calculateCrc("escort");
	const unsigned int bounty      = CrcLowerString::calculateCrc("bounty");
	const unsigned int survey      = CrcLowerString::calculateCrc("survey");
	const unsigned int crafting    = CrcLowerString::calculateCrc("crafting");
	const unsigned int musician    = CrcLowerString::calculateCrc("musician");
	const unsigned int dancer      = CrcLowerString::calculateCrc("dancer");
	const unsigned int entertainer = CrcLowerString::calculateCrc("entertainer");
	const unsigned int hunting     = CrcLowerString::calculateCrc("hunting");
	const unsigned int space_assassination = CrcLowerString::calculateCrc("space_assassination");
	const unsigned int space_delivery = CrcLowerString::calculateCrc("space_delivery");
	const unsigned int space_delivery_duty = CrcLowerString::calculateCrc("space_delivery_duty");
	const unsigned int space_destroy = CrcLowerString::calculateCrc("space_destroy");
	const unsigned int space_destroy_duty = CrcLowerString::calculateCrc("space_destroy_duty");
	const unsigned int space_surprise_attack = CrcLowerString::calculateCrc("space_surprise_attack");
	const unsigned int space_escort = CrcLowerString::calculateCrc("space_escort");
	const unsigned int space_escort_duty = CrcLowerString::calculateCrc("space_escort_duty");
	const unsigned int space_inspection = CrcLowerString::calculateCrc("space_inspection");
	const unsigned int space_patrol = CrcLowerString::calculateCrc("space_patrol");
	const unsigned int space_recovery = CrcLowerString::calculateCrc("space_recovery");
	const unsigned int space_recovery_duty = CrcLowerString::calculateCrc("space_recovery_duty");
	const unsigned int space_rescue = CrcLowerString::calculateCrc("space_rescue");
	const unsigned int space_rescue_duty = CrcLowerString::calculateCrc("space_rescue_duty");
	const unsigned int space_battle = CrcLowerString::calculateCrc("space_battle");
	const unsigned int space_survival = CrcLowerString::calculateCrc("space_survival");
	const unsigned int space_mining_destroy = CrcLowerString::calculateCrc("space_mining_destroy");
	
	const std::string destroyFilename = "appearance/ui_destroy.spr";
	const std::string deliverFilename = "appearance/ui_deliver.spr";
	const std::string bountyFilename  = "appearance/ui_bounty.spr";
	const std::string surveyFilename  = "appearance/ui_survey.spr";
	const std::string entertainerFilename  = "appearance/ui_perform.spr";
	const std::string craftingFilename     = "appearance/ui_craft.spr";
	const std::string reconFilename        = "appearance/ui_recon.spr";
	const std::string escortFilename       = "appearance/ui_escort.spr";

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const ClientMissionObject::Messages::EnteredMissionBag::Payload &, ClientMissionObject::Messages::EnteredMissionBag> enteredMissionBag;
		MessageDispatch::Transceiver<const ClientMissionObject::Messages::LeftMissionBag::Payload &,    ClientMissionObject::Messages::LeftMissionBag>    leftMissionBag;
	}

	std::string const & ms_debugInfoSectionName = "ClientMissionObject";
}

using namespace ClientMissionObjectNamespace;

//-----------------------------------------------------------------------

std::map<Watcher<ClientMissionObject>, NetworkId> ClientMissionObject::ms_deferredMissionObjectMap;

//-----------------------------------------------------------------------

ClientMissionObject::ClientMissionObject(const SharedMissionObjectTemplate * tmpl)
: IntangibleObject(tmpl),
m_description(),
m_difficulty(),
m_endLocation(),
m_missionCreator(),
m_missionType(),
m_reward(),
m_startLocation(),
m_targetAppearanceCrc(),
m_title(),
m_status(),
m_targetName(),
m_waypoint(),
m_localWaypoint()
{
	m_status.setSourceObject(this);
	m_missionType.setSourceObject  (this);
	m_waypoint.setSourceObject (this);

	addSharedVariable(m_difficulty);
	addSharedVariable(m_endLocation);
	addSharedVariable(m_missionCreator);
	addSharedVariable(m_reward);
	addSharedVariable(m_startLocation);
	addSharedVariable(m_targetAppearanceCrc);
	addSharedVariable(m_description);
	addSharedVariable(m_title);
	addSharedVariable(m_status);
	addSharedVariable(m_missionType);
	addSharedVariable(m_targetName);
	addSharedVariable(m_waypoint);
}

//-----------------------------------------------------------------------

ClientMissionObject::~ClientMissionObject()
{
	if(m_localWaypoint.getPointer())
	{
		IGNORE_RETURN(AlterScheduler::removeObject(*m_localWaypoint.getPointer()));
		delete m_localWaypoint.getPointer();
	}
}

//-----------------------------------------------------------------------

void ClientMissionObject::updateName ()
{
	if(getTitle().isValid())
		setObjectName(getTitle().localize());
}

//-----------------------------------------------------------------------

void ClientMissionObject::Callbacks::StatusChange::modified (ClientMissionObject & target, const int & , const int & , bool) const
{
	ClientMissionObjectNamespace::Transceivers::enteredMissionBag.emitMessage(target);

	target.updateName();
}

//-----------------------------------------------------------------------

void ClientMissionObject::Callbacks::WaypointChange::modified(ClientMissionObject & target, const Waypoint & oldValue, const Waypoint & newValue, bool ) const
{
	ClientWaypointObject * wp = dynamic_cast<ClientWaypointObject*>(NetworkIdManager::getObjectById(oldValue.getNetworkId()));
	if(wp)
	{
		*wp = newValue;
	}
	else
	{
		SlottedContainer * const equipmentContainer = safe_cast<SlottedContainer*>(Game::getPlayer()->getProperty(SlottedContainer::getClassPropertyId()));
		const SlotId inventorySlot = SlotIdManager::findSlotId (ConstCharCrcLowerString ("mission_bag"));
		DEBUG_FATAL (inventorySlot == SlotId::invalid, ("invalid inventory slot\n"));

		Container::ContainerErrorCode tmp = Container::CEC_Success;
		const CachedNetworkId dataStorageId (equipmentContainer->getObjectInSlot(inventorySlot, tmp));
		const Object * const missionBagObject = NetworkIdManager::getObjectById (dataStorageId);
		const ContainedByProperty * const containedByProperty = target.getContainedByProperty ();
		if (containedByProperty)
		{
			const Object * const containedBy = containedByProperty->getContainedBy ();
			if(missionBagObject && missionBagObject != containedBy)
			{
				//create the client-only waypoint object, put it in the mission's container
				wp = ClientWaypointObject::createClientWaypoint(newValue);
				if(wp)
				{
					ClientWaypointObject * current = target.getLocalWaypoint();
					bool successfullyAddedToMissionObject = false;
					if(current)
						delete current;
					current = wp;

					VolumeContainer* const vc = ContainerInterface::getVolumeContainer(target);
					if(vc)
					{
						Container::ContainerErrorCode temp = Container::CEC_Success;
						bool res = vc->add(*wp, temp);
						successfullyAddedToMissionObject = res;
					}

					if(!successfullyAddedToMissionObject)
					{	
						delete current;
						current  = 0;
					}
					target.setLocalWaypoint(current);
				}
			}
		}
	}

	if (wp)
	{
		wp->setObjectName (target.getLocalizedName ());
	}

	target.updateName();
}

//-----------------------------------------------------------------------

void ClientMissionObject::Callbacks::TypeChange::modified (ClientMissionObject & target, const unsigned int & old, const unsigned int & value, bool) const
{
	if(old == value)
		return;

	std::string appearance = ClientMissionObjectNamespace::destroyFilename;

	if(value == ClientMissionObjectNamespace::destroy)
	{
		appearance = ClientMissionObjectNamespace::destroyFilename;
	}
	else if(value == ClientMissionObjectNamespace::deliver)
	{
		appearance = ClientMissionObjectNamespace::deliverFilename;
	}
	else if(value == ClientMissionObjectNamespace::bounty)
	{
		appearance = ClientMissionObjectNamespace::bountyFilename;
	}
	else if(value == ClientMissionObjectNamespace::survey)
	{
		appearance = ClientMissionObjectNamespace::surveyFilename;
	}
	else if(value == ClientMissionObjectNamespace::entertainer || value == ClientMissionObjectNamespace::musician || value == ClientMissionObjectNamespace::dancer)
	{
		appearance = ClientMissionObjectNamespace::entertainerFilename;
	}
	else if(value == ClientMissionObjectNamespace::crafting)
	{
		appearance = ClientMissionObjectNamespace::craftingFilename;
	}
	else if(value == ClientMissionObjectNamespace::recon)
	{
		appearance = ClientMissionObjectNamespace::reconFilename;
	}
	else if(value == ClientMissionObjectNamespace::escort || value == ClientMissionObjectNamespace::escorttocreator)
	{
		appearance = ClientMissionObjectNamespace::escortFilename;
	}
	else if(value == ClientMissionObjectNamespace::hunting)
	{
		//TODO this needs its own icon
		appearance = ClientMissionObjectNamespace::destroyFilename;
	}
	else if(value == ClientMissionObjectNamespace::space_assassination)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_delivery)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_delivery_duty)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_destroy)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_destroy_duty)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_surprise_attack)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_escort)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_escort_duty)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_inspection)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_patrol)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_recovery)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_recovery_duty)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_rescue)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_rescue_duty)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_battle)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_survival)
	{
		appearance = ClientMissionObjectNamespace::escortFilename; //TODO: get real art
	}
	else if(value == ClientMissionObjectNamespace::space_mining_destroy)
	{
		appearance = ClientMissionObjectNamespace::surveyFilename; //TODO: get real art
	}

	else
	{
		DEBUG_WARNING(true, ("No valid mission type found"));
	}
	Appearance* const app = AppearanceTemplateList::createAppearance (appearance.c_str ());
	target.setAppearance(app);

	target.updateName();
}

//-----------------------------------------------------------------------

const int ClientMissionObject::getDifficulty () const
{
	return m_difficulty.get();
}

//-----------------------------------------------------------------------

const StringId & ClientMissionObject::getDescription () const
{
	return m_description.get();
}

//-----------------------------------------------------------------------

const Location & ClientMissionObject::getEndLocation () const
{
	return m_endLocation.get();
}

//-----------------------------------------------------------------------

const Unicode::String & ClientMissionObject::getMissionCreator () const
{
	return m_missionCreator.get();
}

//-----------------------------------------------------------------------

const unsigned int ClientMissionObject::getType () const
{
	return m_missionType.get();
}

//-----------------------------------------------------------------------

const int ClientMissionObject::getReward () const
{
	return m_reward.get();
}

//-----------------------------------------------------------------------

const Location & ClientMissionObject::getStartLocation () const
{
	return m_startLocation.get();
}

//-----------------------------------------------------------------------

const uint32 ClientMissionObject::getTargetAppearanceCrc () const
{
	return m_targetAppearanceCrc.get();
}

//-----------------------------------------------------------------------

const StringId & ClientMissionObject::getTitle () const
{
	return m_title.get();
}

//-----------------------------------------------------------------------

const std::string & ClientMissionObject::getTargetName () const
{
	return m_targetName.get();
}

//-----------------------------------------------------------------------

const int ClientMissionObject::getStatus() const
{
	return m_status.get();
}

//-----------------------------------------------------------------------

ClientWaypointObject* ClientMissionObject::getLocalWaypoint() const
{
	return m_localWaypoint.getPointer();
}

//-----------------------------------------------------------------------

void ClientMissionObject::setLocalWaypoint(ClientWaypointObject* wp)
{
	m_localWaypoint = wp;
}

//-----------------------------------------------------------------------

void ClientMissionObject::movedFromMissionBag()
{
	//create the client-only waypoint object, put it in the mission's container
	if(m_waypoint.get().getColor() != Waypoint::Invisible)
	{
		if(! m_localWaypoint)
		{
			m_localWaypoint = ClientWaypointObject::createClientWaypoint(m_waypoint.get());
		}

		VolumeContainer* const vc = ContainerInterface::getVolumeContainer(*this);
		if(vc)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			IGNORE_RETURN(vc->add(*m_localWaypoint, tmp));
		}
		
		m_localWaypoint->setObjectName (getLocalizedName ());
		
		ClientMissionObjectNamespace::Transceivers::leftMissionBag.emitMessage (*this);
	}

	updateName();
}

//-----------------------------------------------------------------------

void ClientMissionObject::containedByModified(const NetworkId & oldValue, const NetworkId & newValue, bool isLocal)
{
	SlottedContainer *const equipmentContainer = safe_cast<SlottedContainer*>(Game::getPlayer()->getProperty(SlottedContainer::getClassPropertyId()));
	const SlotId inventorySlot = SlotIdManager::findSlotId (ConstCharCrcLowerString ("mission_bag"));
	DEBUG_FATAL (inventorySlot == SlotId::invalid, ("invalid inventory slot\n"));
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	CachedNetworkId dataStorageId (equipmentContainer->getObjectInSlot(inventorySlot, tmp));
	NetworkId missionBag = dataStorageId;
	//if the player hasn't been finished being setup (i.e. loading), defer this processing until 
	if(missionBag == NetworkId::cms_invalid)
	{
		addDeferredMissionObject(this, newValue);
	}
	else
	{
		if(oldValue == missionBag && newValue != missionBag)
		{
			movedFromMissionBag();
		}
	}

	updateName();

	IntangibleObject::containedByModified(oldValue, newValue, isLocal);
}

//----------------------------------------------------------------------

void ClientMissionObject::addDeferredMissionObject(ClientMissionObject* mission, const NetworkId& newValue)
{
	if(mission)
	{
		Watcher<ClientMissionObject> w(mission);
		ms_deferredMissionObjectMap[w] = newValue;
	}
}


//---------------------------------------------------------------------

void ClientMissionObject::applyDeferredMissionObjectProcessing()
{
	SlottedContainer *const equipmentContainer = safe_cast<SlottedContainer*>(Game::getPlayer()->getProperty(SlottedContainer::getClassPropertyId()));
	const SlotId inventorySlot = SlotIdManager::findSlotId (ConstCharCrcLowerString ("mission_bag"));
	DEBUG_FATAL (inventorySlot == SlotId::invalid, ("invalid inventory slot\n"));
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	CachedNetworkId dataStorageId (equipmentContainer->getObjectInSlot(inventorySlot, tmp));
	NetworkId missionBag = dataStorageId;
	//if the player hasn't been finished being setup (i.e. loading), defer this processing until 

	for(std::map<Watcher<ClientMissionObject>, NetworkId>::iterator i = ms_deferredMissionObjectMap.begin(); i != ms_deferredMissionObjectMap.end(); ++i)
	{
		
		ClientMissionObject* const mission  = i->first;
		const NetworkId &          newValue = i->second;
		if(mission)
		{
			if(newValue != missionBag)
			{
				mission->movedFromMissionBag();
			}
		}
		else
		{
			DEBUG_WARNING(true, ("Found a deleted/NULL mission in ClientMissionObject::applyDeferredMissionObjectProcessing"));
		}
		
	}
	ms_deferredMissionObjectMap.clear();
}

//-----------------------------------------------------------------------

bool ClientMissionObject::isSpaceMission() const
{
	return (m_missionType.get() == ClientMissionObjectNamespace::space_assassination
			|| m_missionType.get() == ClientMissionObjectNamespace::space_delivery
			|| m_missionType.get() == ClientMissionObjectNamespace::space_delivery_duty
			|| m_missionType.get() == ClientMissionObjectNamespace::space_destroy
			|| m_missionType.get() == ClientMissionObjectNamespace::space_destroy_duty
			|| m_missionType.get() == ClientMissionObjectNamespace::space_surprise_attack
			|| m_missionType.get() == ClientMissionObjectNamespace::space_escort
			|| m_missionType.get() == ClientMissionObjectNamespace::space_escort_duty
			|| m_missionType.get() == ClientMissionObjectNamespace::space_inspection
			|| m_missionType.get() == ClientMissionObjectNamespace::space_patrol
			|| m_missionType.get() == ClientMissionObjectNamespace::space_recovery
			|| m_missionType.get() == ClientMissionObjectNamespace::space_recovery_duty
			|| m_missionType.get() == ClientMissionObjectNamespace::space_rescue
			|| m_missionType.get() == ClientMissionObjectNamespace::space_rescue_duty
			|| m_missionType.get() == ClientMissionObjectNamespace::space_battle
			|| m_missionType.get() == ClientMissionObjectNamespace::space_survival
			|| m_missionType.get() == ClientMissionObjectNamespace::space_mining_destroy);
}

//-----------------------------------------------------------------------

bool ClientMissionObject::isDuty() const
{
	return (m_missionType.get() == ClientMissionObjectNamespace::space_delivery_duty
			|| m_missionType.get() == ClientMissionObjectNamespace::space_destroy_duty
			|| m_missionType.get() == ClientMissionObjectNamespace::space_escort_duty
			|| m_missionType.get() == ClientMissionObjectNamespace::space_recovery_duty
			|| m_missionType.get() == ClientMissionObjectNamespace::space_rescue_duty);
}

//-----------------------------------------------------------------------

const std::string & ClientMissionObject::getMissionTypeString() const
{
	// @todo this should be a crc string table!
	static const unsigned int destroy = CrcLowerString::calculateCrc("destroy");
	static const unsigned int recon = CrcLowerString::calculateCrc("recon");
	static const unsigned int deliver = CrcLowerString::calculateCrc("deliver");
	static const unsigned int escorttocreator = CrcLowerString::calculateCrc("escorttocreator");
	static const unsigned int escort = CrcLowerString::calculateCrc("escort");
	static const unsigned int bounty = CrcLowerString::calculateCrc("bounty");
	static const unsigned int survey = CrcLowerString::calculateCrc("survey");
	static const unsigned int crafting = CrcLowerString::calculateCrc("crafting");
	static const unsigned int musician = CrcLowerString::calculateCrc("musician");
	static const unsigned int dancer =  CrcLowerString::calculateCrc("dancer");
	static const unsigned int hunting =  CrcLowerString::calculateCrc("hunting");
	static const unsigned int space_assassination = CrcLowerString::calculateCrc("space_assassination");
	static const unsigned int space_delivery = CrcLowerString::calculateCrc("space_delivery");
	static const unsigned int space_delivery_duty = CrcLowerString::calculateCrc("space_delivery_duty");
	static const unsigned int space_destroy = CrcLowerString::calculateCrc("space_destroy");
	static const unsigned int space_destroy_duty = CrcLowerString::calculateCrc("space_destroy_duty");
	static const unsigned int space_surprise_attack = CrcLowerString::calculateCrc("space_surprise_attack");
	static const unsigned int space_escort = CrcLowerString::calculateCrc("space_escort");
	static const unsigned int space_escort_duty = CrcLowerString::calculateCrc("space_escort duty");
	static const unsigned int space_inspection = CrcLowerString::calculateCrc("space_inspection");
	static const unsigned int space_patrol = CrcLowerString::calculateCrc("space_patrol");
	static const unsigned int space_recovery = CrcLowerString::calculateCrc("space_recovery");
	static const unsigned int space_recovery_duty = CrcLowerString::calculateCrc("space_recovery_duty");
	static const unsigned int space_rescue = CrcLowerString::calculateCrc("space_rescue");
	static const unsigned int space_rescue_duty = CrcLowerString::calculateCrc("space_rescue_duty");
	static const unsigned int space_battle = CrcLowerString::calculateCrc("space_battle");
	static const unsigned int space_survival = CrcLowerString::calculateCrc("space_survival");
	static const unsigned int space_mining_destroy = CrcLowerString::calculateCrc("space_mining_destroy");
	
	if(m_missionType.get() == destroy)
	{
		static const std::string r = "destroy";
		return r;
	}
	else if(m_missionType.get() == recon)
	{
		static const std::string r = "recon";
		return r;
	}
	else if(m_missionType.get() == deliver)
	{
		static const std::string r = "deliver";
		return r;
	}
	else if(m_missionType.get() == escorttocreator)
	{
		static const std::string r = "escorttocreator";
		return r;
	}
	else if(m_missionType.get() == escort)
	{
		static const std::string r = "escort";
		return r;
	}
	else if(m_missionType.get() == bounty)
	{
		static const std::string r = "bounty";
		return r;
	}
	else if(m_missionType.get() == survey)
	{
		static const std::string r = "survey";
		return r;
	}
	else if(m_missionType.get() == crafting)
	{
		static const std::string r = "crafting";
		return r;
	}
	else if(m_missionType.get() == musician)
	{
		static const std::string r = "musician";
		return r;
	}
	else if(m_missionType.get() == dancer)
	{
		static const std::string r = "dancer";
		return r;
	}
	else if(m_missionType.get() == hunting)
	{
		static const std::string r = "hunting";
		return r;
	}
	else if(m_missionType.get() == space_assassination)
	{
		static const std::string r = "space_assassination";
		return r;
	}
	else if(m_missionType.get() == space_delivery)
	{
		static const std::string r = "space_delivery";
		return r;
	}
	else if(m_missionType.get() == space_delivery_duty)
	{
		static const std::string r = "space_delivery_duty";
		return r;
	}
	else if(m_missionType.get() == space_destroy)
	{
		static const std::string r = "space_destroy";
		return r;
	}
	else if(m_missionType.get() == space_destroy_duty)
	{
		static const std::string r = "space_destroy_duty";
		return r;
	}
	else if(m_missionType.get() == space_surprise_attack)
	{
		static const std::string r = "space_surprise_attack";
		return r;
	}
	else if(m_missionType.get() == space_escort)
	{
		static const std::string r = "space_escort";
		return r;
	}
	else if(m_missionType.get() == space_escort_duty)
	{
		static const std::string r = "space_escort_duty";
		return r;
	}
	else if(m_missionType.get() == space_inspection)
	{
		static const std::string r = "space_inspection";
		return r;
	}
	else if(m_missionType.get() == space_patrol)
	{
		static const std::string r = "space_patrol";
		return r;
	}
	else if(m_missionType.get() == space_recovery)
	{
		static const std::string r = "space_recovery";
		return r;
	}
	else if(m_missionType.get() == space_recovery_duty)
	{
		static const std::string r = "space_recovery_duty";
		return r;
	}
	else if(m_missionType.get() == space_rescue)
	{
		static const std::string r = "space_rescue";
		return r;
	}
	else if(m_missionType.get() == space_rescue_duty)
	{
		static const std::string r = "space_rescue_duty";
		return r;
	}
	else if(m_missionType.get() == space_battle)
	{
		static const std::string r = "space_battle";
		return r;
	}
	else if(m_missionType.get() == space_survival)
	{
		static const std::string r = "space_survival";
		return r;
	}
	else if(m_missionType.get() == space_mining_destroy)
	{
		static const std::string r = "space_mining_destroy";
		return r;
	}

	static const std::string unknown("unknown");
	return unknown;
	
}

//-----------------------------------------------------------------------

void ClientMissionObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
#if PRODUCTION == 0
/**
  When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
*/

/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
*/

//	static stdmap<Watcher<ClientMissionObject>, NetworkId>::fwd                                   ms_deferredMissionObjectMap;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Description", m_description.get().getCanonicalRepresentation());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Difficulty", m_difficulty.get());
//	Archive::AutoDeltaVariable<Location>                                                          m_endLocation;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "MissionCreator", m_missionCreator.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "MissionType", m_missionType.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Reward", m_reward.get());
//	Archive::AutoDeltaVariable<Location>                                                          m_startLocation;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "TargetAppearanceCrc", m_targetAppearanceCrc.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Title", m_title.get().getCanonicalRepresentation());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Status", m_status.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "TargetName", m_targetName.get());
//	Archive::AutoDeltaVariableCallback<Waypoint, Callbacks::WaypointChange, ClientMissionObject>  m_waypoint;
//	Watcher<ClientWaypointObject>                                                                 m_localWaypoint;

	IntangibleObject::getObjectInfo(propertyMap);

#else
	UNREF(propertyMap);
#endif
}

//-----------------------------------------------------------------------
