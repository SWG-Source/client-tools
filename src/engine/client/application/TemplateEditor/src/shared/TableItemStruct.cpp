// ============================================================================
//
// TableItemStruct.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemStruct.h"

#include "ListDialogEdit.h"
#include "sharedTemplateDefinition/File.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateDataIterator.h"
#include "sharedTemplateDefinition/TemplateDefinitionFile.h"
#include "sharedTemplateDefinition/TpfFile.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "StructDialogEdit.h"
#include "TemplateLoader.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// TableItemStruct
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemStruct::TableItemStruct(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
void TableItemStruct::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
	UNREF(parameterProperty);
	UNREF(weightedListIndex);

	// See if we need to for this to a simple type

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
					StructParamOT *structParamOT = tpfTemplate->getStructParamOT(parameterName.latin1(), true, parameterIndex);
					NOT_NULL(structParamOT);

					initAsSingle(structParamOT);
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
QString TableItemStruct::getTypeString() const
{
	return "Structure";
}

//-----------------------------------------------------------------------------
void TableItemStruct::initAsSingle(StructParamOT const *structParamOT)
{
	// Create the editor

	if (m_dataTypeEdit == NULL)
	{
		m_dataTypeEdit = new StructDialogEdit(table()->viewport(), "StructDialogEdit", *this, getTemplateTableRow());
	}

	// Set the editor value

	TpfTemplate *structTemplate = dynamic_cast<TpfTemplate *>(structParamOT->getValue());
	NOT_NULL(structTemplate);

	getStructDialogEdit()->setValue(*structTemplate);
}

//-----------------------------------------------------------------------------
StructDialogEdit *TableItemStruct::getStructDialogEdit() const
{
	StructDialogEdit *structDialogEdit = dynamic_cast<StructDialogEdit *>(m_dataTypeEdit);
	NOT_NULL(structDialogEdit);

	return structDialogEdit;
}

//-----------------------------------------------------------------------------
void TableItemStruct::setContentFromEditor(QWidget *widget)
{
	StructDialogEdit *structDialogEdit = dynamic_cast<StructDialogEdit *>(widget);

	if (structDialogEdit != NULL)
	{
		QString text(structDialogEdit->getText());
		setText(text);
	}
	else
	{
		TableItemBase::setContentFromEditor(widget);
	}
}

//-----------------------------------------------------------------------------
int TableItemStruct::getSupportedParameterProperties() const
{
	return TableItemBase::getSupportedParameterProperties();
}

// ============================================================================
