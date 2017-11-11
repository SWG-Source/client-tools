//======================================================================
//
// AttachmentsChassisEditorList.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "AttachmentsChassisEditorList.h"
#include "AttachmentsChassisEditorList.moc"

#include "ChassisNewDialog.h"
#include "CompatibilityEditor.h"
#include "QStringUtil.h"
#include "ShipComponentEditorServerTemplate.h"
#include "ShipComponentEditorServerTemplateManager.h"
#include "TemplateNewDialog.h"
#include "UnicodeUtils.h"
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
#include "sharedObject/ObjectTemplateList.h"
#include <vector>

//======================================================================

namespace AttachmentsChassisEditorListNamespace
{
	//----------------------------------------------------------------------

	class MyAttachmentTableItem : public QTableItem
	{
	public:
		void paint(QPainter* p,const QColorGroup & cg,const QRect & cr,bool selected)
		{
			QColorGroup g (cg);
			
			QColor const & normalBase = g.base();
			QColor const yellow(255, 200, 0); //lint !e578 //hides Qt:: member
			QColor const myBaseYellow((normalBase.red() * 3 + yellow.red()) / 4, (normalBase.green() * 3 + yellow.green()) / 4, (normalBase.blue() * 3 + yellow.blue()) / 4);
			QColor const green(0, 255, 0); //lint !e578 //hides Qt:: member
			QColor const myBaseGreen((normalBase.red() * 3 + green.red()) / 4, (normalBase.green() * 3 + green.green()) / 4, (normalBase.blue() * 3 + green.blue()) / 4);

			QColor const blue(0, 100, 255); //lint !e578 //hides Qt:: member
			QColor const myBaseBlue((normalBase.red() * 3 + blue.red()) / 4, (normalBase.green() * 3 + blue.green()) / 4, (normalBase.blue() * 3 + blue.blue()) / 4);

			if (0 != m_count)
			{
				if (m_valid)
				{
					g.setColor(QColorGroup::Background, myBaseGreen);
					g.setColor(QColorGroup::Base, myBaseGreen);

				}
				else
				{
					g.setColor(QColorGroup::Background, myBaseYellow);
					g.setColor(QColorGroup::Base, myBaseYellow);
				}
			}
			else if (!m_compatible)
			{
				g.setColor(QColorGroup::Background, myBaseBlue);
				g.setColor(QColorGroup::Base, myBaseBlue);
			}
			
			QTableItem::paint( p , g , cr , selected );
		}
		
		MyAttachmentTableItem (QTable * const table, QString const & text, int count, bool valid, bool compatible) :
			QTableItem(table, QTableItem::Never, text),
			m_count(count),
			m_valid(valid),
			m_compatible(compatible)
		{
		}
	private:

		MyAttachmentTableItem();

	private:
		int m_count;
		bool m_valid;
		bool m_compatible;
	};

	//----------------------------------------------------------------------

	class MyDisabledTableItem : public QTableItem
	{
	public:
		void paint(QPainter* p,const QColorGroup & cg,const QRect & cr,bool selected)
		{
			QColorGroup g (cg);
			if (m_possible)
			{
				g.setColor(QColorGroup::Background , g.midlight());
				g.setColor(QColorGroup::Base , g.midlight());
			}
			else
			{
				g.setColor(QColorGroup::Background , g.dark());
				g.setColor(QColorGroup::Base , g.dark());
			}
			
			QTableItem::paint( p , g , cr , selected );
		}
		
		MyDisabledTableItem(QTable * const table, EditType et, QString const & text, bool possible) : 
		QTableItem(table, et, text),
			m_possible(possible)
		{
		}

		bool m_possible; //lint !e1925 //public

	private:
		MyDisabledTableItem();
	};

	//----------------------------------------------------------------------

	enum ContextMenuItems
	{
		CMI_compatibilityComponentAdd,
		CMI_compatibilityChassisAdd,
		CMI_compatibilityBothAdd,
		CMI_compatibilityComponentRemove,
		CMI_compatibilityChassisRemove,
		CMI_attachmentsDelete
	};

	char const * const s_contextMenuItemNames[] =
	{
		"Make Compatibile Via Component",
		"Make Compatibile Via Chassis",
		"Make Compatibile Via Both",
		"Make Incompatibile Via Component",
		"Make Incompatibile Via Chassis",
		"Delete Attachments"
	};
}

