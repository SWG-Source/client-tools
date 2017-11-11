// ============================================================================
//
// PEAttributeViewer.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "PEAttributeViewer.h"
#include "PEAttributeViewer.moc"

#include "AttributeViewParticleAttachment.h"
#include "clientParticle/ParticleDescriptionMesh.h"
#include "clientParticle/ParticleDescriptionQuad.h"
#include "clientParticle/ParticleEffectDescription.h"
#include "clientParticle/ParticleEmitterDescription.h"
#include "ParticleAttribute.h"
#include "ParticleEmitterAttribute.h"
#include "ParticleEmitterGroupAttribute.h"
#include "PEAttributeView.h"
#include "PEAttributeViewParticleQuad.h"
#include "PEAttributeViewParticleMesh.h"
#include "PEAttributeViewParticleEmitter.h"
#include "PEAttributeViewParticleEmitterGroup.h"
#include "PEAttributeViewParticleEffect.h"
#include "PEAttributeViewParticleEffectGroup.h"

///////////////////////////////////////////////////////////////////////////////
//
// PEAttributeViewer
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
PEAttributeViewer::PEAttributeViewer(QWidget *parentWidget, char const *name, WFlags const flags)
 : QWidget(parentWidget, name, flags)
 , m_attributeView(AV_invalid)
 , m_attributeViewParticleEffect(NULL)
 , m_attributeViewParticleEffectGroup(NULL)
 , m_attributeViewParticleEmitter(NULL)
 , m_attributeViewParticleEmitterGroup(NULL)
 , m_attributeViewParticleMesh(NULL)
 , m_attributeViewParticleQuad(NULL)
 , m_attributeViewParticleAttachment(NULL)
 , m_attributeViewSelected(NULL)
 , m_lastParticlePosition(0, 0)
 , m_lastEmitterPosition(0, 0)

