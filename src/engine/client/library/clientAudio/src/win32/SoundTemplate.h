// ============================================================================
//
// SoundTemplate.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef SOUNDTEMPLATE_H
#define SOUNDTEMPLATE_H

#include "clientAudio/Audio.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"

class Iff;
class Sound2;

//-----------------------------------------------------------------------------
class SoundTemplate
{
friend class SoundTemplateList;

public:

	enum PlayOrder
	{
		PO_random,
		PO_inOrder,
		PO_randomNoRepeat
	};

	enum PlayCount
	{
		PC_oneAtATime,
		PC_allAtOnce
	};

	enum LoopDelaySampleRate
	{
		LDSR_noDelay,
		LDSR_initial,
		LDSR_everySample
	};

	enum FadeInSampleRate
	{
		FISR_noFade,
		FISR_firstSample,
		FISR_everySample
	};

	enum FadeOutSampleRate
	{
		FOSR_noFade,
		FOSR_lastSample,
		FOSR_everySample
	};

	enum VolumeSampleRateType
	{
		VSRT_fullVolume,
		VSRT_initial,
		VSRT_everySample,
		VSRT_seconds
	};

	enum PitchSampleRateType
	{
		PSRT_basePitch,
		PSRT_initial,
		PSRT_everySample,
		PSRT_seconds
	};

public:

	typedef stdvector<CrcString const *>::fwd StringList;

	SoundTemplate();
	explicit SoundTemplate(char const *name);
	virtual ~SoundTemplate() = 0;

	virtual bool operator ==(SoundTemplate const &rhs) const;
	virtual bool operator !=(SoundTemplate const &rhs) const;

	int                        getReferenceCount() const;
	PersistentCrcString const &getCrcName() const;
	char const *               getName() const;
	StringList const &         getSampleList() const;

	void                       addSample(char const *path, bool const forceCacheSample);
	void                       clearSampleList();
	int                        getSampleCount() const;

	virtual Sound2 *           createSound() const = 0;
	virtual void               load(Iff &iff) = 0;
	virtual void               write(Iff &iff, bool const moveToTopOfForm) const = 0;

	virtual bool               is2d() const;
	virtual bool               is3d() const;
	virtual bool               isInfiniteLooping() const = 0;

	virtual float                    getStartDelayMin() const = 0;
	virtual float                    getStartDelayMax() const = 0;
	virtual float                    getFadeInMin() const = 0;
	virtual float                    getFadeInMax() const = 0;
	virtual float                    getFadeOutMin() const = 0;
	virtual float                    getFadeOutMax() const = 0;
	virtual int                      getLoopCountMin() const = 0;
	virtual int                      getLoopCountMax() const = 0;
	virtual float                    getLoopDelayMin() const = 0;
	virtual float                    getLoopDelayMax() const = 0;
	virtual Audio::SoundCategory     getSoundCategory() const = 0;
	virtual PlayOrder                getPlayOrder() const = 0;
	virtual PlayCount                getPlayCount() const = 0;
	virtual LoopDelaySampleRate      getLoopDelaySampleRate() const = 0;
	virtual FadeInSampleRate         getFadeInSampleRate() const = 0;
	virtual FadeOutSampleRate        getFadeOutSampleRate() const = 0;
	virtual PitchSampleRateType      getPitchSampleRateType() const = 0;
	virtual float                    getPitchSampleRate() const = 0;
	virtual float                    getPitchMin() const = 0;
	virtual float                    getPitchMax() const = 0;
	virtual float                    getPitchInterpolationRate() const = 0;
	virtual int                      getPriority() const = 0;
	virtual VolumeSampleRateType     getVolumeSampleRateType() const = 0;
	virtual float                    getVolumeSampleRate() const = 0;
	virtual float                    getVolumeMin() const = 0;
	virtual float                    getVolumeMax() const = 0;
	virtual float                    getVolumeInterpolationRate() const = 0;
	virtual Audio::AttenuationMethod getAttenuationMethod() const = 0;
	virtual float                    getDistanceAtMaxVolume() const = 0;

protected:

	StringList *        m_samplePathList;

private:

	mutable int         m_referenceCount;
	PersistentCrcString m_crcName;

private:

	// Disabled

	SoundTemplate(SoundTemplate const &);
	SoundTemplate &operator =(SoundTemplate const &);
};

//=============================================================================

#endif // SOUNDTEMPLATE_H
