//======================================================================
//
// ComponentDescriptorsList.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "ComponentDescriptorsList.h"
#include "ComponentDescriptorsList.moc"

#include "ChassisNewDialog.h"
#include "CompatibilityEditor.h"
#include "ConfigShipComponentEditor.h"
#include "QStringUtil.h"
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
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentDescriptorWritable.h"
#include "sharedMath/Vector.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/ObjectTemplateList.h"
#include <vector>

//======================================================================

namespace ComponentDescriptorsListNamespace
{
	//----------------------------------------------------------------------

	bool checkComponentNameValidChars(std::string const & name)
	{
		if (std::string::npos != name.find_first_of(" \t\n\r.;,-=+"))
			return false;

		return true;
	}

	//----------------------------------------------------------------------

	bool checkComponentNamePrefix(int componentType, std::string const & name)
	{	
		std::string const & prefix = ShipComponentType::getCanonicalPrefixForType(static_cast<ShipComponentType::Type>(componentType));
		//-- make sure string has proper prefix
		if (0 != name.find(prefix))
		{
			return false;
		}
		
		return true;
	}

	//----------------------------------------------------------------------

	class MyTemplateTableItem : public QTableItem
	{
	public:
		void paint(QPainter* p,const QColorGroup & cg,const QRect & cr,bool selected)
		{
			QColorGroup g (cg);
			
			ShipComponentEditorServerTemplate const * const st = ShipComponentEditorServerTemplateManager::findTemplateByName(m_objectTemplateName.c_str());
			
			std::string const & templateName = QStringUtil::toString(text());
			
			//-- this is a shared template, check existance and the server-shared linkage
			if (m_isShared)
			{
				g.setColor(QColorGroup::Background , g.midlight());
				g.setColor(QColorGroup::Base , g.midlight());

				if (ObjectTemplateList::lookUp(templateName.c_str()).isEmpty())
				{
					g.setColor(QColorGroup::Text, "red");
					g.setColor(QColorGroup::HighlightedText, "red");
				}
				else if (NULL != st)
				{
					//-- shared template does not exist
					if (ObjectTemplateList::lookUp(st->sharedTemplateName.c_str()).isEmpty())
					{
						g.setColor(QColorGroup::Text, "red");
						g.setColor(QColorGroup::HighlightedText, "red");
					}
					//-- templates don't match up
					else if (st->sharedTemplateName != templateName)
					{
						g.setColor(QColorGroup::Text, "violet");
						g.setColor(QColorGroup::HighlightedText, "violet");
					}				
				}
			}
			else
			{
				if (NULL == st)
				{
					g.setColor(QColorGroup::Text, "red");
					g.setColor(QColorGroup::HighlightedText, "red");
				}
			}
			
			QTableItem::paint( p , g , cr , selected );
		}
		
		MyTemplateTableItem (QTable * const table, EditType et, std::string const & objectTemplateName, std::string const & sharedObjectTemplateName) :
		QTableItem(table, et, sharedObjectTemplateName.empty() ? objectTemplateName.c_str() : sharedObjectTemplateName.c_str()),
			m_objectTemplateName(objectTemplateName),
			m_isShared(!sharedObjectTemplateName.empty())
		{
		}
	private:
		std::string m_objectTemplateName;
		bool m_isShared;
	};

	//----------------------------------------------------------------------

	class MyNameTableItem : public QTableItem
	{
	public:
		void paint(QPainter* p,const QColorGroup & cg,const QRect & cr,bool selected)
		{
			std::string const & name = QStringUtil::toString(text());

			ShipComponentDescriptorWritable const * const shipComponentDescriptor = static_cast<ShipComponentDescriptorWritable const *>(ShipComponentDescriptor::findShipComponentDescriptorByCrc(m_componentDescriptorCrc));

			QColorGroup g (cg);

			if (NULL == shipComponentDescriptor)
			{
				g.setColor(QColorGroup::Text, "violet");
				g.setColor(QColorGroup::HighlightedText, "violet");
			}
			else
			{
				bool ok = true;

				if (!checkComponentNamePrefix(shipComponentDescriptor->getComponentType(), name))
				{
					ok = false;
				}
				//-- make sure string has no garbage in it
				else if (!checkComponentNameValidChars(name))
				{
					ok = false;
				}

				if (!ok)
				{
					g.setColor(QColorGroup::Text, "red");
					g.setColor(QColorGroup::HighlightedText, "red");
				}
			}
			
			QTableItem::paint( p , g , cr , selected );
		}
		
