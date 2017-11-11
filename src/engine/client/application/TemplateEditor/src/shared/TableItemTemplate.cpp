// ============================================================================
//
// TableItemTemplate.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemTemplate.h"

#include "FilenameDialogEdit.h"
#include "ListDialogEdit.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "TemplateLoader.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "WeightedListDialogEdit.h"

// ============================================================================
//
// TableItemTemplate
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemTemplate::TableItemTemplate(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
void TableItemTemplate::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
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
						StringParam const *stringParam = tpfTemplate->getStringParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(stringParam);

						init(stringParam, parameterIndex, weightedListIndex);
					}
					else if (tpfTemplate->getListLength(parameterName.latin1()) > 0)
					{
						DEBUG_FATAL((parameterIndex >= tpfTemplate->getListLength(parameterName.latin1())), ("Invalid list parameter index %d of %d", parameterIndex, tpfTemplate->getListLength(parameterName.latin1())));

						// This is a list value

						StringParam const *stringParam = tpfTemplate->getStringParam(parameterName.latin1(), true, parameterIndex);
						NOT_NULL(stringParam);

						init(stringParam, parameterIndex, weightedListIndex);
					}
					else if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
					{
						// This is a weighted list element

						StringParam const *stringParam = tpfTemplate->getStringParam(parameterName.latin1(), true, 0);
						NOT_NULL(stringParam);

						init(stringParam, parameterIndex, weightedListIndex);
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
void TableItemTemplate::init(StringParam const *stringParam, int const parameterIndex, int const weightedListIndex)
{
	// Initialize to the correct integer type

	StringParam::DataTypeId dataTypeId = stringParam->getType();

	switch (dataTypeId)
	{
		case StringParam::SINGLE:
			{
				initAsSingle(stringParam);
			}
			break;
		case StringParam::WEIGHTED_LIST:
			{
				if (getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
				{
					StringParam::WeightedList const *weightedList = stringParam->getRawWeightedList();
					StringParam::WeightedValue const &weightedValue = (*weightedList)[weightedListIndex];
					StringParam *newStringParam = dynamic_cast<StringParam *>(weightedValue.value);
					NOT_NULL(newStringParam);

					init(newStringParam, parameterIndex, weightedListIndex);

					// Set the weight

					m_dataTypeEdit->setWeightedItemStatus(true);
					m_dataTypeEdit->setWeight(weightedValue.weight);
				}
				else
				{
					initAsWeightedList(stringParam, parameterIndex);
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
void TableItemTemplate::initAsSingle(StringParam const *stringParam)
{
	// Create the table item if it has not been created

	delete m_dataTypeEdit;
	m_dataTypeEdit = new FilenameDialogEdit(table()->viewport(), "FilenameDialogEdit", *this);
	NOT_NULL(m_dataTypeEdit);

	// Set the table item value

	QString path(stringParam->getValue().c_str());

	getFileNameDialogEdit()->setFilter("Object Template Files (*.iff)");
	getFileNameDialogEdit()->setPath(path);
}

//-----------------------------------------------------------------------------
void TableItemTemplate::initAsWeightedList(StringParam const *stringParam, int const parameterIndex)
{
	// Create the editor

	delete m_dataTypeEdit;
	m_dataTypeEdit = new WeightedListDialogEdit(table()->viewport(), "WeightedListDialogEdit", *this, getTemplateTableRow());

	// Mark this as the main weighted list item

	getTemplateTableRow().addParameterProperty(TemplateTableRow::PP_weightedList);

	// Create the child items

	int const rowCount = stringParam->getRawWeightedList()->size();
	WeightedListDialogEdit *weightedListDialogEdit = dynamic_cast<WeightedListDialogEdit *>(m_dataTypeEdit);
	NOT_NULL(weightedListDialogEdit);

	weightedListDialogEdit->createRows(rowCount, parameterIndex);
}

//-----------------------------------------------------------------------------
QString TableItemTemplate::getTypeString() const
{
	return "Template";
}

//-----------------------------------------------------------------------------
FilenameDialogEdit *TableItemTemplate::getFileNameDialogEdit()
{
	FilenameDialogEdit *fileNameDialogEdit = dynamic_cast<FilenameDialogEdit *>(m_dataTypeEdit);

	return fileNameDialogEdit;
}

//-----------------------------------------------------------------------------
void TableItemTemplate::setContentFromEditor(QWidget *widget)
{
	if (getFileNameDialogEdit() != NULL)
	{
		QString text(getFileNameDialogEdit()->getText());
		setText(text);
	}
	else
	{
		TableItemBase::setContentFromEditor(widget);
	}
}

//-----------------------------------------------------------------------------
int TableItemTemplate::getSupportedParameterProperties() const
{
	return TableItemBase::getSupportedParameterProperties();
}

// ============================================================================
