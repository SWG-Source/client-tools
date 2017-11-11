// ============================================================================
//
// Sound2d.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/Sound2d.h"

#include "clientAudio/Audio.h"
#include "clientAudio/Sound2dTemplate.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

#include <string>
#include <vector>

// ============================================================================
//
// Sound2dNamespace
//
// ============================================================================

class IndexPool : public std::vector<int>
{
};

namespace Sound2dNamespace
{
	int s_maxAllocatedSoundCount = 0;

	std::vector< IndexPool* > s_indexPools;

	IndexPool* getIndexPool()
	{
		if (s_indexPools.size() == 0)
		{
			IndexPool* pool = new IndexPool;
			pool->reserve(10);

			s_indexPools.push_back(pool);
		}

		IndexPool* ret = s_indexPools.back();
		s_indexPools.pop_back();
		return ret;
	}

	void releaseIndexPool(IndexPool* buf)
	{
		s_indexPools.push_back(buf);
	}

	void initIndexPools()
	{
		s_indexPools.reserve(10);

		int i;
		for (i = 0; i < 10; ++i)
		{
			IndexPool* pool = new IndexPool;
			pool->reserve(10);

			s_indexPools.push_back(pool);
		}
	}

	void clearIndexPools()
	{
		unsigned int i;
		for (i = 0; i < s_indexPools.size(); ++i)
		{
			delete s_indexPools[i];
		}

		s_indexPools.clear();
	}

}

using namespace Sound2dNamespace;

// ============================================================================
//
// Sound2d
//
// ============================================================================

MemoryBlockManager *Sound2d::m_memoryBlockManager = NULL;

//-----------------------------------------------------------------------------
Sound2d::Sound2d(Sound2dTemplate const *sound2dTemplate, SoundId const &soundId)
 : Sound2(sound2dTemplate, soundId)
 , m_currentSoundTime(0.0f)
 , m_startDelay(0.0f)
 , m_currentStartDelay(0.f)
 , m_fadeInTime(0.0f)
 , m_currentLoop(0)
 , m_fadeOutTime(0.0f)
 , m_templateVolume(0.0f)
 , m_templateVolumeStart(0.0f)
 , m_templateVolumeGoal(0.0f)
 , m_templateVolumeTimer(0.0f)
 , m_pitchHalfStepCurrent(0.0f)
 , m_pitchHalfStepStart(0.0f)
 , m_pitchHalfStepGoal(0.0f)
 , m_pitchTimer(0.0f)
 , m_endOfSample(true)
 , m_fadeOutTimer(0.0f)
 , m_manualFadeOutStarted(false)
 , m_stoppedOutOfRange(false)
 , m_initialPlayBackRate(-1)
 , m_sampleStarted(false)
 , m_indexPool(0)
 , m_indexPoolDepth(0)

#ifdef _DEBUG
 , m_currentSamplePath()
 , m_totalSampleSize(0)
#endif // _DEBUG
{
	DEBUG_WARNING((getTemplate()->getSampleCount() < 0), ("Trying to create a sound with no samples specified."));

#ifdef _DEBUG

	Sound2dTemplate::StringList::const_iterator iterSampleImagePathList = getTemplate()->getSampleList().begin();

	for (; iterSampleImagePathList != getTemplate()->getSampleList().end(); ++iterSampleImagePathList)
	{
		CrcString const *path = (*iterSampleImagePathList);

		if (path != NULL)
		{
			m_totalSampleSize += Audio::getSampleSize(path->getString());
		}
	}
#endif // _DEBUG

	// Loop count

	m_loopCount = Random::random(getTemplate()->getLoopCountMin(), getTemplate()->getLoopCountMax());

	// index pool (for randomNoRepeats)

	int const count = static_cast<int>(getTemplate()->getSampleCount());

	if (count > 1 && getTemplate()->getPlayOrder() == Sound2dTemplate::PO_randomNoRepeat)
	{
		m_indexPool = getIndexPool();
		m_indexPoolDepth = count;

		m_indexPool->resize(count);

		int i;
		for (i = 0; i < count; ++i)
		{
			(*m_indexPool)[i] = i;
		}
	}
}

//-----------------------------------------------------------------------------
Sound2d::~Sound2d()
{
	if (m_indexPool != 0)
	{
		releaseIndexPool(m_indexPool);
	}
}

