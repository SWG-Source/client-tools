//======================================================================
//
// CompatibilityEditor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "CompatibilityEditor.h"
#include "CompatibilityEditor.moc"

#include "CompatibilityConfirmDialog.h"
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

namespace CompatibilityEditorNamespace
{
	typedef CompatibilityEditor::StringVector StringVector;

	//----------------------------------------------------------------------

	void getAvailableCompatibilities(StringVector & sv)
	{		
		ShipChassis::PersistentCrcStringVector const & chassisNames = ShipChassis::getShipChassisCrcVector();
		{
			for (ShipChassis::PersistentCrcStringVector::const_iterator it = chassisNames.begin(); it != chassisNames.end(); ++it)
			{
				PersistentCrcString const * const pcs = NON_NULL(*it);
				
				ShipChassis const * const shipChassis = NON_NULL(ShipChassis::findShipChassisByCrc(pcs->getCrc()));
				
				ShipChassis::SlotVector const & shipSlots = shipChassis->getSlots();
				
				for (ShipChassis::SlotVector::const_iterator sit = shipSlots.begin(); sit != shipSlots.end(); ++sit)
				{
					ShipChassisSlot const & chassisSlot = *sit;
					
					ShipChassisSlot::CompatibilityVector const & compats = chassisSlot.getCompatibilities();
					
					for (ShipChassisSlot::CompatibilityVector::const_iterator cit = compats.begin(); cit != compats.end(); ++cit)
					{
						CrcString const * const cs = NON_NULL(*cit);
						std::string const nameStr(cs->getString());
						
						if (!std::binary_search(sv.begin(), sv.end(), nameStr))
						{
							sv.push_back(nameStr);
							std::sort(sv.begin(), sv.end());
						}
					}
				}
			}
		}
		
		
		ShipComponentDescriptor::NameComponentMap const & nameComponentMap = ShipComponentDescriptor::getNameComponentMap();
		{
			for (ShipComponentDescriptor::NameComponentMap::const_iterator it = nameComponentMap.begin(); it != nameComponentMap.end(); ++it)
			{
				ShipComponentDescriptor const * const shipComponentDescriptor = NON_NULL((*it).second);
				std::string const & nameStr = shipComponentDescriptor->getCompatibility().getString();
				if (nameStr.empty())
					continue;

				if (!std::binary_search(sv.begin(), sv.end(), nameStr))
				{
					sv.push_back(nameStr);
					std::sort(sv.begin(), sv.end());
				}
			}
		}
	}

	//----------------------------------------------------------------------

	void renameCompatibilities(std::string const & compatToRename, std::string const & compatNewName, bool notify, StringVector & svChassisSlotsResult, StringVector & svComponentsResult)
	{		
		ConstCharCrcString const compatToRenameCrcString(compatToRename.c_str());

		ShipChassis::PersistentCrcStringVector const & chassisNames = ShipChassis::getShipChassisCrcVector();
		{
			for (ShipChassis::PersistentCrcStringVector::const_iterator it = chassisNames.begin(); it != chassisNames.end(); ++it)
			{
				PersistentCrcString const * const pcs = NON_NULL(*it);
				
				ShipChassis const * const shipChassis = NON_NULL(ShipChassis::findShipChassisByCrc(pcs->getCrc()));
				ShipChassisWritable * const shipChassisWritable = const_cast<ShipChassisWritable *>(safe_cast<ShipChassisWritable const *>(shipChassis));
				NOT_NULL(shipChassisWritable);
				
				ShipChassis::SlotVector const & shipSlots = shipChassisWritable->getSlots();
				
				for (ShipChassis::SlotVector::const_iterator sit = shipSlots.begin(); sit != shipSlots.end(); ++sit)
				{
					ShipChassisSlot & chassisSlot = const_cast<ShipChassisSlot &>(*sit);
					
					if (chassisSlot.canAcceptCompatibility(compatToRenameCrcString))
					{
						if (notify)
						{
							svChassisSlotsResult.push_back(std::string(shipChassis->getName().getString()) + " " + ShipChassisSlotType::getNameFromType(chassisSlot.getSlotType()));
						}
						else
						{							
							chassisSlot.removeCompatibility(compatToRename);
							if (!compatNewName.empty())
								chassisSlot.addCompatibility(compatNewName);
						}
					}
				}
			}
		}

		ShipComponentDescriptor::NameComponentMap const & nameComponentMap = ShipComponentDescriptor::getNameComponentMap();
		{
			for (ShipComponentDescriptor::NameComponentMap::const_iterator it = nameComponentMap.begin(); it != nameComponentMap.end(); ++it)
			{
				ShipComponentDescriptor const * const shipComponentDescriptor = NON_NULL((*it).second);
				ShipComponentDescriptorWritable * const shipComponentDescriptorWritable = const_cast<ShipComponentDescriptorWritable *>(safe_cast<ShipComponentDescriptorWritable const *>(shipComponentDescriptor));
				std::string const & nameStr = shipComponentDescriptor->getCompatibility().getString();

				if (nameStr == compatToRename)
				{
					if (notify)
					{
						svComponentsResult.push_back(std::string(shipComponentDescriptorWritable->getName().getString()));
					}
					else
					{
						shipComponentDescriptorWritable->setCompatibility(compatNewName);
					}
				}
			}
		}
	}
	
