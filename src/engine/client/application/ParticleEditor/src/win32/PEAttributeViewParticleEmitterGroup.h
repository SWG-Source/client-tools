// ============================================================================
//
// PEAttributeViewParticleEmitterGroup.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef PEATTRIBUTEVIEWPARTICLEEMITTERGROUP_H
#define PEATTRIBUTEVIEWPARTICLEEMITTERGROUP_H

#include "PEAttributeView.h"
#include "clientParticle/ParticleEmitterGroupDescription.h"

class ParticleNameAttribute;
class ParticleTimingAttribute;

//-----------------------------------------------------------------------------
class PEAttributeViewParticleEmitterGroup : public PEAttributeView
{
	Q_OBJECT

public:

	PEAttributeViewParticleEmitterGroup(QWidget *parentWidget, char const *name);

	void                            setParticleEmitterGroupDescription(ParticleEmitterGroupDescription const &particleEmitterGroupDescription);
	ParticleEmitterGroupDescription getParticleEmitterGroupDescription() const;
	virtual void                    setNewCaption(char const *caption);

public slots:

	void onAttributeNameChanged(const QString &);

private:

	ParticleNameAttribute *  m_nameWidget;
	ParticleTimingAttribute *m_timingAttribute;
};

// ============================================================================

#endif // PEATTRIBUTEVIEWPARTICLEEMITTERGROUP_H