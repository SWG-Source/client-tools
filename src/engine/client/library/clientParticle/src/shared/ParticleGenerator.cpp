// ============================================================================
//
// ParticleGenerator.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleGenerator.h"

//-----------------------------------------------------------------------------
ParticleGenerator::ParticleGenerator(ParticleEffectAppearance const * const parentParticleEffectAppearance)
 : m_extent_w()
 , m_parentParticleEffectAppearance(NON_NULL(parentParticleEffectAppearance))
{
}

//-----------------------------------------------------------------------------
ParticleEffectAppearance const & ParticleGenerator::getParentParticleEffectAppearance() const
{
	return *NON_NULL(m_parentParticleEffectAppearance);
}

// ============================================================================