	//----------------------------------------------------------------------
	
	void getSlotCompatibilities(ShipChassisSlot const & chassisSlot, StringVector & sv)
	{
		ShipChassisSlot::CompatibilityVector const & cv = chassisSlot.getCompatibilities();
		
		{
			sv.reserve(cv.size());
			for (ShipChassisSlot::CompatibilityVector::const_iterator it = cv.begin(); it != cv.end(); ++it)
			{
				CrcString const * const crcString = NON_NULL(*it);
				
				sv.push_back(std::string(crcString->getString()));
			}
		}
	}

	//----------------------------------------------------------------------
	
	void getComponentCompatibilities(ShipComponentDescriptor const & shipComponentDescriptor, StringVector & sv)
	{
		std::string const compat = shipComponentDescriptor.getCompatibility().getString();
		if (!compat.empty())
			sv.push_back(compat);
	}

	//----------------------------------------------------------------------


}

using namespace CompatibilityEditorNamespace;

//----------------------------------------------------------------------

CompatibilityEditor::CompatibilityEditor(QWidget *parent, char const *name, ShipChassisSlot const & chassisSlot, ShipComponentDescriptor const * shipComponentDescriptor) :
BaseCompatibilityEditor(parent, name, true),
m_mode(M_multi),
m_finalSelection(),
m_ok(false)
{
	StringVector svCompats;
	getSlotCompatibilities(chassisSlot, svCompats);

	StringVector svCompatsDescriptor;
	if (NULL != shipComponentDescriptor)
		getComponentCompatibilities(*shipComponentDescriptor, svCompatsDescriptor);
	
	setupSignals();
	populateLists(svCompats, svCompatsDescriptor);
}

//----------------------------------------------------------------------

CompatibilityEditor::CompatibilityEditor(QWidget *parent, char const *name, ShipComponentDescriptor const & shipComponentDescriptor, ShipChassisSlot const * chassisSlot) :
BaseCompatibilityEditor(parent, name, true),
m_mode(M_single),
m_finalSelection(),
m_ok(false)
{
	StringVector svCompats;
	getComponentCompatibilities(shipComponentDescriptor, svCompats);
	
	StringVector svCompatsChassis;
	if (NULL != chassisSlot)
		getSlotCompatibilities(*chassisSlot, svCompatsChassis);
	
	setupSignals();
	populateLists(svCompats, svCompatsChassis);
}

//----------------------------------------------------------------------

CompatibilityEditor::CompatibilityEditor(QWidget *parent, char const *name, Mode mode, StringVector const & initalSelection) :
BaseCompatibilityEditor(parent, name, true),
m_mode(mode),
m_finalSelection(),
m_ok(false)
{
	DEBUG_FATAL(M_single == m_mode && initalSelection.size() > 1, ("CompatibilityEditor invalid selection"));

	setupSignals();

	populateLists(initalSelection, StringVector());
}

//----------------------------------------------------------------------

