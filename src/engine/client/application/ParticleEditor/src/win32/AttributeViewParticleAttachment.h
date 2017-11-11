// ============================================================================
//
// AttributeViewParticleAttachment.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_AttributeViewParticleAttachment_H
#define INCLUDED_AttributeViewParticleAttachment_H

#include "clientParticle/ParticleAttachmentDescription.h"
#include "PEAttributeView.h"

class AttributeWidgetParticleAttachment;

//-----------------------------------------------------------------------------
class AttributeViewParticleAttachment : public PEAttributeView
{
	Q_OBJECT

public:

	AttributeViewParticleAttachment(QWidget *parentWidget, char const *name);

	virtual void                 setNewCaption(char const *caption);
	void                         setParticleAttachmentDescription(ParticleAttachmentDescription const &particleAttachmentDescription);

	ParticleAttachmentDescription getParticleAttachmentDescription() const;

public slots:

	void onAttributeNameChanged(const QString &);

private:

	AttributeWidgetParticleAttachment *m_attributeWidgetParticleAttachment;
};

// ============================================================================

#endif // INCLUDED_AttributeViewParticleAttachment_H
