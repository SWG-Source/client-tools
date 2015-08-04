// ============================================================================
//
// ParticleQuad.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleQuad_H
#define INCLUDED_ParticleQuad_H

#include "clientParticle/Particle.h"
#include "sharedMath/WaveForm.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------------
class ParticleQuad : public Particle
{
friend class ParticleEmitter;

public:

	ParticleQuad();
	virtual ~ParticleQuad();

	static void *operator new (size_t size);
	static void  operator delete (void *pointer);
	static void install();
	static void remove();

	static bool isParticlePoolFull(bool priority);
	static int  getGlobalCount();

protected:

	// These variables are to be modified by Emitter directly

	Vector                    m_upVector;
	Vector                    m_sideVector;
	float                     m_initialRotation;
	WaveFormControlPointIter  m_iterLength;
	WaveFormControlPointIter  m_iterRotation;
	WaveFormControlPointIter  m_iterWidth;
	
private:

	static MemoryBlockManager *m_memoryBlockManager;

	// Disabled

	ParticleQuad &operator =(ParticleQuad const &);
};

// ============================================================================

#endif // INCLUDED_ParticleQuad_H
