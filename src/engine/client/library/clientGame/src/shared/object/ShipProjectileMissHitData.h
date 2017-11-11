//======================================================================
//
// ShipProjectileMissHitData.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipProjectileMissHitData_H
#define INCLUDED_ShipProjectileMissHitData_H

//======================================================================

class ShipObject;
class CellProperty;
class Vector;
class Object;
class ClientEffectTemplate;

//----------------------------------------------------------------------

class ShipProjectileMissHitData
{
public:
	
	enum HitType
	{
		HT_other,  // other
		HT_metal,  // ships
		HT_stone,  // asteroids
		HT_acid,
		HT_ice,
		HT_molten,
		HT_obsidian,
		HT_shield, // shield
		HT_shieldShell, // shield shell
		HT_friendlyFire // friendly fire
	};
	
	ClientEffectTemplate const * m_missHitOtherClientEffectTemplate;
	ClientEffectTemplate const * m_missHitMetalClientEffectTemplate;
	ClientEffectTemplate const * m_missHitStoneClientEffectTemplate;
	ClientEffectTemplate const * m_missHitAcidClientEffectTemplate;
	ClientEffectTemplate const * m_missHitIceClientEffectTemplate;
	ClientEffectTemplate const * m_missHitMoltenClientEffectTemplate;
	ClientEffectTemplate const * m_missHitObsidianClientEffectTemplate;
	ClientEffectTemplate const * m_missHitShieldClientEffectTemplate;
	ClientEffectTemplate const * m_missHitShieldShellClientEffectTemplate;
	ClientEffectTemplate const * m_missHitFriendlyFireClientEffectTemplate;
	
public:
	
	static void install();
	static void remove();

public:

	ShipProjectileMissHitData();
	~ShipProjectileMissHitData();
	
	void fetch();
	void release();
	ClientEffectTemplate const * getClientEffectTemplateForHitType(HitType hitType) const;
	HitType findHitTypeForObject(Object const & object) const;
	
	bool handleCollision(Object & collider, ShipObject * ownerShip, int weaponIndex, CellProperty const * lastCellProperty, Vector const & lastPosition_w, Vector const & endPosition_w, Vector & resultEndPosition_w, bool canDisplayHitEffect);
	void handleHit      (Object & collider, ShipObject * ownerShip, int weaponIndex, Object const & childObject, Vector hitPoint_w, Vector hitNormal_w);
	static void triggerDamageFeedback(Object const * const object, ShipObject * ownerShip, Vector const & pos_w);	
};

//======================================================================

#endif
