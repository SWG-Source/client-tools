//======================================================================
//
// SwgCuiChatWindow_TabEditor.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiChatWindow_TabEditor_H
#define INCLUDED_SwgCuiChatWindow_TabEditor_H

//======================================================================

#include "swgClientUserInterface/SwgCuiChatWindow.h"

class CuiChatRoomDataNode;
class UIButton;
class UIList;
class UIText;

//----------------------------------------------------------------------

class SwgCuiChatWindow::TabEditor :
public CuiMediator,
public UIEventCallback
{
public:

	//----------------------------------------------------------------------
	//-- messages emitted on transceivers when changes are accepted

	struct Messages
	{
		struct ChangeCompleted
		{
			typedef Tab Payload;
		};
	};

	//----------------------------------------------------------------------

	explicit            TabEditor                 (UIPage & page);

	void                OnGenericSelectionChanged (UIWidget * context);
	bool                OnMessage (UIWidget *context, const UIMessage & msg);

	void                OnButtonPressed           (UIWidget *context );

	static TabEditor *  createInto                (UIPage & parent);

	void                updateFromListState       ();

	void                setTab                    (const Tab & tab);
	int                 getTabId                  ();

	bool                handleChatRoomJoin        (const CuiChatRoomDataNode & roomNode);
	void                handleChatRoomLeave       (const CuiChatRoomDataNode & roomNode);

protected:
	void                performActivate      ();
	void                performDeactivate    ();

private:
	                   ~TabEditor            ();
	                    TabEditor            ();
	                    TabEditor            (const TabEditor &);
	TabEditor &         operator=            (const TabEditor &);

	void                reset                ();

	void                addChannel           ();
	void                removeChannel        ();
	void                joinChannel          ();
	void                setDefaultChannel    ();

	Tab *               m_tab;

	UIButton *          m_buttonOk;
	UIButton *          m_buttonCancel;
	UIButton *          m_buttonAdd;
	UIButton *          m_buttonRemove;
	UIButton *          m_buttonJoin;
	UIButton *          m_buttonDefault;

	UIList *            m_listAvailable;
	UIList *            m_listCurrent;

	UIText *            m_textDefault;

	UITextbox *         m_textboxSetName;

	bool                m_ignoreListChange;
};

//======================================================================

#endif
