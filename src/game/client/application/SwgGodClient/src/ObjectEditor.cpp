// ======================================================================
//
// ObjectEditor.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ObjectEditor.h"
#include "ObjectEditor.moc"

#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/Object.h"
#include "sharedObject/World.h"

#include "clientGame/ClientObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"

#include "ActionHack.h"
#include "ActionsScript.h"
#include "ActionsSkill.h"
#include "GameWidget.h"
#include "GodClientData.h"
#include "MainFrame.h"
#include "ServerCommander.h"
#include "ServerObjectData.h"
#include "Singleton/Singleton.h"

#include "Unicode.h"
#include "UnicodeUtils.h"

#include <qdragobject.h>
#include <qinputdialog.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qmessagebox.h>

// ======================================================================

namespace ObjectEditorNamespace
{
	// List cleanup support
	void emptyListView(QListView * const list);
	void deleteListViewItem(QListView * const list, QListViewItem * const item);
};

using namespace ObjectEditorNamespace;

//-----------------------------------------------------------------

void ObjectEditorNamespace::emptyListView(QListView * const list)
{
	QListViewItem * item = list->firstChild();

	while(item)
	{
		deleteListViewItem(list, item);
		item = list->firstChild();
	}
}

//-----------------------------------------------------------------

void ObjectEditorNamespace::deleteListViewItem(QListView * const list, QListViewItem * const item)
{
	list->takeItem(item);
	delete item;
}

//-----------------------------------------------------------------

