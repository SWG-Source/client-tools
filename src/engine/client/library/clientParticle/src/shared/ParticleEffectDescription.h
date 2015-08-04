// ============================================================================
//
// ParticleEffectDescription.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEffectDescription_H
#define INCLUDED_ParticleEffectDescription_H

#include "clientParticle/ParticleTiming.h"
#include "clientParticle/ParticleEmitterGroupDescription.h"
#include <string>

class ParticleEmitterGroupDescription;
class Iff;

//-----------------------------------------------------------------------------
class ParticleEffectDescription
{
public:

	typedef stdvector<ParticleEmitterGroupDescription>::fwd ParticleEmitterGroupDescriptions;

public:

	ParticleEffectDescription();
	ParticleEffectDescription(ParticleEffectDescription const &rhs);
	~ParticleEffectDescription();

	ParticleEffectDescription &operator =(ParticleEffectDescription const &rhs);

	void load(Iff &iff);
	void write(Iff &iff) const;
	void initializeDefault();

	void setName(char const *name);
	void setParticleEmitterGroupDescriptions(ParticleEmitterGroupDescriptions &particleEmitterGroupDescriptions);
	void setParticleTiming(ParticleTiming const &particleTiming);

	std::string const &                     getName() const;
	ParticleEmitterGroupDescriptions const &getParticleEmitterGroupDescriptions() const;
	ParticleTiming const &                  getParticleTiming() const;
	bool                                    isInfiniteLooping() const;

	void setInitialPlayBackRate(float const initialPlayBackRate);
	float getInitialPlayBackRate() const;

	void setInitialPlayBackRateTime(float const initialPlayBackRateTime);
	float getInitialPlayBackRateTime() const;

	void setPlayBackRate(float const playBackRate);
	float getPlayBackRate() const;

	void setScale(float const scale);
	float getScale() const;

	void clearParticleEmitterGroupDescriptions();

private:

	std::string                       m_name;
	ParticleTiming                    m_timing;                        // The timing information
	ParticleEmitterGroupDescriptions *m_particleEmitterGroupDescriptions; // The list of emitter group description that create the whole effect
	float m_initialPlayBackRate;
	float m_initialPlayBackRateTime;
	float m_playBackRate;
	float m_scale;

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
	void load_0002(Iff &iff);
};

// ============================================================================

#endif // INCLUDED_ParticleEffectDescription_H
