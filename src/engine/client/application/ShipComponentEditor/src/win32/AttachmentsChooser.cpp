//======================================================================
//
// AttachmentsChooser.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "AttachmentsChooser.h"
#include "AttachmentsChooser.moc"

#include "CompatibilityEditor.h"
#include "ConfigShipComponentEditor.h"
#include "QStringUtil.h"
#include "ShipComponentEditorServerTemplate.h"
#include "ShipComponentEditorServerTemplateManager.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientDataFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipChassisWritable.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentDescriptorWritable.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Hardpoint.h"
#include "sharedObject/ObjectTemplateList.h"
#include <qsplitter.h>
#include <vector>

//======================================================================

namespace AttachmentsChooserNamespace
{	
	//-- the compiler wants this to be fwd-declared
	void getStringListHardpointsForShip(SharedObjectTemplate const & sot, std::string const & hardpointName, int * currentItem, QStringList & result);

	//----------------------------------------------------------------------

	bool getStringListHardpointsForShipByName(std::string const & otName, std::string const & hardpointName, int * currentItem, QStringList & result)
	{

		ObjectTemplate const * const ot = ObjectTemplateList::fetch(otName.c_str());
		if (NULL == ot)
			return false;

		SharedObjectTemplate const * const sot = dynamic_cast<SharedObjectTemplate const *>(ot);
		if (NULL == sot)
		{
			ot->releaseReference();
			return false;
		}

		getStringListHardpointsForShip(*sot, hardpointName, currentItem, result);
		sot->releaseReference();
		return true;
	}
	
	//----------------------------------------------------------------------

	bool getStringListHardpointsForShipAppearance(std::string const & appearanceFilename, QStringList & result)
	{
		AppearanceTemplate const * const at = AppearanceTemplateList::fetch(appearanceFilename.c_str());
		if (NULL == at)
			return false;
		
		int const hardpointCount = at->getHardpointCount();
		{
			for (int i = 0; i < hardpointCount; ++i)
			{
				Hardpoint const & hardpoint = at->getHardpoint(i);
				result.push_back(hardpoint.getName().getString());
			}
		}
		AppearanceTemplateList::release(at);
		return true;
	}
	
	//----------------------------------------------------------------------

	void getStringListHardpointsForShip(SharedObjectTemplate const & sot, std::string const & hardpointName, int * currentItem, QStringList & result)
	{
		std::string const & appearanceFilename = sot.getAppearanceFilename();
		IGNORE_RETURN(getStringListHardpointsForShipAppearance(appearanceFilename, result));

		//-- make sure we include our client data file children as well
		ClientDataFile const * const cdf = dynamic_cast<ClientDataFile const*> (sot.getClientData());
		if (NULL != cdf)
		{
			ClientDataFile::StringVector sv;
			cdf->getAllChildObjects(sv);

			{
				for (ClientDataFile::StringVector::const_iterator it = sv.begin(); it != sv.end(); ++it)
				{
					std::string const & childObjectTemplateName = *it;
					//-- ignore the value of current item, we will sort the list and recalc it below
					IGNORE_RETURN(getStringListHardpointsForShipByName(childObjectTemplateName, hardpointName, NULL, result));
				}
			}

			sv.clear();
			cdf->getAllChildAppearances(sv);

			{
				for (ClientDataFile::StringVector::const_iterator it = sv.begin(); it != sv.end(); ++it)
				{
					std::string const & childAppearanceTemplateName = *it;
					//-- ignore the value of current item, we will sort the list and recalc it below
					IGNORE_RETURN(getStringListHardpointsForShipAppearance(childAppearanceTemplateName, result));
				}
			}
		}
		
		result.sort();
		
		if (NULL != currentItem)
		{
			*currentItem = result.findIndex(QString(hardpointName.c_str()));
			if (*currentItem < 0)
			{
				result.push_front(QString((std::string("INVALID: ") + hardpointName).c_str()));
				*currentItem = 0;
			}
		}
	}
	
	//----------------------------------------------------------------------
	