ObjectEditor::ObjectEditor(QWidget* theParent, const char* theName)
: BaseObjectEditor(theParent, theName),
  MessageDispatch::Receiver(),
	m_pmi(),
  m_obj(NULL),
	m_callback(NULL)
{
	setAcceptDrops(true);

	m_scriptsList->clear();
	m_objVarsList->clear();
	m_objVarsList->setSorting(-1);

	connectToMessage(Game::Messages::SCENE_CHANGED);
	connectToMessage(GodClientData::Messages::SELECTION_CHANGED);
	connectToMessage(GodClientData::Messages::SELECTED_OBJECTS_CHANGED);
	connectToMessage(ServerObjectData::Messages::OBJECT_INFO_CHANGED);

	m_attributesList->setSorting(-1);
	
	{
		m_pmi.client.generalItem                = new QListViewItem(m_attributesList, "General");
		m_pmi.client.transformItem              = new QListViewItem(m_attributesList, "Transform");
		m_pmi.client.generalItem->setSelectable(false);
		m_pmi.client.transformItem->setSelectable(false);
		
		{
			m_pmi.client.general.networkId        = new QListViewItem(m_pmi.client.generalItem, "NetworkId");
			m_pmi.client.general.authoritative    = new QListViewItem(m_pmi.client.generalItem, "Authoritative");
			m_pmi.client.general.watchable        = new QListViewItem(m_pmi.client.generalItem, "Watchable");
			m_pmi.client.general.active           = new QListViewItem(m_pmi.client.generalItem, "Active");
			m_pmi.client.general.appearance       = new QListViewItem(m_pmi.client.generalItem, "Appearance");
			m_pmi.client.general.objectTemplate   = new QListViewItem(m_pmi.client.generalItem, "ObjectTemplate");
			m_pmi.client.general.volume           = new QListViewItem(m_pmi.client.generalItem, "Volume");
			m_pmi.client.general.script           = new QListViewItem(m_pmi.client.generalItem, "Script");
			m_pmi.client.general.slotType         = new QListViewItem(m_pmi.client.generalItem, "SlotType");
			m_pmi.client.general.name             = new QListViewItem(m_pmi.client.generalItem, "Name");
			m_pmi.client.general.container        = new QListViewItem(m_pmi.client.generalItem, "Container");
		}

		{
			m_pmi.client.transform.translation      = new QListViewItem(m_pmi.client.transformItem, "Translation");
			m_pmi.client.transform.translation->setSelectable (false);
			m_pmi.client.transform.translateZ       = new QListViewItem(m_pmi.client.transform.translation, "Z");
			m_pmi.client.transform.translateZ->setRenameEnabled(1, true);
			m_pmi.client.transform.translateY       = new QListViewItem(m_pmi.client.transform.translation, "Y");
			m_pmi.client.transform.translateY->setRenameEnabled(1, true);
			m_pmi.client.transform.translateX       = new QListViewItem(m_pmi.client.transform.translation, "X");
			m_pmi.client.transform.translateX->setRenameEnabled(1, true);

			m_pmi.client.transform.rotation         = new QListViewItem(m_pmi.client.transformItem, "Rotation");
			m_pmi.client.transform.rotation->setSelectable    (false);
			m_pmi.client.transform.roll             = new QListViewItem(m_pmi.client.transform.rotation, "Roll");
			m_pmi.client.transform.roll->setRenameEnabled(1, true);
			m_pmi.client.transform.pitch            = new QListViewItem(m_pmi.client.transform.rotation, "Pitch");
			m_pmi.client.transform.pitch->setRenameEnabled(1, true);
			m_pmi.client.transform.yaw              = new QListViewItem(m_pmi.client.transform.rotation, "Yaw");
			m_pmi.client.transform.yaw->setRenameEnabled(1, true);
			
			m_pmi.client.transform.scale            = new QListViewItem(m_pmi.client.transformItem, "Scale");
			m_pmi.client.transform.scale->setSelectable       (false);
			m_pmi.client.transform.scaleZ           = new QListViewItem(m_pmi.client.transform.scale, "Z-Scale");
			m_pmi.client.transform.scaleZ->setRenameEnabled(1, true);
			m_pmi.client.transform.scaleY           = new QListViewItem(m_pmi.client.transform.scale, "Y-Scale");
			m_pmi.client.transform.scaleY->setRenameEnabled(1, true);
			m_pmi.client.transform.scaleX           = new QListViewItem(m_pmi.client.transform.scale, "X-Scale");
			m_pmi.client.transform.scaleX->setRenameEnabled(1, true);
		}

		m_attributesList->takeItem(m_pmi.client.generalItem);
		m_attributesList->takeItem(m_pmi.client.transformItem);
	}

	{
		m_pmi.tangible.tangibleItem             = new QListViewItem(m_attributesList, "Tangible");
		m_pmi.tangible.tangibleItem->setSelectable(false);
		
		{
			m_pmi.tangible.weight                 = new QListViewItem(m_pmi.tangible.tangibleItem, "Weight");
			m_pmi.tangible.HP                     = new QListViewItem(m_pmi.tangible.tangibleItem, "HP");
			m_pmi.tangible.maxHP                  = new QListViewItem(m_pmi.tangible.tangibleItem, "MaxHP");
			m_pmi.tangible.visible                = new QListViewItem(m_pmi.tangible.tangibleItem, "Visible");
			m_pmi.tangible.popupHelp              = new QListViewItem(m_pmi.tangible.tangibleItem, "PopupHelp");
			m_pmi.tangible.armorEffectiveness     = new QListViewItem(m_pmi.tangible.tangibleItem, "ArmorEffectiveness");
			m_pmi.tangible.frozen                 = new QListViewItem(m_pmi.tangible.tangibleItem, "Frozen");
			m_pmi.tangible.squelch                = new QListViewItem(m_pmi.tangible.tangibleItem, "Squelch");
			m_pmi.tangible.resourceList           = new QListViewItem(m_pmi.tangible.tangibleItem, "ResourceList");
		}

		m_attributesList->takeItem(m_pmi.tangible.tangibleItem);
	}

	{
		m_pmi.creature.creatureItem             = new QListViewItem(m_attributesList, "Creature");
		m_pmi.creature.creatureItem->setSelectable(false);

		{
			m_pmi.creature.maxAttribsItem         = new QListViewItem(m_pmi.creature.creatureItem, "Max Attributes");
			m_pmi.creature.maxAttribsItem         = new QListViewItem(m_pmi.creature.creatureItem, "Max Attributes");
		
			m_pmi.creature.attribs.health         = new QListViewItem(m_pmi.creature.maxAttribsItem, "Health");
			m_pmi.creature.attribs.strength       = new QListViewItem(m_pmi.creature.maxAttribsItem, "Strength");
			m_pmi.creature.attribs.constitution   = new QListViewItem(m_pmi.creature.maxAttribsItem, "Constitution");
			m_pmi.creature.attribs.action         = new QListViewItem(m_pmi.creature.maxAttribsItem, "Action");
			m_pmi.creature.attribs.quickness      = new QListViewItem(m_pmi.creature.maxAttribsItem, "Quickness");
			m_pmi.creature.attribs.stamina        = new QListViewItem(m_pmi.creature.maxAttribsItem, "Stamina");
			m_pmi.creature.attribs.mind           = new QListViewItem(m_pmi.creature.maxAttribsItem, "Mind");
			m_pmi.creature.attribs.focus          = new QListViewItem(m_pmi.creature.maxAttribsItem, "Focus");
			m_pmi.creature.attribs.willpower      = new QListViewItem(m_pmi.creature.maxAttribsItem, "Willpower");
		}

		{
			m_pmi.creature.attribsItem            = new QListViewItem(m_pmi.creature.creatureItem, "Attributes");
			m_pmi.creature.attribsItem->setSelectable(false);
		
			m_pmi.creature.attribs.health         = new QListViewItem(m_pmi.creature.attribsItem, "Health");
			m_pmi.creature.attribs.strength       = new QListViewItem(m_pmi.creature.attribsItem, "Strength");
			m_pmi.creature.attribs.constitution   = new QListViewItem(m_pmi.creature.attribsItem, "Constitution");
			m_pmi.creature.attribs.action         = new QListViewItem(m_pmi.creature.attribsItem, "Action");
			m_pmi.creature.attribs.quickness      = new QListViewItem(m_pmi.creature.attribsItem, "Quickness");
			m_pmi.creature.attribs.stamina        = new QListViewItem(m_pmi.creature.attribsItem, "Stamina");
			m_pmi.creature.attribs.mind           = new QListViewItem(m_pmi.creature.attribsItem, "Mind");
			m_pmi.creature.attribs.focus          = new QListViewItem(m_pmi.creature.attribsItem, "Focus");
			m_pmi.creature.attribs.willpower      = new QListViewItem(m_pmi.creature.attribsItem, "Willpower");
		}

		{
			m_pmi.creature.mood                   = new QListViewItem(m_pmi.creature.creatureItem, "Mood");
			m_pmi.creature.sayMode                = new QListViewItem(m_pmi.creature.creatureItem, "Say Mode");
			m_pmi.creature.gender                 = new QListViewItem(m_pmi.creature.creatureItem, "Gender");
		}

		m_attributesList->takeItem(m_pmi.creature.creatureItem);
	}

	IGNORE_RETURN(connect(m_attributesList, SIGNAL(itemRenamed(QListViewItem*, int, const QString&)), this, SLOT(onAttributeRenamed(QListViewItem*, int, const QString&))));

	if(m_creatureSkills)
	{
		IGNORE_RETURN(connect(m_creatureSkills, SIGNAL(contextMenuRequested(QListBoxItem *, const QPoint &)), this, SLOT(onCreatureSkillsContextMenuRequested(QListBoxItem *, const QPoint &))));
		IGNORE_RETURN(connect(Singleton<ActionsSkill>::getInstance().revokeSkill, SIGNAL(activated()), this, SLOT(onRevokeCreatureSkill())));
	}

	if(m_scriptsList)
	{
		IGNORE_RETURN(connect(m_scriptsList, SIGNAL(contextMenuRequested (QListViewItem *, const QPoint &, int)), this, SLOT(onScriptsListContextMenuRequested(QListViewItem *, const QPoint &, int))));
		IGNORE_RETURN(connect(Singleton<ActionsScript>::getInstance().removeScript, SIGNAL(activated()), this, SLOT(onRemoveScript())));
	}

	if(m_objVarsList)
	{
		IGNORE_RETURN(connect(m_objVarsList, SIGNAL(contextMenuRequested (QListViewItem *, const QPoint &, int)), this, SLOT(onObjvarListContextMenuRequested(QListViewItem *, const QPoint &, int))));
		IGNORE_RETURN(connect(Singleton<ActionsScript>::getInstance().removeObjvar, SIGNAL(activated()), this, SLOT(onRemoveObjvar())));
		IGNORE_RETURN(connect(Singleton<ActionsScript>::getInstance().setObjvar, SIGNAL(activated()), this, SLOT(onSetObjvar())));
	}

	// Whenever anyone says that the object's view needs to be 
	// updated, invoke updateObjects (see below)
	m_callback = new MessageDispatch::Callback;
	m_callback->connect(*this, &ObjectEditor::updateObjects);

	connectToMessage(Game::Messages::SCENE_CHANGED);
	connectToMessage(GodClientData::Messages::SELECTION_CHANGED);
	connectToMessage(GodClientData::Messages::SELECTED_OBJECTS_CHANGED);
	connectToMessage(ServerObjectData::Messages::OBJECT_INFO_CHANGED);
	connectToMessage("ConGenericMessage");
}

