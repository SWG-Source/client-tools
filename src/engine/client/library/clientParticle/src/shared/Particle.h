// ============================================================================
//
// Particle.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_Particle_H
#define INCLUDED_Particle_H

#include "clientParticle/ColorRamp.h"
#include "clientParticle/ParticleAttachment.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedMath/Vector.h"
#include "sharedMath/WaveForm.h"

//-----------------------------------------------------------------------------
class Particle
{
friend class ParticleEmitter;

public:

	Particle();
	virtual ~Particle() = 0;

	void alter(float const deltaTime);
	void calculateAgePercent();
	void kill();

	bool isAlive() const;
	float getAgePercent() const;

protected:

	// These variables are to be modified by ParticleEmitter directly

	float                       m_age;
	float                       m_distanceZ;
	Vector                      m_velocity;
	WaveFormControlPointIter    m_iterAlpha;
	ColorRampControlPointIter   m_iterColor;
	WaveFormControlPointIter    m_iterSpeedScale;
	WaveFormControlPointIter    m_iterParticleRelativeRotationX;
	WaveFormControlPointIter    m_iterParticleRelativeRotationY;
	WaveFormControlPointIter    m_iterParticleRelativeRotationZ;
	float                       m_lifeTime;
	Vector                      m_position;
	Vector                      m_positionPrevious;
	float                       m_weight;
	Watcher<ParticleAttachment> m_particleAttachment;

private:

	float  m_agePercent;

	// Disabled

	Particle &operator =(Particle const &);
};

// ============================================================================

//-----------------------------------------------------------------------------
inline void Particle::kill()
{
	m_age = m_lifeTime;
}

//-----------------------------------------------------------------------------
inline bool Particle::isAlive() const
{
	return (m_age < m_lifeTime);
}

//-----------------------------------------------------------------------------
inline float Particle::getAgePercent() const
{
	return m_agePercent;
}

//-----------------------------------------------------------------------------
inline void Particle::calculateAgePercent()
{
	DEBUG_FATAL((m_age < 0.0f), ("age(%f) must be >= 0", m_age));
	DEBUG_FATAL((m_age > m_lifeTime), ("age(%f) > lifetime(%f)", m_age, m_lifeTime));
	DEBUG_FATAL((m_lifeTime <= 0.0f), ("lifetime(%f) must be > 0", m_lifeTime));

	m_agePercent = m_age / m_lifeTime;
}

//-----------------------------------------------------------------------------
inline void Particle::alter(float const /*deltaTime*/)
{
	if (m_particleAttachment != NULL)
	{
		m_particleAttachment->setPosition(m_position);
	}
}

#endif // INCLUDED_Particle_H