	class MyColorTextItem : public QTableItem
	{
	public:
		void paint(QPainter* p,const QColorGroup & cg,const QRect & cr,bool selected)
		{
			QColorGroup g (cg);
			g.setColor(QColorGroup::Foreground, m_colorName);
			g.setColor(QColorGroup::Text, m_colorName);
			
			QTableItem::paint( p , g , cr , selected );
		}
		
		MyColorTextItem(QTable * const _table, EditType et, QString const & _text, QString const & colorName) : 
		QTableItem(_table, et, _text),
			m_colorName(colorName)
		{
		}

	private:

		MyColorTextItem();

	private:

		QString m_colorName;
	};
	
	//----------------------------------------------------------------------

	enum TableCols
	{
		TC_template,
		TC_hardpoint,
		TC_clientDataFile
	};

	//----------------------------------------------------------------------

	enum ContextMenuItems
	{
		CMI_templateSelect,
		CMI_templateEdit,
		CMI_cdfEdit
	};

	char const * const s_contextMenuItemNames[] =
	{
		"Select Template",
		"Edit Template",
		"Edit CDF"
	};
	
	//----------------------------------------------------------------------
}

//lint -esym(641, AttachmentsChooserNamespace::TableCols)
//lint -esym(641, AttachmentsChooserNamespace::ContextMenuItems)

using namespace AttachmentsChooserNamespace;

//----------------------------------------------------------------------
//-- AttachmentsChooser::State
//----------------------------------------------------------------------

class AttachmentsChooser::State
{
public:

	State() :
	thpv(),
	modified(false)
	{
	}
	ShipComponentAttachmentManager::TemplateHardpointPairVector thpv; //lint !e1925 // public data member
	bool modified; //lint !e1925 // public data member
};

//----------------------------------------------------------------------

AttachmentsChooser::AttachmentsChooser(QWidget *_parent, char const *_name) :
BaseAttachmentsChooser(_parent, _name),
m_chassisCrc(0),
m_componentCrc(0),
m_chassisSlotType(0),
m_state(new State),
m_tableNeedsReset(true),
m_tableResetCountdown(0),
m_callback(new MessageDispatch::Callback)
{
	IGNORE_RETURN(connect(m_buttonApply, SIGNAL(clicked()), SLOT(onButtonApplyClicked())));
	IGNORE_RETURN(connect(m_buttonRevert, SIGNAL(clicked()), SLOT(onButtonRevertClicked())));
	IGNORE_RETURN(connect(m_buttonAddAttachment, SIGNAL(clicked()), SLOT(onButtonAddClicked())));
	IGNORE_RETURN(connect(m_buttonDeleteAttachment, SIGNAL(clicked()), SLOT(onButtonDeleteClicked())));

	IGNORE_RETURN(connect(m_table, SIGNAL(selectionChanged()), SLOT(onTableSelectionChanged())));
	IGNORE_RETURN(connect(m_table, SIGNAL(doubleClicked(int, int, int, const QPoint &)), SLOT(onTableDoubleClicked(int, int, int, const QPoint &))));
	IGNORE_RETURN(connect(m_table, SIGNAL(valueChanged(int, int)), SLOT(onTableValueChanged(int, int))));
	IGNORE_RETURN(connect(m_table, SIGNAL(contextMenuRequested(int, int, const QPoint &)), SLOT(onTableContextMenuRequested(int, int, const QPoint &))));
	
	IGNORE_RETURN(connect(m_buttonSlotAdd, SIGNAL(clicked()), SLOT(onButtonSlotAddClicked())));
	IGNORE_RETURN(connect(m_buttonSlotCompat, SIGNAL(clicked()), SLOT(onButtonSlotCompatClicked())));
	IGNORE_RETURN(connect(m_buttonComponentCompat, SIGNAL(clicked()), SLOT(onButtonComponentCompatClicked()))); 

	QTimer *timer = new QTimer(this);
	IGNORE_RETURN(connect(timer, SIGNAL(timeout()), SLOT(onTimerUpdate())));
	IGNORE_RETURN(timer->start(100));

	m_callback->connect(*this, &AttachmentsChooser::onCallbackComponentListChanged, static_cast<ShipComponentDescriptorWritable::Messages::ComponentListChanged *>(NULL));
	m_callback->connect(*this, &AttachmentsChooser::onCallbackComponentChanged, static_cast<ShipComponentDescriptorWritable::Messages::ComponentChanged *>(NULL));
	m_callback->connect(*this, &AttachmentsChooser::onCallbackAttachmentsChanged, static_cast<ShipComponentAttachmentManager::Messages::AttachmentsChanged *>(NULL));

	m_callback->connect(*this, &AttachmentsChooser::onCallbackChassisListChanged, static_cast<ShipChassisWritable::Messages::ChassisListChanged *>(NULL));
	m_callback->connect(*this, &AttachmentsChooser::onCallbackChassisChanged, static_cast<ShipChassisWritable::Messages::ChassisChanged *>(NULL));	

	m_callback->connect(*this, &AttachmentsChooser::onCallbackTemplateListChanged, static_cast<ShipComponentEditorServerTemplateManager::Messages::TemplateListChanged *>(NULL));	
} //lint !e429 //custodial timer

