// ============================================================================
//
// TableItemInt.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemInt.h"

#include "ArrayDialogEdit.h"
#include "DiceRollDialogEdit.h"
#include "IntegerEdit.h"
#include "IntegerRangeDialogEdit.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "TemplateLoader.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "WeightedListDialogEdit.h"

// ============================================================================
//
// TableItemInt
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemInt::TableItemInt(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
void TableItemInt::initDefault()
{
	initAsSingle(NULL);
}

//-----------------------------------------------------------------------------
void TableItemInt::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
	// See if we need to change this to a simple type

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
					IntegerParam *integerParam = NULL;

					if (loaded)
					{
						// Since the parameter is loaded, get the actual template parameter

						integerParam = tpfTemplate->getCompilerIntegerParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(integerParam);
					}
					else if (tpfTemplate->getListLength(parameterName.latin1()) > 0)
					{
						DEBUG_FATAL((parameterIndex >= tpfTemplate->getListLength(parameterName.latin1())), ("Invalid list parameter index %d of %d", parameterIndex, tpfTemplate->getListLength(parameterName.latin1())));

						// This is a list element

						integerParam = tpfTemplate->getCompilerIntegerParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(integerParam);
					}
					else if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
					{
						// This is a weighted list element

						integerParam = tpfTemplate->getCompilerIntegerParam(parameterName.latin1(), true, 0);
						NOT_NULL(integerParam);
					}

					init(integerParam, parameterIndex, weightedListIndex, parameterProperty);
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
void TableItemInt::init(IntegerParam const *integerParam, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
	bool differentProperty = false;

	if (parameterProperty == TemplateTableRow::PP_undefined)
	{
		differentProperty = false;
	}
	else if (integerParam == NULL)
	{
		differentProperty = true;
	}
	else
	{
		differentProperty = (integerParam->getType() == IntegerParam::NONE) ||
		                    (integerParam->getType() == IntegerParam::SINGLE) && !(parameterProperty == TemplateTableRow::PP_simple) ||
		                    (integerParam->getType() == IntegerParam::WEIGHTED_LIST) && !(parameterProperty == TemplateTableRow::PP_weightedList) ||
		                    (integerParam->getType() == IntegerParam::RANGE) && !(parameterProperty == TemplateTableRow::PP_randomRange) ||
		                    (integerParam->getType() == IntegerParam::DIE_ROLL) && !(parameterProperty == TemplateTableRow::PP_dieRoll);
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
			case TemplateTableRow::PP_dieRoll:
				{
					initAsDieRoll(NULL);
				}
				break;
			default:
				{
					FATAL(true, ("Invalid integer param type."));
				}
		}
	}
	else
	{
		NOT_NULL(integerParam);

		// Initialize to the correct integer type

		IntegerParam::DataTypeId dataTypeId = integerParam->getType();

		switch (dataTypeId)
		{
			case IntegerParam::SINGLE:
				{
					initAsSingle(integerParam);
				}
				break;
			case IntegerParam::WEIGHTED_LIST:
				{
					if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
					{
						IntegerParam::WeightedList const *weightedList = integerParam->getRawWeightedList();
						IntegerParam::WeightedValue const &weightedValue = (*weightedList)[weightedListIndex];
						IntegerParam *newIntegerParam = dynamic_cast<IntegerParam *>(weightedValue.value);
						NOT_NULL(newIntegerParam);

						init(newIntegerParam, parameterIndex, weightedListIndex, parameterProperty);

						// Set the weight

						getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_weightedItem);

						m_dataTypeEdit->setWeightedItemStatus(true);
						m_dataTypeEdit->setWeight(weightedValue.weight);
					}
					else
					{
						initAsWeightedList(integerParam, parameterIndex);
					}
				}
				break;
			case IntegerParam::RANGE:
				{
					initAsRandomRange(integerParam);
				}
				break;
			case IntegerParam::DIE_ROLL:
				{
					initAsDieRoll(integerParam);
				}
				break;
			default:
				{
					FATAL(true, ("Invalid integer param type."));
				}
		}
	}

	// Deltas are valid for every type but weighted list items

	if ((integerParam != NULL) && !getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
	{
		// Setup the deltas
		
		if (integerParam->getDeltaType() == '+')
		{
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPositive(true);
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPercent(false);
		}
		else if (integerParam->getDeltaType() == '-')
		{
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaNegative(true);
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPercent(false);
		}
		else if (integerParam->getDeltaType() == '=')
		{
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPositive(true);
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPercent(true);
		}
		else if (integerParam->getDeltaType() == '_')
		{
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaNegative(true);
			m_dataTypeEdit->getTableItemBase().getTemplateTableRow().setDeltaPercent(true);
		}
	}

	setText(m_dataTypeEdit->getText());
}

//-----------------------------------------------------------------------------
void TableItemInt::initAsSingle(IntegerParam const *integerParam)
{
	// Get the value and limits

	int limitMin = std::numeric_limits<int>::min();
	int limitMax = std::numeric_limits<int>::max();
	int value = 0;

	if (integerParam != NULL)
	{
		// Get the value

		value = integerParam->getValue();

		TemplateData::Parameter const *parameter = getTemplateTableRow().getParameter();

		// Get the min/max

		if (parameter != NULL)
		{
			limitMin = parameter->min_int_limit;
			limitMax = parameter->max_int_limit;
		}
	}

	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new IntegerEdit(table()->viewport(), "IntegerEdit", *this, limitMin, limitMax);
	NOT_NULL(m_dataTypeEdit);

	// Set the parent template table row to simple

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_simple);

	// Set the editor value

	getIntegerEdit()->setValue(value);
}

