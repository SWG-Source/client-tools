//======================================================================
//
// ShipWeaponGroupsForShip.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipWeaponGroupsForShip.h"

#include "UnicodeUtils.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipWeaponGroup.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedGame/ShipTurretManager.h"
#include "sharedObject/NetworkIdManager.h"
#include <algorithm>
#include <vector>

//======================================================================

namespace ShipWeaponGroupsForShipNamespace
{
	namespace Tags
	{
		Tag const SWGI = TAG(S,W,G,I);
	}

	int const NUM_GROUPS = 8;
}

using namespace ShipWeaponGroupsForShipNamespace;

//----------------------------------------------------------------------

ShipWeaponGroupsForShip::ShipWeaponGroupsForShip(NetworkId const & shipId) :
m_shipId(shipId),
m_weaponGroups(new ShipWeaponGroupVector(NUM_GROUPS, static_cast<ShipWeaponGroup *>(NULL))),
m_userModified(false)
{
	{
		int index = 0;
		for (ShipWeaponGroupVector::iterator it = m_weaponGroups->begin(); it != m_weaponGroups->end(); ++it, ++index)
			(*it) = new ShipWeaponGroup(index);
	}
}

//----------------------------------------------------------------------

ShipWeaponGroupsForShip::~ShipWeaponGroupsForShip()
{
	std::for_each(m_weaponGroups->begin(), m_weaponGroups->end(), PointerDeleter());
	delete m_weaponGroups;
	m_weaponGroups = NULL;
}

//----------------------------------------------------------------------

int ShipWeaponGroupsForShip::getGroupCount() const
{
	return static_cast<int>(m_weaponGroups->size());
}

//----------------------------------------------------------------------

ShipWeaponGroup const & ShipWeaponGroupsForShip::getGroup(int groupId) const
{
	return const_cast<ShipWeaponGroupsForShip *>(this)->getGroup(groupId);
}

//----------------------------------------------------------------------

ShipWeaponGroup & ShipWeaponGroupsForShip::getGroup(int groupId)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, groupId, getGroupCount());
	return *NON_NULL((*m_weaponGroups)[static_cast<size_t>(groupId)]);
}

//----------------------------------------------------------------------

void ShipWeaponGroupsForShip::resetDefaults()
{
	{
		for (ShipWeaponGroupVector::iterator it = m_weaponGroups->begin(); it != m_weaponGroups->end(); ++it)
		{
			ShipWeaponGroup * const shipWeaponGroup = NON_NULL(*it);
			shipWeaponGroup->clear();
			shipWeaponGroup->clearUserModified();
		}
	}

	//--
	//-- Each weapon goes into its own group
	//-- all projectiles come first
	//-- then ammo consuming non missiles
	//-- then missiles
	//-- then countermeasures
	//--

	int const groupCount = getGroupCount();

	int group = 0;

	ShipObject const * const ship = dynamic_cast<ShipObject const * >(NetworkIdManager::getObjectById(m_shipId));
	if (ship != NULL)
	{
		for (int groupType = 0; groupType < 4; ++groupType)
		{
			for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices && group < groupCount; ++weaponIndex)
			{
				int const chassisSlot = static_cast<int>(ShipChassisSlotType::SCST_weapon_first) + weaponIndex;
				uint32 const componentCrc = ship->getComponentCrc(chassisSlot);
				
				if (componentCrc == 0 || ShipTurretManager::isTurret(ship->getChassisType(), weaponIndex))
					continue;
								
				bool const isAmmoConsuming = ShipComponentWeaponManager::isAmmoConsuming(componentCrc);
				bool const isMissile = ShipComponentWeaponManager::isMissile(componentCrc);
				bool const isCountermeasure = ShipComponentWeaponManager::isCountermeasure(componentCrc);
				
				switch (groupType)
				{
					//-- projectile
				case 0:
					if (isAmmoConsuming || isMissile || isCountermeasure)
						continue;
					break;

					//-- ammo consuming non missile
				case 1:
					if (!isAmmoConsuming || isMissile || isCountermeasure)
						continue;
					break;
					//-- missile
				case 2:
					if (!isMissile)
						continue;
					break;
					//-- countermeasures
				case 3:
					if (!isCountermeasure)
						continue;
					break;
				default:
					break;
				}

				ShipWeaponGroup & shipWeaponGroup = getGroup(group);
				shipWeaponGroup.addWeapon(weaponIndex, isMissile, isCountermeasure);
				shipWeaponGroup.clearUserModified();

				++group;
			}
		}
	}

	m_userModified = false;
}

