// ======================================================================
//
// SwgCuiCommunity_Ignore.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommunity_Ignore.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/Game.h"
#include "clientGame/MatchMakingManager.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "UIButton.h"
#include "UIData.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITextbox.h"

// ======================================================================

SwgCuiCommunity::Ignore::Ignore(UIPage &page, SwgCuiCommunity const &communityMediator)
 : CuiMediator("SwgCuiCommunity_Ignore", page)
 , UIEventCallback()
 , m_communityMediator(communityMediator)
 , m_removeButton(NULL)
 , m_addButton(NULL)
 , m_nameTextBox(NULL)
 , m_ignoreList(NULL)
 , m_callBack(new MessageDispatch::Callback)
 , m_ignoreListPopulated(false)
{
	m_callBack->connect(*this, &SwgCuiCommunity::Ignore::onIgnoreListChanged, static_cast<CommunityManager::Messages::IgnoreListChanged *>(0));

	getCodeDataObject(TUIButton, m_removeButton, "buttonRemove");
	registerMediatorObject(*m_removeButton, true);

	getCodeDataObject(TUIButton, m_addButton, "buttonAdd");
	registerMediatorObject(*m_addButton, true);

	getCodeDataObject(TUITextbox, m_nameTextBox, "textboxName");
	registerMediatorObject(*m_nameTextBox, true);
	m_nameTextBox->SetEditable(true);
	m_nameTextBox->SetLocalText(Unicode::emptyString);

	getCodeDataObject(TUIList, m_ignoreList, "listIgnore");
	registerMediatorObject(*m_ignoreList, true);

	m_ignoreList->SetSelectionAllowedMultiRow(true);
}

//-----------------------------------------------------------------

SwgCuiCommunity::Ignore::~Ignore()
{
	delete m_callBack;
	m_callBack = NULL;

	m_removeButton = NULL;
	m_addButton = NULL;
	m_nameTextBox = NULL;
	m_ignoreList = NULL;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Ignore::performActivate()
{
	if (!m_ignoreListPopulated)
	{
		// Make sure the list has been populated at least once, this code
		// gets hit as a result of the community ui has never being opened
		// before the ignore list is synchronized first.

		buildIgnoreList();
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Ignore::performDeactivate()
{
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Ignore::OnButtonPressed(UIWidget *context)
{
	if (context == m_removeButton)
	{
		removeSelectedItems();
	}
	else if (context == m_addButton)
	{
		addIgnoreName();
	}
}

//-----------------------------------------------------------------

bool SwgCuiCommunity::Ignore::OnMessage(UIWidget *context, const UIMessage &message)
{
	bool result = true;

	if (context == m_nameTextBox)
	{
		if ((message.Keystroke == UIMessage::Enter) &&
		    (message.Type == UIMessage::KeyDown))
		{
			result = false;

			addIgnoreName();
		}
	}

	return result;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Ignore::addIgnoreName() const
{
	// Tell the game this person needs to be ignored

	Unicode::String name;

	m_nameTextBox->GetLocalText(name);

	CommunityManager::addIgnore(name);

	m_nameTextBox->SetLocalText(Unicode::emptyString);
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Ignore::removeSelectedItems()
{
	// Figure out which and remove the current selected items

	SwgCuiCommunity::UnicodeStringVector removedItems;
	
	m_communityMediator.removeSelectedItemsFromList(*m_ignoreList, removedItems);

	// Tell the game we want these players to not be ignored anymore

	SwgCuiCommunity::UnicodeStringVector::iterator iterRemovedItems = removedItems.begin();

	for (; iterRemovedItems != removedItems.end(); ++iterRemovedItems)
	{
		Unicode::String const &name = (*iterRemovedItems);

		CommunityManager::removeIgnore(name);
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Ignore::onIgnoreListChanged(CommunityManager::Messages::IgnoreListChanged::Status const &)
{
	buildIgnoreList();
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Ignore::buildIgnoreList()
{
	CommunityManager::IgnoreList const &ignoreList = CommunityManager::getIgnoreList();

	// Mark that the friend list has been populated at least once

	m_ignoreListPopulated = true;

	// Clear the old list

	m_ignoreList->Clear();

	// Setup the current state of the list

	CommunityManager::IgnoreList::const_iterator iterIgnoreList = ignoreList.begin();

	for (; iterIgnoreList != ignoreList.end(); ++iterIgnoreList)
	{
		Unicode::String const &name = iterIgnoreList->second;

		if (!name.empty())
		{
			m_communityMediator.addItemToSortedList(*m_ignoreList, name, Unicode::wideToNarrow(name));
		}
		else
		{
			DEBUG_REPORT_LOG(true, ("SwgCuiCommunity::Ignore::onIgnoreListChanged() - Trying to add an empty name to the ignore list."));
		}
	}
}

// ======================================================================
