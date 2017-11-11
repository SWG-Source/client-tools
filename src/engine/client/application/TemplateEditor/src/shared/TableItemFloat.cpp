// ============================================================================
//
// TableItemFloat.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemFloat.h"

#include "ArrayDialogEdit.h"
#include "FloatEdit.h"
#include "FloatRangeDialogEdit.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "WeightedListDialogEdit.h"

// ============================================================================
//
// TableItemFloat
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemFloat::TableItemFloat(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
void TableItemFloat::initDefault()
{
	initAsSingle(NULL);
}

//-----------------------------------------------------------------------------
void TableItemFloat::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
	// See if we need to for this to a simple type

	TemplateData::ListType listType = getTemplateTableRow().getParameter()->list_type;

	if (getTemplateTableRow().isSingle())
	{
		listType = TemplateData::LIST_NONE;
	}

	// Initialize depending on the list type

	if ((getTemplateTableRow().isParameterDerived() && (parameterProperty == TemplateTableRow::PP_undefined)) || parameterProperty == TemplateTableRow::PP_derived)
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
					FloatParam *floatParam = NULL;

					if (loaded)
					{
						// Since the parameter is loaded, get the actual parameter type

						floatParam = tpfTemplate->getFloatParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(floatParam);
					}
					else if (tpfTemplate->getListLength(parameterName.latin1()) > 0)
					{
						DEBUG_FATAL((parameterIndex >= tpfTemplate->getListLength(parameterName.latin1())), ("Invalid list parameter index %d of %d", parameterIndex, tpfTemplate->getListLength(parameterName.latin1())));

						// This is a list element

						floatParam = tpfTemplate->getFloatParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(floatParam);
					}
					else if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
					{
						// This is a weighted list element

						floatParam = tpfTemplate->getFloatParam(parameterName.latin1(), true, 0);
						NOT_NULL(floatParam);
					}

					init(floatParam, parameterIndex, weightedListIndex, parameterProperty);
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
void TableItemFloat::init(FloatParam const *floatParam, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
	bool differentProperty = false;

	if (parameterProperty == TemplateTableRow::PP_undefined)
	{
		differentProperty = false;
	}
	else if (floatParam == NULL)
	{
		differentProperty = true;
	}
	else
	{
		differentProperty = (floatParam->getType() == FloatParam::DIE_ROLL) ||
		                    (floatParam->getType() == FloatParam::NONE) ||
		                    (floatParam->getType() == FloatParam::SINGLE) && !(parameterProperty == TemplateTableRow::PP_simple) ||
		                    (floatParam->getType() == FloatParam::WEIGHTED_LIST) && !(parameterProperty == TemplateTableRow::PP_weightedList) ||
		                    (floatParam->getType() == FloatParam::RANGE) && !(parameterProperty == TemplateTableRow::PP_randomRange);
	}

	if (differentProperty)
	{
		switch (parameterProperty)
		{
			case TemplateTableRow::PP_simple:
			case TemplateTableRow::PP_undefined:
				{
					initAsSingle(NULL);
				}
				break;
			case TemplateTableRow::PP_weightedList:
				{
					initAsWeightedList(NULL, 0);
				}
				break;
			case TemplateTableRow::PP_randomRange:
				{
					initAsRandomRange(NULL);
				}
				break;
			default:
				{
					FATAL(true, ("Invalid float param type."));
				}
		}
	}
	else
	{
		NOT_NULL(floatParam);

		// Initialize to the correct float type

		FloatParam::DataTypeId dataTypeId = floatParam->getType();

		switch (dataTypeId)
		{
			case FloatParam::SINGLE:
				{
					initAsSingle(floatParam);
				}
				break;
			case FloatParam::WEIGHTED_LIST:
				{
					if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
					{
						FloatParam::WeightedList const *weightedList = floatParam->getRawWeightedList();
						FloatParam::WeightedValue const &weightedValue = (*weightedList)[weightedListIndex];
						FloatParam *newFloatParam = dynamic_cast<FloatParam *>(weightedValue.value);
						NOT_NULL(newFloatParam);

						init(newFloatParam, parameterIndex, weightedListIndex, parameterProperty);

						// Set the weight

						getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_weightedItem);

						m_dataTypeEdit->setWeightedItemStatus(true);
						m_dataTypeEdit->setWeight(weightedValue.weight);
					}
					else
					{
						initAsWeightedList(floatParam, parameterIndex);
					}
				}
				break;
			case FloatParam::RANGE:
				{
					initAsRandomRange(floatParam);
				}
				break;
			default:
				{
					FATAL(true, ("Invalid integer param type."));
				}
		}
	}

	// Deltas are valid for every type but weighted list items

	if ((floatParam != NULL) && !getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
	{
		// Setup the deltas
		
		if (floatParam->getDeltaType() == '+')
		{
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPositive(true);
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPercent(false);
		}
		else if (floatParam->getDeltaType() == '-')
		{
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaNegative(true);
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPercent(false);
		}
		else if (floatParam->getDeltaType() == '=')
		{
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPositive(true);
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPercent(true);
		}
		else if (floatParam->getDeltaType() == '_')
		{
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaNegative(true);
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPercent(true);
		}
	}

	setText(m_dataTypeEdit->getText());
}

//-----------------------------------------------------------------------------
void TableItemFloat::initAsSingle(FloatParam const *floatParam)
{
	float limitMin = -std::numeric_limits<float>::max();
	float limitMax = std::numeric_limits<float>::max();
	float value = 0.0f;

	if (floatParam != NULL)
	{
		// Get the value

		value = floatParam->getValue();

		// Get the min, max

		TemplateData::Parameter const *parameter = getTemplateTableRow().getParameter();

		if (parameter != NULL)
		{
			limitMin = parameter->min_float_limit;
			limitMax = parameter->max_float_limit;
		}
	}

	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new FloatEdit(table()->viewport(), "FloatEdit", *this, limitMin, limitMax);
	NOT_NULL(m_dataTypeEdit);

	// Set the parent template table row to simple

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_simple);

	// Set the initial value

	getFloatEdit()->setValue(value);
}

