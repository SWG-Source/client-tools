// ============================================================================
//
// StringIDDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "StringIDDialog.h"

#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "TableItemBase.h"
#include "TemplateLoader.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// StringIDDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
StringIDDialog::StringIDDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow)
 : StaticListDialog(parent, name, parentTemplateTableRow)
 , m_templateTableRowString(NULL)
 , m_templateTableRowIndex(NULL)
{
	// Add the string row

	m_templateTableRowString = new TemplateTableRow(*m_templateTable, 0, TemplateData::TYPE_STRING, "Table Name");
	m_templateTableRowString->setChecked(true);
	m_templateTableRowString->setCheckEnabled(false);

	// Add the index row

	m_templateTableRowIndex = new TemplateTableRow(*m_templateTable, 1, TemplateData::TYPE_STRING, "Base Name");
	m_templateTableRowIndex->setChecked(true);
	m_templateTableRowIndex->setCheckEnabled(false);
}

//-----------------------------------------------------------------------------
void StringIDDialog::setValue(const StringId &stringId)
{
	StringParam tableName;
	tableName.setValue(stringId.getTable());
	m_templateTableRowString->getColumnValue()->init(&tableName, 0, 0);

	StringParam indexName;
	indexName.setValue(stringId.getText());
	m_templateTableRowIndex->getColumnValue()->init(&indexName, 0, 0);
}

//-----------------------------------------------------------------------------
QString StringIDDialog::getTableName() const
{
	return m_templateTableRowString->getColumnValue()->text();
}

//-----------------------------------------------------------------------------
QString StringIDDialog::getIndexName() const
{
	return m_templateTableRowIndex->getColumnValue()->text();
}

// ============================================================================
