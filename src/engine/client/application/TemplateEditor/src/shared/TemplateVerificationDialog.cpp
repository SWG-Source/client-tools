8// ============================================================================
//
// TemplateVerificationDialog.cpp
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TemplateVerificationDialog.h"
#include "TemplateVerificationDialog.moc"

#include "TemplateEditorUtility.h"

//-----------------------------------------------------------------------------
TemplateVerificationDialog::TemplateVerificationDialog(QWidget *parent, const char *name, int const templateCount, int const directoryCount)
 : BaseTemplateVerificationDialog(parent, name, true, !Qt::WDestructiveClose)
 , m_templateMin(1)
 , m_templateMax(templateCount)
{
	QString text;
	text.sprintf("%d", m_templateMax);
	m_templateCountTextLabel->setText(text);

	text.sprintf("%d", directoryCount);
	m_directoryCountTextLabel->setText(text);

	QIntValidator *intValidator = new QIntValidator(this, "QIntValidator");
	m_firstLineEdit->setValidator(intValidator);
	m_lastLineEdit->setValidator(intValidator);

	m_firstLineEdit->setText("0");
	
	text.sprintf("%d", m_templateMax);
	m_lastLineEdit->setText(text);

	slotCheckAllRadioButtonClicked();

	connect(m_checkAllRadioButton, SIGNAL(clicked()), this, SLOT(slotCheckAllRadioButtonClicked()));
	connect(m_checkOneRadioButton, SIGNAL(clicked()), this, SLOT(slotCheckOneRadioButtonClicked()));
	connect(m_checkSubsetRadioButton, SIGNAL(clicked()), this, SLOT(slotCheckSubsetRadioButtonClicked()));
	connect(m_firstLineEdit, SIGNAL(returnPressed()), this, SLOT(slotFirstLineEditReturnPressed()));
	connect(m_lastLineEdit, SIGNAL(returnPressed()), this, SLOT(slotLastLineEditReturnPressed()));
	connect(m_cancelPushButton, SIGNAL(clicked()), this, SLOT(slotCancelPushButtonClicked()));
	connect(m_openPushButton, SIGNAL(clicked()), this, SLOT(slotOpenPushButtonClicked()));
}

//-----------------------------------------------------------------------------
void TemplateVerificationDialog::slotFirstLineEditReturnPressed()
{
	TemplateEditorUtility::validateLineEditInt(m_firstLineEdit, m_templateMin, m_templateMax);

	if (TemplateEditorUtility::getInt(m_firstLineEdit) > TemplateEditorUtility::getInt(m_lastLineEdit))
	{
		// Set the last value to the first value

		TemplateEditorUtility::setLineEditInt(m_lastLineEdit, TemplateEditorUtility::getInt(m_firstLineEdit), m_templateMin, m_templateMax);
	}

	TemplateEditorUtility::validateLineEditInt(m_lastLineEdit, m_templateMin, m_templateMax);
}

//-----------------------------------------------------------------------------
void TemplateVerificationDialog::slotLastLineEditReturnPressed()
{
	TemplateEditorUtility::validateLineEditInt(m_lastLineEdit, m_templateMin, m_templateMax);

	if (TemplateEditorUtility::getInt(m_lastLineEdit) < TemplateEditorUtility::getInt(m_firstLineEdit))
	{
		// Set the last value to the first value

		TemplateEditorUtility::setLineEditInt(m_firstLineEdit, TemplateEditorUtility::getInt(m_lastLineEdit), m_templateMin, m_templateMax);
	}

	TemplateEditorUtility::validateLineEditInt(m_firstLineEdit, m_templateMin, m_templateMax);
}

//-----------------------------------------------------------------------------
void TemplateVerificationDialog::slotCancelPushButtonClicked()
{
	BaseTemplateVerificationDialog::reject();
}

//-----------------------------------------------------------------------------
void TemplateVerificationDialog::slotOpenPushButtonClicked()
{
	slotFirstLineEditReturnPressed();

	BaseTemplateVerificationDialog::accept();
}

//-----------------------------------------------------------------------------
void TemplateVerificationDialog::slotCheckAllRadioButtonClicked()
{
	slotCheckOneRadioButtonClicked();
}

//-----------------------------------------------------------------------------
void TemplateVerificationDialog::slotCheckOneRadioButtonClicked()
{
	m_firstLineEdit->setEnabled(false);
	m_lastLineEdit->setEnabled(false);

	m_firstLineEdit->setText("");
	m_lastLineEdit->setText("");
}

//-----------------------------------------------------------------------------
void TemplateVerificationDialog::slotCheckSubsetRadioButtonClicked()
{
	m_firstLineEdit->setEnabled(true);
	m_lastLineEdit->setEnabled(true);

	// Set the default text

	m_firstLineEdit->setText("0");
	
	QString text;
	text.sprintf("%d", m_templateMax);
	m_lastLineEdit->setText(text);
}

// ============================================================================