		MyNameTableItem(QTable * const table, EditType et, QString const & text, uint32 const componentDescriptorCrc) : 
		QTableItem(table, et, text),
		m_componentDescriptorCrc(componentDescriptorCrc)
		{
		}
	private:
		uint32 m_componentDescriptorCrc;
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
		
		MyDisabledTableItem(QTable * const table, EditType et, QString const & text) : 
		QTableItem(table, et, text)
		{
		}
	};

	//----------------------------------------------------------------------

	class MyTargetableTableItem : public QTableItem
	{
	public:
		void paint(QPainter* p,const QColorGroup & cg,const QRect & cr,bool selected)
		{
			QTableItem::paint( p , cg , cr , selected );
		}
		
		MyTargetableTableItem(QTable * const table, QString const & text) : 
		QTableItem(table, QTableItem::Never, text)
		{
		}

		virtual int alignment() const
		{
			return Qt::AlignCenter;
		}
	};

	//----------------------------------------------------------------------

	enum TableCols
	{
		TC_name = 0,
		TC_template = 1,
		TC_sharedTemplate = 2,
		TC_compatibility = 3,
		TC_type = 4,
		TC_numColumns = 5
	};

	enum ContextMenuItems
	{
		CMI_template,
		CMI_templateNew,
		CMI_sharedTemplateFix,
		CMI_delete,
		CMI_new,
		CMI_clone

	};

	char const * const s_contextMenuItemNames[] =
	{
		"Choose Template",
		"Assign to New Template",
		"Fix Shared Template",
		"Delete Component",
		"New Component",
		"Clone Component"
	};
}

using namespace ComponentDescriptorsListNamespace;

//----------------------------------------------------------------------

ComponentDescriptorsList::ComponentDescriptorsList(QWidget *parent, char const *name) :
BaseComponentDescriptorsList(parent, name),
m_tableNeedsReset(true),
m_tableResetCountdown(0),
m_callback(new MessageDispatch::Callback),
m_shouldAdjustColumns(true)
{	
	connect(m_table, SIGNAL(doubleClicked(int, int, int, const QPoint & )), SLOT(onTableDoubleClicked(int, int, int, const QPoint &)));
	connect(m_table, SIGNAL(valueChanged(int, int)), SLOT(onTableValueChanged(int, int))); 
	connect(m_table, SIGNAL(contextMenuRequested ( int , int , const QPoint &  )), SLOT(onTableContextMenuRequested ( int , int , const QPoint &  )));
	connect(m_table, SIGNAL(selectionChanged ()), SLOT(onTableSelectionChanged ()));
	
	resetTable();

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(onTimerUpdate()));
	timer->start(100);

	m_callback->connect(*this, &ComponentDescriptorsList::onCallbackComponentListChanged, static_cast<ShipComponentDescriptorWritable::Messages::ComponentListChanged *>(NULL));
	m_callback->connect(*this, &ComponentDescriptorsList::onCallbackComponentChanged, static_cast<ShipComponentDescriptorWritable::Messages::ComponentChanged *>(NULL));

//	m_table->setSorting(true);
}

//----------------------------------------------------------------------