//----------------------------------------------------------------------

bool ShipWeaponGroupsForShip::isUserModified() const
{
	return m_userModified;
}

//----------------------------------------------------------------------

void ShipWeaponGroupsForShip::load_0000(Iff & iff)
{
	ShipObject const * const ship = dynamic_cast<ShipObject const * >(NetworkIdManager::getObjectById(m_shipId));
	int const groupCount = getGroupCount();

	while (iff.enterChunk(Tags::SWGI, true))
	{
		int const groupId = iff.read_int32();

		if (groupId < 0 || groupId >= groupCount)
		{
			WARNING(true, ("ShipWeaponGroupsForShip load_0000 [%s] group [%d] invalid", m_shipId.getValueString().c_str(), groupId));
			iff.exitChunk(Tags::SWGI, true);
			continue;
		}
		
		ShipWeaponGroup & shipWeaponGroup = getGroup(groupId);
		shipWeaponGroup.clear();

		int const numWeapons = iff.read_int32();
		for (int i = 0; i < numWeapons; ++i)
		{
			int const weaponIndex = iff.read_int32();
			bool const isMissile = ship ? ship->isMissile(weaponIndex) : false;
			bool const isCountermeasure = ship ? ship->isCountermeasure(weaponIndex) : false;
			shipWeaponGroup.addWeapon(weaponIndex, isMissile, isCountermeasure);
		}

		if (shipWeaponGroup.isUserModified())
		{
			m_userModified = true;
			shipWeaponGroup.clearUserModified();
		}

		iff.exitChunk(Tags::SWGI);
	}
}

//----------------------------------------------------------------------

void ShipWeaponGroupsForShip::load_0001(Iff & iff)
{
	int const groupCount = getGroupCount();

	while (iff.enterChunk(Tags::SWGI, true))
	{
		int const groupId = iff.read_int32();

		if (groupId < 0 || groupId >= groupCount)
		{
			WARNING(true, ("ShipWeaponGroupsForShip load_0001 [%s] group [%d] invalid", m_shipId.getValueString().c_str(), groupId));
			iff.exitChunk(Tags::SWGI, true);
			continue;
		}
		
		ShipWeaponGroup & shipWeaponGroup = getGroup(groupId);
		shipWeaponGroup.clear();

		int const numWeapons = iff.read_int32();
		for (int i = 0; i < numWeapons; ++i)
		{
			int const weaponIndex = iff.read_int32();
			uint8 const isMissile = iff.read_uint8();
			uint8 const isCountermeasure = iff.read_uint8();
			shipWeaponGroup.addWeapon(weaponIndex, isMissile != 0, isCountermeasure != 0);
		}

		if (shipWeaponGroup.isUserModified())
		{
			m_userModified = true;
			shipWeaponGroup.clearUserModified();
		}

		iff.exitChunk(Tags::SWGI);
	}
}
//----------------------------------------------------------------------

