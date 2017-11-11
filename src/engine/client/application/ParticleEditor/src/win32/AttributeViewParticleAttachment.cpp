// ============================================================================
//
// AttributeViewParticleAttachment.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "AttributeViewParticleAttachment.h"
#include "AttributeViewParticleAttachment.moc"

#include "AttributeWidgetParticleAttachment.h"

///////////////////////////////////////////////////////////////////////////////
//
// AttributeViewParticleAttachment
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
AttributeViewParticleAttachment::AttributeViewParticleAttachment(QWidget *parentWidget, char const *name)
 : PEAttributeView(parentWidget, name)
 , m_attributeWidgetParticleAttachment(NULL)
{
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);

	// Particle attachment attribute

	m_attributeWidgetParticleAttachment = new AttributeWidgetParticleAttachment(m_attributesVBox, "AttributeWidgetParticleAttachment");
	connect(m_attributeWidgetParticleAttachment, SIGNAL(signalForceRebuild()), this, SLOT(onAttributeChangedForceRebuild()));

	NOT_NULL(m_attributeWidgetParticleAttachment);

	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
}

//-----------------------------------------------------------------------------
void AttributeViewParticleAttachment::setNewCaption(char const *caption)
{
	char text[256];
	sprintf(text, "Particle Attachment (%s)", caption);

	(dynamic_cast<QWidget *>(parent()))->setCaption(text);
}

//-----------------------------------------------------------------------------
void AttributeViewParticleAttachment::onAttributeNameChanged(const QString &name)
{
	setNewCaption(name);
}

//-----------------------------------------------------------------------------
void AttributeViewParticleAttachment::setParticleAttachmentDescription(ParticleAttachmentDescription const &particleAttachmentDescription)
{
	m_attributeWidgetParticleAttachment->setDescription(particleAttachmentDescription);
}

//-----------------------------------------------------------------------------
ParticleAttachmentDescription AttributeViewParticleAttachment::getParticleAttachmentDescription() const
{
	ParticleAttachmentDescription particleAttachmentDescription;

	particleAttachmentDescription.setAttachmentPath(m_attributeWidgetParticleAttachment->getAttachmentPath());
	particleAttachmentDescription.setKillAttachmentWhenParticleDies(m_attributeWidgetParticleAttachment->m_killAttachmentWhenParticleDiesCheckBox->isChecked());

	switch (m_attributeWidgetParticleAttachment->m_spawnComboBox->currentItem())
	{
		case 0: // Created
			{
				particleAttachmentDescription.setSpawn(ParticleAttachmentDescription::S_whenParentIsCreated);
				particleAttachmentDescription.setStartPercent(0, 0);
			}
			break;
		case 1: // Dies
			{
				particleAttachmentDescription.setSpawn(ParticleAttachmentDescription::S_whenParentDies);
				particleAttachmentDescription.setStartPercent(1.0f, 1.0f);
			}
			break;
		case 2: // Percent
			{
				particleAttachmentDescription.setSpawn(ParticleAttachmentDescription::S_atPercent);

				float const startPercentMin = static_cast<float>(ParticleEditorUtility::getInt(m_attributeWidgetParticleAttachment->m_startPercentMinLineEdit)) / 100.0f;
				float const startPercentMax = static_cast<float>(ParticleEditorUtility::getInt(m_attributeWidgetParticleAttachment->m_startPercentMaxLineEdit)) / 100.0f;

				particleAttachmentDescription.setStartPercent(startPercentMin, startPercentMax);
			}
			break;
		case 3: // Collision
			{
				particleAttachmentDescription.setSpawn(ParticleAttachmentDescription::S_onCollision);
				particleAttachmentDescription.setStartPercent(1.0f, 1.0f);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("Unexpected spawn method."));
			}
			break;
	}

	return particleAttachmentDescription;
}

// ============================================================================