//----------------------------------------------------------------------

AttachmentsChooser::~AttachmentsChooser()
{
	delete m_state;
	m_state = NULL;

	delete m_callback;
	m_callback = NULL;
}
 
//----------------------------------------------------------------------

void AttachmentsChooser::onTimerUpdate()
{
	if(m_tableNeedsReset)
	{
		m_tableResetCountdown -= 100;
		if (m_tableResetCountdown <= 0)
			resetTable();
	}
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void AttachmentsChooser::onButtonApplyClicked()
{
	ShipComponentAttachmentManager::setAttachmentsForShip(m_chassisCrc, m_componentCrc, m_chassisSlotType, m_state->thpv);
	setModified(false);
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChooser::onButtonRevertClicked()
{
	setEditingAttachments(m_chassisCrc, m_componentCrc, m_chassisSlotType, true);
}

//----------------------------------------------------------------------

void AttachmentsChooser::onButtonAddClicked()
{
	m_state->thpv.push_back(ShipComponentAttachmentManager::TemplateHardpointPair(0, PersistentCrcString()));
	setModified(true);
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChooser::onButtonDeleteClicked()
{
	int const row = getLastSelectedRow();

	if (row < 0 || row >= static_cast<int>(m_state->thpv.size()))
		return;

	IGNORE_RETURN(m_state->thpv.erase(m_state->thpv.begin() + row));

	setModified(true);
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChooser::onTableSelectionChanged()
{
	if (m_table->numSelections() <= 0)
		m_buttonDeleteAttachment->setEnabled(false);
	else
		m_buttonDeleteAttachment->setEnabled(true);
}

//----------------------------------------------------------------------

void AttachmentsChooser::onButtonSlotAddClicked()
{
	ShipChassisWritable * const shipChassisWritable = getShipChassisWritable();
	if (NULL == shipChassisWritable)
		return;

	ShipChassisSlot const chassisSlot(static_cast<ShipChassisSlotType::Type>(m_chassisSlotType), std::string(), 0, false);
	shipChassisWritable->addSlot(chassisSlot);
}

//----------------------------------------------------------------------

void AttachmentsChooser::onButtonSlotCompatClicked()
{
	ShipChassisWritable * const shipChassisWritable = getShipChassisWritable();
	if (NULL == shipChassisWritable)
		return;

	ShipChassisSlot * const chassisSlot = shipChassisWritable->getSlot(static_cast<ShipChassisSlotType::Type>(m_chassisSlotType));
	if (NULL == chassisSlot)
		return;
	
	ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc(m_componentCrc);

	CompatibilityEditor * const ce = new CompatibilityEditor(this, "compat_editor", *chassisSlot, shipComponentDescriptor);
	std::string finalSelection;
	if (ce->showAndTell(finalSelection))
	{
		chassisSlot->setCompatibilities(finalSelection);
		shipChassisWritable->notifyChanged();
	}

	delete ce;
}

//----------------------------------------------------------------------

void AttachmentsChooser::onButtonComponentCompatClicked()
{
	ShipChassisWritable * const shipChassisWritable = getShipChassisWritable();
	if (NULL == shipChassisWritable)
		return;

	ShipChassisSlot * const chassisSlot = shipChassisWritable->getSlot(static_cast<ShipChassisSlotType::Type>(m_chassisSlotType));
	if (NULL == chassisSlot)
		return;

	ShipComponentDescriptorWritable * const shipComponentDescriptorWritable = getShipComponentDescriptorWritable();

	if (NULL == shipComponentDescriptorWritable)
		return;
	
	CompatibilityEditor * const ce = new CompatibilityEditor(this, "compat_editor", *shipComponentDescriptorWritable, chassisSlot);
	std::string finalSelection;
	if (ce->showAndTell(finalSelection))
	{
		shipComponentDescriptorWritable->setCompatibility(finalSelection);
	}

	delete ce;
}

//----------------------------------------------------------------------

void AttachmentsChooser::onTableDoubleClicked(int row, int col, int, const QPoint &)
{
	if (row >= static_cast<int>(m_state->thpv.size()))
		return;
	
	switch (col)
	{
		//-- 
	case TC_template:
		{
//			handleSelectTemplateForRow(row);
		}
		break;
	case TC_clientDataFile:
		{
			handleEditCdfForRow(row);
		}
		break;
	default:
		break;
	}
} //lint !e1762 // not CONST!!

//----------------------------------------------------------------------

void AttachmentsChooser::onTableContextMenuRequested ( int, int, const QPoint & _pos )
{
	ShipChassisWritable * const shipChassisWritable = getShipChassisWritable();
	if (NULL == shipChassisWritable)
		return;

//	int const lastSelectedRow = getLastSelectedRow();
	int const lastSelectedCol = getLastSelectedCol();

	if (TC_hardpoint == lastSelectedCol)
	{
		return;
	}

	QPopupMenu * const pop = new QPopupMenu(m_table);

	if (TC_template == lastSelectedCol)
	{
		IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_templateSelect], this, SLOT(onTableContextTemplateSelect(int)), 0, CMI_templateSelect));
		IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_templateEdit], this, SLOT(onTableContextTemplateEdit(int)), 0, CMI_templateEdit));
	}
	else if (CMI_cdfEdit == lastSelectedCol)
	{
		IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_cdfEdit], this, SLOT(onTableContextCdfEdit(int)), 0, CMI_cdfEdit));
	}

	IGNORE_RETURN(pop->insertSeparator());
	
	pop->popup(_pos);
} //lint !e429 //custodial pop

