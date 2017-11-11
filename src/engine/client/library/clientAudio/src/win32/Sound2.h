// ============================================================================
//
// Sound2.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef SOUND2_H
#define SOUND2_H

#include "clientAudio/Audio.h"
#include "clientAudio/SoundId.h"
#include "clientAudio/SampleId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

class Object;
class SampleId;
class SoundTemplate;
class TextAppearance;
class VectorArgb;

//-----------------------------------------------------------------------------
class Sound2
{
public:

	typedef Object* (*CreateTextAppearanceObjectFunction) (const char* text, const VectorArgb& color, const float scale);
	static void setCreateTextAppearanceObjectFunction (CreateTextAppearanceObjectFunction createTextAppearanceObjectFunction);

	typedef void (*DrawCircleFunction) (const Vector &center, const Vector &normal, real radius, int segments, const VectorArgb &argb);
	static void setDrawCircleFunction(DrawCircleFunction drawCircleFunction);

public:

	static void install();

public:

	Sound2(SoundTemplate const *soundTemplate, SoundId const &soundId);
	virtual ~Sound2();

	typedef void (*EndOfSampleCallBack) ();

	void                             setEndOfSampleCallBack(EndOfSampleCallBack callBack);
	void                             setAutoDelete(bool const autoDelete);
	void                             setSampleId(SampleId const &sampleId);
	void                             setUserVolume(float const volume);
	void                             setUserPitchDelta(float const pitchDelta);

	CrcString const *                getSamplePath() const;
	SampleId const &                 getSampleId() const;

	void                             kill(float const fadeOutTime = 0.0f);
	virtual void                     alter(float const deltaTime) = 0;
	virtual void                     endOfSample() = 0;
	virtual void                     stop(float const fadeOutTime, bool const stoppingOfOutOfRange = false) = 0;
	virtual bool                     isSampleFinished() const = 0;
	virtual bool                     is2d() const = 0;
	virtual bool                     is3d() const = 0;

	virtual void                     setCurrentTime(int const milliSeconds) = 0;
	virtual void                     setPlayBackRate(int const playBackRate) = 0;

	virtual int                      getCurrentTime() const = 0;
	virtual int                      getTotalTime() const = 0;

	virtual float                    getCurrentSoundTime() const = 0;
	virtual bool                     getSampleTime(float &timeCurrent, float &timeTotal) = 0;
	virtual float                    getPitchDelta() const = 0;
	virtual float                    getTemplateVolume() const = 0;  // Volume specified in the template
	virtual float                    getAttenuation() const = 0;   // Volume percent based on distance from listener
	SoundTemplate const *            getTemplate() const;
	virtual float                    getDistanceFromListener() const = 0;
	virtual float                    getDistanceSquaredFromListener() const = 0;
	virtual float                    getDistanceAtVolumeCutOff() const = 0;
	SoundId const &                  getSoundId() const;
	virtual float                    getVolume() = 0;
	virtual int                      getPlayBackRate() const = 0;
	virtual float                    getPlayBackRateDelta() const = 0;
	float                            getUserVolume() const;
	float                            getUserPitchDelta() const;
	const TemporaryCrcString &       getHardpointName () const;

	void                             setPosition_w(Vector const &position_w);
	void                             setObject(Object const *object);
	void                             setHardPointName(char const * const hardPointName);

	bool                             isInfiniteLooping() const;
	int                              getLoopCount() const;
	Object const * const             getObject() const;
	Vector                           getPosition_w() const;
	bool                             isPositionSpecified() const;

#ifdef _DEBUG
	virtual int                      getTotalSampleSize() const = 0; // Bytes
	virtual bool                     isStreamed() const = 0;
#endif // _DEBUG

	virtual bool                     isDeletable() const = 0;
	virtual bool                     isPlaying() const = 0;
	bool                             isOccluded() const;
	bool                             isObstructed() const;
	float                            getOcclusion() const;
	float                            getObstruction() const;
	float                            getOcclusionPercent() const;
	float                            getObstructionPercent() const;
	virtual bool                     usesSample(CrcString const &path) const = 0;

#ifdef _DEBUG
	void setText(char const *string, VectorArgb const &color);
#endif // _DEBUG

	void                       setParentCell(CellProperty const *parentCell);
	CellProperty const * const getParentCell() const;

protected:

	SoundTemplate const *m_template;
	SoundId const        m_soundId;
	bool                 m_autoDelete;
	EndOfSampleCallBack  m_callBack;
	SampleId             m_sampleId;
	int                  m_sampleIndex;
	int                  m_loopCount;
	bool                 m_positionSpecified;
	bool                 m_wasParented;
	ConstWatcher<Object> m_object;
	TemporaryCrcString   m_hardPointName;
	float                m_samplePreviousTime;
	float                m_userVolume;
	float                m_userPitchDelta;
	float                m_obstructionPercent;
	float                m_occlusionPercent;

	virtual void reset() = 0;

private:

	Vector mutable       m_position_w;
	CellProperty const * m_parentCell;

#ifdef _DEBUG
	void addDebugText(Object const *object, Vector const &position);

	Watcher<Object>  m_locationTextObject;
	TextAppearance * m_appearance;
#endif // _DEBUG

	// Disabled

	Sound2();
	Sound2(Sound2 const &);
	Sound2 &operator =(Sound2 const &);
};

//----------------------------------------------------------------------

inline const TemporaryCrcString & Sound2::getHardpointName () const
{
	return m_hardPointName;
}

// ============================================================================

#endif // SOUND2_H