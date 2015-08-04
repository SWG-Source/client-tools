
// ============================================================================
//
// Sound2dTemplate.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/Sound2dTemplate.h"

#include "clientAudio/SoundTemplateList.h"
#include "clientAudio/Sound2d.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Tag.h"
#include <vector>
#include <string>

// ============================================================================
//
// Sound2dTemplate
//
// ============================================================================

bool Sound2dTemplate::m_installed = false;
Tag const Sound2dTemplate::m_tag = TAG(S,D,2,D);

//-----------------------------------------------------------------------------
Sound2dTemplate::Sound2dTemplate()
 : SoundTemplate()
 , m_startDelayMin(0.0f)
 , m_startDelayMax(0.0f)
 , m_fadeInMin(0.0f)
 , m_fadeInMax(0.0f)
 , m_loopCountMin(0)
 , m_loopCountMax(0)
 , m_loopDelayMin(0.0f)
 , m_loopDelayMax(0.0f)
 , m_fadeOutMin(0.0f)
 , m_fadeOutMax(0.0f)
 , m_soundCategory(Audio::SC_ambient)
 , m_playOrder(PO_inOrder)
 , m_playCount(PC_oneAtATime)
 , m_loopDelaySampleRate(LDSR_noDelay)
 , m_fadeInSampleRate(FISR_noFade)
 , m_fadeOutSampleRate(FOSR_noFade)
 , m_pitchSampleRateType(PSRT_basePitch)
 , m_pitchSampleRate(0.0f)
 , m_pitchMin(0.0f)
 , m_pitchMax(0.0f)
 , m_pitchInterpolationRate(0.0f)
 , m_volumeSampleRateType(VSRT_fullVolume)
 , m_volumeSampleRate(0.0f)
 , m_volumeMin(0.0f)
 , m_volumeMax(0.0f)
 , m_volumeInterpolationRate(0.0f)
 , m_priority(9)
 , m_attenuationMethod(Audio::AM_none)
 , m_distanceAtMaxVolume(0.0f)
{
}

//-----------------------------------------------------------------------------
Sound2dTemplate::Sound2dTemplate(const char *name)
 : SoundTemplate(name)
 , m_startDelayMin(0.0f)
 , m_startDelayMax(0.0f)
 , m_fadeInMin(0.0f)
 , m_fadeInMax(0.0f)
 , m_loopCountMin(0)
 , m_loopCountMax(0)
 , m_loopDelayMin(0.0f)
 , m_loopDelayMax(0.0f)
 , m_fadeOutMin(0.0f)
 , m_fadeOutMax(0.0f)
 , m_soundCategory(Audio::SC_ambient)
 , m_playOrder(PO_inOrder)
 , m_playCount(PC_oneAtATime)
 , m_loopDelaySampleRate(LDSR_noDelay)
 , m_fadeInSampleRate(FISR_noFade)
 , m_fadeOutSampleRate(FOSR_noFade)
 , m_pitchSampleRateType(PSRT_basePitch)
 , m_pitchSampleRate(0.0f)
 , m_pitchMin(0.0f)
 , m_pitchMax(0.0f)
 , m_pitchInterpolationRate(0.0f)
 , m_volumeSampleRateType(VSRT_fullVolume)
 , m_volumeSampleRate(0.0f)
 , m_volumeMin(0.0f)
 , m_volumeMax(0.0f)
 , m_volumeInterpolationRate(0.0f)
 , m_priority(9)
 , m_attenuationMethod(Audio::AM_none)
 , m_distanceAtMaxVolume(0.0f)
{
}

//-----------------------------------------------------------------------------
Sound2dTemplate::~Sound2dTemplate()
{
}

//-----------------------------------------------------------------------------
Tag const &Sound2dTemplate::getTag()
{
	return m_tag;
}

//-----------------------------------------------------------------------------
Sound2 * Sound2dTemplate::createSound() const
{
	return new Sound2d(this, SoundId(0, getName()));
}