void CompatibilityEditor::setupSignals()
{
	connect(m_listCompatsAvailable, SIGNAL(doubleClicked(QListBoxItem *)), SLOT(onPushButtonSelectClicked()));
	connect(m_listCompatsChosen, SIGNAL(doubleClicked(QListBoxItem *)), SLOT(onPushButtonDeselectClicked()));
	connect(m_buttonDeselect, SIGNAL(clicked()), SLOT(onPushButtonDeselectClicked()));
	connect(m_buttonSelect, SIGNAL(clicked()), SLOT(onPushButtonSelectClicked()));
	connect(m_buttonTypeNew, SIGNAL(clicked()), SLOT(onPushButtonTypeNewClicked()));
	connect(m_buttonTypeRename, SIGNAL(clicked()), SLOT(onPushButtonTypeRenameClicked()));
	connect(m_buttonTypeDelete, SIGNAL(clicked()), SLOT(onPushButtonTypeDeleteClicked()));
	connect(m_buttonOk, SIGNAL(clicked()), SLOT(onPushButtonOkClicked()));
	connect(m_lineEditNew, SIGNAL(textChanged(const QString &)), SLOT(onLineEditNewTextChanged(const QString &)));
	connect(m_lineEditRename, SIGNAL(textChanged(const QString &)), SLOT(onLineEditRenameTextChanged(const QString &)));

	connect(m_listCompatsAvailable, SIGNAL(selectionChanged()), SLOT(onListCompatAvailableSelectionChanged()));
	connect(m_listCompatsChosen, SIGNAL(selectionChanged()), SLOT(onListCompatChosenSelectionChanged()));

	m_buttonTypeNew->setEnabled(false);
	m_buttonTypeRename->setEnabled(false);
	m_buttonTypeDelete->setEnabled(false);
}

//----------------------------------------------------------------------

CompatibilityEditor::~CompatibilityEditor()
{
	
}

//----------------------------------------------------------------------

void CompatibilityEditor::populateLists(StringVector const & initalSelection, StringVector const & appropriateAvailableCompats)
{	
	StringVector sv;
	getAvailableCompatibilities(sv);
	
	m_listCompatsAvailable->clear();
	m_listCompatsChosen->clear();
	
	{
		for (StringVector::const_iterator it = initalSelection.begin(); it != initalSelection.end(); ++it)
		{
			std::string const & sel = *it;
			StringVector::iterator const existing = std::find(sv.begin(), sv.end(), sel);

			if (sv.end() == existing)
			{
				WARNING_STRICT_FATAL(true, ("CompatibilityEditor specified invalid compat type [%s]", sel.c_str()));
				return;
			}

			m_listCompatsChosen->insertItem(QString(sel.c_str()));
			sv.erase(existing);
		}
	}

	m_listCompatsChosen->sort();

	for (StringVector::const_iterator it = sv.begin(); it != sv.end(); ++it)
	{
		std::string const & compat = *it;

		if (std::binary_search(appropriateAvailableCompats.begin(), appropriateAvailableCompats.end(), compat))
		{
			//@todo: somehow make this highlight colored
			m_listCompatsAvailable->insertItem(QString(compat.c_str()));
		}
		else
			m_listCompatsAvailable->insertItem(QString(compat.c_str()));
	}
}

//----------------------------------------------------------------------

void CompatibilityEditor::handleSelectionAdd()
{
	QString const text = m_listCompatsAvailable->currentText();
	if (text.isEmpty())
		return;

	if (M_single == m_mode)
	{
		m_listCompatsAvailable->removeItem(m_listCompatsAvailable->currentItem());
		for (int i = 0; i < static_cast<int>(m_listCompatsChosen->count()); ++i)
		{
			QString const & str = m_listCompatsChosen->text(i);
			m_listCompatsAvailable->insertItem(str);
		}
		m_listCompatsChosen->clear();
		m_listCompatsChosen->insertItem(text);
		m_listCompatsAvailable->sort();
		return;
	}

	m_listCompatsChosen->insertItem(text);
	m_listCompatsChosen->sort();
	m_listCompatsAvailable->removeItem(m_listCompatsAvailable->currentItem());
}

//----------------------------------------------------------------------

void CompatibilityEditor::handleSelectionRemove()
{
	QString const & text = m_listCompatsChosen->currentText();
	if (text.isEmpty())
		return;

	m_listCompatsAvailable->insertItem(text);
	m_listCompatsAvailable->sort();
	m_listCompatsChosen->removeItem(m_listCompatsChosen->currentItem());
}

//----------------------------------------------------------------------

