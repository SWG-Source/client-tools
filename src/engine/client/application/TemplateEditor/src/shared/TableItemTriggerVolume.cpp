// ============================================================================
//
// TableItemTriggerVolume.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemTriggerVolume.h"

#include "ListDialogEdit.h"
#include "sharedUtility/TemplateParameter.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "TriggerVolumeDialogEdit.h"
#include "WeightedListDialogEdit.h"

// ============================================================================
//
// TableItemTriggerVolume
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemTriggerVolume::TableItemTriggerVolume(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
void TableItemTriggerVolume::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
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
						TriggerVolumeParam const *triggerVolumeParam = tpfTemplate->getTriggerVolumeParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(triggerVolumeParam);

						init(triggerVolumeParam, parameterIndex, weightedListIndex);
					}
					else if (tpfTemplate->getListLength(parameterName.latin1()) > 0)
					{
						DEBUG_FATAL((parameterIndex >= tpfTemplate->getListLength(parameterName.latin1())), ("Invalid list parameter index %d of %d", parameterIndex, tpfTemplate->getListLength(parameterName.latin1())));

						// This is a list element

						TriggerVolumeParam const *triggerVolumeParam = tpfTemplate->getTriggerVolumeParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(triggerVolumeParam);

						init(triggerVolumeParam, parameterIndex, weightedListIndex);
					}
					else if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
					{
						// This is a weighted list element

						TriggerVolumeParam const *triggerVolumeParam = tpfTemplate->getTriggerVolumeParam(parameterName.latin1(), true, 0);
						NOT_NULL(triggerVolumeParam);

						init(triggerVolumeParam, parameterIndex, weightedListIndex);
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
void TableItemTriggerVolume::init(TriggerVolumeParam const *triggerVolumeParam, int const parameterIndex, int const weightedListIndex)
{
	// Initialize to the correct integer type

	TriggerVolumeParam::DataTypeId dataTypeId = triggerVolumeParam->getType();

	switch (dataTypeId)
	{
		case TriggerVolumeParam::SINGLE:
			{
				initAsSingle(triggerVolumeParam);
			}
			break;
		case TriggerVolumeParam::WEIGHTED_LIST:
			{
				if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
				{
					TriggerVolumeParam::WeightedList const *weightedList = triggerVolumeParam->getRawWeightedList();
					TriggerVolumeParam::WeightedValue const &weightedValue = (*weightedList)[weightedListIndex];
					TriggerVolumeParam *newTriggerVolumeParam = dynamic_cast<TriggerVolumeParam *>(weightedValue.value);
					NOT_NULL(newTriggerVolumeParam);

					init(newTriggerVolumeParam, parameterIndex, weightedListIndex);

					// Set the weight

					m_dataTypeEdit->setWeightedItemStatus(true);
					m_dataTypeEdit->setWeight(weightedValue.weight);
				}
				else
				{
					initAsWeightedList(triggerVolumeParam, parameterIndex);
				}
			}
			break;
		default:
			{
				FATAL(true, ("Invalid integer param type."));
			}
	}
}

//-----------------------------------------------------------------------------
void TableItemTriggerVolume::initAsSingle(TriggerVolumeParam const *triggerVolumeParam)
{
	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new TriggerVolumeDialogEdit(table()->viewport(), "TriggerVolumeDialogEdit", *this);
	NOT_NULL(m_dataTypeEdit);

	// Set the current value

	TriggerVolumeParamData triggerVolumeParamData(triggerVolumeParam->TemplateBase<TriggerVolumeParamData, TriggerVolumeParamData>::getValue());
	
	getTriggerVolumeDialogEdit()->setValue(triggerVolumeParamData);
}

//-----------------------------------------------------------------------------
void TableItemTriggerVolume::initAsWeightedList(TriggerVolumeParam const *triggerVolumeParam, int const parameterIndex)
{
	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new WeightedListDialogEdit(table()->viewport(), "WeightedListDialogEdit", *this, getTemplateTableRow());

	// Mark this as the main weighted list item

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_weightedList);

	// Create the child items

	int const rowCount = triggerVolumeParam->getRawWeightedList()->size();
	WeightedListDialogEdit *weightedListDialogEdit = dynamic_cast<WeightedListDialogEdit *>(m_dataTypeEdit);
	NOT_NULL(weightedListDialogEdit);

	weightedListDialogEdit->createRows(rowCount, parameterIndex);
}

//-----------------------------------------------------------------------------
QString TableItemTriggerVolume::getTypeString() const
{
	return "Trigger Volume";
}

//-----------------------------------------------------------------------------
TriggerVolumeDialogEdit *TableItemTriggerVolume::getTriggerVolumeDialogEdit() const
{
	TriggerVolumeDialogEdit *triggerVolumeDialogEdit = dynamic_cast<TriggerVolumeDialogEdit *>(m_dataTypeEdit);

	return triggerVolumeDialogEdit;
}

//-----------------------------------------------------------------------------
void TableItemTriggerVolume::setContentFromEditor(QWidget *widget)
{
	if (getTriggerVolumeDialogEdit() != NULL)
	{
		QString text(getTriggerVolumeDialogEdit()->getText());
		setText(text);
	}
	else
	{
		TableItemBase::setContentFromEditor(widget);
	}
}

//-----------------------------------------------------------------------------
int TableItemTriggerVolume::getSupportedParameterProperties() const
{
	return TableItemBase::getSupportedParameterProperties();
}

// ============================================================================
