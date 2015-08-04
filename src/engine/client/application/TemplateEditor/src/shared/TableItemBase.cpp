// ============================================================================
//
// TableItemBase.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemBase.h"

#include "ArrayDialogEdit.h"
#include "DataTypeEdit.h"
#include "ListDialogEdit.h"
#include "MustDeriveEdit.h"
#include "sharedTemplateDefinition/tpfTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "WeightedListDialogEdit.h"

// ============================================================================
//
// TableItemBase
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemBase::TableItemBase(TemplateTableRow &templateTableRow)
 : QTableItem(&templateTableRow.getParentTable(), QTableItem::WhenCurrent, "** This cell is not yet implemented **")
 , m_templateTableRow(templateTableRow)
 , m_dataTypeEdit(NULL)
{
}

//-----------------------------------------------------------------------------
QWidget *TableItemBase::createEditor() const
{
	// Return the already constructed editor. Do not delete this object that is
	// returned. This class is the owner.

	return m_dataTypeEdit;
}

//-----------------------------------------------------------------------------
void TableItemBase::initDefault()
{
	DEBUG_FATAL(true, ("N0eeds to be overriden by derived class"));
}

//-----------------------------------------------------------------------------
void TableItemBase::init(IntegerParam const *integerParam, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
	UNREF(integerParam);
	UNREF(parameterIndex);
	UNREF(weightedListIndex);
	UNREF(parameterProperty);

	DEBUG_FATAL(true, ("Needs to be overriden by derived class"));
}

//-----------------------------------------------------------------------------
void TableItemBase::init(FloatParam const *floatParam, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
	UNREF(floatParam);
	UNREF(parameterIndex);
	UNREF(weightedListIndex);
	UNREF(parameterProperty);

	DEBUG_FATAL(true, ("Needs to be overriden by derived class"));
}

//-----------------------------------------------------------------------------
void TableItemBase::init(StringParam const *stringParam, int const parameterIndex, int const weightedListIndex)
{
	UNREF(stringParam);
	UNREF(parameterIndex);
	UNREF(weightedListIndex);

	DEBUG_FATAL(true, ("Needs to be overriden by derived class"));
}

//-----------------------------------------------------------------------------
void TableItemBase::init(DynamicVariableParamData const *dynamicVariableParamData)
{
	UNREF(dynamicVariableParamData);

	DEBUG_FATAL(true, ("Needs to be overriden by derived class"));
}

////-----------------------------------------------------------------------------
//void TableItemBase::init(std::vector<DynamicVariableParamData *> *dynamicVariableParamDataVector)
//{
//	UNREF(dynamicVariableParamDataVector);
//
//	DEBUG_FATAL(true, ("Needs to be overriden by derived class"));
//}

////-----------------------------------------------------------------------------
//void TableItemBase::init(StringIdParam const *stringIdParam)
//{
//}

//-----------------------------------------------------------------------------
void TableItemBase::initializeEditor()
{
	// Update the initial value

	setContentFromEditor(m_dataTypeEdit);
}

// Force all TableItemBases to align left
//-----------------------------------------------------------------------------
int TableItemBase::alignment() const
{
	return Qt::AlignLeft;
}

//-----------------------------------------------------------------------------
int TableItemBase::getSupportedParameterProperties() const
{
	int supportedFlags = 0;
	
	supportedFlags |= TemplateTableRow::PP_simple;
	supportedFlags |= TemplateTableRow::PP_weightedList;
	supportedFlags |= TemplateTableRow::PP_derived;
	supportedFlags |= TemplateTableRow::PP_useParent;
	
	// If this row is a list, we can add append
	
	if (getTemplateTableRow().getListType() == TemplateTableRow::PP_list)
	{
		supportedFlags |= TemplateTableRow::PP_positive;
	}
	
	return supportedFlags;
}

//-----------------------------------------------------------------------------
int TableItemBase::getWeight() const
{
	return m_dataTypeEdit->getWeight();
}

