//======================================================================
//
// ShipWeaponGroup.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipWeaponGroup_H
#define INCLUDED_ShipWeaponGroup_H

//======================================================================

class BitArray;

class ShipWeaponGroup
{
public:
	typedef stdvector<int>::fwd IntVector;

	explicit ShipWeaponGroup(int groupId);
	~ShipWeaponGroup();

	bool hasWeapon(int weaponIndex) const;
	bool isUserModified() const;
	int getGroupId() const;
	bool isEmpty() const;
	int getWeaponCount() const;

	bool canWeaponAdd(bool isMissile, bool isCountermeasure) const;

	void addWeapon(int weaponIndex, bool isMissile, bool isCountermeasure);
	void removeWeapon(int weaponIndex);
	void clear();
	void clearUserModified();

	bool hasMissiles() const;
	bool hasMissile(int weaponIndex) const;

	bool hasCountermeasures() const;
	bool hasCountermeasure(int weaponIndex) const;

private:

	ShipWeaponGroup(ShipWeaponGroup const & rhs);
	ShipWeaponGroup & operator=(ShipWeaponGroup const & rhs);

	int         m_groupId;
	BitArray *  m_weaponIndices;
	BitArray *  m_missileIndices;
	BitArray *  m_countermeasureIndices;
	bool        m_userModified;
};

//======================================================================

#endif
