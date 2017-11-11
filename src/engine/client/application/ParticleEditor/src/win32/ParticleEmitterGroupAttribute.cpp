// ============================================================================
//
// ParticleEmitterGroupAttribute.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "ParticleEmitterGroupAttribute.h"
#include "ParticleEmitterGroupAttribute.moc"

#include "clientParticle/ParticleEmitterGroupDescription.h"

// ============================================================================
//
// ParticleEmitterGroupAttribute
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleEmitterGroupAttribute::ParticleEmitterGroupAttribute(QWidget *parent, char const *name)
 : BaseParticleEmitterGroupAttribute(parent, name)
{
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroupAttribute::onParticleEmitterGroupDescriptionChanged(ParticleEmitterGroupDescription const &particleEmitterGroupDescription)
{
	// Start time

	ParticleEditorUtility::setLineEditFloat(m_startDelayLineEdit, particleEmitterGroupDescription.getParticleTiming().getStartDelayMin(), ParticleBoundFloat(0.0f, 4096.0f), 2);
	ParticleEditorUtility::setLineEditFloat(m_startDelayRandomLineEdit, particleEmitterGroupDescription.getParticleTiming().getStartDelayMax(), ParticleBoundFloat(0.0f, 4096.0f), 2);

	// Loop time

	ParticleEditorUtility::setLineEditFloat(m_loopDelayLineEdit, particleEmitterGroupDescription.getParticleTiming().getLoopDelayMin(), ParticleBoundFloat(0.0f, 4096.0f), 2);
	ParticleEditorUtility::setLineEditFloat(m_loopDelayRandomLineEdit, particleEmitterGroupDescription.getParticleTiming().getLoopDelayMax(), ParticleBoundFloat(0.0f, 4096.0f), 2);

	// Loop count

	ParticleEditorUtility::setLineEditInt(m_loopCountLineEdit, particleEmitterGroupDescription.getParticleTiming().getLoopCountMin(), ParticleBoundInt(-1, 4096));
	ParticleEditorUtility::setLineEditInt(m_loopCountRandomLineEdit, particleEmitterGroupDescription.getParticleTiming().getLoopCountMax(), ParticleBoundInt(0, 4096));
}

// ============================================================================
