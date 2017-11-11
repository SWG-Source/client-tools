// ======================================================================
//
// TreeBrowser.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "TreeBrowser.h"
#include "TreeBrowser.moc"

#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/World.h"

#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"

#include "ActionHack.h"
#include "ActionsBuildoutArea.h"
#include "ActionsEdit.h"
#include "ActionsObjectTemplate.h"
#include "ActionsScript.h"
#include "ActionsView.h"
#include "ClientTemplateListView.h"
#include "FilesystemTree.h"
#include "FilterManager.h"
#include "GodClientData.h"
#include "GodClientPerforce.h"
#include "IconLoader.h"
#include "ObjectTemplateData.h"
#include "ServerObjectData.h"
#include "ScriptListView.h"
#include "ServerTemplateListView.h"

#include <qdragobject.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qtimer.h>

// ======================================================================

namespace
{
	/**
	 * This class represents a QListViewItem with the additional logic to study an Object* and determine what text to display
	 */
	struct ObjectListItem : public QListViewItem
	{
		ObjectListItem(QListView* theParent, ClientObject* obj)
		: QListViewItem(theParent),
		  m_objPtr(obj)
		{
			NOT_NULL(obj);
			//TODO optionally show name, networkId, short template name, long template name, short client template name, long client template name
			//should all this logic really be in a constructor?

			const ServerObjectData::ObjectInfo* objData = ServerObjectData::getInstance().getObjectInfo(obj->getNetworkId(), false);
			
			std::string serverTemplateName = "";

			if(objData)
			{
				serverTemplateName = objData->serverTemplateName;
			}
			else if (obj->getTemplateName())
			{
				serverTemplateName = obj->getTemplateName();
			}
			std::string shortTemplateName = serverTemplateName;
		

			//always show the avatar name, since it doesn't necessarily have some of the other data
			GroundScene* gs            = NON_NULL(dynamic_cast<GroundScene*>(Game::getScene()));
			const Object* const player = NON_NULL(gs->getPlayer());
			if(obj == player)
			{
				shortTemplateName = "Avatar(";
				shortTemplateName += obj->getDebugName();
				shortTemplateName += ")";
			}	
			//for every object except the avatar, look for the last \ or / and only show the last part
			else
			{
				if(shortTemplateName == "")
					shortTemplateName = "ServerTemplate name unavailable";
				size_t backPos  = serverTemplateName.find_last_of("\\");
				size_t frontPos = serverTemplateName.find_last_of("/");

				//case1: only a \\ was found
				if(backPos != std::string::npos && frontPos == std::string::npos) //lint !e737 !e650 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
				{
					shortTemplateName = serverTemplateName.substr(backPos+1); //implicit copy to end of string
				}
				//case2: only a / was found
				if(frontPos != std::string::npos && backPos == std::string::npos) //lint !e737 !e650 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
				{
					shortTemplateName = serverTemplateName.substr(frontPos+1); //implicit copy to end of string
				}
				//case3: both were found, use the latter
				else if(backPos != std::string::npos && frontPos != std::string::npos) //lint !e737 !e650 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
				{
					if(backPos > frontPos)
					{
						shortTemplateName = serverTemplateName.substr(backPos+1); //implicit copy to end of string
					}
					else
						shortTemplateName = serverTemplateName.substr(frontPos+1); //implicit copy to end of string
				}
				//else use the whole string(as set earlier)

				//now pull off any ".iff" string
				size_t iffPos = shortTemplateName.find(".iff");
				//assume that the .iff is at the end, and strip it off
				if(shortTemplateName == "")
					shortTemplateName = "No ServerTemplate available(bad?)";
				shortTemplateName = shortTemplateName.substr(0, iffPos);

			}
			QListViewItem::setText(0, shortTemplateName.c_str());
		}

		ClientObject* m_objPtr;
	private:
		//disabled
		ObjectListItem();
	};

	ObjectListItem* findObjectItem(QListViewItem* item, const ClientObject* obj)
	{
		while(item)
		{
			ObjectListItem* const moli = dynamic_cast<ObjectListItem*>(item);

			if(moli && moli->m_objPtr == obj)
				return moli;

			item = item->nextSibling();
		}
		return 0;
	}
}

//-----------------------------------------------------------------

