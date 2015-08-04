// ======================================================================
//
// SwgCuiCommunity_Friend.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommunity_Friend_H
#define INCLUDED_SwgCuiCommunity_Friend_H

#include "clientGame/CommunityManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "swgClientUserInterface/SwgCuiCommunity.h"

class CuiMessageBox;
class UIButton;
class UICheckbox;
class UIComboBox;
class UITable;
class UITextbox;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiCommunity::Friend : public CuiMediator, public UIEventCallback
{
public:

	Friend(UIPage &page, SwgCuiCommunity const &communityMediator);

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnButtonPressed(UIWidget *context);
	virtual void OnCheckboxSet(UIWidget *context);
	virtual void OnCheckboxUnset(UIWidget *context);
	virtual void OnGenericSelectionChanged(UIWidget *context);
	virtual bool OnMessage(UIWidget *context, const UIMessage &message);
	virtual void OnTextboxChanged(UIWidget *context);

	void onFriendListChanged(CommunityManager::Messages::FriendListChanged::Status const &status);
	void onFriendOnlineStatusChanged(CommunityManager::Messages::FriendOnlineStatusChanged::Name const &name);
	void onCloseWithoutSaving(const CuiMessageBox &messageBox);
	void onChangeWithoutSaving(const CuiMessageBox &messageBox);
	void onRemoveSelectedFriends(const CuiMessageBox &messageBox);

private:
	
	enum Column
	{
		C_name = 0,
		C_onlineStatus = 1,
		C_group = 2,
		C_comment = 3,
		C_count = 4
	};

	void addFriend() const;
	void removeSelectedFriends();
	void removeFriendFromServer(Unicode::String const &name) const;
	void buildFriendList();

	void doConfirmChangeWithoutSave();
	void doConfirmCloseWithoutSave();

	SwgCuiCommunity const &    m_communityMediator;
	UIButton *                 m_removeButton;
	UIButton *                 m_mailButton;
	UIButton *                 m_selectAllButton;
	UIButton *                 m_addButton;
	UIButton *                 m_clearButton;
	UITextbox *                m_nameTextBox;
	UITextbox *                m_groupTextBox;
	UIComboBox *               m_groupComboBox;
	UITextbox *                m_commentTextBox;
	UICheckbox *               m_notifyOnlineStatusCheckBox;
	UICheckbox *               m_hideOfflineCheckBox;
	UICheckbox *               m_removalConfirmationCheckBox;
	UITable *                  m_friendTable;
	MessageDispatch::Callback *m_callBack;
	bool                       m_friendListPopulated;
	mutable bool               m_currentEntryModified;

	// Disabled

	~Friend();
	Friend(Friend const &rhs);
	Friend &operator =(Friend const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommunity_Friend_H
