//======================================================================
//
// ChassisEditorList.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "ChassisEditorList.h"
#include "ChassisEditorList.moc"

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
#include "ChassisNewDialog.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "TemplateNewDialog.h"
#include "QStringUtil.h"
#include "CompatibilityEditor.h"
#include <vector>

//======================================================================

namespace ChassisEditorListNamespace
{
	//----------------------------------------------------------------------

	class MyFlybyTableItem : public QTableItem
	{
	public:
		void paint(QPainter* p,const QColorGroup & cg,const QRect & cr,bool selected)
		{
			QColorGroup g (cg);
			
			const char * const flybySound = text().latin1();
			if (NULL != flybySound && NULL != *flybySound)
			{
				if (!TreeFile::exists(flybySound))
				{
					g.setColor(QColorGroup::Text, "red");
					g.setColor(QColorGroup::HighlightedText, "red");
				}
			}
			QTableItem::paint( p , g , cr , selected );
		}
		
		MyFlybyTableItem(QTable * const _table, EditType et, QString const & _text) : 
		QTableItem(_table, et, _text)
		{
		}
	private:
		MyFlybyTableItem();
	};

	//----------------------------------------------------------------------

	class MyDisabledTableItem : public QTableItem
	{
	public:
		void paint(QPainter* p,const QColorGroup & cg,const QRect & cr,bool selected)
		{
			QColorGroup g (cg);
			g.setColor(QColorGroup::Background , g.midlight());//"grey");
			g.setColor(QColorGroup::Base , g.midlight());
			
			QTableItem::paint( p , g , cr , selected );
		}
		
		MyDisabledTableItem(QTable * const _table, QString const & _text) : 
		QTableItem(_table, QTableItem::Never, _text)
		{
		}
	private:
		MyDisabledTableItem();
	};

	//----------------------------------------------------------------------

	class MyTargetableTableItem : public QTableItem
	{
	public:
		void paint(QPainter* p,const QColorGroup & cg,const QRect & cr,bool selected)
		{
			QTableItem::paint( p , cg , cr , selected );
		}
		
		MyTargetableTableItem(QTable * const _table, QString const & _text) : 
		QTableItem(_table, QTableItem::Never, _text)
		{
		}

		virtual int alignment() const
		{
			return static_cast<int>(Qt::AlignCenter);
		}
	private:
		MyTargetableTableItem();
	};

	//----------------------------------------------------------------------

	class MyCompatibilityTableItem : public QTableItem
	{
	public:
		void paint(QPainter* p,const QColorGroup & cg,const QRect & cr,bool selected)
		{
			QColorGroup g (cg);

			QColor const & normalBase = g.base();
			QColor const myYellow(128, 255, 0);
			QColor const myBaseYellow((normalBase.red() * 3 + myYellow.red()) / 4, (normalBase.green() * 3 + myYellow.green()) / 4, (normalBase.blue() * 3 + myYellow.blue()) / 4);
			QColor const myGreen(0, 255, 0);
			QColor const myBaseGreen((normalBase.red() * 3 + myGreen.red()) / 4, (normalBase.green() * 3 + myGreen.green()) / 4, (normalBase.blue() * 3 + myGreen.blue()) / 4);

			const char * const flybySound = text().latin1();
			if (NULL != flybySound && NULL != *flybySound)
			{
				g.setColor(QColorGroup::Background , myBaseGreen);
				g.setColor(QColorGroup::Base , myBaseGreen);
			}
			else
			{
				g.setColor(QColorGroup::Background , myBaseYellow);
				g.setColor(QColorGroup::Base , myBaseYellow);
			}
			
			QTableItem::paint( p , g , cr , selected );
		}
		
		MyCompatibilityTableItem(QTable * const _table, QString const & _text) : 
		QTableItem(_table, QTableItem::OnTyping, _text)
		{
		}
	private:
		MyCompatibilityTableItem();
	};

	//----------------------------------------------------------------------

	enum TableCols
	{
		TC_name = 0,
		TC_flybySound = 1,
		TC_hitSoundGroup = 2,
		TC_wingOpenSpeedFactor = 3,
		TC_firstSlot = 4
	};