//----------------------------------------------------------------------

void AttachmentsChooser::onTableContextTemplateSelect(int)
{
	int const lastSelectedRow = getLastSelectedRow();
	handleSelectTemplateForRow(lastSelectedRow);
}

//----------------------------------------------------------------------

void AttachmentsChooser::onTableContextTemplateEdit(int)
{
	int const lastSelectedRow = getLastSelectedRow();
	handleEditTemplateForRow(lastSelectedRow);
}  //lint !e1762 // not CONST!!

//----------------------------------------------------------------------

void AttachmentsChooser::onTableContextCdfEdit(int)
{
	int const lastSelectedRow = getLastSelectedRow();
	handleEditCdfForRow(lastSelectedRow);
}  //lint !e1762 // not CONST!!

//----------------------------------------------------------------------

void AttachmentsChooser::onTableValueChanged(int row, int col)
{
	if (row >= static_cast<int>(m_state->thpv.size()))
		return;
	
	ShipComponentAttachmentManager::TemplateHardpointPair & thp = m_state->thpv[static_cast<size_t>(row)];

	std::string const & valueStr = QStringUtil::toString(m_table->text(row, col));
	
	switch (col)
	{
	case TC_template:
		{
			CrcString const & crcString = ObjectTemplateList::lookUp(valueStr.c_str());
			if (0 == crcString.getCrc())
			{
				IGNORE_RETURN(QMessageBox::warning(this, QString("Invalid template"), QString((std::string("Invalid template:\n") + valueStr).c_str())));
				return;
			}
			thp.first = crcString.getCrc();
			setModified(true);
		}
		break;
	case TC_hardpoint:
		{
			thp.second.set(valueStr.c_str(), true);
			setModified(true);
		}
		break;
	default:
		break;
	}

	setTableNeedsReset();
}

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- CALLBACKS
//----------------------------------------------------------------------

