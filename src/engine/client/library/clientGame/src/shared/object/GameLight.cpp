//
// GameLight.cpp
// asommers 7-7-99
//
// copyright 1999, bootprint entertainment
//

//-------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/GameLight.h"
#include "clientGame/TangibleObject.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "sharedFile/Iff.h"
#include "sharedObject/AlterResult.h"
#include "sharedRandom/Random.h"

//-------------------------------------------------------------------

namespace GameLightNamespace
{
	bool ms_night;
}

using namespace GameLightNamespace;

//-------------------------------------------------------------------

void GameLight::setNight (const bool night)
{
	ms_night = night;
}

//-------------------------------------------------------------------

GameLight::GameLight (void)
: Light (Light::T_point, VectorArgb::solidBlack),
	minColor (),
	maxColor (),
	minRange (CONST_REAL (0)),
	maxRange (CONST_REAL (0)),
	minTime  (CONST_REAL (0)),
	maxTime  (CONST_REAL (0)),
	timer (),
	m_flicker (false),
	m_dayNightAware (false),
	m_onOffAware (false)
{
}

GameLight::~GameLight (void)
{
}

//-------------------------------------------------------------------

float GameLight::alter (float time)
{
	if(m_dayNightAware || m_onOffAware)
	{
		bool finalActiveState = true;
		if (m_dayNightAware)
		{
			finalActiveState = ms_night;
		}

		if (m_onOffAware)
		{
			Object *obj = getAttachedTo();
			if(obj)
			{
				TangibleObject *tangibleObject = obj->asClientObject()->asTangibleObject();
				if(tangibleObject)
				{
					finalActiveState = finalActiveState && tangibleObject->hasCondition(TangibleObject::C_onOff);
				}
			}
		}

		if (finalActiveState && !isActive ())
		{
			setActive (true);
			
			// restart the particle system - movable furniture lights with particles.
			// Can be moved when off and restarting the particle system resets particles with long
			// durations so they won't dangle back at the previous location.
			Appearance * appearance = getAppearance();
			if(appearance)
			{
				ParticleEffectAppearance* const particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(appearance);
				if(particleEffectAppearance)
					particleEffectAppearance->restart();
			}
		}
		else if (!finalActiveState && isActive ())
		{
			setActive (false);
		}
	}

	if (m_flicker && maxTime != CONST_REAL (0) && timer.updateZero (time))
	{
		//-- reset the color
		Light::setDiffuseColor (VectorArgb::linearInterpolate (minColor, maxColor, Random::randomReal (CONST_REAL (0), CONST_REAL (1))));
		
		//-- reset the timer
		timer.setExpireTime (Random::randomReal (minTime, maxTime));
		timer.reset ();

		//-- reset the range
		Light::setRange (Random::randomReal (minRange, maxRange));
	}

	IGNORE_RETURN (Light::alter (time));

	return AlterResult::cms_alterNextFrame;
}

//-------------------------------------------------------------------

Light* GameLight::createLight (Iff* iff)
{
	NOT_NULL (iff);

	Light* light = 0;

	switch (iff->getCurrentName ())
	{
	case TAG (A,M,B,N):
		{
			iff->enterChunk (TAG (A,M,B,N));

				real r = iff->read_float ();
				real g = iff->read_float ();
				real b = iff->read_float ();
				VectorArgb argb (CONST_REAL (1), r, g, b);

				light = new Light (Light::T_ambient, argb);
				light->setDebugName ("ambient");

			iff->exitChunk ();
		}
		break;

	case TAG (P,A,R,A):
		{
			iff->enterChunk (TAG (P,A,R,A));

				real r = iff->read_float ();
				real g = iff->read_float ();
				real b = iff->read_float ();
				VectorArgb argb (CONST_REAL (1), r, g, b);

				real yaw   = convertDegreesToRadians (iff->read_float ());
				real pitch = convertDegreesToRadians (iff->read_float ());

				light = new Light (Light::T_parallel, argb);
				light->setDebugName ("parallel");
				light->yaw_o (yaw);
				light->pitch_o (pitch);

			iff->exitChunk ();
		}
		break;

	case TAG (P,O,I,N):
		{
			iff->enterChunk (TAG (P,O,I,N));

				real r = iff->read_float ();
				real g = iff->read_float ();
				real b = iff->read_float ();
				VectorArgb argb (CONST_REAL (1), r, g, b);

				real x = iff->read_float ();
				real y = iff->read_float ();
				real z = iff->read_float ();

				real pointRange = iff->read_float ();

				light = new GameLight ();
				light->setDebugName ("point");
				light->move_o (Vector (x, y, z));
				light->setDiffuseColor (argb);
				light->setRange (pointRange);

			iff->exitChunk ();
		}
		break;

	case TAG (F,L,I,C):
		{
			iff->enterChunk (TAG (F,L,I,C));

				real minr = iff->read_float ();
				real ming = iff->read_float ();
				real minb = iff->read_float ();
				VectorArgb flickerMinArgb (CONST_REAL (1), minr, ming, minb);

				real maxr = iff->read_float ();
				real maxg = iff->read_float ();
				real maxb = iff->read_float ();
				VectorArgb flickerMaxArgb (CONST_REAL (1), maxr, maxg, maxb);

				real x = iff->read_float ();
				real y = iff->read_float ();
				real z = iff->read_float ();

				real flickerMinRange = iff->read_float ();
				real flickerMaxRange = iff->read_float ();

				real flickerMinTime = iff->read_float ();
				real flickerMaxTime = iff->read_float ();

			iff->exitChunk ();

			GameLight* gameLight = new GameLight ();
			gameLight->setDebugName ("point flicker");
			gameLight->move_o (Vector (x, y, z));
			gameLight->setFlicker (true);
			gameLight->setRange (flickerMinRange, flickerMaxRange);
			gameLight->setColor (flickerMinArgb, flickerMaxArgb);
			gameLight->setTime (flickerMinTime, flickerMaxTime);

			light = gameLight;
		}
		break;

	default:
		{
			char currentTagName [5];
			ConvertTagToString (iff->getCurrentName(), currentTagName);

			DEBUG_FATAL (true, ("GameLight::createLight - unknown light type %s", currentTagName));
		}
		break;
	}

	return light;
}

//-------------------------------------------------------------------

