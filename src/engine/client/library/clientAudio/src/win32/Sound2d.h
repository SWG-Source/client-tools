// ============================================================================
//
// Sound2d.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_Sound2d_H
#define INCLUDED_Sound2d_H

#include "clientAudio/Audio.h"
#include "clientAudio/Sound2.h"

#ifdef _DEBUG
#include <string>
#endif // _DEBUG

class MemoryBlockManager;
class Sound2dTemplate;
class IndexPool;

//-----------------------------------------------------------------------------
class Sound2d : public Sound2
{
public:

	explicit Sound2d(Sound2dTemplate const *sound2dTemplate, SoundId const &soundId);
	virtual ~Sound2d();

	static void *operator new (size_t size);
	static void  operator delete (void *pointer);
	static void install();
	static void remove();
	static int  getCount();
	static int  getMaxCount();

	
	virtual void            alter(float const deltaTime);
	virtual void            endOfSample();
	virtual bool            isSampleFinished() const;
	virtual void            stop(float const fadeOutTime, bool const stoppingOfOutOfRange = false);
	virtual bool            is2d() const;
	virtual bool            is3d() const;

	virtual void            setCurrentTime(int const milliSeconds);
	virtual int             getCurrentTime() const;
	virtual int             getTotalTime() const;

	virtual bool            usesSample(CrcString const &path) const;
	virtual bool            isDeletable() const;
	virtual bool            isPlaying() const;
	bool                    isStillLooping() const;

	virtual bool            getSampleTime(float &timeCurrent, float &timeTotal);
	virtual float           getPitchDelta() const;
	virtual float           getCurrentSoundTime() const;
	//virtual float           getCurrentSampleTime() const;
	float                   getStartDelay() const;
	Sound2dTemplate const * getTemplate() const;
	virtual float           getTemplateVolume() const;
	virtual float           getAttenuation() const;
	virtual float           getDistanceFromListener() const;
	virtual float           getDistanceSquaredFromListener() const;
	virtual float           getDistanceAtVolumeCutOff() const;
	virtual float           getVolume();
	virtual int             getPlayBackRate() const;
	virtual float           getPlayBackRateDelta() const;
	float                   getFadeoutVolume () const;

#ifdef _DEBUG
	virtual int             getTotalSampleSize() const;
	virtual bool            isStreamed() const;
#endif // _DEBUG

	virtual void               setPlayBackRate(int const playBackRate);

protected:

	virtual void     reset();

private:

	void  startSample();
	void  setPlayBackRate();
	void  setupNextSample();
	void  setupStartDelay();
	void  setupFadeIn();
	void  setupFadeOut();
	void  setupVolume();
	void  setupPitch();
	void  releaseSample();
	float getFade();
	void  updateTemplateVolume(float const deltaTime);
	void  updateTemplatePitch(float const deltaTime);

	float               m_currentSoundTime;
	float               m_startDelay;
	float m_currentStartDelay;
	float               m_fadeInTime;
	int                 m_currentLoop;
	float               m_fadeOutTime;
	float               m_templateVolume;
	float               m_templateVolumeStart;
	float               m_templateVolumeGoal;
	float               m_templateVolumeTimer;
	float               m_pitchHalfStepCurrent;
	float               m_pitchHalfStepStart;
	float               m_pitchHalfStepGoal;
	float               m_pitchTimer;
	bool                m_endOfSample;
	float               m_fadeOutTimer;
	bool                m_manualFadeOutStarted;
	bool                m_stoppedOutOfRange;
	int                 m_initialPlayBackRate;
	bool                m_sampleStarted;


	IndexPool*          m_indexPool;
	int                 m_indexPoolDepth;

	static MemoryBlockManager *m_memoryBlockManager;

#ifdef _DEBUG
	TemporaryCrcString  m_currentSamplePath;
	int                 m_totalSampleSize;
#endif // _DEBUG

private:

	// Disabled

	Sound2d();
	Sound2d(Sound2d const &);
	Sound2d &operator =(Sound2d const &);
};

//----------------------------------------------------------------------

inline float Sound2d::getFadeoutVolume () const
{
	if (m_manualFadeOutStarted && m_fadeOutTime > 0.0f)
	{
		if (m_fadeOutTime <= 0.0f)
			return 0.0f;
		else
		{
			const float vol = m_fadeOutTimer / m_fadeOutTime;

			if (vol < 0.0f)
				return 0.0f;

			return vol;
		}
	}

	return 1.0f;
}


// ============================================================================

#endif // INCLUDED_Sound2d_H
