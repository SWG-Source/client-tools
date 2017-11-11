// ======================================================================
//
// ClientEffect.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientEffect.h"

#include "clientAudio/Audio.h"
#include "clientDirectInput/ForceFeedbackEffectTemplate.h"
#include "clientDirectInput/ForceFeedbackEffectTemplateList.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/GameCamera.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/PackedRgb.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedRandom/Random.h"

#include <vector>

// ======================================================================

namespace ClientEffectNamespace
{
	float const cms_unsetScaleOverride = -1.0f;
}

using namespace ClientEffectNamespace;

// ======================================================================
// PUBLIC ClientEffect
// ======================================================================

ClientEffect::~ClientEffect()
{
	m_clientEffectTemplate->release();
	delete m_label;
}

// ----------------------------------------------------------------------

/** This function handles execution of effects that aren't dependent on being 
 *  attached to a location or an object (camera and other screen effects).  Note that even though
 *  this is a pure virtual function, we need to implement it here.
 */
void ClientEffect::execute()
{
}

// ======================================================================
// PROTECTED ClientEffect
// ======================================================================

ClientEffect::ClientEffect(const ClientEffectTemplate* const clientEffectTemplate) : 
	m_clientEffectTemplate(clientEffectTemplate),
	m_scaleOverride(cms_unsetScaleOverride),
	m_useRenderEnableFlags(true),
	m_label(new CrcLowerString (""))
{
	NOT_NULL(m_clientEffectTemplate);
	m_clientEffectTemplate->fetch();
}

// ----------------------------------------------------------------------

void ClientEffect::shakeCamera(const Vector& objectPosition_w, const float falloff, const float time, const float magnitude, const float frequency)
{
	if(CuiPreferences::getScreenShake())
	{	
		//-- shake the camera
		GameCamera* const camera = dynamic_cast<GameCamera*> (const_cast<Camera*> (Game::getCamera()));
		if (camera)
		{
			const Vector cameraPosition_w = camera->getPosition_w();
			const float  distanceSquared  = cameraPosition_w.magnitudeBetweenSquared(objectPosition_w);
			const float  falloffSquared   = sqr(falloff);

			if (distanceSquared < falloffSquared)
			{
				const float ratio = 1.f - (distanceSquared / falloffSquared);
				camera->jitter(Vector::unitX, time, magnitude * ratio, frequency);
			}
		}
	}
}

// ----------------------------------------------------------------------

void ClientEffect::playForceFeedbackEffect(Vector const & effectPosition, Vector const & playerPosition, ForceFeedbackEffectTemplate const * const forceFeedbackTemplate, int iterations, float range) const
{
	float const distanceFromEffectToPlayer = effectPosition.magnitudeBetween(playerPosition);
	if(distanceFromEffectToPlayer < range)
	{
		if(forceFeedbackTemplate)
			forceFeedbackTemplate->playEffect(iterations);
	}
}

// ----------------------------------------------------------------------

void ClientEffect::setUniformScale(float const scale)
{
	m_scaleOverride = scale;
}

// ----------------------------------------------------------------------

void ClientEffect::setUseRenderEnableFlags(bool const use)
{
	m_useRenderEnableFlags = use;
}

void ClientEffect::setLabel(CrcLowerString const & label)
{
	*m_label = label;
}

// ======================================================================
// PUBLIC PositionClientEffect
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(PositionClientEffect, true, 0, 0, 0);

// ======================================================================

PositionClientEffect::~PositionClientEffect()
{
}

// ----------------------------------------------------------------------

/** Execute location-based effects (put a footprint decal at (0,0,0), etc.)
 */
