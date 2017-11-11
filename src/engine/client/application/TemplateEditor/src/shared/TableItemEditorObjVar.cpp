// ============================================================================
//
// TableItemEditorObjVar.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemEditorObjVar.h"
#include "TableItemBase.h"
#include "ObjvarDialog.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// TableItemEditorObjVar
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemEditorObjVar::TableItemEditorObjVar(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, TemplateTableRow &parentRow)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
{
	m_inputDialog = new ObjvarDialog(this, "ObjvarDialog", parentRow);

	setInputDialog(*m_inputDialog);
}

//-----------------------------------------------------------------------------
void TableItemEditorObjVar::setValue(const DynamicVariableParamData &dynamicVariableParamData)
{
	getObjvarDialog()->init(dynamicVariableParamData);
}

//-----------------------------------------------------------------------------
QString TableItemEditorObjVar::getTextFromDialog() const
{
	QString result;

	if (getTableItemBase().getTemplateTableRow().getListType() == TemplateTableRow::PP_list)
	{
		result += "[ ";
		result += getObjvarDialog()->getValue().latin1();
		result += " ]";
	}
	else
	{
		result = getObjvarDialog()->getValue().latin1();
	}

	return result;
}

//-----------------------------------------------------------------------------
ObjvarDialog *TableItemEditorObjVar::getObjvarDialog() const
{
	ObjvarDialog *objvarDialog = dynamic_cast<ObjvarDialog *>(m_inputDialog);
	NOT_NULL(objvarDialog);

	return objvarDialog;
}

// ============================================================================
