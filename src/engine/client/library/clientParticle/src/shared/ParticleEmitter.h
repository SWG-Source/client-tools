// ============================================================================
//
// ParticleEmitter.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEmitter_H
#define INCLUDED_ParticleEmitter_H

#include "clientAudio/SoundId.h"
#include "clientParticle/ParticleGenerator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedMath/Transform.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMath/WaveForm.h"
#include "sharedObject/MemoryBlockManagedObject.h"

class Object;
class Particle;
class ParticleAttachment;
class ParticleDescriptionMesh;
class ParticleDescriptionQuad;
class ParticleEmitterDescription;
class ParticleQuad;
class TextAppearance;
class Vector;

//-----------------------------------------------------------------------------
class ParticleEmitter : public ParticleGenerator
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();
	static int getGlobalCount();

	typedef Object* (*CreateTextAppearanceObjectFunction) (const char* text, const VectorArgb& color, const float scale);
	static void setCreateTextAppearanceObjectFunction(CreateTextAppearanceObjectFunction createTextAppearanceObjectFunction);

	typedef void (*DrawBoxFunction) (AxialBox const & box, const VectorArgb &argb);
	static void setDrawBoxFunction(DrawBoxFunction drawBoxFunction);

	typedef void (*DrawCylinderFunction) (const Vector &base, float radius, float height, int tessTheta, int tessRho, int tessZ, int nSpokes, const VectorArgb &argb);
	static void setDrawCylinderFunction(DrawCylinderFunction drawCylinderFunction);

public:

	explicit ParticleEmitter(ParticleEmitterDescription const &particleEmitterDescription, ParticleEffectAppearance const * const particleEffectAppearance);
	virtual ~ParticleEmitter();

	virtual void             addToCameraScene(Camera const *camera, Object const *object) const;
	virtual void             alter(float const deltaTime);
	virtual bool             isDeletable() const;
	virtual void             restart();
	virtual void             setOwner(Object *newOwner);
	virtual void             addToWorld();
	virtual void             removeFromWorld();

	virtual BoxExtent const &getExtent() const;
	virtual int              getParticleCount() const;
	bool hasAliveParticles() const;
	int                      getParticleCountIncludingAttachments() const;
	Particle const &         getParticle(int const particleIndex) const;
	int                      getCurrentLoopCount();

	// Get the number of alive emitters

#ifdef _DEBUG
	static int               getTerrainHeightCallCount();
#endif // _DEBUG

private:

	class LocalShaderPrimitive;
	friend LocalShaderPrimitive;

	typedef stdvector<Particle *>::fwd                   Particles;
	typedef stdvector<Watcher<ParticleAttachment> >::fwd ParticleAttachments;

	ParticleEmitterDescription const &m_particleEmitterDescription;
	Particles * const m_particles;                  // Indexes of the particles into the global particle list
	ParticleAttachments *             m_particleAttachments;
	float                             m_lifeTime;
	float                             m_age;
	float                             m_timeElapsed;
	bool                              m_enabled;
	bool                              m_frameFirst;                 // First frame that the emitter is alive. This is after any start delays.
	bool                              m_frameLast;                  // Last frame the emitter is alive.
	float                             m_newParticles;
	LocalShaderPrimitive *            m_localShaderPrimitive;
	Watcher<MemoryBlockManagedObject> const m_object;
	Transform                         m_previousTransform_o2w;
	float                             m_startDelay;
	int                               m_loopCount;                  // -1 is infinite
	int                               m_currentLoop;
	SoundId                           m_soundId;
	WaveFormControlPointIter          m_iterEmitterTranslationX;
	WaveFormControlPointIter          m_iterEmitterTranslationY;
	WaveFormControlPointIter          m_iterEmitterTranslationZ;
	WaveFormControlPointIter          m_iterEmitterRotationX;
	WaveFormControlPointIter          m_iterEmitterRotationY;
	WaveFormControlPointIter          m_iterEmitterRotationZ;
	WaveFormControlPointIter          m_iterEmitterDistance;
	WaveFormControlPointIter          m_iterEmitterShapeSize;
	WaveFormControlPointIter          m_iterEmitterSpread;
	WaveFormControlPointIter          m_iterParticleGenerationRate;
	WaveFormControlPointIter          m_iterParticleEmitSpeed;
	WaveFormControlPointIter          m_iterParticleInheritVelocityPercent;
	WaveFormControlPointIter          m_iterParticleClusterCount;
	WaveFormControlPointIter          m_iterParticleClusterRadius;
	WaveFormControlPointIter          m_iterParticleLifeTime;
	WaveFormControlPointIter          m_iterParticleWeight;
	float                             m_particleForwardVelocityMaintained;
	float                             m_particleUpVelocityMaintained;
	float                             m_lodPercent;
	float                             m_accumulatedDistance;       // Stored as distance squared
	mutable Vector                    m_currentCameraPosition_w;
	Vector                            m_averageParticlePosition;
	Vector                            m_averageParticleVelocity;

#ifdef _DEBUG
	Object * m_debugTextObject;
	TextAppearance * m_debugTextObjectAppearance;
	VectorArgb m_debugTextColor;
#endif // _DEBUG

private:

	static void remove();

private:

	void  calculateRenderOrientationVectors(ParticleDescriptionQuad const *particleDescriptionQuad, ParticleQuad &particle, Vector &upVector, Vector &sideVector, Transform const &transform) const;
	void  createSingleParticle(float const deltaTime);
	void  createSingleParticleMesh(ParticleDescriptionMesh const *particleDescriptionMesh, float const deltaTime);
	void  createSingleParticleQuad(ParticleDescriptionQuad const *particleDescriptionQuad, float const deltaTime);
	void  createNewParticles(float const deltaTime);
	void  draw() const;
	void  drawParticlesQuad(ParticleDescriptionQuad const *particleDescriptionQuad) const;
	float getAgePercent() const;
	void  initializeSingleParticle(Particle &particle);
	void  loop();
	void  removeAllParticles();
	void  removeAllAttachments();
	void  removeOldParticles();
	void  removeOldAttachments();
	void  updateExistingParticles(float const deltaTime);
	void  updateSingleParticle(Particle *particle, float const deltaTime);
	void  frameFirst();
	void  frameLast();
	void  calculateLod();
	void  sortParticles() const;
	bool  isInfiniteLooping() const;
	void createParticleAttachment(Particle & particle, std::string const & attachmentPath, float const deltaTime);

	void  drawDebugAxis() const;
	void  drawDebugParticleOrientation() const;
	void  drawDebugVelocity() const;
	void  drawDebugExtents() const;

private:

	// Disabled

	ParticleEmitter();
	ParticleEmitter(ParticleEmitter const &);
	ParticleEmitter &operator =(ParticleEmitter const &);
};

// ============================================================================

#endif // INCLUDED_ParticleEmitter_H
