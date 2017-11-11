// ============================================================================
//
// TableItemVector.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemVector.h"

#include "sharedUtility/TemplateParameter.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "VectorDialogEdit.h"


// ============================================================================
//
// TableItemVector
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemVector::TableItemVector(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
TableItemVector::~TableItemVector()
{
}

//-----------------------------------------------------------------------------
void TableItemVector::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
	UNREF(tpfTemplate);
	UNREF(parameterName);
	UNREF(parameterIndex);
	UNREF(weightedListIndex);
	UNREF(parameterProperty);
}

//-----------------------------------------------------------------------------
void TableItemVector::init()
{
	//TemplateTableRow const &parentTemplateTableRow = getTemplateTableRow();
	//
	//if(parentTemplateTableRow.getParameterTypes() & TemplateTableRow::PT_literal)
	//{
	//	initAsSingle(parameterData);
	//}

	//TableItemBase::init();
}

//-----------------------------------------------------------------------------
QString TableItemVector::getTypeString() const
{
	return "Vector";
}

////-----------------------------------------------------------------------------
//void TableItemVector::initAsSingle(const VectorParam *parameterData)
//{
//	if(m_vectorEditor == NULL)
//	{
//		m_vectorEditor = new VectorDialogEdit(this);
//	}
//
//	if(parameterData != NULL)
//	{
//		VectorParamData vectorParamData = parameterData->getValue();
//		m_vectorEditor->setValue(vectorParamData);
//	}
//
//	setCurrentEditor(*m_vectorEditor);
//}

//-----------------------------------------------------------------------------
void TableItemVector::setContentFromEditor(QWidget *widget)
{
	VectorDialogEdit *vectorDialogEdit = dynamic_cast<VectorDialogEdit *>(widget);

	if (vectorDialogEdit != NULL)
	{
		QString text(vectorDialogEdit->getText());
		setText(text);
	}
	else
	{
		TableItemBase::setContentFromEditor(widget);
	}
}

//-----------------------------------------------------------------------------
int TableItemVector::getSupportedParameterProperties() const
{
	return TableItemBase::getSupportedParameterProperties();
}

// ============================================================================