void AttachmentsChooser::onCallbackComponentListChanged(ShipComponentDescriptorWritable const & payload)
{
	UNREF(payload);
	//-- if the component list changes, just dump the current edit
	setEditingAttachments(0, 0, 0, true);
}

//----------------------------------------------------------------------

void AttachmentsChooser::onCallbackComponentChanged(bool const & payload)
{
	UNREF(payload);
	//-- if the component list changes, just dump the current edit
	setEditingAttachments(0, 0, 0, true);
}

//----------------------------------------------------------------------

void AttachmentsChooser::onCallbackAttachmentsChanged(ShipComponentAttachmentManager::Messages::AttachmentsChanged::Payload const & payload)
{
	//-- if the attachments change from the outside, ignore it for now.
	//-- we should already be handling most cases by catching changes to chassis & component lists
	UNREF(payload);
} //lint !e1762 //can not be made const

//----------------------------------------------------------------------

void AttachmentsChooser::onCallbackChassisListChanged(bool const & payload)
{
	UNREF(payload);
	//-- if the chassis list changes, just dump the current edit
	setEditingAttachments(0, 0, 0, true);
} //lint !e1762 //can not be made const

//----------------------------------------------------------------------

void AttachmentsChooser::onCallbackChassisChanged(ShipChassisWritable::Messages::ChassisChanged::Payload const & payload)
{
	UNREF(payload);
	//-- if the chassis list changes, just dump the current edit
	setEditingAttachments(0, 0, 0, true);
} //lint !e1762 //can not be made const

//----------------------------------------------------------------------

void AttachmentsChooser::onCallbackTemplateListChanged(bool const & payload)
{
	UNREF(payload);
	setTableNeedsReset();
} //lint !e1762 //can not be made const

//----------------------------------------------------------------------
//-- END CALLBACKS
//----------------------------------------------------------------------

ShipChassisWritable * AttachmentsChooser::getShipChassisWritable()
{
	return const_cast<ShipChassisWritable *>(safe_cast<ShipChassisWritable const *>(ShipChassis::findShipChassisByCrc(m_chassisCrc)));
}

//----------------------------------------------------------------------

ShipComponentDescriptorWritable * AttachmentsChooser::getShipComponentDescriptorWritable()
{
	return const_cast<ShipComponentDescriptorWritable *>(safe_cast<ShipComponentDescriptorWritable const *>(ShipComponentDescriptor::findShipComponentDescriptorByCrc(m_componentCrc)));
}

//----------------------------------------------------------------------

