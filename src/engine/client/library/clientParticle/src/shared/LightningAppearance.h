// ============================================================================
//
// LightningAppearance.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_LightningAppearance_H
#define INCLUDED_LightningAppearance_H

#include "clientAudio/SoundId.h"
#include "clientParticle/LightningBolt.h"
#include "sharedCollision/BoxExtent.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/MemoryBlockManagedObject.h"

class Iff;
class LightningAppearanceTemplate;

//-----------------------------------------------------------------------------
class LightningAppearance : public Appearance
{
public:

	explicit LightningAppearance(LightningAppearanceTemplate const *LightningAppearanceTemplate);
	virtual ~LightningAppearance();

	virtual LightningAppearance * asLightningAppearance();
	virtual LightningAppearance const * asLightningAppearance() const;

	static LightningAppearance * asLightningAppearance(Appearance * appearance);
	static LightningAppearance const * asLightningAppearance(Appearance const * appearance);

	virtual void                       addToWorld();
	virtual void                       removeFromWorld();
	virtual void                       render() const;
	virtual float                      alter(float deltaTime);

	bool                               isDeletable() const;
	bool                               isEnabled() const;
	bool                               isPaused() const;

	void                               setPosition_w(int const index, Vector const &startPosition_w, Vector const &endPosition_w);
	void                               setEnabled(bool const enabled);

	void                               restart();
	void                               setPaused(bool const paused);
	void                               setTimeScale(float const timeScale);
	void                               setUnBounded(bool const unBounded);
	void                               setColorModifier(VectorArgb const &color);
	void                               setGlobalColorModifier(VectorArgb const &color);
	virtual void setScale(Vector const & scale);

	virtual Extent const *             getExtent() const;
	virtual Sphere const &             getSphere() const;
	float                              getTimeScale() const;
	VectorArgb const &                 getColorModifier() const;
	VectorArgb const &                 getGlobalColorModifier() const;
	static int                         getMaxLightningBoltCount();

	void setStartAppearancesEnabled(bool enabled);
	void setEndAppearancesEnabled(bool enabled);

	int getActiveBoltCount() const;

#ifdef _DEBUG
	static int                         getGlobalCount();
#endif // _DEBUG

protected:

	DPVS::Object *                     getDpvsObject() const;

private:

	class LocalShaderPrimitive;
	friend LocalShaderPrimitive;

	LightningAppearanceTemplate const *m_lightningAppearanceTemplate;
	LocalShaderPrimitive *             m_localShaderPrimitive;
	float                              m_currentTime;
	float                              m_timeScale;
	bool                               m_paused;
	bool                               m_enabled;
	BoxExtent                          m_extent_w;
	int                                m_accumulatedSmallerRadiusChanges;
	bool                               m_unBounded;
	DPVS::Object *                     m_dpvsObject;
	mutable Vector                     m_currentCameraPosition_w;
	void                               drawDebugExtents() const;
	void                               renderGeometry() const;
	Vector                             getAverageStartPosition() const;
	Vector                             getAverageEndPosition() const;

	class LightningBoltData
	{
	public:

		LightningBoltData();
		~LightningBoltData();

		bool        m_enabled;
		bool        m_largeBolt;
		Vector      m_morphPositionOffsetList_o[LightningBolt::m_positionListSize];
		Vector      m_positionList_w[LightningBolt::m_positionListSize];
		int         m_lightningBoltIndex;
		float       m_lightningBoltIndexTimer;
		Vector      m_startPosition_w;
		Vector      m_endPosition_w;
		mutable int m_textureFrame;
		float       m_textureFrameTimer;

		Watcher<MemoryBlockManagedObject> m_startObject;
		Watcher<MemoryBlockManagedObject> m_endObject;

	private:

		// Disabled

		LightningBoltData(LightningBoltData const &);
		LightningBoltData &operator =(LightningBoltData const &);
	};

	enum
	{
		m_maxLightningBoltCount = 5
	};

	LightningBoltData m_lightningBoltDataList[m_maxLightningBoltCount];
	int               m_activeBoltCount;
	bool              m_hasBeenAltered;
	SoundId           m_soundId;
	VectorArgb        m_colorModifier;
	float             m_largeBoltShaderScrollPosition;
	float             m_smallBoltShaderScrollPosition;

#ifdef _DEBUG
	static int        m_globalCount;
#endif // _DEBUG

private:

	// Disabled

	LightningAppearance();
	LightningAppearance(const LightningAppearance &);
	LightningAppearance &operator =(const LightningAppearance &);
};

//----------------------------------------------------------------------

inline int LightningAppearance::getActiveBoltCount() const
{
	return m_activeBoltCount;
}

// ============================================================================

#endif // INCLUDED_LightningAppearance_H
