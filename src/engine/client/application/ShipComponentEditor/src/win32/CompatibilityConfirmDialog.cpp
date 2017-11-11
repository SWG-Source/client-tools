//======================================================================
//
// CompatibilityConfirmDialog.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "CompatibilityConfirmDialog.h"
#include "CompatibilityConfirmDialog.moc"

#include "QStringUtil.h"
#include "ShipComponentEditorServerTemplate.h"
#include "ShipComponentEditorServerTemplateManager.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipChassisWritable.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentDescriptorWritable.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include <map>
#include <vector>

//======================================================================

namespace CompatibilityConfirmDialogNamespace
{

}

using namespace CompatibilityConfirmDialogNamespace;

//----------------------------------------------------------------------

CompatibilityConfirmDialog::CompatibilityConfirmDialog(QWidget *parent, char const *name, std::string const & oldCompat, std::string const & newCompat, StringVector const & chassisSlotsAffected, StringVector const & componentsAffected) :
BaseCompatibilityConfirmDialog(parent, name, true),
m_ok(false)
{
	setupSignals();

	populateLists(chassisSlotsAffected, componentsAffected);

	if (newCompat.empty())
		m_textMessage->setText(QString((std::string("Deleting the [") + oldCompat + "] compatibility will affect the following slots & components:").c_str()));
	else
		m_textMessage->setText(QString((std::string("Renaming the [") + oldCompat + "] compatibility to [" + newCompat + "] will affect the following slots & components:").c_str()));
}

//----------------------------------------------------------------------

void CompatibilityConfirmDialog::setupSignals()
{
	connect(m_buttonOk, SIGNAL(clicked()), SLOT(onPushButtonOkClicked()));
	connect(m_buttonCancel, SIGNAL(clicked()), SLOT(onPushButtonCancelClicked()));
}

//----------------------------------------------------------------------

CompatibilityConfirmDialog::~CompatibilityConfirmDialog()
{
	
}

//----------------------------------------------------------------------

void CompatibilityConfirmDialog::populateLists(StringVector const & chassisSlotsAffected, StringVector const & componentsAffected)
{	
	{
		m_tableSlots->setNumRows(chassisSlotsAffected.size());

	
		{
			int row = 0;
			for (StringVector::const_iterator it = chassisSlotsAffected.begin(); it != chassisSlotsAffected.end(); ++it, ++row)
			{
				std::string const & sel = *it;
				size_t const spacepos = sel.find(' ');

				if (std::string::npos != spacepos)
				{
					m_tableSlots->setText(row, 0, QString(sel.substr(0, spacepos).c_str()));
					m_tableSlots->setText(row, 1, QString(sel.substr(spacepos + 1).c_str()));
				}
				else
				{
					m_tableSlots->setText(row, 0, QString(sel.c_str()));
				}
			}
		}
		
		m_tableSlots->adjustColumn(0);
		m_tableSlots->adjustColumn(1);
		m_tableSlots->sortColumn(0);

	}

	{
		m_listComponents->clear();
		
		{
			for (StringVector::const_iterator it = componentsAffected.begin(); it != componentsAffected.end(); ++it)
			{
				std::string const & sel = *it;
				m_listComponents->insertItem(QString(sel.c_str()));
			}
		}
		
		m_listComponents->sort();
	}
}

//----------------------------------------------------------------------

bool CompatibilityConfirmDialog::showAndTell()
{
	exec();
	return m_ok;
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void CompatibilityConfirmDialog::onPushButtonOkClicked()
{
	hide();
	m_ok = true;
}

//----------------------------------------------------------------------

void CompatibilityConfirmDialog::onPushButtonCancelClicked()
{
	hide();
	m_ok = false;
}

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

//======================================================================
