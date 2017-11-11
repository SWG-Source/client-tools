// ============================================================================
//
// Sound2dTemplate.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_Sound2dTemplate_H
#define INCLUDED_Sound2dTemplate_H

#include "clientAudio/Audio.h"
#include "clientAudio/SoundTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

//-----------------------------------------------------------------------------
class Sound2dTemplate : public SoundTemplate
{
public:

	Sound2dTemplate();
	explicit Sound2dTemplate(char const *name);
	virtual ~Sound2dTemplate();

	virtual bool operator ==(Sound2dTemplate const &rhs) const;
	virtual bool operator !=(Sound2dTemplate const &rhs) const;

	virtual Sound2 *      createSound() const;
	static SoundTemplate *create(char const *name, Iff &iff);
	static void           install();
	static void           remove();
	static Tag const &    getTag();

	virtual void          write(Iff &iff, bool const moveToTopOfForm = true) const;
	virtual void          load(Iff &iff);
	virtual void          load(Iff &iff, bool const forceCacheSample);

	void                             setStartDelayMin(float const startDelayMin);
	void                             setStartDelayMax(float const startDelayMax);
	void                             setFadeInMin(float const fadeInMin);
	void                             setFadeInMax(float const fadeInMax);
	void                             setFadeOutMin(float const fadeOutMin);
	void                             setFadeOutMax(float const fadeOutMax);
	void                             setLoopCountMin(int const loopCountMin);
	void                             setLoopCountMax(int const loopCountMax);
	void                             setLoopDelayMin(float const loopDelayMin);
	void                             setLoopDelayMax(float const loopDelayMax);
	void                             setSoundCategory(Audio::SoundCategory const soundCategory);
	void                             setPlayOrder(PlayOrder const playOrder);
	void                             setPlayCount(PlayCount const playCount);
	void                             setLoopDelaySampleRate(LoopDelaySampleRate const loopDelaySampleRate);
	void                             setFadeInSampleRate(FadeInSampleRate const fadeInSampleRate);
	void                             setFadeOutSampleRate(FadeOutSampleRate const fadeOutSampleRate);
	void                             setPitchSampleRateType(PitchSampleRateType const pitchSampleRateType);
	void                             setPitchSampleRate(float const pitchSampleRate);
	void                             setPitchMin(float const pitchMin);
	void                             setPitchMax(float const pitchMax);
	void                             setPitchInterpolationRate(float const pitchInterpolationRate);
	void                             setPriority(int const priority);
	void                             setVolumeSampleRateType(VolumeSampleRateType const volumeSampleRateType);
	void                             setVolumeSampleRate(float const volumeSampleRate);
	void                             setVolumeMin(float const volumeMin);
	void                             setVolumeMax(float const volumeMax);
	void                             setVolumeInterpolationRate(float const volumeInterpolationRate);
	void                             setAttenuationMethod(Audio::AttenuationMethod const attenuationMethod);
	void                             setDistanceAtMaxVolume(float const distanceAtMaxVolume);

	virtual bool                     is2d() const;
	virtual bool                     isInfiniteLooping() const;
	virtual float                    getStartDelayMin() const;
	virtual float                    getStartDelayMax() const;
	virtual float                    getFadeInMin() const;
	virtual float                    getFadeInMax() const;
	virtual float                    getFadeOutMin() const;
	virtual float                    getFadeOutMax() const;
	virtual int                      getLoopCountMin() const;
	virtual int                      getLoopCountMax() const;
	virtual float                    getLoopDelayMin() const;
	virtual float                    getLoopDelayMax() const;
	virtual Audio::SoundCategory     getSoundCategory() const;
	virtual PlayOrder                getPlayOrder() const;
	virtual PlayCount                getPlayCount() const;
	virtual LoopDelaySampleRate      getLoopDelaySampleRate() const;
	virtual FadeInSampleRate         getFadeInSampleRate() const;
	virtual FadeOutSampleRate        getFadeOutSampleRate() const;
	virtual PitchSampleRateType      getPitchSampleRateType() const;
	virtual float                    getPitchSampleRate() const;
	virtual float                    getPitchMin() const;
	virtual float                    getPitchMax() const;
	virtual float                    getPitchInterpolationRate() const;
	virtual int                      getPriority() const;
	virtual VolumeSampleRateType     getVolumeSampleRateType() const;
	virtual float                    getVolumeSampleRate() const;
	virtual float                    getVolumeMin() const;
	virtual float                    getVolumeMax() const;
	virtual float                    getVolumeInterpolationRate() const;
	virtual Audio::AttenuationMethod getAttenuationMethod() const;
	virtual float                    getDistanceAtMaxVolume() const;

protected:

	float                    m_startDelayMin;
	float                    m_startDelayMax;
	float                    m_fadeInMin;
	float                    m_fadeInMax;
	int                      m_loopCountMin;
	int                      m_loopCountMax;
	float                    m_loopDelayMin;
	float                    m_loopDelayMax;
	float                    m_fadeOutMin;
	float                    m_fadeOutMax;
	Audio::SoundCategory     m_soundCategory;
	PlayOrder                m_playOrder;
	PlayCount                m_playCount;
	LoopDelaySampleRate      m_loopDelaySampleRate;
	FadeInSampleRate         m_fadeInSampleRate;
	FadeOutSampleRate        m_fadeOutSampleRate;
	PitchSampleRateType      m_pitchSampleRateType;
	float                    m_pitchSampleRate;
	float                    m_pitchMin;
	float                    m_pitchMax;
	float                    m_pitchInterpolationRate;
	VolumeSampleRateType     m_volumeSampleRateType;
	float                    m_volumeSampleRate;
	float                    m_volumeMin;
	float                    m_volumeMax;
	float                    m_volumeInterpolationRate;
	int                      m_priority;
	Audio::AttenuationMethod m_attenuationMethod;
	float                    m_distanceAtMaxVolume;

private:

	static bool      m_installed;
	static Tag const m_tag;

	void load_0000(Iff &iff, bool const forceCacheSample);
	void load_0001(Iff &iff, bool const forceCacheSample);
	void load_0002(Iff &iff, bool const forceCacheSample);
	void load_0003(Iff &iff, bool const forceCacheSample);

private:

	// Disabled

	Sound2dTemplate(const Sound2dTemplate &);
	Sound2dTemplate &operator =(Sound2dTemplate const &);
};

//=============================================================================

#endif // INCLUDED_Sound2dTemplate_H
