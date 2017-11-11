// ============================================================================
//
// TriggerVolumeDialogEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TriggerVolumeDialogEdit.h"

#include "TableItemBase.h"
#include "TriggerVolumeDialog.h"
#include "sharedUtility/TemplateParameter.h"

// ============================================================================
//
// TriggerVolumeDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
TriggerVolumeDialogEdit::TriggerVolumeDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
{
	m_inputDialog = new TriggerVolumeDialog(this, "TriggerVolumeDialog", parentTableItemBase.getTemplateTableRow());

	setInputDialog(*m_inputDialog);
}

//-----------------------------------------------------------------------------
void TriggerVolumeDialogEdit::setValue(TriggerVolumeParamData const &triggerVolumeParamData)
{
	getTriggerVolumeDialog()->setValue(triggerVolumeParamData);

	setText(getTextFromDialog());
}

//-----------------------------------------------------------------------------
QString TriggerVolumeDialogEdit::getTextFromDialog() const
{
	QString result;

	// Trigger volume value

	result = getTriggerVolumeDialog()->getName() + " " + getTriggerVolumeDialog()->getRadius();

	// Weighted percent

	if (isWeighted())
	{
		result += ":" + QString::number(getWeight());
	}

	return result;
}

//-----------------------------------------------------------------------------
TriggerVolumeDialog *TriggerVolumeDialogEdit::getTriggerVolumeDialog() const
{
	TriggerVolumeDialog *triggerVolumeDialog = dynamic_cast<TriggerVolumeDialog *>(m_inputDialog);
	NOT_NULL(triggerVolumeDialog);

	return triggerVolumeDialog;
}

// ============================================================================
