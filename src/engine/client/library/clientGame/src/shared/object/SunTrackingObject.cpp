//===================================================================
//
// SunTrackingObject.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SunTrackingObject.h"

#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "sharedObject/AlterResult.h"

//===================================================================
// PUBLIC SunTrackingObject
//===================================================================

SunTrackingObject::SunTrackingObject (const float yawMaximumRadiansPerSecond) :
	Object (),
	m_yawMaximumRadiansPerSecond (yawMaximumRadiansPerSecond),
	m_barrel (0),
	m_pitchMinimumRadians (0.f),
	m_pitchMaximumRadians (0.f),
	m_pitchMaximumRadiansPerSecond (0.f),
	m_on (true)
{
}

//-------------------------------------------------------------------

SunTrackingObject::~SunTrackingObject ()
{
	delete m_barrel;
	m_barrel = NULL;
}

//-------------------------------------------------------------------

void SunTrackingObject::setBarrel (Object* const object, const float pitchMinimumRadians, const float pitchMaximumRadians, const float pitchMaximumRadiansPerSecond)
{
	if (m_barrel)
		delete m_barrel;

	m_barrel = object;
	m_pitchMinimumRadians = pitchMinimumRadians;
	m_pitchMaximumRadians = pitchMaximumRadians;
	m_pitchMaximumRadiansPerSecond = pitchMaximumRadiansPerSecond;
}

//-------------------------------------------------------------------

void SunTrackingObject::setState (const bool on)
{
	m_on = on;
}

//-------------------------------------------------------------------

float SunTrackingObject::alter (const float elapsedTime)
{
	const float result = Object::alter (elapsedTime);
	if (result == AlterResult::cms_kill) //lint !e777 // testing floats for equality // Yes, okay here, we're using constants.
		return AlterResult::cms_kill;

	if (!m_on)
		return AlterResult::cms_alterNextFrame;

	const Vector& directionToSun_w = ClientProceduralTerrainAppearance::getDirectionToLight ();

	//-- base face target
	{
		const Vector facing_o = rotate_w2o (directionToSun_w);
		if (facing_o != Vector::zero)
		{
			const float maximumYawThisFrame = m_yawMaximumRadiansPerSecond * elapsedTime;
			const float yaw = clamp (-maximumYawThisFrame, facing_o.theta (), maximumYawThisFrame);
			yaw_o (yaw);
		}
	}

	//-- barrel face target
	{
		if (m_barrel)
		{
			//-- barrel face target
			{
				const Vector facing_o = m_barrel->rotate_w2o (directionToSun_w);
				if (facing_o != Vector::zero)
				{
					//-- pitch
					{
						const float maximumPitchThisFrame = m_pitchMaximumRadiansPerSecond * elapsedTime;
						const float pitch = clamp (-maximumPitchThisFrame, facing_o.phi (), maximumPitchThisFrame);
						m_barrel->pitch_o (pitch);
					}
				}
			}
		}
	}

	return AlterResult::cms_alterNextFrame;
}

//===================================================================

