// ======================================================================
//
// SwgCuiCommunity.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommunity.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "sharedFile/Iff.h"
#include "sharedGame/MatchMakingId.h"
#include "sharedUtility/DataTable.h"
#include "swgClientUserInterface/SwgCuiCommunity_Character.h"
#include "swgClientUserInterface/SwgCuiCommunity_History.h"
#include "swgClientUserInterface/SwgCuiCommunity_Personal.h"
#include "swgClientUserInterface/SwgCuiCommunity_Search.h"
#include "swgClientUserInterface/SwgCuiCommunity_Friend.h"
#include "swgClientUserInterface/SwgCuiCommunity_Ignore.h"
#include "UIButton.h"
#include "UICombobox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UITabbedPane.h"
#include "UITable.h"
#include "UITableModelDefault.h"

#undef TEXT // This suppresses a lint warning in this file

// ======================================================================

SwgCuiCommunity::SwgCuiCommunity(UIPage &page)
 : CuiMediator("SwgCuiCommunity", page)
 , UIEventCallback()
 , m_doneButton(NULL)
 , m_personalMediator(NULL)
 , m_historyMediator(NULL)
 , m_characterMediator(NULL)
 , m_searchMediator(NULL)
 , m_friendMediator(NULL)
 , m_ignoreMediator(NULL)
 , m_tabs(NULL)
 , m_matchMakingTypeIndexes(NULL)
{
	UIPage *subPage = NULL;

	getCodeDataObject(TUIPage, subPage, "pagePersonal");
	m_personalMediator = new Personal(*subPage, *this);
	m_personalMediator->fetch();
	m_mediators[M_personal] = m_personalMediator;

	getCodeDataObject(TUIPage, subPage, "pageHistory");
	m_historyMediator = new History(*subPage, *this);
	m_historyMediator->fetch();
	m_mediators[M_history] = m_historyMediator;

	getCodeDataObject(TUIPage, subPage, "pageCharacter");
	m_characterMediator = new Character(*subPage, *this);
	m_characterMediator->fetch();
	m_mediators[M_character] = m_characterMediator;

	getCodeDataObject(TUIPage, subPage, "pageSearch");
	m_searchMediator = new Search(*subPage, *this);
	m_searchMediator->fetch();
	m_mediators[M_search] = m_searchMediator;

	getCodeDataObject(TUIPage, subPage, "pageFriend");
	m_friendMediator = new Friend(*subPage, *this);
	m_friendMediator->fetch();
	m_mediators[M_friend] = m_friendMediator;

	getCodeDataObject(TUIPage, subPage, "pageIgnore");
	m_ignoreMediator = new Ignore(*subPage, *this);
	m_ignoreMediator->fetch();
	m_mediators[M_ignore] = m_ignoreMediator;

	getCodeDataObject(TUIButton, m_doneButton, "buttonDone");
	registerMediatorObject(*m_doneButton, true);

	getCodeDataObject (TUITabbedPane, m_tabs, "tabs");
	registerMediatorObject(*m_tabs, true);

	m_tabs->SetActiveTab(-1);
	m_tabs->SetActiveTab(0);

	IGNORE_RETURN(setState(MS_closeable));
	IGNORE_RETURN(setState(MS_closeDeactivates));

	// Load the data table information

	DataTable dataTable;
	Iff matchMakingIff("datatables/matchmaking/matchmaking.iff");
	dataTable.load(matchMakingIff);
	m_matchMakingTypeIndexes = new StringToIntMap;

	for (int i = 0; i < dataTable.getNumRows(); ++i)
	{
		int const row = i;
		int const column = 1;
		std::string const &type = dataTable.getStringValue(column, row);
		IGNORE_RETURN(m_matchMakingTypeIndexes->insert(std::make_pair(type, i)));
	}
}

//-----------------------------------------------------------------

