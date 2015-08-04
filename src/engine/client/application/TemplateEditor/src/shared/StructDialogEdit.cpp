// ============================================================================
//
// StructDialogEdits.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "StructDialogEdit.h"

#include "TableItemBase.h"
#include "StructDialog.h"
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
// StructDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
StructDialogEdit::StructDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, TemplateTableRow &parentRow)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
{
	m_inputDialog = new StructDialog(this, "StructDialog", parentRow);

	setInputDialog(*m_inputDialog);
}

//-----------------------------------------------------------------------------
void StructDialogEdit::setValue(TpfTemplate &structTemplate)
{
	getStructDialog()->setValue(structTemplate, "");
	setText(getTextFromDialog());
}

//-----------------------------------------------------------------------------
QString StructDialogEdit::getTextFromDialog() const
{
	std::string result;

	result = "[ ";
	
	int itemCount = getStructDialog()->getTemplateTableRowCount();

	for(int itemCurrent = 0; itemCurrent < itemCount; ++itemCurrent)
	{
		const TemplateTableRow *currRow = getStructDialog()->getTemplateTableRow(itemCurrent);

		QString parameterName(currRow->getParameterName());

		result += parameterName;
		result += " = ";

		TableItemBase *data = currRow->getColumnValue();

		result += data->text();
		
		if(itemCurrent != itemCount - 1)
		{
			 result += ", ";
		}
	}

	result += " ]";

	return result.c_str();
}

//-----------------------------------------------------------------------------
StructDialog *StructDialogEdit::getStructDialog() const
{
	StructDialog *structDialog = dynamic_cast<StructDialog *>(m_inputDialog);
	NOT_NULL(structDialog);

	return structDialog;
}

// ============================================================================