void AttachmentsChooser::setEditingAttachments(uint32 chassisCrc, uint32 componentCrc, int chassisSlotType, bool confirmed)
{
	if (m_state->modified && !confirmed)
	{
		int const retval = QMessageBox::question(this, QString("Apply or Discard Changes?"), QString("Apply or Discard Attachment Changes?"), QString("Apply"), QString("Discard"), QString("Cancel"));
		if (retval < 0)
			return;

		switch (retval)
		{
		case 0:
			{
				//-- apply
				onButtonApplyClicked();
			}
			break;
		case 1:
			{
				//-- do nothing... discard changes
			}
			break;
		case 2:
			{
				//-- cancel
				return;
			}
			break; //lint !e527 // unreachable code			
		default:
			break;
		}
	}

	m_table->setNumRows(0);
	m_labelShip->setText("");
	m_labelSlot->setText("");
	m_labelComponent->setText("");

	m_buttonApply->setEnabled(false);
	m_buttonRevert->setEnabled(false);
	m_buttonSlotAdd->setEnabled(false);
	m_buttonSlotCompat->setEnabled(false);

	m_chassisCrc = chassisCrc;
	m_componentCrc = componentCrc;
	m_chassisSlotType = chassisSlotType;

	ShipChassisWritable * const shipChassisWritable = getShipChassisWritable();

	int effectiveCompatibilityType = chassisSlotType;

	// For capships weapons beyond 7 cycle back to the beginning
	if (chassisSlotType >= ShipChassisSlotType::SCST_num_explicit_types && chassisSlotType <= ShipChassisSlotType::SCST_weapon_last)
	{
		ShipComponentEditorServerTemplateManager::ServerTemplateVector result;
		ShipComponentEditorServerTemplateManager::findObjectTemplatesForChassisType(shipChassisWritable->getName().getString(), result);
		if (!result.empty())
		{
			ShipComponentEditorServerTemplate const & st = result.front();
			SharedObjectTemplate const * const temp = dynamic_cast<SharedObjectTemplate const *>(ObjectTemplateList::fetch(st.sharedTemplateName.c_str()));
			if (   temp
			    && (   temp->getGameObjectType() == SharedObjectTemplate::GOT_ship_capital
			        || temp->getGameObjectType() == SharedObjectTemplate::GOT_ship_station))
				effectiveCompatibilityType = ShipChassisSlotType::SCST_weapon_first+((chassisSlotType-ShipChassisSlotType::SCST_weapon_first)&7);
		}
	}

	if (NULL == shipChassisWritable)
	{
		setEnabled(false);
		return;
	}

	ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc(m_componentCrc);
	if (NULL == shipComponentDescriptor)
	{
		setEnabled(false);
		return;
	}

	if (chassisSlotType < 0 || chassisSlotType >= static_cast<int>(ShipChassisSlotType::SCST_num_types))
	{
		setEnabled(false);
		return;
	}

	int const componentTypeForSlot = ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
	if (componentTypeForSlot != shipComponentDescriptor->getComponentType())
	{
		setEnabled(false);
		return;
	}

	setEnabled(true);

	ShipChassisSlot const * const chassisSlot = shipChassisWritable->getSlot(static_cast<ShipChassisSlotType::Type>(effectiveCompatibilityType));
	if (NULL == chassisSlot || !chassisSlot->canAcceptComponent(*shipComponentDescriptor))
	{
		m_groupAttachments->setEnabled(false);

		if (NULL == chassisSlot)
		{
			m_groupAttachments->setTitle("SLOT DOES NOT EXIST -- NO ATTACHMENTS");
			m_buttonSlotAdd->setEnabled(true);
			m_buttonSlotCompat->setEnabled(false);
		}
		else
		{
			m_groupAttachments->setTitle("INCOMPATIBLE COMPONENT -- NO ATTACHMENTS");
			m_buttonSlotAdd->setEnabled(false);
			m_buttonSlotCompat->setEnabled(true);
		}

		return;
	}

	m_buttonSlotCompat->setEnabled(true);
	m_groupAttachments->setTitle("ATTACHMENTS");
	m_groupAttachments->setEnabled(true);

	m_labelShip->setText(shipChassisWritable->getName().getString());
	m_labelSlot->setText(shipComponentDescriptor->getName().getString());
	m_labelComponent->setText(ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlotType)).c_str());

	m_state->thpv = ShipComponentAttachmentManager::getAttachmentsForShip(m_chassisCrc, m_componentCrc, m_chassisSlotType);
	setModified(false);

	resetTable();
}

//----------------------------------------------------------------------

