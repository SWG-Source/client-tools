//======================================================================
//
// ClientDataFile_VehicleThrusterSoundRuntimeData.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_VehicleThrusterSoundRuntimeData.h"
#include "clientGame/ClientDataFile_VehicleThrusterSoundData.h"

#include "clientAudio/Audio.h"
#include "clientAudio/Sound2.h"
#include "clientAudio/Sound2d.h"
#include "sharedObject/Object.h"

//======================================================================

VehicleThrusterSoundRuntimeData ::VehicleThrusterSoundRuntimeData() :
m_idle(),
m_run(),
m_accel(),
m_decel(),
m_fadeInVolume(1.0f),
m_fadeInRate(1.0f),
m_fadeInOffset(0.0f),
m_fadeInLoaded(false),
m_fadeInSoundId(),
m_stateCurrent(SS_idle),
m_stateTarget(SS_idle),
m_accelTotalSoundTime(0),
m_decelTotalSoundTime(0),
m_maximumSpeed(0.0f),
m_object(),
m_hardpointName()
{
}

//----------------------------------------------------------------------

void VehicleThrusterSoundRuntimeData::transitionToIntermediateState()
{
	Audio::stopSound(getSoundIdByState(getOppositeState(m_stateCurrent)),   0.0f);
	Audio::stopSound(getSoundIdByState(m_stateCurrent),  3.0f);
	
	const SoundState transitionState = getIntermediateTransitionState(m_stateCurrent);
	fadeSoundIn(getSoundIdByState(transitionState), 1.0f, 0);
	m_stateCurrent = transitionState;
}

//----------------------------------------------------------------------

void VehicleThrusterSoundRuntimeData::transitionToFinalState()
{
	bool transitionReady = false;
	
	int currentSoundTime = 0;
	int totalSoundTime   = 0;
	int soundTimeRemaining = 0;
	
	//	if (m_stateCurrent != getIntermediateTransitionState(s))
	//		return;
	
	SoundId & transitionSoundId = getSoundIdByState(m_stateCurrent);
	
	Sound2 * const s2 = Audio::getSoundById(transitionSoundId);
	
	if (!Audio::isSoundPlaying(transitionSoundId))
	{
		transitionReady = true;
	}
	else if (s2 && s2->isInfiniteLooping())
	{
		transitionReady = true;
	}
	else if (Audio::isSampleForSoundIdPlaying(transitionSoundId) && Audio::getCurrentSoundTime(transitionSoundId, totalSoundTime, currentSoundTime) && totalSoundTime > 0)
	{
		getTotalTimeHolder(m_stateCurrent) = totalSoundTime;
		soundTimeRemaining = totalSoundTime - currentSoundTime;
		int const transitionThreshold = clamp(500, totalSoundTime / 4, 2000);
		if (soundTimeRemaining < transitionThreshold)
		{
			transitionReady = true;
		}
	}
	
	if (transitionReady)
	{
		SoundId & targetSoundId = getSoundIdByState(m_stateTarget);
		
		const float fadeInTime = soundTimeRemaining * 0.001f;
		Audio::stopSound(getSoundIdByState(getOppositeState(m_stateTarget)),  0.0f);
		Audio::stopSound(transitionSoundId, fadeInTime);
		fadeSoundIn(targetSoundId, fadeInTime * 0.7f, 0);
		m_stateCurrent = m_stateTarget;
	}
}

//----------------------------------------------------------------------