	enum ContextMenuItems
	{
		CMI_targetableTrue,
		CMI_targetableFalse,
		CMI_targetableInvert,
		CMI_flybyChooser,
		CMI_flybyPlayer,
		CMI_deleteChassis,
		CMI_newChassis,
		CMI_cloneChassis,
		CMI_newTemplate

	};

	char const * const s_contextMenuItemNames[] =
	{
		"Set Targetable",
		"Set Untargetable",
		"Invert Targetable",
		"Choose Flyby Sound",
		"Play Flyby Sound",
		"Delete Chassis",
		"New Chassis",
		"Clone Chassis",
		"New Template for Chassis"
	};
}
using namespace ChassisEditorListNamespace;

//lint -esym(641, ChassisEditorListNamespace::TableCols)
//lint -esym(641, ChassisEditorListNamespace::ContextMenuItems)

//----------------------------------------------------------------------

ChassisEditorList::ChassisEditorList(QWidget *_parent, char const *_name) :
BaseChassisEditorList(_parent, _name),
m_tableNeedsReset(true),
m_tableResetCountdown(0),
m_nameFilter(),
m_callback(new MessageDispatch::Callback)
{	
//	connect(m_lineEditNameFilter, SIGNAL(textChanged(const QString &)), SLOT(onLineEditNameFiltertextChanged(const QString &)));
	IGNORE_RETURN(connect(m_table, SIGNAL(doubleClicked(int, int, int, const QPoint & )), SLOT(onTableDoubleClicked(int, int, int, const QPoint &))));
	IGNORE_RETURN(connect(m_table, SIGNAL(valueChanged(int, int)), SLOT(onTableValueChanged(int, int)))); 
	IGNORE_RETURN(connect(m_table, SIGNAL(valueChanged(int, int)), SLOT(onTableValueChanged(int, int)))); 
	IGNORE_RETURN(connect(m_table, SIGNAL(contextMenuRequested ( int , int , const QPoint &  )), SLOT(onTableContextMenuRequested ( int , int , const QPoint &  ))));
	IGNORE_RETURN(connect(m_table, SIGNAL(selectionChanged ()), SLOT(onTableSelectionChanged ())));
	
	resetTable();

	QTimer *timer = new QTimer(this);
	IGNORE_RETURN(connect(timer, SIGNAL(timeout()), SLOT(updateTimer())));
	IGNORE_RETURN(timer->start(100));

	m_callback->connect(*this, &ChassisEditorList::onCallbackChassisListChanged, static_cast<ShipChassisWritable::Messages::ChassisListChanged *>(NULL));
	m_callback->connect(*this, &ChassisEditorList::onCallbackChassisChanged, static_cast<ShipChassisWritable::Messages::ChassisChanged *>(NULL));
} //lint !e429 //custodial timer

//----------------------------------------------------------------------