void AttachmentsChooser::resetTable()
{
	m_tableNeedsReset = false;

	ShipChassisWritable * const shipChassisWritable = getShipChassisWritable();

	if (NULL == shipChassisWritable)
	{
		m_table->setNumRows(0);
		return;
	}

	SharedObjectTemplate const * shipSot = NULL;

	{
		ShipComponentEditorServerTemplateManager::ServerTemplateVector result;
		ShipComponentEditorServerTemplateManager::findObjectTemplatesForChassisType(shipChassisWritable->getName().getString(), result);
		if (!result.empty())
		{
			ShipComponentEditorServerTemplate const & st = result.front();
			ObjectTemplate const * const shipOt = ObjectTemplateList::fetch(st.sharedTemplateName.c_str());
			shipSot = dynamic_cast<SharedObjectTemplate const *>(shipOt);
		}
	}

	ShipComponentAttachmentManager::TemplateHardpointPairVector const & thpv = m_state->thpv;

	{
		m_table->setNumRows(static_cast<int>(thpv.size()));

		QStringList qsl;
		int row = 0;
		for (ShipComponentAttachmentManager::TemplateHardpointPairVector::const_iterator it = thpv.begin(); it != thpv.end(); ++it, ++row)
		{
			ShipComponentAttachmentManager::TemplateHardpointPair const & thp = *it;

			uint32 const attachmentTemplateCrc = thp.first;

			ObjectTemplate const * const attachmentOt = ObjectTemplateList::fetch(attachmentTemplateCrc);
			SharedObjectTemplate const * const attachmentSot = dynamic_cast<SharedObjectTemplate const *>(attachmentOt);

			m_table->setItem(row, TC_template, new QTableItem(m_table, QTableItem::OnTyping, (NULL != attachmentSot) ? attachmentSot->getName() : ""));

			if (NULL != shipSot)
			{
				CrcString const & hardpointName = thp.second;
				int currentItem;
				qsl.clear();
				std::string const hardpointNameStr(hardpointName.getString());

				getStringListHardpointsForShip(*shipSot, hardpointNameStr, &currentItem, qsl);
				QComboTableItem * const item = new QComboTableItem(m_table, qsl);
				item->setCurrentItem(currentItem);
				m_table->setItem(row, TC_hardpoint, item);
			} //lint !e429 //custodial item
			else
				m_table->clearCell(row, TC_hardpoint);
	
			ClientDataFile const * const cdf = (NULL != attachmentSot) ? dynamic_cast<ClientDataFile const*> (attachmentSot->getClientData()) : NULL;
			if (NULL != cdf)
			{
				m_table->setItem(row, TC_clientDataFile, new QTableItem(m_table, QTableItem::Never, QString(cdf->getName())));
			}
			else
			{
				//-- try to display the invalid value

				std::string const & cdfName = (NULL != attachmentSot) ? attachmentSot->getClientDataFile() : "";
				if (cdfName.empty())
					m_table->clearCell(row, TC_clientDataFile);
				else
					m_table->setItem(row, TC_clientDataFile, new MyColorTextItem(m_table, QTableItem::Never, QString(cdfName.c_str()), "red"));
			}

			if (NULL != attachmentSot)
			{
				attachmentSot->releaseReference();
			}
		}

		
		if (!thpv.empty())
		{
			m_table->adjustColumn(0);
			m_table->adjustColumn(1);
			m_table->adjustColumn(2);
		}
	}

	if (NULL != shipSot)
	{
		shipSot->releaseReference();
	}

	onTableSelectionChanged();
}

//----------------------------------------------------------------------

void AttachmentsChooser::updateButtonsOnModificationChanged()
{
	m_buttonApply->setEnabled(m_state->modified);
	m_buttonRevert->setEnabled(m_state->modified);
}

//----------------------------------------------------------------------

void AttachmentsChooser::setModified(bool modified)
{
	m_state->modified = modified;
	updateButtonsOnModificationChanged();
}

//----------------------------------------------------------------------

void AttachmentsChooser::setTableNeedsReset()
{
	m_tableNeedsReset = true;
	m_tableResetCountdown = 0;
}

//----------------------------------------------------------------------

