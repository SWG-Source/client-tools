//======================================================================
//
// ShipWeaponGroup.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipWeaponGroup.h"

#include "sharedFoundation/BitArray.h"

//======================================================================

ShipWeaponGroup::ShipWeaponGroup(int groupId) :
m_groupId(groupId),
m_weaponIndices(new BitArray(8)),
m_missileIndices(new BitArray(8)),
m_countermeasureIndices(new BitArray(8)),
m_userModified(false)
{
}

//----------------------------------------------------------------------

ShipWeaponGroup::~ShipWeaponGroup()
{
	delete m_weaponIndices;
	m_weaponIndices = NULL;

	delete m_missileIndices;
	m_missileIndices = NULL;

	delete m_countermeasureIndices;
	m_countermeasureIndices = NULL;
}

//----------------------------------------------------------------------

bool ShipWeaponGroup::hasWeapon(int weaponIndex) const
{
	return m_weaponIndices->testBit(weaponIndex);
}

//----------------------------------------------------------------------

bool ShipWeaponGroup::isUserModified() const
{
	return m_userModified;
}

//----------------------------------------------------------------------

int ShipWeaponGroup::getGroupId() const
{
	return m_groupId;
}

//----------------------------------------------------------------------

bool ShipWeaponGroup::canWeaponAdd(bool isMissile, bool isCountermeasure) const
{
	if (isEmpty())
		return true;

	if (isMissile)
		return hasMissiles();

	if (isCountermeasure)
		return hasCountermeasures();

	return !hasMissiles() && !hasCountermeasures();
}

//----------------------------------------------------------------------

void ShipWeaponGroup::addWeapon(int weaponIndex, bool isMissile, bool isCountermeasure)
{
	if (!canWeaponAdd(isMissile, isCountermeasure))
		return;

	if (!m_weaponIndices->testBit(weaponIndex))
	{
		m_weaponIndices->setBit(weaponIndex);

		if (isMissile)
			m_missileIndices->setBit(weaponIndex);
		else
			m_missileIndices->clearBit(weaponIndex);

		if (isCountermeasure)
			m_countermeasureIndices->setBit(weaponIndex);
		else
			m_countermeasureIndices->clearBit(weaponIndex);

		m_userModified = true;
	}
}

//----------------------------------------------------------------------

void ShipWeaponGroup::removeWeapon(int weaponIndex)
{
	if (m_weaponIndices->testBit(weaponIndex))
	{
		m_weaponIndices->clearBit(weaponIndex);
		m_missileIndices->clearBit(weaponIndex);
		m_countermeasureIndices->clearBit(weaponIndex);
		m_userModified = true;
	}
}

//----------------------------------------------------------------------

void ShipWeaponGroup::clear()
{
	m_weaponIndices->clear();
	m_missileIndices->clear();
	m_countermeasureIndices->clear();
	m_userModified = true;
}

//----------------------------------------------------------------------

void ShipWeaponGroup::clearUserModified()
{
	m_userModified = false;
}

//----------------------------------------------------------------------

bool ShipWeaponGroup::isEmpty() const
{
	return m_weaponIndices->empty();
}

//----------------------------------------------------------------------

int ShipWeaponGroup::getWeaponCount() const
{
	return m_weaponIndices->getNumberOfSetBits();
}

//----------------------------------------------------------------------

bool ShipWeaponGroup::hasMissiles() const
{
	return !m_missileIndices->empty();
}

//----------------------------------------------------------------------

bool ShipWeaponGroup::hasMissile(int weaponIndex) const
{
	return m_missileIndices->testBit(weaponIndex);
}

//----------------------------------------------------------------------

bool ShipWeaponGroup::hasCountermeasures() const
{
	return !m_countermeasureIndices->empty();
}

//----------------------------------------------------------------------

bool ShipWeaponGroup::hasCountermeasure(int weaponIndex) const
{
	return m_countermeasureIndices->testBit(weaponIndex);
}

//======================================================================
