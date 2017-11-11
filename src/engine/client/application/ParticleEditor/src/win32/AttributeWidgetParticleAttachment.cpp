// ============================================================================
//
// AttributeWidgetParticleAttachment.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "AttributeWidgetParticleAttachment.h"
#include "AttributeWidgetParticleAttachment.moc"

#include "clientParticle/ParticleAttachmentDescription.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/TreeFile.h"

// ============================================================================
//
// AttributeWidgetParticleAttachment
//
// ============================================================================

ParticleBoundInt AttributeWidgetParticleAttachment::m_boundStartPercent(100, 0);
ParticleBoundInt AttributeWidgetParticleAttachment::m_boundSpawnCount(16, 1);
ParticleBoundInt AttributeWidgetParticleAttachment::m_boundSpawnGroupSize(16, 1);

//-----------------------------------------------------------------------------
AttributeWidgetParticleAttachment::AttributeWidgetParticleAttachment(QWidget *parent, char const *name)
 : BaseAttributeWidgetParticleAttachment(parent, name)
 , m_attachmentPath()
{
	// Messaging

	connect(m_loadAttachmentPushButton, SIGNAL(clicked()), this, SLOT(slotLoadAttachmentPushButtonClicked()));
	connect(m_removeAttachmentPushButton, SIGNAL(clicked()), this, SLOT(slotRemoveAttachmentPushButtonClicked()));
	connect(m_startPercentMinLineEdit, SIGNAL(returnPressed()), this, SLOT(slotStartPercentMinLineEditReturnPressed()));
	connect(m_startPercentMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(slotStartPercentMaxLineEditReturnPressed()));
	connect(m_killAttachmentWhenParticleDiesCheckBox, SIGNAL(clicked()), this, SLOT(slotValidateAndEmit()));
	connect(m_spawnComboBox, SIGNAL(activated(int)), this, SLOT(slotValidateAndEmit(int)));

	// Validators

	QIntValidator *intValidator = new QIntValidator(this, "QIntValidator");
	m_startPercentMinLineEdit->setValidator(intValidator);
	m_startPercentMaxLineEdit->setValidator(intValidator);

	validate();
}

//-----------------------------------------------------------------------------
void AttributeWidgetParticleAttachment::slotStartPercentMinLineEditReturnPressed()
{
	if (ParticleEditorUtility::getInt(m_startPercentMinLineEdit) > ParticleEditorUtility::getInt(m_startPercentMaxLineEdit))
	{
		ParticleEditorUtility::setLineEditInt(m_startPercentMaxLineEdit, ParticleEditorUtility::getInt(m_startPercentMinLineEdit), m_boundStartPercent);
	}

	slotValidateAndEmit();
}

//-----------------------------------------------------------------------------
void AttributeWidgetParticleAttachment::slotStartPercentMaxLineEditReturnPressed()
{
	if (ParticleEditorUtility::getInt(m_startPercentMaxLineEdit) < ParticleEditorUtility::getInt(m_startPercentMinLineEdit))
	{
		ParticleEditorUtility::setLineEditInt(m_startPercentMinLineEdit, ParticleEditorUtility::getInt(m_startPercentMaxLineEdit), m_boundStartPercent);
	}

	slotValidateAndEmit();
}

//-----------------------------------------------------------------------------
void AttributeWidgetParticleAttachment::setAttachmentPath(std::string const &path)
{
	bool result = TreeFile::stripTreeFileSearchPathFromFile(path, m_attachmentPath);

	// If the file is not found in the treefile, assign the full path

	if (!result)
	{
		m_attachmentPath = path;
	}

	// Set the filename and extension

	std::string attachmentFileName(FileNameUtils::get(m_attachmentPath, FileNameUtils::fileName | FileNameUtils::extension));

	m_fileNameLineEdit->setText(attachmentFileName.c_str());

	// Set the drive and path

	std::string attachmentPath(FileNameUtils::get(m_attachmentPath, FileNameUtils::drive | FileNameUtils::directory));

	m_pathLineEdit->setText(attachmentPath.c_str());
}

//-----------------------------------------------------------------------------
void AttributeWidgetParticleAttachment::setDescription(ParticleAttachmentDescription const &particleAttachmentDescription)
{
	// Path

	setAttachmentPath(particleAttachmentDescription.getAttachmentPath());

	// Start percent

	ParticleEditorUtility::setLineEditInt(m_startPercentMinLineEdit, static_cast<int>(particleAttachmentDescription.getStartPercentMin() * 100.0f), m_boundStartPercent);
	ParticleEditorUtility::setLineEditInt(m_startPercentMaxLineEdit, static_cast<int>(particleAttachmentDescription.getStartPercentMax() * 100.0f), m_boundStartPercent);

	// Kill attachment at death

	m_killAttachmentWhenParticleDiesCheckBox->setChecked(particleAttachmentDescription.isKillAttachmentWhenParticleDies());

	// Spawn

	m_spawnComboBox->setCurrentItem(static_cast<int>(particleAttachmentDescription.getSpawn()));

	validate();
}

//-----------------------------------------------------------------------------
void AttributeWidgetParticleAttachment::validate()
{
	// Start percent

	ParticleEditorUtility::validateLineEditInt(m_startPercentMinLineEdit, m_boundStartPercent);
	ParticleEditorUtility::validateLineEditInt(m_startPercentMaxLineEdit, m_boundStartPercent);

	// Spawn

	switch (m_spawnComboBox->currentItem())
	{
		case 0:
		case 1:
		case 3:
			{
				m_startPercentMinLineEdit->setEnabled(false);
				m_startPercentMaxLineEdit->setEnabled(false);
			}
			break;
		case 2:
			{
				m_startPercentMinLineEdit->setEnabled(true);
				m_startPercentMaxLineEdit->setEnabled(true);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("Unexpected spawn method."));
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void AttributeWidgetParticleAttachment::slotLoadAttachmentPushButtonClicked()
{
	// Get an attachment path

	QString attachmentPath(QFileDialog::getOpenFileName(ParticleEditorUtility::getParticleAttachmentPath(), "Any Appearance (*.prt *.swh *.msh *.apt *.sat)", this, "AttachmentFileDialog", "Open Attachment"));

	if (!attachmentPath.isNull())
	{
		// Save the path

		ParticleEditorUtility::setParticleAttachmentPath(attachmentPath.latin1());

		// Save the attachment path

		std::string path(attachmentPath.latin1());
		FileNameUtils::swapChar(path, '/', '\\');
		setAttachmentPath(path);

		// Signal the attachment changed

		slotValidateAndEmit();
	}
}

//-----------------------------------------------------------------------------
void AttributeWidgetParticleAttachment::slotRemoveAttachmentPushButtonClicked()
{
	setAttachmentPath("");

	// Signal the attachment changed

	slotValidateAndEmit();
}

//-----------------------------------------------------------------------------
void AttributeWidgetParticleAttachment::slotValidateAndEmit(int)
{
	validate();
	
	emit signalForceRebuild();
}

//-----------------------------------------------------------------------------
std::string const &AttributeWidgetParticleAttachment::getAttachmentPath() const
{
	return m_attachmentPath;
}

// ============================================================================
