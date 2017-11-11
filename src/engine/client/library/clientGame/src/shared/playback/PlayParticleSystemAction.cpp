// ======================================================================
//
// PlayParticleSystemAction.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayParticleSystemAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAudio/Audio.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/PlayParticleSystemActionTemplate.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGame/CellObject.h"
#include "clientObject/HardpointObject.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"

// ======================================================================

namespace PlayParticleSystemActionNamespace
{
	bool  s_logActivity;
	Tag const TAG_TLCX = TAG(T,L,C,X);
	Tag const TAG_TLCY = TAG(T,L,C,Y);
	Tag const TAG_TLCZ = TAG(T,L,C,Z);
	Tag const TAG_TLCC = TAG(T,L,C,C);

	struct LoopingSoundRecord {
		int refCount;
		SoundId soundId;
	};

	std::map<uint32, LoopingSoundRecord> s_loopingSoundRefCount;
}

using namespace PlayParticleSystemActionNamespace;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(PlayParticleSystemAction, true, 0, 0, 0);

// ======================================================================

bool PlayParticleSystemAction::update(float deltaTime, PlaybackScript &script)
{
	if (!m_initialized)
	{
		//-- Handle startup delay (prevents us from shooting flamethrower at the wrong spot).
		if (!m_startupWaitTimer.updateNoReset(deltaTime))
		{
			//-- We're still delaying startup.
			return true;
		}

		//-- Get elapsed time from playback script.
		m_timer.setExpireTime(getTimeDuration(script));

		//-- Perform all startup actions.
		bool const startupSuccess = startup(script);
		if (!startupSuccess)
		{
			DEBUG_REPORT_LOG(s_logActivity, ("PlayParticleSystemAction: killing action because startup initialization failed.\n"));
			return false;
		}
	}
	else
	{
		//-- Stop action when timer goes off.
		if (m_timer.updateNoReset(deltaTime))
		{
			//-- Stop activity.
			shutdown(script);
			return false;
		}
	}
	
	//-- We're not done yet, indicate we should keep going.
	return true;
}

// ----------------------------------------------------------------------

void PlayParticleSystemAction::cleanup(PlaybackScript &playbackScript)
{
	shutdown(playbackScript);
}

// ----------------------------------------------------------------------

void PlayParticleSystemAction::stop(PlaybackScript &script)
{
	shutdown(script);
}

// ======================================================================

PlayParticleSystemAction::PlayParticleSystemAction(PlayParticleSystemActionTemplate const &actionTemplate):
	PlaybackAction(&actionTemplate),
	m_startupWaitTimer(0.25f),
	m_initialized(false),
	m_timer(),
	m_object(),
	m_useLocation(false),
	m_emitterLocation()
{
}

// ----------------------------------------------------------------------

inline PlayParticleSystemActionTemplate const &PlayParticleSystemAction::getActionTemplate() const
{
	return *NON_NULL(safe_cast<PlayParticleSystemActionTemplate const *>(getPlaybackActionTemplate()));
}

// ----------------------------------------------------------------------

