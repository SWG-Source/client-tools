//======================================================================
//
// ShipWeaponGroupManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipWeaponGroupManager_H
#define INCLUDED_ShipWeaponGroupManager_H

//======================================================================

class Iff;
class NetworkId;
class ShipWeaponGroup;
class ShipWeaponGroupsForShip;

//----------------------------------------------------------------------

class ShipWeaponGroupManager
{
public:

	enum GroupType { GT_Projectile, GT_Missile, GT_Types };

	struct Messages
	{
		struct WeaponGroupChanged
		{
			typedef std::pair<GroupType, int> Payload;
		};

		struct WeaponGroupsResetForShip
		{
			typedef NetworkId Payload;
		};
	};

	static void install();
	static void remove();

	static void reset();
	static void clear();
	static void update(float elapsedTimeSecs);

	static ShipWeaponGroupsForShip & getShipWeaponGroupsForShip(NetworkId const & shipId);

	typedef stdvector<ShipWeaponGroupsForShip const *>::fwd ShipWeaponGroupsForShipVector;

	static void getAllShipWeaponGroupsForShip(ShipWeaponGroupsForShipVector & result);

	static bool fireWeaponGroupForCurrentShip(int groupId);
	static bool fireCurrentWeaponGroupForShip(GroupType groupType);
	static int getCurrentGroup(GroupType groupType);
	static bool findCurrentGroup(GroupType groupType, bool isPrevious);
	static bool fireCountermeasure();

	static int getMissileTypeIdFromCurrentGroup();
	static int getMissileTypeIdFromGroupId(int groupId);
	static int getWeaponIndexFromCurrentGroup(GroupType gt);

	static void resetDefaultsForShip(NetworkId const & shipId, bool override);
	static void resetDefaults();
	static void updateWeaponGroups();

	static void stopFiringWeaponGroupForCurrentShip(int groupId);
	static void stopFiringCurrentWeaponGroupForShip(GroupType groupType);

private:

	static void save();
	static void load();
	static void load_0000(Iff & iff);
	static void load_0001(Iff & iff);
};

//======================================================================

#endif
