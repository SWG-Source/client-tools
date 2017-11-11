// ======================================================================
//
// SwgCuiOptKeymap.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiOptKeymap_H
#define INCLUDED_SwgCuiOptKeymap_H

#include "UIEventCallback.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedMessageDispatch/Receiver.h"

class CuiControlsMenuBindEntry;
class CuiMessageBox;
class InputMap;
class UIButton;
class UICheckbox;
class UIComboBox;
class UIDataSource;
class UIPage;
class UITable;
class UITableModelDefault;

#include "swgClientUserInterface/SwgCuiOptBase.h"

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiOptKeymap :
public SwgCuiOptBase,
public MessageDispatch::Receiver
{
public:

	explicit                 SwgCuiOptKeymap (UIPage & page, Game::SceneType sceneType);

	// Control specific notifications.
	virtual void OnButtonPressed               (UIWidget * context );
	virtual bool OnMessage                     (UIWidget * context, const UIMessage & msg );
	virtual void OnGenericSelectionChanged     (UIWidget * context);
	virtual void OnTabbedPaneChanged           (UIWidget * context);
	virtual void OnCheckboxSet                 (UIWidget * context);
	virtual void OnCheckboxUnset               (UIWidget * context);

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

	void                     updateData        ();

	bool                     doApply           ();
	void                     receiveMessage    (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	virtual bool             close             ();
	void					onSceneChange(bool const &);
	void                     onInputSchemeReset ();
	void                     onModalChatConfirmClosed     (const CuiMessageBox & box);

	virtual void             revert            ();

private:
	virtual                 ~SwgCuiOptKeymap ();
	                         SwgCuiOptKeymap ();
	                         SwgCuiOptKeymap (const SwgCuiOptKeymap & rhs);
	SwgCuiOptKeymap &     operator=          (const SwgCuiOptKeymap & rhs);


private:

	void                     resetTabs    ();
	void                     resetPresets ();
	void                     resetChecks  ();
	void                     handleCheck  (UICheckbox & box);
	void resetDefaults();

	UIButton *               m_buttonDelete;
	UIButton *               m_buttonRebind;

	UIPage *                 m_entryPage;

	CuiControlsMenuBindEntry * m_bindEntry;
	InputMap *               m_workingInputMap;
	InputMap *               m_originalInputMap;

	UITable *                m_tableBinds;
	UITableModelDefault *    m_tableModelBinds;

	UIComboBox *             m_comboPreset;
	UITabbedPane *           m_tabBinds;

	UICheckbox *             m_checkMouseMode;
	UICheckbox *             m_checkModalChat;
	UICheckbox *             m_checkChaseCam;
	UICheckbox *             m_checkTurnStrafes;
	UICheckbox *             m_checkCanFireSecondariesFromToolbar;
	UICheckbox *             m_middleMouseDrivesMovement;
	UICheckbox *             m_mouseLeftAndRightDrivesMovement;
	UICheckbox *             m_scrollThroughDefaultActions;

	class InputSchemeCallbackReceiver;

	InputSchemeCallbackReceiver * m_callbackReceiver;
	CuiMessageBox *               m_messageBoxModalChatConfirm;
	std::string                   m_lastSelectedCategory;

	Game::SceneType          m_sceneType;
};

// ======================================================================

#endif