using namespace AttachmentsChassisEditorListNamespace;

//lint -esym(641, AttachmentsChassisEditorListNamespace::ContextMenuItems)

//----------------------------------------------------------------------

AttachmentsChassisEditorList::AttachmentsChassisEditorList(QWidget *_parent, char const *_name) :
BaseAttachmentsEditorList(_parent, _name),
m_tableNeedsReset(true),
m_tableResetCountdown(0),
m_nameFilter(),
m_callback(new MessageDispatch::Callback),
m_filterComponentTypes(),
m_chassisName(),
m_hideEmptyRows(false),
m_hideEmptyCols(true),
m_hideUninstallableRows(false),
m_hideUninstallableCols(true)
{	
	IGNORE_RETURN(connect(m_table, SIGNAL(doubleClicked(int, int, int, const QPoint & )), SLOT(onTableDoubleClicked(int, int, int, const QPoint &))));
	IGNORE_RETURN(connect(m_table, SIGNAL(valueChanged(int, int)), SLOT(onTableValueChanged(int, int)))); 
	IGNORE_RETURN(connect(m_table, SIGNAL(contextMenuRequested ( int , int , const QPoint &  )), SLOT(onTableContextMenuRequested ( int , int , const QPoint &  ))));
	IGNORE_RETURN(connect(m_table, SIGNAL(selectionChanged ()), SLOT(onTableSelectionChanged ())));
	
	resetTable();

	QTimer *timer = new QTimer(this);
	IGNORE_RETURN(connect(timer, SIGNAL(timeout()), SLOT(onTimerUpdate())));
	IGNORE_RETURN(timer->start(100));

	m_callback->connect(*this, &AttachmentsChassisEditorList::onCallbackComponentListChanged, static_cast<ShipComponentDescriptorWritable::Messages::ComponentListChanged *>(NULL));
	m_callback->connect(*this, &AttachmentsChassisEditorList::onCallbackComponentChanged, static_cast<ShipComponentDescriptorWritable::Messages::ComponentChanged *>(NULL));

	m_callback->connect(*this, &AttachmentsChassisEditorList::onCallbackAttachmentsChanged, static_cast<ShipComponentAttachmentManager::Messages::AttachmentsChanged *>(NULL));
	m_callback->connect(*this, &AttachmentsChassisEditorList::onCallbackChassisListChanged, static_cast<ShipChassisWritable::Messages::ChassisListChanged *>(NULL));
	m_callback->connect(*this, &AttachmentsChassisEditorList::onCallbackChassisChanged, static_cast<ShipChassisWritable::Messages::ChassisChanged *>(NULL));
	
//	m_table->setSorting(true);
} //lint !e429 //custodial timer

//----------------------------------------------------------------------

