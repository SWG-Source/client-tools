// ============================================================================
//
// AttributeWidgetParticleAttachment.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_AttributeWidgetParticleAttachment_H
#define INCLUDED_AttributeWidgetParticleAttachment_H

#include "BaseAttributeWidgetParticleAttachment.h"
#include "ParticleEditorUtility.h"

class ParticleAttachmentDescription;

//-----------------------------------------------------------------------------
class AttributeWidgetParticleAttachment : public BaseAttributeWidgetParticleAttachment
{
	Q_OBJECT

public:

	AttributeWidgetParticleAttachment(QWidget *parentWidget, char const *name);

	void               setDescription(ParticleAttachmentDescription const &particleAttachmentDescription);
	std::string const &getAttachmentPath() const;

private slots:

	void slotLoadAttachmentPushButtonClicked();
	void slotRemoveAttachmentPushButtonClicked();
	void slotValidateAndEmit(int = 0);
	void slotStartPercentMinLineEditReturnPressed();
	void slotStartPercentMaxLineEditReturnPressed();

signals:

	void signalForceRebuild();

protected:

private:

	static ParticleBoundInt m_boundStartPercent;
	static ParticleBoundInt m_boundSpawnCount;
	static ParticleBoundInt m_boundSpawnGroupSize;

	std::string m_attachmentPath;

	void setAttachmentPath(std::string const &path);
	void validate();
};

// ============================================================================

#endif // INCLUDED_AttributeWidgetParticleAttachment_H
