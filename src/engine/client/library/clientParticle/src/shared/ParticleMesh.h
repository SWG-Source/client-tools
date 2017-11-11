// ============================================================================
//
// ParticleMesh.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleMesh_H
#define INCLUDED_ParticleMesh_H

#include "clientParticle/Particle.h"
#include "sharedMath/Vector.h"
#include "sharedMath/WaveForm.h"

class Object;
class MemoryBlockManagedObject;
class MemoryBlockManager;

//-----------------------------------------------------------------------------
class ParticleMesh : public Particle
{
friend class ParticleEmitter;

public:

	ParticleMesh();
	virtual ~ParticleMesh();

	static void *operator new (size_t size);
	static void  operator delete (void *pointer);
	static void install();
	static void remove();

	static bool isParticlePoolFull();
	static int  getGlobalCount();

protected:

	// These variables are to be modified by ParticleEmitter directly

	WaveFormControlPointIter          m_iterScale;
	WaveFormControlPointIter          m_iterRotationX;
	WaveFormControlPointIter          m_iterRotationY;
	WaveFormControlPointIter          m_iterRotationZ;
	Vector                            m_rotationInitial;
	Watcher<MemoryBlockManagedObject> m_object;

private:

	static MemoryBlockManager *m_memoryBlockManager;

	// Disabled

	ParticleMesh &operator =(ParticleMesh const &);
};

// ============================================================================

#endif // INCLUDED_ParticleMesh_H