{
	int const minimumHeight = 78 * 3;
	setMinimumHeight(minimumHeight);

	m_attributeViewParticleAttachment = new AttributeViewParticleAttachment(this, "AttributeViewParticleAttachment");
	connect(m_attributeViewParticleAttachment, SIGNAL(attributeChangedForceRebuild()), this, SLOT(onAttributeChangedForceRebuild()));

	m_attributeViewParticleMesh = new PEAttributeViewParticleMesh(this, "PEAttributeViewParticleMesh");
	connect(m_attributeViewParticleMesh, SIGNAL(waveFormAttributeSelected(const WaveForm &)), this, SLOT(onWaveFormAttributeSelected(const WaveForm &)));
	connect(m_attributeViewParticleMesh, SIGNAL(colorRampAttributeSelected(const ColorRamp &, const WaveForm &)), this, SLOT(onColorRampAttributeSelected(const ColorRamp &, const WaveForm &)));
	connect(m_attributeViewParticleMesh, SIGNAL(attributeChangedForceRebuild()), this, SLOT(onAttributeChangedForceRebuild()));

	m_attributeViewParticleQuad = new PEAttributeViewParticleQuad(this, "PEAttributeViewParticleQuad");
	connect(m_attributeViewParticleQuad, SIGNAL(waveFormAttributeSelected(const WaveForm &)), this, SLOT(onWaveFormAttributeSelected(const WaveForm &)));
	connect(m_attributeViewParticleQuad, SIGNAL(colorRampAttributeSelected(const ColorRamp &, const WaveForm &)), this, SLOT(onColorRampAttributeSelected(const ColorRamp &, const WaveForm &)));
	connect(m_attributeViewParticleQuad, SIGNAL(attributeChangedForceRebuild()), this, SLOT(onAttributeChangedForceRebuild()));

	m_attributeViewParticleEmitter = new PEAttributeViewParticleEmitter(this, "PEAttributeViewParticleEmitter");
	connect(m_attributeViewParticleEmitter, SIGNAL(waveFormAttributeSelected(const WaveForm &)), this, SLOT(onWaveFormAttributeSelected(const WaveForm &)));
	connect(m_attributeViewParticleEmitter, SIGNAL(colorRampAttributeSelected(const ColorRamp &, const WaveForm &)), this, SLOT(onColorRampAttributeSelected(const ColorRamp &, const WaveForm &)));
	connect(m_attributeViewParticleEmitter, SIGNAL(attributeChangedForceRebuild()), this, SLOT(onAttributeChangedForceRebuild()));

	m_attributeViewParticleEmitterGroup = new PEAttributeViewParticleEmitterGroup(this, "PEAttributeViewParticleEmitterGroup");
	connect(m_attributeViewParticleEmitterGroup, SIGNAL(waveFormAttributeSelected(const WaveForm &)), this, SLOT(onWaveFormAttributeSelected(const WaveForm &)));
	connect(m_attributeViewParticleEmitterGroup, SIGNAL(colorRampAttributeSelected(const ColorRamp &, const WaveForm &)), this, SLOT(onColorRampAttributeSelected(const ColorRamp &, const WaveForm &)));
	connect(m_attributeViewParticleEmitterGroup, SIGNAL(attributeChangedForceRebuild()), this, SLOT(onAttributeChangedForceRebuild()));

	m_attributeViewParticleEffect = new PEAttributeViewParticleEffect(this, "PEAttributeViewParticleEffect");
	connect(m_attributeViewParticleEffect, SIGNAL(waveFormAttributeSelected(const WaveForm &)), this, SLOT(onWaveFormAttributeSelected(const WaveForm &)));
	connect(m_attributeViewParticleEffect, SIGNAL(colorRampAttributeSelected(const ColorRamp &, const WaveForm &)), this, SLOT(onColorRampAttributeSelected(const ColorRamp &, const WaveForm &)));
	connect(m_attributeViewParticleEffect, SIGNAL(attributeChangedForceRebuild()), this, SLOT(onAttributeChangedForceRebuild()));

	m_attributeViewParticleEffectGroup = new PEAttributeViewParticleEffectGroup(this, "PEAttributeViewParticleEffectGroup");
	connect(m_attributeViewParticleEffectGroup, SIGNAL(waveFormAttributeSelected(const WaveForm &)), this, SLOT(onWaveFormAttributeSelected(const WaveForm &)));
	connect(m_attributeViewParticleEffectGroup, SIGNAL(colorRampAttributeSelected(const ColorRamp &, const WaveForm &)), this, SLOT(onColorRampAttributeSelected(const ColorRamp &, const WaveForm &)));
	connect(m_attributeViewParticleEffectGroup, SIGNAL(attributeChangedForceRebuild()), this, SLOT(onAttributeChangedForceRebuild()));

	onShowParticleEffectGroupAttributes();
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::hideAll()
{
	if (m_attributeViewSelected != NULL)
	{
		QWidget *widget = m_attributeViewSelected->cornerWidget();
		QPoint cornerPosition(0, 0);

		if (widget != NULL)
		{
			cornerPosition = widget->pos();
		}

		if (m_attributeViewParticleMesh->isVisible() ||
		    m_attributeViewParticleQuad->isVisible())
		{
			m_lastParticlePosition = cornerPosition;
		}
		else if (m_attributeViewParticleEmitter->isVisible())
		{
			m_lastEmitterPosition = cornerPosition;
		}
	}

	m_attributeViewParticleEffect->hide();
	m_attributeViewParticleEffectGroup->hide();
	m_attributeViewParticleEmitter->hide();
	m_attributeViewParticleEmitterGroup->hide();
	m_attributeViewParticleMesh->hide();
	m_attributeViewParticleQuad->hide();
	m_attributeViewParticleAttachment->hide();
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onShowParticleEffectGroupAttributes()
{
	hideAll();
	m_attributeViewSelected = m_attributeViewParticleEffectGroup;
	m_attributeView = AV_particleEffectGroup;
	resizeEvent(NULL);
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onShowParticleEffectAttributes(const ParticleEffectDescription &particleEffectDescription)
{
	UNREF(particleEffectDescription);

	hideAll();
	m_attributeViewSelected = m_attributeViewParticleEffect;
	m_attributeViewParticleEffect->setNewCaption(particleEffectDescription.getName().c_str());
	m_attributeViewParticleEffect->setParticleEffectDescription(particleEffectDescription);
	m_attributeView = AV_particleEffect;
	resizeEvent(NULL);
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onShowParticleEmitterGroupAttributes(const ParticleEmitterGroupDescription &particleEmitterGroupDescription)
{
	hideAll();
	m_attributeViewSelected = m_attributeViewParticleEmitterGroup;
	m_attributeViewParticleEmitterGroup->setNewCaption("Default");
	m_attributeViewParticleEmitterGroup->setParticleEmitterGroupDescription(particleEmitterGroupDescription);
	m_attributeView = AV_particleEmitterGroup;
	resizeEvent(NULL);
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onShowParticleEmitterAttributes(const ParticleEmitterDescription &particleEmitterDescription)
{
	hideAll();
	m_attributeViewSelected = m_attributeViewParticleEmitter;
#ifdef _DEBUG
	m_attributeViewParticleEmitter->setNewCaption(particleEmitterDescription.m_emitterName.c_str());
#endif // _DEBUG
	m_attributeViewParticleEmitter->setParticleEmitterDescription(particleEmitterDescription);
	m_attributeView = AV_particleEmitter;
	resizeEvent(NULL);
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onShowParticleAttributes(const ParticleDescription &particleDescription)
{
	hideAll();

	if (particleDescription.getParticleType() == ParticleDescription::PT_quad)
	{
		ParticleDescriptionQuad const *particleDescrptionQuad = safe_cast<ParticleDescriptionQuad const *>(&particleDescription);

		m_attributeView = AV_particleQuad;
		m_attributeViewSelected = m_attributeViewParticleQuad;
		m_attributeViewParticleQuad->setNewCaption(particleDescrptionQuad->getName().c_str());
		m_attributeViewParticleQuad->setParticleDescriptionQuad(*particleDescrptionQuad);
	}
	else if (particleDescription.getParticleType() == ParticleDescription::PT_mesh)
	{
		ParticleDescriptionMesh const *particleDescriptionMesh = safe_cast<ParticleDescriptionMesh const *>(&particleDescription);

		m_attributeView = AV_particleMesh;
		m_attributeViewSelected = m_attributeViewParticleMesh;
		m_attributeViewParticleMesh->setNewCaption(particleDescriptionMesh->getName().c_str());
		m_attributeViewParticleMesh->setParticleDescriptionMesh(*particleDescriptionMesh);
	}
	else
	{
		DEBUG_FATAL(true, ("PEAttributeViewer::onShowParticleAttributes() - Un-supported particle type."));
	}
	
	resizeEvent(NULL);
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onShowParticleAttachmentAttributes(const ParticleAttachmentDescription &particleAttachmentDescription)
{
	hideAll();
	m_attributeViewSelected = m_attributeViewParticleAttachment;
	m_attributeViewParticleAttachment->setNewCaption(particleAttachmentDescription.getAttachmentPath().c_str());
	m_attributeViewParticleAttachment->setParticleAttachmentDescription(particleAttachmentDescription);
	m_attributeView = AV_particleAttachment;
	resizeEvent(NULL);
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::resizeEvent(QResizeEvent *)
{
	if (m_attributeViewSelected != NULL)
	{
		m_attributeViewSelected->show();
		m_attributeViewSelected->resize(width(), height());

		switch (m_attributeView)
		{
			case AV_particleQuad:
			case AV_particleMesh:
			case AV_particleEmitter:
				{
					m_attributeViewSelected->ensureVisible(m_lastEmitterPosition.x(), m_lastEmitterPosition.y());
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onWaveFormChanged(const WaveForm &waveForm, const bool forceRebuild)
{
	if (m_attributeViewSelected != NULL)
	{
		m_attributeViewSelected->setAttributeWaveForm(waveForm);
	}

	if (forceRebuild)
	{
		onAttributeChangedForceRebuild();
	}
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onColorRampChanged(const ColorRamp &colorRamp, const WaveForm &alphaWaveForm, const bool forceRebuild)
{
	if (m_attributeViewSelected != NULL)
	{
		m_attributeViewSelected->setAttributeColorRamp(colorRamp, alphaWaveForm);
	}

	if (forceRebuild)
	{
		onAttributeChangedForceRebuild();
	}
}

//-----------------------------------------------------------------------------
PEAttributeViewParticleEmitter const &PEAttributeViewer::getAttributeViewParticleEmitter() const
{
	return *m_attributeViewParticleEmitter;
}

//-----------------------------------------------------------------------------
PEAttributeViewParticleEmitterGroup const &PEAttributeViewer::getAttributeViewParticleEmitterGroup() const
{
	return *m_attributeViewParticleEmitterGroup;
}

//-----------------------------------------------------------------------------
PEAttributeViewParticleEffect const &PEAttributeViewer::getAttributeViewParticleEffect() const
{
	return *m_attributeViewParticleEffect;
}

//-----------------------------------------------------------------------------
PEAttributeViewParticleEffectGroup const &PEAttributeViewer::getAttributeViewParticleEffectGroup() const
{
	return *m_attributeViewParticleEffectGroup;
}

//-----------------------------------------------------------------------------
PEAttributeViewParticleMesh const &PEAttributeViewer::getAttributeViewParticleMesh() const
{
	return *m_attributeViewParticleMesh;
}

//-----------------------------------------------------------------------------
PEAttributeViewParticleQuad const &PEAttributeViewer::getAttributeViewParticleQuad() const
{
	return *m_attributeViewParticleQuad;
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onColorRampAttributeSelected(const ColorRamp &colorRamp, const WaveForm &waveForm)
{
	emit colorRampAttributeSelected(colorRamp, waveForm);
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onWaveFormAttributeSelected(const WaveForm &waveForm)
{
	emit waveFormAttributeSelected(waveForm);
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onAttributeChanged()
{
	emit attributeChanged();
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onAttributeChangedForceRebuild(int)
{
	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onAttributeNameChanged(const QString &name)
{
	emit attributeNameChanged(static_cast<char const *>(name));
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::onAttributeChangedForceRebuild()
{
	// Recreate the changes

	switch (m_attributeView)
	{
		case AV_particleEffectGroup:
			{
			}
			break;
		case AV_particleEffect:
			{
				emit particleEffectDescriptionChanged(m_attributeViewParticleEffect->getParticleEffectDescription());
			}
			break;
		case AV_particleEmitterGroup:
			{
				emit particleEmitterGroupDescriptionChanged(m_attributeViewParticleEmitterGroup->getParticleEmitterGroupDescription());
			}
			break;
		case AV_particleEmitter:
			{
				emit particleEmitterDescriptionChanged(m_attributeViewParticleEmitter->getParticleEmitterDescription());
			}
			break;
		case AV_particleMesh:
			{
				ParticleDescriptionMesh const &particleDescriptionMesh = m_attributeViewParticleMesh->getParticleDescriptionMesh();

				emit particleDescriptionChanged(particleDescriptionMesh);
			}
			break;
		case AV_particleQuad:
			{
				ParticleDescriptionQuad const &particleDescriptionQuad = m_attributeViewParticleQuad->getParticleDescriptionQuad();

				emit particleDescriptionChanged(particleDescriptionQuad);
			}
			break;
		case AV_particleAttachment:
			{
				ParticleAttachmentDescription const &particleAttachmentDescription = m_attributeViewParticleAttachment->getParticleAttachmentDescription();

				emit particleAttachmentDescriptionChanged(particleAttachmentDescription);
			}
			break;
		default:
			{
				//assert(0);
			}
	}

	// Signal the that some attribute changed and the AV_particleMesh effect
	// needs to be rebuilt

	emit buildParticleEffect();
}

//-----------------------------------------------------------------------------
void PEAttributeViewer::slotSwitchAttributeView(PEAttributeViewer::AttributeView const attributeView)
{
	// Set the new attribute view

	m_attributeView = attributeView;

	// Propogate the changes to everything

	onAttributeChangedForceRebuild();
}

// ============================================================================