//-----------------------------------------------------------------

ObjectEditor::~ObjectEditor()
{
	while(m_attributesList->firstChild())
		m_attributesList->takeItem(m_attributesList->firstChild());

	emptyListView(m_scriptsList);
	emptyListView(m_objVarsList);

	delete m_pmi.client.generalItem;
	delete m_pmi.client.transformItem;
	delete m_pmi.tangible.tangibleItem;
	delete m_pmi.creature.creatureItem;

	m_pmi.client.generalItem    = 0;
	m_pmi.client.transformItem  = 0;
	m_pmi.tangible.tangibleItem = 0;
	m_pmi.creature.creatureItem = 0;

	delete m_callback;
	m_obj = 0;
}

//-----------------------------------------------------------------------

void ObjectEditor::updateObjects(const UpdateObjects *)
{
	refreshObjects();
}

//----------------------------------------------------------------------

void ObjectEditor::updateObjectData()
{
	//-- update scripts
	GodClientData::ObjectList_t olist;
	GodClientData::getInstance().getSelection(olist);

	m_creatureSkills->clear();

	if(olist.empty())
		return;

	std::vector <std::string> commonScripts;
	const ServerObjectData::ObjectInfo* oinfo = 0; 

	//-- find the first valid object info
	GodClientData::ObjectList_t::const_iterator it = olist.begin();
	for(; it != olist.end(); ++it)
	{
		oinfo = ServerObjectData::getInstance().getObjectInfo(NON_NULL(m_obj)->getNetworkId(), false);
		if(oinfo)
			break;
	}

	if(!oinfo)
		return;

	for(std::vector<std::string>::const_iterator sli = oinfo->scriptList.begin(); sli != oinfo->scriptList.end(); ++sli)
	{
		bool thisStringOk = true;

		for(GodClientData::ObjectList_t::const_iterator tit = it; thisStringOk && tit != olist.end(); ++tit)
		{
			const ServerObjectData::ObjectInfo* const toinfo = ServerObjectData::getInstance().getObjectInfo(NON_NULL(m_obj)->getNetworkId(), false);
			if(toinfo)
			{
				if(std::find(toinfo->scriptList.begin(), toinfo->scriptList.end(), *sli) == toinfo->scriptList.end())
				{
					thisStringOk = false;
					break;
				}
			}
		}

		if(thisStringOk)
		{
			new QListViewItem(m_scriptsList,(*sli).c_str()); //lint !e522 m_scriptsList takes ownership of the object, so storing it into a variable isn't important (except to lint)
		}
	}

	for(std::vector<std::string>::const_iterator oli = oinfo->objvarList.begin(); oli != oinfo->objvarList.end(); ++oli)
	{
		bool thisStringOk = true;

		for(GodClientData::ObjectList_t::const_iterator tit = it; thisStringOk && tit != olist.end(); ++tit)
		{
			const ServerObjectData::ObjectInfo* const toinfo = ServerObjectData::getInstance().getObjectInfo(NON_NULL(m_obj)->getNetworkId(), false);
			if(toinfo)
			{
				if(std::find(toinfo->objvarList.begin(), toinfo->objvarList.end(), *oli) == toinfo->objvarList.end())
				{
					thisStringOk = false;
					break;
				}
			}
		}

		if(thisStringOk)
		{
			new QListViewItem(m_objVarsList,(*oli).c_str()); //lint !e522 m_objvarsList takes ownership of the object, so storing it into a variable isn't important (except to lint)
		}
	}

	std::vector<std::string>::const_iterator i;
	for(i = oinfo->skillList.begin(); i != oinfo->skillList.end(); ++i)
	{
		m_creatureSkills->insertItem((*i).c_str());
	}
}

