//===================================================================
//
// ShipProjectile.h
// asommers 5-31-2000
//
// copyright 2000, verant interactive, inc.
//
//===================================================================

#ifndef INCLUDED_ShipProjectile_H
#define INCLUDED_ShipProjectile_H

//===================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

class ShipDamageMessage;

//===================================================================

#include "clientGame/Projectile.h"
#include "clientGame/ShipObject.h"
#include "clientGAme/ShipProjectileMissHitData.h"

//===================================================================

class ShipProjectile : public Projectile
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	typedef ShipProjectileMissHitData MissHitData;

public:

	static void install();

	static int getNumberOfInstances();

public:

	ShipProjectile(AppearanceTemplate const * appearanceFileName, MissHitData const & missHitData);
	virtual ~ShipProjectile();

	virtual float alter(float elapsedTime);

	void setOwnerShip(ShipObject *ownerShip);
	void setTravelDistance(float travelDistance);
	void setWeaponIndex(int weaponIndex);

private:

	static void remove();

private:


private:

	ShipProjectile();
	ShipProjectile(ShipProjectile const &);
	ShipProjectile & operator=(ShipProjectile const &);

private:

	MissHitData m_missHitData;

	float m_travelDistance;
	Watcher<ShipObject> m_ownerShip;
	bool m_canPlayFlyby;
	int m_weaponIndex;
};

//===================================================================

#endif