void VehicleThrusterSoundRuntimeData::transitionToReverseState()
{
	SoundId & firstSoundId       = getSoundIdByState(m_stateCurrent);
	const SoundState secondState = getOppositeState(m_stateCurrent);
	SoundId & secondSoundId      = getSoundIdByState(secondState);
	
	int firstCurrentSoundTime = 0;
	int firstTotalSoundTime   = 0;
	if (Audio::isSampleForSoundIdPlaying(firstSoundId) && Audio::getCurrentSoundTime(firstSoundId, firstTotalSoundTime, firstCurrentSoundTime) && firstTotalSoundTime > 0)
	{
		getTotalTimeHolder(m_stateCurrent) = firstTotalSoundTime;
		
		float firstSoundTimePercent   =(firstTotalSoundTime > 0) ? static_cast<float>(firstCurrentSoundTime) / static_cast<float>(firstTotalSoundTime) : 0.0f;
		
		int & secondTotalTime = getTotalTimeHolder(secondState);
		if (secondTotalTime == 0)
			IGNORE_RETURN(Audio::getTotalSoundTime(secondSoundId, secondTotalTime));
		
		int secondCurrentSoundTime = secondTotalTime - static_cast<int>(firstSoundTimePercent * secondTotalTime);
		secondCurrentSoundTime = secondCurrentSoundTime / 2;
		secondCurrentSoundTime = std::min(static_cast<int>(secondTotalTime * 0.75f), secondCurrentSoundTime);
				
		const float fadeInTime = std::min(1.0f, firstSoundTimePercent * 1.5f);
		
		Audio::stopSound(m_idle,       std::max(1.0f, fadeInTime * 2.0f));
		Audio::stopSound(m_run,        std::max(1.0f, fadeInTime * 2.0f));
		Audio::stopSound(firstSoundId, std::max(1.0f, fadeInTime * 2.0f));
		fadeSoundIn(secondSoundId, fadeInTime, secondCurrentSoundTime);
		m_stateCurrent = secondState;
	}
	//-- just force it
	else
	{
		Audio::stopSound(m_idle, 1.0f);
		Audio::stopSound(m_run, 1.0f);
		Audio::stopSound(firstSoundId, 1.0f);
		fadeSoundIn(secondSoundId, 1.0f, 0);
		m_stateCurrent = secondState;
	}
}

//----------------------------------------------------------------------

void VehicleThrusterSoundRuntimeData::update(float elapsedTime, float desiredSpeed)
{
	if (m_fadeInSoundId.isValid())
	{
		if (!m_fadeInLoaded)
		{
			//				s_soundFadeInOffset += elapsedTime;
			
			int total = 0;
			int current = 0;
			
			if (Audio::isSampleForSoundIdPlaying(m_fadeInSoundId) && 
				Audio::getCurrentSoundTime(m_fadeInSoundId, total, current) && total > 0)
			{
				int offset = std::min(total, static_cast<int>(m_fadeInOffset * 1000.0f));
				Audio::setCurrentSoundTime(m_fadeInSoundId, offset);
				m_fadeInLoaded = true;
			}
		}
		
		m_fadeInVolume = std::min(1.0f, m_fadeInVolume +(m_fadeInRate * elapsedTime));
		Audio::setSoundVolume(m_fadeInSoundId, m_fadeInVolume);
		
		if (m_fadeInVolume >= 1.0f)
		{
			m_fadeInSoundId.invalidate();
		}
	}
	
	if (desiredSpeed > m_maximumSpeed * 0.1f)
		m_stateTarget = SS_run;
	else
		m_stateTarget = SS_idle;
	
	if (m_stateTarget == SS_run)
	{
		if (m_stateCurrent == SS_idle)
		{
			transitionToIntermediateState();
		}
		else if (m_stateCurrent == SS_accel)
		{
			transitionToFinalState();
		}
		else if (m_stateCurrent == SS_decel)
		{
			transitionToReverseState();
		}
		else if (!m_run.getPath().isEmpty() && !Audio::isSoundPlaying(m_run))
		{
			fadeSoundIn(m_run, 1.0f, 0);
		}
	}
	//-- transition to idling
	else if (m_stateTarget == SS_idle)
	{
		if (m_stateCurrent == SS_run)
		{
			transitionToIntermediateState();
		}
		else if (m_stateCurrent == SS_decel)
		{
			transitionToFinalState();
		}
		
		else if (m_stateCurrent == SS_accel)
		{	
			transitionToReverseState();
		}
		else if (!m_idle.getPath().isEmpty() && !Audio::isSoundPlaying(m_idle))
		{
			fadeSoundIn(m_idle, 1.0f, 0);
		}
	}
}

//----------------------------------------------------------------------

