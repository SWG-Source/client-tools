// ============================================================================
//
// TableItemAtBase.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemAtBase.h"

#include "FilenameDialogEdit.h"
#include "ListDialogEdit.h"
#include "sharedUtility/TemplateParameter.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "TemplateLoader.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// TableItemAtBase
//
// ============================================================================

//-----------------------------------------------------------------------------
TableItemAtBase::TableItemAtBase(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
void TableItemAtBase::init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty)
{
	UNREF(tpfTemplate);
	UNREF(parameterName);
	UNREF(parameterIndex);
	UNREF(weightedListIndex);
	UNREF(parameterProperty);

	DEBUG_FATAL((parameterProperty != TemplateTableRow::PP_simple), ("Invalid parameter property"));

	if (m_dataTypeEdit == NULL)
	{
		m_dataTypeEdit = new FilenameDialogEdit(table()->viewport(), "FilenameDialogEdit", *this);

		getFilenameDialogEdit()->setFilter("Object Template Files (*.iff)");
	}

	TableItemBase::initializeEditor();
}

//-----------------------------------------------------------------------------
QString TableItemAtBase::getTypeString() const
{
	return "Base Name";
}

//-----------------------------------------------------------------------------
int TableItemAtBase::getSupportedRowTypes() const
{
	return TemplateTableRow::PP_simple;
}

//-----------------------------------------------------------------------------
void TableItemAtBase::setPath(QString const &path)
{
	getFilenameDialogEdit()->setPath(path);
	setText(path);
}

//-----------------------------------------------------------------------------
void TableItemAtBase::setContentFromEditor(QWidget *widget)
{
	FilenameDialogEdit *fileNameDialogEdit = dynamic_cast<FilenameDialogEdit *>(widget);

	if (fileNameDialogEdit != NULL)
	{
		QString text(fileNameDialogEdit->getText());
		setText(text);
	}
	else
	{
		TableItemBase::setContentFromEditor(widget);
	}
}

//-----------------------------------------------------------------------------
FilenameDialogEdit *TableItemAtBase::getFilenameDialogEdit() const
{
	FilenameDialogEdit *fileNameDialogEdit = dynamic_cast<FilenameDialogEdit *>(m_dataTypeEdit);
	NOT_NULL(fileNameDialogEdit);

	return fileNameDialogEdit;
}
	
//-----------------------------------------------------------------------------
int TableItemAtBase::getSupportedParameterProperties() const
{
	return TableItemBase::getSupportedParameterProperties();
}

// ============================================================================