ChassisEditorList::~ChassisEditorList()
{
	delete m_callback;
	m_callback = NULL;
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void ChassisEditorList::updateTimer()
{
	if(m_tableNeedsReset)
	{
		m_tableResetCountdown -= 100;
		if (m_tableResetCountdown <= 0)
			resetTable();
	}
}

//----------------------------------------------------------------------

void ChassisEditorList::onLineEditNameFiltertextChanged(QString const & text)
{
	m_nameFilter = Unicode::toLower(QStringUtil::toString(text)).c_str();
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void ChassisEditorList::onTableDoubleClicked(int row, int col, int , const QPoint & )
{
	ShipChassisWritable * const shipChassis = findShipChassisWritableForRow(row);
	if (NULL == shipChassis)
	{
		WARNING(true, ("ChassisEditorList failed to find chassis for row [%d]", row));
		return;
	}
	
	char const * const text = m_table->text(row, col).latin1();
	std::string const & textStr = (NULL != text) ? text : "";

	if (col == TC_name)
	{
	}
	else if (col == TC_flybySound)
	{
		std::string const & flybySound = shipChassis->getFlyBySound();
		if (!flybySound.empty())
		{
			char buf[1024];
			size_t const buf_size = sizeof(buf);
			if (!TreeFile::getPathName(flybySound.c_str(), buf, buf_size))
				WARNING(true, ("ChassisEditorList failed to lookup path for sound [%s]", flybySound.c_str()));
			else
			{
			}
		}
	}
	else if (col == TC_wingOpenSpeedFactor)
	{
	}
	else
	{
		int const slotCol = col - TC_firstSlot;
		int const chassisSlotType = findChassisSlotTypeForColumn(col);
		ShipChassisSlot * chassisSlot = shipChassis->getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));

		if (NULL == chassisSlot)
		{
			ShipChassisSlot const newSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType), std::string(), 10, true);
			shipChassis->addSlot(newSlot);
			chassisSlot = shipChassis->getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));

			if (NULL == chassisSlot)
			{
				WARNING(true, ("ChassisEditorList unable to create new slot."));
				IGNORE_RETURN(QMessageBox::warning(NULL, "Failed to Create Slot", "An error occured while creating the slot."));
				return;
			}
		}
		
		//-- compats
		if ((slotCol % 3) == 0)
		{
			CompatibilityEditor * const ce = new CompatibilityEditor(this, "compat_editor", *chassisSlot, NULL);
			std::string finalSelection;
			if (ce->showAndTell(finalSelection))
			{
				chassisSlot->setCompatibilities(finalSelection);
				shipChassis->notifyChanged();
			}

		} //lint !e429 //custodial ce
		//-- hitweight
		else if ((slotCol % 3) == 1)
		{
		}
		//-- targetable
		else if ((slotCol % 3) == 2)
		{
			if (NULL == chassisSlot) //lint !e774 //null
			{
				WARNING(true, ("ChassisEditorList could not find slot [%d] for chassis [%s]", chassisSlotType, shipChassis->getName().getString()));
				return;
			}
			
			bool const targetable = !(!textStr.empty() && textStr[0] != '0');
			m_table->setText(row, col, targetable ? "X" : "");

//			shipChassis->setSlotTargetable(chassisSlotType, targetable);
		}
	}

}

//----------------------------------------------------------------------

void ChassisEditorList::onTableValueChanged (int row,int col)
{
	ShipChassisWritable * const shipChassis = findShipChassisWritableForRow(row);
	if (NULL == shipChassis)
	{
		WARNING(true, ("ChassisEditorList invalid chassis on row [%d]", row));
		return;
	}
	
	std::string const & textStr = QStringUtil::toString(m_table->text(row, col));
	
	//-- flyby column
	if (col == TC_flybySound)
	{
		shipChassis->setFlyBySound(textStr);
	}
	else if (col == TC_hitSoundGroup)
	{
		shipChassis->setHitSoundGroup(textStr);
	}
	else if (col == TC_wingOpenSpeedFactor)
	{
		float const factor = static_cast<float>(atof(textStr.c_str()));
		shipChassis->setWingOpenSpeedFactor(factor);
	}
	else if (col == TC_name)
	{
		if (textStr.empty())
		{
			IGNORE_RETURN(QMessageBox::warning(NULL, "Failed to Rename", "You cannot set the component name to an empty string."));
		}
		else if (textStr == shipChassis->getName().getString())
		{
			//-- silently ignore changing the name back to the current name
		}
		else if (!shipChassis->setName(ConstCharCrcString(textStr.c_str())))
		{
			IGNORE_RETURN(QMessageBox::warning(NULL, "Failed to Rename", "Failed to Rename the chassis, check DebugView warnings for more details."));
		}
	}
	else
	{
		int const slotCol = col - TC_firstSlot;
		int const chassisSlotType = findChassisSlotTypeForColumn(col);
		ShipChassisSlot * const chassisSlot = shipChassis->getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
		
		//-- compats
		if ((slotCol % 3) == 0)
		{
			
		}
		//-- hitweight
		else if ((slotCol % 3) == 1)
		{
			if (NULL == chassisSlot)
			{
				WARNING(true, ("ChassisEditorList could not find slot [%d] for chassis [%s]", chassisSlotType, shipChassis->getName().getString()));
				return;
			}
			
			int const hitWeight = atoi(textStr.c_str());
			chassisSlot->setHitWeight(hitWeight);
		}
		//-- targetable
		else if ((slotCol % 3) == 2)
		{
			if (NULL == chassisSlot)
			{
				WARNING(true, ("ChassisEditorList could not find slot [%d] for chassis [%s]", chassisSlotType, shipChassis->getName().getString()));
				return;
			}
			
			bool const targetable = !textStr.empty() && textStr[0] != '0';
			shipChassis->setSlotTargetable(chassisSlotType, targetable);
		}
	}
}

//----------------------------------------------------------------------