AttachmentsChassisEditorList::~AttachmentsChassisEditorList()
{
	delete m_callback;
	m_callback = NULL;
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onTimerUpdate()
{
	if(m_tableNeedsReset)
	{
		m_tableResetCountdown -= 100;
		if (m_tableResetCountdown <= 0)
			resetTable();
	}
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onLineEditNameFiltertextChanged(QString const & text)
{
	m_nameFilter = Unicode::toLower(QStringUtil::toString(text)).c_str();
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onTableDoubleClicked(int row, int col, int , const QPoint & )
{
	ShipComponentDescriptorWritable * const componentDescriptorWritable = findComponentDescriptorWritableForRow(row);
	if (NULL == componentDescriptorWritable)
	{
		WARNING(true, ("AttachmentsChassisEditorList failed to find component descriptor for row [%d]", row));
		return;
	}
	
	std::string const & textStr = QStringUtil::toString(m_table->text(row, col));
	UNREF(textStr);
} //lint !e1762 //not made const

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onTableValueChanged (int row,int col)
{
	ShipComponentDescriptorWritable * const componentDescriptorWritable = findComponentDescriptorWritableForRow(row);
	if (NULL == componentDescriptorWritable)
	{
		WARNING(true, ("AttachmentsChassisEditorList failed to find component descriptor for row [%d]", row));
		return;
	}
	
	std::string const & textStr = QStringUtil::toString(m_table->text(row, col));
	UNREF(textStr);

	//-- fallthrough means an error occured
} //lint !e1762 //not made const

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onTableContextMenuRequested( int , int , const QPoint & _pos)
{	
	ShipChassisWritable * const shipChassisWritable = getShipChassisWritable();
	if (NULL == shipChassisWritable)
		return;

	int totalRowsSelected = 0;
	int anchorCol = -1;

	int const numSelections = m_table->numSelections();
	for (int s = 0; s < numSelections; ++s)
	{
		QTableSelection const & selection = m_table->selection(s);
		
		totalRowsSelected += selection.numRows();

		if (s == 0)
			anchorCol = selection.anchorCol();

		int const colRight = selection.rightCol();
		for (int col = selection.leftCol(); col <= colRight; ++col)
		{
		}
	}

	QPopupMenu * const pop = new QPopupMenu(m_table);
	
	/*
		CMI_compatibilityComponentAdd,
		CMI_compatibilityChassisAdd,
		CMI_compatibilityBothAdd,
		CMI_attachmentsDelete
*/

	IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_compatibilityComponentAdd], this, SLOT(onTableContextCompatibility(int)), 0, CMI_compatibilityComponentAdd));
	IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_compatibilityChassisAdd], this, SLOT(onTableContextCompatibility(int)), 0, CMI_compatibilityChassisAdd));
	IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_compatibilityBothAdd], this, SLOT(onTableContextCompatibility(int)), 0, CMI_compatibilityBothAdd));
	IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_compatibilityComponentRemove], this, SLOT(onTableContextCompatibility(int)), 0, CMI_compatibilityComponentRemove));
	IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_compatibilityChassisRemove], this, SLOT(onTableContextCompatibility(int)), 0, CMI_compatibilityChassisRemove));

	IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_attachmentsDelete], this, SLOT(onTableContextAttachmentsDelete(int)), 0, CMI_attachmentsDelete));

	if (totalRowsSelected != 1)
	{
		pop->setItemEnabled(CMI_compatibilityComponentAdd, false);
		pop->setItemEnabled(CMI_compatibilityChassisAdd, false);
		pop->setItemEnabled(CMI_compatibilityBothAdd, false);
		pop->setItemEnabled(CMI_attachmentsDelete, false);
	}
	else
	{
		bool okComponentAdd = false;
		bool okChassisAdd = false;
		bool okBothAdd = true;

		ShipComponentDescriptorWritable * const shipComponentDescriptorWritable = findComponentDescriptorWritableLastSelected();
		NOT_NULL(shipComponentDescriptorWritable);
		
		bool currentlyCompatible = false;

		int const chassisSlotType = getChassisSlotTypeForColumn(anchorCol);
		if (ShipChassisSlotType::SCST_invalid != chassisSlotType)
		{
			ShipChassisSlot const * const chassisSlot = shipChassisWritable->getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
			if (NULL != chassisSlot)
			{
				if (!chassisSlot->getCompatibilities().empty())
					okComponentAdd = true;
				
				if (NULL != shipComponentDescriptorWritable && chassisSlot->canAcceptComponent(*shipComponentDescriptorWritable)) //lint !e774 //not always true
					currentlyCompatible = true;
			}
		}

		if (NULL != shipComponentDescriptorWritable)  //lint !e774 //not always true
		{
			CrcString const & compatComponent = shipComponentDescriptorWritable->getCompatibility();

			if (!compatComponent.isEmpty())
				okChassisAdd = true;

			if (ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType)) != shipComponentDescriptorWritable->getComponentType())
			{
				okChassisAdd = false;
				okComponentAdd = false;
				okBothAdd = false;
			}
		}
		
		pop->setItemEnabled(CMI_compatibilityComponentRemove, currentlyCompatible);
		pop->setItemEnabled(CMI_compatibilityChassisRemove, currentlyCompatible);

		pop->setItemEnabled(CMI_compatibilityComponentAdd, okComponentAdd);
		pop->setItemEnabled(CMI_compatibilityChassisAdd, okChassisAdd);
		pop->setItemEnabled(CMI_compatibilityBothAdd, okBothAdd);
		
		if (NULL != shipComponentDescriptorWritable)  //lint !e774 //not always true
		{
			if (ShipComponentAttachmentManager::getAttachmentsForShip(shipChassisWritable->getCrc(), shipComponentDescriptorWritable->getCrc(), chassisSlotType).empty())
				pop->setItemEnabled(CMI_attachmentsDelete, false);
		}
	}

	//-- @TODO: remove this once these features are implemented
	pop->setItemEnabled(CMI_compatibilityComponentAdd, false);
	pop->setItemEnabled(CMI_compatibilityChassisAdd, false);
	pop->setItemEnabled(CMI_compatibilityBothAdd, false);
	pop->setItemEnabled(CMI_compatibilityComponentRemove, false);
	pop->setItemEnabled(CMI_compatibilityChassisRemove, false);
	
	pop->popup(_pos);
} //lint !e429 //custodial pop

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onTableContextCompatibility(int id)
{
	ShipComponentDescriptorWritable * const shipComponentDescriptorWritable = findComponentDescriptorWritableLastSelected();
	if (NULL == shipComponentDescriptorWritable)
		return;

	switch (id)
	{
	case CMI_compatibilityComponentAdd:
		IGNORE_RETURN(QMessageBox::information(this, "Not yet implemented", "This feature is not yet implemented.\r\nUse the component and/or chassis screens to adjust compatibilities"));
		break;
	case CMI_compatibilityChassisAdd:
		IGNORE_RETURN(QMessageBox::information(this, "Not yet implemented", "This feature is not yet implemented.\r\nUse the component and/or chassis screens to adjust compatibilities"));
		break;
	case CMI_compatibilityBothAdd:
		IGNORE_RETURN(QMessageBox::information(this, "Not yet implemented", "This feature is not yet implemented.\r\nUse the component and/or chassis screens to adjust compatibilities"));
		break;
	case CMI_compatibilityComponentRemove:
		IGNORE_RETURN(QMessageBox::information(this, "Not yet implemented", "This feature is not yet implemented.\r\nUse the component and/or chassis screens to adjust compatibilities"));
		break;
	case CMI_compatibilityChassisRemove:
		IGNORE_RETURN(QMessageBox::information(this, "Not yet implemented", "This feature is not yet implemented.\r\nUse the component and/or chassis screens to adjust compatibilities"));
		break;
	default:
		break;
	}
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onTableContextAttachmentsDelete(int)
{
	ShipChassisWritable * const shipChassisWritable = getShipChassisWritable();
	if (NULL == shipChassisWritable)
		return;

	uint32 const chassisCrc = shipChassisWritable->getCrc();

	static ShipComponentAttachmentManager::TemplateHardpointPairVector const emptyAttachments;

	int const numSelections = m_table->numSelections();
	for (int s = 0; s < numSelections; ++s)
	{
		QTableSelection const & selection = m_table->selection(s);
		
		int const rowTop = selection.topRow();
		for (int row = selection.bottomRow(); row <= rowTop; ++row)
		{
			ShipComponentDescriptorWritable * const shipComponentDescriptorWritable = findComponentDescriptorWritableForRow(row);
			NOT_NULL(shipComponentDescriptorWritable);

			int const colRight = selection.rightCol();
			for (int col = selection.leftCol(); col <= colRight; ++col)
			{
				int const chassisSlotType = getChassisSlotTypeForColumn(col);
				ShipComponentAttachmentManager::setAttachmentsForShip(chassisCrc, shipComponentDescriptorWritable->getCrc(), chassisSlotType, emptyAttachments);
			}
		}
	}
} //lint !e1762 //not made const

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onTableSelectionChanged()
{
	uint32 chassisCrc = 0;
	uint32 componentCrc = 0;
	int chassisSlotType = getChassisSlotTypeForColumnLastSelected();

	ShipChassisWritable const * const shipChassisWritable = getShipChassisWritable();
	if (NULL != shipChassisWritable)
		chassisCrc = shipChassisWritable->getCrc();

	ShipComponentDescriptorWritable const * const shipComponentDescriptorWritable = findComponentDescriptorWritableLastSelected();
	if (NULL != shipComponentDescriptorWritable)
		componentCrc = shipComponentDescriptorWritable->getCrc();

	//resetTemplateList();
	emit attachmentSelectionChanged(chassisCrc, componentCrc, chassisSlotType, false);
}

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- CALLBACKS
//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onCallbackComponentChanged(ShipComponentDescriptorWritable const & )
{
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onCallbackComponentListChanged(bool const & )
{
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onCallbackAttachmentsChanged(ShipComponentAttachmentManager::Messages::AttachmentsChanged::Payload const & payload)
{
	uint32 const chassisCrc = payload.first;
	uint32 const componentCrc = payload.second.first;
	int const chassisSlotType = payload.second.second;

	ShipChassisWritable * const shipChassisWritable = getShipChassisWritable();
	if (NULL == shipChassisWritable)
		return;

	if (shipChassisWritable->getCrc() != chassisCrc)
		return;

	//@todo: update only the appropriate row
	UNREF(componentCrc);
	UNREF(chassisSlotType);

	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onCallbackChassisListChanged(bool const &)
{
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::onCallbackChassisChanged(ShipChassisWritable const &)
{
	setTableNeedsReset();
}

//----------------------------------------------------------------------
//-- End CALLBACKS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//--
//----------------------------------------------------------------------

void AttachmentsChassisEditorList::resetTable()
{
	m_table->hide();

	ShipChassisWritable * const shipChassisWritable = getShipChassisWritable();
	if (NULL == shipChassisWritable)
		return;

	m_tableNeedsReset = false;

	ShipComponentAttachmentManager::TemplateHardpointPairVector const * thpvsForSlots[static_cast<size_t>(ShipChassisSlotType::SCST_num_types)];		
	ShipChassisSlot const * chassisSlots[static_cast<size_t>(ShipChassisSlotType::SCST_num_types)];
	bool columnPopulated[static_cast<size_t>(ShipChassisSlotType::SCST_num_types)];
	bool columnVisible[static_cast<size_t>(ShipChassisSlotType::SCST_num_types)];
	bool columnInstallable[static_cast<size_t>(ShipChassisSlotType::SCST_num_types)];

	{
		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotType)
		{
			int effectiveCompatibilityType = chassisSlotType;

			// For capships weapons beyond 7 cycle back to the beginning
			if (chassisSlotType >= ShipChassisSlotType::SCST_num_explicit_types && chassisSlotType <= ShipChassisSlotType::SCST_weapon_last)
			{
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
			}

			columnVisible[chassisSlotType] = true;
			columnPopulated[chassisSlotType] = false;
			chassisSlots[chassisSlotType] = shipChassisWritable->getSlot(static_cast<ShipChassisSlotType::Type>(effectiveCompatibilityType));
			columnInstallable[chassisSlotType] = false;
			
			int const componentTypeForSlot = ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
			
			//-- skip slots of we've rejected them based on our component type filter
			if (!m_filterComponentTypes.empty())
			{
				if (!std::binary_search(m_filterComponentTypes.begin(), m_filterComponentTypes.end(), componentTypeForSlot))
				{
					columnVisible[chassisSlotType] = false;
//					REPORT_LOG_PRINT(true, ("AttachmentsChassisEditorList skipping %d due to component filter\n", chassisSlotType));
				}
			}
		}
	}

	ShipComponentDescriptor::StringVector const componentNames = ShipComponentDescriptor::getComponentDescriptorNames();

	m_table->setNumRows(static_cast<int>(componentNames.size()));
	m_table->setNumCols(static_cast<int>(ShipChassisSlotType::SCST_num_types));
	
	QHeader * const headerVertical = NON_NULL(m_table->verticalHeader());

	int rowCount = 0;
	for (ShipComponentDescriptor::StringVector::const_iterator it = componentNames.begin(); it != componentNames.end(); ++it, ++rowCount)
	{
		std::string const & componentName = *it;
		
		ShipComponentDescriptorWritable const * const shipComponentDescriptorWritable = safe_cast<ShipComponentDescriptorWritable const *>(ShipComponentDescriptor::findShipComponentDescriptorByName(PersistentCrcString(componentName.c_str(), true)));
		
		NOT_NULL(shipComponentDescriptorWritable);

		if (m_hideUninstallableRows)
		{
			if (!shipChassisWritable->canAcceptComponent(*shipComponentDescriptorWritable))
			{
				m_table->hideRow(rowCount);
				continue;
			}
		}

		if (!m_nameFilter.empty())
		{
			if (static_cast<size_t>(std::string::npos) == componentName.find(m_nameFilter))
			{
				m_table->hideRow(rowCount);
				continue;
			}
		}
		
		if (!m_filterComponentTypes.empty())
		{
			if (!std::binary_search(m_filterComponentTypes.begin(), m_filterComponentTypes.end(), static_cast<int>(shipComponentDescriptorWritable->getComponentType())))
			{
				m_table->hideRow(rowCount);
				continue;
			}
		}
		
		bool const hasSomeAttachments = ShipComponentAttachmentManager::getAttachmentsForShip(shipChassisWritable->getCrc(), shipComponentDescriptorWritable->getCrc(), thpvsForSlots);

		if (m_hideEmptyRows && !hasSomeAttachments)
		{
			m_table->hideRow(rowCount);
			continue;
		}

		m_table->showRow(rowCount);

		headerVertical->setLabel(rowCount, QString(componentName.c_str()));

		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotType)
		{
			int const col = chassisSlotType;
			if (!columnVisible[col])
				continue;

			if (NULL == chassisSlots[chassisSlotType])
			{
				int const componentTypeForSlot = ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
				if (shipComponentDescriptorWritable->getComponentType() != componentTypeForSlot)
					m_table->setItem(rowCount, col, new MyDisabledTableItem(m_table, QTableItem::Never, QString(""), false));
				else
					m_table->setItem(rowCount, col, new MyDisabledTableItem(m_table, QTableItem::Never, QString(""), true));
			}
			else
			{
				char buf[1024];
				size_t const buf_size = sizeof(buf);
				
				ShipComponentAttachmentManager::TemplateHardpointPairVector const * const thpv = thpvsForSlots[chassisSlotType];
				int const count = (NULL == thpv) ? 0 : static_cast<int>(thpv->size());
				std::string const & matchingCompat = chassisSlots[chassisSlotType]->getMatchingCompatibilityString(*shipComponentDescriptorWritable);
				IGNORE_RETURN(snprintf(buf, buf_size, "%d:  %s", count, matchingCompat.c_str()));
				
				if (NULL != thpv)
				{					
					m_table->setItem(rowCount, col, new MyAttachmentTableItem(m_table, QString(buf), count, true, true));
					
					if (!columnPopulated[col])
						columnPopulated[col] = true;
				}
				
				else if (!chassisSlots[chassisSlotType]->canAcceptComponent(*shipComponentDescriptorWritable))
				{
					m_table->setItem(rowCount, col, new MyAttachmentTableItem(m_table, QString(""), 0, true, false));
				}
				else
				{
					m_table->setItem(rowCount, col, new MyAttachmentTableItem(m_table, QString(buf), 0, true, true));
				}

				if (!columnInstallable[col])
					columnInstallable[col] = true;
			}
		}
	}

	{
		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotType)
		{
			int const col = chassisSlotType;

			if (m_hideUninstallableCols && !columnInstallable[col])
				columnVisible[col] = false;
			else if (m_hideEmptyCols && !columnPopulated[col])
				columnVisible[col] = false;

			if (!columnVisible[col])
				m_table->hideColumn(col);
			else
				m_table->showColumn(col);
		}
	}

	{
		QHeader * const headerHorizontal = NON_NULL(m_table->horizontalHeader());
		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotType)
		{
			int const col = chassisSlotType;
			std::string const & slotName = ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
			headerHorizontal->setLabel(col, QString(slotName.c_str()));
		}
	}

	m_table->show();
}

//----------------------------------------------------------------------

ShipComponentDescriptorWritable * AttachmentsChassisEditorList::findComponentDescriptorWritableForRow(int const row) const
{
	QHeader * const headerVertical = NON_NULL(m_table->verticalHeader());
	std::string const & componentName = QStringUtil::toString(headerVertical->label(row));

	if (!componentName.empty())
	{
		ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByName(ConstCharCrcString(componentName.c_str()));
		ShipComponentDescriptorWritable const * const shipComponentDescriptorWritable = safe_cast<ShipComponentDescriptorWritable const *>(shipComponentDescriptor);
		return const_cast<ShipComponentDescriptorWritable *>(shipComponentDescriptorWritable);
	}

	return NULL;
}

//----------------------------------------------------------------------

ShipComponentDescriptorWritable * AttachmentsChassisEditorList::findComponentDescriptorWritableLastSelected() const
{
	int const numSelections = m_table->numSelections();
	if (numSelections > 0)
	{
		QTableSelection const & selection = m_table->selection(0);
		
		int const row = selection.anchorRow();
		
		return findComponentDescriptorWritableForRow(row);
	}

	return NULL;
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::setTableNeedsReset()
{
	m_tableNeedsReset = true;
	m_tableResetCountdown = 250;
}

//----------------------------------------------------------------------

QTable * AttachmentsChassisEditorList::getTable()
{
	return m_table;
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::getSelectedComponentsList(StringVector & result) const
{
	if (NULL == m_table)
		return;

	int const numSelections = m_table->numSelections();
	
	if (numSelections > 0)
	{
		for (int s = 0; s < numSelections; ++s)
		{			
			QTableSelection const & selection = m_table->selection(s);
			int const rowBottom = selection.bottomRow();
			//		result.resize(result.size() + (rowBottom - selection.topRow()));
			
			for (int row = selection.topRow(); row <= rowBottom; ++row)
			{
				ShipComponentDescriptorWritable const * const shipComponentDescriptorWritable = findComponentDescriptorWritableForRow(row);

				if (NULL == shipComponentDescriptorWritable)
				{
					WARNING(true, ("ComponentDescriptorsDetails invalid shipComponentDescriptorWritable on row [%d]", row));
					continue;
				}
				std::string const & chassisType = shipComponentDescriptorWritable->getName().getString();
				result.push_back(chassisType);
			}
		}
	}
	else
	{
		int const currentRow = m_table->currentRow();
		ShipComponentDescriptorWritable const * const shipComponentDescriptorWritable = findComponentDescriptorWritableForRow(currentRow);		
		if (NULL == shipComponentDescriptorWritable)
		{
			WARNING(true, ("ComponentDescriptorsDetails invalid shipComponentDescriptorWritable on row [%d]", currentRow));
		}
		else
		{
			std::string const & componentName = shipComponentDescriptorWritable->getName().getString();
			result.push_back(componentName);
		}
	}
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::handleComponentTypeFilterChange(IntVector const & componentTypes)
{
	m_filterComponentTypes = componentTypes;
	std::sort(m_filterComponentTypes.begin(), m_filterComponentTypes.end());
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::handleChassisNameChange(std::string const & chassisName)
{
	m_chassisName = chassisName;
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void AttachmentsChassisEditorList::handleHideFlags(bool hideEmptyRows, bool hideEmptyCols, bool hideUninstallableRows, bool hideUninstallableCols)
{
	m_hideEmptyRows = hideEmptyRows;
	m_hideEmptyCols = hideEmptyCols;
	m_hideUninstallableRows = hideUninstallableRows;
	m_hideUninstallableCols = hideUninstallableCols;
	setTableNeedsReset();
}

//----------------------------------------------------------------------

ShipChassisWritable * AttachmentsChassisEditorList::getShipChassisWritable() const
{
	ShipChassisWritable const * const shipChassisWritable = safe_cast<ShipChassisWritable const*>(ShipChassis::findShipChassisByName(ConstCharCrcString(m_chassisName.c_str())));
	return const_cast<ShipChassisWritable *>(shipChassisWritable);
}

//----------------------------------------------------------------------

int AttachmentsChassisEditorList::getChassisSlotTypeForColumn(int col) const
{
	QHeader const * const horizontalHeader = m_table->horizontalHeader();
	if (NULL != horizontalHeader)
	{
		std::string const & slotName = QStringUtil::toString(horizontalHeader->label(col));
		return ShipChassisSlotType::getTypeFromName(slotName);
	}
	return ShipChassisSlotType::SCST_invalid;
}

//----------------------------------------------------------------------

int AttachmentsChassisEditorList::getChassisSlotTypeForColumnLastSelected() const
{
	int const numSelections = m_table->numSelections();
	if (numSelections > 0)
		return getChassisSlotTypeForColumn(m_table->selection(0).anchorCol());

	return static_cast<int>(ShipChassisSlotType::SCST_invalid);

}

//======================================================================