//-----------------------------------------------------------------

void ObjectEditor::refreshObjects()
{
	while(m_attributesList->firstChild())
		m_attributesList->takeItem(m_attributesList->firstChild());

	emptyListView(m_scriptsList);
	emptyListView(m_objVarsList);

	m_obj = 0;

	GodClientData::ObjectList_t olist;
	GodClientData::getInstance().getSelection(olist);

	if(olist.empty())
		return;
	
	m_obj = olist.back();

	ServerObjectData::getInstance().requestObjectInfo(m_obj->getNetworkId(), true);

	if(dynamic_cast<CreatureObject *>(m_obj))
		addInfoCreatureObject();
	
	if(dynamic_cast<TangibleObject *>(m_obj))
		addInfoTangibleObject();

	addInfoClientObject();

	updateObjectData();
}

//-----------------------------------------------------------------

namespace
{
	const char* makeBoolean(bool b)
	{
		return b ? "True" : "False";
	}
}
//-----------------------------------------------------------------

void ObjectEditor::addInfoClientObject()
{
	if(!m_obj)
		return;

	m_attributesList->insertItem(m_pmi.client.transformItem);
	m_attributesList->insertItem(m_pmi.client.generalItem);

	{
		m_pmi.client.general.networkId->setText          (1, m_obj->getNetworkId().getValueString().c_str() );
		m_pmi.client.general.authoritative->setText      (1, makeBoolean(m_obj->isAuthoritative()));
		m_pmi.client.general.watchable->setText          (1, makeBoolean(m_obj->isAuthoritative()));
		m_pmi.client.general.active->setText             (1, makeBoolean(m_obj->isActive()));
		if(m_obj->getAppearance() && m_obj->getAppearance()->getAppearanceTemplate())
			m_pmi.client.general.appearance->setText       (1, m_obj->getAppearance()->getAppearanceTemplate()->getName());
		m_pmi.client.general.objectTemplate->setText     (1, m_obj->getTemplateName());
		m_pmi.client.general.volume->setText             (1, "xxx");//_itoa(m_obj->getVolume(), numbuf, 10));
		m_pmi.client.general.script->setText             (1, "xxx");
		m_pmi.client.general.slotType->setText           (1, "xxx");
		m_pmi.client.general.name->setText               (1, m_obj->getDebugName());
		m_pmi.client.general.container->setText          (1, "xxx");
	}
	
	{
		QString buffer;

		//set position text
		const Vector &position = m_obj->getPosition_p();
		IGNORE_RETURN(buffer.setNum(position.x));
		m_pmi.client.transform.translateX->setText(1, buffer.latin1());
		IGNORE_RETURN(buffer.setNum(position.y));
		m_pmi.client.transform.translateY->setText(1, buffer.latin1());
		IGNORE_RETURN(buffer.setNum(position.z));
		m_pmi.client.transform.translateZ->setText(1, buffer.latin1());

		//set scale text
		const Vector & scale = m_obj->getScale();
		IGNORE_RETURN(buffer.setNum(scale.x));
		m_pmi.client.transform.scaleX->setText(1, buffer.latin1());
		IGNORE_RETURN(buffer.setNum(scale.y));
		m_pmi.client.transform.scaleY->setText(1, buffer.latin1());
		IGNORE_RETURN(buffer.setNum(scale.z));
		m_pmi.client.transform.scaleZ->setText(1, buffer.latin1());

		//set rotation text
		IGNORE_RETURN(buffer.setNum(m_obj->getObjectFrameK_w().theta()));
		m_pmi.client.transform.pitch->setText(1, buffer.latin1());
		IGNORE_RETURN(buffer.setNum(m_obj->getObjectFrameK_w().phi()));
		m_pmi.client.transform.yaw->setText(1, buffer.latin1());
		IGNORE_RETURN(buffer.setNum(m_obj->getObjectFrameJ_w().theta()));
		m_pmi.client.transform.roll->setText(1, buffer.latin1());
	}
}
//-----------------------------------------------------------------

