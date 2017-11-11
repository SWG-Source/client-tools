// ======================================================================
//
// ReticleObject.cpp
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ReticleObject.h"

#include "clientGraphics/RenderWorld.h"
#include "clientObject/ReticleManager.h"

#include "dpvsObject.hpp"
//#include "dpvsModel.hpp"

// ======================================================================

ReticleObject::ReticleObject (float const radius) : 
	Object (),
	m_radius (radius),
	m_visible(true),
	m_dpvsRegionOfInfluence (0)
{
	RenderWorld::addObjectNotifications (*this);

	m_dpvsRegionOfInfluence = RenderWorld::createRegionOfInfluence (this, radius);
}

// ----------------------------------------------------------------------

ReticleObject::~ReticleObject ()
{
	if (isInWorld ())
		removeFromWorld ();

	m_dpvsRegionOfInfluence->release ();
#ifdef _DEBUG
	m_dpvsRegionOfInfluence = NULL;
#endif
}

// ----------------------------------------------------------------------

void ReticleObject::addToWorld ()
{
	addDpvsObject (m_dpvsRegionOfInfluence);
	Object::addToWorld ();
}

// ----------------------------------------------------------------------

void ReticleObject::removeFromWorld ()
{
	removeDpvsObject (m_dpvsRegionOfInfluence);
	Object::removeFromWorld ();
}

// ----------------------------------------------------------------------

void ReticleObject::setRegionOfInfluenceEnabled (const bool enabled) const
{
	if (isActive ())
	{
		if (enabled && m_visible)
			ReticleManager::enableReticleObject (*this);
		else
			ReticleManager::disableReticleObject (*this);
	}
}

// ----------------------------------------------------------------------

float ReticleObject::getRadius () const
{
	return m_radius;
}

// ----------------------------------------------------------------------

void ReticleObject::setRadius (float r)
{
	bool wasInWorld = isInWorld();
	if(wasInWorld)
	{		
		removeFromWorld();
	}
	m_dpvsRegionOfInfluence->release ();	
	m_dpvsRegionOfInfluence = RenderWorld::createRegionOfInfluence (this, r);
	m_radius = r;
	if(wasInWorld)
	{	
		addToWorld();
	}
}

// ----------------------------------------------------------------------

bool ReticleObject::isVisible () const
{
	return m_visible;
}

// ----------------------------------------------------------------------

void ReticleObject::setVisible (bool visible)
{
	m_visible = visible;
}

// ======================================================================
