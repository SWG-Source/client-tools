//======================================================================
//
// ShipWeaponGroupsForShip.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipWeaponGroupsForShip_H
#define INCLUDED_ShipWeaponGroupsForShip_H

//======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class Iff;
class ShipWeaponGroup;

//----------------------------------------------------------------------

class ShipWeaponGroupsForShip
{
public:

	explicit ShipWeaponGroupsForShip(NetworkId const & shipId);
	~ShipWeaponGroupsForShip();

	int getGroupCount() const;
	NetworkId const & getShipId() const;
	ShipWeaponGroup const & getGroup(int groupId) const;
	ShipWeaponGroup & getGroup(int groupId);

	void resetDefaults();
	bool isUserModified() const;

	void load_0000(Iff & iff);
	void load_0001(Iff & iff);
	void save(Iff & iff);

	bool checkForModifications();
	void printInfo(Unicode::String & result) const;

private:

	ShipWeaponGroupsForShip(ShipWeaponGroupsForShip const & rhs);
	ShipWeaponGroupsForShip operator=(ShipWeaponGroupsForShip const & rhs);

	typedef stdvector<ShipWeaponGroup *>::fwd ShipWeaponGroupVector;

	NetworkId m_shipId;
	ShipWeaponGroupVector * m_weaponGroups;
	bool m_userModified;
};

//======================================================================

#endif
