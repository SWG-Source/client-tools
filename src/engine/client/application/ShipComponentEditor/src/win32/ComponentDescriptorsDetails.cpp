//======================================================================
//
// ComponentDescriptorsDetails.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "ComponentDescriptorsDetails.h"
#include "ComponentDescriptorsDetails.moc"

#include "ComponentDescriptors.h"
#include "QStringUtil.h"
#include "ShipComponentEditor.h"
#include "ShipComponentEditorServerTemplate.h"
#include "ShipComponentEditorServerTemplateManager.h"
#include "TemplateNewDialog.h"
#include "UnicodeUtils.h"
#include "clientAudio/Audio.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipChassisWritable.h"
#include "sharedGame/ShipComponentType.h"
#include "sharedMath/Vector.h"
#include "sharedMessageDispatch/Transceiver.h"
#include <qtabwidget.h>
#include <vector>

//======================================================================

namespace ComponentDescriptorsDetailsNamespace
{
}

using namespace ComponentDescriptorsDetailsNamespace;

//----------------------------------------------------------------------

ComponentDescriptorsDetails::ComponentDescriptorsDetails(QWidget *parent, char const *name, ComponentDescriptors & descriptors) :
BaseComponentDescriptorsDetails(parent, name),
m_componentDescriptors(&descriptors),
m_callback(new MessageDispatch::Callback)
{

	/*
	m_listComponentTypes
	m_listChassisTypes
	*/

	connect(m_lineEditNameFilter, SIGNAL(textChanged(const QString &)), SLOT(onLineEditNameFiltertextChanged(const QString &)));

	connect(m_listComponentTypes, SIGNAL(selectionChanged()), SLOT(onListComponentTypesSelectionChanged()));
	connect(m_listChassisTypes, SIGNAL(selectionChanged()), SLOT(onListChassisSelectionChanged()));
	
	resetListComponentTypes();
	resetListChassisTypes();

	m_callback->connect(*this, &ComponentDescriptorsDetails::onCallbackChassisListChanged, static_cast<ShipChassisWritable::Messages::ChassisListChanged *>(NULL));

}

//----------------------------------------------------------------------

ComponentDescriptorsDetails::~ComponentDescriptorsDetails()
{
	delete m_callback;
	m_callback = NULL;
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void ComponentDescriptorsDetails::onLineEditNameFiltertextChanged(QString const & text)
{
	m_lineEditNameFilter->setText(Unicode::toLower(QStringUtil::toString(text)).c_str());
}

//----------------------------------------------------------------------

void ComponentDescriptorsDetails::onListComponentTypesHighlighted(const QString &)
{
}

//----------------------------------------------------------------------

void ComponentDescriptorsDetails::onListChassisTypesHighlighted(const QString &)
{
}

//----------------------------------------------------------------------

void ComponentDescriptorsDetails::onListComponentTypesSelectionChanged()
{
	ComponentDescriptors::IntVector iv;

	int const count = m_listComponentTypes->numRows();

	for (int i = 0; i < count; ++i)
	{
		if (m_listComponentTypes->isSelected(i))
		{
			if (0 == i)
				break;

			std::string itemString = QStringUtil::toString(m_listComponentTypes->text(i));
			size_t const spacepos = itemString.find(' ');
			if (std::string::npos != spacepos)
			{
				itemString = itemString.substr(0, spacepos);
			}

			int const componentType = static_cast<int>(ShipComponentType::getTypeFromName(itemString));

			iv.push_back(componentType);
		}
	}

	m_componentDescriptors->handleComponentTypeFilterChange(iv);
}

//----------------------------------------------------------------------

void ComponentDescriptorsDetails::onListChassisSelectionChanged()
{
	ComponentDescriptors::CrcVector cv;

	int const count = m_listComponentTypes->numRows();

	for (int i = 0; i < count; ++i)
	{
		if (m_listComponentTypes->isSelected(i))
		{
			if (0 == i)
				break;

			std::string chassisString = QStringUtil::toString(m_listComponentTypes->text(i));
			size_t const spacepos = chassisString.find(' ');
			if (std::string::npos != spacepos)
			{
				chassisString = chassisString.substr(0, spacepos);
			}

			ShipChassis const * const chassis = ShipChassis::findShipChassisByName(ConstCharCrcString(chassisString.c_str()));

			if (NULL == chassis)
			{
				WARNING(true, ("ComponentDescriptorsDetails Invalid chassis in filter list [%s]", chassisString.c_str()));
			}
			else
				cv.push_back(chassis->getCrc());

		}
	}

	m_componentDescriptors->handleChassisFilterChange(cv);
}

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- CALLBACKS
//----------------------------------------------------------------------

void ComponentDescriptorsDetails::onCallbackChassisListChanged(bool const & )
{
	resetListChassisTypes();
}

//----------------------------------------------------------------------
//-- End CALLBACKS
//----------------------------------------------------------------------

//----------------------------------------------------------------------

void ComponentDescriptorsDetails::resetListComponentTypes()
{
	m_listComponentTypes->clear();

	m_listComponentTypes->insertItem(QString("--- ALL COMPONENT TYPES ---"));
	
	{
		for (int componentType = 0; componentType < static_cast<int>(ShipComponentType::SCT_num_types); ++componentType)
		{
			std::string const & componentTypeName = ShipComponentType::getNameFromType(static_cast<ShipComponentType::Type>(componentType));
			
			m_listComponentTypes->insertItem(QString(componentTypeName.c_str()));
		}
	}
}

//----------------------------------------------------------------------

void ComponentDescriptorsDetails::resetListChassisTypes()
{
	m_listChassisTypes->clear();
	m_listChassisTypes->insertItem(QString("--- ALL CHASSIS TYPES ---"));

	typedef stdvector<std::string>::fwd StringVector;
	StringVector sv;
	{
		ShipChassis::PersistentCrcStringVector const & shipChassisNameVector = ShipChassis::getShipChassisCrcVector();
		for (ShipChassis::PersistentCrcStringVector::const_iterator it = shipChassisNameVector.begin(); it != shipChassisNameVector.end(); ++it)
		{
			PersistentCrcString const * const pcs = *it;
			sv.push_back(pcs->getString());
		}

		std::sort(sv.begin(), sv.end());
	}

	{
		for (StringVector::const_iterator it = sv.begin(); it != sv.end(); ++it)
		{
			std::string const & chassisName = *it;
			m_listChassisTypes->insertItem(chassisName.c_str());
		}
	}

	m_listChassisTypes->setCurrentItem(0);
}

//----------------------------------------------------------------------

QLineEdit * ComponentDescriptorsDetails::getLineEditNameFilter()
{
	return m_lineEditNameFilter;
}

//======================================================================