void VehicleThrusterSoundRuntimeData::fadeSoundIn(SoundId & id, float fadeInTime, int offsetStartMs)
{
	if (id.getPath().isEmpty())
		return;
	
	m_fadeInOffset = offsetStartMs * 0.001f;
	m_fadeInLoaded = false;
	
	m_fadeInVolume = 0.0f;
	
	//-- if the sound is already fading out, start the volume at that fade point rather than resetting to zero and fading in from there
	
	Sound2d * const s2 = dynamic_cast<Sound2d *>(Audio::getSoundById(id));
	if (s2)
	{
		m_fadeInVolume = s2->getFadeoutVolume();
		fadeInTime *=(1.0f - m_fadeInVolume);
	}
	
	Object * const object = m_object.getPointer();
	if (object == NULL)
		return;

	id  = Audio::attachSound(id.getPath().getString(), object, m_hardpointName.empty() ? NULL : m_hardpointName.c_str());
	
	m_fadeInSoundId = id;
	if (fadeInTime > 0.0f)
	{
		m_fadeInRate = RECIP(fadeInTime);
		Audio::setSoundVolume(m_fadeInSoundId, 0.0f);
	}
	else
	{
		m_fadeInRate = 0.0f;
		m_fadeInVolume = 1.0f;
		Audio::setSoundVolume(m_fadeInSoundId, 1.0f);
	}
}

//----------------------------------------------------------------------

void VehicleThrusterSoundRuntimeData::updateSoundData (const VehicleThrusterSoundData & vtsd, bool damage)
{
	m_hardpointName = vtsd.m_hardpointName;

	if (damage)
	{
		updateSoundId (m_idle,         vtsd.m_damageIdle);
		updateSoundId (m_accel,        vtsd.m_damageAccel);
		updateSoundId (m_decel,        vtsd.m_damageDecel);
		updateSoundId (m_run,          vtsd.m_damageRun);
	}
	else
	{
		updateSoundId (m_idle,         vtsd.m_idle);
		updateSoundId (m_accel,        vtsd.m_accel);
		updateSoundId (m_decel,        vtsd.m_decel);
		updateSoundId (m_run,          vtsd.m_run);
	}
}

//----------------------------------------------------------------------

void VehicleThrusterSoundRuntimeData::updateSoundId (SoundId & soundId, const std::string & soundTemplateName)
{
	if (soundId.isValid () && m_fadeInSoundId.getPath () == soundId.getPath ())
	{
		if (!m_fadeInLoaded || !Audio::isSampleForSoundIdPlaying (m_fadeInSoundId))
		{
			const int current = static_cast<int>(m_fadeInOffset * 1000.0f);
			Audio::stopSound (soundId, 3.0f);
			soundId = SoundId (0, soundTemplateName.c_str ());
			fadeSoundIn (soundId, 1.0f, current);
			return;
		}

		int total = 0;
		int current = 0;
		
		if (!Audio::getCurrentSoundTime (m_fadeInSoundId, total, current) || total <= 0)
			current =0;
		
		Audio::stopSound (soundId, 3.0f);
		soundId = SoundId (0, soundTemplateName.c_str ());
		fadeSoundIn (soundId, 1.0f, current);
		return;
	}

	if (Audio::isSampleForSoundIdPlaying (soundId))
	{
		Audio::stopSound (soundId, 3.0f);
		soundId = SoundId (0, soundTemplateName.c_str ());
		fadeSoundIn (soundId, 1.0f, 0);
		return;
	}

	Audio::stopSound (soundId, 2.0f);
	soundId = SoundId (0, soundTemplateName.c_str ());
}

//----------------------------------------------------------------------

void VehicleThrusterSoundRuntimeData::setMaximumSpeed(float maximumSpeed)
{
	m_maximumSpeed = maximumSpeed;
}

//----------------------------------------------------------------------

void VehicleThrusterSoundRuntimeData::setObject(Object * object)
{
	m_object = object;
}

//----------------------------------------------------------------------

Object const * VehicleThrusterSoundRuntimeData::getObject() const
{
	return m_object.getPointer();
}

//----------------------------------------------------------------------

void VehicleThrusterSoundRuntimeData::stopAllSounds()
{
	Audio::stopSound(m_idle, 1.0f);
	Audio::stopSound(m_run, 1.0f);
	Audio::stopSound(m_accel, 1.0f);
	Audio::stopSound(m_decel, 1.0f);
}

//======================================================================
