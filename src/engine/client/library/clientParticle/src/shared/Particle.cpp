// ============================================================================
//
// Particle.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/Particle.h"

// ============================================================================
//
// Particle
//
// ============================================================================

//-----------------------------------------------------------------------------
Particle::Particle()
 : m_age(0.0f)
 , m_distanceZ(0.0f)
 , m_velocity(Vector::zero)
 , m_iterAlpha()
 , m_iterColor()
 , m_iterSpeedScale()
 , m_iterParticleRelativeRotationX()
 , m_iterParticleRelativeRotationY()
 , m_iterParticleRelativeRotationZ()
 , m_lifeTime(1.0f)
 , m_position(Vector::zero)
 , m_positionPrevious(Vector::zero)
 , m_weight(0.0f)
 , m_particleAttachment(NULL)
 , m_agePercent(0.0f)
{
}

//-----------------------------------------------------------------------------
Particle::~Particle()
{
	if (m_particleAttachment != NULL)
	{
		if (m_particleAttachment->getKillWhenParticleDies())
		{
			m_particleAttachment->kill();
		}
	}
}

// ============================================================================
