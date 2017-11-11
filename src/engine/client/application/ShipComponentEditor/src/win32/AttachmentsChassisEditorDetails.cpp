//======================================================================
//
// AttachmentsChassisEditorDetails.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "AttachmentsChassisEditorDetails.h"
#include "AttachmentsChassisEditorDetails.moc"

#include "AttachmentsChassisEditor.h"
#include "QStringUtil.h"
#include "ShipComponentEditor.h"
#include "TemplateNewDialog.h"
#include "UnicodeUtils.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipChassisWritable.h"
#include "sharedGame/ShipComponentType.h"
#include "sharedMessageDispatch/Transceiver.h"
#include <qtabwidget.h>
#include <vector>

//======================================================================

namespace AttachmentsChassisEditorDetailsNamespace
{
}

using namespace AttachmentsChassisEditorDetailsNamespace;

//----------------------------------------------------------------------

AttachmentsChassisEditorDetails::AttachmentsChassisEditorDetails(QWidget *_parent, char const *_name, AttachmentsChassisEditor & editor) :
BaseAttachmentsChassisEditorDetails(_parent, _name),
m_editor(&editor),
m_callback(new MessageDispatch::Callback)
{
	IGNORE_RETURN(connect(m_checkHideEmptyRows, SIGNAL(toggled(bool)), SLOT(onCheckHideGenericToggled(bool))));
	IGNORE_RETURN(connect(m_checkHideEmptyCols, SIGNAL(toggled(bool)), SLOT(onCheckHideGenericToggled(bool))));
	IGNORE_RETURN(connect(m_checkHideUninstallableRows, SIGNAL(toggled(bool)), SLOT(onCheckHideGenericToggled(bool))));
	IGNORE_RETURN(connect(m_checkHideUninstallableCols, SIGNAL(toggled(bool)), SLOT(onCheckHideGenericToggled(bool))));
	IGNORE_RETURN(connect(m_lineEditNameFilter, SIGNAL(textChanged(const QString &)), SLOT(onLineEditNameFiltertextChanged(const QString &))));
	IGNORE_RETURN(connect(m_listComponentTypes, SIGNAL(selectionChanged()), SLOT(onListComponentTypesSelectionChanged())));
	IGNORE_RETURN(connect(m_comboChassis, SIGNAL(activated(const QString &)), SLOT(onComboChassisHighlighted(const QString &))));
	
	resetListComponentTypes();
	resetComboChassisTypes();

	onComboChassisHighlighted(m_comboChassis->currentText());

	m_callback->connect(*this, &AttachmentsChassisEditorDetails::onCallbackChassisListChanged, static_cast<ShipChassisWritable::Messages::ChassisListChanged *>(NULL));
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorDetails::setupState()
{
	m_checkHideEmptyRows->setChecked(false);
	m_checkHideEmptyCols->setChecked(false);
	m_checkHideUninstallableRows->setChecked(true);
	m_checkHideUninstallableCols->setChecked(true);
}

//----------------------------------------------------------------------

AttachmentsChassisEditorDetails::~AttachmentsChassisEditorDetails()
{
	delete m_callback;
	m_callback = NULL;

	m_editor = NULL;
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void AttachmentsChassisEditorDetails::onLineEditNameFiltertextChanged(QString const & text)
{
	m_lineEditNameFilter->setText(Unicode::toLower(QStringUtil::toString(text)).c_str());
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorDetails::onListComponentTypesHighlighted(const QString &)
{
}  //lint !e1762 // could not be made const

//----------------------------------------------------------------------

void AttachmentsChassisEditorDetails::onListComponentTypesSelectionChanged()
{
	AttachmentsChassisEditor::IntVector iv;

	int const count = m_listComponentTypes->numRows();

	for (int i = 0; i < count; ++i)
	{
		if (m_listComponentTypes->isSelected(i))
		{
			if (0 == i)
				break;

			std::string itemString = QStringUtil::toString(m_listComponentTypes->text(i));
			size_t const spacepos = itemString.find(' ');
			if (static_cast<size_t>(std::string::npos) != spacepos)
			{
				itemString = itemString.substr(0, spacepos);
			}

			int const componentType = static_cast<int>(ShipComponentType::getTypeFromName(itemString));

			iv.push_back(componentType);
		}
	}

	m_editor->handleComponentTypeFilterChange(iv);
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorDetails::onComboChassisHighlighted(const QString & chassisName)
{
	m_editor->handleChassisNameChange(QStringUtil::toString(chassisName));
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorDetails::onCheckHideGenericToggled(bool)
{
	m_editor->handleHideFlags(m_checkHideEmptyRows->isChecked(), m_checkHideEmptyCols->isChecked(), m_checkHideUninstallableRows->isChecked(), m_checkHideUninstallableCols->isChecked());
}

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- CALLBACKS
//----------------------------------------------------------------------

void AttachmentsChassisEditorDetails::onCallbackChassisListChanged(bool const & )
{
	resetComboChassisTypes();
}

//----------------------------------------------------------------------
//-- End CALLBACKS
//----------------------------------------------------------------------

//----------------------------------------------------------------------

void AttachmentsChassisEditorDetails::resetListComponentTypes()
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

void AttachmentsChassisEditorDetails::resetComboChassisTypes()
{
	m_comboChassis->clear();

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
			m_comboChassis->insertItem(chassisName.c_str());
		}
	}

	m_comboChassis->setCurrentItem(0);
}

//----------------------------------------------------------------------

QLineEdit * AttachmentsChassisEditorDetails::getLineEditNameFilter()
{
	return m_lineEditNameFilter;
}

//======================================================================