void ShipWeaponGroupsForShip::save(Iff & iff)
{
	if (!isUserModified())
	{
		WARNING(true, ("ShipWeaponGroupsForShip::save attempt to save groups that are not user modified."));
		return;
	}
	
	for (ShipWeaponGroupVector::iterator it = m_weaponGroups->begin(); it != m_weaponGroups->end(); ++it)
	{
		ShipWeaponGroup * const shipWeaponGroup = NON_NULL(*it);

		if (shipWeaponGroup->isEmpty())
			continue;

		iff.insertChunk(Tags::SWGI);
		{
			iff.insertChunkData(static_cast<int32>(shipWeaponGroup->getGroupId()));
			
			int const weaponCount = shipWeaponGroup->getWeaponCount();
			iff.insertChunkData(static_cast<int32>(weaponCount));
			
			{
				int weaponsInserted = 0;
				for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices && weaponsInserted < weaponCount; ++weaponIndex)
				{
					if (shipWeaponGroup->hasWeapon(weaponIndex))
					{
						iff.insertChunkData(static_cast<int32>(weaponIndex));
						iff.insertChunkData(static_cast<uint8>(shipWeaponGroup->hasMissile(weaponIndex)));
						iff.insertChunkData(static_cast<uint8>(shipWeaponGroup->hasCountermeasure(weaponIndex)));
						++weaponsInserted;
					}
				}
			}
		}
		iff.exitChunk(Tags::SWGI);
	}
}

//----------------------------------------------------------------------

bool ShipWeaponGroupsForShip::checkForModifications()
{
	bool modifications = false;

	{
		for (ShipWeaponGroupVector::iterator it = m_weaponGroups->begin(); it != m_weaponGroups->end(); ++it)
		{
			ShipWeaponGroup * const shipWeaponGroup = NON_NULL(*it);
			if (shipWeaponGroup->isUserModified())
			{
				modifications = true;
				shipWeaponGroup->clearUserModified();
			}
		}
	}

	m_userModified = m_userModified || modifications;
	return modifications;
}

//----------------------------------------------------------------------

NetworkId const & ShipWeaponGroupsForShip::getShipId() const
{
	return m_shipId;
}

//----------------------------------------------------------------------

void ShipWeaponGroupsForShip::printInfo(Unicode::String & result) const
{
	ShipObject const * const ship = dynamic_cast<ShipObject const * >(NetworkIdManager::getObjectById(m_shipId));

	char buf[256];
	size_t const buf_size = sizeof(buf);

	snprintf(buf, buf_size, "(unlocalized) Ship [%s] (%s)\n", m_shipId.getValueString().c_str(), (ship != NULL) ? Unicode::wideToNarrow(ship->getLocalizedName()).c_str() : "NOT LOADED");
	result += Unicode::narrowToWide(buf);

	snprintf(buf, buf_size, "(unlocalized) User Modifed [%s]\n", m_userModified ? "yes" : "default");
	result += Unicode::narrowToWide(buf);

	int const groupCount = getGroupCount();

	for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex)
	{
		ShipWeaponGroup const & group = getGroup(groupIndex);
		if (!group.isEmpty())
		{
			snprintf(buf, buf_size, "(unlocalized) Group [%d]\n", groupIndex);
			result += Unicode::narrowToWide(buf);
			
			int const weaponCount = group.getWeaponCount();
			int weaponsPrinted = 0;
			
			for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices && weaponsPrinted < weaponCount; ++weaponIndex)
			{
				if (group.hasWeapon(weaponIndex))
				{
					if (ship != NULL)
					{
						int const chassisSlot = weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first);
						uint32 const componentCrc = ship->getComponentCrc(chassisSlot);
						if (componentCrc != 0)
						{
							ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc(componentCrc);
							std::string const descriptorName = (shipComponentDescriptor != NULL) ? shipComponentDescriptor->getName().getString() : "none";
							Unicode::String const & weaponName = ship->getComponentName(chassisSlot);
							snprintf(buf, buf_size, "%20d -[%s] (%s)\n", weaponIndex, descriptorName.c_str(), Unicode::wideToNarrow(weaponName).c_str());
						}
						else
							snprintf(buf, buf_size, "%20d - WEAPON SLOT NOT INSTALLED\n", weaponIndex);
					}
					else
						snprintf(buf, buf_size, "%20d - SHIP NOT LOADED\n", weaponIndex);

					result += Unicode::narrowToWide(buf);
					++weaponsPrinted;
				}
			}
		}
	}
}


//======================================================================
