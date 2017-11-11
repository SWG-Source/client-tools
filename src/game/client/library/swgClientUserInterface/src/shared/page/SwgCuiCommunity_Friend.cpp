// ======================================================================
//
// SwgCuiCommunity_Friend.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommunity_Friend.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/CommunityManager_FriendData.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GuildObject.h"
#include "clientGame/MatchMakingManager.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsCommunity.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/TextIterator.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "swgClientUserInterface/SwgCuiPersistentMessageComposer.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIMessage.h"
#include "UITextbox.h"
#include "UIPage.h"
#include "UITable.h"
#include "UITableModelDefault.h"

// ======================================================================

namespace SwgCuiCommunityFriendNamespace
{
	bool s_friendRemovalConfirmation = false;
};

using namespace SwgCuiCommunityFriendNamespace;

// ======================================================================

SwgCuiCommunity::Friend::Friend(UIPage &page, SwgCuiCommunity const &communityMediator)
 : CuiMediator("SwgCuiCommunity_Friend", page)
 , UIEventCallback()
 , m_communityMediator(communityMediator)
 , m_removeButton(NULL)
 , m_mailButton(NULL)
 , m_selectAllButton(NULL)
 , m_addButton(NULL)
 , m_clearButton(NULL)
 , m_nameTextBox(NULL)
 , m_groupTextBox(NULL)
 , m_groupComboBox(NULL)
 , m_commentTextBox(NULL)
 , m_notifyOnlineStatusCheckBox(NULL)
 , m_hideOfflineCheckBox(NULL)
 , m_removalConfirmationCheckBox(NULL)
 , m_friendTable(NULL)
 , m_callBack(new MessageDispatch::Callback)
 , m_friendListPopulated(false)
 , m_currentEntryModified(false)
{
	m_callBack->connect(*this, &SwgCuiCommunity::Friend::onFriendListChanged, static_cast<CommunityManager::Messages::FriendListChanged *>(0));
	m_callBack->connect(*this, &SwgCuiCommunity::Friend::onFriendOnlineStatusChanged, static_cast<CommunityManager::Messages::FriendOnlineStatusChanged *>(0));

	getCodeDataObject(TUIButton, m_removeButton, "buttonRemove");
	registerMediatorObject(*m_removeButton, true);

	getCodeDataObject(TUIButton, m_mailButton, "buttonMailGroup");
	registerMediatorObject(*m_mailButton, true);

	getCodeDataObject(TUIButton, m_selectAllButton, "buttonSelectAll");
	registerMediatorObject(*m_selectAllButton, true);

	getCodeDataObject(TUIButton, m_addButton, "buttonAdd");
	registerMediatorObject(*m_addButton, true);

	getCodeDataObject(TUIButton, m_clearButton, "buttonClear");
	registerMediatorObject(*m_clearButton, true);

	getCodeDataObject(TUITextbox, m_nameTextBox, "textboxName");
	registerMediatorObject(*m_nameTextBox, true);
	m_nameTextBox->SetEditable(true);
	m_nameTextBox->SetLocalText(Unicode::emptyString);
	m_nameTextBox->SetMaximumCharacters(32);

	getCodeDataObject(TUITextbox, m_groupTextBox, "textboxGroup");
	registerMediatorObject(*m_groupTextBox, true);
	m_groupTextBox->SetEditable(true);
	m_groupTextBox->SetLocalText(Unicode::emptyString);
	m_groupTextBox->SetMaximumCharacters(16);

	getCodeDataObject(TUIComboBox, m_groupComboBox, "comboGroup");
	registerMediatorObject(*m_groupComboBox, true);

	getCodeDataObject(TUITextbox, m_commentTextBox, "textboxComment");
	registerMediatorObject(*m_commentTextBox, true);
	m_commentTextBox->SetEditable(true);
	m_commentTextBox->SetLocalText(Unicode::emptyString);
	m_commentTextBox->SetMaximumCharacters(32);

	getCodeDataObject(TUICheckbox, m_notifyOnlineStatusCheckBox, "checkNotify");
	registerMediatorObject(*m_notifyOnlineStatusCheckBox, true);

	getCodeDataObject(TUICheckbox, m_hideOfflineCheckBox, "checkHideOffline");
	registerMediatorObject(*m_hideOfflineCheckBox, true);

	getCodeDataObject(TUICheckbox, m_removalConfirmationCheckBox, "checkRemoveConfirmation");
	registerMediatorObject(*m_removalConfirmationCheckBox, true);

	getCodeDataObject(TUITable, m_friendTable, "tableFriends");
	registerMediatorObject(*m_friendTable, true);

	m_friendTable->SetPropertyBoolean(UITable::PropertyName::SelectionAllowedMultiRow, true);

	// Clear out any data from the UI builder

	UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_friendTable->GetTableModel());

	if (tableModel != NULL)
	{
		tableModel->ClearTable();
	}

	CurrentUserOptionManager::registerOption(s_friendRemovalConfirmation, "SwgClientUserInterface", "FriendRemovalConfirmation");
}

