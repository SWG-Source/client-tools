// ======================================================================
//
// CelestialObject.cpp
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/CelestialObject.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/RenderWorld.h"
#include "clientTerrain/CelestialAppearance.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Production.h"
#include "sharedObject/CellProperty.h"
#include "sharedMath/VectorArgb.h"

// ======================================================================

namespace CelestialObjectNamespace
{
	bool ms_disableCollide = true;

	void remove ();

	std::string const & ms_debugInfoSectionName = "CelestialObject";
}

using namespace CelestialObjectNamespace;

// ======================================================================

CelestialObject::CollideFunction      CelestialObject::ms_collideFunction;
uint16                                CelestialObject::ms_collideFlags;
CelestialObject::NotificationFunction CelestialObject::ms_notificationFunction;

// ======================================================================

void CelestialObject::install ()
{
	DebugFlags::registerFlag (ms_disableCollide, "ClientTerrain/CelestialObject", "disableCollide");

	ExitChain::add (remove, "CelestialObject::remove");
}

// ----------------------------------------------------------------------

void CelestialObjectNamespace::remove ()
{
	DebugFlags::unregisterFlag (ms_disableCollide);
}

// ----------------------------------------------------------------------

void CelestialObject::setCollideFunction (CelestialObject::CollideFunction collideFunction, const uint16 flags)
{
	ms_collideFunction = collideFunction;
	ms_collideFlags    = flags;
}

//-------------------------------------------------------------------

void CelestialObject::setNotificationFunction (CelestialObject::NotificationFunction notificationFunction)
{
	ms_notificationFunction = notificationFunction;
}

// ======================================================================

CelestialObject::CelestialObject (char const * const shaderTemplateName, float const size, char const * const glowShaderTemplateName, float const glowSize, bool const space) :
	Object(),
	m_space(space),
	m_celestialAppearance (0),
	m_alpha (space ? 1.f : 0.f),
	m_glowObject (0),
	m_glowCelestialAppearance (0),
	m_glowAlpha (0.f)
{
	if (shaderTemplateName && *shaderTemplateName && size > 0.f)
	{
		m_celestialAppearance = new CelestialAppearance (size, size, 0.f, shaderTemplateName);
		setAppearance (m_celestialAppearance);
	}

	if (glowShaderTemplateName && *glowShaderTemplateName && glowSize > 0.f)
	{
		m_glowObject = new Object;
		m_glowCelestialAppearance = new CelestialAppearance (glowSize, glowSize, 0.f, glowShaderTemplateName);
		m_glowObject->setAppearance (m_glowCelestialAppearance);

		if (ms_notificationFunction)
			ms_notificationFunction (*m_glowObject);

		RenderWorld::addObjectNotifications (*m_glowObject);

		if (m_space)
		{
			m_glowObject->attachToObject_p (this, false);
			m_glowObject->addToWorld();
		}
		else
		{
			m_glowObject->attachToObject_p (this, true);
		}
	}
}

// ----------------------------------------------------------------------

CelestialObject::~CelestialObject ()
{
	m_celestialAppearance = 0;

	if (m_glowObject)
	{
		delete m_glowObject;
		m_glowObject = 0;

		m_glowCelestialAppearance = 0;
	}
}

// ----------------------------------------------------------------------

void CelestialObject::setActive (const bool active)
{
	Object::setActive (active);

	if (m_glowObject)
	{
		if (isActive () && !m_glowObject->isActive ())
			m_glowObject->setActive (true);
		else
			if (!isActive () && m_glowObject->isActive ())
				m_glowObject->setActive (false);
	}
}

// ----------------------------------------------------------------------

void CelestialObject::setAlpha (const float alpha)
{
	m_alpha = alpha;

	if (m_celestialAppearance)
		m_celestialAppearance->setAlpha (true, alpha, true, alpha);

	if (m_glowCelestialAppearance)
		m_glowCelestialAppearance->setAlpha (true, m_glowAlpha * alpha, true, m_glowAlpha * alpha);
}

// ----------------------------------------------------------------------

void CelestialObject::setColor (const VectorArgb& color)
{
	if (m_celestialAppearance)
		m_celestialAppearance->setColor (color);

	if (m_glowCelestialAppearance)
		m_glowCelestialAppearance->setColor (color);
}

// ----------------------------------------------------------------------

void CelestialObject::update (const float elapsedTime, const Camera& camera)
{
	//-- if we don't have a glow, return
	if (!m_glowObject || !m_glowObject->isActive ())
		return;

	float desiredAlpha = 0.f;

	if (m_space)
	{
		desiredAlpha = 1.f;
	}
	else
	{
		//-- calculate desired glow alpha
		float const angle = acos(camera.getObjectFrameK_w().dot(-getObjectFrameK_w()));

		if (angle < PI_OVER_2)
			desiredAlpha = 1.f - (angle / PI_OVER_2);
	}

	if (m_space || (!ms_disableCollide && ms_collideFunction))
	{
		const Vector start_w = camera.getPosition_w ();
		Vector const end_w = start_w + -getObjectFrameK_w() * camera.getFarPlane();

		CollideParameters collideParameters;
		collideParameters.setType(CollideParameters::T_opaqueSolid);

		CollisionInfo result;
		if (ms_collideFunction (camera.getParentCell (), start_w, end_w, collideParameters, result, ms_collideFlags, 0, NULL))
			desiredAlpha = 0.f;
	}

	if (m_glowAlpha < desiredAlpha)
	{
		m_glowAlpha += 2.f * elapsedTime;
		if (m_glowAlpha > desiredAlpha)
			m_glowAlpha = desiredAlpha;
	}
	else
		if (m_glowAlpha > desiredAlpha)
		{
			m_glowAlpha -= 2.f * elapsedTime;
			if (m_glowAlpha < desiredAlpha)
				m_glowAlpha = desiredAlpha;
		}

	//-- set the desired alpha
	setAlpha (m_alpha);
}

// ----------------------------------------------------------------------

float CelestialObject::getGlowAlpha () const
{
	return m_glowAlpha;
}

//-----------------------------------------------------------------------

void CelestialObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
#if PRODUCTION == 0
/**
  When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
*/

/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
*/
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Space", m_space);
//	CelestialAppearance*        m_celestialAppearance;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Alpha", m_alpha);
//	Object*                     m_glowObject;
//	CelestialAppearance*        m_glowCelestialAppearance;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "GlowAlpha", m_glowAlpha);

	Object::getObjectInfo(propertyMap);

#else
	UNREF(propertyMap);
#endif
}

// ======================================================================