//-----------------------------------------------------------------------------
void Sound2d::install()
{
	DEBUG_FATAL(m_memoryBlockManager, ("Sound2d::install() - Already installed"));

	char const * const name = "Sound2d::install::m_memoryBlockManager";
	bool const shared = false;
	int const elementSize = sizeof(Sound2d);
	int const elementsPerBlock = 128;
	int const minimumNumberOfBlocks = 1;
	int const maximumNumberOfBlocks = 0;

	m_memoryBlockManager = new MemoryBlockManager(name, shared, elementSize, elementsPerBlock, minimumNumberOfBlocks, maximumNumberOfBlocks);

	initIndexPools();

	ExitChain::add(&remove, "Sound2d::remove()");
}

//-----------------------------------------------------------------------------
void Sound2d::remove()
{
	DEBUG_FATAL(!m_memoryBlockManager, ("Sound2d::remove() - Sound2d is not installed"));

	clearIndexPools();

	delete m_memoryBlockManager;
	m_memoryBlockManager = NULL;

}

//-----------------------------------------------------------------------------
void *Sound2d::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(m_memoryBlockManager);
	DEBUG_FATAL(size != sizeof (Sound2d), ("bad size"));
	DEBUG_FATAL(size != static_cast<size_t> (m_memoryBlockManager->getElementSize()), ("installed with bad size"));

	//DEBUG_REPORT_LOG(true, ("Sound2d: sound newed (%d)\n", m_memoryBlockManager->getElementCount() + 1));

	if (m_memoryBlockManager->getElementCount() + 1 > s_maxAllocatedSoundCount)
	{
		s_maxAllocatedSoundCount = m_memoryBlockManager->getElementCount() + 1;
	}

	return m_memoryBlockManager->allocate();
}

//-----------------------------------------------------------------------------
void Sound2d::operator delete(void *pointer)
{
	NOT_NULL(m_memoryBlockManager);

	m_memoryBlockManager->free(pointer);

	//DEBUG_REPORT_LOG(true, ("Sound2d: sound deleted (%d)\n", m_memoryBlockManager->getElementCount()));
}

//-----------------------------------------------------------------------------
int Sound2d::getCount()
{
	return m_memoryBlockManager->getElementCount();
}

//-----------------------------------------------------------------------------
int Sound2d::getMaxCount()
{
	return s_maxAllocatedSoundCount;
}

//-----------------------------------------------------------------------------
float Sound2d::getDistanceSquaredFromListener() const
{
	float result = 0.0f;

	if (isPositionSpecified())
	{
		result = getPosition_w().magnitudeBetweenSquared(Audio::getListenerPosition());
	}

	return result;
}

//-----------------------------------------------------------------------------
float Sound2d::getDistanceFromListener() const
{
	float result = 0.0f;

	if (isPositionSpecified())
	{
		result = getPosition_w().magnitudeBetween(Audio::getListenerPosition());
	}

	return result;
}

// Volume percent based on distance from listener
//-----------------------------------------------------------------------------
float Sound2d::getAttenuation() const
{
	float result = 1.0f;

	if (getTemplate()->getAttenuationMethod() != Audio::AM_none)
	{
		float const distanceFromListener = getDistanceFromListener();
		float const distanceAtMaxVolume = getTemplate()->getDistanceAtMaxVolume();
		float const distanceAtVolumeCutOff = getDistanceAtVolumeCutOff();

		if (distanceFromListener > distanceAtVolumeCutOff)
		{
			result = 0.0f;
		}
		else if (distanceFromListener > 0.0f)
		{
			if (distanceFromListener < distanceAtMaxVolume)
			{
				result = 1.0f;
			}
			else
			{
				float const soundfadeDistance = distanceAtVolumeCutOff - distanceAtMaxVolume;

				if (soundfadeDistance > 0.0f)
				{
					result = 1.0f - ((distanceFromListener - distanceAtMaxVolume) / soundfadeDistance);
				}
			}
		}
	}

	DEBUG_FATAL((result < 0.0f), ("Sound attenuation (%.2f) must be >= 0", result));
	DEBUG_FATAL((result > 1.0f), ("Sound attenuation (%.2f) must be <= 1", result));

	return clamp(0.0f, result, 1.0f);
}

//-----------------------------------------------------------------------------
float Sound2d::getTemplateVolume() const
{
	return m_templateVolume;
}

//-----------------------------------------------------------------------------
bool Sound2d::isDeletable() const
{
	bool result = false;
	bool const inAudibleRange = getDistanceSquaredFromListener() <= sqr(getDistanceAtVolumeCutOff());

	if (!inAudibleRange && !isInfiniteLooping())
	{
		result = true;
	}
	else
	{
		bool const parentObjectDied = (m_wasParented && (m_object == NULL));
		result = ((parentObjectDied || !isPlaying()) && m_autoDelete);
	}

	return result;
}

