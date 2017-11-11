// ============================================================================
//
// PEAttributeViewParticleEmitterGroup.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "PEAttributeViewParticleEmitterGroup.h"
#include "PEAttributeViewParticleEmitterGroup.moc"

#include "ParticleNameAttribute.h"
#include "ParticleTimingAttribute.h"

///////////////////////////////////////////////////////////////////////////////
//
// PEAttributeViewParticleEmitterGroup
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
PEAttributeViewParticleEmitterGroup::PEAttributeViewParticleEmitterGroup(QWidget *parentWidget, char const *name)
 : PEAttributeView(parentWidget, name)
{
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);

	// Name attribute

	m_nameWidget = new ParticleNameAttribute(m_attributesVBox, "ParticleEmitterGroupNameAttribute");
	m_nameWidget->setEnabled(false);

	// Timing attribute

	m_timingAttribute = new ParticleTimingAttribute(m_attributesVBox, "ParticleEmitterGroupTimingAttribute");
	connect(m_timingAttribute, SIGNAL(timingChanged()), parentWidget, SLOT(onAttributeChangedForceRebuild()));

	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitterGroup::setNewCaption(char const *caption)
{
	char text[256];
	sprintf(text, "Emitter Group (%s)", caption);
	(dynamic_cast<QWidget *>(parent()))->setCaption(text);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitterGroup::onAttributeNameChanged(const QString &name)
{
	setNewCaption(name);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitterGroup::setParticleEmitterGroupDescription(ParticleEmitterGroupDescription const &particleEmitterGroupDescription)
{
	m_timingAttribute->setTiming(particleEmitterGroupDescription.getParticleTiming());
}

//-----------------------------------------------------------------------------
ParticleEmitterGroupDescription PEAttributeViewParticleEmitterGroup::getParticleEmitterGroupDescription() const
{
	ParticleEmitterGroupDescription particleEmitterGroupDescription;

	particleEmitterGroupDescription.setParticleTiming(m_timingAttribute->getTiming());

	return particleEmitterGroupDescription;
}

// ============================================================================