void PositionClientEffect::execute()
{
	NOT_NULL(m_clientEffectTemplate);
	ClientEffect::execute();

	NOT_NULL(m_cell);

	const Vector position_w = (m_cell == CellProperty::getWorldCellProperty()) ? m_position : m_cell->getOwner().rotateTranslate_o2w(m_position);

	//-- create the appearances
	{
		ClientEffectTemplate::CreateAppearanceList::const_iterator i = m_clientEffectTemplate->m_cpaFuncs->begin();
		for (; i != m_clientEffectTemplate->m_cpaFuncs->end(); ++i)
		{
			Appearance* const appearance = i->appearanceTemplate->createAppearance ();

			appearance->useRenderEffectsFlag(m_useRenderEnableFlags);

			//-- turn off auto deletion of particle systems (we want to manage the lifetimes)
			ParticleEffectAppearance* const particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(appearance);
			if (particleEffectAppearance)
				particleEffectAppearance->setAutoDelete(false);

			//-- create the object
			Object* object = new MemoryBlockManagedObject();
			object->setAppearance(appearance);
			object->setParentCell(const_cast<CellProperty*>(m_cell));
			CellProperty::setPortalTransitionsEnabled(false);
			object->setPosition_p(m_position);
			CellProperty::setPortalTransitionsEnabled(true);
			RenderWorld::addObjectNotifications(*object);
			object->addNotification(ClientWorld::getIntangibleNotification());
			object->addToWorld();

			float finalScale = 1.0f;
			//use the programatic scale if given and valid, else pick one from the range given from data
			if((m_scaleOverride != cms_unsetScaleOverride) && (m_scaleOverride > 0.0f))
				finalScale = m_scaleOverride;
			else if(i->minScale <= i->maxScale)
				finalScale = Random::randomReal(i->minScale, i->maxScale);
			appearance->setScale(Vector(finalScale, finalScale, finalScale));

			float playbackRate = 1.0f;
			if(i->minPlaybackRate <= i->maxPlaybackRate)
			{
				playbackRate = Random::randomReal(i->minPlaybackRate, i->maxPlaybackRate);
				if (particleEffectAppearance)
					particleEffectAppearance->setPlayBackRate(playbackRate);
			}

			//-- hand the particle system off to the manager, which controls its lifetime
			ClientEffectManager::addManagedParticleSystem(object, NULL, CrcLowerString::empty, Transform::identity, i->timeInSeconds, i->softParticleTerminate, i->ignoreDuration, *m_label);
		}
	}

	//-- create the sounds
	{
		ClientEffectTemplate::PlaySoundList::const_iterator i =  m_clientEffectTemplate->m_psFuncs->begin();
		for (; i != m_clientEffectTemplate->m_psFuncs->end(); ++i)
			if (!i->soundTemplateName.empty())
				Audio::playSound(i->soundTemplateName.c_str(), position_w, m_cell);
	}

	//-- create the lights
	{
		ClientEffectTemplate::CreateLightList::const_iterator i =  m_clientEffectTemplate->m_clFuncs->begin();
		for(; i != m_clientEffectTemplate->m_clFuncs->end(); ++i)
		{
			PackedRgb color(i->r, i->g, i->b);
			Light* const light = new Light(Light::T_point, color.convert());
			light->setConstantAttenuation(i->constantAttenuation);
			light->setLinearAttenuation(i->linearAttenuation);
			light->setQuadraticAttenuation(i->quadraticAttenuation);
			light->setRange(i->range);
			light->setParentCell(const_cast<CellProperty*>(m_cell));
			CellProperty::setPortalTransitionsEnabled(false);
				light->setPosition_p(m_position);
			CellProperty::setPortalTransitionsEnabled(true);
			ClientWorld::addLight(light);

			//-- hand the particle system off to the light, which controls its lifetime
			ClientEffectManager::addManagedLight(light, NULL, CrcLowerString::empty, Transform::identity, i->timeInSeconds, *m_label);
		}
	}

	//-- shake the camera
	{
		if (!m_clientEffectTemplate->m_csFuncs->empty())
		{
			//-- really only need to apply the first camera shake
			ClientEffectTemplate::CameraShakeList::const_iterator i = m_clientEffectTemplate->m_csFuncs->begin();
			shakeCamera(position_w, i->falloffRadius, i->timeInSeconds, i->magnitudeInMeters, i->frequencyInHz);
		}
	}

	ClientObject const * const player = Game::getPlayerCreature();
	if(!player)
		return;
	ClientObject const * const ship = Game::getPlayerPilotedShip();
	ClientObject const * const currentPlayerObject = ship ? ship : player;
	if(!currentPlayerObject)
		return;

	//-- play the force feedback effects
	ClientEffectTemplate::ForceFeedbackList::const_iterator i = m_clientEffectTemplate->m_ffbFuncs->begin();
	for (; i != m_clientEffectTemplate->m_ffbFuncs->end(); ++i)
	{
		playForceFeedbackEffect(position_w, currentPlayerObject->getPosition_w(), i->forceFeedbackTemplate, i->iterations, i->range);
	}
}