void ChassisEditorList::onTableContextMenuRequested( int , int , const QPoint & _pos)
{
	bool useTargetable = false;
	bool enableTargetable = false;
	bool useFlyby = false;
	bool enableFlybyPlayer = false;
	
	int const numSelections = m_table->numSelections();
	for (int s = 0; s < numSelections; ++s)
	{
		QTableSelection const & selection = m_table->selection(s);
		
		int const colRight = selection.rightCol();
		for (int col = selection.leftCol(); col <= colRight; ++col)
		{
			if (col > TC_firstSlot)
				useTargetable = true;

			if (col == TC_flybySound)
				useFlyby = true;
			
//			int const slotCol = col - TC_firstSlot;			
			int const chassisSlotType = findChassisSlotTypeForColumn(col);
			
			int const rowBottom = selection.bottomRow();
			for (int row = selection.topRow(); row <= rowBottom; ++row)
			{
				ShipChassisWritable * const shipChassis = findShipChassisWritableForRow(row);
				if (NULL == shipChassis)
				{
					WARNING(true, ("ChassisEditorList invalid chassis on row [%d]", row));
					continue;
				}

				if (!shipChassis->getFlyBySound().empty())
					enableFlybyPlayer = true;
				
				ShipChassisSlot * const chassisSlot = shipChassis->getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
				
				if (NULL == chassisSlot)
				{
					continue;
				}
				
				if (col > TC_firstSlot)
					enableTargetable = true;
			}
		}
	}

	QPopupMenu * const pop = new QPopupMenu(m_table);
	
	if(useTargetable)
	{
		IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_targetableTrue], this, SLOT(onTableContextTargetable(int)), 0, CMI_targetableTrue));
		IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_targetableFalse], this, SLOT(onTableContextTargetable(int)), 0, CMI_targetableFalse));
		IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_targetableInvert], this, SLOT(onTableContextTargetable(int)), 0, CMI_targetableInvert));

		if (!enableTargetable)
		{
			pop->setItemEnabled(CMI_targetableTrue, false);
			pop->setItemEnabled(CMI_targetableFalse, false);
			pop->setItemEnabled(CMI_targetableInvert, false);
		}
		IGNORE_RETURN(pop->insertSeparator());
	}
	if (useFlyby)
	{
		IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_flybyChooser], this, SLOT(onTableContextFlyby(int)), 0, CMI_flybyChooser));
		IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_flybyPlayer], this, SLOT(onTableContextFlyby(int)), 0, CMI_flybyPlayer));

		if (!enableFlybyPlayer)
		{
			pop->setItemEnabled(CMI_flybyPlayer, false);
		}
		IGNORE_RETURN(pop->insertSeparator());
	}

	
	IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_newChassis], this, SLOT(onTableContextChassisNew(int)), 0, CMI_newChassis));
	IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_cloneChassis], this, SLOT(onTableContextChassisNew(int)), 0, CMI_cloneChassis));
	IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_deleteChassis], this, SLOT(onTableContextChassisDelete(int)), 0, CMI_deleteChassis));

	IGNORE_RETURN(pop->insertSeparator());
	IGNORE_RETURN(pop->insertItem(s_contextMenuItemNames[CMI_newTemplate], this, SLOT(onTableContextChassisNewTemplate(int)), 0, CMI_newTemplate));
	
	pop->popup(_pos);

} //lint !e429 //custodial pop

//----------------------------------------------------------------------

void ChassisEditorList::onTableContextTargetable(int id)
{
	int const numSelections = m_table->numSelections();

	for (int s = 0; s < numSelections; ++s)
	{
		QTableSelection const & selection = m_table->selection(s);
		
		int const colRight = selection.rightCol();
		for (int col = selection.leftCol(); col <= colRight; ++col)
		{
			if (col < TC_firstSlot)
				continue;
			
			int const slotCol = col - TC_firstSlot;
			
			int const chassisSlotType = findChassisSlotTypeForColumn(col);
			
			int const rowBottom = selection.bottomRow();
			for (int row = selection.topRow(); row <= rowBottom; ++row)
			{
				ShipChassisWritable * const shipChassis = findShipChassisWritableForRow(row);
				if (NULL == shipChassis)
				{
					WARNING(true, ("ChassisEditorList invalid chassis on row [%d]", row));
					continue;
				}
				
				ShipChassisSlot * const chassisSlot = shipChassis->getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
				
				if (NULL == chassisSlot)
					continue;
				
				bool targetable = false;
				
				switch (id)
				{
				case CMI_targetableTrue:
					targetable = true;
					break;
				case CMI_targetableFalse:
					targetable = false;
					break;
				case CMI_targetableInvert:
					targetable = !chassisSlot->isTargetable();
					break;					
				default:
					break;
				}
				
				shipChassis->setSlotTargetable(static_cast<ShipChassisSlotType::Type>(chassisSlotType), targetable);
				m_table->setText(row, ((slotCol / 3) * 3) + (2 + TC_firstSlot), targetable ? "X" : "");
			}
		}
	}
}