// Get the row in the template table that this cell belongs too
//-----------------------------------------------------------------------------
TemplateTableRow &TableItemBase::getTemplateTableRow() const
{
	return m_templateTableRow;
}

// Get the template table that this cell belongs too
//-----------------------------------------------------------------------------
TemplateTable &TableItemBase::getParentTable() const
{
	return getTemplateTableRow().getParentTable();
}

//-----------------------------------------------------------------------------
void TableItemBase::initAsList()
{
	delete m_dataTypeEdit;
	m_dataTypeEdit = new ListDialogEdit(table()->viewport(), "ListDialogEdit", *this);
	NOT_NULL(m_dataTypeEdit);

	// See if this parameter is appended

	QString parameterName(m_templateTableRow.getParameterName());
	
	TpfTemplate *tpfTemplate = m_templateTableRow.getTpfTemplate();

	if (tpfTemplate != NULL)
	{
		bool append = m_templateTableRow.getTpfTemplate()->isAppend(parameterName.latin1());

		m_templateTableRow.setDeltaPositive(append);
		m_templateTableRow.setDeltaPercent(false);
	}
}

//-----------------------------------------------------------------------------
void TableItemBase::initAsArray()
{
	delete m_dataTypeEdit;
	m_dataTypeEdit = new ArrayDialogEdit(table()->viewport(), "ArrayDialogEdit", *this);
	NOT_NULL(m_dataTypeEdit);
}

//-----------------------------------------------------------------------------
void TableItemBase::initAsDerived()
{
	delete m_dataTypeEdit;
	m_dataTypeEdit = new MustDeriveEdit(table()->viewport(), "MustDeriveEdit", *this);
	NOT_NULL(m_dataTypeEdit);

	// Set the parent template table row to derived

	m_templateTableRow.addParameterProperty(TemplateTableRow::PP_derived);

	// See if this parameter is @derived at the top level

	QString const parameterName(m_templateTableRow.getParameterName());
	int const parameterIndex = m_templateTableRow.getParameterIndex();
	bool const pureVirtual = m_templateTableRow.getTpfTemplate()->isParamPureVirtual(parameterName.latin1(), false, parameterIndex);

	if (pureVirtual)
	{
		m_templateTableRow.setChecked(true);
		m_templateTableRow.setCheckEnabled(false);
	}
}

//-----------------------------------------------------------------------------
void TableItemBase::setContentFromEditor(QWidget *widget)
{
	ListDialogEdit *listDialogEdit = dynamic_cast<ListDialogEdit *>(widget);

	if (listDialogEdit != NULL)
	{
		QString text(listDialogEdit->getText());
		setText(text);
	}
	else
	{
		ArrayDialogEdit *arrayDialogEdit = dynamic_cast<ArrayDialogEdit *>(widget);

		if (arrayDialogEdit != NULL)
		{
			QString text(arrayDialogEdit->getText());
			setText(text);
		}
		else
		{
			WeightedListDialogEdit *weightedListDialogEdit = dynamic_cast<WeightedListDialogEdit *>(widget);

			if (weightedListDialogEdit != NULL)
			{
				QString text(weightedListDialogEdit->getText());
				setText(text);
			}
			else
			{
				MustDeriveEdit *mustDeriveEdit = dynamic_cast<MustDeriveEdit *>(widget);

				if (mustDeriveEdit != NULL)
				{
					QString text(mustDeriveEdit->getText());
					setText(text);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
WeightedListDialogEdit *TableItemBase::getWeightedListDialogEdit() const
{
	WeightedListDialogEdit *weightedListDialogEdit = dynamic_cast<WeightedListDialogEdit *>(m_dataTypeEdit);

	return weightedListDialogEdit;
}

//-----------------------------------------------------------------------------
void TableItemBase::update()
{
	setContentFromEditor(m_dataTypeEdit);
}

// ============================================================================
