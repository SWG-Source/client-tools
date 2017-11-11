// ============================================================================
//
// TemplateEditorManager.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TemplateEditorManager.h"
#include "TemplateEditorManager.moc"

#include "sharedTemplateDefinition/File.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateDataIterator.h"
#include "sharedTemplateDefinition/TemplateDefinitionFile.h"
#include "sharedTemplateDefinition/TpfFile.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "TemplateEditor.h"
#include "TemplateLoader.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// TemplateTableRow
//
// ============================================================================

//-----------------------------------------------------------------------------
TemplateEditorManager::TemplateEditorManager(TemplateEditor &templateEditor)
 : m_templateEditor(&templateEditor)
{
	m_templateEditor->m_templateTable->setTemplateEditorManager(*this);

	TemplateLoader::install();
}

//-----------------------------------------------------------------------------
TemplateEditorManager::~TemplateEditorManager()
{
	TemplateLoader::remove();
}

////-----------------------------------------------------------------------------
//void TemplateEditorManager::initializeWithTDF(QString const &path)
//{
//	// Load the template definition data
//
//	TemplateLoader::loadTdf(path);
//}

//-----------------------------------------------------------------------------
void TemplateEditorManager::initializeWithTemplate(QString const &path)
{
	// Load the template definition data
	// Check for TpfTdfMismatch, DHERMAN

	TemplateLoader::loadTpf(path);
}

//-----------------------------------------------------------------------------
TemplateTable *TemplateEditorManager::getTemplateTable() const
{
	return m_templateEditor->m_templateTable;
}

//-----------------------------------------------------------------------------
void TemplateEditorManager::describeRow(TemplateTableRow const *templateTableRow) const
{
	QLabel *parameterNameLabel = m_templateEditor->m_parameterNameLabel;
	QListView *descriptionListView = m_templateEditor->m_descriptionListView;
	QTextEdit *parameterComments = m_templateEditor->m_commentsTextEdit;

	parameterNameLabel->clear();
	descriptionListView->clear();
	parameterComments->clear();

	if (templateTableRow != NULL)
	{
		// Set the name label

		parameterNameLabel->setText(templateTableRow->getParameterName());

		// Prepare the description box

		descriptionListView->setSorting(-1);
		descriptionListView->clear();

		// Prepare the description

		QListViewItem* paramType = new QListViewItem(descriptionListView);

		paramType->setText(0, "Parameter Type");
		paramType->setText(1, templateTableRow->getType());

		QListViewItem* listType = new QListViewItem(descriptionListView, paramType);

		listType->setText(0, "List Type");

		switch(templateTableRow->getListType())
		{
			case TemplateTableRow::PP_simple:
				{
					listType->setText(1, "Simple");
				}
				break;
			case TemplateTableRow::PP_list:
				{
					listType->setText(1, "List");
				}
				break;
			case TemplateTableRow::PP_array:
				{
					listType->setText(1, "Array");
				}
				break;
			default:
				{
					listType->setText(1, "Unexpected list type?!");
				}
				break;
		}
		
		// Add any other interesting pieces of information here

		// Show comment for this item

		parameterComments->setText(templateTableRow->getComment());
	}
}

//-----------------------------------------------------------------------------
void TemplateEditorManager::initializeTemplateTable(bool const quickLoad)
{
	// Initialize the template table to the specified definition

	bool const addBaseParameter = true;

	m_templateEditor->m_templateTable->load(TemplateLoader::getTemplateData(), &TemplateLoader::getTpfTemplate(), quickLoad, addBaseParameter);

	describeRow(m_templateEditor->m_templateTable->getTemplateTableRow(0));
}

//-----------------------------------------------------------------------------
void TemplateEditorManager::autoSave()
{
	if (m_templateEditor != NULL)
	{
		m_templateEditor->autoSave();
	}
}

// ============================================================================