//-----------------------------------------------------------------------------
void TableItemInt::initAsWeightedList(IntegerParam const *integerParam, int const parameterIndex)
{
	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new WeightedListDialogEdit(table()->viewport(), "WeightedListDialogEdit", *this, getTemplateTableRow());
	NOT_NULL(m_dataTypeEdit);

	// Mark this as the main weighted list item

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_weightedList);

	// Get the row count

	int const rowCount = (integerParam == NULL) ? 1 : integerParam->getRawWeightedList()->size();
	
	// Create the child items

	WeightedListDialogEdit *weightedListDialogEdit = dynamic_cast<WeightedListDialogEdit *>(m_dataTypeEdit);
	NOT_NULL(weightedListDialogEdit);

	weightedListDialogEdit->createRows(rowCount, parameterIndex);
}

//-----------------------------------------------------------------------------
void TableItemInt::initAsRandomRange(IntegerParam const *integerParam)
{
	// Get the value and limits

	int valueMin = 0;
	int valueMax = 100;
	int limitMin = std::numeric_limits<int>::min() + 1;
	int limitMax = std::numeric_limits<int>::max();

	if (integerParam != NULL)
	{
		valueMin = integerParam->getMinValue();
		valueMax = integerParam->getMaxValue();

		TemplateData::Parameter const *parameter = getTemplateTableRow().getParameter();

		if (parameter != NULL)
		{
			limitMin = parameter->min_int_limit;
			limitMax = parameter->max_int_limit;
		}
	}

	// Set the parent template table row to random range

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_randomRange);

	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new IntegerRangeDialogEdit(table()->viewport(), "IntegerRangeDialogEdit", *this, limitMin, limitMax);
	NOT_NULL(m_dataTypeEdit);

	// Set the values

	getIntegerRangeDialogEdit()->setRangeData(valueMin, valueMax);
}

//-----------------------------------------------------------------------------
void TableItemInt::initAsDieRoll(IntegerParam const *integerParam)
{
	// Create the editor

	TemplateData::Parameter const *parameter = getTemplateTableRow().getParameter();

	delete m_dataTypeEdit;
	m_dataTypeEdit = new DiceRollDialogEdit(table()->viewport(), "DiceRollDialogEdit", *this, parameter->min_int_limit, parameter->max_int_limit);

	// Set the parent template table row to dice roll

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_dieRoll);

	// Set the dice values

	int dieCount = 4;
	int dieSides = 6;
	int dieBase = 10;

	if (integerParam != NULL)
	{
		IntegerParam::DieRoll const *dieRoll = integerParam->getDieRollStruct();

		dieCount = dieRoll->num_dice;
		dieSides = dieRoll->die_sides;
		dieBase = dieRoll->base;
	}

	getDiceRollDialogEdit()->setDiceData(dieCount, dieSides, dieBase);
}

//-----------------------------------------------------------------------------
QString TableItemInt::getTypeString() const
{
	return "Integer";
}

//-----------------------------------------------------------------------------
int TableItemInt::getSupportedParameterProperties() const
{
	int supportedFlags = TableItemBase::getSupportedParameterProperties();
	
	supportedFlags |= TemplateTableRow::PP_randomRange;
	supportedFlags |= TemplateTableRow::PP_dieRoll;
	supportedFlags |= TemplateTableRow::PP_delta;
	supportedFlags |= TemplateTableRow::PP_weightedItem;
	supportedFlags |= TemplateTableRow::PP_weightedList;

	return supportedFlags;
}

//-----------------------------------------------------------------------------
DiceRollDialogEdit *TableItemInt::getDiceRollDialogEdit() const
{
	DiceRollDialogEdit *diceRollDialogEdit = dynamic_cast<DiceRollDialogEdit *>(m_dataTypeEdit);

	return diceRollDialogEdit;
}

//-----------------------------------------------------------------------------
IntegerEdit *TableItemInt::getIntegerEdit() const
{
	IntegerEdit *integerEdit = dynamic_cast<IntegerEdit *>(m_dataTypeEdit);

	return integerEdit;
}

//-----------------------------------------------------------------------------
IntegerRangeDialogEdit *TableItemInt::getIntegerRangeDialogEdit() const
{
	IntegerRangeDialogEdit *integerRangeDialogEdit = dynamic_cast<IntegerRangeDialogEdit *>(m_dataTypeEdit);

	return integerRangeDialogEdit;
}

//-----------------------------------------------------------------------------
void TableItemInt::setContentFromEditor(QWidget *widget)
{
	if (getIntegerRangeDialogEdit() != NULL)
	{
		QString text(getIntegerRangeDialogEdit()->getText());
		setText(text);
	}
	else
	{
		if (getIntegerEdit() != NULL)
		{
			QString text(getIntegerEdit()->getText());
			setText(text);
		}
		else
		{
			if (getDiceRollDialogEdit() != NULL)
			{
				QString text(getDiceRollDialogEdit()->getText());
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
}

//=============================================================================
