//======================================================================
//
// SwgCuiChatRoomsCreateCreate.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiChatRoomsCreateCreate_H
#define INCLUDED_SwgCuiChatRoomsCreateCreate_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class ChatRoomDataNode;
class CuiChatRoomDataNode;
class UIButton;
class UIPage;
class UIText;
class UITextbox;
class UICheckbox;
struct ChatRoomData;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiChatRoomsCreate :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                    SwgCuiChatRoomsCreate     (UIPage & page);

	void                        performActivate           ();
	void                        performDeactivate         ();

	void                        OnButtonPressed           (UIWidget *context );

	static SwgCuiChatRoomsCreate *    createInto          (UIPage & parent);

	void                        setRootPath               (const std::string & path);

private:
	                             ~SwgCuiChatRoomsCreate ();
	                              SwgCuiChatRoomsCreate (const SwgCuiChatRoomsCreate &);
	SwgCuiChatRoomsCreate &       operator=             (const SwgCuiChatRoomsCreate &);

	void                          ok ();

	UIButton *              m_buttonCancel;
	UIButton *              m_buttonOk;

	UIText *                m_textPath;

	UITextbox *             m_textboxName;
	UITextbox *             m_textboxTitle;

	UICheckbox *            m_checkPrivate;
	UICheckbox *            m_checkModerated;
};

//======================================================================

#endif
