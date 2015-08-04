//======================================================================
//
// CuiTurntableAdapter.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiTurntableAdapter.h"

#include "clientGraphics/Light.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/Clock.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/ObjectList.h"

//======================================================================


namespace
{
	float angleClamp2Pi (float a)
	{		
		if (a < 0)
			return a + floorf (-a / PI_TIMES_2) * PI_TIMES_2 + PI_TIMES_2;
		
		if (a > PI_TIMES_2)
			return a - floorf (a / PI_TIMES_2) * PI_TIMES_2;
		
		return a;
	}

	float angleDiff2Pi( float a, float b )
	{
        float diff = a - b;
		
        if ( diff < -PI )
			diff += PI_TIMES_2;
        else if ( diff > PI )
			diff -= PI_TIMES_2;
		
        return diff;
	}
}

//----------------------------------------------------------------------


CuiTurntableAdapter::CuiTurntableAdapter (CuiWidget3dObjectListViewer & viewer) :
m_viewer (&viewer),
m_angles             (new AngleVector),
m_isTurning          (false),
m_baseObjectRotation (PI),
m_autoRotateObjects  (true),
m_currentIndex       (0),
m_previousAngle      (0.0f),
m_spotLight          (new Light (Light::T_point, VectorArgb::solidWhite)),
m_arrangementRadius  (0.0f),
m_appearanceRadius   (0.0f),
m_spacingModifier    (1.0f),
m_turnToFace         (true)
{
//	m_viewer->SetAutoZoomOutOnly (true);
	m_viewer->Attach (0);
	
	m_spotLight->setConstantAttenuation  (0.0f);
	m_spotLight->setLinearAttenuation    (0.5f);
	m_spotLight->setQuadraticAttenuation (1.0f);
	m_spotLight->setRange (100.0f);

	m_viewer->addObject (*m_spotLight);
}

//----------------------------------------------------------------------

CuiTurntableAdapter::~CuiTurntableAdapter ()
{
	m_viewer->removeObject (*m_spotLight);

	delete m_spotLight;
	m_spotLight = 0;

	delete m_angles;
	m_angles = 0;

	m_viewer->Detach (0);
	m_viewer = 0;
}

//----------------------------------------------------------------------

void CuiTurntableAdapter::arrange ()
{
	m_angles->clear ();

	ObjectVector ov;
	getObjectVector (ov);
	
	if (ov.empty ())
		return;
	
	m_angles->reserve (ov.size ());
	
	const int count = static_cast<int>(ov.size ());
	
	m_appearanceRadius = 0.0f;
	
	{
		for (ObjectVector::const_iterator it = ov.begin (); it != ov.end (); ++it)
		{
			Object * const obj = *it;
			if (obj)
			{
				const Appearance * const app = obj->getAppearance ();
				
				if (app)
					m_appearanceRadius = std::max (m_appearanceRadius, obj->getAppearanceSphereRadius ());
			}
		}
	}
	
	float theta     = PI_TIMES_2 / count;

	if (count == 1)
		m_arrangementRadius   =  0.0f;
	else if (count == 2)
	{
		m_arrangementRadius   = m_appearanceRadius;
		theta = PI * 0.7f;
	}
	else
		m_arrangementRadius   = (m_spacingModifier * m_appearanceRadius) / sin (theta);

	m_spotLight->setRange (m_arrangementRadius * 3.0f);
	if (m_appearanceRadius)
	{
		m_spotLight->setQuadraticAttenuation (0.50f / m_appearanceRadius);
	}
	else
	{
		m_spotLight->setQuadraticAttenuation (1.00f);
	}
	
	float alpha = 0.0f;
	
	for (ObjectVector::const_iterator it = ov.begin (); it != ov.end (); ++it, alpha += theta)
	{
		Object * const obj = *it;
		if (obj)
		{
			const float arrangementAlpha = - (PI * 0.5f + alpha);
			Vector pos;
			pos.x = m_arrangementRadius * cos (arrangementAlpha);
			pos.z = m_arrangementRadius * sin (arrangementAlpha);
			obj->setPosition_p (pos);
		}

		//-- clamp alpha for sanity checking
		m_angles->push_back (angleClamp2Pi(alpha));
	}

	if (m_autoRotateObjects)
		updateObjectRotations ();

	m_viewer->setViewDirty (true);
//	m_viewer->SetAutoZoomOutOnly (false);
	m_viewer->recomputeZoom ();
	m_viewer->forceUpdate ();
//	m_viewer->SetAutoZoomOutOnly (true);
}