TreeBrowser::TreeBrowser(QWidget* theParent, const char* theName)
: BaseTreeBrowser(theParent, theName),
  MessageDispatch::Receiver(),
  m_refreshTimer(0),
  m_updatingSelectionFromWorld(false)
{
	m_objectList->clear();
	m_objectList->setResizeMode(QListView::NoColumn);
	m_objectList->setRootIsDecorated(true);
	m_objectList->setColumnWidthMode(0, QListView::Maximum);

	//connect the buttons and Qt signals to the appropriate Qt slots
	IGNORE_RETURN(connect(m_serverTemplateRefreshButton, SIGNAL(clicked()), ActionsObjectTemplate::getInstance().m_serverRefresh, SLOT(doActivate())));
	IGNORE_RETURN(connect(m_clientTemplateRefreshButton, SIGNAL(clicked()), ActionsObjectTemplate::getInstance().m_clientRefresh, SLOT(doActivate())));
	IGNORE_RETURN(connect(m_scriptRefreshButton,   SIGNAL(clicked()), ActionsScript::getInstance().refresh, SLOT(doActivate())));
	IGNORE_RETURN(connect(m_buildoutAreaRefreshButton,   SIGNAL(clicked()), ActionsBuildoutArea::getInstance().actionRefresh, SLOT(doActivate())));

	IGNORE_RETURN(connect(m_refreshButton, SIGNAL(clicked()), this, SLOT(refreshObjects())));
	IGNORE_RETURN(connect(m_objectList,    SIGNAL(selectionChanged()), this, SLOT(onObjectSelectionChanged())));
	IGNORE_RETURN(connect(m_serverTemplateList,  SIGNAL(selectionChanged()), this, SLOT(onServerTemplateListSelectionChanged())));
	IGNORE_RETURN(connect(m_clientTemplateList,  SIGNAL(selectionChanged()), this, SLOT(onClientTemplateListSelectionChanged())));
	IGNORE_RETURN(connect(m_objectList,    SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(onObjectDoubleClicked(QListViewItem*))));

	//connect to the MessageDispatch messages we care about(selection, object changes, and scene changes)
	connectToMessage(GodClientData::Messages::SELECTION_CHANGED);
	connectToMessage(World::Messages::OBJECT_ADDED);
	connectToMessage(World::Messages::OBJECT_REMOVED);
	connectToMessage(Game::Messages::SCENE_CHANGED);

	//initialize the views with the data from perforce
	ActionsScript::getInstance().refresh->doActivate();
	ActionsObjectTemplate::getInstance().m_serverRefresh->doActivate();
	ActionsObjectTemplate::getInstance().m_clientRefresh->doActivate();
	ActionsBuildoutArea::getInstance().actionRefresh->doActivate();
}

//-----------------------------------------------------------------

TreeBrowser::~TreeBrowser()
{
	m_objectList->clear();
	delete m_refreshTimer;
}

//-----------------------------------------------------------------

