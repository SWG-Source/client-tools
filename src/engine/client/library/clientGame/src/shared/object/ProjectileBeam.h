//======================================================================
//
// ProjectileBeam.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ProjectileBeam_H
#define INCLUDED_ProjectileBeam_H

//======================================================================

class AppearanceTemplate;
class ClientEffectTemplate;

//===================================================================

#include "sharedObject/Object.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedMath/Vector.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "clientGame/ShipProjectileMissHitData.h"

//===================================================================

class ProjectileBeam : public Object
{
public:

	typedef ShipProjectileMissHitData MissHitData;

	ProjectileBeam(ShipObject * ownerShip, int weaponIndex, float length, const AppearanceTemplate* appearanceTemplate, int numLightningBolts, MissHitData const & missHitData);
	virtual ~ProjectileBeam();

	virtual float       alter(float time);

private:

	ProjectileBeam();
	ProjectileBeam(const ProjectileBeam& rhs);
	ProjectileBeam& operator=(const ProjectileBeam& rhs);

private:

	float m_lengthMaximum;
	float m_lengthCurrent;
	MissHitData m_missHitData;
	Watcher<ShipObject> m_ownerShip;
	int m_weaponIndex;
	Timer m_hitEffectTimer;
};

//======================================================================

#endif