void CompatibilityEditor::handleRenameCompatibiliy(std::string const & oldName, std::string const & newName)
{	
	if (oldName.empty() || oldName == newName)
		return;

	StringVector svChassisSlots;
	StringVector svComponents;

	renameCompatibilities(oldName, newName, true, svChassisSlots, svComponents);
	
	if (!svChassisSlots.empty() || !svComponents.empty())
	{
		CompatibilityConfirmDialog * const ccd = new CompatibilityConfirmDialog(this, "confirm", oldName, newName, svChassisSlots, svComponents);

		bool const retval = ccd->showAndTell();

		if (!retval)
			return;
	}
	
	renameCompatibilities(oldName, newName, false, svChassisSlots, svComponents);
	
	QListBoxItem * const item = m_listCompatsAvailable->findItem(QString(oldName.c_str()));
	
	if (NULL != item)
	{
		delete item;

		//-- renaming, must replace
		if (!newName.empty())
		{
			m_listCompatsAvailable->insertItem(QString(newName.c_str()));
			m_listCompatsAvailable->sort();
		}
	}
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void CompatibilityEditor::onPushButtonTypeNewClicked()
{
	QString const & text = m_lineEditNew->text();
	if (!text.isEmpty())
	{
		if (NULL != m_listCompatsAvailable->findItem(text) && NULL != m_listCompatsChosen->findItem(text))
		{
			QMessageBox::warning(this, QString("Invalid compatibility name"), QString("That compatibility already exists."));
			return;
		}

		m_listCompatsAvailable->insertItem(text);
		m_listCompatsAvailable->sort();
	}
}

//----------------------------------------------------------------------

void CompatibilityEditor::onPushButtonTypeDeleteClicked()
{
	QString const & oldText = m_listCompatsAvailable->currentText();

	if (oldText.isEmpty())
		return;

	std::string const & oldNameStr = QStringUtil::toString(oldText);

	handleRenameCompatibiliy(oldNameStr, std::string());
}

//----------------------------------------------------------------------

void CompatibilityEditor::onPushButtonTypeRenameClicked()
{
	QString const & oldText = m_listCompatsAvailable->currentText();

	if (oldText.isEmpty())
		return;

	QString const & text = m_lineEditRename->text();

	if (oldText == text)
		return;

	std::string const & oldNameStr = QStringUtil::toString(oldText);
	std::string const & nameStr = QStringUtil::toString(text);

	handleRenameCompatibiliy(oldNameStr, nameStr);
}

//----------------------------------------------------------------------

void CompatibilityEditor::onPushButtonOkClicked()
{
	m_ok = true;
	
	for (int i = 0; i < static_cast<int>(m_listCompatsChosen->count()); ++i)
	{
		QString const & str = m_listCompatsChosen->text(i);
		m_finalSelection += QStringUtil::toString(str) + " ";
	}

	Unicode::trim(m_finalSelection);

	hide();
}

//----------------------------------------------------------------------

void CompatibilityEditor::onPushButtonSelectClicked()
{
	handleSelectionAdd();
}

//----------------------------------------------------------------------

void CompatibilityEditor::onPushButtonDeselectClicked()
{
	handleSelectionRemove();
}

//----------------------------------------------------------------------

void CompatibilityEditor::onLineEditNewTextChanged(const QString & text)
{
	if (text.isEmpty())
	{
		m_buttonTypeNew->setEnabled(false);
		return;
	}
	
	m_buttonTypeNew->setEnabled(m_listCompatsAvailable->currentItem() >= 0);
}

//----------------------------------------------------------------------

void CompatibilityEditor::onLineEditRenameTextChanged(const QString & text)
{
	if (text.isEmpty())
	{
		m_buttonTypeRename->setEnabled(false);
		return;
	}

	m_buttonTypeRename->setEnabled(m_listCompatsAvailable->currentItem() >= 0);
}

//----------------------------------------------------------------------

void CompatibilityEditor::onListCompatAvailableSelectionChanged()
{
	onLineEditNewTextChanged(m_lineEditNew->text());
	onLineEditRenameTextChanged(m_lineEditRename->text());
	m_buttonTypeDelete->setEnabled(m_listCompatsAvailable->currentItem() >= 0);

}

//----------------------------------------------------------------------

void CompatibilityEditor::onListCompatChosenSelectionChanged()
{
}

//----------------------------------------------------------------------

bool CompatibilityEditor::showAndTell(std::string & finalSelection)
{
	exec();
	if (m_ok)
	{
		finalSelection = m_finalSelection;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

//======================================================================
