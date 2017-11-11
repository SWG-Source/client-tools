// ============================================================================
//
// DialogObjvarList.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "DialogObjvarList.h"

#include "sharedTemplateDefinition/TemplateData.h"
#include "TableItemBase.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// DialogObjvarList
//
// ============================================================================

//-----------------------------------------------------------------------------
DialogObjvarList::DialogObjvarList(QWidget *parent, char const *name, TemplateTableRow &parentTemplateTableRow)
 : DynamicListDialog(parent, name, parentTemplateTableRow)
{
	//setFocusTarget(*m_objvarTypeComboBox);
}

//-----------------------------------------------------------------------------
void DialogObjvarList::init(DynamicVariableParamData const *dynamicVariableParamData)
{
	NOT_NULL(dynamicVariableParamData);

	// What type of dynamic variable is this?

	switch (dynamicVariableParamData->m_type)
	{
		case DynamicVariableParamData::INTEGER:
		case DynamicVariableParamData::FLOAT:
		case DynamicVariableParamData::STRING:
			{
				createObjVarTemplateTableRows(0, *dynamicVariableParamData);
			}
			break;
		case DynamicVariableParamData::LIST:
			{
				std::vector<DynamicVariableParamData *>::const_iterator iterDynamicVariableParamData = dynamicVariableParamData->m_data.lparam->begin();

				int row = 0;
				int const count = dynamicVariableParamData->m_data.lparam->size();
				UNREF(count);

				for (; iterDynamicVariableParamData != dynamicVariableParamData->m_data.lparam->end(); ++iterDynamicVariableParamData)
				{
					DynamicVariableParamData *dynamicVariableParamDataCurrent = (*iterDynamicVariableParamData);

					createObjVarTemplateTableRows(row, *dynamicVariableParamDataCurrent);

					++row;
				}
			}
			break;
		case DynamicVariableParamData::UNKNOWN:
			{
				// This is an empty list
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("Unexpected dynamic variable type specified"));
			}
	}
}

//-----------------------------------------------------------------------------
QString DialogObjvarList::getText() const
{
	QString parameterName(getTemplateTableRow(0)->getColumnValue()->text());
	UNREF(parameterName);

	QString result;
	int const rowCount = getRowCount();

	if (rowCount >= 2)
	{
		//if (getTemplateTableRow(1)->getTemplateDataParamType() != TemplateData::TYPE_DYNAMIC_VAR)
		//{
		//	result += "[ ";
		//}

		for (int row = 0; row < rowCount; row += 2)
		{
			TableItemBase *tableItemBaseKey = getTemplateTableRow(row)->getColumnValue();
			TableItemBase *tableItemBaseValue = getTemplateTableRow(row + 1)->getColumnValue();
			
			if (getTemplateTableRow(row + 1)->getTemplateDataParamType() != TemplateData::TYPE_DYNAMIC_VAR)
			{
				result += tableItemBaseKey->text();
				result += " = ";
			}

			result += tableItemBaseValue->text();

			if (row != (rowCount - 2))
			{
				result += ", ";
			}
		}

		//if (getTemplateTableRow(1)->getTemplateDataParamType() != TemplateData::TYPE_DYNAMIC_VAR)
		//{
		//	result += " ]";
		//}
	}

	return result;
}

//-----------------------------------------------------------------------------
void DialogObjvarList::addNewRowAboveCurrent()
{
}

//-----------------------------------------------------------------------------
void DialogObjvarList::addNewRowBelowCurrent()
{
}

//-----------------------------------------------------------------------------
void DialogObjvarList::deleteCurrentRow()
{
}

//-----------------------------------------------------------------------------
void DialogObjvarList::createObjVarTemplateTableRows(int const index, DynamicVariableParamData const &dynamicVariableParamData)
{
	// Set the key

	QString name;
	name.sprintf("Key    [%d]", index + 1);
	TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, index * 2, TemplateData::TYPE_STRING, name);

	StringParam stringParam;
	stringParam.setValue(dynamicVariableParamData.m_name);
	templateTableRow->getColumnValue()->init(&stringParam, 0, 0);

	// Set the value

	QString value;
	value.sprintf("Value [%d]", index + 1);

	// Figure out the value type

	switch (dynamicVariableParamData.m_type)
	{
		case DynamicVariableParamData::INTEGER:
			{
				TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, index * 2 + 1, TemplateData::TYPE_INTEGER, value);
				NOT_NULL(templateTableRow);

				TableItemBase *tableItemBaseValue = templateTableRow->getColumnValue();
				tableItemBaseValue->init(dynamicVariableParamData.m_data.iparam, 0, 0, TemplateTableRow::PP_simple);
			}
			break;
		case DynamicVariableParamData::FLOAT:
			{
				TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, index * 2 + 1, TemplateData::TYPE_FLOAT, value);
				NOT_NULL(templateTableRow);

				TableItemBase *tableItemBaseValue = templateTableRow->getColumnValue();
				tableItemBaseValue->init(dynamicVariableParamData.m_data.fparam, 0, 0, TemplateTableRow::PP_simple);
			}
			break;
		case DynamicVariableParamData::STRING:
			{
				TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, index * 2 + 1, TemplateData::TYPE_STRING, value);
				NOT_NULL(templateTableRow);

				TableItemBase *tableItemBaseValue = templateTableRow->getColumnValue();
				tableItemBaseValue->init(dynamicVariableParamData.m_data.sparam, 0, 0);
			}
			break;
		case DynamicVariableParamData::LIST:
			{
				TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, index * 2 + 1, TemplateData::TYPE_DYNAMIC_VAR, value);
				NOT_NULL(templateTableRow);

				TableItemBase *tableItemBaseValue = templateTableRow->getColumnValue();
				tableItemBaseValue->init(&dynamicVariableParamData);
			}
			break;
		case DynamicVariableParamData::UNKNOWN:
		default:
			{
				DEBUG_FATAL(true, ("Unexpected DynamicVariableParamDataType"));
			}
			break;
	}
}

// ============================================================================