//-----------------------------------------------------------------

SwgCuiCommunity::Friend::~Friend()
{
	delete m_callBack;
	m_callBack = NULL;

	m_removeButton = NULL;
	m_mailButton = NULL;
	m_selectAllButton = NULL;
	m_addButton = NULL;
	m_clearButton = NULL;
	m_nameTextBox = NULL;
	m_groupTextBox = NULL;
	m_groupComboBox = NULL;
	m_commentTextBox = NULL;
	m_notifyOnlineStatusCheckBox = NULL;
	m_hideOfflineCheckBox = NULL;
	m_removalConfirmationCheckBox = NULL;
	m_friendTable = NULL;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::performActivate()
{
	if (!m_friendListPopulated)
	{
		// Make sure the list has been populated at least once, this code
		// gets hit as a result of the community ui has never being opened
		// before the friend list is synchronized first.

		buildFriendList();
	}

	// Unselect all selections when opened

	m_friendTable->SelectRow(-1);
	m_removeButton->SetEnabled(false);

	m_hideOfflineCheckBox->SetChecked(CommunityManager::isHideOfflineFriends());
	m_removalConfirmationCheckBox->SetChecked(s_friendRemovalConfirmation);

	m_currentEntryModified = false;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::performDeactivate()
{
	if(m_currentEntryModified)
		doConfirmCloseWithoutSave();
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::removeSelectedFriends()
{
	// Remove all the selected items in the table

	UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_friendTable->GetTableModel());

	if (tableModel != NULL)
	{
		for (int row = 0; row < m_friendTable->GetRowCount(); ++row)
		{
			if (m_friendTable->IsRowSelected(row))
			{
				Unicode::String name;

				if (tableModel->GetValueAtText(row, static_cast<int>(C_name), name))
				{
					TextIterator textIterator(name);

					removeFriendFromServer(textIterator.getPrintableText());
				}
			}
		}
	}
	m_currentEntryModified = false;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::OnButtonPressed(UIWidget *context)
{
	if (context == m_removeButton)
	{
		if (m_removalConfirmationCheckBox->IsChecked())
		{
			Unicode::String friendList;

			// Build a list of selected friends

			UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_friendTable->GetTableModel());

			if (tableModel != NULL)
			{
				for (int row = 0; row < m_friendTable->GetRowCount(); ++row)
				{
					if (m_friendTable->IsRowSelected(row))
					{
						Unicode::String name;

						if (tableModel->GetValueAtText(row, static_cast<int>(C_name), name))
						{
							TextIterator textIterator(name);

							if (!friendList.empty())
							{
								friendList.append(1, ',');
								friendList.append(1, ' ');
							}

							friendList += textIterator.getPrintableText();
						}
					}
				}
			}

			// Make sure there is a friend selected

			if (!friendList.empty())
			{
				// Confirm friend removal

				CuiStringVariablesData data;
				data.sourceName = friendList;
				Unicode::String removeFriendsString;
				CuiStringVariablesManager::process(CuiStringIdsCommunity::friend_remove_confirmation, data, removeFriendsString);

				CuiMessageBox *messageBox = CuiMessageBox::createYesNoBox(removeFriendsString);
				m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiCommunity::Friend::onRemoveSelectedFriends);
			}
		}
		else
		{
			removeSelectedFriends();
		}
	}
	else if (context == m_mailButton)
	{
		std::string recipients;
		Unicode::String subject;
		Unicode::String body;

		// Build the recipient list

		UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_friendTable->GetTableModel());

		if (tableModel != NULL)
		{
			for (int row = 0; row < m_friendTable->GetRowCount(); ++row)
			{
				if (m_friendTable->IsRowSelected(row))
				{
					Unicode::String name;

					if (tableModel->GetValueAtText(row, static_cast<int>(C_name), name))
					{
						if (!recipients.empty())
						{
							recipients.append(1, ';');
							recipients.append(1, ' ');
						}

						TextIterator textIterator(name);
						recipients += Unicode::wideToNarrow(textIterator.getPrintableText());
					}
				}
			}
		}

		// Create the email

		SwgCuiPersistentMessageComposer::setNextParameters(recipients, subject, body, Unicode::emptyString);
		IGNORE_RETURN(CuiActionManager::performAction(CuiActions::persistentMessageComposer, Unicode::emptyString));
	}
	else if (context == m_selectAllButton)
	{
		for (int row = 0; row < m_friendTable->GetRowCount(); ++row)
		{
			m_friendTable->AddRowSelection(row);
		}
	}
	else if (context == m_addButton)
	{
		addFriend();
	}
	else if (context == m_clearButton)
	{
		m_nameTextBox->SetLocalText(Unicode::emptyString);
		OnTextboxChanged(m_nameTextBox);
		m_groupTextBox->SetLocalText(Unicode::emptyString);
		m_commentTextBox->SetLocalText(Unicode::emptyString);
		m_notifyOnlineStatusCheckBox->SetChecked(true);
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::addFriend() const
{
	Unicode::String name;
	m_nameTextBox->GetLocalText(name);

	Unicode::String group;
	m_groupTextBox->GetLocalText(group);

	Unicode::String comment;
	m_commentTextBox->GetLocalText(comment);

	bool const notifyOnlineStatus = m_notifyOnlineStatusCheckBox->IsChecked();

	m_currentEntryModified = false;
	CommunityManager::addFriend(name, group, comment, notifyOnlineStatus);
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::removeFriendFromServer(Unicode::String const &name) const
{
	CommunityManager::removeFriend(name);
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::buildFriendList()
{
	CommunityManager::FriendList const &friendList = CommunityManager::getFriendList();

	int offlineFriendCount = 0;

	// Mark that the friend list has been populated at least once

	m_friendListPopulated = true;

	// Build a group list to populate the group combo box with

	typedef std::set<Unicode::String> GroupList;
	GroupList groupList;

	// If the player is in a guild, insert their guild abbrev by default

	CreatureObject const * const playerCreature = Game::getPlayerCreature();

	if ((playerCreature != NULL) &&
	    (playerCreature->getGuildId() != 0))
	{
		Unicode::String guildAbbreviation;

		guildAbbreviation.append(1, '<');
		guildAbbreviation += GuildObject::getGuildAbbrevUnicode(playerCreature->getGuildId());
		guildAbbreviation.append(1, '>');
		groupList.insert(guildAbbreviation);
	}

	// Populate the table

	UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_friendTable->GetTableModel());

	if (tableModel != NULL)
	{
		Unicode::String lastSelectedName;
		tableModel->GetValueAtText(m_friendTable->GetLastSelectedRow(), static_cast<int>(C_name), lastSelectedName);
		TextIterator textIterator(lastSelectedName);
		lastSelectedName = textIterator.getPrintableText();
		m_friendTable->SelectRow(-1);
		tableModel->ClearTable();

		CommunityManager::FriendList::const_iterator iterFriendList = friendList.begin();

		for (; iterFriendList != friendList.end(); ++iterFriendList)
		{
			CommunityManager::FriendData const &friendData = iterFriendList->second;

			// Possibly skip offline friends

			if (   CommunityManager::isHideOfflineFriends()
			    && !friendData.isOnline())
			{
				++offlineFriendCount;
				continue;
			}

			for (int column = 0; column < static_cast<int>(C_count); ++column)
			{
				switch (column)
				{
					case C_name:
						{
							Unicode::String text;

							if (friendData.isOnline())
							{
								text += ClientTextManager::getColorCode(ClientTextManager::TT_onlineStatus);
								text += friendData.getName();
								text += ClientTextManager::getResetTagCode();
							}
							else
							{
								text += friendData.getName();
							}

							IGNORE_RETURN(tableModel->AppendCell(column, NULL, text));

							if (friendData.getName() == lastSelectedName)
							{
								m_friendTable->SelectRow(m_friendTable->GetRowCount() - 1);
							}
						}
						break;
					case C_onlineStatus:
						{
							Unicode::String text;
							
							if (friendData.isOnline())
							{
								text += ClientTextManager::getColorCode(ClientTextManager::TT_onlineStatus);
								text += CuiStringIdsCommunity::friend_status_online.localize();
								text += ClientTextManager::getResetTagCode();
							}
							else
							{
								text += CuiStringIdsCommunity::friend_status_offline.localize();
							}

							if (friendData.isNotifyOnlineStatus())
							{
								text.append(1, '*');
							}

							IGNORE_RETURN(tableModel->AppendCell(column, NULL, text));
						}
						break;
					case C_group:
						{
							IGNORE_RETURN(tableModel->AppendCell(column, NULL, friendData.getGroup()));

							groupList.insert(friendData.getGroup());
						}
						break;
					case C_comment:
						{
							IGNORE_RETURN(tableModel->AppendCell(column, NULL, friendData.getComment()));
						}
						break;
					default:
						{
							DEBUG_FATAL(true, ("Invalid column specified."));
						}
						break;
				}
			}
		}
	}

	// Populate the group combo box with all existing group entries

	GroupList::const_iterator iterGroupList = groupList.begin();

	m_groupComboBox->SetSelectedIndex(-1);
	m_groupComboBox->Clear();
	m_groupComboBox->AddItem(Unicode::emptyString, "");

	for (; iterGroupList != groupList.end(); ++iterGroupList)
	{
		Unicode::String const &entry = (*iterGroupList);

		if (!entry.empty())
		{
			m_groupComboBox->AddItem(entry, "");
		}
	}

	// Set the number of offline friends

	CuiStringVariablesData data;
	data.digit_i = offlineFriendCount;
	Unicode::String hiddenFriendCountString;
	CuiStringVariablesManager::process(CuiStringIdsCommunity::friend_list_hide_offline, data, hiddenFriendCountString);
	m_hideOfflineCheckBox->SetLocalText(hiddenFriendCountString);
}

//-----------------------------------------------------------------

bool SwgCuiCommunity::Friend::OnMessage(UIWidget *context, const UIMessage &message)
{
	bool result = true;

	if (   (context == m_nameTextBox)
	    || (context == m_groupTextBox)
	    || (context == m_commentTextBox))
	{
		if ((message.Keystroke == UIMessage::Enter) &&
		    (message.Type == UIMessage::KeyDown))
		{
			result = false;

			addFriend();
		}
	}

	return result;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::onFriendListChanged(CommunityManager::Messages::FriendListChanged::Status const &)
{
	buildFriendList();
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::onFriendOnlineStatusChanged(CommunityManager::Messages::FriendOnlineStatusChanged::Name const &name)
{
	// Set the online status

	UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_friendTable->GetTableModel());

	if (tableModel != NULL)
	{
		for (int row = 0; row < m_friendTable->GetRowCount(); ++row)
		{
			Unicode::String localText;

			if (tableModel->GetValueAtText(row, static_cast<int>(C_name), localText))
			{
				TextIterator textIterator(localText);
				localText = textIterator.getPrintableText();

				if (localText == name)
				{
					if (CommunityManager::isFriendOnline(name))
					{
						// Name

						Unicode::String text;
						text += ClientTextManager::getColorCode(ClientTextManager::TT_onlineStatus);
						text += name;
						text += ClientTextManager::getResetTagCode();
						tableModel->SetValueAtText(row, static_cast<int>(C_name), text);

						// Online status

						text.clear();
						text += ClientTextManager::getColorCode(ClientTextManager::TT_onlineStatus);
						text += CuiStringIdsCommunity::friend_status_online.localize();
						text += ClientTextManager::getResetTagCode();
						if (CommunityManager::isNotifyOnlineStatus(name))
						{
							text.append(1, '*');
						}

						tableModel->SetValueAtText(row, static_cast<int>(C_onlineStatus), text);
					}
					else
					{
						// Name

						tableModel->SetValueAtText(row, static_cast<int>(C_name), localText);

						// Online status

						Unicode::String text;
						text += CuiStringIdsCommunity::friend_status_offline.localize();
						if (CommunityManager::isNotifyOnlineStatus(name))
						{
							text.append(1, '*');
						}

						tableModel->SetValueAtText(row, static_cast<int>(C_onlineStatus), text);
					}

					break;
				}
			}
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::OnGenericSelectionChanged(UIWidget *context)
{
	if (context == m_friendTable)
	{
		if(m_currentEntryModified)
		{
			doConfirmChangeWithoutSave();
			return;
		}
		int const selectedRow = m_friendTable->GetLastSelectedRow();

		if (   (selectedRow >= 0)
		    && (selectedRow < m_friendTable->GetRowCount()))
		{
			m_removeButton->SetEnabled(true);

			// Set the group combo to an invalid value

			m_groupComboBox->SetSelectedIndex(-1);
			
			// Fill in the name, group and comment from the selected
			// table entry

			UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_friendTable->GetTableModel());

			if (tableModel != NULL)
			{
				Unicode::String name;

				if (tableModel->GetValueAtText(selectedRow, static_cast<int>(C_name), name))
				{
					TextIterator textIterator(name);

					name = textIterator.getPrintableText();

					m_nameTextBox->SetLocalText(name);
					OnTextboxChanged(m_nameTextBox);

					Unicode::String group;

					if (CommunityManager::getFriendGroup(name, group))
					{
						m_groupTextBox->SetLocalText(group);
					}
					else
					{
						m_groupTextBox->SetLocalText(Unicode::emptyString);
					}

					Unicode::String comment;

					if (CommunityManager::getFriendComment(name, comment))
					{
						m_commentTextBox->SetLocalText(comment);
					}
					else
					{
						m_commentTextBox->SetLocalText(Unicode::emptyString);
					}

					m_notifyOnlineStatusCheckBox->SetChecked(CommunityManager::isNotifyOnlineStatus(name));
				}
			}
		}
		else
		{
			m_removeButton->SetEnabled(false);
		}
		m_currentEntryModified = false;
	}
	else if (context == m_groupComboBox)
	{
		Unicode::String group;
		m_groupComboBox->GetSelectedIndexLocalText(group);
		m_groupTextBox->SetLocalText(group);
		m_currentEntryModified = true;
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::OnCheckboxSet(UIWidget *context)
{
	if (context == m_hideOfflineCheckBox)
	{
		CommunityManager::setHideOfflineFriends(m_hideOfflineCheckBox->IsChecked());
		buildFriendList();
	}
	else if (context == m_removalConfirmationCheckBox)
	{
		s_friendRemovalConfirmation = m_removalConfirmationCheckBox->IsChecked();
	}
	else if (context == m_notifyOnlineStatusCheckBox)
	{
		m_currentEntryModified = true;
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::OnCheckboxUnset(UIWidget *context)
{
	if (context == m_hideOfflineCheckBox)
	{
		CommunityManager::setHideOfflineFriends(m_hideOfflineCheckBox->IsChecked());
		buildFriendList();
	}
	else if (context == m_removalConfirmationCheckBox)
	{
		s_friendRemovalConfirmation = m_removalConfirmationCheckBox->IsChecked();
	}
	else if (context == m_notifyOnlineStatusCheckBox)
	{
		m_currentEntryModified = true;
	}
}

//-----------------------------------------------------------------
void SwgCuiCommunity::Friend::onRemoveSelectedFriends(const CuiMessageBox &messageBox)
{
	if (messageBox.completedAffirmative())
	{
		removeSelectedFriends();
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::onChangeWithoutSaving(const CuiMessageBox &messageBox)
{
	m_currentEntryModified = false;
	if (messageBox.completedAffirmative())
	{
		addFriend();
	}
	OnGenericSelectionChanged(m_friendTable);
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::onCloseWithoutSaving(const CuiMessageBox &messageBox)
{
	m_currentEntryModified = false;
	if (messageBox.completedAffirmative())
	{
		addFriend();
	}
	deactivate();
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::OnTextboxChanged(UIWidget *context)
{
	if (context == m_nameTextBox)
	{
		if (CommunityManager::isFriend(m_nameTextBox->GetLocalText()))
		{
			m_addButton->SetLocalText(CuiStringIdsCommunity::friend_list_modify.localize());
		}
		else
		{
			m_addButton->SetLocalText(CuiStringIdsCommunity::friend_list_add.localize());
		}
	}

	if((context == m_nameTextBox) || (context == m_groupTextBox) || (context == m_commentTextBox))
		m_currentEntryModified = true;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::doConfirmChangeWithoutSave()
{
	CuiMessageBox *messageBox = CuiMessageBox::createYesNoBox(CuiStringIds::change_friend_without_saving.localize());
	m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiCommunity::Friend::onChangeWithoutSaving);
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Friend::doConfirmCloseWithoutSave()
{
	CuiMessageBox *messageBox = CuiMessageBox::createYesNoBox(CuiStringIds::close_friend_without_saving.localize());
	m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiCommunity::Friend::onCloseWithoutSaving);
}
// ======================================================================