//----------------------------------------------------------------------

void ChassisEditorList::onTableContextFlyby(int id)
{
	std::string newFlybySoundName;

	if (id == CMI_flybyChooser)
	{
		QString const & s = QFileDialog::getOpenFileName(
			"",
			"Sound Templates (*.snd)", 
			this,
			"open file dialog"
			"Choose a Sound Template" );
		
		newFlybySoundName = QStringUtil::toString(s);

		if (!TreeFile::stripTreeFileSearchPathFromFile(newFlybySoundName, newFlybySoundName))
		{
			WARNING(true, ("ChassisEditorList invalid sound path"));
			return;
		}

		if (newFlybySoundName.empty())
			return;
	}
	
	int const numSelections = m_table->numSelections();
	
	std::string pathToLookup;
	
	for (int s = 0; s < numSelections; ++s)
	{
		QTableSelection const & selection = m_table->selection(s);
		
		int const colRight = selection.rightCol();
		for (int col = selection.leftCol(); col <= colRight; ++col)
		{
			if (col != TC_flybySound)
				continue;
			
			//			int const slotCol = col - TC_firstSlot;
			
//			int const chassisSlotType = findChassisSlotTypeForColumn(col);
			
			int const rowBottom = selection.bottomRow();
			for (int row = selection.topRow(); row <= rowBottom; ++row)
			{
				ShipChassisWritable * const shipChassis = findShipChassisWritableForRow(row);
				if (NULL == shipChassis)
				{
					WARNING(true, ("ChassisEditorList invalid chassis on row [%d]", row));
					continue;
				}
				
				if (id == CMI_flybyPlayer)
				{
					std::string const & flyby = shipChassis->getFlyBySound();
					if (!flyby.empty())
					{
						IGNORE_RETURN(Audio::playSound(flyby.c_str(), Vector::zero, NULL));
					}
				}
				else if (id == CMI_flybyChooser)
				{
					shipChassis->setFlyBySound(newFlybySoundName);
					m_table->setText(row, col, newFlybySoundName.c_str());
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void ChassisEditorList::onTableContextChassisNew(int id)
{
	switch (id)
	{
	case CMI_newChassis:
		{
			ChassisNewDialog * const diag = new ChassisNewDialog(NULL, "New Chassis", std::string());
			diag->show();
		} //lint !e429 //custodial diag
		break;
	case CMI_cloneChassis:
		{
			ShipChassisWritable const * const chassis = findShipChassisWritableLastSelected();
			
			if (NULL != chassis)
			{
				ChassisNewDialog * const diag = new ChassisNewDialog(NULL, "New Chassis", std::string(chassis->getName().getString()));
				diag->show();
			} //lint !e429 //custodial diag
		}
		break;
	default:
		break;
	}
}

//----------------------------------------------------------------------

void ChassisEditorList::onTableContextChassisDelete(int )
{
	ChassisEditorList::StringVector selectedChassisList;
	getSelectedChassisList(selectedChassisList);


	ChassisEditorList::StringVector chassisWithOutstandingTemplates;

	{
		ShipComponentEditorServerTemplateManager::ServerTemplateVector serverTemplates;

		for (StringVector::const_iterator it = selectedChassisList.begin(); it != selectedChassisList.end(); ++it)
		{
			std::string const & chassisName = *it;
			serverTemplates.clear();
			ShipComponentEditorServerTemplateManager::findObjectTemplatesForChassisType(chassisName, serverTemplates);
			if (!serverTemplates.empty())
			{
				chassisWithOutstandingTemplates.push_back(chassisName);
			}
		}
	}
	
	char buf[1024];
	size_t const buf_size = sizeof(buf);

	if (!chassisWithOutstandingTemplates.empty())
	{		
		strcpy(buf, "You are deleting chassis types with outstanding referent object templates:\n");

		for (StringVector::const_iterator it = chassisWithOutstandingTemplates.begin(); it != chassisWithOutstandingTemplates.end(); ++it)
		{
			std::string const & outstanding = *it;
			strcat(buf, outstanding.c_str());
			strcat(buf, "\n");
		}

		strcat(buf, "Continue?");
		
		int const result = QMessageBox::question(NULL, "Orphan Templates", buf);
		if (result != QMessageBox::Yes)
			return;
	}

	{
		bool yesAll = false;

		int count = 0;
		for (StringVector::const_iterator it = selectedChassisList.begin(); it != selectedChassisList.end(); ++it, ++count)
		{
			std::string const & chassisName = *it;

			bool ok = false;

			if (!yesAll)
			{
				IGNORE_RETURN(snprintf(buf, buf_size, "REALLY DELETE chassis [%s]?", chassisName.c_str()));

				int result = 0;
				if ((count + 1) < static_cast<int>(selectedChassisList.size()))
					result = QMessageBox::question(NULL, "Confirm Delete", buf, QMessageBox::No, QMessageBox::Yes, QMessageBox::YesAll);
				else
					result = QMessageBox::question(NULL, "Confirm Delete", buf, QMessageBox::No, QMessageBox::Yes);

				if (QMessageBox::YesAll == result)
				{
					yesAll = true;
					ok = true;
				}
				else if (QMessageBox::Yes == result)
					ok = true;
				else
					continue;
			}
			else
				ok = true;

			if (ok) //lint !e738 !e774 //true
			{
				ShipChassisWritable * const shipChassisWritable = const_cast<ShipChassisWritable *>(safe_cast<ShipChassisWritable const * >(ShipChassis::findShipChassisByName(PersistentCrcString(chassisName.c_str(), true))));
				NOT_NULL(shipChassisWritable);

				if (shipChassisWritable->removeChassis())
				{
					IGNORE_RETURN(snprintf(buf, buf_size, "Deleted chassis [%s]", chassisName.c_str()));
					IGNORE_RETURN(QMessageBox::information(NULL, "Deleted Chassis", buf));
				}
				else
				{
					IGNORE_RETURN(snprintf(buf, buf_size, "Failed to delete [%s]", chassisName.c_str()));
					IGNORE_RETURN(QMessageBox::critical(NULL, "Failed to Delete Chassis", buf));
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void ChassisEditorList::onTableContextChassisNewTemplate(int)
{
	ShipChassisWritable const * const shipChassisWritable = ChassisEditorList::findShipChassisWritableLastSelected();

	std::string const & chassisName = (NULL != shipChassisWritable) ? shipChassisWritable->getName().getString() : "";

	TemplateNewDialog * const tnd = new TemplateNewDialog(NULL, "fff", chassisName, std::string());
	IGNORE_RETURN(tnd->exec());
} //lint !e429 //custodial tnd

//----------------------------------------------------------------------

void ChassisEditorList::onTableSelectionChanged()
{
	//resetTemplateList();
} //lint !e1762 //const

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- CALLBACKS
//----------------------------------------------------------------------

void ChassisEditorList::onCallbackChassisListChanged(bool const & )
{
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void ChassisEditorList::onCallbackChassisChanged(ShipChassisWritable const & )
{
	setTableNeedsReset();
}

//----------------------------------------------------------------------
//-- End CALLBACKS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//--
//----------------------------------------------------------------------

void ChassisEditorList::resetTable()
{
	m_tableNeedsReset = false;
//	m_table->clear();

	StringVector chassisNames;
	{
		ShipChassis::PersistentCrcStringVector const & shipChassisNameVector = ShipChassis::getShipChassisCrcVector();
		chassisNames.reserve(shipChassisNameVector.size());
		for (ShipChassis::PersistentCrcStringVector::const_iterator it = shipChassisNameVector.begin(); it != shipChassisNameVector.end(); ++it)
		{
			PersistentCrcString const * const pcs = *it;
			chassisNames.push_back(pcs->getString());
		}

		std::sort(chassisNames.begin(), chassisNames.end());
	}

	bool showAllColumns = true;
	
	BitArray slotsForVisibleChassis(static_cast<int>(ShipChassisSlotType::SCST_num_explicit_types));
	
	m_table->hide();

	m_table->setNumRows(static_cast<int>(chassisNames.size()));
	int rowCount = 0;
	
	char buf[64];
	size_t const buf_size = sizeof(buf);

	QHeader * const headerVertical = NON_NULL(m_table->verticalHeader());

	{
		for (StringVector::const_iterator it = chassisNames.begin(); it != chassisNames.end(); ++it)
		{
			std::string const & chassisName = *it;
			ShipChassis const * const chassis = ShipChassis::findShipChassisByName(PersistentCrcString(chassisName.c_str(), true));
			NOT_NULL(chassis);
			
			if (!m_nameFilter.empty())
			{
				if (static_cast<size_t>(std::string::npos) == chassisName.find(m_nameFilter))
					continue;
			}
			
			m_table->setText(rowCount, TC_name, QString(chassis->getName().getString()));
			
			m_table->setItem(rowCount, TC_flybySound, new MyFlybyTableItem(m_table, QTableItem::OnTyping, QString(chassis->getFlyBySound().c_str())));
			m_table->setItem(rowCount, TC_hitSoundGroup, new QTableItem(m_table, QTableItem::OnTyping, QString(chassis->getHitSoundGroup().c_str())));

			float const wingOpenSpeedFactor = chassis->getWingOpenSpeedFactor();
			if (wingOpenSpeedFactor != 1.0f)
				IGNORE_RETURN(snprintf(buf, buf_size, "%2.2f", chassis->getWingOpenSpeedFactor()));
			else
				*buf = 0;
			m_table->setText(rowCount, TC_wingOpenSpeedFactor, buf);

			headerVertical->setLabel(rowCount, QString(chassis->getName().getString()));

			if (!showAllColumns)
			{
				ShipChassis::SlotVector const & slotVector = chassis->getSlots();
				
				for (ShipChassis::SlotVector::const_iterator sit = slotVector.begin(); sit != slotVector.end(); ++sit)
				{
					ShipChassisSlot const & slot = *sit;
					slotsForVisibleChassis.setBit(static_cast<int>(slot.getSlotType()));
				}
			}
			else
			{
				for (int i = 0; i < static_cast<int>(ShipChassisSlotType::SCST_num_explicit_types); ++i)
					slotsForVisibleChassis.setBit(i);
			}

			
			++rowCount;
		}
	}
	
	m_table->setNumRows(rowCount);
	
	int const slotCountToDisplay = showAllColumns ? static_cast<int>(ShipChassisSlotType::SCST_num_explicit_types) : slotsForVisibleChassis.getNumberOfSetBits();
	int const slotColsToDisplay = slotCountToDisplay * 3;
	int const slotColsStartingRow = TC_firstSlot;
	
	int const currentNumCols = m_table->numCols();
	
	if (currentNumCols != (slotColsToDisplay + slotColsStartingRow))
		m_table->setNumCols(slotColsToDisplay + slotColsStartingRow);
	
	//-- setup the column header
	{
		QHeader * const headerHorizontal = NON_NULL(m_table->horizontalHeader());
		int visibleChassisSlotIndex = 0;
		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_explicit_types); ++chassisSlotType)
		{
			if (!slotsForVisibleChassis.testBit(chassisSlotType))
				continue;

			int const colForSlot = (visibleChassisSlotIndex * 3) + slotColsStartingRow;
			std::string const & chassisSlotTypeName = ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
			headerHorizontal->setLabel(colForSlot, QString(chassisSlotTypeName.c_str()));
			headerHorizontal->setLabel(colForSlot + 1, "HitWeight");
			headerHorizontal->setLabel(colForSlot + 2, "Targetable");
			++visibleChassisSlotIndex;
		}
	}
	
	{		
		for (int row = 0; row < rowCount; ++row)
		{
			ShipChassisWritable const * const shipChassisWritable = findShipChassisWritableForRow(row);			
			if (shipChassisWritable)
			{
				int slotBitIndex = 0;

				for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_explicit_types); ++chassisSlotType)
				{
					if (!slotsForVisibleChassis.testBit(chassisSlotType))
						continue;

					ShipChassisSlot const * const chassisSlot = shipChassisWritable->ShipChassis::getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
					int const colForSlot = (slotBitIndex * 3) + slotColsStartingRow;
					
					++slotBitIndex;

					if (NULL == chassisSlot)
					{
						m_table->setItem(row, colForSlot, new MyCompatibilityTableItem(m_table, QString()));
						m_table->setItem(row, colForSlot + 1, new MyDisabledTableItem(m_table, QString()));
						m_table->setItem(row, colForSlot + 2, new MyDisabledTableItem(m_table, QString()));
						continue;
					}

					m_table->setItem(row, colForSlot, new MyCompatibilityTableItem(m_table, QString(chassisSlot->getCompatibilityString().c_str())));

					IGNORE_RETURN(snprintf(buf, buf_size, "%d", chassisSlot->getHitWeight()));
					m_table->setText(row, colForSlot + 1, QString(buf));
					m_table->setItem(row, colForSlot + 2, new MyTargetableTableItem(m_table, QString(chassisSlot->isTargetable() ? "X": "")));

					m_table->setColumnWidth(colForSlot + 1, 32);
					m_table->setColumnWidth(colForSlot + 2, 32);
				}
			}
		}
	}

	m_table->show();
}

//----------------------------------------------------------------------

ShipChassisWritable * ChassisEditorList::findShipChassisWritableForRow(int const row)
{
	QHeader * const headerVertical = NON_NULL(m_table->verticalHeader());
	std::string const & chassisName = QStringUtil::toString(headerVertical->label(row));
	if (!chassisName.empty())
	{
		ShipChassis const * const chassis = ShipChassis::findShipChassisByName(ConstCharCrcString(chassisName.c_str()));
		ShipChassisWritable const * const chassisWritable = safe_cast<ShipChassisWritable const *>(chassis);
		return const_cast<ShipChassisWritable *>(chassisWritable);
	}

	return NULL;
} //lint !e1762 //const

//----------------------------------------------------------------------

ShipChassisWritable * ChassisEditorList::findShipChassisWritableLastSelected()
{
	int const numSelections = m_table->numSelections();
	if (numSelections > 0)
	{
		QTableSelection const & selection = m_table->selection(0);
		
		int const row = selection.anchorRow();
		
		return findShipChassisWritableForRow(row);
	}

	return NULL;
}

//----------------------------------------------------------------------

int ChassisEditorList::findChassisSlotTypeForColumn(int col) const
{
	//-- flyby column
	if (col < TC_firstSlot)
		return static_cast<int>(ShipChassisSlotType::SCST_invalid);

	int const slotCol = col - TC_firstSlot;
	int const slotCompatsCol  = TC_firstSlot + slotCol - (slotCol % 3);
	
	QHeader * const headerHorizontal = NON_NULL(m_table->horizontalHeader());
	char const * const slotName = headerHorizontal->label(slotCompatsCol).latin1();
	std::string const slotNameStr = (NULL != slotName) ? slotName : "";
	
	int const chassisSlotType = static_cast<int>(ShipChassisSlotType::getTypeFromName(slotNameStr));

	return chassisSlotType;
} 

//----------------------------------------------------------------------

void ChassisEditorList::setTableNeedsReset()
{
	m_tableNeedsReset = true;
	m_tableResetCountdown = 250;
}

//----------------------------------------------------------------------

QTable * ChassisEditorList::getTable()
{
	return m_table;
}

//----------------------------------------------------------------------

void ChassisEditorList::getSelectedChassisList(StringVector & result)
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
				ShipChassisWritable * const shipChassis = findShipChassisWritableForRow(row);
				if (NULL == shipChassis)
				{
					WARNING(true, ("ChassisEditorDetails invalid chassis on row [%d]", row));
					continue;
				}
				std::string const & chassisType = shipChassis->getName().getString();
				result.push_back(chassisType);
			}
		}
	}
	else
	{
		int const currentRow = m_table->currentRow();
		ShipChassisWritable * const shipChassis = findShipChassisWritableForRow(currentRow);
		if (NULL == shipChassis)
		{
			WARNING(true, ("ChassisEditorDetails invalid chassis on row [%d]", currentRow));
		}
		else
		{
			std::string const & chassisType = shipChassis->getName().getString();
			result.push_back(chassisType);
		}
	}
}

//======================================================================
