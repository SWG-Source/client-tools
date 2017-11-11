// ============================================================================
//
// ParticleEmitterGroupDescription.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEmitterGroupDescription_H
#define INCLUDED_ParticleEmitterGroupDescription_H

#include "clientParticle/ParticleGenerator.h"
#include "clientParticle/ParticleTiming.h"

class ParticleEmitterDescription;
class Iff;

//-----------------------------------------------------------------------------
class ParticleEmitterGroupDescription
{
public:

	typedef stdvector<ParticleEmitterDescription>::fwd ParticleEmitterDescriptions;

public:

	ParticleEmitterGroupDescription();
	ParticleEmitterGroupDescription(ParticleEmitterGroupDescription const &rhs);
	~ParticleEmitterGroupDescription();
	ParticleEmitterGroupDescription &operator =(ParticleEmitterGroupDescription const &rhs);

	void       load(Iff &iff);
	void       write(Iff &iff) const;
	void       initializeDefault();

	// Get the total number of ParticleEmitterGroupDescription objects alive right now

	static int getGlobalCount();

	void setParticleEmitterDescriptions(ParticleEmitterDescriptions &particleEmitterDescriptions);
	void setParticleTiming(ParticleTiming const &particleTiming);

	ParticleEmitterDescriptions const &getParticleEmitterDescriptions() const;
	ParticleTiming const &             getParticleTiming() const;
	bool                               isInfiniteLooping() const;

	void clearParticleEmitterDescriptions();

private:

	static int                   m_globalCount;
	ParticleEmitterDescriptions *m_particleEmitterDescriptions; // The list of particle emitter descriptions
	ParticleTiming               m_timing;                      // The timing information

#ifdef _DEBUG
	void validate();
#else
	void validate() {}
#endif // _DEBUG

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
};

// ============================================================================

#endif // INCLUDED_ParticleEmitterGroupDescription_H
