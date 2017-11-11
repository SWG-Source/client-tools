// ============================================================================
//
// PEAttributeViewParticleEffect.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_AttributeViewParticleEffect_H
#define INCLUDED_AttributeViewParticleEffect_H

#include "PEAttributeView.h"

class ParticleEffectAttribute;
class ParticleEffectDescription;

//-----------------------------------------------------------------------------
class PEAttributeViewParticleEffect : public PEAttributeView
{
	Q_OBJECT

public:

	PEAttributeViewParticleEffect(QWidget * parentWidget, char const * name);

	void setParticleEffectDescription(ParticleEffectDescription const & particleEffectDescription);
	ParticleEffectDescription const getParticleEffectDescription() const;

	virtual void setNewCaption(char const * caption);

public slots:

	void onAttributeNameChanged(const QString &);

private:

	ParticleEffectAttribute * m_particleEffectAttribute;
};

// ============================================================================

#endif // INCLUDED_AttributeViewParticleEffect_H
