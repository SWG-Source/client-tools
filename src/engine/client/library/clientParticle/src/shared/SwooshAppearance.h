// ============================================================================
//
// SwooshAppearance.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_SwooshAppearance_H
#define INCLUDED_SwooshAppearance_H

#include "clientAudio/SoundId.h"
#include "clientParticle/SwooshAppearanceTemplate.h"
#include "sharedCollision/BoxExtent.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/MemoryBlockManagedObject.h"

class Iff;
class SwooshAppearanceTemplate;

//-----------------------------------------------------------------------------
class SwooshAppearance : public Appearance
{
public:

#ifdef _DEBUG
	static int getGlobalCount();
	static int getCullCountThisFrame();
#endif // _DEBUG

public:

	explicit SwooshAppearance(SwooshAppearanceTemplate const *swooshAppearanceTemplate);
	SwooshAppearance(SwooshAppearanceTemplate const *swooshAppearanceTemplate, Object const * const referenceObject);
	SwooshAppearance(SwooshAppearanceTemplate const *swooshAppearanceTemplate, Appearance const * const hardPointAppearance, CrcString const &hardPoint);
	SwooshAppearance(SwooshAppearanceTemplate const *swooshAppearanceTemplate, Appearance const * const hardPointAppearance, CrcString const &hardPoint1, CrcString const &hardPoint2);
	virtual ~SwooshAppearance();

	virtual SwooshAppearance * asSwooshAppearance();
	virtual SwooshAppearance const * asSwooshAppearance() const;

	static SwooshAppearance * asSwooshAppearance(Appearance * appearance);
	static SwooshAppearance const * asSwooshAppearance(Appearance const * appearance);

	virtual void                       addToWorld();
	virtual void                       removeFromWorld();
	virtual void                       render() const;
	virtual float                      alter(float deltaTime);

	bool                               isPaused() const;
	bool isCameraFacing() const;

	void                               setWidth(float const width);
	void                               setPosition_w(Vector const &position_w);
	void                               setPosition_w(Vector const &topPosition_w, Vector const &endPosition_w);
	void                               setAllowAutoDelete(bool const allowAutoDelete);
	void                               setReferenceObject(Object const *referenceObject);

	void                               restart();
	void                               kill();

	void                               setUnBounded(bool const unBounded);
	void                               setColorModifier(VectorArgb const &color);
	static void                        setGlobalColorModifier(VectorArgb const &color);

	virtual Extent const *             getExtent() const;
	virtual Sphere const &             getSphere() const;
	VectorArgb const &                 getColorModifier() const;
	VectorArgb const &                 getGlobalColorModifier() const;

	CrcString const & getHardPoint1() const;

protected:

	DPVS::Object *                     getDpvsObject() const;
	virtual bool                       realIsAlive() const;
	unsigned int                       getPositionSampleCount() const;

private:

	class LocalShaderPrimitive;
	friend LocalShaderPrimitive;

	class PositionData
	{
	public:

		PositionData()
		 : m_valid(true)
		 , m_nextValidIndex(0)
		 , m_topPosition_w()
		 , m_middlePosition_w()
		 , m_bottomPosition_w()
		{
		}
	
		bool m_valid;
		unsigned int m_nextValidIndex;
		Vector m_topPosition_w;
		Vector m_middlePosition_w;
		Vector m_bottomPosition_w;
	};

	enum PositionReference
	{
		PR_none,
		PR_object,
		PR_oneHardPoint,
		PR_twoHardPoints
	};

	SwooshAppearanceTemplate const *  m_swooshAppearanceTemplate;
	LocalShaderPrimitive *            m_localShaderPrimitive;
	float                             m_currentTime;
	BoxExtent                         m_extent_w;
	int                               m_accumulatedSmallerRadiusChanges;
	bool                              m_unBounded;
	DPVS::Object *                    m_dpvsObject;
	PositionData * const m_positionList;
	unsigned int                      m_positionIndex;
	Watcher<MemoryBlockManagedObject> m_startObject;
	Watcher<MemoryBlockManagedObject> m_endObject;
	bool                              m_hasBeenAltered;
	SoundId                           m_soundId;
	VectorArgb                        m_colorModifier;
	float                             m_shaderScrollPosition;
	unsigned int                      m_sampledPositionCount;
	int                               m_textureFrame;
	float                             m_width;
	unsigned int                      m_killIndex;
	PersistentCrcString               m_hardPoint1;
	PersistentCrcString               m_hardPoint2;
	ConstWatcher<Object>              m_referenceObject;
	Appearance const * const          m_hardPointAppearance;
	bool                              m_allowAutoDelete;
	float                             m_movementSpeed;
	mutable bool                      m_renderThisFrame;
	float                             m_visibilityPercent;
	float                             m_lastDeltaTime;
	float m_secondsBetweenSamples;
	float m_secondsSinceLastSample;
	bool m_enabled;
	unsigned int const m_templateSamplePositionCount;
	unsigned int m_currentValidSamplePositionCount;
	PositionReference m_positionReference;

	void  drawDebugExtents() const;
	void  renderGeometry() const;
	float getAlpha(float const initialAlpha, float const percent, SwooshAppearanceTemplate::FadeAlpha const fadeAlpha) const;
	void  getTaperGeometry(float const percent, SwooshAppearanceTemplate::TaperGeometry const taperGeometry, Vector const &topPosition_w, Vector const &bottomPosition_w, Vector &result) const;
	Vector const & getTopPosition_w(unsigned int const index) const;
	Vector const & getMiddlePosition_w(unsigned int const index) const;
	Vector const & getBottomPosition_w(unsigned int const index) const;
	int getVisualIndex(unsigned int const index) const;

private:

	// Disabled

	SwooshAppearance();
	SwooshAppearance(const SwooshAppearance &);
	SwooshAppearance &operator =(const SwooshAppearance &);
};

//----------------------------------------------------------------------

inline CrcString const & SwooshAppearance::getHardPoint1() const
{
	return m_hardPoint1;
}

// ============================================================================

#endif // INCLUDED_SwooshAppearance_H