void ObjectEditor::addInfoTangibleObject()
{
	m_attributesList->insertItem(m_pmi.tangible.tangibleItem);
}

//-----------------------------------------------------------------

void ObjectEditor::addInfoCreatureObject()
{
	m_attributesList->insertItem(m_pmi.creature.creatureItem);
}

//-----------------------------------------------------------------

void ObjectEditor::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{ 
	if(message.isType(Game::Messages::SCENE_CHANGED))
	{
		refreshObjects();
	}
	else if(message.isType(GodClientData::Messages::SELECTION_CHANGED))
	{
		refreshObjects();
	}
	else if(message.isType(GodClientData::Messages::SELECTED_OBJECTS_CHANGED))
	{
		refreshObjects();
	}
	else if(message.isType(ServerObjectData::Messages::OBJECT_INFO_CHANGED))
	{
		updateObjectData();
	}
	if(message.isType("ConGenericMessage"))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage&>(message).getByteStream().begin();
		ConGenericMessage cg(ri);
		const std::string msg = cg.getMsg();

		if(msg.find("script list object ") != msg.npos)
		{
			emptyListView(m_scriptsList);

			if(GodClientData::getInstance().getSelectionSize() == 1)
			{
				std::string line;
				std::vector<std::string> lines;
				size_t startpos = 0;
				size_t endpos = 0;

				if(msg.find("\n") == msg.npos)
				{
					DEBUG_FATAL(true, ("Expected at least one newline"));
				}
				while(Unicode::getFirstToken(msg, startpos, endpos, line, "\n") && endpos != msg.npos)
				{
					lines.push_back(line);
					startpos = endpos + 1;
				}
				char n[256];
				int numScripts = 0;
				int result = sscanf(lines[0].c_str(), "script list object %s, num %d\n", n, numScripts);
				if(result > 0)
				{
					NetworkId nid(n);
					std::vector<std::string>::iterator i = lines.begin();
					//advance past header line				
					++i;
					for(; i != lines.end(); ++i)
					{
						const std::string& script = (*i);
						new QListViewItem(m_scriptsList,script.c_str()); //lint !e522 m_scriptsList takes ownership of the object, so storing it into a variable isn't important (except to lint)
					}
				}
			}
		}

		else if(msg.find("objvar list") != msg.npos)
		{
			emptyListView(m_objVarsList);

			if(GodClientData::getInstance().getSelectionSize() == 1)
			{
				std::string line;
				std::vector<std::string> lines;
				size_t startpos = 0;
				size_t endpos;

				if(msg.find("\n") == msg.npos)
				{
					DEBUG_FATAL(true, ("Expected at least one newline"));
				}
				while(Unicode::getFirstToken(msg, startpos, endpos, line, "\n") && endpos != msg.npos)
				{
					lines.push_back(line);
					startpos = endpos + 1;
				}
				std::vector<std::string>::iterator i = lines.begin();
				//advance past header line				
				++i;

				QListViewItem* last = NULL;
				for(; i != lines.end(); ++i)
				{
					const std::string& objvar = (*i);
					QListViewItem* current = new QListViewItem(m_objVarsList, objvar.c_str()); //lint !e522 m_scriptsList takes ownership of the object, so storing it into a variable isn't important (except to lint)
					if(last)
						current->moveItem(last);
					last = current;
				}
			}
		}
	}
}

