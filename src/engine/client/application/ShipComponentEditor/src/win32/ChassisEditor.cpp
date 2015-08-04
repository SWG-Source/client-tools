//======================================================================
//
// ChassisEditor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "ChassisEditor.h"
#include "ChassisEditor.moc"

#include "ChassisEditorList.h"
#include "ChassisEditorDetails.h"
#include "UnicodeUtils.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipChassisWritable.h"
#include "clientAudio/Audio.h"
#include "sharedMath/Vector.h"
#include "ShipComponentEditorServerTemplateManager.h"
#include "ShipComponentEditorServerTemplate.h"

#include <vector>
#include <qsplitter.h>

//======================================================================

namespace ChassisEditorNamespace
{

}
using namespace ChassisEditorNamespace;

//----------------------------------------------------------------------

ChassisEditor::ChassisEditor(QWidget *parent, char const *name) :
BaseChassisEditor(parent, name),
m_details(NULL),
m_list(NULL)
{	
	QSplitter * const splitter = new QSplitter(this, "The Splitter");
	splitter->setOrientation (QSplitter::Horizontal);
	splitter->setOpaqueResize (true);
	
	QGridLayout * const layout = new QGridLayout(this);
	layout->addWidget(splitter, 0, 0);

	m_details = new ChassisEditorDetails(splitter, "det", *this);
	m_list = new ChassisEditorList(splitter, "lst");

	connect(m_details->getLineEditNameFilter(), SIGNAL(textChanged(const QString &)), m_list, SLOT(onLineEditNameFiltertextChanged(const QString &)));

	//-- setup details slots
	connect(m_list->getTable(), SIGNAL(selectionChanged ()), m_details, SLOT(onTableSelectionChanged ()));
	connect(m_list->getTable(), SIGNAL(currentChanged(int, int)), m_details, SLOT(onTableSelectionChanged ()));
}

//----------------------------------------------------------------------

ChassisEditor::~ChassisEditor()
{
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

void ChassisEditor::getSelectedChassisList(StringVector & result)
{
	if (NULL != m_list)
		m_list->getSelectedChassisList(result);
}


//======================================================================