ComponentDescriptorsList::~ComponentDescriptorsList()
{
	delete m_callback;
	m_callback = NULL;
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void ComponentDescriptorsList::onTimerUpdate()
{
	if(m_tableNeedsReset)
	{
		m_tableResetCountdown -= 100;
		if (m_tableResetCountdown <= 0)
			resetTable();
	}
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::onLineEditNameFiltertextChanged(QString const & text)
{
	m_nameFilter = Unicode::toLower(QStringUtil::toString(text)).c_str();
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::onTableDoubleClicked(int row, int col, int , const QPoint & )
{
	ShipComponentDescriptorWritable * const componentDescriptorWritable = findComponentDescriptorWritableForRow(row);
	if (NULL == componentDescriptorWritable)
	{
		WARNING(true, ("ComponentDescriptorsList failed to find component descriptor for row [%d]", row));
		return;
	}
	
	std::string const & textStr = QStringUtil::toString(m_table->text(row, col));
	UNREF(textStr);

	if (col == TC_name)
	{
	}
	else if (col == TC_template)
	{
	}
	else if (col == TC_sharedTemplate)
	{
	}
	else if (col == TC_compatibility)
	{
		CompatibilityEditor::StringVector sv;
		std::string const componentCompatibility(componentDescriptorWritable->getCompatibility().getString());
		if (!componentCompatibility.empty())
			sv.push_back(componentCompatibility);

		CompatibilityEditor * const ce = new CompatibilityEditor(this, "compat_editor", CompatibilityEditor::M_single, sv);
		std::string finalSelection;
		if (ce->showAndTell(finalSelection))
		{
			componentDescriptorWritable->setCompatibility(finalSelection);
		}
	}
	else if (col == TC_type)
	{
	}
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::onTableValueChanged (int row,int col)
{
	ShipComponentDescriptorWritable * const componentDescriptorWritable = findComponentDescriptorWritableForRow(row);
	if (NULL == componentDescriptorWritable)
	{
		WARNING(true, ("ComponentDescriptorsList failed to find component descriptor for row [%d]", row));
		return;
	}
	
	std::string const & textStr = QStringUtil::toString(m_table->text(row, col));
	
	if (col == TC_name)
	{
		//-- setting back to the original name, ignore it
		if (textStr == componentDescriptorWritable->getName().getString())
			return;

		if (!checkComponentNameValidChars(textStr))
		{
			QMessageBox::warning(NULL, "Invalid Component Name", "The name contains invalid characters.");
		}

		else if (!checkComponentNamePrefix(componentDescriptorWritable->getComponentType(), textStr))
		{
			QMessageBox::warning(NULL, "Invalid Component Name", "The name has an invalid prefix.");
		}

		else if (!componentDescriptorWritable->setName(textStr))
		{
			QMessageBox::warning(NULL, "Invalid Component Name", "The name is already in use.");
		}
		else
		{
			QHeader * const headerVertical = NON_NULL(m_table->verticalHeader());
			headerVertical->setLabel(row, QString(componentDescriptorWritable->getName().getString()));
			return;
		}
	}
	else if (col == TC_template)
	{
	}
	else if (col == TC_sharedTemplate)
	{
	}
	else if (col == TC_compatibility)
	{
	}
	else if (col == TC_type)
	{
		ShipComponentType::Type const type = ShipComponentType::getTypeFromName(textStr);
		if (ShipComponentType::SCT_num_types != type)
		{
			componentDescriptorWritable->setComponentType(type);
			ShipComponentAttachmentManager::udpateComponentCrc(componentDescriptorWritable->getCrc(), 0);
		}
	}

	//-- fallthrough means an error occured
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::onTableContextMenuRequested( int , int , const QPoint & pos)
{	
	int totalRowsSelected = 0;

	int const numSelections = m_table->numSelections();
	for (int s = 0; s < numSelections; ++s)
	{
		QTableSelection const & selection = m_table->selection(s);
		
		totalRowsSelected += selection.numRows();

		int const colRight = selection.rightCol();
		for (int col = selection.leftCol(); col <= colRight; ++col)
		{
		}
	}

	QPopupMenu * const pop = new QPopupMenu(m_table);
	
	pop->insertItem(s_contextMenuItemNames[CMI_template], this, SLOT(onTableContextTemplate(int)), 0, CMI_template);
	pop->insertItem(s_contextMenuItemNames[CMI_templateNew], this, SLOT(onTableContextTemplate(int)), 0, CMI_templateNew);
	pop->insertItem(s_contextMenuItemNames[CMI_sharedTemplateFix], this, SLOT(onTableContextTemplateFix(int)), 0, CMI_sharedTemplateFix);

	pop->insertSeparator();
	pop->insertItem(s_contextMenuItemNames[CMI_new], this, SLOT(onTableContextComponentNew(int)), 0, CMI_new);
	pop->insertItem(s_contextMenuItemNames[CMI_clone], this, SLOT(onTableContextComponentNew(int)), 0, CMI_clone);

	pop->insertSeparator();
	pop->insertItem(s_contextMenuItemNames[CMI_delete], this, SLOT(onTableContextComponentDelete(int)), 0, CMI_delete);

	if (totalRowsSelected > 1)
	{
		pop->setItemEnabled(CMI_template, false);
		pop->setItemEnabled(CMI_templateNew, false);
	}
	
	pop->popup(pos);
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::onTableContextTemplate(int id)
{
	ShipComponentDescriptorWritable * const shipComponentDescriptorWritable = findComponentDescriptorWritableLastSelected();
	NOT_NULL(shipComponentDescriptorWritable);
	if (NULL == shipComponentDescriptorWritable)
	{
		return;
	}
		
	if (CMI_template == id)
	{
		std::string const & pathPrefix = ConfigShipComponentEditor::getServerPathData() + "/";
		std::string path = pathPrefix;
		if (!shipComponentDescriptorWritable->getObjectTemplateName().empty())
			path += shipComponentDescriptorWritable->getObjectTemplateName();
		else
			path += std::string("object/tangible/ship/components/*.*");

		QString const & fileSelection = QFileDialog::getOpenFileName(QString(path.c_str()), QString("*.iff"));
		if (fileSelection.isEmpty())
			return;
		
		std::string fileSelectionStr = Unicode::toLower(QStringUtil::toString(fileSelection));
		
		if (!_strnicmp(pathPrefix.c_str(), fileSelectionStr.c_str(), pathPrefix.size()))
			fileSelectionStr = fileSelectionStr.substr(pathPrefix.size());

		ShipComponentEditorServerTemplate const * const shipComponentEditorServerTemplate = ShipComponentEditorServerTemplateManager::findTemplateByName(fileSelectionStr);
		if (NULL == shipComponentEditorServerTemplate)
		{
			QMessageBox::warning(NULL, "Invalid Template", "Could not find template in template database."); 
			return;
		}
		if (!shipComponentEditorServerTemplate->chassisType.empty())
		{
			QMessageBox::warning(NULL, "Invalid Template", "Selected template is a ship template and not valid for use as a component."); 
			return;
		}
		if (shipComponentEditorServerTemplate->sharedTemplateName.empty())
		{
			QMessageBox::warning(NULL, "Invalid Template", "Selected template has an empty shared template."); 
			return;
		}

		if (!shipComponentDescriptorWritable->setObjectTemplateByName(fileSelectionStr, shipComponentEditorServerTemplate->sharedTemplateName))
		{
			QMessageBox::warning(NULL, "Invalid Template", "An error occured while assigning the template to the component."); 
			return;
			
		}

		setTableNeedsReset();
	}
	else if (CMI_templateNew == id)
	{
		TemplateNewDialog * const tnd = new TemplateNewDialog(this, "Create new Component Template", shipComponentDescriptorWritable->getObjectTemplateName());
		std::string const & templateName = tnd->showAndTell();
		
		if (!templateName.empty())
		{
			//-- regenerate the templates
			ShipComponentEditorServerTemplateManager::regenerateTemplateDb();

			ShipComponentEditorServerTemplate const * const shipComponentEditorServerTemplate = ShipComponentEditorServerTemplateManager::findTemplateByName(templateName);
			if (NULL == shipComponentEditorServerTemplate)
			{
				QMessageBox::warning(NULL, "Invalid Template", "Could not find new template in template database."); 
				return;
			}

			if (!shipComponentDescriptorWritable->setObjectTemplateByName(templateName, shipComponentEditorServerTemplate->sharedTemplateName))
			{
				QMessageBox::warning(NULL, "Invalid Template", "An error occured while assigning the new template(s) to the component."); 
				return;
			}
		}
	}
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::onTableContextTemplateFix(int)
{
	int const numSelections = m_table->numSelections();
	for (int s = 0; s < numSelections; ++s)
	{
		QTableSelection const & selection = m_table->selection(s);
		
		int const rowTop = selection.topRow();
		for (int row = selection.bottomRow(); row <= rowTop; ++row)
		{
			ShipComponentDescriptorWritable * const shipComponentDescriptorWritable = findComponentDescriptorWritableForRow(row);
			NOT_NULL(shipComponentDescriptorWritable);
			
			std::string const & templateName = shipComponentDescriptorWritable->getObjectTemplateName(); 
			
			ShipComponentEditorServerTemplate const * const shipComponentEditorServerTemplate = ShipComponentEditorServerTemplateManager::findTemplateByName(templateName);
			if (NULL != shipComponentEditorServerTemplate)
			{
				std::string const & sharedTemplateName = shipComponentDescriptorWritable->getSharedTemplateName();
				
				if (sharedTemplateName != shipComponentEditorServerTemplate->sharedTemplateName)
				{
					if (!shipComponentDescriptorWritable->setObjectTemplateByName(templateName, shipComponentEditorServerTemplate->sharedTemplateName))
					{
						QMessageBox::warning(NULL, "Invalid Template", "An error occured while assigning the new template(s) to the component."); 
						return;
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::onTableContextComponentNew(int id)
{
	int componentType = static_cast<int>(ShipComponentType::SCT_reactor);
	uint32 oldCrc = 0;
	std::string compatibilites;

	char buf[1024];
	size_t const buf_size = sizeof(buf);
	snprintf(buf, buf_size, "_tmp_%x", time(0));

	switch (id)
	{
	case CMI_new:
		{
			componentType = static_cast<int>(ShipComponentType::SCT_reactor);
		}
		break;
	case CMI_clone:
		{
			ShipComponentDescriptorWritable * shipDescriptorWritableSelected = findComponentDescriptorWritableLastSelected();
			UNREF(shipDescriptorWritableSelected);
			componentType = static_cast<int>(shipDescriptorWritableSelected->getComponentType());
			oldCrc = shipDescriptorWritableSelected->getCrc();
			compatibilites = shipDescriptorWritableSelected->getCompatibility().getString();
			strcat(buf, "_");
			strcat(buf, shipDescriptorWritableSelected->getName().getString());
		}
		break;
	}

	ShipComponentDescriptorWritable * const shipComponentDescriptorWritableNew = new ShipComponentDescriptorWritable(ConstCharCrcString(buf), static_cast<ShipComponentType::Type>(componentType), ConstCharCrcString(compatibilites.c_str()), std::string(), std::string());
	if (!shipComponentDescriptorWritableNew->addShipComponentDescriptor(true, true))
	{
		QMessageBox::warning(NULL, "Invalid Component", "Could not add new component to table.  Check DebugView warnings for more details.");
		delete shipComponentDescriptorWritableNew;
	}

	if (CMI_clone == id)
	{
		ShipComponentAttachmentManager::copyAttachmentsForComponent(oldCrc, shipComponentDescriptorWritableNew->getCrc());
	}
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::onTableContextComponentDelete(int )
{
	ComponentDescriptorsList::StringVector selectedComponentsList;
	getSelectedComponentsList(selectedComponentsList);
	
	char buf[1024];
	size_t const buf_size = sizeof(buf);	
	
	bool yesAll = false;
	
	int count = 0;
	for (StringVector::const_iterator it = selectedComponentsList.begin(); it != selectedComponentsList.end(); ++it, ++count)
	{
		std::string const & componentName = *it;
		
		bool ok = false;
		
		if (!yesAll)
		{
			snprintf(buf, buf_size, "REALLY DELETE component [%s]?", componentName.c_str());
			
			int result = 0;
			if ((count + 1) < static_cast<int>(selectedComponentsList.size()))
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
		
		if (ok)
		{
			ShipComponentDescriptorWritable * const shipComponentDescriptorWritable = 
				const_cast<ShipComponentDescriptorWritable *>(safe_cast<ShipComponentDescriptorWritable const *>(ShipComponentDescriptor::findShipComponentDescriptorByName(ConstCharCrcString(componentName.c_str()))));
			NOT_NULL(shipComponentDescriptorWritable);

			shipComponentDescriptorWritable->removeShipComponentDescriptor();
		}
	}
	
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::onTableSelectionChanged()
{
	//resetTemplateList();
}


//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- CALLBACKS
//----------------------------------------------------------------------

void ComponentDescriptorsList::onCallbackComponentChanged(ShipComponentDescriptorWritable const & )
{
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::onCallbackComponentListChanged(bool const & )
{
	setTableNeedsReset();
}

//----------------------------------------------------------------------
//-- End CALLBACKS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//--
//----------------------------------------------------------------------

void ComponentDescriptorsList::resetTable()
{
	m_tableNeedsReset = false;
//	m_table->clear();

	ShipComponentDescriptor::StringVector const componentNames = ShipComponentDescriptor::getComponentDescriptorNames();

//	bool showAllColumns = false;
	
	BitArray slotsForVisibleChassis(static_cast<int>(ShipChassisSlotType::SCST_num_types));
	
	m_table->setNumRows(componentNames.size());
	m_table->setNumCols(TC_numColumns);

	m_table->hide();
	
	QStringList componentTypeList;
//	componentTypeList.reserve(static_cast<int>(ShipComponentType::SCT_num_types));
	{
		for (int i = 0; i < static_cast<int>(ShipComponentType::SCT_num_types); ++i)
			componentTypeList.push_back(ShipComponentType::getNameFromType(static_cast<ShipComponentType::Type>(i)).c_str());
	}

	{
		QHeader * const headerVertical = NON_NULL(m_table->verticalHeader());
		
		int rowCount = 0;
		for (ShipComponentDescriptor::StringVector::const_iterator it = componentNames.begin(); it != componentNames.end(); ++it)
		{
			std::string const & componentName = *it;

			if (!m_nameFilter.empty())
			{
				if (std::string::npos == componentName.find(m_nameFilter))
					continue;
			}

			ShipComponentDescriptorWritable const * const shipComponentDescriptor = static_cast<ShipComponentDescriptorWritable const *>(ShipComponentDescriptor::findShipComponentDescriptorByName(PersistentCrcString(componentName.c_str(), true)));
			
			NOT_NULL(shipComponentDescriptor);

			if (!m_filterComponentTypes.empty())
			{
				if (!std::binary_search(m_filterComponentTypes.begin(), m_filterComponentTypes.end(), static_cast<int>(shipComponentDescriptor->getComponentType())))
					continue;
			}

			if (!m_filterChassis.empty())
			{
				bool ok = false;
				
				for (CrcVector::const_iterator cit = m_filterChassis.begin(); cit != m_filterChassis.end(); ++cit)
				{
					uint32 const chassisCrc = *cit;
					ShipChassisWritable const * const shipChassisWritable = static_cast<ShipChassisWritable const *>(ShipChassis::findShipChassisByCrc(chassisCrc));
					if (NULL == shipChassisWritable)
						continue;
					
					if (shipChassisWritable->canAcceptComponent(*shipComponentDescriptor))
					{
						ok = true;
						break;
					}
				}

				if (!ok)
					continue;
			}
			
			std::string const & objectTemplateName = shipComponentDescriptor->getObjectTemplateName();

//			uint32 const objectTemplateCrc = shipComponentDescriptor->getObjectTemplateCrc();

			ShipComponentEditorServerTemplate const * const st = ShipComponentEditorServerTemplateManager::findTemplateByName(objectTemplateName);

			m_table->setItem(rowCount, TC_name, new MyNameTableItem(m_table, QTableItem::OnTyping, QString(componentName.c_str()), shipComponentDescriptor->getCrc()));

			UNREF(st);
			m_table->setItem(rowCount, TC_template, new MyTemplateTableItem(m_table, QTableItem::Never, objectTemplateName, std::string()));

			ConstCharCrcString const & sharedOjectTemplateName = ObjectTemplateList::lookUp(shipComponentDescriptor->getSharedObjectTemplateCrc());

			m_table->setItem(rowCount, TC_sharedTemplate, new MyTemplateTableItem(m_table, QTableItem::Never, objectTemplateName, std::string(sharedOjectTemplateName.getString())));

			m_table->setItem(rowCount, TC_compatibility, new QTableItem(m_table, QTableItem::Never, QString(shipComponentDescriptor->getCompatibility().getString())));

//			m_table->setText(rowCount, TC_type, QString(ShipComponentType::getNameFromType(shipComponentDescriptor->getComponentType()).c_str()));

			QComboTableItem * const comboItem = new QComboTableItem(m_table, componentTypeList, true);
			comboItem->setCurrentItem(shipComponentDescriptor->getComponentType());
			m_table->setItem(rowCount, TC_type, comboItem);

			headerVertical->setLabel(rowCount, QString(componentName.c_str()));

			++rowCount;
		}
		m_table->setNumRows(rowCount);
	}

	if (m_shouldAdjustColumns)
	//-- setup the column header
	{
		QHeader * const headerHorizontal = NON_NULL(m_table->horizontalHeader());
		headerHorizontal->setLabel(TC_name, "Name");
		headerHorizontal->setLabel(TC_template, "Template");
		headerHorizontal->setLabel(TC_sharedTemplate, "Shared Template");
		headerHorizontal->setLabel(TC_compatibility, "Compatibility");
		headerHorizontal->setLabel(TC_type, "Type");

		m_table->adjustColumn(TC_name);
		m_table->adjustColumn(TC_type);
		m_shouldAdjustColumns = false;
	}	

	m_table->show();
}

//----------------------------------------------------------------------

ShipComponentDescriptorWritable * ComponentDescriptorsList::findComponentDescriptorWritableForRow(int const row)
{
	QHeader * const headerVertical = NON_NULL(m_table->verticalHeader());
	std::string const & componentName = QStringUtil::toString(headerVertical->label(row));

	if (!componentName.empty())
	{
		ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByName(ConstCharCrcString(componentName.c_str()));
		ShipComponentDescriptorWritable const * const shipComponentDescriptorWritable = static_cast<ShipComponentDescriptorWritable const *>(shipComponentDescriptor);
		return const_cast<ShipComponentDescriptorWritable *>(shipComponentDescriptorWritable);
	}

	return NULL;
}

//----------------------------------------------------------------------

ShipComponentDescriptorWritable * ComponentDescriptorsList::findComponentDescriptorWritableLastSelected()
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

void ComponentDescriptorsList::setTableNeedsReset()
{
	m_tableNeedsReset = true;
	m_tableResetCountdown = 250;
}

//----------------------------------------------------------------------

QTable * ComponentDescriptorsList::getTable()
{
	return m_table;
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::getSelectedComponentsList(StringVector & result)
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

void ComponentDescriptorsList::handleComponentTypeFilterChange(IntVector const & componentTypes)
{
	m_filterComponentTypes = componentTypes;
	std::sort(m_filterComponentTypes.begin(), m_filterComponentTypes.end());
	setTableNeedsReset();
}

//----------------------------------------------------------------------

void ComponentDescriptorsList::handleChassisFilterChange(CrcVector const & chassisCrcs)
{
	m_filterChassis = chassisCrcs;
	std::sort(m_filterChassis.begin(), m_filterChassis.end());
	setTableNeedsReset();
}

//======================================================================