//-----------------------------------------------------------------

void ObjectEditor::dragEnterEvent(QDragEnterEvent * evt)
{
	if(GodClientData::getInstance().getSelectionEmpty())
		return;

	QString text;
	if(QTextDrag::decode(evt, text))
	{
		if(text == ActionsScript::DragMessages::SCRIPT_DRAGGED)
			evt->accept();
	}
}

//-----------------------------------------------------------------

void ObjectEditor::dropEvent(QDropEvent* evt)
{
	if(GodClientData::getInstance().getSelectionEmpty())
		return;
	
	QString text;
	if(QTextDrag::decode(evt, text))
	{
		if(text == ActionsScript::DragMessages::SCRIPT_DRAGGED)
		{
			evt->accept();

			std::string scriptName;
			if(!ActionsScript::getInstance().getSelectedScript(scriptName))
			{
				IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", "No script selected!"));
				return;
			}

			GodClientData::ObjectList_t olist;
			GodClientData::getInstance().getSelection(olist);
			
			for(GodClientData::ObjectList_t::const_iterator it = olist.begin(); it != olist.end(); ++it)
			{
				IGNORE_RETURN(ServerCommander::getInstance().scriptAttach(*(*it), scriptName));
				refreshObjects();
			}
		}
	}
}

//-----------------------------------------------------------------