SwgCuiCommunity::~SwgCuiCommunity()
{
	m_personalMediator->release();
	m_personalMediator = NULL;

	m_historyMediator->release();
	m_historyMediator = NULL;

	m_characterMediator->release();
	m_characterMediator = NULL;

	m_searchMediator->release();
	m_searchMediator = NULL;

	m_friendMediator->release();
	m_friendMediator = NULL;

	m_ignoreMediator->release();
	m_ignoreMediator = NULL;

	m_doneButton = NULL;
	m_tabs = NULL;

	delete m_matchMakingTypeIndexes;
	m_matchMakingTypeIndexes = NULL;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::performActivate()
{
	OnTabbedPaneChanged(m_tabs);

	CuiManager::requestPointer(true);
}

//-----------------------------------------------------------------

void SwgCuiCommunity::performDeactivate()
{
	CuiManager::requestPointer(false);

	// Deactivate all the mediators

	m_personalMediator->deactivate();
	m_historyMediator->deactivate();
	m_characterMediator->deactivate();
	m_searchMediator->deactivate();
	m_friendMediator->deactivate();
	m_ignoreMediator->deactivate();
}

//-----------------------------------------------------------------

void SwgCuiCommunity::OnButtonPressed(UIWidget *context)
{
	if (context == m_doneButton)
	{
		deactivate();
	}
}

//-----------------------------------------------------------------

SwgCuiCommunity *SwgCuiCommunity::createInto(UIPage *parent)
{
	NOT_NULL (parent);
	UIPage * const page = NON_NULL(safe_cast<UIPage *>(parent->GetObjectFromPath("/Community.main", TUIPage)));
	UIPage * const dupe = NON_NULL(safe_cast<UIPage *>(page->DuplicateObject()));
	IGNORE_RETURN(parent->AddChild(dupe));
	IGNORE_RETURN(parent->MoveChild(dupe, UIBaseObject::Top));
	dupe->Link();
	dupe->Center();
	
	SwgCuiCommunity * const creation = new SwgCuiCommunity(*dupe);
	return creation;
}

//----------------------------------------------------------------------

int SwgCuiCommunity::getMatchMakingTypeIndex(std::string const &type) const
{
	int result = 0;

	if (m_matchMakingTypeIndexes != NULL)
	{
		StringToIntMap::iterator iterMatchMakingTypeIndexes = m_matchMakingTypeIndexes->find(type);

		if (iterMatchMakingTypeIndexes != m_matchMakingTypeIndexes->end())
		{
			result = iterMatchMakingTypeIndexes->second;
		}
	}

	return result;
}

//----------------------------------------------------------------------

void SwgCuiCommunity::OnTabbedPaneChanged(UIWidget *context)
{
	if (context == m_tabs)
	{
		if ((m_tabs->GetActiveTab() >= 0) &&
		    (m_tabs->GetActiveTab() < static_cast<int>(M_count)))
		{
			// Deactivate all the pages

			for (int i = 0; i < static_cast<int>(M_count); ++i)
			{
				if (i != m_tabs->GetActiveTab())
				{
					m_mediators[i]->deactivate();
				}
			}

			m_mediators[m_tabs->GetActiveTab()]->activate();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCommunity::removeSelectedItemsFromList(UIList &list, StringVector &names) const
{
	// Build a list of selected items
	
	typedef std::vector<UIData *> UIDataVector;
	UIDataVector selectedItems;
	selectedItems.reserve(static_cast<unsigned int>(list.GetRowCount()));

	UIDataSource *dataSource = list.GetDataSource();

	if (dataSource != NULL)
	{
		for (int row = 0; row < list.GetRowCount(); ++row)
		{
			if (list.IsRowSelected(row))
			{
				selectedItems.push_back(list.GetDataAtRow(row));
			}
		}

		// Delete all the selected items

		names.clear();
		UIDataVector::iterator iterSelectedItems = selectedItems.begin();
		names.reserve(selectedItems.size());

		for (; iterSelectedItems != selectedItems.end(); ++iterSelectedItems)
		{
			UIData *data = (*iterSelectedItems);

			if (data != NULL)
			{
				names.push_back(data->GetName());
				IGNORE_RETURN(dataSource->RemoveChild(data));
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCommunity::removeSelectedItemsFromList(UIList &list, UnicodeStringVector &labels) const
{
	// Build a list of selected items
	
	typedef std::vector<UIData *> UIDataVector;
	UIDataVector selectedItems;
	selectedItems.reserve(static_cast<unsigned int>(list.GetRowCount()));

	UIDataSource *dataSource = list.GetDataSource();

	if (dataSource != NULL)
	{
		for (int row = 0; row < list.GetRowCount(); ++row)
		{
			if (list.IsRowSelected(row))
			{
				selectedItems.push_back(list.GetDataAtRow(row));
			}
		}

		// Delete all the selected items

		labels.clear();
		UIDataVector::iterator iterSelectedItems = selectedItems.begin();
		labels.reserve(selectedItems.size());

		for (; iterSelectedItems != selectedItems.end(); ++iterSelectedItems)
		{
			UIData *data = (*iterSelectedItems);

			if (data != NULL)
			{
				Unicode::String value;
				if (data->GetProperty(UIList::DataProperties::LOCALTEXT, value))
				{

					labels.push_back(value);
					IGNORE_RETURN(dataSource->RemoveChild(data));
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCommunity::addItemToSortedList(UIList &list, Unicode::String const &localizedString, std::string const &nameString) const
{
	// We need to keep everything sorted in this list, so pull
	// everything out to a sorted container, then insert the 
	// new item, then put everything back in the list

	// Pull everything out of the list

	typedef std::map<Unicode::String, std::string> ListItem;
	ListItem listItems;

	{
		for (int row = 0; row < list.GetRowCount(); ++row)
		{
			UIData *data = list.GetDataAtRow(row);

			if (data != NULL)
			{
				Unicode::String value;

				if (data->GetProperty(UIList::DataProperties::LOCALTEXT, value))
				{
					IGNORE_RETURN(listItems.insert(std::make_pair(value, data->GetName())));
				}
			}
		}
	}

	// Insert the new item

	IGNORE_RETURN(listItems.insert(std::make_pair(localizedString, nameString)));

	// Now put everything back into the list

	UIDataSource *listDataSource = list.GetDataSource();

	if (listDataSource != NULL)
	{
		list.Clear();
		listDataSource->Attach(NULL);
		list.SetDataSource(NULL);

		ListItem::iterator iterListItems = listItems.begin();
		long row = 0;
		long newItemRow = -1;

		for (; iterListItems != listItems.end(); ++iterListItems)
		{
			if (nameString == iterListItems->second)
			{
				newItemRow = row;
			}

			// Add the item to the list

			UIData *data = new UIData;
			IGNORE_RETURN(data->SetProperty(UIList::DataProperties::LOCALTEXT, iterListItems->first));
			data->SetName(iterListItems->second);
			IGNORE_RETURN(listDataSource->AddChild(data));
			++row;
		}

		list.SetDataSource(listDataSource);
		IGNORE_RETURN(listDataSource->Detach(NULL));

		// Make sure the item that was inserted is selected and visible

		if (newItemRow >= 0)
		{
			list.SelectRow(newItemRow);
			list.ScrollToRow(newItemRow);
		}
	}
}

//-----------------------------------------------------------------

PlayerCreatureController *SwgCuiCommunity::getLocalPlayerCreatureController() const
{
	PlayerCreatureController *result = NULL;
	CreatureObject * const playerCreatureObject = Game::getPlayerCreature ();

	if (playerCreatureObject != NULL)
	{
		result = dynamic_cast<PlayerCreatureController *>(playerCreatureObject->getController());
	}

	return result;
}

//-----------------------------------------------------------------

CreatureObject *SwgCuiCommunity::getLocalCreatureObject() const
{
	return dynamic_cast<CreatureObject *>(Game::getPlayer());
}

//-----------------------------------------------------------------

void SwgCuiCommunity::addItem(UIDataSource &dataSource, Unicode::String const &localText, std::string const &name) const
{
	UIData *data = new UIData;
	IGNORE_RETURN(data->SetProperty(UIList::DataProperties::LOCALTEXT, localText));
	data->SetName(name);
	IGNORE_RETURN(dataSource.AddChild(data));
}

//-----------------------------------------------------------------

void SwgCuiCommunity::getProfessionTitles(SkillObject::SkillVector const &skillBoxes, UnicodeStringToStringIdMap &listItems, bool onlySearchableTitles) const
{
	SkillObject::SkillVector::const_iterator iterSkillVector = skillBoxes.begin();

	for (; iterSkillVector != skillBoxes.end(); ++iterSkillVector)
	{
		SkillObject const *skillObject = (*iterSkillVector);

		if (   (skillObject != NULL)
	        && (strstr(skillObject->getSkillName().c_str(), "jedi") == NULL))
		{
			if (skillObject->isTitle() && (!onlySearchableTitles || skillObject->isSearchable()))
			{
				Unicode::String localizedTitle;
				IGNORE_RETURN(CuiSkillManager::localizeSkillTitle(skillObject->getSkillName(), localizedTitle));

				IGNORE_RETURN(listItems.insert(std::make_pair(localizedTitle, skillObject->getSkillName())));
			}

		    if (!skillObject->getNextSkillBoxes().empty())
			{
				getProfessionTitles(skillObject->getNextSkillBoxes(), listItems, onlySearchableTitles);
			}
		}
	}
}

// ======================================================================
