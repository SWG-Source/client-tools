 //======================================================================
//
// ShipWeaponGroupManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipWeaponGroupManager.h"

#include "clientDirectInput/ForceFeedbackEffectTemplate.h"
#include "clientDirectInput/ForceFeedbackEffectTemplateList.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientGame/ShipWeaponGroup.h"
#include "clientGame/ShipWeaponGroupsForShip.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMessageDispatch/Transceiver.h"
#include <algorithm>
#include <map>
#include <vector>

//======================================================================

namespace ShipWeaponGroupManagerNamespace
{
	//----------------------------------------------------------------------

	typedef stdmap<NetworkId /* ship id */, ShipWeaponGroupsForShip *>::fwd ShipWeaponGroupsForShipMap;
	ShipWeaponGroupsForShipMap s_shipWeaponGroupsForShipMap;

	//----------------------------------------------------------------------

	NetworkId const getCurrentShipNetworkId()
	{
		ShipObject const * const ship = Game::getPlayerPilotedShip();
		return (ship != NULL) ? ship->getNetworkId() : NetworkId::cms_invalid;
	}

	//----------------------------------------------------------------------

	std::string s_filePath;
	ForceFeedbackEffectTemplate const * ms_fireGunsForceFeedbackEffect = NULL;

	//----------------------------------------------------------------------

	namespace Tags
	{
		Tag const WPGS = TAG(W,P,G,S);
		Tag const WGFS = TAG(W,G,F,S);
	}

	namespace Transceivers
	{
		MessageDispatch::Transceiver<ShipWeaponGroupManager::Messages::WeaponGroupChanged::Payload const &, ShipWeaponGroupManager::Messages::WeaponGroupChanged> weaponGroupChanged;
		MessageDispatch::Transceiver<ShipWeaponGroupManager::Messages::WeaponGroupsResetForShip::Payload const &, ShipWeaponGroupManager::Messages::WeaponGroupsResetForShip> weaponGroupsResetForShip;
	}

	//----------------------------------------------------------------------

	int const cs_noMissile = -1;

	//----------------------------------------------------------------------
	int const cs_resetGroup = -1;

	int s_currentGroupByGroupType[ShipWeaponGroupManager::GT_Types] = { cs_resetGroup, cs_resetGroup };

	void resetSelectedGroups()
	{
		for(int n = 0; n < static_cast<int>(ShipWeaponGroupManager::GT_Types); ++n)
		{
			s_currentGroupByGroupType[n] = cs_resetGroup;
			Transceivers::weaponGroupChanged.emitMessage(std::make_pair(static_cast<ShipWeaponGroupManager::GroupType>(n), s_currentGroupByGroupType[n]));
		}
	}
}

using namespace ShipWeaponGroupManagerNamespace;

//----------------------------------------------------------------------

