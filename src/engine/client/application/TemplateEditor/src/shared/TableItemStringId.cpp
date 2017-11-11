// ============================================================================
//
// TableItemStringId.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemStringId.h"

#include "ArrayDialogEdit.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "StringIDDialogEdit.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "WeightedListDialogEdit.h"

// ============================================================================
//
// TableItemStringId
//
// ============================================================================

TableItemStringId::TableItemStringId(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
void TableItemStringId::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
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
					bool const loaded = tpfTemplate->isParamLoaded(parameterName.latin1(), false, parameterIndex);

					if (loaded)
					{
						// Since the parameter is loaded, get the actual parameter type

						StringIdParam *stringIdParam = tpfTemplate->getStringIdParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(stringIdParam);

						init(stringIdParam, parameterIndex, weightedListIndex);
					}
					else if (tpfTemplate->getListLength(parameterName.latin1()) > 0)
					{
						DEBUG_FATAL((parameterIndex >= tpfTemplate->getListLength(parameterName.latin1())), ("Invalid list parameter index %d of %d", parameterIndex, tpfTemplate->getListLength(parameterName.latin1())));

						// This is a list element

						StringIdParam *stringIdParam = tpfTemplate->getStringIdParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(stringIdParam);

						init(stringIdParam, parameterIndex, weightedListIndex);
					}
					else if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
					{
						// This is a weighted list element

						StringIdParam *stringIdParam = tpfTemplate->getStringIdParam(parameterName.latin1(), true, 0);
						NOT_NULL(stringIdParam);

						init(stringIdParam, parameterIndex, weightedListIndex);
					}
					else
					{
						initAsSingle(NULL);
					}
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
					FATAL(true, ("Invalid list type specified."));
				}
				break;
		}
	}

	TableItemBase::initializeEditor();
}

//-----------------------------------------------------------------------------
void TableItemStringId::init(StringIdParam const *stringIdParam, int const parameterIndex, int const weightedListIndex)
{
	// Initialize to the correct integer type

	StringIdParam::DataTypeId dataTypeId = stringIdParam->getType();

	switch (dataTypeId)
	{
		case StringIdParam::SINGLE:
			{
				initAsSingle(stringIdParam);
			}
			break;
		case StringIdParam::WEIGHTED_LIST:
			{
				if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
				{
					StringIdParam::WeightedList const *weightedList = stringIdParam->getRawWeightedList();
					StringIdParam::WeightedValue const &weightedValue = (*weightedList)[weightedListIndex];
					StringIdParam *newStringIdParam = dynamic_cast<StringIdParam *>(weightedValue.value);
					NOT_NULL(newStringIdParam);

					init(newStringIdParam, parameterIndex, weightedListIndex);

					// Set the weight

					m_dataTypeEdit->setWeightedItemStatus(true);
					m_dataTypeEdit->setWeight(weightedValue.weight);
				}
				else
				{
					initAsWeightedList(stringIdParam, parameterIndex);
				}
			}
			break;
		default:
			{
				FATAL(true, ("Invalid string id param type."));
			}
	}

	setText(m_dataTypeEdit->getText());
}

//-----------------------------------------------------------------------------
void TableItemStringId::initAsSingle(StringIdParam const *stringIdParam)
{
	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new StringIDDialogEdit(table()->viewport(), "StringIDDialogEdit", *this);
	NOT_NULL(m_dataTypeEdit);

	// Set the editor value

	StringId stringId(stringIdParam->getValue());

	getStringIDDialogEdit()->setValue(stringId);
}

//-----------------------------------------------------------------------------
void TableItemStringId::initAsWeightedList(StringIdParam const *stringIdParam, int const parameterIndex)
{
	NOT_NULL(stringIdParam);

	delete m_dataTypeEdit;
	m_dataTypeEdit = new WeightedListDialogEdit(table()->viewport(), "WeightedListDialogEdit", *this, getTemplateTableRow());

	// Mark this as the main weighted list item

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_weightedList);

	// Create the child items

	int const rowCount = stringIdParam->getRawWeightedList()->size();
	WeightedListDialogEdit *weightedListDialogEdit = dynamic_cast<WeightedListDialogEdit *>(m_dataTypeEdit);
	NOT_NULL(weightedListDialogEdit);

	weightedListDialogEdit->createRows(rowCount, parameterIndex);
}

//-----------------------------------------------------------------------------
QString TableItemStringId::getTypeString() const
{
	return "String Id";
}

//-----------------------------------------------------------------------------
StringIDDialogEdit *TableItemStringId::getStringIDDialogEdit() const
{
	StringIDDialogEdit *stringIdDialogEdit = dynamic_cast<StringIDDialogEdit *>(m_dataTypeEdit);

	return stringIdDialogEdit;
}

//-----------------------------------------------------------------------------
void TableItemStringId::setContentFromEditor(QWidget *widget)
{
	if (getStringIDDialogEdit() != NULL)
	{
		QString text(getStringIDDialogEdit()->getText());
		setText(text);
	}
	else
	{
		TableItemBase::setContentFromEditor(widget);
	}
}

//-----------------------------------------------------------------------------
int TableItemStringId::getSupportedParameterProperties() const
{
	return TableItemBase::getSupportedParameterProperties();
}

// ============================================================================
