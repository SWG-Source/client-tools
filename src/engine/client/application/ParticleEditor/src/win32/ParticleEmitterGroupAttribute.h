// ============================================================================
//
// ParticleEmitterGroupAttribute.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEmitterGroupAttribute_H
#define INCLUDED_ParticleEmitterGroupAttribute_H

#include "BaseParticleEmitterGroupAttribute.h"

class ParticleEmitterGroupDescription;

//-----------------------------------------------------------------------------
class ParticleEmitterGroupAttribute : public BaseParticleEmitterGroupAttribute
{
	Q_OBJECT

public:

	ParticleEmitterGroupAttribute(QWidget *parent, char const *name);

public slots:

	void onParticleEmitterGroupDescriptionChanged(ParticleEmitterGroupDescription const &particleEmitterGroupDescription);
};

// ============================================================================

#endif // INCLUDED_ParticleEmitterGroupAttribute_H