// ======================================================================
// PROTECTED PositionClientEffect
// ======================================================================

PositionClientEffect::PositionClientEffect(const ClientEffectTemplate* const clientEffectTemplate, const CellProperty* const cell, const Vector& position, const Vector& up) : 
	ClientEffect(clientEffectTemplate),
	m_cell(cell),
	m_position(position),
	m_up(up)
{
}

// ======================================================================
// PUBLIC ObjectClientEffect
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ObjectClientEffect, true, 0, 0, 0);

// ======================================================================

ObjectClientEffect::~ObjectClientEffect()
{
}

// ----------------------------------------------------------------------

/** Execute object-based effects (i.e. particles attached to gun barrels, etc.)
 */
void ObjectClientEffect::execute()
{
	NOT_NULL(m_clientEffectTemplate);

	ClientEffect::execute();

	//-- determine the hardpoint transform if needed
	Transform               hardpointToObjectTransform;
	const Appearance* const appearance = m_object->getAppearance();
	const bool              hardpointNameValid = m_hardPoint.getString() && *m_hardPoint.getString();
	const bool              hardpointFound = hardpointNameValid && appearance && appearance->findHardpoint(m_hardPoint, hardpointToObjectTransform);

	//-- don't show the particle system if the effect was called on a missing hardpoint
	if (!hardpointFound)
	{
		if (hardpointNameValid)
		{
			DEBUG_WARNING(true, ("ClientEffect::execute: aborting appearance creation hardpoint [%s] doesn't exist on emitting object", m_hardPoint.getString()));
			return;
		}

		hardpointToObjectTransform = m_transform;
	}

	//-- shake the camera
	{
		if (!m_clientEffectTemplate->m_csFuncs->empty())
		{
			//-- really only need to apply the first camera shake
			ClientEffectTemplate::CameraShakeList::const_iterator i = m_clientEffectTemplate->m_csFuncs->begin();
			shakeCamera(m_object->getPosition_w(), i->falloffRadius, i->timeInSeconds, i->magnitudeInMeters, i->frequencyInHz);
		}
	}

	//-- create the appearances
	{
		ClientEffectTemplate::CreateAppearanceList::const_iterator i = m_clientEffectTemplate->m_cpaFuncs->begin();
		for (; i != m_clientEffectTemplate->m_cpaFuncs->end(); ++i)
		{
			Appearance* const appearance = i->appearanceTemplate->createAppearance ();

			appearance->useRenderEffectsFlag(m_useRenderEnableFlags);

			//-- turn off auto deletion of particle systems (we want to manage the lifetimes)
			ParticleEffectAppearance* const particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(appearance);
			if (particleEffectAppearance)
				particleEffectAppearance->setAutoDelete(false);

			//-- create the object
			Object* const object = new MemoryBlockManagedObject();
			object->setAppearance(appearance);
			RenderWorld::addObjectNotifications(*object);
			object->attachToObject_w(m_object, true);
			object->setTransform_o2p(hardpointToObjectTransform);

			float finalScale = 1.0f;
			//use the programatic scale if given and valid, else pick one from the range given from data
			if((m_scaleOverride != cms_unsetScaleOverride) && (m_scaleOverride > 0.0f))
				finalScale = m_scaleOverride;
			else if(i->minScale <= i->maxScale)
				finalScale = Random::randomReal(i->minScale, i->maxScale);
			appearance->setScale(Vector(finalScale, finalScale, finalScale));

			float playbackRate = 1.0f;
			if(i->minPlaybackRate <= i->maxPlaybackRate)
			{
				playbackRate = Random::randomReal(i->minPlaybackRate, i->maxPlaybackRate);
				if (particleEffectAppearance)
					particleEffectAppearance->setPlayBackRate(playbackRate);
			}

			//-- hand the particle system off to the manager, which controls its lifetime
			ClientEffectManager::addManagedParticleSystem(object, m_object, m_hardPoint, m_transform, i->timeInSeconds, i->softParticleTerminate, i->ignoreDuration, *m_label);
		}
	}

	//-- create the sounds
	{
		ClientEffectTemplate::PlaySoundList::const_iterator i =  m_clientEffectTemplate->m_psFuncs->begin();
		for (; i != m_clientEffectTemplate->m_psFuncs->end(); ++i)
		{
			if (i->soundTemplateName.c_str () && *i->soundTemplateName.c_str () != 0)
			{
				if (hardpointNameValid)
					Audio::attachSound(i->soundTemplateName.c_str(), m_object, m_hardPoint.getString());
				else
					Audio::attachSound(i->soundTemplateName.c_str(), m_object);
			}
		}  
	}

	//-- create the lights
	{
		ClientEffectTemplate::CreateLightList::const_iterator i =  m_clientEffectTemplate->m_clFuncs->begin();
		for(; i != m_clientEffectTemplate->m_clFuncs->end(); ++i)
		{
			const PackedRgb color(i->r, i->g, i->b);
			Light* const light = new Light(Light::T_point, color.convert());
			light->setConstantAttenuation(i->constantAttenuation);
			light->setLinearAttenuation(i->linearAttenuation);
			light->setQuadraticAttenuation(i->quadraticAttenuation);
			light->setRange(i->range);
			light->attachToObject_w(m_object, true);
			light->setTransform_o2p(hardpointToObjectTransform);

			//-- hand the particle system off to the light, which controls its lifetime
			ClientEffectManager::addManagedLight(light, m_object, m_hardPoint, m_transform, i->timeInSeconds, *m_label);
		}
	}

	ClientObject const * const player = Game::getPlayerCreature();
	if(!player)
		return;
	ClientObject const * const ship = Game::getPlayerPilotedShip();
	ClientObject const * const currentPlayerObject = ship ? ship : player;
	if(!currentPlayerObject)
		return;

	//-- play the force feedback effects
	ClientEffectTemplate::ForceFeedbackList::const_iterator i = m_clientEffectTemplate->m_ffbFuncs->begin();
	for (; i != m_clientEffectTemplate->m_ffbFuncs->end(); ++i)
	{
		playForceFeedbackEffect(m_object->getPosition_w(), currentPlayerObject->getPosition_w(), i->forceFeedbackTemplate, i->iterations, i->range);
	}
}

// ======================================================================
// PROTECTED ObjectClientEffect
// ======================================================================

ObjectClientEffect::ObjectClientEffect(const ClientEffectTemplate* const clientEffectTemplate, Object* const object, const CrcLowerString& hardpoint) : 
ClientEffect(clientEffectTemplate),
m_object(object),
m_hardPoint(hardpoint),
m_transform()
{
}

//----------------------------------------------------------------------

ObjectClientEffect::ObjectClientEffect(ClientEffectTemplate const * const clientEffectTemplate, Object* object, Transform const & transform) :
ClientEffect(clientEffectTemplate),
m_object(object),
m_hardPoint(CrcLowerString::empty),
m_transform(transform)
{
}

// ======================================================================