//-----------------------------------------------------------------------------
bool Sound2d::isStillLooping() const
{
	int const sampleCount = getTemplate()->getSampleCount();
	float const loopDelayMax = getTemplate()->getLoopDelayMax();
	bool loopingInHardware = (m_currentLoop > 0) && (loopDelayMax <= 0.0f) && (sampleCount <= 1);
	bool const infiniteLooping = isInfiniteLooping();

	return ((m_currentLoop < m_loopCount) && !loopingInHardware) || infiniteLooping;
}

//-----------------------------------------------------------------------------
void Sound2d::alter(float const deltaTime)
{
	if (m_endOfSample)
	{
		reset();
	}

	if (isDeletable())
	{
		return;
	}

	if (m_manualFadeOutStarted)
	{
		// If the manual fade out was started, we may need to stop this sample

		float const fadeOutTime = m_fadeOutTime;
		bool const stoppingOfOutOfRange = false;

		stop(fadeOutTime, stoppingOfOutOfRange);
	}

	if (m_currentStartDelay <= 0.f)
	{
		bool const inAudibleRange = getDistanceSquaredFromListener() <= sqr(getDistanceAtVolumeCutOff());

		if (inAudibleRange)
		{
			bool const stillLooping = isStillLooping();

			if ((!m_sampleStarted || m_stoppedOutOfRange) && stillLooping && !m_manualFadeOutStarted)
			{
				startSample();
			}
			else
			{
				// The sound is currently playing

				if (m_sampleId.isValid())
				{
					// Update the volume

					updateTemplateVolume(deltaTime);
					Audio::setSampleVolume(m_sampleId, getVolume());

					// Update the pitch

					updateTemplatePitch(deltaTime);
					Audio::setSamplePlayBackRate(m_sampleId, m_initialPlayBackRate, getPlayBackRateDelta());
					
					// Calculate the occlusion/obstruction for 3d and 2d attenuated sounds

					if (getTemplate()->getAttenuationMethod() != Audio::AM_none)
					{
						float const deltaTimeScaled = (deltaTime / 3.0f);

						if (isObstructed())
						{
							m_obstructionPercent += (1.0f - m_obstructionPercent) * deltaTimeScaled;

							if (m_obstructionPercent > 0.95f)
							{
								m_obstructionPercent = 1.0f;
							}
						}
						else
						{
							m_obstructionPercent += (0.0f - m_obstructionPercent) * deltaTimeScaled;

							if (m_obstructionPercent < 0.05f)
							{
								m_obstructionPercent = 0.0f;
							}
						}

						if (isOccluded())
						{
							m_occlusionPercent += (1.0f - m_occlusionPercent) * deltaTimeScaled;

							if (m_occlusionPercent > 0.95f)
							{
								m_occlusionPercent = 1.0f;
							}
						}
						else
						{
							m_occlusionPercent += (0.0f - m_occlusionPercent) * deltaTimeScaled;

							if (m_occlusionPercent < 0.05f)
							{
								m_occlusionPercent = 0.0f;
							}
						}

						m_obstructionPercent = clamp(0.0f, m_obstructionPercent, 1.0f);
						m_occlusionPercent = clamp(0.0f, m_occlusionPercent, 1.0f);
					}

					// Set the occlusion/obstruction for 3d sounds

					if (getTemplate()->getAttenuationMethod() == Audio::AM_3d)
					{
						float const obstruction = getObstruction();
						float const occlusion = getOcclusion();

						Audio::setSampleObstruction(m_sampleId, obstruction);
						Audio::setSampleOcclusion(m_sampleId, occlusion);
					}
				}
			}
		}

		// Increase the time this sample has been alive

		m_currentSoundTime += deltaTime;
	}
	else
		m_currentStartDelay -= deltaTime;
}