void TreeBrowser::refreshObjects()
{
	QListViewItem* item = 0;
	std::vector<const ClientObject*> previousList;
	std::vector<const ClientObject*> currentList;
	for(item = m_objectList->firstChild(); item; item = item->nextSibling())
	{
		ObjectListItem* const moli = dynamic_cast<ObjectListItem*>(item);
		previousList.push_back(moli->m_objPtr);
	}

	m_updatingSelectionFromWorld = true;

	GroundScene* const gs = dynamic_cast<GroundScene*>(Game::getScene());

	if(gs == 0)
	{
		m_updatingSelectionFromWorld = false;
		return;
	}

	const Object* const player = NON_NULL(gs->getPlayer());
	if(player == 0)
	{
		m_updatingSelectionFromWorld = false;
		return;
	}
	const std::string playerBuildoutName 
		= SharedBuildoutAreaManager::getBuildoutNameForPosition(Game::getSceneId().c_str(), player->getPosition_w(), false);
	const CellProperty * const playerCell = player->getParentCell();

	//show all tangibles (buildings, stormtroopers, etc.)
	for(int i = 0; i < ClientWorld::getNumberOfObjects(static_cast<int>(WOL_Tangible)); ++i)
	{
		ClientObject* obj = dynamic_cast<ClientObject*>(ClientWorld::getObject(static_cast<int>(WOL_Tangible), i));
		if(obj)
		{	
			const CellProperty * const otherCell = obj->getParentCell();
			//only show objects in the same cell as the player
			if (playerCell == otherCell)
			{
				// if player is in the world cell check to see if the object and player are in the same buildout
				bool inSameBuildoutOrNotInWorldCell = true;
				if(playerCell->isWorldCell())
				{
					const std::string objectBuildoutName = 
						SharedBuildoutAreaManager::getBuildoutNameForPosition(Game::getSceneId().c_str(), obj->getPosition_w(), false);
					inSameBuildoutOrNotInWorldCell = (objectBuildoutName == playerBuildoutName);
				}
				
				if(inSameBuildoutOrNotInWorldCell)
				{
					//only show objects that have appearances
					const Appearance* appearance = obj->getAppearance();
					if(appearance)
					{
						//only add the item to the listview if passes our filters
						if(FilterManager::validate(*obj))
						{
							if(std::find(previousList.begin(), previousList.end(), obj) == previousList.end())
							{
								IGNORE_RETURN(new ObjectListItem(m_objectList,obj));
							}
							currentList.push_back(obj);
						}	
					}
				}
			}
		}
	}

	//show all statics (object that the client can't interact with, i.e. rocks, particle sytems)
	for(int j = 0; j < ClientWorld::getNumberOfObjects(static_cast<int>(WOL_TangibleNotTargetable)); ++j)
	{
		ClientObject* obj = dynamic_cast<ClientObject*>(ClientWorld::getObject(static_cast<int>(WOL_TangibleNotTargetable), j));
		if(obj)
		{
			const CellProperty * const otherCell = obj->getParentCell();
			//only show objects in the same cell as the camera
			if (playerCell == otherCell)
			{
				// if player is in the world cell check to see if the object and player are in the same buildout
				bool inSameBuildoutOrNotInWorldCell = true;
				if(playerCell->isWorldCell())
				{
					const std::string objectBuildoutName = 
						SharedBuildoutAreaManager::getBuildoutNameForPosition(Game::getSceneId().c_str(), obj->getPosition_w(), false);
					inSameBuildoutOrNotInWorldCell = (objectBuildoutName == playerBuildoutName);
				}
				
				if(inSameBuildoutOrNotInWorldCell)
				{
					//only show objects that have appearances
					const Appearance* appearance = obj->getAppearance();
					if(appearance)
					{
						//only add the item to the listview if passes our filters
						if(FilterManager::validate(*obj))
						{
							if(std::find(previousList.begin(), previousList.end(), obj) == previousList.end())
							{
								IGNORE_RETURN(new ObjectListItem(m_objectList,obj));
							}
							currentList.push_back(obj);
						}
					}
				}
			}
		}
	}

	if(!previousList.empty())
	{
		//previousList holds every in the list from last frame, currentList holds everything in this frame, look for leftovers and delete
		std::vector<const ClientObject*>  difference;
		for(std::vector<const ClientObject*>::iterator it = previousList.begin(); it != previousList.end(); ++it)
		{
			if(std::find(currentList.begin(), currentList.end(), *it) == currentList.end())
			{
				difference.push_back(*it);
			}
		}
	
		for(item = m_objectList->firstChild(); item; )
		{
			ObjectListItem* moli = dynamic_cast<ObjectListItem*>(item);
			const ClientObject* objPtr = moli->m_objPtr;
			std::vector<const ClientObject*>::iterator it2 = std::find(difference.begin(), difference.end(),objPtr);
			item = item->nextSibling();
			if(it2 != difference.end())
			{
				m_objectList->takeItem(moli);
				delete moli;
			}
		}
	}
	updateSelectionFromWorld();

}

//-----------------------------------------------------------------

