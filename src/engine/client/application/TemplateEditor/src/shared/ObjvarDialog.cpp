// ============================================================================
//
// ObjvarDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "ObjvarDialog.h"

#include "TableItemBase.h"
#include "TableItemObjvarType.h"
#include "DialogObjvarList.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "sharedUtility/TemplateParameter.h"
#include "sharedFoundation/dynamicVariableList.h"

// ============================================================================
//
// ObjvarDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
ObjvarDialog::ObjvarDialog(QWidget *parent, char const *name, TemplateTableRow &parentTemplateTableRow)
 : StaticListDialog(parent, name, parentTemplateTableRow)
 , m_nameTemplateTableRow(NULL)
 , m_valueTemplateTableRow(NULL)
 , m_dialogObjVarList(NULL)
{
	// Add the name and value rows

	m_nameTemplateTableRow = new TemplateTableRow(*m_templateTable, 0, TemplateData::TYPE_STRING, "Name");
	m_nameTemplateTableRow->setChecked(true);
	m_nameTemplateTableRow->setCheckEnabled(false);
	m_nameTemplateTableRow->getColumnValue()->initDefault();

	m_valueTemplateTableRow = new TemplateTableRow(*m_templateTable, 1, TemplateData::TYPE_DYNAMIC_VAR, "Value");
	m_valueTemplateTableRow->setChecked(true);
	m_valueTemplateTableRow->setCheckEnabled(false);

	// Create the editor
	
	m_dialogObjVarList = new DialogObjvarList(this, "DialogObjvarList", parentTemplateTableRow);
}

//-----------------------------------------------------------------------------
void ObjvarDialog::init(DynamicVariableParamData const &dynamicVariableParamData)
{
	// Set the name column

	StringParam stringParam;
	stringParam.setValue(dynamicVariableParamData.m_name);
	m_nameTemplateTableRow->getColumnValue()->init(&stringParam, 0, 0);

	// Initialize the objVar edit dialog

	m_dialogObjVarList->init(&dynamicVariableParamData);
}

//-----------------------------------------------------------------------------
QString ObjvarDialog::getName() const
{
	QString result(m_nameTemplateTableRow->getColumnValue()->text().latin1());

	return result;
}

//-----------------------------------------------------------------------------
QString ObjvarDialog::getValue() const
{
	QString result;

	if (getName().length() > 2) // This is a hack, but I don't have time to play around with this goofy implementation now.
	{
		result += getName();
		result += " = ";
		
		if (getTemplateTableRow().getTemplateDataParamType() == TemplateData::TYPE_DYNAMIC_VAR)
		{
			result += "[ " + m_dialogObjVarList->getText() + " ]";
		}
		else
		{
			result += m_dialogObjVarList->getText();
		}
	}
	
	return result;
}

// ============================================================================