void ShipWeaponGroupManager::install()
{
	InstallTimer const installTimer("ShipWeaponGroupManager::install");

	ExitChain::add(ShipWeaponGroupManager::remove, "ShipWeaponGroupManager::remove");
	ms_fireGunsForceFeedbackEffect = ForceFeedbackEffectTemplateList::fetch("forcefeedback/guns_fire.ffe");
	resetSelectedGroups();
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::remove()
{
	clear();
	if(ms_fireGunsForceFeedbackEffect)
		ForceFeedbackEffectTemplateList::release(ms_fireGunsForceFeedbackEffect);
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::clear()
{
	std::for_each(s_shipWeaponGroupsForShipMap.begin(), s_shipWeaponGroupsForShipMap.end(), PointerDeleterPairSecond());
	s_shipWeaponGroupsForShipMap.clear();
	resetSelectedGroups();
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::reset()
{
	ShipWeaponGroupManager::save();
	ShipWeaponGroupManager::clear();
	ShipWeaponGroupManager::load();
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::load()
{
	s_filePath.clear();

	std::string loginId;
	std::string clusterName;
	Unicode::String playerName;
	NetworkId id;
	if (!Game::getPlayerPath(loginId, clusterName, playerName, id))
		return;

	s_filePath = std::string("profiles/") + loginId + "/" + clusterName + "/" + id.getValueString() + ".wpg";

	Iff iff;
	if (!iff.open(s_filePath.c_str(), true))
	{
		resetDefaults();
		return;
	}

	iff.enterForm(Tags::WPGS);
	{
		switch (iff.getCurrentName())
		{
		case TAG_0000:
			ShipWeaponGroupManager::load_0000(iff);
			break;
		case TAG_0001:
			ShipWeaponGroupManager::load_0001(iff);
			break;
		default:
			DEBUG_FATAL(true, ("ShipWeaponGroupManager version [%d] unknown", iff.getCurrentName()));
			break;
		}
	}
	iff.exitForm(Tags::WPGS);

	iff.close();

	updateWeaponGroups();
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::load_0000(Iff & iff)
{
	iff.enterForm(TAG_0000);
	{
		while (iff.enterForm(Tags::WGFS, true))
		{
			iff.enterChunk(TAG_INFO);
			NetworkId const shipId(iff.read_string());
			iff.exitChunk(TAG_INFO);

			ShipWeaponGroupsForShip * const shipWeaponGroupsForShip = new ShipWeaponGroupsForShip(shipId);
			shipWeaponGroupsForShip->load_0000(iff);

			IGNORE_RETURN(s_shipWeaponGroupsForShipMap.insert(std::make_pair(shipId, shipWeaponGroupsForShip)));

			iff.exitForm(Tags::WGFS);
		}
	}
	iff.exitForm(TAG_0000);
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::load_0001(Iff & iff)
{
	iff.enterForm(TAG_0001);
	{
		while (iff.enterForm(Tags::WGFS, true))
		{
			iff.enterChunk(TAG_INFO);
			NetworkId const shipId(iff.read_string());
			iff.exitChunk(TAG_INFO);

			ShipWeaponGroupsForShip * const shipWeaponGroupsForShip = new ShipWeaponGroupsForShip(shipId);
			shipWeaponGroupsForShip->load_0001(iff);

			IGNORE_RETURN(s_shipWeaponGroupsForShipMap.insert(std::make_pair(shipId, shipWeaponGroupsForShip)));

			iff.exitForm(Tags::WGFS);
		}
	}
	iff.exitForm(TAG_0001);
}
//----------------------------------------------------------------------

void ShipWeaponGroupManager::save()
{
	if (s_filePath.empty())
		return;

	Iff iff (8196, true);

	iff.insertForm(Tags::WPGS);
	{
		iff.insertForm(TAG_0001);
		{
			for (ShipWeaponGroupsForShipMap::iterator it = s_shipWeaponGroupsForShipMap.begin(); it != s_shipWeaponGroupsForShipMap.end(); ++it)
			{
				ShipWeaponGroupsForShip * const shipWeaponGroupsForShip = NON_NULL((*it).second);

				if (shipWeaponGroupsForShip->isUserModified())
				{
					iff.insertForm(Tags::WGFS);
					{
						iff.insertChunk(TAG_INFO);
						iff.insertChunkString(shipWeaponGroupsForShip->getShipId().getValueString().c_str());
						iff.exitChunk(TAG_INFO);

						shipWeaponGroupsForShip->save(iff);
					}
					iff.exitForm(Tags::WGFS);
				}
			}

		}
		iff.exitForm(TAG_0001);
	}
	iff.exitForm(Tags::WPGS);

	const size_t lastSlash = s_filePath.rfind ('/');
	IGNORE_RETURN (Os::createDirectories (s_filePath.substr (0, lastSlash).c_str ()));

	if (!iff.write(s_filePath.c_str(), true))
	{
		WARNING(true, ("ShipWeaponGroupManager failed to write file [%s]", s_filePath.c_str()));
	}
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::resetDefaults()
{
	ShipWeaponGroupManager::clear();
	ShipWeaponGroupManager::save();
	updateWeaponGroups();
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::update(float)
{
	bool hasModifications = false;

	{
		for (ShipWeaponGroupsForShipMap::iterator it = s_shipWeaponGroupsForShipMap.begin(); it != s_shipWeaponGroupsForShipMap.end(); ++it)
		{
			ShipWeaponGroupsForShip * const shipWeaponGroupsForShip = NON_NULL((*it).second);
			hasModifications = shipWeaponGroupsForShip->checkForModifications() || hasModifications;
		}
	}

	if (hasModifications)
	{
		ShipWeaponGroupManager::save();
		updateWeaponGroups();
	}
}

//----------------------------------------------------------------------

ShipWeaponGroupsForShip & ShipWeaponGroupManager::getShipWeaponGroupsForShip(NetworkId const & shipId)
{
	FATAL(!shipId.isValid(), ("ShipWeaponGroupManager::getShipWeaponGroupsForShip invalid shipId"));

	ShipWeaponGroupsForShipMap::iterator const it = s_shipWeaponGroupsForShipMap.find(shipId);
	if (it != s_shipWeaponGroupsForShipMap.end())
		return *NON_NULL((*it).second);

	ShipWeaponGroupsForShip * const shipWeaponGroupsForShip = new ShipWeaponGroupsForShip(shipId);
	shipWeaponGroupsForShip->resetDefaults();
	IGNORE_RETURN(s_shipWeaponGroupsForShipMap.insert(std::make_pair(shipId, shipWeaponGroupsForShip)));

	return *shipWeaponGroupsForShip;
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::getAllShipWeaponGroupsForShip(ShipWeaponGroupsForShipVector & result)
{
	result.reserve(s_shipWeaponGroupsForShipMap.size() + result.size());
	for (ShipWeaponGroupsForShipMap::const_iterator it = s_shipWeaponGroupsForShipMap.begin(); it != s_shipWeaponGroupsForShipMap.end(); ++it)
	{
		ShipWeaponGroupsForShip const * const shipWeaponGroupsForShip = NON_NULL((*it).second);
		result.push_back(shipWeaponGroupsForShip);
	}
}

//----------------------------------------------------------------------

bool ShipWeaponGroupManager::fireWeaponGroupForCurrentShip(int groupId)
{
	CreatureObject * const player = Game::getPlayerCreature();

	if (player == NULL)
		return false;

	ShipObject * ship = Game::getPlayerPilotedShip();

	//-- player is pilot
	if (ship != NULL)
	{
		ShipWeaponGroupsForShip & shipWeaponGroupsForShip = ShipWeaponGroupManager::getShipWeaponGroupsForShip(ship->getNetworkId());

		if (groupId < 0 || groupId >= shipWeaponGroupsForShip.getGroupCount())
		{
			return false;
		}

		ShipWeaponGroup & shipWeaponGroup = shipWeaponGroupsForShip.getGroup(groupId);

		int weaponsFired = 0;
		{
			int const weaponCount = shipWeaponGroup.getWeaponCount();

			for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices && weaponsFired < weaponCount; ++weaponIndex)
			{
				if (shipWeaponGroup.hasWeapon(weaponIndex) && !ship->isTurret(weaponIndex))
				{
					if (ship->fireWeapon(weaponIndex))
						++weaponsFired;
				}
			}
		}
		return weaponsFired > 0;
	}
	else
	{
		ship = Game::getPlayerContainingShip();

		if (ship != NULL)
		{
			int const weaponIndex = ShipStation::getWeaponIndexForGunnerStation(player->getShipStation());
			if (weaponIndex >= 0)
			{
				return ship->fireWeapon(weaponIndex);
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::resetDefaultsForShip(NetworkId const & shipId, bool override)
{
	ShipWeaponGroupsForShip & shipWeaponGroupsForShip = ShipWeaponGroupManager::getShipWeaponGroupsForShip(shipId);
	if (override || !shipWeaponGroupsForShip.isUserModified())
	{
		shipWeaponGroupsForShip.resetDefaults();

		ShipObject const * const playerShip = Game::getPlayerPilotedShip();
		if (NULL != playerShip && playerShip->getNetworkId() == shipId)
			updateWeaponGroups();

		Transceivers::weaponGroupsResetForShip.emitMessage(shipId);
	}
}

//----------------------------------------------------------------------

bool ShipWeaponGroupManager::fireCurrentWeaponGroupForShip(GroupType const groupType)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<GroupType>(0), groupType, GT_Types);
	return fireWeaponGroupForCurrentShip(s_currentGroupByGroupType[groupType]);
}

//----------------------------------------------------------------------

bool ShipWeaponGroupManager::fireCountermeasure()
{
	CreatureObject * const player = Game::getPlayerCreature();
	if (NULL == player)
		return false;
		
	ShipObject * const ship = Game::getPlayerPilotedShip();
	if (NULL == ship)
		return false;
	
	bool retval = false;

	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		int chassisSlotIndex = static_cast<int>(ShipChassisSlotType::SCST_weapon_first) + weaponIndex;

		if (ship->isSlotInstalled(chassisSlotIndex) && ship->isCountermeasure(weaponIndex))
		{
			retval = ship->fireWeapon(weaponIndex) || retval;
		}
	}
	
	return retval;
}

//----------------------------------------------------------------------

int ShipWeaponGroupManager::getCurrentGroup(GroupType const groupType)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<GroupType>(0), groupType, GT_Types);
	return s_currentGroupByGroupType[groupType];
}

//----------------------------------------------------------------------

int ShipWeaponGroupManager::getMissileTypeIdFromCurrentGroup()
{
	int missileType = cs_noMissile;
	int const currentGroup = getCurrentGroup(GT_Missile);
	if ( currentGroup > 0)
	{
		missileType = getMissileTypeIdFromGroupId(currentGroup);
	}
	return missileType;
}

//----------------------------------------------------------------------

int ShipWeaponGroupManager::getMissileTypeIdFromGroupId(int groupId)
{
	int missileTypeId = cs_noMissile;

	CreatureObject * const player = Game::getPlayerCreature();
	if (player)
	{
		ShipObject * ship = Game::getPlayerPilotedShip();

		//-- player is pilot
		if (ship != NULL)
		{
			ShipWeaponGroupsForShip & weaponGroup = ShipWeaponGroupManager::getShipWeaponGroupsForShip(ship->getNetworkId());

			if (groupId < 0 || groupId >= weaponGroup.getGroupCount())
			{
				WARNING(true, ("ShipWeaponGroupManager::getMissileTypeIdFromGroupId Invalid group id [%d]", groupId));
			}
			else
			{
				ShipWeaponGroup const & shipWeaponGroup = weaponGroup.getGroup(groupId);

				int const weaponCount = shipWeaponGroup.getWeaponCount();
				int weaponsFired = 0;

				for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices && weaponsFired < weaponCount; ++weaponIndex)
				{
					if (shipWeaponGroup.hasWeapon(weaponIndex) && ship->isMissile(weaponIndex))
					{
						missileTypeId = ship->getWeaponAmmoType(ShipChassisSlotType::SCST_weapon_first + weaponIndex);
						break;
					}
				}
			}
		}
		else
		{
			ship = Game::getPlayerContainingShip();

			if (ship != NULL)
			{
				int const weaponIndex = ShipStation::getWeaponIndexForGunnerStation(player->getShipStation());
				if (weaponIndex >= 0 && ship->isMissile(weaponIndex))
				{
					missileTypeId = ship->getWeaponAmmoType(ShipChassisSlotType::SCST_weapon_first + weaponIndex);
				}
			}
		}
	}

	return missileTypeId;
}

//----------------------------------------------------------------------

bool ShipWeaponGroupManager::findCurrentGroup(GroupType const groupType, bool const isPrevious)
{
	bool retval = false;

	CreatureObject * const player = Game::getPlayerCreature();
	if (player)
	{
		ShipObject * ship = Game::getPlayerPilotedShip();

		//-- Player is pilot
		if (ship != NULL)
		{
			ShipWeaponGroupsForShip const & weaponGroup = ShipWeaponGroupManager::getShipWeaponGroupsForShip(ship->getNetworkId());
			int const groupCounterMax = weaponGroup.getGroupCount();
			if (groupCounterMax)
			{
				// Get the next or previous  group.
				for (GroupType gt = GT_Projectile; gt < GT_Types; gt = static_cast<GroupType>(static_cast<int>(gt) + 1))
				{
					if (groupType != GT_Types && groupType != gt)
						continue;

					int const oldGroup = s_currentGroupByGroupType[gt];
					int selectedGroup = oldGroup;
					bool validGroupFound = false;
					int groupIncrement = isPrevious ? -1 : 1;

					for (int groupCounter = 0; groupCounter < groupCounterMax && !validGroupFound; ++groupCounter)
					{
						selectedGroup += groupIncrement;

						if (selectedGroup >= groupCounterMax)
							selectedGroup = 0;
						else if(selectedGroup < 0)
							selectedGroup = groupCounterMax - 1;

						ShipWeaponGroup const & shipWeaponGroup = weaponGroup.getGroup(selectedGroup);

						for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
						{
							if (shipWeaponGroup.hasWeapon(weaponIndex))
							{
								bool const isMissile = ship->isMissile(weaponIndex);
								if ((isMissile && (gt == GT_Missile)) || (!isMissile && (gt == GT_Projectile)))
								{
									bool const isCountermeasure = ship->isCountermeasure(weaponIndex);

									//-- don't include countermeasure groups in this cycling
									{
										if (!isCountermeasure)
											validGroupFound = true;
										break;
									}
								}
							}
						}

						s_currentGroupByGroupType[gt] = validGroupFound ? selectedGroup : cs_resetGroup;

						if(oldGroup != s_currentGroupByGroupType[gt])
						{
							ShipWeaponGroupManager::stopFiringWeaponGroupForCurrentShip(oldGroup);
							Transceivers::weaponGroupChanged.emitMessage(std::make_pair(static_cast<GroupType>(gt), s_currentGroupByGroupType[gt]));
							retval = true;
						}
					}
				}
			}
		}
	}

	return retval;
}

//----------------------------------------------------------------------

int ShipWeaponGroupManager::getWeaponIndexFromCurrentGroup(GroupType gt)
{
	int currentWeaponIndex = ShipChassisSlotType::cms_numWeaponIndices;
	
	CreatureObject * const player = Game::getPlayerCreature();
	if (player && gt < GT_Types)
	{
		ShipObject * ship = Game::getPlayerPilotedShip();
		
		//-- Player is pilot
		if (ship != NULL)
		{
			ShipWeaponGroupsForShip const & weaponGroup = ShipWeaponGroupManager::getShipWeaponGroupsForShip(ship->getNetworkId());
			int const groupCounterMax = weaponGroup.getGroupCount();
			if (groupCounterMax)
			{
				int const currentGroup = s_currentGroupByGroupType[gt];
				
				if (currentGroup >= 0)
				{
					ShipWeaponGroup const & shipWeaponGroup = weaponGroup.getGroup(currentGroup);
					
					//-- Scan the weapons in the group for the current weapon index.
					for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
					{
						if (shipWeaponGroup.hasWeapon(weaponIndex))
						{
							bool const isMissile = ship->isMissile(weaponIndex);
							if ((isMissile && (gt == GT_Missile)) || (!isMissile && (gt == GT_Projectile)))
							{
								currentWeaponIndex = weaponIndex;
								break;
							}
						}
					}
				}
			}
		}
	}
	
	return currentWeaponIndex;
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::updateWeaponGroups()
{
	findCurrentGroup(GT_Types, false);
}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::stopFiringWeaponGroupForCurrentShip(int groupId)
{
	CreatureObject * const player = Game::getPlayerCreature();

	if (player == NULL)
		return;

	ShipObject * ship = Game::getPlayerPilotedShip();

	//-- player is pilot
	if (ship != NULL)
	{
		ShipWeaponGroupsForShip & shipWeaponGroupsForShip = ShipWeaponGroupManager::getShipWeaponGroupsForShip(ship->getNetworkId());

		if (groupId < 0 || groupId >= shipWeaponGroupsForShip.getGroupCount())
		{
			return;
		}

		ShipWeaponGroup & shipWeaponGroup = shipWeaponGroupsForShip.getGroup(groupId);

		{
			int const weaponCount = shipWeaponGroup.getWeaponCount();
			int weaponsStopped = 0;

			for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices && weaponsStopped < weaponCount; ++weaponIndex)
			{
				if (shipWeaponGroup.hasWeapon(weaponIndex) && !ship->isTurret(weaponIndex))
				{
					ship->stopFiringWeapon(weaponIndex, true);
					++weaponsStopped;
				}
			}
		}
	}
	else
	{
		ship = Game::getPlayerContainingShip();

		if (ship != NULL)
		{
			int const weaponIndex = ShipStation::getWeaponIndexForGunnerStation(player->getShipStation());
			if (weaponIndex >= 0)
			{
				ship->stopFiringWeapon(weaponIndex, true);
			}
		}
	}

}

//----------------------------------------------------------------------

void ShipWeaponGroupManager::stopFiringCurrentWeaponGroupForShip(GroupType groupType)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<GroupType>(0), groupType, GT_Types);
	ShipWeaponGroupManager::stopFiringWeaponGroupForCurrentShip(s_currentGroupByGroupType[groupType]);
}

//======================================================================