void ObjectEditor::onAttributeRenamed(QListViewItem* item, int col, const QString &text) const
{
	UNREF(item);
	UNREF(col);
	UNREF(text);

	//all editing must occur on the "Value" column, which is the second column
	if(col != 1)
		return;

	//all uses of this data are floats, so convert and check for failure
	bool result;
	float floatValue = text.toFloat(&result);
	if(!result)
		return;

	if(item == 	    m_pmi.client.transform.translateX)
	{
		GodClientData::getInstance().setSelectionX(floatValue);
	}

	else if(item == m_pmi.client.transform.translateY)
	{
		GodClientData::getInstance().setSelectionY(floatValue);
	}

	else if(item == m_pmi.client.transform.translateZ)
	{
		GodClientData::getInstance().setSelectionZ(floatValue);
	}

	else if(item == m_pmi.client.transform.yaw)
	{
		GodClientData::getInstance().setSelectionYaw(floatValue);
	}

	else if(item == m_pmi.client.transform.pitch)
	{
		GodClientData::getInstance().setSelectionPitch(floatValue);
	}

	else if(item == m_pmi.client.transform.roll)
	{
		GodClientData::getInstance().setSelectionRoll(floatValue);
	}

	else if(item == m_pmi.client.transform.scaleX)
	{
		//we don't have scaling in right now, so don't set it
	}

	else if(item == m_pmi.client.transform.scaleY)
	{
		//we don't have scaling in right now, so don't set it
	}

	else if(item == m_pmi.client.transform.scaleZ)
	{
		//we don't have scaling in right now, so don't set it
	}
}

//-----------------------------------------------------------------------

void ObjectEditor::onCreatureSkillsContextMenuRequested(QListBoxItem * item, const QPoint & point)
{
	if(item == 0)
		return;

	QPopupMenu* const m_pop = new QPopupMenu(this, "menu");
	ActionsSkill & as = Singleton<ActionsSkill>::getInstance();


	IGNORE_RETURN(as.revokeSkill->addTo(m_pop));
	m_pop->popup(point);

} //lint !e818 item could be const, don't change sig of Qt function

//-----------------------------------------------------------------------