//-----------------------------------------------------------------------------
void Sound2d::updateTemplatePitch(float const deltaTime)
{
	switch (getTemplate()->getPitchSampleRateType())
	{
		case Sound2dTemplate::PSRT_seconds:
			{
				m_pitchTimer += deltaTime;

				if (m_pitchTimer >= getTemplate()->getPitchSampleRate())
				{
					m_pitchTimer = 0.0f;
					m_pitchHalfStepStart = m_pitchHalfStepCurrent;
					m_pitchHalfStepGoal = Random::randomReal(getTemplate()->getPitchMin(), getTemplate()->getPitchMax());
				}

				float const pitchDifference = m_pitchHalfStepGoal - m_pitchHalfStepStart;

				if (m_pitchHalfStepCurrent != m_pitchHalfStepGoal)
				{
					float const interpolationRate = getTemplate()->getPitchInterpolationRate();

					if (interpolationRate <= 0.0f)
					{
						m_pitchHalfStepCurrent = m_pitchHalfStepGoal;
					}
					else
					{
						float const interpolation = (interpolationRate <= 0.0f) ? 0.0f : (1.0f / interpolationRate);

						m_pitchHalfStepCurrent += pitchDifference * deltaTime * interpolation;

						if (pitchDifference >= 0.0f)
						{
							if (m_pitchHalfStepCurrent > m_pitchHalfStepGoal)
							{
								m_pitchHalfStepCurrent = m_pitchHalfStepGoal;
							}
						}
						else
						{
							if (m_pitchHalfStepCurrent < m_pitchHalfStepGoal)
							{
								m_pitchHalfStepCurrent = m_pitchHalfStepGoal;
							}
						}
					}
				}
			}
			break;
		case Sound2dTemplate::PSRT_basePitch:
		case Sound2dTemplate::PSRT_initial:
		case Sound2dTemplate::PSRT_everySample:
		default:
			{
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void Sound2d::updateTemplateVolume(float const deltaTime)
{
	m_fadeOutTimer -= deltaTime;

	switch (getTemplate()->getVolumeSampleRateType())
	{
		case Sound2dTemplate::VSRT_seconds:
			{
				m_templateVolumeTimer += deltaTime;

				if (m_templateVolumeTimer >= getTemplate()->getVolumeSampleRate())
				{
					m_templateVolumeTimer = 0.0f;
					m_templateVolumeStart = m_templateVolume;
					m_templateVolumeGoal = Random::randomReal(getTemplate()->getVolumeMin(), getTemplate()->getVolumeMax());
				}

				float const volumeDifference = m_templateVolumeGoal - m_templateVolumeStart;

				if (m_templateVolume != m_templateVolumeGoal)
				{
					float const interpolationRate = getTemplate()->getVolumeInterpolationRate();

					if (interpolationRate <= 0.0f)
					{
						m_templateVolume = m_templateVolumeGoal;
					}
					else
					{
						float const interpolation = (interpolationRate <= 0.0f) ? 0.0f : (1.0f / interpolationRate);

						m_templateVolume += volumeDifference * deltaTime * interpolation;

						if (volumeDifference >= 0.0f)
						{
							if (m_templateVolume > m_templateVolumeGoal)
							{
								m_templateVolume = m_templateVolumeGoal;
							}
						}
						else
						{
							if (m_templateVolume < m_templateVolumeGoal)
							{
								m_templateVolume = m_templateVolumeGoal;
							}
						}
					}
				}
			}
			break;
		case Sound2dTemplate::VSRT_fullVolume:
		case Sound2dTemplate::VSRT_initial:
		case Sound2dTemplate::VSRT_everySample:
		default:
			{
				// These cases are handled in reset()
			}
			break;
	}

	// If we are infinite looping, determine when the loops are occuring by checking the sample times
	// because we do not get end of sample callbacks when infinite looping.

	if (getTemplate()->getSampleList().size() == 1)
	{
		float totalSampleTime;
		float currentSampleTime;

		if (getSampleTime(currentSampleTime, totalSampleTime))
		{
			if (currentSampleTime < m_samplePreviousTime)
			{
				// The sound has looped

				++m_currentLoop;
				m_currentSoundTime = 0.0f;

				setupFadeIn();
				setupFadeOut();
				setupVolume();
				setupPitch();
			}

			m_samplePreviousTime = currentSampleTime;
		}
	}

	DEBUG_FATAL((m_templateVolume < 0.0f), ("Sound template volume (%.2f) must be >= 0", m_templateVolume));
	DEBUG_FATAL((m_templateVolume > 1.0f), ("Sound template volume (%.2f) must be <= 1", m_templateVolume));

	m_templateVolume = clamp(0.0f, m_templateVolume, 1.0f);
}

//-----------------------------------------------------------------------------
float Sound2d::getFade()
{
	float result = 1.0f;

	if (m_manualFadeOutStarted)
	{
		if (m_fadeOutTime <= 0.0f)
		{
			result = 0.0f;
		}
		else
		{
			result = m_fadeOutTimer / m_fadeOutTime;

			if (result < 0.0f)
			{
				result = 0.0f;
			}
		}
	}
	else
	{
		float totalSampleTime;
		float currentSampleTime;

		if (getSampleTime(currentSampleTime, totalSampleTime))
		{
			if ((m_fadeInTime + m_fadeOutTime) > totalSampleTime)
			{
				// Fixup the fade times because the total fade in/out time is greater than
				// the length of the sample

				float const totalFadeTime = m_fadeInTime + m_fadeOutTime;
				float const fadeInPercent = (totalFadeTime > 0.0f) ? (m_fadeInTime / totalFadeTime) : 0.0f;
				float const fadeOutPercent = (totalFadeTime > 0.0f) ? (m_fadeOutTime / totalFadeTime) : 0.0f;

				m_fadeInTime = fadeInPercent * totalSampleTime;
				m_fadeOutTime = fadeOutPercent * totalSampleTime;
			}

			float fadeInPercent = 0.0f;
			float fadeOutPercent = 0.0f;

			if (currentSampleTime <= totalSampleTime)
			{
				if ((m_fadeInTime > 0.0f) && (currentSampleTime <= m_fadeInTime))
				{
					fadeInPercent = currentSampleTime / m_fadeInTime;

					result *= fadeInPercent;
				}
				else if (m_fadeOutTime > 0.0f)
				{
					if (m_fadeOutTime >= (totalSampleTime - currentSampleTime))
					{
						fadeOutPercent = (totalSampleTime - currentSampleTime) / m_fadeOutTime;

						result *= fadeOutPercent;
					}
				}
			}
			else
			{
				result = 0.0f;
			}
		}
		else if (m_fadeInTime > 0.0f)
		{
			result = 0.0f;
		}
	}

	DEBUG_FATAL((result < 0.0f), ("Sound fade (%.2f) must be >= 0", result));
	DEBUG_FATAL((result > 1.0f), ("Sound fade (%.2f) must be <= 1", result));

	return clamp(0.0f, result, 1.0f);
}

//-----------------------------------------------------------------------------
float Sound2d::getPlayBackRateDelta() const
{
	float const pitchDelta = getPitchDelta();

	return (pitchDelta == 0.0f) ? 1.0f : powf(2.0f, pitchDelta / 12.0f);
}

//-----------------------------------------------------------------------------
bool Sound2d::isPlaying() const
{
	bool result = true;

	if (!m_stoppedOutOfRange)
	{
		if (m_manualFadeOutStarted)
		{
			bool const sampleValid = Audio::isSampleValid(m_sampleId);

			if (sampleValid)
			{
				float const volume = Audio::getSampleVolume(m_sampleId);

				if (volume <= 0.0f)
				{
					result = false;
				}
			}
			else
			{
				result = false;
			}
		}
		else if (!m_sampleStarted)
		{
			if (!isStillLooping())
			{
				result = false;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void Sound2d::stop(float const fadeOutTime, bool const stoppingOfOutOfRange)
{
	//DEBUG_REPORT_LOG(!stoppingOfOutOfRange, ("Sound2d::stop() - %d\n", m_soundId.getId()));

	// Marking it as out of range makes the audio system unable to delete it, so
	// when it comes back in range we can turn the sound back on

	m_stoppedOutOfRange = stoppingOfOutOfRange;

	if (fadeOutTime <= 0.0f &&
		Audio::isSampleValid(m_sampleId))
	{
		// Kill the sound immediately

		Audio::releaseSampleId(*this);
		m_sampleId.invalidate();
	}
	else if (!m_manualFadeOutStarted)
	{
		// Start the manual fade out

		m_fadeOutTimer = fadeOutTime;
		m_fadeOutTime = fadeOutTime;
	}
	else if (m_manualFadeOutStarted)
	{
		if (m_fadeOutTime > 0.0f)
		{			
			const float fadeProgress = m_fadeOutTimer / m_fadeOutTime;
			m_fadeOutTime  = fadeOutTime;
			m_fadeOutTimer = fadeOutTime * fadeProgress;
		}
		else
		{
			m_fadeOutTime  = fadeOutTime;
			m_fadeOutTimer = fadeOutTime;
		}
	}

	if (!m_stoppedOutOfRange)
	{
		m_manualFadeOutStarted = true;
	}
}

//-----------------------------------------------------------------------------
Sound2dTemplate const *Sound2d::getTemplate() const
{
	NOT_NULL(m_template);

	Sound2dTemplate const *sound2dTemplate = static_cast<Sound2dTemplate const *>(m_template);
	NOT_NULL(sound2dTemplate);

	return sound2dTemplate;
}

// This sets up all the properties for how a new sample plays
//-----------------------------------------------------------------------------
void Sound2d::reset()
{
	m_endOfSample = false;
	m_sampleStarted = false;

	// Release the sample Id from the audio system

	if (m_autoDelete &&
	    Audio::isSampleValid(m_sampleId))
	{
		Audio::releaseSampleId(*this);
		m_sampleId.invalidate();
	}

	// Setup values for the next loop

	if (isStillLooping())
	{
		setupNextSample();
		setupStartDelay();
		setupFadeIn();
		setupFadeOut();
		setupVolume();
		setupPitch();
	}
}

//-----------------------------------------------------------------------------
bool Sound2d::usesSample(CrcString const &path) const
{
	Sound2dTemplate::StringList::const_iterator imageFileNameIter = std::find(getTemplate()->getSampleList().begin(), getTemplate()->getSampleList().end(), &path);

	bool result = (imageFileNameIter != getTemplate()->getSampleList().end());

	return result;
}

//-----------------------------------------------------------------------------
void Sound2d::endOfSample()
{
	//DEBUG_REPORT_LOG(true, ("Sound2d::endOfSample() - %d\n", m_soundId.getId()));

	// You can not call certain Miles functions when in this function, that is
	// why I must release the sample during the next alter

	// Flag that Miles made a end of sample callback.

	m_endOfSample = true;

	// Increment the current loop

	++m_currentLoop;

	// Signal the end of the sample was reached

	if (m_callBack != NULL)
	{
		(*m_callBack)();
	}

	// Reset the fade out timer

	m_fadeOutTimer = 0.0f;

	// Reset the sample time

	m_currentSoundTime = 0.0f;
}

//-----------------------------------------------------------------------------
bool Sound2d::getSampleTime(float &timeCurrent, float &timeTotal)
{
	bool result = true;

	if (Audio::isSampleValid(m_sampleId))
	{
		Audio::getSampleTime(m_sampleId, timeTotal, timeCurrent);

		if (timeCurrent > timeTotal)
		{
			timeCurrent = timeTotal;
		}
	}
	else
	{
		result = false;
		timeTotal = 0.0f;
		timeCurrent = 0.0f;
	}

	return result;
}

//-----------------------------------------------------------------------------
float Sound2d::getCurrentSoundTime() const
{
	return m_currentSoundTime;
}

//-----------------------------------------------------------------------------
float Sound2d::getStartDelay() const
{
	return m_startDelay;
}

// Returns whether the sample was successfully started
//-----------------------------------------------------------------------------
void Sound2d::startSample()
{
	if (m_stoppedOutOfRange)
	{
		reset();
	}

	DEBUG_FATAL(m_sampleId.isValid(), ("The sample id should not be valid"));

	m_obstructionPercent = isObstructed() ? 1.0f : 0.0f;
	m_occlusionPercent = isOccluded() ? 1.0f : 0.0f;

	bool const soundIsAlreadyPlaying = false;
	
	if (Audio::queueSample(*this, soundIsAlreadyPlaying))
	{
		m_stoppedOutOfRange = false;
		m_manualFadeOutStarted = false;
		m_sampleStarted = true;
	}
	else
	{
		endOfSample();
	}

}

//-----------------------------------------------------------------------------
void Sound2d::setupNextSample()
{
	// Find a new sample to play

	int const sampleCount = static_cast<int>(getTemplate()->getSampleCount());

	if (sampleCount <= 1)
	{
		m_sampleIndex = 0;
	}
	else switch(getTemplate()->getPlayOrder())
	{
	case Sound2dTemplate::PO_inOrder: 
		if (++m_sampleIndex >= sampleCount)
		{
			m_sampleIndex = 0;
		}
		break;

	case Sound2dTemplate::PO_random:
		m_sampleIndex = rand() % sampleCount;
		break;

	case Sound2dTemplate::PO_randomNoRepeat:
		{
			int const noRepeatDepth = 1; // TODO: make configurable in template
			int const nextIndex = rand() % m_indexPoolDepth;

			m_sampleIndex = (*m_indexPool)[nextIndex];

			m_indexPool->erase(m_indexPool->begin() + nextIndex);
			m_indexPool->push_back(m_sampleIndex);

			if (m_indexPoolDepth > sampleCount - noRepeatDepth)
			{
				--m_indexPoolDepth;
			}
		}
		break;

	default:
		DEBUG_FATAL(true, ("Unknown sound play order specified."));
	}

#ifdef _DEBUG
	// Current sample path - for debug only

	if (sampleCount > 0)
	{
		CrcString const *path = (getTemplate()->getSampleList())[static_cast<unsigned int>(m_sampleIndex)];
		m_currentSamplePath.set(path->getString(), true);
	}
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
void Sound2d::setupStartDelay()
{
	if (m_currentLoop <= 0)
	{
		m_startDelay = Random::randomReal(getTemplate()->getStartDelayMin(), getTemplate()->getStartDelayMax());
	}
	else
	{
		switch (getTemplate()->getLoopDelaySampleRate())
		{
			case Sound2dTemplate::LDSR_initial:
				{
					if (m_currentLoop <= 1)
					{
						m_startDelay = Random::randomReal(getTemplate()->getLoopDelayMin(), getTemplate()->getLoopDelayMax());
					}
				}
				break;
			case Sound2dTemplate::LDSR_everySample:
				{
					m_startDelay = Random::randomReal(getTemplate()->getLoopDelayMin(), getTemplate()->getLoopDelayMax());
				}
				break;
			case Sound2dTemplate::LDSR_noDelay:
				{
					m_startDelay = 0.0f;
				}
				break;
			default:
				{
					m_startDelay = 0.0f;
					DEBUG_FATAL(true, ("Unknown loop delay sample rate specified."));
				}
				break;
		}
	}

	m_currentStartDelay = m_startDelay;
}

//-----------------------------------------------------------------------------
void Sound2d::setupFadeIn()
{
	switch (getTemplate()->getFadeInSampleRate())
	{
		case Sound2dTemplate::FISR_firstSample:
			{
				if (m_currentLoop <= 0)
				{
					m_fadeInTime = Random::randomReal(getTemplate()->getFadeInMin(), getTemplate()->getFadeInMax());
				}
				else
				{
					m_fadeInTime = 0.0f;
				}
			}
			break;
		case Sound2dTemplate::FISR_everySample:
			{
				m_fadeInTime = Random::randomReal(getTemplate()->getFadeInMin(), getTemplate()->getFadeInMax());
			}
			break;
		case Sound2dTemplate::FISR_noFade:
			{
				m_fadeInTime = 0.0f;
			}
			break;
		default:
			{
				m_fadeInTime = 0.0f;
				DEBUG_FATAL(true, ("Unknown fade-in sample rate specified."));
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void Sound2d::setupFadeOut()
{
	if (!m_manualFadeOutStarted)
	{
		switch (getTemplate()->getFadeOutSampleRate())
		{
			case Sound2dTemplate::FOSR_lastSample:
				{
					if (m_currentLoop == (m_loopCount - 1))
					{
						m_fadeOutTime = Random::randomReal(getTemplate()->getFadeOutMin(), getTemplate()->getFadeOutMax());
					}
					else
					{
						m_fadeOutTime = 0.0f;
					}
				}
				break;
			case Sound2dTemplate::FOSR_everySample:
				{
					m_fadeOutTime = Random::randomReal(getTemplate()->getFadeOutMin(), getTemplate()->getFadeOutMax());
				}
				break;
			case Sound2dTemplate::FOSR_noFade:
				{
					m_fadeOutTime = 0.0f;
				}
				break;
			default:
				{
					m_fadeOutTime = 0.0f;
					DEBUG_FATAL(true, ("Unknown fade-out sample rate specified."));
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void Sound2d::setupVolume()
{
	Sound2dTemplate::VolumeSampleRateType volumeSampleRateType = getTemplate()->getVolumeSampleRateType();

	switch (volumeSampleRateType)
	{
		case Sound2dTemplate::VSRT_initial:
			{
				if (m_currentLoop <= 0)
				{
					m_templateVolume = Random::randomReal(getTemplate()->getVolumeMin(), getTemplate()->getVolumeMax());
				}
			}
			break;
		case Sound2dTemplate::VSRT_everySample:
			{
				m_templateVolume = Random::randomReal(getTemplate()->getVolumeMin(), getTemplate()->getVolumeMax());
			}
			break;
		case Sound2dTemplate::VSRT_seconds:
			{
				if (m_currentLoop <= 0)
				{
					m_templateVolumeStart = Random::randomReal(getTemplate()->getVolumeMin(), getTemplate()->getVolumeMax());
					m_templateVolumeGoal = m_templateVolumeStart;
					m_templateVolume = m_templateVolumeStart;
				}
			}
			break;
		case Sound2dTemplate::VSRT_fullVolume:
			{
				m_templateVolume = 1.0f;
			}
			break;
		default:
			{
				m_templateVolume = 1.0f;
				DEBUG_FATAL(true, ("Unknown volume sample rate specified."));
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void Sound2d::setupPitch()
{
	Sound2dTemplate::PitchSampleRateType pitchSampleRateType = getTemplate()->getPitchSampleRateType();

	m_pitchTimer = 0.0f;

	switch (pitchSampleRateType)
	{
		case Sound2dTemplate::PSRT_initial:
			{
				if (m_currentLoop <= 0)
				{
					m_pitchHalfStepCurrent = Random::randomReal(getTemplate()->getPitchMin(), getTemplate()->getPitchMax());
				}
			}
			break;
		case Sound2dTemplate::PSRT_everySample:
			{
				m_pitchHalfStepCurrent = Random::randomReal(getTemplate()->getPitchMin(), getTemplate()->getPitchMax());
			}
			break;
		case Sound2dTemplate::PSRT_seconds:
			{
				if (m_currentLoop <= 0)
				{
					m_pitchHalfStepStart = Random::randomReal(getTemplate()->getPitchMin(), getTemplate()->getPitchMax());
					m_pitchHalfStepGoal = m_pitchHalfStepStart;
					m_pitchHalfStepCurrent = m_templateVolumeStart;
				}
			}
			break;
		case Sound2dTemplate::PSRT_basePitch:
			{
				m_pitchHalfStepCurrent = 0.0f;
			}
			break;
		default:
			{
				m_pitchHalfStepCurrent = 0.0f;
				DEBUG_FATAL(true, ("Unknown pitch sample rate specified."));
			}
			break;
	}
}

//-----------------------------------------------------------------------------
float Sound2d::getPitchDelta() const
{
	return m_pitchHalfStepCurrent + getUserPitchDelta();
}

//-----------------------------------------------------------------------------
float Sound2d::getDistanceAtVolumeCutOff() const
{
	return Audio::getFallOffDistance(getTemplate()->getDistanceAtMaxVolume());
}

//-----------------------------------------------------------------------------
bool Sound2d::isSampleFinished() const
{
	return m_endOfSample;
}

//-----------------------------------------------------------------------------
void Sound2d::setCurrentTime(int const milliSeconds)
{
	Audio::setSampleCurrentTime(m_sampleId, milliSeconds);

	int const totalTime = getTotalTime();

	if (milliSeconds < totalTime)
	{
		m_endOfSample = false;
	}
}

//-----------------------------------------------------------------------------
int Sound2d::getCurrentTime() const
{
	return Audio::getSampleCurrentTime(m_sampleId);
}

//-----------------------------------------------------------------------------
int Sound2d::getTotalTime() const
{
	return Audio::getSampleTotalTime(m_sampleId);
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
int Sound2d::getTotalSampleSize() const
{
	return m_totalSampleSize;
}

//-----------------------------------------------------------------------------
bool Sound2d::isStreamed() const
{
	return ((getTemplate()->getAttenuationMethod() != Audio::AM_3d) && (Audio::getSampleSize(m_currentSamplePath.getString())) > Audio::getMaxCached2dSampleSize());
}
#endif // _DEBUG

//-----------------------------------------------------------------------------
int Sound2d::getPlayBackRate() const
{
	return m_initialPlayBackRate;
}

//-----------------------------------------------------------------------------
float Sound2d::getVolume()
{
	float result = m_templateVolume;

	result *= getFade();
	result *= getAttenuation();
	result *= getUserVolume();

	// Add in the occlusion which is just software dimming for 2d attenuated sounds,
	// 3d sounds are attenuated in hardware

	if (getTemplate()->getAttenuationMethod() == Audio::AM_2d)
	{
		float const obstruction = getObstruction();

		if (obstruction > 0.0f)
		{
			result *= (1.0f - obstruction);
		}
		else
		{
			float const occlusion = getOcclusion();

			if (occlusion > 0.0f)
			{
				result *= (1.0f - occlusion);
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void Sound2d::setPlayBackRate(int const playBackRate)
{
	m_initialPlayBackRate = playBackRate;
}

//-----------------------------------------------------------------------------
bool Sound2d::is2d() const
{
	return true;
}

//-----------------------------------------------------------------------------
bool Sound2d::is3d() const
{
	return false;
}

// ============================================================================