//-----------------------------------------------------------------------------
SoundTemplate * Sound2dTemplate::create(char const *name, Iff &iff)
{
	SoundTemplate *soundTemplate = new Sound2dTemplate(name);
	soundTemplate->load(iff);

	return soundTemplate;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::install()
{
	DEBUG_FATAL(m_installed, ("Sound2dTemplate::install already installed"));
	m_installed = true;

	SoundTemplateList::assignBinding(m_tag, create);
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::remove()
{
	SoundTemplateList::removeBinding(m_tag);
}

//-----------------------------------------------------------------------------
bool Sound2dTemplate::operator !=(Sound2dTemplate const &rhs) const
{
	return !operator==(rhs);
}

//-----------------------------------------------------------------------------
bool Sound2dTemplate::operator ==(Sound2dTemplate const &rhs) const
{
	bool result = false;

	if (this == &rhs)
	{
		result = true;
	}
	else
	{
		if ((m_startDelayMin == rhs.m_startDelayMin) &&
			(m_startDelayMax == rhs.m_startDelayMax) &&
			(m_fadeInMin == rhs.m_fadeInMin) &&
			(m_fadeInMax == rhs.m_fadeInMax) &&
			(m_loopCountMin == rhs.m_loopCountMin) &&
			(m_loopCountMax == rhs.m_loopCountMax) &&
			(m_loopDelayMin == rhs.m_loopDelayMin) &&
			(m_loopDelayMax == rhs.m_loopDelayMax) &&
			(m_fadeOutMin == rhs.m_fadeOutMin) &&
			(m_fadeOutMax == rhs.m_fadeOutMax) &&
			(m_soundCategory == rhs.m_soundCategory) &&
			(m_playOrder == rhs.m_playOrder) &&
			(m_playCount == rhs.m_playCount) &&
			(m_loopDelaySampleRate == rhs.m_loopDelaySampleRate) &&
			(m_fadeInSampleRate == rhs.m_fadeInSampleRate) &&
			(m_fadeOutSampleRate == rhs.m_fadeOutSampleRate) &&
			(m_pitchSampleRateType == rhs.m_pitchSampleRateType) &&
			(m_pitchSampleRate == rhs.m_pitchSampleRate) &&
			(m_pitchMin == rhs.m_pitchMin) &&
			(m_pitchMax == rhs.m_pitchMax) &&
			(m_pitchInterpolationRate == rhs.m_pitchInterpolationRate) &&
			(m_priority == rhs.m_priority) &&
			(m_volumeSampleRateType == rhs.m_volumeSampleRateType) &&
			(m_volumeSampleRate == rhs.m_volumeSampleRate) &&
			(m_volumeMin == rhs.m_volumeMin) &&
			(m_volumeMax == rhs.m_volumeMax) &&
			(m_volumeInterpolationRate == rhs.m_volumeInterpolationRate) &&
			(m_attenuationMethod == rhs.m_attenuationMethod) &&
			(m_distanceAtMaxVolume == rhs.m_distanceAtMaxVolume))
		{
			result = SoundTemplate::operator==(rhs);
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
bool SoundTemplate::operator !=(SoundTemplate const &rhs) const
{
	return !operator==(rhs);
}

//-----------------------------------------------------------------------------
bool Sound2dTemplate::is2d() const
{
	return true;
}

//-----------------------------------------------------------------------------
bool Sound2dTemplate::isInfiniteLooping() const
{
	return (m_loopCountMin == -1) || (m_loopCountMax == -1);
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setStartDelayMin(float const startDelayMin)
{
	m_startDelayMin = startDelayMin;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setStartDelayMax(float const startDelayMax)
{
	m_startDelayMax = startDelayMax;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setFadeInMin(float const fadeInMin)
{
	m_fadeInMin = fadeInMin;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setFadeInMax(float const fadeInMax)
{
	m_fadeInMax = fadeInMax;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setFadeOutMin(float const fadeOutMin)
{
	m_fadeOutMin = fadeOutMin;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setFadeOutMax(float const fadeOutMax)
{
	m_fadeOutMax = fadeOutMax;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setLoopCountMin(int const loopCountMin)
{
	m_loopCountMin = loopCountMin;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setLoopCountMax(int const loopCountMax)
{
	m_loopCountMax = loopCountMax;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setLoopDelayMin(float const loopDelayMin)
{
	m_loopDelayMin = loopDelayMin;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setLoopDelayMax(float const loopDelayMax)
{
	m_loopDelayMax = loopDelayMax;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setSoundCategory(Audio::SoundCategory const soundCategory)
{
	m_soundCategory = soundCategory;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setPlayOrder(PlayOrder const playOrder)
{
	m_playOrder = playOrder;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setPlayCount(PlayCount const playCount)
{
	m_playCount = playCount;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setLoopDelaySampleRate(LoopDelaySampleRate const loopDelaySampleRate)
{
	m_loopDelaySampleRate = loopDelaySampleRate;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setFadeInSampleRate(FadeInSampleRate const fadeInSampleRate)
{
	m_fadeInSampleRate = fadeInSampleRate;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setFadeOutSampleRate(FadeOutSampleRate const fadeOutSampleRate)
{
	m_fadeOutSampleRate = fadeOutSampleRate;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setPitchSampleRateType(PitchSampleRateType const pitchSampleRateType)
{
	m_pitchSampleRateType = pitchSampleRateType;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setPitchSampleRate(float const pitchSampleRate)
{
	m_pitchSampleRate = pitchSampleRate;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setPitchMin(float const pitchMin)
{
	m_pitchMin = pitchMin;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setPitchMax(float const pitchMax)
{
	m_pitchMax = pitchMax;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setPitchInterpolationRate(float const pitchInterpolationRate)
{
	m_pitchInterpolationRate = pitchInterpolationRate;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setPriority(int const priority)
{
	m_priority = priority;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setVolumeSampleRateType(VolumeSampleRateType const volumeSampleRateType)
{
	m_volumeSampleRateType = volumeSampleRateType;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setVolumeSampleRate(float const volumeSampleRate)
{
	m_volumeSampleRate = volumeSampleRate;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setVolumeMin(float const volumeMin)
{
	m_volumeMin = volumeMin;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setVolumeMax(float const volumeMax)
{
	m_volumeMax = volumeMax;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setVolumeInterpolationRate(float const volumeInterpolationRate)
{
	m_volumeInterpolationRate = volumeInterpolationRate;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setAttenuationMethod(Audio::AttenuationMethod const attenuationMethod)
{
	m_attenuationMethod = attenuationMethod;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::setDistanceAtMaxVolume(float const distanceAtMaxVolume)
{
	m_distanceAtMaxVolume = distanceAtMaxVolume;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getDistanceAtMaxVolume() const
{
	float result = m_distanceAtMaxVolume;

	if (result < 0.0f)
	{
		result = 0.0f;
	}

	return result;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getStartDelayMin() const
{
	return m_startDelayMin;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getStartDelayMax() const
{
	return m_startDelayMax;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getFadeInMin() const
{
	return m_fadeInMin;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getFadeInMax() const
{
	return m_fadeInMax;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getFadeOutMin() const
{
	return m_fadeOutMin;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getFadeOutMax() const
{
	return m_fadeOutMax;
}

//-----------------------------------------------------------------------------
int Sound2dTemplate::getLoopCountMin() const
{
	return m_loopCountMin;
}

//-----------------------------------------------------------------------------
int Sound2dTemplate::getLoopCountMax() const
{
	return m_loopCountMax;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getLoopDelayMin() const
{
	return m_loopDelayMin;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getLoopDelayMax() const
{
	return m_loopDelayMax;
}

//-----------------------------------------------------------------------------
Audio::SoundCategory Sound2dTemplate::getSoundCategory() const
{
	return m_soundCategory;
}

//-----------------------------------------------------------------------------
SoundTemplate::PlayOrder Sound2dTemplate::getPlayOrder() const
{
	return m_playOrder;
}

//-----------------------------------------------------------------------------
SoundTemplate::PlayCount Sound2dTemplate::getPlayCount() const
{
	return m_playCount;
}

//-----------------------------------------------------------------------------
SoundTemplate::LoopDelaySampleRate Sound2dTemplate::getLoopDelaySampleRate() const
{
	return m_loopDelaySampleRate;
}

//-----------------------------------------------------------------------------
SoundTemplate::FadeInSampleRate Sound2dTemplate::getFadeInSampleRate() const
{
	return m_fadeInSampleRate;
}

//-----------------------------------------------------------------------------
SoundTemplate::FadeOutSampleRate Sound2dTemplate::getFadeOutSampleRate() const
{
	return m_fadeOutSampleRate;
}

//-----------------------------------------------------------------------------
SoundTemplate::PitchSampleRateType Sound2dTemplate::getPitchSampleRateType() const
{
	return m_pitchSampleRateType;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getPitchSampleRate() const
{
	return m_pitchSampleRate;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getPitchMin() const
{
	return m_pitchMin;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getPitchMax() const
{
	return m_pitchMax;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getPitchInterpolationRate() const
{
	return m_pitchInterpolationRate;
}

//-----------------------------------------------------------------------------
int Sound2dTemplate::getPriority() const
{
	return m_priority;
}

//-----------------------------------------------------------------------------
SoundTemplate::VolumeSampleRateType Sound2dTemplate::getVolumeSampleRateType() const
{
	return m_volumeSampleRateType;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getVolumeSampleRate() const
{
	return m_volumeSampleRate;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getVolumeMin() const
{
	return m_volumeMin;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getVolumeMax() const
{
	return m_volumeMax;
}

//-----------------------------------------------------------------------------
float Sound2dTemplate::getVolumeInterpolationRate() const
{
	return m_volumeInterpolationRate;
}

//-----------------------------------------------------------------------------
Audio::AttenuationMethod Sound2dTemplate::getAttenuationMethod() const
{
	return m_attenuationMethod;
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::load(Iff &iff)
{
	bool const forceCacheSample = false;

	load(iff, forceCacheSample);
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::load(Iff &iff, bool const forceCacheSample)
{
	iff.enterForm(m_tag);
	{
		switch (iff.getCurrentName())
		{
			case TAG_0000:
				{
					load_0000(iff, forceCacheSample);
					break;
				}
			case TAG_0001:
				{
					load_0001(iff, forceCacheSample);
					break;
				}
			case TAG_0002:
				{
					load_0002(iff, forceCacheSample);
					break;
				}
			case TAG_0003:
				{
					load_0003(iff, forceCacheSample);
					break;
				}
			default:
				{
					char currentTagName[256];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
		
					FATAL(true, ("Sound2dTemplate::load() - Unsupported data version: %s", currentTagName));
				}
		}
	}
	iff.exitForm(m_tag);

	// Sound priority re-organization

	if (m_soundCategory == Audio::SC_ambient)
	{
		m_priority = 5;
	}
	else if (m_soundCategory == Audio::SC_playerMusic)
	{
		m_priority = 3;
	}

	// Cache all the sounds this sound template references

	if (m_distanceAtMaxVolume > 0.0f)
	{
		m_attenuationMethod = Audio::AM_2d;
	}

	if ((getAttenuationMethod() != Audio::AM_none) &&
	    (m_distanceAtMaxVolume <= 2.f))
	{
		m_distanceAtMaxVolume = 2.f;
		DEBUG_REPORT_LOG(true, ("The sound template distance min distance <= 2: %s\n", getName()));
	}

#ifdef _DEBUG
	if (DataLint::isEnabled())
	{
		if (strstr(getName(), "player_music") != NULL)
		{
			DEBUG_WARNING((m_attenuationMethod != Audio::AM_2d), ("The sound template is for player music and should be 2D attenuated."));
			DEBUG_WARNING((m_loopCountMin == -1), ("The sound template is for player music and should not be infinite looping."));
		}

		// Check if "_lp" is in the name but the sound is not looping

		DEBUG_WARNING((strstr(getName(), "_lp") != NULL) && (m_loopCountMin != -1), ("The sound template name suggests infinite looping, but the sound template is not infinite looping."));

		// Validate the sound template

		StringList::iterator iterStringList = m_samplePathList->begin();

		for (; iterStringList != m_samplePathList->end(); ++iterStringList)
		{
			CrcString const *path = (*iterStringList);

			DEBUG_WARNING((Audio::getSampleSize(path->getString()) <= 0), ("Sample size is zero. This sample needs to be replaced: %s", path->getString()));
			DEBUG_WARNING((strstr(path->getString(), ".wav") != NULL) && (m_loopCountMin == 0) && (m_loopCountMax == 0) && (Audio::getSampleSize(path->getString()) > 512 * 1024), ("This is a non-looping WAV file that is large, should it really be an MP3? %s %d KB", path->getString(), Audio::getSampleSize(path->getString()) / 1024));
		}

		DEBUG_WARNING((m_startDelayMin > m_startDelayMax), ("m_startDelayMin(%f) > m_startDelayMax(%f)", m_startDelayMin, m_startDelayMax));
		DEBUG_WARNING((m_fadeInMin > m_fadeInMax), ("m_fadeInMin(%f) > m_fadeInMax(%f)", m_fadeInMin, m_fadeInMax));
		DEBUG_WARNING((m_loopCountMin > m_loopCountMax), ("m_loopCountMin(%d) > m_loopCountMax(%d)", m_loopCountMin, m_loopCountMax));
		DEBUG_WARNING((m_loopDelayMin > m_loopDelayMax), ("m_loopDelayMin(%f) > m_loopDelayMax(%f)", m_loopDelayMin, m_loopDelayMax));
		DEBUG_WARNING((m_fadeOutMin > m_fadeOutMax), ("m_fadeOutMin(%f) > m_fadeOutMax(%f)", m_fadeOutMin, m_fadeOutMax));
		DEBUG_WARNING((m_volumeMin > m_volumeMax), ("m_volumeMin(%f) > m_volumeMax(%f)", m_volumeMin, m_volumeMax));
		DEBUG_WARNING((m_pitchMin > m_pitchMax), ("m_pitchMin(%f) > m_pitchMax(%f)", m_pitchMin, m_pitchMax));
		DEBUG_WARNING((m_attenuationMethod != Audio::AM_none) && (m_distanceAtMaxVolume <= 0.0f), ("Distance at max volume (%f) for attenuated sounds must be > 0", m_distanceAtMaxVolume));
	}

	// Sanity checking

	if (m_startDelayMin > m_startDelayMax)
	{
		std::swap(m_startDelayMin, m_startDelayMax);
	}

	if (m_fadeInMin > m_fadeInMax)
	{
		std::swap(m_fadeInMin, m_fadeInMax);
	}

	if (m_loopCountMin > m_loopCountMax)
	{
		std::swap(m_loopCountMin, m_loopCountMax);
	}

	if (m_loopDelayMin > m_loopDelayMax)
	{
		std::swap(m_loopDelayMin, m_loopDelayMax);
	}

	if (m_fadeOutMin > m_fadeOutMax)
	{
		std::swap(m_fadeOutMin, m_fadeOutMax);
	}

	if (m_volumeMin > m_volumeMax)
	{
		std::swap(m_volumeMin, m_volumeMax);
	}
	
	if (m_pitchMin > m_pitchMax)
	{
		std::swap(m_pitchMin, m_pitchMax);
	}

	// Force player music template to not be infinite looping, infinite looping samples breaks
	// the player music logic

	if (strstr(getName(), "player_music") != NULL)
	{
		m_loopCountMin = 1;
		m_loopCountMax = 1;
	}
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::load_0000(Iff &iff, bool const forceCacheSample)
{
	iff.enterChunk(TAG_0000);
	{
		int fileNameCount = iff.read_int32();

		m_samplePathList->reserve(static_cast<unsigned int>(fileNameCount));
		m_samplePathList->clear();

		char path[MAX_PATH];

		for (int i = 0; i < fileNameCount; ++i)
		{
			iff.read_string(path, sizeof(path));

			addSample(path, forceCacheSample);
		}

		m_startDelayMin = iff.read_float();
		m_startDelayMax = iff.read_float();
		m_fadeInMin = iff.read_float();
		m_fadeInMax = iff.read_float();
		m_loopCountMin = iff.read_int32();
		m_loopCountMax = iff.read_int32();

		if (m_loopCountMin > -1)
		{
			++m_loopCountMin;
			++m_loopCountMax;
		}

		m_loopDelayMin = iff.read_float();
		m_loopDelayMax = iff.read_float();
		m_fadeOutMin = iff.read_float();
		m_fadeOutMax = iff.read_float();
		m_soundCategory = static_cast<Audio::SoundCategory>(iff.read_int32());
		m_playOrder = static_cast<PlayOrder>(iff.read_int32());
		m_playCount = static_cast<PlayCount>(iff.read_int32());
		m_loopDelaySampleRate = static_cast<LoopDelaySampleRate>(iff.read_int32());
		m_fadeInSampleRate = static_cast<FadeInSampleRate>(iff.read_int32());
		m_fadeOutSampleRate = static_cast<FadeOutSampleRate>(iff.read_int32());
		m_volumeSampleRateType = static_cast<VolumeSampleRateType>(iff.read_int32());
		m_volumeSampleRate = iff.read_float();
		m_volumeMin = iff.read_float();
		m_volumeMax = iff.read_float();
		m_volumeInterpolationRate = iff.read_float();
		m_pitchSampleRateType = static_cast<PitchSampleRateType>(iff.read_int32());
		m_pitchSampleRate = iff.read_float();
		m_pitchMin = iff.read_float();
		m_pitchMax = iff.read_float();
		m_pitchInterpolationRate = iff.read_float();
		m_priority = iff.read_int32();
	}
	iff.exitChunk();
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::load_0001(Iff &iff, bool const forceCacheSample)
{
	iff.enterChunk(TAG_0001);
	{
		int fileNameCount = iff.read_int32();

		m_samplePathList->reserve(static_cast<unsigned int>(fileNameCount));
		m_samplePathList->clear();

		char path[MAX_PATH];

		for (int i = 0; i < fileNameCount; ++i)
		{
			iff.read_string(path, sizeof(path));

			addSample(path, forceCacheSample);
		}

		m_startDelayMin = iff.read_float();
		m_startDelayMax = iff.read_float();
		m_fadeInMin = iff.read_float();
		m_fadeInMax = iff.read_float();
		m_loopCountMin = iff.read_int32();
		m_loopCountMax = iff.read_int32();

		if (m_loopCountMin > -1)
		{
			++m_loopCountMin;
			++m_loopCountMax;
		}

		m_loopDelayMin = iff.read_float();
		m_loopDelayMax = iff.read_float();
		m_fadeOutMin = iff.read_float();
		m_fadeOutMax = iff.read_float();
		m_soundCategory = static_cast<Audio::SoundCategory>(iff.read_int32());
		m_playOrder = static_cast<PlayOrder>(iff.read_int32());
		m_playCount = static_cast<PlayCount>(iff.read_int32());
		m_loopDelaySampleRate = static_cast<LoopDelaySampleRate>(iff.read_int32());
		m_fadeInSampleRate = static_cast<FadeInSampleRate>(iff.read_int32());
		m_fadeOutSampleRate = static_cast<FadeOutSampleRate>(iff.read_int32());
		m_volumeSampleRateType = static_cast<VolumeSampleRateType>(iff.read_int32());
		m_volumeSampleRate = iff.read_float();
		m_volumeMin = iff.read_float();
		m_volumeMax = iff.read_float();
		m_volumeInterpolationRate = iff.read_float();
		m_pitchSampleRateType = static_cast<PitchSampleRateType>(iff.read_int32());
		m_pitchSampleRate = iff.read_float();
		m_pitchMin = iff.read_float();
		m_pitchMax = iff.read_float();
		m_pitchInterpolationRate = iff.read_float();
		m_priority = iff.read_int32();
		m_attenuationMethod = static_cast<Audio::AttenuationMethod>(iff.read_int32());
		m_distanceAtMaxVolume = iff.read_float();
	}
	iff.exitChunk();
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::load_0002(Iff &iff, bool const forceCacheSample)
{
	iff.enterChunk(TAG_0002);
	{
		int fileNameCount = iff.read_int32();

		m_samplePathList->reserve(static_cast<unsigned int>(fileNameCount));
		m_samplePathList->clear();

		char path[MAX_PATH];

		for (int i = 0; i < fileNameCount; ++i)
		{
			iff.read_string(path, sizeof(path));

			addSample(path, forceCacheSample);
		}

		m_startDelayMin = iff.read_float();
		m_startDelayMax = iff.read_float();
		m_fadeInMin = iff.read_float();
		m_fadeInMax = iff.read_float();
		m_loopCountMin = iff.read_int32();
		m_loopCountMax = iff.read_int32();

		if (m_loopCountMin > -1)
		{
			++m_loopCountMin;
			++m_loopCountMax;
		}

		m_loopDelayMin = iff.read_float();
		m_loopDelayMax = iff.read_float();
		m_fadeOutMin = iff.read_float();
		m_fadeOutMax = iff.read_float();
		m_soundCategory = static_cast<Audio::SoundCategory>(iff.read_int32());
		m_playOrder = static_cast<PlayOrder>(iff.read_int32());
		m_playCount = static_cast<PlayCount>(iff.read_int32());
		m_loopDelaySampleRate = static_cast<LoopDelaySampleRate>(iff.read_int32());
		m_fadeInSampleRate = static_cast<FadeInSampleRate>(iff.read_int32());
		m_fadeOutSampleRate = static_cast<FadeOutSampleRate>(iff.read_int32());
		m_volumeSampleRateType = static_cast<VolumeSampleRateType>(iff.read_int32());
		m_volumeSampleRate = iff.read_float();
		m_volumeMin = iff.read_float();
		m_volumeMax = iff.read_float();
		m_volumeInterpolationRate = iff.read_float();
		m_pitchSampleRateType = static_cast<PitchSampleRateType>(iff.read_int32());
		m_pitchSampleRate = iff.read_float();
		m_pitchMin = iff.read_float();
		m_pitchMax = iff.read_float();
		m_pitchInterpolationRate = iff.read_float();
		m_priority = iff.read_int32();
		m_distanceAtMaxVolume = iff.read_float();
	}
	iff.exitChunk();
}

//-----------------------------------------------------------------------------
void Sound2dTemplate::load_0003(Iff &iff, bool const forceCacheSample)
{
	iff.enterChunk(TAG_0003);
	{
		int fileNameCount = iff.read_int32();

		m_samplePathList->reserve(static_cast<unsigned int>(fileNameCount));
		m_samplePathList->clear();

		char path[MAX_PATH];

		for (int i = 0; i < fileNameCount; ++i)
		{
			iff.read_string(path, sizeof(path));

			addSample(path, forceCacheSample);
		}

		m_startDelayMin = iff.read_float();
		m_startDelayMax = iff.read_float();
		m_fadeInMin = iff.read_float();
		m_fadeInMax = iff.read_float();
		m_loopCountMin = iff.read_int32();
		m_loopCountMax = iff.read_int32();
		m_loopDelayMin = iff.read_float();
		m_loopDelayMax = iff.read_float();
		m_fadeOutMin = iff.read_float();
		m_fadeOutMax = iff.read_float();
		m_soundCategory = static_cast<Audio::SoundCategory>(iff.read_int32());
		m_playOrder = static_cast<PlayOrder>(iff.read_int32());
		m_playCount = static_cast<PlayCount>(iff.read_int32());
		m_loopDelaySampleRate = static_cast<LoopDelaySampleRate>(iff.read_int32());
		m_fadeInSampleRate = static_cast<FadeInSampleRate>(iff.read_int32());
		m_fadeOutSampleRate = static_cast<FadeOutSampleRate>(iff.read_int32());
		m_volumeSampleRateType = static_cast<VolumeSampleRateType>(iff.read_int32());
		m_volumeSampleRate = iff.read_float();
		m_volumeMin = iff.read_float();
		m_volumeMax = iff.read_float();
		m_volumeInterpolationRate = iff.read_float();
		m_pitchSampleRateType = static_cast<PitchSampleRateType>(iff.read_int32());
		m_pitchSampleRate = iff.read_float();
		m_pitchMin = iff.read_float();
		m_pitchMax = iff.read_float();
		m_pitchInterpolationRate = iff.read_float();
		m_priority = iff.read_int32();
		m_distanceAtMaxVolume = iff.read_float();
	}
	iff.exitChunk();
}

//--------------------------------------------------------------------------
void Sound2dTemplate::write(Iff &iff, bool const moveToTopOfForm) const
{
	iff.insertForm(m_tag);
	{
		iff.insertChunk(TAG_0003);
		{
			iff.insertChunkData(static_cast<int32>(m_samplePathList->size()));

			StringList::const_iterator iterSampleImagePathList = m_samplePathList->begin();

			for (; iterSampleImagePathList != m_samplePathList->end(); ++iterSampleImagePathList)
			{
				CrcString const *path = (*iterSampleImagePathList);

				iff.insertChunkString(path->getString());
			}

			iff.insertChunkData(m_startDelayMin);
			iff.insertChunkData(m_startDelayMax);
			iff.insertChunkData(m_fadeInMin);
			iff.insertChunkData(m_fadeInMax);
			iff.insertChunkData(m_loopCountMin);
			iff.insertChunkData(m_loopCountMax);
			iff.insertChunkData(m_loopDelayMin);
			iff.insertChunkData(m_loopDelayMax);
			iff.insertChunkData(m_fadeOutMin);
			iff.insertChunkData(m_fadeOutMax);
			iff.insertChunkData(static_cast<int32>(m_soundCategory));
			iff.insertChunkData(static_cast<int32>(m_playOrder));
			iff.insertChunkData(static_cast<int32>(m_playCount));
			iff.insertChunkData(static_cast<int32>(m_loopDelaySampleRate));
			iff.insertChunkData(static_cast<int32>(m_fadeInSampleRate));
			iff.insertChunkData(static_cast<int32>(m_fadeOutSampleRate));
			iff.insertChunkData(static_cast<int32>(m_volumeSampleRateType));
			iff.insertChunkData(m_volumeSampleRate);
			iff.insertChunkData(m_volumeMin);
			iff.insertChunkData(m_volumeMax);
			iff.insertChunkData(m_volumeInterpolationRate);
			iff.insertChunkData(static_cast<int32>(m_pitchSampleRateType));
			iff.insertChunkData(m_pitchSampleRate);
			iff.insertChunkData(m_pitchMin);
			iff.insertChunkData(m_pitchMax);
			iff.insertChunkData(m_pitchInterpolationRate);
			iff.insertChunkData(m_priority);
			iff.insertChunkData(m_distanceAtMaxVolume);
		}
		iff.exitChunk();
	}
	iff.exitForm(m_tag);

	// Move back to the top of the form

	if (moveToTopOfForm)
	{
		iff.allowNonlinearFunctions();
		IGNORE_RETURN(iff.seek(m_tag));
		iff.goToTopOfForm();
	}
}

// ============================================================================
