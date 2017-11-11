// ============================================================================
//
// ParticleEmitterGroup.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEmitterGroup_H
#define INCLUDED_ParticleEmitterGroup_H

#include "clientParticle/ParticleGenerator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/MemoryBlockManagedObject.h"

class Iff;
class ParticleEmitter;
class ParticleEmitterGroupDescription;

//-----------------------------------------------------------------------------
class ParticleEmitterGroup : public ParticleGenerator
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();
	static int getGlobalCount();

public:

	explicit ParticleEmitterGroup(ParticleEmitterGroupDescription const &particleEmitterGroupDescription, ParticleEffectAppearance const * const particleEffectAppearance);
	virtual ~ParticleEmitterGroup();

	virtual void addToCameraScene(Camera const *camera, Object const *object) const;
	virtual void alter(float const deltaTime);
	virtual void restart();
	virtual bool isDeletable() const;
	virtual void setOwner(Object *newOwner);
	virtual void addToWorld();
	virtual void removeFromWorld();

	virtual int                   getParticleCount() const;
	bool hasAliveParticles() const;
	virtual BoxExtent const &     getExtent() const;
	ParticleEmitter const * const getEmitter(int const index) const;
	int                           getEmitterCount() const;
	bool                          hasChildrenLoopedOnce() const;

private:

	static void remove();

private:

	bool isInfiniteLooping() const;

	typedef stdvector<ParticleEmitter *>::fwd ParticleEmitterList;

	ParticleEmitterList *                  m_particleEmitterList;
	ParticleEmitterGroupDescription const &m_particleEmitterGroupDescription;
	float                                  m_currentTime;
	float                                  m_startDelay;
	int                                    m_loopCount;               // -1 is infinite
	int                                    m_currentLoop;
	Watcher<MemoryBlockManagedObject>      m_object;
	void                                   drawDebugExtents() const;

	void loop();

	// Disabled

	ParticleEmitterGroup();
	ParticleEmitterGroup(ParticleEmitterGroup const &);
	ParticleEmitterGroup &operator =(ParticleEmitterGroup const &);
};

// ============================================================================

#endif // INCLUDED_ParticleEmitterGroup_H
