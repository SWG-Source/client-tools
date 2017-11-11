//======================================================================
//
// ChassisEditorDetails.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "ChassisEditorDetails.h"
#include "ChassisEditorDetails.moc"

#include "ChassisEditor.h"
#include "ConfigShipComponentEditor.h"
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
#include "sharedMath/Vector.h"
#include "sharedMessageDispatch/Transceiver.h"
#include <qtabwidget.h>
#include <vector>

//======================================================================

namespace ChassisEditorDetailsNamespace
{
	enum ContextMenuItems
	{
		CMI_editTemplate,
		CMI_editSharedTemplate,
		CMI_jumpToP4,
		CMI_jumpToP4SharedTemplate,
		CMI_newTemplate
	};

	char const * const s_contextMenuItemNames[] =
	{
		"Edit Template",
		"Edit Shared Template",
		"Jump to P4",
		"Jump to P4 Shared Template",
		"New Template",
	};
}
using namespace ChassisEditorDetailsNamespace;

//----------------------------------------------------------------------

ChassisEditorDetails::ChassisEditorDetails(QWidget *parent, char const *name, ChassisEditor & editor) :
BaseChassisEditorDetails(parent, name),
m_chassisEditor(&editor),
m_callback(new MessageDispatch::Callback)
{
	connect(m_lineEditNameFilter, SIGNAL(textChanged(const QString &)), SLOT(onLineEditNameFiltertextChanged(const QString &)));
	connect(m_listTemplates, SIGNAL(contextMenuRequested (QListBoxItem *, const QPoint &)), SLOT(onListContextMenuRequested (QListBoxItem *, const QPoint &)));
	connect(m_listTemplatesOrphaned, SIGNAL(contextMenuRequested (QListBoxItem *, const QPoint &)), SLOT(onListContextMenuRequested (QListBoxItem *, const QPoint &)));
	connect(m_buttonReloadTemplates, SIGNAL(clicked ()), SLOT(onButtonReloadTemplatesClicked ()));
	connect(m_buttonRegenerateCrcDb, SIGNAL(clicked ()), SLOT(onButtonRegenerateDbClicked ()));

	resetTemplateList();
	resetTemplateOrphanList();

	m_callback->connect(*this, &ChassisEditorDetails::onCallbackChassisListChanged, static_cast<ShipChassisWritable::Messages::ChassisListChanged *>(NULL));
	m_callback->connect(*this, &ChassisEditorDetails::onCallbackTemplateListChanged, static_cast<ShipComponentEditorServerTemplateManager::Messages::TemplateListChanged *>(NULL));

}

//----------------------------------------------------------------------

