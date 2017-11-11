//======================================================================
//
// AttachmentsChassisEditor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "AttachmentsChassisEditor.h"
#include "AttachmentsChassisEditor.moc"

#include "AttachmentsChassisEditorDetails.h"
#include "AttachmentsChassisEditorList.h"
#include "AttachmentsChooser.h"
#include <qsplitter.h>
#include <vector>

//======================================================================

namespace AttachmentsChassisEditorNamespace
{
}
using namespace AttachmentsChassisEditorNamespace;

//----------------------------------------------------------------------

AttachmentsChassisEditor::AttachmentsChassisEditor(QWidget *_parent, char const *_name) :
QWidget(_parent, _name),
m_details(NULL),
m_list(NULL),
m_chooser(NULL)
{	
	QSplitter * const splitter = new QSplitter(this, "The Splitter");
	splitter->setOrientation (QSplitter::Horizontal);
	splitter->setOpaqueResize (true);
	
	QSplitter * const vSplitter = new QSplitter(splitter, "V Splitter");
	vSplitter->setOrientation (QSplitter::Vertical);
	vSplitter->setOpaqueResize (true);

	QGridLayout * const _layout = new QGridLayout(this);
	_layout->addWidget(splitter, 0, 0);

	m_list = new AttachmentsChassisEditorList(vSplitter, "lst");
	m_chooser = new AttachmentsChooser(vSplitter, "chooser");

	m_details = new AttachmentsChassisEditorDetails(splitter, "det", *this);

	if (!connect(m_details->getLineEditNameFilter(), SIGNAL(textChanged(const QString &)), m_list, SLOT(onLineEditNameFiltertextChanged(const QString &))))
		WARNING(true, ("AttachmentsChassisEditor connection failed"));

	if (!connect(m_list, SIGNAL(attachmentSelectionChanged(uint32, uint32, int, bool)), m_chooser, SLOT(setEditingAttachments(uint32, uint32, int, bool))))
		WARNING(true, ("AttachmentsChassisEditor connection failed"));

	m_details->setupState();
	m_chooser->setEditingAttachments(0, 0, 0, true);
} //lint !e429 // _layout not freed

//----------------------------------------------------------------------

AttachmentsChassisEditor::~AttachmentsChassisEditor()
{
	m_details = NULL; //lint !e423 //not a memory leak
	m_list = NULL; //lint !e423 //not a memory leak
	m_chooser = NULL; //lint !e423 //not a memory leak
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

void AttachmentsChassisEditor::getSelectedComponentsList(StringVector & result)
{
	if (NULL != m_list)
		m_list->getSelectedComponentsList(result);
}

//----------------------------------------------------------------------

void AttachmentsChassisEditor::handleComponentTypeFilterChange(IntVector const & componentTypes)
{
	if (NULL != m_list)
		m_list->handleComponentTypeFilterChange(componentTypes);
}

//----------------------------------------------------------------------

void AttachmentsChassisEditor::handleChassisNameChange(std::string const & chassisName)
{
	if (NULL != m_list)
		m_list->handleChassisNameChange(chassisName);
}

//----------------------------------------------------------------------

void AttachmentsChassisEditor::handleHideFlags(bool hideEmptyRows, bool hideEmptyCols, bool hideUninstallableRows, bool hideUninstallableCols)
{
	if (NULL != m_list)
		m_list->handleHideFlags(hideEmptyRows, hideEmptyCols, hideUninstallableRows, hideUninstallableCols);
}

//======================================================================
