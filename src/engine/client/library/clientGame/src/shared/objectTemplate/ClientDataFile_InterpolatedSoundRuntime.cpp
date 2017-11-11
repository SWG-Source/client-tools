//======================================================================
//
// ClientDataFile_InterpolatedSoundRuntime.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_InterpolatedSoundRuntime.h"

#include "clientAudio/Audio.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientDataFile_InterpolatedSound.h"
#include "clientGame/ClientObject.h"
#include "sharedObject/Appearance.h"

//======================================================================

InterpolatedSoundRuntime::InterpolatedSoundRuntime() :
m_clientObjectWatcher(new ClientObjectWatcher(NULL)),
m_interpolatedSound(NULL),
m_currentInterpolationLevel(0.0f),
m_soundId(new SoundId),
m_active(true)
{

}

//----------------------------------------------------------------------

InterpolatedSoundRuntime::~InterpolatedSoundRuntime()
{
	setObject(NULL);
	delete m_clientObjectWatcher;
	delete m_soundId;

	m_clientObjectWatcher = NULL;
	m_soundId = NULL;

	delete m_interpolatedSound; //just in case
	m_interpolatedSound = NULL;
}

//----------------------------------------------------------------------

void InterpolatedSoundRuntime::setObject(ClientObject & object, InterpolatedSound const & interpolatedSound)
{
	if (&object == m_clientObjectWatcher->getPointer())
		return;

	if (Audio::isSoundPlaying(*m_soundId))
		Audio::stopSound(*m_soundId, 1.0f);
		
	if (NULL == m_interpolatedSound)
		m_interpolatedSound = new InterpolatedSound;

	*m_clientObjectWatcher = &object;	
	*m_interpolatedSound = interpolatedSound;
	*m_clientObjectWatcher = &object;
}

//----------------------------------------------------------------------

void InterpolatedSoundRuntime::setObject(ClientObject * const object)
{
	if (object == m_clientObjectWatcher->getPointer())
		return;

	delete m_interpolatedSound;
	m_interpolatedSound = NULL;
	*m_clientObjectWatcher = NULL;
	if (Audio::isSoundPlaying(*m_soundId))
		Audio::stopSound(*m_soundId, 1.0f);
	
	ClientDataFile const * const clientDataFile = (object != NULL) ? object->getClientData() : NULL;
	if (clientDataFile == NULL)
		return;
	
	*m_clientObjectWatcher = object;
	
	InterpolatedSound const * const interpolatedSound = clientDataFile->getInterpolatedSound();
	if (interpolatedSound)
	{
		m_interpolatedSound = new InterpolatedSound(*interpolatedSound);
	}
}

//----------------------------------------------------------------------

void InterpolatedSoundRuntime::update(bool const enabled, float const desiredInterpolationLevel, float const elapsedTimeSecs)
{
	ClientObject * const clientObject = m_clientObjectWatcher->getPointer();

	if (clientObject == NULL)
		return;

	NOT_NULL(m_interpolatedSound);

	if (m_currentInterpolationLevel < desiredInterpolationLevel)
	{
		float const interpolationDeltaThisFrame = elapsedTimeSecs * m_interpolatedSound->m_interpolationRateUp;
		m_currentInterpolationLevel = std::min(desiredInterpolationLevel, m_currentInterpolationLevel + interpolationDeltaThisFrame);
	}
	else if (m_currentInterpolationLevel > desiredInterpolationLevel)
	{
		float const interpolationDeltaThisFrame = elapsedTimeSecs * m_interpolatedSound->m_interpolationRateDown;
		m_currentInterpolationLevel = std::max(desiredInterpolationLevel, m_currentInterpolationLevel - interpolationDeltaThisFrame);
	}

	//-- inactive sound overrides the enabled flag passed in
	if (!enabled || !m_active)
	{
		if (m_interpolatedSound->m_interpolationRateDown > 0.0f)
			Audio::stopSound(*m_soundId, RECIP(m_interpolatedSound->m_interpolationRateDown));
		else
			Audio::stopSound(*m_soundId, 2.0f);
	}
	else if (!Audio::isSoundPlaying(*m_soundId))
	{
		Object * objectToAttachTo = clientObject;
		std::string const & hardpointName = m_interpolatedSound->m_hardpointName.c_str();
		TemporaryCrcString const cardpointNameCrcString (hardpointName.c_str(), true);
		if (!hardpointName.empty())
		{
			Transform transform;
			while (objectToAttachTo != NULL)
			{
				Appearance const * const app = objectToAttachTo->getAppearance();
				if (app != NULL)
				{
					if (app->findHardpoint(cardpointNameCrcString, transform))
						break;
				}

				objectToAttachTo = objectToAttachTo->getParent();
			}

			if (objectToAttachTo == NULL)
			{
				objectToAttachTo = clientObject;
				WARNING(true, ("ClientDataFile_InterpolatedSoundRuntime setObject no hardpoint [%s] on object or parents.  [%s]", hardpointName.c_str(), clientObject->getDebugInformation().c_str()));
			}
		}
		*m_soundId = Audio::attachSound(m_interpolatedSound->m_soundTemplateName.c_str(), objectToAttachTo, hardpointName.empty() ? NULL : hardpointName.c_str());
	}

	//-- audio can still be playing if it is fading out to stop
	if (Audio::isSoundPlaying(*m_soundId))
	{
		float const pitchDelta = linearInterpolate(m_interpolatedSound->m_pitchDeltaMin, m_interpolatedSound->m_pitchDeltaMax, m_currentInterpolationLevel);
		float const volume = clamp(0.0f, linearInterpolate(m_interpolatedSound->m_volumeMin, m_interpolatedSound->m_volumeMax, m_currentInterpolationLevel), 1.0f);

		Audio::setSoundPitchDelta(*m_soundId, pitchDelta);
		Audio::setSoundVolume(*m_soundId, volume);
	}
}

//----------------------------------------------------------------------

bool InterpolatedSoundRuntime::isValid() const
{
	return (m_clientObjectWatcher->getPointer() != NULL) && (m_interpolatedSound != NULL);
}

//----------------------------------------------------------------------

bool InterpolatedSoundRuntime::isActive() const
{
	return m_active;
}

//----------------------------------------------------------------------

void InterpolatedSoundRuntime::setActive(bool active)
{
	m_active = active;
}

//======================================================================
