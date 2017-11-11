//======================================================================
//
// ComponentDescriptors.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "ComponentDescriptors.h"
#include "ComponentDescriptors.moc"

#include "ComponentDescriptorsList.h"
#include "ComponentDescriptorsDetails.h"
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

namespace ComponentDescriptorsNamespace
{
}
using namespace ComponentDescriptorsNamespace;

//----------------------------------------------------------------------

ComponentDescriptors::ComponentDescriptors(QWidget *parent, char const *name) :
BaseComponentDescriptors(parent, name),
m_details(NULL),
m_list(NULL)
{	
	QSplitter * const splitter = new QSplitter(this, "The Splitter");
	splitter->setOrientation (QSplitter::Horizontal);
	splitter->setOpaqueResize (true);
	
	QGridLayout * const layout = new QGridLayout(this);
	layout->addWidget(splitter, 0, 0);

	m_details = new ComponentDescriptorsDetails(splitter, "det", *this);
	m_list = new ComponentDescriptorsList(splitter, "lst");

	connect(m_details->getLineEditNameFilter(), SIGNAL(textChanged(const QString &)), m_list, SLOT(onLineEditNameFiltertextChanged(const QString &)));
}

//----------------------------------------------------------------------

ComponentDescriptors::~ComponentDescriptors()
{
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

void ComponentDescriptors::getSelectedComponentsList(StringVector & result)
{
	if (NULL != m_list)
		m_list->getSelectedComponentsList(result);
}

//----------------------------------------------------------------------

void ComponentDescriptors::handleComponentTypeFilterChange(IntVector const & componentTypes)
{
	m_list->handleComponentTypeFilterChange(componentTypes);
}

//----------------------------------------------------------------------

void ComponentDescriptors::handleChassisFilterChange(CrcVector const & chassisCrcs)
{
	m_list->handleChassisFilterChange(chassisCrcs);
}

//======================================================================
