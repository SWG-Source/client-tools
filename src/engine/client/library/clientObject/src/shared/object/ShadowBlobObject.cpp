// ======================================================================
//
// ShadowBlobObject.cpp
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ShadowBlobObject.h"

#include "clientGraphics/RenderWorld.h"
#include "clientObject/ShadowBlobManager.h"

#include "dpvsObject.hpp"
//#include "dpvsModel.hpp"

// ======================================================================

ShadowBlobObject::ShadowBlobObject (float const radius) : 
	Object (),
	m_radius (radius),
	m_visible(true),
	m_dpvsRegionOfInfluence (0)
{
	RenderWorld::addObjectNotifications (*this);

	m_dpvsRegionOfInfluence = RenderWorld::createRegionOfInfluence (this, radius);
}

// ----------------------------------------------------------------------

ShadowBlobObject::~ShadowBlobObject ()
{
	if (isInWorld ())
		removeFromWorld ();

	m_dpvsRegionOfInfluence->release ();
#ifdef _DEBUG
	m_dpvsRegionOfInfluence = NULL;
#endif
}

// ----------------------------------------------------------------------

void ShadowBlobObject::addToWorld ()
{
	addDpvsObject (m_dpvsRegionOfInfluence);
	Object::addToWorld ();
}

// ----------------------------------------------------------------------

void ShadowBlobObject::removeFromWorld ()
{
	removeDpvsObject (m_dpvsRegionOfInfluence);
	Object::removeFromWorld ();
}

// ----------------------------------------------------------------------

void ShadowBlobObject::setRegionOfInfluenceEnabled (const bool enabled) const
{
	if (isActive ())
	{
		if (enabled && m_visible)
			ShadowBlobManager::enableShadowBlobObject (*this);
		else
			ShadowBlobManager::disableShadowBlobObject (*this);
	}
}

// ----------------------------------------------------------------------

float ShadowBlobObject::getRadius () const
{
	return m_radius;
}

// ----------------------------------------------------------------------

bool ShadowBlobObject::isVisible () const
{
	return m_visible;
}

// ----------------------------------------------------------------------

void ShadowBlobObject::setVisible (bool visible)
{
	m_visible = visible;
}

// ======================================================================
