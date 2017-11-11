//===================================================================
//
// ProjectileBeam.cpp
//
// Copyright 2005 Sony Online Entertainment
// All Rights Reserved.
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ProjectileBeam.h"

#include "clientGame/ShipObject.h"
#include "clientParticle/LightningAppearance.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"

//----------------------------------------------------------------------

namespace ProjectileBeamNamespace
{
	float s_hitEffectTimerThreshold = 0.3f;
}

using namespace ProjectileBeamNamespace;


//----------------------------------------------------------------------

ProjectileBeam::ProjectileBeam(ShipObject * ownerShip, int weaponIndex, float length, const AppearanceTemplate* appearanceTemplate, int numLightningBolts, MissHitData const & missHitData) :
m_lengthMaximum(length),
m_lengthCurrent(length),
m_missHitData(missHitData),
m_ownerShip(ownerShip),
m_weaponIndex(weaponIndex),
m_hitEffectTimer(0.0f)
{
	m_missHitData.fetch();

	if (NULL == appearanceTemplate)
		WARNING(true, ("ProjectileBeam NULL appearanceTemplate"));
	else
		setAppearance(appearanceTemplate->createAppearance());

	{
		Vector const & startPos_w = getPosition_w();
		Vector const & endPos_w = startPos_w + (getObjectFrameK_w() * m_lengthMaximum);

		Appearance * const appearance = getAppearance();
		LightningAppearance * const lightningAppearance = appearance ? appearance->asLightningAppearance() : NULL;
		
		if (NULL != lightningAppearance)
		{
			int const boltCount = clamp(1, numLightningBolts, LightningAppearance::getMaxLightningBoltCount());
			for (int i = 0; i < boltCount; ++i)
				lightningAppearance->setPosition_w(i, startPos_w, endPos_w);
			
			lightningAppearance->setEndAppearancesEnabled(false);
		}
	}
}

//----------------------------------------------------------------------

ProjectileBeam::~ProjectileBeam()
{
	m_missHitData.release();
}

//----------------------------------------------------------------------

float ProjectileBeam::alter(float time)
{
	Appearance * const appearance = getAppearance();
	LightningAppearance * const lightningAppearance = appearance ? appearance->asLightningAppearance() : NULL;

	CellProperty const * const cellProperty = getParentCell();
	Vector const & startPos_w = getPosition_w();
	Vector const & endPos_w = startPos_w + (getObjectFrameK_w() * m_lengthMaximum);

	bool const canDisplayHitEffect = m_hitEffectTimer.updateNoReset(time);

	bool hitSomething = false;

	Vector resultEndPos_w = endPos_w;
	if (m_missHitData.handleCollision(*this, m_ownerShip, m_weaponIndex, cellProperty, startPos_w, endPos_w, resultEndPos_w, canDisplayHitEffect))
	{
		hitSomething = true;
		if (canDisplayHitEffect)
		{
			m_hitEffectTimer.setExpireTime(s_hitEffectTimerThreshold);
			m_hitEffectTimer.reset();
		}
	}
	
	if (NULL != lightningAppearance)
	{
		int const numBolts = lightningAppearance->getActiveBoltCount();
		for (int i = 0; i < numBolts; ++i)
			lightningAppearance->setPosition_w(i, startPos_w, resultEndPos_w);

		lightningAppearance->setEndAppearancesEnabled(hitSomething);
	}

	if (Object::alter(time) == AlterResult::cms_kill)
		return AlterResult::cms_kill;

	return AlterResult::cms_alterNextFrame;
}

//----------------------------------------------------------------------
