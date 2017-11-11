//========================================================================
//
// ClientWeaponObjectTemplate.h - A wrapper around SharedWeaponObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientWeaponObjectTemplate_H
#define _INCLUDED_ClientWeaponObjectTemplate_H

#include "sharedGame/SharedWeaponObjectTemplate.h"

class AppearanceTemplate;
class ClientEffectTemplate;
class Projectile;

class ClientWeaponObjectTemplate : public SharedWeaponObjectTemplate
{
public:
	         ClientWeaponObjectTemplate(const std::string & filename);
	virtual ~ClientWeaponObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	virtual Object *createObject(void) const;
	Projectile* createProjectile (bool nonTracking, bool useRicochet) const;

	const ClientEffectTemplate* getFireClientEffectTemplate() const;

protected:
	virtual void postLoad ();

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientWeaponObjectTemplate(const ClientWeaponObjectTemplate &);
	ClientWeaponObjectTemplate & operator =(const ClientWeaponObjectTemplate &);

private:

	std::string                 m_lookUpId;

	const AppearanceTemplate*   m_appearanceTemplate;
	const ClientEffectTemplate* m_fireClientEffectTemplate;
	const ClientEffectTemplate* m_hitCreatureClientEffectTemplate;
	const ClientEffectTemplate* m_hitMetalClientEffectTemplate;
	const ClientEffectTemplate* m_hitStoneClientEffectTemplate;
	const ClientEffectTemplate* m_hitWoodClientEffectTemplate;
	const ClientEffectTemplate* m_hitOtherClientEffectTemplate;
	const ClientEffectTemplate* m_missHitWaterClientEffectTemplate;
	const ClientEffectTemplate* m_missHitTerrainClientEffectTemplate;
	const ClientEffectTemplate* m_missHitCreatureClientEffectTemplate;
	const ClientEffectTemplate* m_missHitMetalClientEffectTemplate;
	const ClientEffectTemplate* m_missHitStoneClientEffectTemplate;
	const ClientEffectTemplate* m_missHitWoodClientEffectTemplate;
	const ClientEffectTemplate* m_missHitOtherClientEffectTemplate;
	const ClientEffectTemplate* m_missHitNothingClientEffectTemplate;
	const ClientEffectTemplate* m_ricochetClientEffectTemplate;  // used for lightsaber blocking
};

#endif	// _INCLUDED_ClientWeaponObjectTemplate_H
