// ============================================================================
//
// VectorDialogEdits.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "VectorDialogEdit.h"

#include "TableItemBase.h"
#include "VectorDialog.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "sharedTemplateDefinition/File.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateDataIterator.h"
#include "sharedTemplateDefinition/TemplateDefinitionFile.h"
#include "sharedTemplateDefinition/TpfFile.h"
#include "TemplateLoader.h"

// ============================================================================
//
// VectorDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
VectorDialogEdit::VectorDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
{
	m_inputDialog = new VectorDialog(this, "VectorDialog", parentTableItemBase.getTemplateTableRow());

	setInputDialog(*m_inputDialog);
}

//-----------------------------------------------------------------------------
void VectorDialogEdit::setValue(const VectorParamData &vectorParamData)
{
	getVectorDialog()->setValue(vectorParamData);

	setText(getTextFromDialog());
}

//-----------------------------------------------------------------------------
QString VectorDialogEdit::getTextFromDialog() const
{
	QString resultText;

	bool ignoreY = false;

	if(getVectorDialog()->ignoreIndexY())
	{
		ignoreY = true;
	}

	resultText += getVectorDialog()->getCoordXText();
	resultText += " ";

	if(!ignoreY)
	{
		resultText += getVectorDialog()->getCoordYText();
		resultText += " ";
	}

	resultText += getVectorDialog()->getCoordZText();
	resultText += " ";

	resultText += getVectorDialog()->getRadiusText();
	resultText;

	return resultText;
}

//-----------------------------------------------------------------------------
VectorDialog *VectorDialogEdit::getVectorDialog() const
{
	VectorDialog *vectorDialog = dynamic_cast<VectorDialog *>(m_inputDialog);
	NOT_NULL(vectorDialog);

	return vectorDialog;
}

// ============================================================================
