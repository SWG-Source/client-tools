// ============================================================================
//
// TableItemEnum.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemEnum.h"

#include "ArrayDialogEdit.h"
#include "EnumEdit.h"
#include "ListDialogEdit.h"
#include "sharedTemplateDefinition/File.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateDataIterator.h"
#include "sharedTemplateDefinition/TemplateDefinitionFile.h"
#include "sharedTemplateDefinition/TpfFile.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "TemplateLoader.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "WeightedListDialogEdit.h"

// ============================================================================
//
// TableItemEnum
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemEnum::TableItemEnum(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
void TableItemEnum::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
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
					// Make sure the parameter exists in this tpf template

					bool const loaded = tpfTemplate->isParamLoaded(parameterName.latin1(), false, parameterIndex);

					if (loaded)
					{
						IntegerParam *integerParam = tpfTemplate->getCompilerIntegerParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(integerParam);

						init(integerParam, parameterIndex, weightedListIndex, parameterProperty);
					}
					else if (tpfTemplate->getListLength(parameterName.latin1()) > 0)
					{
						DEBUG_FATAL((parameterIndex >= tpfTemplate->getListLength(parameterName.latin1())), ("Invalid list parameter index %d of %d", parameterIndex, tpfTemplate->getListLength(parameterName.latin1())));

						// This is a list element

						IntegerParam *integerParam = tpfTemplate->getCompilerIntegerParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(integerParam);

						init(integerParam, parameterIndex, weightedListIndex, parameterProperty);
					}
					else if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
					{
						// This is a weighted list element

						IntegerParam *integerParam = tpfTemplate->getCompilerIntegerParam(parameterName.latin1(), true, 0);
						NOT_NULL(integerParam);

						init(integerParam, parameterIndex, weightedListIndex, parameterProperty);
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
void TableItemEnum::init(IntegerParam const *integerParam, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
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
		case IntegerParam::DIE_ROLL:
		default:
			{
				FATAL(true, ("Invalid enum param type."));
			}
	}

	setText(m_dataTypeEdit->getText());
}

//-----------------------------------------------------------------------------
void TableItemEnum::initAsSingle(IntegerParam const *integerParam)
{
	// Get the enum list

	TemplateData::Parameter const *parameter = getTemplateTableRow().getParameter();
	NOT_NULL(parameter);

	TemplateData::EnumList const *enumList = TemplateLoader::getTemplateData().getEnumList(parameter->extendedName, false);
	NOT_NULL(enumList);

	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new EnumEdit(table()->viewport(), "EnumEdit", *this, *enumList);
	NOT_NULL(m_dataTypeEdit);

	// Set the editor value

	int value = 0;

	if (integerParam != NULL)
	{
		value = integerParam->getValue();
	}

	getEnumEdit()->setValue(value);
}

//-----------------------------------------------------------------------------
void TableItemEnum::initAsWeightedList(IntegerParam const *integerParam, int const parameterIndex)
{
	NOT_NULL(integerParam);

	delete m_dataTypeEdit;
	m_dataTypeEdit = new WeightedListDialogEdit(table()->viewport(), "WeightedListDialogEdit", *this, getTemplateTableRow());
	NOT_NULL(m_dataTypeEdit);

	// Mark this as the main weighted list item

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_weightedList);

	// Create the child items

	int const rowCount = integerParam->getRawWeightedList()->size();
	WeightedListDialogEdit *weightedListDialogEdit = dynamic_cast<WeightedListDialogEdit *>(m_dataTypeEdit);
	NOT_NULL(weightedListDialogEdit);

	weightedListDialogEdit->createRows(rowCount, parameterIndex);
}

//-----------------------------------------------------------------------------
QString TableItemEnum::getTypeString() const
{
	return "Enumeration";
}

//-----------------------------------------------------------------------------
EnumEdit *TableItemEnum::getEnumEdit() const
{
	EnumEdit *enumEdit = dynamic_cast<EnumEdit *>(m_dataTypeEdit);
	NOT_NULL(enumEdit);

	return enumEdit;
}

//-----------------------------------------------------------------------------
void TableItemEnum::setContentFromEditor(QWidget *widget)
{
	EnumEdit *enumEdit = dynamic_cast<EnumEdit *>(widget);

	if (enumEdit != NULL)
	{
		QString text(enumEdit->getText());
		setText(text);
	}
	else
	{
		TableItemBase::setContentFromEditor(widget);
	}
}

//-----------------------------------------------------------------------------
int TableItemEnum::getSupportedParameterProperties() const
{
	return TableItemBase::getSupportedParameterProperties();
}

// ============================================================================
