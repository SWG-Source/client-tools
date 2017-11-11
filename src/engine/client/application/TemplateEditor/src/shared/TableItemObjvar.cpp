// ============================================================================
//
// TableItemObjvar.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemObjvar.h"

#include "ListDialogEdit.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedUtility/TemplateParameter.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "TableItemEditorObjVar.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// TableItemObjvar
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemObjvar::TableItemObjvar(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
 , m_value(NULL)
{
}

//-----------------------------------------------------------------------------
void TableItemObjvar::initDefault()
{
	//initAsSingle(NULL);
}

//-----------------------------------------------------------------------------
void TableItemObjvar::init(DynamicVariableParamData const *dynamicVariableParamData)
{
	NOT_NULL(dynamicVariableParamData);

	//Initialize the table Item

	initAsSingle(dynamicVariableParamData);

	TableItemBase::initializeEditor();
}

//-----------------------------------------------------------------------------
void TableItemObjvar::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
	UNREF(weightedListIndex);
	UNREF(parameterProperty);

	// See if we need to change this to a simple type

	TemplateData::ListType listType = getTemplateTableRow().getParameter()->list_type;

	if (getTemplateTableRow().isSingle())
	{
		listType = TemplateData::LIST_NONE;
	}

	// Initialize depending on the list type

	if (getTemplateTableRow().isParameterDerived())
	{
		initAsDerived();
	}
	else
	{
		switch (listType)
		{
			case TemplateData::LIST_NONE:
				{
					DynamicVariableParam const *dynamicVariableParam = tpfTemplate->getDynamicVariableParam(parameterName.latin1(), true, parameterIndex);
					NOT_NULL(dynamicVariableParam);

					DynamicVariableParamData const *dynamicVariableParamData = &dynamicVariableParam->getValue();

					//if (dynamicVariableParam->getValue().m_type == DynamicVariableParamData::LIST)
					//{
					//	dynamicVariableParamData = *dynamicVariableParam->getValue().m_data.lparam->begin();
					//}
					//else
					//{
					//	DEBUG_FATAL(true, ("Unexpected type of dynamic variable."));
					//}

					initAsSingle(dynamicVariableParamData);
				}
				break;
			case TemplateData::LIST_LIST:
				{
					initAsList();
				}
				break;
			case TemplateData::LIST_INT_ARRAY:
			case TemplateData::LIST_ENUM_ARRAY:
				{
					initAsArray();
				}
				break;
			default:
				{
					FATAL(true, ("Invalid list type specified for table item objvar."));
				}
				break;
		}
	}

	TableItemBase::initializeEditor();
}

//-----------------------------------------------------------------------------
QString TableItemObjvar::getTypeString() const
{
	return "Objvar";
}

//-----------------------------------------------------------------------------
void TableItemObjvar::initAsSingle(DynamicVariableParamData const *dynamicVariableParamData)
{
	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new TableItemEditorObjVar(table()->viewport(), "TableItemEditorObjVar", *this, getTemplateTableRow());
	NOT_NULL(m_dataTypeEdit);

	// Set the current data

	if (dynamicVariableParamData != NULL)
	{
		getTableItemEditorObjVar()->setValue(*dynamicVariableParamData);
	}

	setText(m_dataTypeEdit->getText());
}

//-----------------------------------------------------------------------------
TableItemEditorObjVar *TableItemObjvar::getTableItemEditorObjVar() const
{
	TableItemEditorObjVar *tableItemEditorObjVar = dynamic_cast<TableItemEditorObjVar *>(m_dataTypeEdit);
	NOT_NULL(tableItemEditorObjVar);

	return tableItemEditorObjVar;
}

//-----------------------------------------------------------------------------
void TableItemObjvar::setContentFromEditor(QWidget *widget)
{
	TableItemEditorObjVar *tableItemEditorObjVar = dynamic_cast<TableItemEditorObjVar *>(widget);

	if (tableItemEditorObjVar != NULL)
	{
		QString text(tableItemEditorObjVar->getText());
		setText(text);
	}
	else
	{
		TableItemBase::setContentFromEditor(widget);
	}
}

//-----------------------------------------------------------------------------
int TableItemObjvar::getSupportedParameterProperties() const
{
	return TableItemBase::getSupportedParameterProperties();
}

// ============================================================================