namespace
{
	template <typename T> void populateListItem(T* view, const AbstractFilesystemTree::Node* node)
	{
		for(AbstractFilesystemTree::Node::ConstIterator it = node->begin(); it != node->end(); ++it)
		{
			std::string name =(*it)->name;
			
			if((*it)->type == FilesystemTree::Node::File)
			{
				const size_t dotpos = name.find_last_of('.');
				
				if(dotpos != name.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
					name = name.substr(0, dotpos);
			}

			QListViewItem* const ni = new QListViewItem(view, name.c_str());

			if((*it)->type == FilesystemTree::Node::Folder)
			{
				ni->setSelectable(false);
				populateListItem(ni,*it);
			}
			else
			{
				ni->setDragEnabled(true);
				ni->setPixmap(0, IconLoader::fetchPixmap(IconLoader::hi16_mime_document));
			}
		}
	}
}

//-----------------------------------------------------------------

void TreeBrowser::onObjectSelectionChanged() const
{
	if(m_updatingSelectionFromWorld)
		return;

	GroundScene* const gs = dynamic_cast<GroundScene*>(Game::getScene());

	if(gs == 0)
		return;
	
	GodClientData::ObjectList_t olist;
	GodClientData* gcd = &GodClientData::getInstance();
	gcd->getSelection(olist);

	GodClientData::ObjectList_t new_olist;

	QListViewItem* item = 0;

	//-- only tangible works right now
	for(item = m_objectList->firstChild(); item; item = item->nextSibling())
	{
		if(!item->isSelected())
			continue;

		ObjectListItem* const moli = dynamic_cast<ObjectListItem*>(item);
		
		ClientObject* const obj = moli->m_objPtr;
		
		if(obj)
			new_olist.push_back(obj);
	}

	//-- check for removals, remove from old list
	{
		for(GodClientData::ObjectList_t::iterator it = olist.begin(); it != olist.end();)
		{
			if(std::find(new_olist.begin(), new_olist.end(),*it) == new_olist.end())
				it = olist.erase(it);
			else
				++it;
		}
	}
	//-- add non-dupes to old list
	{
		for(GodClientData::ObjectList_t::iterator it = new_olist.begin(); it != new_olist.end(); ++it)
		{
			if(std::find(olist.begin(), olist.end(),*it) == olist.end())
			{
				olist.push_back(*it);
			}
		}
	}

	gcd->replaceSelection(olist);
}

//-----------------------------------------------------------------

void TreeBrowser::onServerTemplateListSelectionChanged()
{
	const QListViewItem* item = m_serverTemplateList->selectedItem();

	if(item == 0)
	{
		//emit signal
		serverTemplateSelectionChanged("");
		return;
	}

	std::string path = item->text(0).latin1();

	for(;;)
	{
		item = dynamic_cast <QListViewItem*>(item->parent());
		if(!item)
			break;

		//if the template is being edited, we add a false item to the treelist, don't use that for the real file path
		std::string newString = item->text(0).latin1();
		if(newString != "[NEW]" && newString != "[EDIT]")
			path = newString + "/" + path;
	}

	//for the server objects, we want to track what object to create
	ActionsEdit::getInstance().onServerTemplateSelectionChanged(path);
}

//-----------------------------------------------------------------

void TreeBrowser::onClientTemplateListSelectionChanged()
{
	const QListViewItem* item = m_clientTemplateList->selectedItem();

	if(item == 0)
	{
		//emit signal
		clientTemplateSelectionChanged("");
		return;
	}

	std::string path = item->text(0).latin1();

	for(;;)
	{
		item = dynamic_cast <QListViewItem*>(item->parent());
		if(!item)
			break;

		//if the template is being edited, we add a false item to the treelist, don't use that for the real file path
		std::string newString = item->text(0).latin1();
		if(newString != "[NEW]" && newString != "[EDIT]")
			path = newString + "/" + path;
	}

	//for the server objects, we want to track what object to create
	ActionsEdit::getInstance().onClientTemplateSelectionChanged(path);
}

//-----------------------------------------------------------------

void TreeBrowser::onObjectDoubleClicked(QListViewItem* item)
{
	const ObjectListItem* const moli = dynamic_cast<ObjectListItem*>(item);

	if(moli == 0)
		return;

	const ClientObject* const obj = moli->m_objPtr;
	
	if(obj)
	{
		emit objectDoubleClickedSignal(*obj, true, static_cast<int>(ActionsView::Fit_dontCare));
	}
	else
	{
		char buf [128];
		IGNORE_RETURN(_snprintf(buf, 128, "Object id %d not found in world.", moli->m_objPtr->getNetworkId()));
		IGNORE_RETURN(QMessageBox::warning(this, "Warning", buf));
		return;
	}
} 

//-----------------------------------------------------------------

void TreeBrowser::receiveMessage(const MessageDispatch::Emitter& , const MessageDispatch::MessageBase& message)
{
	if(message.isType(World::Messages::OBJECT_ADDED))
	{
		refreshObjects();
	}
	else if(message.isType(World::Messages::OBJECT_REMOVED))
	{
		refreshObjects();
	}
	else if(message.isType(GodClientData::Messages::SELECTION_CHANGED))
	{
		updateSelectionFromWorld();
	}
	else if(message.isType(Game::Messages::SCENE_CHANGED))
	{
		// set up a single shot timer to create the object list after login
		if(m_refreshTimer)
		{
			delete m_refreshTimer;
			m_refreshTimer = 0;
		}
		m_refreshTimer = new QTimer(this, "refreshTimer");
		IGNORE_RETURN(connect(m_refreshTimer, SIGNAL(timeout()), this, SLOT(refreshObjects())));
		IGNORE_RETURN(m_refreshTimer->start(2000, false));
	}
}

//-----------------------------------------------------------------

void TreeBrowser::updateSelectionFromWorld()
{
	GroundScene* const gs = dynamic_cast<GroundScene*>(Game::getScene());
	
	if(gs == 0)
		return;
	
	m_updatingSelectionFromWorld = true;
	{
		
		GodClientData::ObjectList_t olist;
		GodClientData* gcd =&GodClientData::getInstance();
		gcd->getSelection(olist);

		m_objectList->clearSelection();
		
		for(GodClientData::ObjectList_t::iterator it = olist.begin(); it != olist.end(); ++it)
		{
			ObjectListItem* const moli = findObjectItem(m_objectList->firstChild(),*it);

			if(moli)
			{
				moli->setSelected(true);
			}
		}
	}
	m_objectList->triggerUpdate();

	m_updatingSelectionFromWorld = false;
}

//======================================================================