//----------------------------------------------------------------------

void CuiTurntableAdapter::turnTo  (int index, bool playSound)
{
	if (m_currentIndex == index)
		return;

	if (playSound)
		CuiSoundManager::play (CuiSounds::select_rotate);

	m_currentIndex = index;

	m_previousAngle = m_viewer->getCameraYaw ();

	m_isTurning = true;
}

//----------------------------------------------------------------------

void CuiTurntableAdapter::setBaseObjectRotation (float radians)
{
	if (m_baseObjectRotation != radians)
	{
		m_baseObjectRotation = radians;
		
		if (m_autoRotateObjects)
		{
			updateObjectRotations ();
		}
	}
}

//----------------------------------------------------------------------

void CuiTurntableAdapter::setAutoRotateObjects (bool b)
{
	m_autoRotateObjects = b;

	if (m_autoRotateObjects)
		updateObjectRotations ();
}

//----------------------------------------------------------------------

void CuiTurntableAdapter::getObjectVector (ObjectVector & ov)
{
	const int count = m_viewer->getRenderObjectCount();
	ov.reserve (count - 1);
	
	for (int i = 0; i < count; ++i)
	{
		Object * const obj = m_viewer->getRenderObjectByIndex (i);
		if (obj)
		{
			const Appearance * const app = obj->getAppearance ();
			
			if (app)
			{
				ov.push_back (obj);
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiTurntableAdapter::updateObjectRotations ()
{
	ObjectVector ov;
	getObjectVector (ov);

	for (ObjectVector::const_iterator it = ov.begin (); it != ov.end (); ++it)
	{
		Object * const obj = *it;
		if (obj)
		{
			obj->resetRotate_o2p ();
			obj->yaw_o (m_viewer->getCameraYaw () + m_baseObjectRotation);
		}
	}
}

//----------------------------------------------------------------------

void CuiTurntableAdapter::update(float deltaTimeSecs)
{
	if (m_isTurning)
	{
		if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_angles->size ()))
		{
			m_currentIndex = -1;
			m_isTurning = false;
			return;
		}

		static const float EPSILON = 0.001f;
		static const float SPEED   = 5.0f;

		DEBUG_FATAL (static_cast<int>(m_angles->size ()) <= m_currentIndex, ("bad values"));

		//-- m_angles are already clamped
		float const targetAngle = (*m_angles)[m_currentIndex];
		float const currentAngle = angleClamp2Pi (m_viewer->getCameraYaw ());
		float const diffAngle = angleDiff2Pi (targetAngle, currentAngle);
		float newAngle = angleClamp2Pi (currentAngle + (diffAngle * std::min (1.0f, SPEED * deltaTimeSecs)));
		float const angleDiff_2Pi = angleDiff2Pi(newAngle, targetAngle);

		if (WithinEpsilonExclusive (0.0f, angleDiff_2Pi, EPSILON))
		{
			newAngle = targetAngle;
			m_isTurning = false;
		}
		
		m_viewer->setCameraYaw (newAngle);

		if (!m_viewer->hasFlags (CuiWidget3dObjectListViewer::F_lightLockToCamera))
			m_viewer->setLightYaw (newAngle + PI * 0.5f);
		
		{
			Vector lightPos;
			const float arrangementAngle = - (PI * 0.5f + newAngle);
			lightPos.y = m_appearanceRadius * 2.0f + 0.1f;
			lightPos.x = (m_arrangementRadius + m_appearanceRadius) * cos (arrangementAngle);
			lightPos.z = (m_arrangementRadius + m_appearanceRadius) * sin (arrangementAngle);
			
			m_spotLight->setPosition_p (lightPos);
		}
		
		if (m_turnToFace)
			updateObjectRotations ();
	}
}

//----------------------------------------------------------------------

Object * CuiTurntableAdapter::getCurrentObject      ()
{
	if (m_currentIndex < 0)
		return 0;

	ObjectVector ov;
	getObjectVector (ov);

	if (m_currentIndex >= static_cast<int>(ov.size ()))
		return 0;

	return ov [static_cast<size_t>(m_currentIndex)];
}

//----------------------------------------------------------------------

void CuiTurntableAdapter::setSpacingModifier    (float f)
{
	m_spacingModifier = f;
}

//----------------------------------------------------------------------

void CuiTurntableAdapter::setTurnToFace         (bool b)
{
	m_turnToFace = b;
}

//======================================================================
