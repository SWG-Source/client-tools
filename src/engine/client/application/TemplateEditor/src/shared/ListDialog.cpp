// ============================================================================
//
// ListDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "ListDialog.h"

#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "sharedTemplateDefinition/TpfTemplate.h"

// ============================================================================
//
// ListDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
ListDialog::ListDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow)
 : DynamicListDialog(parent, name, parentTemplateTableRow)
{
	if (parentTemplateTableRow.isParameterProperty(TemplateTableRow::PP_list))
	{
		TemplateData::Parameter const *parameter = getParentTemplateTableRow().getParameter();

		int const listSize = parentTemplateTableRow.getTpfTemplate()->getListLength(parameter->name.c_str());

		m_templateTable->setTemplateData(parentTemplateTableRow.getParentTable().getTemplateData());

		for (int row = 0; row < listSize; ++row)
		{
			TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, row, parentTemplateTableRow, *parameter, row);
	
			char text[256];
			sprintf(text, "%s[%d]", parentTemplateTableRow.getParameterName().latin1(), row);
		
			templateTableRow->setDecoratedName(text);
			templateTableRow->setChecked(true);
			templateTableRow->setCheckEnabled(false);
			templateTableRow->refresh();
		}
	}
	else
	{
		DEBUG_FATAL(true, ("Unexpected parameter list type."));
	}

	m_templateTable->update();
	m_templateTable->adjustColumn(TemplateTableRow::CT_edited);
	m_templateTable->adjustColumn(TemplateTableRow::CT_legend);
	m_templateTable->adjustColumn(TemplateTableRow::CT_name);
	m_templateTable->adjustColumn(TemplateTableRow::CT_value);
}

// ============================================================================