ChassisEditorDetails::~ChassisEditorDetails()
{
	delete m_callback;
	m_callback = NULL;
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void ChassisEditorDetails::onLineEditNameFiltertextChanged(QString const & text)
{
	m_lineEditNameFilter->setText(Unicode::toLower(QStringUtil::toString(text)).c_str());
}
//----------------------------------------------------------------------

void ChassisEditorDetails::onTableSelectionChanged()
{
	resetTemplateList();
}
//----------------------------------------------------------------------

void ChassisEditorDetails::onListContextMenuRequested(QListBoxItem * item, const QPoint & pos)
{
	UNREF(item);

	std::string templateName;
	if (m_tabs->currentPageIndex() == 0)
		templateName = QStringUtil::toString(m_listTemplates->currentText());
	else
		templateName = QStringUtil::toString(m_listTemplatesOrphaned->currentText());

	//-- cut the list entry at the space character.  we ignore the additional info that may be appended to the list entry
	{
		size_t const spacepos = templateName.find(' ');
		if (spacepos != std::string::npos)
			templateName = templateName.substr(0, spacepos);
	}

	QPopupMenu * const pop = new QPopupMenu(NULL);
	
	pop->insertItem(s_contextMenuItemNames[CMI_editTemplate], this, SLOT(onListBoxContext(int)), 0, CMI_editTemplate);
	pop->insertItem(s_contextMenuItemNames[CMI_editSharedTemplate], this, SLOT(onListBoxContext(int)), 0, CMI_editSharedTemplate);
	pop->insertSeparator();
	pop->insertItem(s_contextMenuItemNames[CMI_jumpToP4], this, SLOT(onListBoxContext(int)), 0, CMI_jumpToP4);
	pop->insertItem(s_contextMenuItemNames[CMI_jumpToP4SharedTemplate], this, SLOT(onListBoxContext(int)), 0, CMI_jumpToP4SharedTemplate);

	if (item->listBox() == m_listTemplates)
	{
		pop->insertSeparator();
		pop->insertItem(s_contextMenuItemNames[CMI_newTemplate], this, SLOT(onListBoxContext(int)), 0, CMI_newTemplate);
	}
	
	pop->popup(pos);

	ShipComponentEditorServerTemplate const * const st = ShipComponentEditorServerTemplateManager::findTemplateByName(templateName);
	if (NULL == st)
	{
		pop->setItemEnabled(CMI_editTemplate, false);
		pop->setItemEnabled(CMI_editSharedTemplate, false);
		pop->setItemEnabled(CMI_jumpToP4, false);
		pop->setItemEnabled(CMI_jumpToP4SharedTemplate, false);
	}
}

//----------------------------------------------------------------------

void ChassisEditorDetails::onListBoxContext(int id)
{
	std::string templateName;
	if (m_tabs->currentPageIndex() == 0)
		templateName = QStringUtil::toString(m_listTemplates->currentText());
	else
		templateName = QStringUtil::toString(m_listTemplatesOrphaned->currentText());

	//-- cut the list entry at the space character.  we ignore the additional info that may be appended to the list entry
	{
		size_t const spacepos = templateName.find(' ');
		if (spacepos != std::string::npos)
			templateName = templateName.substr(0, spacepos);
	}

	ShipComponentEditorServerTemplate const * const st = ShipComponentEditorServerTemplateManager::findTemplateByName(templateName);

	char buf[1024];
	size_t const buf_size = sizeof(buf);

//	std::string const & dsrcPath = ShipComponentEditorServerTemplateManager::getServerTemplatePathDsrc();

	switch(id)
	{
		case CMI_editTemplate:
			if (NULL != st)
			{
				snprintf(buf, buf_size, "start %s %s", ConfigShipComponentEditor::getTextEditorPath().c_str(), st->fullPath.c_str());
				system(buf);
			}
			break;
		case CMI_editSharedTemplate:
			if (NULL != st)
			{
				snprintf(buf, buf_size, "start %s %s", ConfigShipComponentEditor::getTextEditorPath().c_str(), st->fullPathSharedTemplate.c_str());
				system(buf);
			}
			break;
		case CMI_jumpToP4:
			if (NULL != st)
			{
				snprintf(buf, buf_size, "start p4win -s %s", st->fullPath.c_str());
				system(buf);
			}
			break;
		case CMI_jumpToP4SharedTemplate:
			if (NULL != st)
			{
				snprintf(buf, buf_size, "start p4win -s %s", st->fullPathSharedTemplate.c_str());
				system(buf);
			}
			break;
		case CMI_newTemplate:
			{
				std::string templateName;
				
				if (NULL != st)
					templateName = st->name;
				
				TemplateNewDialog * const tnd = new TemplateNewDialog(NULL, "fff", std::string(), templateName);
				tnd->show();
			}
			break;
	}
}

//----------------------------------------------------------------------

void ChassisEditorDetails::onButtonReloadTemplatesClicked()
{
	ShipComponentEditorServerTemplateManager::load();
}

//----------------------------------------------------------------------

void ChassisEditorDetails::onButtonRegenerateDbClicked()
{
	ShipComponentEditorServerTemplateManager::regenerateTemplateDb();
}

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- CALLBACKS
//----------------------------------------------------------------------

void ChassisEditorDetails::onCallbackChassisListChanged(bool const & )
{
	resetTemplateOrphanList();
}


//----------------------------------------------------------------------

void ChassisEditorDetails::onCallbackTemplateListChanged(bool const &)
{
	resetTemplateList();
}

//----------------------------------------------------------------------
//-- End CALLBACKS
//----------------------------------------------------------------------


void ChassisEditorDetails::resetTemplateList()
{
	m_listTemplates->clear();
		
	ChassisEditor::StringVector chassisNameList;
	m_chassisEditor->getSelectedChassisList(chassisNameList);

	ShipComponentEditorServerTemplateManager::ServerTemplateVector result;
	result.reserve(chassisNameList.size());
	
	{
		for (ChassisEditor::StringVector::const_iterator it = chassisNameList.begin(); it != chassisNameList.end(); ++it)
		{
			std::string const & chassisType = *it;
			ShipComponentEditorServerTemplateManager::findObjectTemplatesForChassisType(chassisType, result);
		}
	}
	
	std::sort(result.begin(), result.end());
	result.erase(std::unique(result.begin(), result.end()), result.end());

	{
		for (ShipComponentEditorServerTemplateManager::ServerTemplateVector::const_iterator it = result.begin(); it != result.end(); ++it)
		{
			ShipComponentEditorServerTemplate const & st = *it;
			m_listTemplates->insertItem(QString(st.name.c_str()));
		}
	}
}

//----------------------------------------------------------------------

void ChassisEditorDetails::resetTemplateOrphanList()
{
	m_listTemplatesOrphaned->clear();

	//-- get all the server ship templates
	ShipComponentEditorServerTemplateManager::ServerTemplateVector serverTemplateVector;
	ShipComponentEditorServerTemplateManager::findObjectTemplatesForChassisType(std::string(), serverTemplateVector);
	
	{
		for (ShipComponentEditorServerTemplateManager::ServerTemplateVector::const_iterator it = serverTemplateVector.begin(); it != serverTemplateVector.end(); ++it)
		{
			ShipComponentEditorServerTemplate const & st = *it;
			
			if (!st.chassisType.empty())
			{
				ShipChassis const * const chassis = ShipChassis::findShipChassisByName(PersistentCrcString(st.chassisType.c_str(), true));
				if (NULL == chassis)
				{
					std::string const & str = st.name + "        (" + st.chassisType + ")";
					m_listTemplatesOrphaned->insertItem(QString(str.c_str()));
				}
			}
		}
	}
}

//----------------------------------------------------------------------

QLineEdit * ChassisEditorDetails::getLineEditNameFilter()
{
	return m_lineEditNameFilter;
}

//======================================================================
