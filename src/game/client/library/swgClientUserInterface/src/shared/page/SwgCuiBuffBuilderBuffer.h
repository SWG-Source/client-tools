//======================================================================
//
// SwgCuiBuffBuilderBuffer.h
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiBuffBuilderBuffer_H
#define INCLUDED_SwgCuiBuffBuilderBuffer_H

#include "clientGame/PlayerCreatureController.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIList;
class UITextbox;
class UIText;
class UITreeView;
class CuiMessageBox;


namespace MessageDispatch
{
	class Callback;
};

// ======================================================================

class SwgCuiBuffBuilderBuffer :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                    SwgCuiBuffBuilderBuffer (UIPage & page);

	virtual void                performActivate           ();
	virtual void                performDeactivate         ();

	virtual bool				OnMessage				  (UIWidget *context, const UIMessage & msg );
	virtual void                OnButtonPressed           (UIWidget *context );
	virtual void                OnGenericSelectionChanged (UIWidget * context);
	virtual void				OnTextboxChanged(UIWidget * context);
	virtual bool close();

	void setRecipient(NetworkId const & recipientId);

	void onBuffBuilderChangeReceived(PlayerCreatureController::Messages::BuffBuilderChangeReceived::Payload const & payload);
	void onBuffBuilderCancelReceived(PlayerCreatureController::Messages::BuffBuilderChangeReceived::Payload const & payload);

private:

	~SwgCuiBuffBuilderBuffer ();
	SwgCuiBuffBuilderBuffer (const SwgCuiBuffBuilderBuffer &);
	SwgCuiBuffBuilderBuffer &  operator= (const SwgCuiBuffBuilderBuffer &);

	void setupPage();
	CreatureObject * getRecipientCreature() const;
	NetworkId const & getRecipientId() const;
	void buildAndSendUpdateToServer(bool accepted) const;
	void initializeBuffTree();
	void initializeExpertiseModifiers();
	void clearBuffList();
	void updateBuffListFromSession();
	void updateBuffeeListFromSession();
	void updatePointsFromSession();
	void updateAddRemoveButtons();
	void updateAcceptButton();
	void updateComponentDescription();
	int getPointsLeft();
	int getExpertiseSkillModValue(const std::string & expertiseName, const std::string & skillModName);
	int getExpertiseModifierForBuffComponent(const std::string & buffComponentName);

	void onVerifyPromptClosed(const CuiMessageBox & box);

	void addBuffToList();
	void removeBuffFromList();

	MessageDispatch::Callback *  m_callback;

	NetworkId m_recipientId;
	UIButton * m_clearButton;
	UIButton * m_cancelButton;
	UIButton * m_acceptButton;
	UIButton * m_addButton;
	UIButton * m_removeButton;
	UITreeView * m_buffTree;
	UIList * m_buffList;
	UIList * m_buffeeList;
	UITextbox * m_coverChargeTextBox;
	UIText * m_pointsLeft;
	UIText * m_totalCost;
	UIText * m_componentDescription;
	UIText * m_recipientName;

	bool m_committed;

	static bool m_failedLastVerification;
};

// ======================================================================

#endif