bool PlayParticleSystemAction::startup(PlaybackScript &script)
{
	//-- Setup as initialized.
	DEBUG_FATAL(m_initialized, ("Logic error."));
	m_initialized = true;

	//-- Get object we'll attach the particle to.
	Object *emitterObject = getEmitterObject(script);
	if (!emitterObject)
	{
		DEBUG_REPORT_LOG(s_logActivity, ("PlayParticleSystemAction: emitter object is NULL, aborting action."));
		return false;
	}

	//-- Create the appearance.
	std::string const appearanceFilename = getAppearanceFilename(script);
	DEBUG_REPORT_LOG(s_logActivity && !TreeFile::exists(appearanceFilename.c_str()), ("PlayParticleSystemAction: specified appearance [%s] doesn't exist, will get default appearance.\n", appearanceFilename.c_str()));

	Appearance *const appearance = AppearanceTemplateList::createAppearance(appearanceFilename.c_str());
	if (!appearance)
	{
		DEBUG_WARNING(s_logActivity, ("PlayParticleSystemAction: creating appearance returned NULL appearance, aborting action."));
		return false;
	}

	PlayParticleSystemActionTemplate const &actionTemplate = getActionTemplate();
	m_useLocation = actionTemplate.getUsesLocation();

	Object * newObject = NULL; 
	std::string hardpointName;
	if(m_useLocation)
	{
		m_emitterLocation = getTargetLocation(script);
		newObject = new Object();
	}
	else
	{	
		//-- Get the attachment hardpoint.
		hardpointName = getHardpointName(script);
		if (hardpointName.empty())
		{
			DEBUG_WARNING(s_logActivity, ("PlayParticleSystemAction: no hardpoint name set, aborting action."));
			return false;
		}

		//-- Create hardpoint object.
		TemporaryCrcString  crcHardpointName(hardpointName.c_str(), true);
	#ifdef _DEBUG
		Transform  unusedTransform(Transform::IF_none);
		DEBUG_WARNING(s_logActivity && (!emitterObject->getAppearance() || !emitterObject->getAppearance()->findHardpoint(crcHardpointName, unusedTransform)), ("PlayParticleSystemAction: Emitter object id=[%s], template=[%s] missing required hardpoint [%s], will play at object origin.", emitterObject->getNetworkId().getValueString().c_str(), emitterObject->getObjectTemplateName(), hardpointName.c_str()));
	#endif

		newObject = new HardpointObject(crcHardpointName);
		
	}
	
	if (!newObject)
	{
		delete appearance;
		DEBUG_WARNING(s_logActivity, ("PlayParticleSystemAction: failed to create hardpoint object, aborting action."));
		return false;
	}

	//-- Make object renderable.
	newObject->setAppearance(appearance);
	RenderWorld::addObjectNotifications(*newObject);

	//-- Attach to emitter.
	if(m_useLocation)
	{
		newObject->addNotification(ClientWorld::getIntangibleNotification());	
		newObject->addToWorld();
		if(getTargetCell(script) != NULL)
			newObject->setParentCell(const_cast<CellProperty *>(getTargetCell(script)->getCellProperty()));
		newObject->setPosition_p(m_emitterLocation);	
	}
	else
	{
		newObject->attachToObject_p(emitterObject, true);		
	}
	
	//-- Keep track of object.
	m_object = newObject;
	
	//-- Play startup sound.
	std::string const beginSoundName = getBeginSoundName(script);
	std::string const loopSoundName = getLoopSoundName(script);
	
	if (!internalGetSoundIsLooping(loopSoundName.c_str()) && !beginSoundName.empty())
		IGNORE_RETURN(Audio::attachSound(beginSoundName.c_str(), emitterObject, m_useLocation ? NULL : hardpointName.c_str()));

	if (!loopSoundName.empty())
	{
		//-- Play loop sound.
		internalPlayLoopingSound(loopSoundName.c_str(), emitterObject, m_useLocation ? NULL : hardpointName.c_str());		
	}

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

void PlayParticleSystemAction::shutdown(PlaybackScript const &script)
{
	if (!m_initialized)
	{
		// nothing to do.
		return;
	}

	//-- Tell particle system to shut down.
	bool  killObjectDirectly = true;

	Appearance *const baseAppearance = (m_object != 0) ? m_object->getAppearance() : 0;
	if (baseAppearance)
	{
		ParticleEffectAppearance *const appearance = ParticleEffectAppearance::asParticleEffectAppearance(baseAppearance);
		if (appearance)
		{
			// Tell the particle system to shut down gracefully over time.  It will kill the object for us.
			DEBUG_REPORT_LOG(s_logActivity, ("PlayParticleSystemAction: shutting down particle system for emitter object id=[%s],template=[%s].\n", m_object->getNetworkId().getValueString().c_str(), m_object->getObjectTemplateName()));
			appearance->setEnabled(false);

			killObjectDirectly = false;
		}
	}

	if (killObjectDirectly && (m_object != 0))
	{
		// Tell the object to kill itself.  Either the Appearance is not a particle appearance or there is no appearance.
		DEBUG_REPORT_LOG(s_logActivity, ("PlayParticleSystemAction: killing appearance (non-particle) for emitter object id=[%s],template=[%s].\n", m_object->getNetworkId().getValueString().c_str(), m_object->getObjectTemplateName()));
		m_object->kill();
	}

	//-- Drop reference to hardpoint object.
	m_object = 0;

	//-- Stop loop sound.
	std::string const loopSoundName = getLoopSoundName(script);
	bool wasPlaying = internalGetSoundIsLooping(loopSoundName.c_str());
	if (wasPlaying)
	{
		internalStopLoopingSound(loopSoundName.c_str());
	}

	//-- Get object we'll attach the particle to.
	Object const *emitterObject = getEmitterConstObject(script);
	if (emitterObject)
	{	
		if(wasPlaying && !internalGetSoundIsLooping(loopSoundName.c_str()))
		{		
			if(m_useLocation)
			{
				//-- Play stop sound.
				std::string const endSoundName = getEndSoundName(script);
				if (!endSoundName.empty())
					IGNORE_RETURN(Audio::attachSound(endSoundName.c_str(), emitterObject, NULL));			
			}
			else
			{
				//-- Get the attachment hardpoint.
				std::string const hardpointName = getHardpointName(script);
				if (!hardpointName.empty())
				{
					//-- Play stop sound.
					std::string const endSoundName = getEndSoundName(script);
					if (!endSoundName.empty())
						IGNORE_RETURN(Audio::attachSound(endSoundName.c_str(), emitterObject, hardpointName.c_str()));
				}
			}
		}
	}

	//-- Remember that we've already cleaned up.
	m_initialized = false;
}

// ----------------------------------------------------------------------

Object *PlayParticleSystemAction::getEmitterObject(PlaybackScript &script) const
{
	PlayParticleSystemActionTemplate const &actionTemplate = getActionTemplate();
	int const actorIndex = actionTemplate.getEmittingActorIndex();

	return script.getActor(actorIndex);
}

// ----------------------------------------------------------------------

Object const *PlayParticleSystemAction::getEmitterConstObject(PlaybackScript const &script) const
{
	PlayParticleSystemActionTemplate const &actionTemplate = getActionTemplate();
	int const actorIndex = actionTemplate.getEmittingActorIndex();

	return script.getActor(actorIndex);
}

// ----------------------------------------------------------------------

float PlayParticleSystemAction::getTimeDuration(PlaybackScript const &script) const
{
	PlayParticleSystemActionTemplate const &actionTemplate = getActionTemplate();

	//-- Retrieve time duration from the playback script.
	float timeDuration  = 3.0f;
	bool const gotValue = script.getFloatVariable(actionTemplate.getTimeDurationTag(), timeDuration);
	DEBUG_WARNING(!gotValue, ("PlayParticleSystemAction: script is missing assignment for time duration tag."));
	UNREF(gotValue);

	return timeDuration;
}

// ----------------------------------------------------------------------

std::string PlayParticleSystemAction::getAppearanceFilename(PlaybackScript const &script) const
{
	PlayParticleSystemActionTemplate const &actionTemplate = getActionTemplate();

	//-- Retrieve time duration from the playback script.
	std::string  appearanceFilename;

	bool const gotValue = script.getStringVariable(actionTemplate.getAppearanceNameTag(), appearanceFilename);
	DEBUG_WARNING(!gotValue, ("PlayParticleSystemAction: script is missing assignment for appearance filename tag."));
	UNREF(gotValue);

	return appearanceFilename;
}

// ----------------------------------------------------------------------

std::string PlayParticleSystemAction::getHardpointName(PlaybackScript const &script) const
{
	PlayParticleSystemActionTemplate const &actionTemplate = getActionTemplate();

	//-- Retrieve time duration from the playback script.
	std::string  hardpointName("muzzle");

	bool const gotValue = script.getStringVariable(actionTemplate.getHardpointNameTag(), hardpointName);
	DEBUG_WARNING(!gotValue, ("PlayParticleSystemAction: script is missing assignment for hardpoint name tag."));
	UNREF(gotValue);

	return hardpointName;
}

// ----------------------------------------------------------------------

std::string PlayParticleSystemAction::getBeginSoundName(PlaybackScript const &script) const
{
	PlayParticleSystemActionTemplate const &actionTemplate = getActionTemplate();

	std::string  soundName;

	bool const gotValue = script.getStringVariable(actionTemplate.getStartSoundNameTag(), soundName);
	DEBUG_WARNING(!gotValue, ("PlayParticleSystemAction: script is missing assignment for start sound name tag."));
	UNREF(gotValue);

	return soundName;
}

// ----------------------------------------------------------------------

std::string PlayParticleSystemAction::getLoopSoundName(PlaybackScript const &script) const
{
	PlayParticleSystemActionTemplate const &actionTemplate = getActionTemplate();

	std::string  soundName;

	bool const gotValue = script.getStringVariable(actionTemplate.getLoopSoundNameTag(), soundName);
	DEBUG_WARNING(!gotValue, ("PlayParticleSystemAction: script is missing assignment for loop sound name tag."));
	UNREF(gotValue);

	return soundName;
}

// ----------------------------------------------------------------------

std::string PlayParticleSystemAction::getEndSoundName(PlaybackScript const &script) const
{
	PlayParticleSystemActionTemplate const &actionTemplate = getActionTemplate();

	std::string  soundName;

	bool const gotValue = script.getStringVariable(actionTemplate.getStopSoundNameTag(), soundName);
	DEBUG_WARNING(!gotValue, ("PlayParticleSystemAction: script is missing assignment for stop sound name tag."));
	UNREF(gotValue);

	return soundName;
}

// ----------------------------------------------------------------------

Vector PlayParticleSystemAction::getTargetLocation(const PlaybackScript &script) const
{
	Vector result;
	script.getFloatVariable(TAG_TLCX, result.x);
	script.getFloatVariable(TAG_TLCY, result.y);
	script.getFloatVariable(TAG_TLCZ, result.z);
	return result;
}

// ----------------------------------------------------------------------

const CellObject *PlayParticleSystemAction::getTargetCell(const PlaybackScript &script) const
{
	return static_cast<CellObject *>(script.getObjectVariable(TAG_TLCC));
}

// ----------------------------------------------------------------------

bool PlayParticleSystemAction::internalGetSoundIsLooping(const char *soundName)
{
	if((soundName == NULL) || (strlen(soundName) == 0))
		return false;
	uint32 crcSoundName = Crc::normalizeAndCalculate(soundName);
	std::map<uint32, LoopingSoundRecord>::iterator i = s_loopingSoundRefCount.find(crcSoundName);
	if(i == s_loopingSoundRefCount.end())
	{
		return false;
	}
	else
	{
		return true;
	}
}

// ----------------------------------------------------------------------

void PlayParticleSystemAction::internalPlayLoopingSound(const char *soundName, Object *obj, const char *hardpointName)
{
	if((soundName == NULL) || (strlen(soundName) == 0))
		return;
	uint32 crcSoundName = Crc::normalizeAndCalculate(soundName);
	std::map<uint32, LoopingSoundRecord>::iterator i = s_loopingSoundRefCount.find(crcSoundName);
	if(i == s_loopingSoundRefCount.end())
	{
		LoopingSoundRecord lsr;
		lsr.refCount = 1;
		lsr.soundId = Audio::attachSound(soundName, obj, hardpointName);
		s_loopingSoundRefCount.insert(std::make_pair(crcSoundName, lsr));
	}
	else
	{
		i->second.refCount++;
	}
}

// ----------------------------------------------------------------------

void PlayParticleSystemAction::internalStopLoopingSound(const char *soundName)
{
	if((soundName == NULL) || (strlen(soundName) == 0))
		return;
	uint32 crcSoundName = Crc::normalizeAndCalculate(soundName);
	std::map<uint32, LoopingSoundRecord>::iterator i = s_loopingSoundRefCount.find(crcSoundName);
	DEBUG_FATAL(i == s_loopingSoundRefCount.end(), ("stopping looping sound that wasn't in the map '%s'", soundName));
	i->second.refCount--;
	DEBUG_FATAL(i->second.refCount < 0, ("stopping looping sound caused ref count to go below zero '%s'", soundName));
	if(i->second.refCount <= 0)
	{
		Audio::stopSound(i->second.soundId);
		s_loopingSoundRefCount.erase(i);
	}
}

// ======================================================================
