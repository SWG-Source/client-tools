// ============================================================================
//
// ParticleEffectAppearance.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEffectAppearance_H
#define INCLUDED_ParticleEffectAppearance_H

#include "clientParticle/ParticleEffectDescription.h"
#include "sharedCollision/BoxExtent.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Appearance.h"

class Iff;
class ParticleEffectAppearanceTemplate;
class ParticleEmitterGroup;

//-----------------------------------------------------------------------------

class ParticleEffectAppearance : public Appearance
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();

	// Total ParticleEffectAppearances created

	static int getGlobalCount();

	// GLOBAL wind affecting all world space particles

	static void setGlobalWind(Vector const &wind);
	static Vector const & getGlobalWind();

	// GLOBAL LOD bias affecting all particles

	static void setGlobalLodBias(float const bias);
	static float getGlobalLodBias();

	// GLOBAL Color Modifier affecting all particles

	static void setGlobalColorModifier(VectorArgb const &color);
	static VectorArgb const & getGlobalColorModifier();

	// GLOBAL Wind affecting all particles

	static void setGlobalWindEnabled(bool const enabled);
	static bool isGlobalWindEnabled();

	// User limit of how many non-priority particles they would like to see.
	static void setGlobalUserLimit(int const limit);
	static int const getGlobalUserLimit();

public:

	explicit ParticleEffectAppearance(ParticleEffectAppearanceTemplate const *particleEffectAppearanceTemplate);
	virtual ~ParticleEffectAppearance();

	virtual ParticleEffectAppearance * asParticleEffectAppearance();
	virtual ParticleEffectAppearance const * asParticleEffectAppearance() const;

	static ParticleEffectAppearance * asParticleEffectAppearance(Appearance * appearance);
	static ParticleEffectAppearance const * asParticleEffectAppearance(Appearance const * appearance);

#ifdef _DEBUG
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;
	virtual bool implementsCollide() const;
#endif

	virtual void                       addToWorld();
	virtual void                       removeFromWorld();
	virtual void                       render() const;
	virtual float                      alter(float deltaTime);
	virtual void                       setKeepAlive(bool keepAlive);
	void                               restart();

	bool                               isDeletable() const;
	bool                               isEnabled() const;
	bool                               isPaused() const;
	bool                               isInfiniteLooping() const;

	virtual void                       setOwner(Object *newOwner);
	void                               setEnabled(bool const enabled);
	void                               setLodBias(float const bias);
	void                               setPaused(bool const paused);
	void                               setPlayBackRate(float const playBackRate);
	void                               setAutoDelete(bool const autoDelete);
	void                               setUnBounded(bool const unBounded);
	void                               setParentParticleEffectAppearance(ParticleEffectAppearance const & parentParticleEffectAppearance);
	void                               setColorModifier(VectorArgb const &color);
	virtual void setScale(Vector const & scale);
	void setRestartOnRemoveFromWorld(bool const restartOnRemoveFromWorld);

	ParticleEffectAppearance const *   getParticleEffectAppearance() const;
	ParticleEmitterGroup const * const getEmitterGroup(int const index) const;
	int                                getEmitterGroupCount() const;
	virtual const Extent *             getSelectionExtent() const;
	virtual Extent const *             getExtent() const;
	int                                getParticleCount() const;
	bool hasAliveParticles() const;
	virtual Sphere const &             getSphere() const;
	float                              getPlayBackRate() const;
	float                              getLodBias() const;
	VectorArgb const &                 getColorModifier() const;
	float getPlayBackRate_w() const;
	float getScale_w() const;

protected:

	DPVS::Object *                     getDpvsObject() const;
	virtual bool                       realIsAlive() const;

private:
	typedef stdvector<ParticleEmitterGroup *>::fwd ParticleEmitterGroups;

	ParticleEffectAppearanceTemplate const *m_particleEffectAppearanceTemplate;
	ParticleEmitterGroups *                 m_particleEmitterGroups;
	ParticleEffectAppearance const *        m_parentParticleEffectAppearance; // Parent for attachments
	float                                   m_age;
	float                                   m_playBackRate;
	bool                                    m_paused;
	bool                                    m_enabled;
	BoxExtent                               m_extent;
	int                                     m_accumulatedSmallerRadiusChanges;
	bool                                    m_autoDelete;
	bool                                    m_unBounded;
	DPVS::Object *                          m_dpvsObject;
	mutable Vector                          m_currentCameraPosition_w;
	bool const                              m_infiniteLooping;
	VectorArgb                              m_colorModifier;
	float                                   m_lodBias;
	bool                                    m_restartOnRemoveFromWorld;
	float m_scale_w;

#ifdef _DEBUG
	Appearance* m_originIcon;
#endif

	static void remove();

#ifdef _DEBUG
	void drawDebugExtents() const;
#endif // _DEBUG

#ifdef _DEBUG
	void drawOriginIcon() const;
#endif

	// Disabled

	ParticleEffectAppearance();
	ParticleEffectAppearance(const ParticleEffectAppearance &);
	ParticleEffectAppearance &operator =(const ParticleEffectAppearance &);
};

// ============================================================================

#endif // INCLUDED_ParticleEffectAppearance_H