void AttachmentsChooser::handleSelectTemplateForRow(int row)
{
	if (row < 0 || row >= static_cast<int>(m_state->thpv.size()))
		return;
	
	ShipComponentAttachmentManager::TemplateHardpointPair & thp = m_state->thpv[static_cast<size_t>(row)];
	
	uint32 const attachmentTemplateCrc = thp.first;
	ObjectTemplate const * const attachmentOt = ObjectTemplateList::fetch(attachmentTemplateCrc);
	SharedObjectTemplate const * const attachmentSot = dynamic_cast<SharedObjectTemplate const *>(attachmentOt);
	
	std::string initialPath = ConfigShipComponentEditor::getSharedPathData();
	
	if (NULL != attachmentSot)
		initialPath += std::string("/") + attachmentSot->getName();
	else
	{
		ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc(m_componentCrc);
		if (NULL == shipComponentDescriptor)
			return;

		initialPath += "/object/tangible/ship/attachment/" + ShipComponentType::getNameFromType(shipComponentDescriptor->getComponentType());
	}
	
	QString const & fileSelection = QFileDialog::getOpenFileName(QString(initialPath.c_str()), QString("*.iff"), this);
	
	if (fileSelection.isEmpty())
		return;
	
	std::string fileSelectionStr = QStringUtil::toString(fileSelection);
	
	if (!TreeFile::stripTreeFileSearchPathFromFile(fileSelectionStr, fileSelectionStr))
	{
		WARNING(true, ("AttachmentsChooser invalid template path"));
		return;
	}
	
	CrcString const & crcString = ObjectTemplateList::lookUp(fileSelectionStr.c_str());
	if (0 == crcString.getCrc())
	{
		WARNING(true, ("AttachmentsChooser invalid template selected."));
		return;
	}
	
	thp.first = crcString.getCrc();
	setModified(true);
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChooser::handleEditTemplateForRow(int row) const
{
	std::string templateName = QStringUtil::toString(m_table->text(row, TC_template));
	if (!templateName.empty() && templateName.size() > 4)
	{
		if (templateName.find(".iff") != (templateName.size() - 4))
		{
			WARNING(true, ("AttachmentsChooser malformed template name [%s]", templateName.c_str()));
			return;
		}
		
		IGNORE_RETURN(templateName.replace(templateName.size() - 4, 4, ".tpf"));
		std::string const & templatePath = ConfigShipComponentEditor::getSharedPathDsrc() + "/" + templateName;
		char buf[1024];
		size_t const buf_size = sizeof(buf);
		IGNORE_RETURN(snprintf(buf, buf_size, "start TextPad %s", templatePath.c_str()));
		IGNORE_RETURN(system(buf));
	}
}

//----------------------------------------------------------------------

void AttachmentsChooser::handleEditCdfForRow(int row) const
{
	std::string cdfName = QStringUtil::toString(m_table->text(row, TC_clientDataFile));
	if (!cdfName.empty() && cdfName.size() > 4)
	{
		if (cdfName.find(".cdf") != (cdfName.size() - 4))
		{
			WARNING(true, ("AttachmentsChooser malformed CDF name [%s]", cdfName.c_str()));
			return;
		}
		
		IGNORE_RETURN(cdfName.replace(cdfName.size() - 4, 4, ".mif"));
		std::string const & cdfPath = ConfigShipComponentEditor::getClientDataFilePathDsrc() + "/" + cdfName;
		char buf[1024];
		size_t const buf_size = sizeof(buf);
		IGNORE_RETURN(snprintf(buf, buf_size, "start TextPad %s", cdfPath.c_str()));
		IGNORE_RETURN(system(buf));
	}
}

//----------------------------------------------------------------------

int AttachmentsChooser::getLastSelectedRow() const
{
	if (m_table->numSelections() <= 0)
		return -1;

	QTableSelection const & selection = m_table->selection(0);
	return selection.anchorRow();
}

//----------------------------------------------------------------------

int AttachmentsChooser::getLastSelectedCol() const
{
	if (m_table->numSelections() <= 0)
		return -1;

	QTableSelection const & selection = m_table->selection(0);
	return selection.anchorCol();

}

//======================================================================
