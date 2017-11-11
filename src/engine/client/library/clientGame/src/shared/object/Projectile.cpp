//===================================================================
//
// Projectile.cpp
//
// Copyright 2000, 2001 Verant Interactive, Inc.
// All Rights Reserved.
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/Projectile.h"

#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ProjectileDynamics.h"
#include "clientGraphics/RenderWorld.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"

//===================================================================

Projectile::Projectile(const AppearanceTemplate* const appearanceTemplate, const ClientEffectTemplate* const fireClientEffectTemplate, const ClientEffectTemplate* const missHitNothingClientEffectTemplate) :
	Object(),
	m_lastCellObject(0),
	m_lastCellProperty(0),
	m_lastPosition_w(),
	m_target(0),
	m_speed(0),
	m_firstAlter(true),
	m_timer(),
	m_createdFireClientEffect(false),
	m_fireClientEffectTemplate(fireClientEffectTemplate),
	m_missHitNothingClientEffectTemplate(missHitNothingClientEffectTemplate),
	m_startPosition_w(),
	m_isPaused(false)
{
	//-- setup appearance
	if (appearanceTemplate)
	{
		setAppearance(appearanceTemplate->createAppearance());
	}

	//-- setup client effect
	if (m_fireClientEffectTemplate)
		m_fireClientEffectTemplate->fetch();

	if (m_missHitNothingClientEffectTemplate)
		m_missHitNothingClientEffectTemplate->fetch();

	//-- setup dynamics
	setDynamics(new ProjectileDynamics(this));

	//-- add notifications
	RenderWorld::addObjectNotifications(*this);
	CellProperty::addPortalCrossingNotification(*this);
	addNotification(ClientWorld::getIntangibleNotification());
}

//-------------------------------------------------------------------

Projectile::~Projectile()
{
	if (m_fireClientEffectTemplate)
	{
		m_fireClientEffectTemplate->release();
		m_fireClientEffectTemplate = 0;
	}

	if (m_missHitNothingClientEffectTemplate)
	{
		m_missHitNothingClientEffectTemplate->release();
		m_missHitNothingClientEffectTemplate = 0;
	}

	m_lastCellProperty = 0;
}

//-------------------------------------------------------------------

void Projectile::setFacing(const CellProperty* const startCell, const Vector& startPosition_w, const Vector& endPosition_w)
{
	if (getParentCell() != startCell)
		setParentCell(const_cast<CellProperty*>(startCell));

	const Vector start_p = startCell != CellProperty::getWorldCellProperty() ? startCell->getOwner().rotateTranslate_w2o(startPosition_w) : startPosition_w;

	CellProperty::setPortalTransitionsEnabled(false);
		setPosition_p(start_p);
	CellProperty::setPortalTransitionsEnabled(true);

	lookAt_o(rotateTranslate_w2o(endPosition_w));
}

//-------------------------------------------------------------------

void Projectile::setExpirationTime(const float expirationTime)
{
	m_timer.setExpireTime(expirationTime);
}

//-------------------------------------------------------------------

void Projectile::setSpeed(const float speed)
{
	m_speed = speed;

	if (!m_isPaused)
	{
		safe_cast<ProjectileDynamics*>(getDynamics())->setCurrentVelocity_w(getObjectFrameK_w() * speed);
	}
}

//-------------------------------------------------------------------

float Projectile::getSpeed()
{
	return m_speed;;
}


//-------------------------------------------------------------------

void Projectile::setTarget(const Object* const target)
{
	m_target = target;
}

//-------------------------------------------------------------------

float Projectile::alter(float time)
{
	//-- create our initial fire effect
	if (!m_createdFireClientEffect)
	{
		m_createdFireClientEffect = true;

		if (m_fireClientEffectTemplate)
		{
			ClientEffect* const clientEffect = m_fireClientEffectTemplate->createClientEffect(getParentCell(), getPosition_p(), Vector::unitY);
			clientEffect->execute();
			delete clientEffect;
		}
	}

	//-- see if we have run out of deltaTime
	if (m_firstAlter)
		m_firstAlter = false;
	else
		if (m_timer.updateZero(time))
		{
			if (m_missHitNothingClientEffectTemplate)
			{
				ClientEffect* const clientEffect = m_missHitNothingClientEffectTemplate->createClientEffect(getParentCell(), getPosition_p(), Vector::unitY);
				clientEffect->execute();
				delete clientEffect;
			}

			return AlterResult::cms_kill;
		}

	//-- move along by velocity and see if we're dead
	m_lastCellProperty = getParentCell();
	m_lastCellObject   = &m_lastCellProperty->getOwner();
	m_lastPosition_w   = getPosition_w();

	return Object::alter(time);
}

//-------------------------------------------------------------------

void Projectile::setStartPosition_w(Vector const &startPosition)
{
	m_startPosition_w = startPosition;
}

//-------------------------------------------------------------------

void Projectile::setPaused(bool paused)
{
	if (paused != m_isPaused)
	{
		m_isPaused = paused;

		if (m_isPaused)
		{
			safe_cast<ProjectileDynamics*>(getDynamics())->setCurrentVelocity_w(Vector(0.f, 0.f, 0.f));
		}
		else
		{
			safe_cast<ProjectileDynamics*>(getDynamics())->setCurrentVelocity_w(getObjectFrameK_w() * m_speed);
		}
	}
}

//-------------------------------------------------------------------

bool Projectile::getPaused()
{
	return m_isPaused;
}

//-------------------------------------------------------------------