//-----------------------------------------------------------------------------
void TableItemFloat::initAsWeightedList(FloatParam const *floatParam, int const parameterIndex)
{
	delete m_dataTypeEdit;
	m_dataTypeEdit = new WeightedListDialogEdit(table()->viewport(), "WeightedListDialogEdit", *this, getTemplateTableRow());
	NOT_NULL(m_dataTypeEdit);

	// Mark this as the main weighted list item

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_weightedList);

	// Get the row count

	int const rowCount = (floatParam == NULL) ? 1 : floatParam->getRawWeightedList()->size();

	// Create the child items

	WeightedListDialogEdit *weightedListDialogEdit = dynamic_cast<WeightedListDialogEdit *>(m_dataTypeEdit);
	NOT_NULL(weightedListDialogEdit);

	weightedListDialogEdit->createRows(rowCount, parameterIndex);
}

//-----------------------------------------------------------------------------
void TableItemFloat::initAsRandomRange(FloatParam const *floatParam)
{
	float limitMin = -std::numeric_limits<float>::max();
	float limitMax = std::numeric_limits<float>::max();
	float valueMin = 0.0f;
	float valueMax = 0.0f;

	if (floatParam != NULL)
	{
		// Get the value min/max

		valueMin = floatParam->getMinValue();
		valueMax = floatParam->getMaxValue();

		// Get the limit min/max

		TemplateData::Parameter const *parameter = getTemplateTableRow().getParameter();

		if (parameter != NULL)
		{
			limitMin = parameter->min_float_limit;
			limitMax = parameter->max_float_limit;
		}
	}

	// Set the parent template table row to random range

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_randomRange);

	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new FloatRangeDialogEdit(table()->viewport(), "FloatRangeDialogEdit", *this, limitMin, limitMax);
	NOT_NULL(m_dataTypeEdit);

	// Set the values

	getFloatRangeDialogEdit()->setRangeData(valueMin, valueMax);
}

//-----------------------------------------------------------------------------
QString TableItemFloat::getTypeString() const
{
	return "Float";
}

//-----------------------------------------------------------------------------
int TableItemFloat::getSupportedParameterProperties() const
{
	int supportedFlags = TableItemBase::getSupportedParameterProperties();
	
	supportedFlags |= TemplateTableRow::PP_randomRange;
	supportedFlags |= TemplateTableRow::PP_delta;
	supportedFlags |= TemplateTableRow::PP_weightedItem;
	supportedFlags |= TemplateTableRow::PP_weightedList;

	return supportedFlags;
}

//-----------------------------------------------------------------------------
FloatEdit *TableItemFloat::getFloatEdit() const
{
	FloatEdit *floatEdit = dynamic_cast<FloatEdit *>(m_dataTypeEdit);

	return floatEdit;
}

//-----------------------------------------------------------------------------
FloatRangeDialogEdit *TableItemFloat::getFloatRangeDialogEdit() const
{
	FloatRangeDialogEdit *floatRangeDialogEdit = dynamic_cast<FloatRangeDialogEdit *>(m_dataTypeEdit);

	return floatRangeDialogEdit;
}

//-----------------------------------------------------------------------------
void TableItemFloat::setContentFromEditor(QWidget *widget)
{
	if (getFloatEdit() != NULL)
	{
		QString text(getFloatEdit()->getText());
		setText(text);
	}
	else
	{
		if (getFloatRangeDialogEdit() != NULL)
		{
			QString text(getFloatRangeDialogEdit()->getText());
			setText(text);
		}
		else
		{
			if (getWeightedListDialogEdit() != NULL)
			{
				QString text(getWeightedListDialogEdit()->getText());
				setText(text);
			}
			else
			{
				TableItemBase::setContentFromEditor(widget);
			}
		}
	}
}

// ============================================================================