void ObjectEditor::onScriptsListContextMenuRequested(QListViewItem * item, const QPoint & point, int)
{
	if(item == 0)
		return;
	QPopupMenu* const m_pop = new QPopupMenu(this, "menu");


	ActionsScript* as =&ActionsScript::getInstance();

	IGNORE_RETURN(as->removeScript->addTo(m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());

	IGNORE_RETURN(as->edit->addTo   (m_pop));
	IGNORE_RETURN(as->view->addTo   (m_pop));
	IGNORE_RETURN(as->revert->addTo (m_pop));
	IGNORE_RETURN(as->submit->addTo (m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());

	IGNORE_RETURN(as->compile->addTo(m_pop));
	IGNORE_RETURN(as->serverReload->addTo(m_pop));
	m_pop->popup(point);
} //lint !e818 item could be const, don't change sig of Qt function

//-----------------------------------------------------------------------

void ObjectEditor::onObjvarListContextMenuRequested(QListViewItem * item, const QPoint & point, int)
{
	QPopupMenu * const m_pop = new QPopupMenu(this, "menu");

	ActionsScript * const as = &ActionsScript::getInstance();

	IGNORE_RETURN(as->setObjvar->addTo(m_pop));
	if (item)
		IGNORE_RETURN(as->removeObjvar->addTo(m_pop));

	m_pop->popup(point);
}

//-----------------------------------------------------------------------

void ObjectEditor::onRevokeCreatureSkill()
{
	QListBoxItem * item = m_creatureSkills->item(m_creatureSkills->currentItem());
	if(item)
	{
		const CreatureObject * creature = dynamic_cast<const CreatureObject *>(m_obj);
		if(creature)
		{
			if(item->text().ascii())
			{
				std::string skillName = item->text().ascii();
				IGNORE_RETURN(ServerCommander::getInstance().revokeSkill(*creature, skillName));
				m_creatureSkills->removeItem(m_creatureSkills->currentItem());
				refreshObjects();
			}
		}
	}
}

//-----------------------------------------------------------------------

void ObjectEditor::onRemoveScript()
{
	QListViewItem * item = m_scriptsList->currentItem();
	if(! item)
		return;

	if(! item->text(0).ascii())
		return;

	std::string scriptName = item->text(0).ascii();
	const ClientObject * o = dynamic_cast<const ClientObject *>(m_obj);
	if(o)
	{
		IGNORE_RETURN(ServerCommander::getInstance().scriptDetach(*o, scriptName));
		deleteListViewItem(m_scriptsList, item);
		refreshObjects();
	}
}

//-----------------------------------------------------------------------

void ObjectEditor::onRemoveObjvar()
{
	QListViewItem * item = m_objVarsList->currentItem();
	if (!item)
		return;

	if (!item->text(0).ascii())
		return;

	size_t endpos = 0;
	std::string objvarName;
	IGNORE_RETURN(Unicode::getFirstToken(std::string(item->text(0).ascii()), 0, endpos, objvarName));

	ClientObject const * const o = dynamic_cast<ClientObject const *>(m_obj);
	if (o)
	{		
		IGNORE_RETURN(ServerCommander::getInstance().objvarRemove(*o, objvarName));
		deleteListViewItem(m_objVarsList, item);
		refreshObjects();
	}
}

//-----------------------------------------------------------------------

void ObjectEditor::onSetObjvar()
{
	ClientObject const * const o = dynamic_cast<ClientObject const *>(m_obj);
	if (o)
	{		
		bool ok = false;

		QString const nameResult =
			QInputDialog::getText(
				tr("Setting an Objvar..."),
				tr("Select name of Objvar to set:"),
				QLineEdit::Normal,
				QString::null,
				&ok);

		if (ok)
		{
			QStringList typeList;
			typeList << "int" << "float" << "string";

			QString const typeResult =
				QInputDialog::getItem(
					tr("Setting an Objvar..."),
					tr("Select type of Objvar to set:"),
					typeList, 0, true, &ok);

			if (ok)
			{
				if (typeResult == "int")
				{
					int const valueResult =
						QInputDialog::getInteger(
							tr("Setting an Objvar..."),
							tr("Enter int value to set:"),
							0, -2147483647, 2147483647, 1, &ok);
					if (ok)
						IGNORE_RETURN(ServerCommander::getInstance().objvarSet(*o, nameResult.ascii(), valueResult));
				}
				else if (typeResult == "float")
				{
					float const valueResult =
						QInputDialog::getDouble(
							tr("Setting an Objvar..."),
							tr("Enter float value to set:"),
							0, -1e30, 1e30, 8, &ok);
					if (ok)
						IGNORE_RETURN(ServerCommander::getInstance().objvarSet(*o, nameResult.ascii(), valueResult));
				}
				else if (typeResult == "string")
				{
					QString const valueResult =
						QInputDialog::getText(
							tr("Setting an Objvar..."),
							tr("Enter string value to set:"),
							QLineEdit::Normal,
							QString::null,
							&ok);
					if (ok)
						IGNORE_RETURN(ServerCommander::getInstance().objvarSet(*o, nameResult.ascii(), std::string(valueResult.ascii())));
				}
				refreshObjects();
			}
		}
	}
}

// ======================================================================
 
