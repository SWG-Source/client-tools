// ============================================================================
//
// PEAttributeViewParticleEffect.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "PEAttributeViewParticleEffect.h"
#include "PEAttributeViewParticleEffect.moc"

#include "clientParticle/ParticleEffectDescription.h"
#include "PEAttributeView.h"
#include "ParticleEffectAttribute.h"


///////////////////////////////////////////////////////////////////////////////
//
// PEAttributeViewParticleEffect
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
PEAttributeViewParticleEffect::PEAttributeViewParticleEffect(QWidget * parentWidget, char const * name)
 : PEAttributeView(parentWidget, name)
 , m_particleEffectAttribute(NULL)
{
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);

	m_particleEffectAttribute = new ParticleEffectAttribute(m_attributesVBox, "ParticleEffectAttribute");
	connect(m_particleEffectAttribute, SIGNAL(dataChanged()), parentWidget, SLOT(onAttributeChangedForceRebuild()));

	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEffect::setNewCaption(char const * caption)
{
	char text[256];
	sprintf(text, "Particle Effect (%s)", caption);
	(dynamic_cast<QWidget *>(parent()))->setCaption(text);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEffect::onAttributeNameChanged(const QString & name)
{
	setNewCaption(name);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEffect::setParticleEffectDescription(ParticleEffectDescription const & particleEffectDescription)
{
	m_particleEffectAttribute->setInitialPlayBackRate(particleEffectDescription.getInitialPlayBackRate());
	m_particleEffectAttribute->setInitialPlayBackRateTime(particleEffectDescription.getInitialPlayBackRateTime());
	m_particleEffectAttribute->setPlayBackRate(particleEffectDescription.getPlayBackRate());
	m_particleEffectAttribute->setScale(particleEffectDescription.getScale());
}

//-----------------------------------------------------------------------------
ParticleEffectDescription const PEAttributeViewParticleEffect::getParticleEffectDescription() const
{
	ParticleEffectDescription particleEffectDescription;

	particleEffectDescription.setInitialPlayBackRate(m_particleEffectAttribute->getInitialPlayBackRate());
	particleEffectDescription.setInitialPlayBackRateTime(m_particleEffectAttribute->getInitialPlayBackRateTime());
	particleEffectDescription.setPlayBackRate(m_particleEffectAttribute->getPlayBackRate());
	particleEffectDescription.setScale(m_particleEffectAttribute->getScale());

	return particleEffectDescription;
}

// ============================================================================
