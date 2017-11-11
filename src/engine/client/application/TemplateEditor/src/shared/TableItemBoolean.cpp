// ============================================================================
//
// TableItemBoolean.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemBoolean.h"

#include "BoolEdit.h"
#include "ListDialogEdit.h"
#include "sharedUtility/TemplateParameter.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "WeightedListDialogEdit.h"

// ============================================================================
//
// TableItemBoolean
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemBoolean::TableItemBoolean(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
void TableItemBoolean::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
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
						BoolParam const *boolParam = tpfTemplate->getBoolParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(boolParam);

						init(boolParam, parameterIndex, weightedListIndex);
					}
					else if (tpfTemplate->getListLength(parameterName.latin1()) > 0)
					{
						DEBUG_FATAL((parameterIndex >= tpfTemplate->getListLength(parameterName.latin1())), ("Invalid list parameter index %d of %d", parameterIndex, tpfTemplate->getListLength(parameterName.latin1())));

						// This is a list element

						BoolParam const *boolParam = tpfTemplate->getBoolParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(boolParam);

						init(boolParam, parameterIndex, weightedListIndex);
					}
					else if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
					{
						// This is a weighted list element

						BoolParam const *boolParam = tpfTemplate->getBoolParam(parameterName.latin1(), true, 0);
						NOT_NULL(boolParam);

						init(boolParam, parameterIndex, weightedListIndex);
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

	// Handle the special cases

	TableItemBase::initializeEditor();
}

//-----------------------------------------------------------------------------
void TableItemBoolean::init(BoolParam const *boolParam, int const parameterIndex, int const weightedListIndex)
{
	// Initialize to the correct integer type

	BoolParam::DataTypeId dataTypeId = boolParam->getType();

	switch (dataTypeId)
	{
		case BoolParam::SINGLE:
			{
				initAsSingle(boolParam);
			}
			break;
		case BoolParam::WEIGHTED_LIST:
			{
				if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
				{
					BoolParam::WeightedList const *weightedList = boolParam->getRawWeightedList();
					BoolParam::WeightedValue const &weightedValue = (*weightedList)[weightedListIndex];
					BoolParam *newBoolParam = dynamic_cast<BoolParam *>(weightedValue.value);
					NOT_NULL(newBoolParam);

					init(newBoolParam, parameterIndex, weightedListIndex);

					// Set the weight

					m_dataTypeEdit->setWeightedItemStatus(true);
					m_dataTypeEdit->setWeight(weightedValue.weight);
				}
				else
				{
					initAsWeightedList(boolParam, parameterIndex);
				}
			}
			break;
		default:
			{
				FATAL(true, ("Invalid integer param type."));
			}
	}

	setText(m_dataTypeEdit->getText());
}

//-----------------------------------------------------------------------------
void TableItemBoolean::initAsSingle(BoolParam const *boolParam)
{
	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new BoolEdit(table()->viewport(), "BoolEdit", *this);
	NOT_NULL(m_dataTypeEdit);

	bool value = false;

	if (boolParam != NULL)
	{
		// Set the editor value

		value = boolParam->getValue();
	}

	getBoolEdit()->setValue(value);
}

//-----------------------------------------------------------------------------
void TableItemBoolean::initAsWeightedList(BoolParam const *boolParam, int const parameterIndex)
{
	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new WeightedListDialogEdit(table()->viewport(), "WeightedListDialogEdit", *this, getTemplateTableRow());

	// Mark this as the main weighted list item

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_weightedList);

	// Create the child items

	int const rowCount = boolParam->getRawWeightedList()->size();
	WeightedListDialogEdit *weightedListDialogEdit = dynamic_cast<WeightedListDialogEdit *>(m_dataTypeEdit);
	NOT_NULL(weightedListDialogEdit);

	weightedListDialogEdit->createRows(rowCount, parameterIndex);
}

//-----------------------------------------------------------------------------
QString TableItemBoolean::getTypeString() const
{
	return "Boolean";
}

//-----------------------------------------------------------------------------
BoolEdit *TableItemBoolean::getBoolEdit() const
{
	BoolEdit *boolEdit = dynamic_cast<BoolEdit *>(m_dataTypeEdit);
	NOT_NULL(boolEdit);

	return boolEdit;
}

//-----------------------------------------------------------------------------
void TableItemBoolean::setContentFromEditor(QWidget *widget)
{
	BoolEdit *boolEdit = dynamic_cast<BoolEdit *>(widget);

	if (boolEdit != NULL)
	{
		QString text(boolEdit->getText().lower());
		setText(text);
	}
	else
	{
		TableItemBase::setContentFromEditor(widget);
	}
}

//-----------------------------------------------------------------------------
int TableItemBoolean::getSupportedParameterProperties() const
{
	return TableItemBase::getSupportedParameterProperties();
}

// ============================================================================
