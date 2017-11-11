// ======================================================================
//
// SwgCuiCommunity_Ignore.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommunity_Ignore_H
#define INCLUDED_SwgCuiCommunity_Ignore_H

#include "clientGame/CommunityManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiCommunity.h"
#include "UIEventCallback.h"

class UIButton;
class UIList;
class UITextbox;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiCommunity::Ignore : public CuiMediator
                              , public UIEventCallback
{
public:

	Ignore(UIPage &page, SwgCuiCommunity const &communityMediator);

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnButtonPressed(UIWidget *context);
	virtual bool OnMessage(UIWidget *context, const UIMessage &message);

	void onIgnoreListChanged(CommunityManager::Messages::IgnoreListChanged::Status const &staus);

private:
	
	void removeSelectedItems();
	void addIgnoreName() const;
	void buildIgnoreList();

	SwgCuiCommunity const &    m_communityMediator;
	UIButton *                 m_removeButton;
	UIButton *                 m_addButton;
	UITextbox *                m_nameTextBox;
	UIList *                   m_ignoreList;
	MessageDispatch::Callback *m_callBack;
	bool                       m_ignoreListPopulated;

	// Disabled

	~Ignore();
	Ignore(Ignore const &rhs);
	Ignore &operator =(Ignore const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommunity_Ignore_H
