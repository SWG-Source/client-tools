// ============================================================================
//
// BaseDialogEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "BaseDialogEdit.h"
#include "BaseDialogEdit.moc"

// ============================================================================
//
// BaseDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
BaseDialogEdit::BaseDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : DataTypeEdit(parentWidget, name, parentTableItemBase)
 , m_inputDialog(NULL)
 , m_openDialogPushButton(NULL)
{
	// This line edit just shows the current value of the associated dialog, so
	// we are setting it to read only

	m_textLineEdit = new QLineEdit(this);
	m_textLineEdit->setReadOnly(true);
	
	// This push button is to the right of the line edit. This allows us to open
	// a dialog with more complex entry capabilities.

	m_openDialogPushButton = new QPushButton("...", this, "QPushButton");

	connect(m_openDialogPushButton, SIGNAL(pressed()), this, SLOT(getUserDialogInput()));

	setFocusTarget(*m_openDialogPushButton);
}

//-----------------------------------------------------------------------------
void BaseDialogEdit::setTextLineEditReadOnly(bool const readOnly)
{
	m_textLineEdit->setReadOnly(readOnly);
}

//-----------------------------------------------------------------------------
void BaseDialogEdit::init()
{
}

//-----------------------------------------------------------------------------
void BaseDialogEdit::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
	if (mouseEvent != NULL)
	{
		getUserDialogInput();

		mouseEvent->accept();
	}
}

//-----------------------------------------------------------------------------
void BaseDialogEdit::setInputDialog(QDialog &dialog)
{
	m_inputDialog = &dialog;
}

//-----------------------------------------------------------------------------
QPushButton *BaseDialogEdit::getOpenDialogButton()
{
	return m_openDialogPushButton;
}

//-----------------------------------------------------------------------------
void BaseDialogEdit::setText(QString const &text)
{
	m_textLineEdit->setText(text);
}

//-----------------------------------------------------------------------------
void BaseDialogEdit::getUserDialogInput()
{
	if (m_inputDialog != NULL)
	{
		if (m_inputDialog->exec() == QDialog::Accepted)
		{
			setText(getTextFromDialog());
		}

		m_openDialogPushButton->setDown(false); // Make sure the button pops up
	}
}

//-----------------------------------------------------------------------------
QString BaseDialogEdit::getText() const
{
	return getTextFromDialog();
}

//-----------------------------------------------------------------------------
QLineEdit const *BaseDialogEdit::getLineEdit() const
{
	return m_textLineEdit;
}

// ============================================================================
