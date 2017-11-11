//======================================================================
//
// SwgCuiChatRoomsInvite.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiChatRoomsInvite_H
#define INCLUDED_SwgCuiChatRoomsInvite_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class ChatRoomDataNode;
class CuiChatRoomDataNode;
class UIButton;
class UIPage;
class UITable;
class UIText;
class UITextbox;
struct ChatAvatarId;
struct ChatRoomData;


//----------------------------------------------------------------------

class SwgCuiChatRoomsInvite :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                          SwgCuiChatRoomsInvite     (UIPage & page);
	void                              OnButtonPressed           (UIWidget *context );
	static SwgCuiChatRoomsInvite *    createInto                (UIPage & parent);
	static SwgCuiChatRoomsInvite *    getMediatorForRoom        (CuiWorkspace & ws, uint32 roomId);
	static SwgCuiChatRoomsInvite *    findMediatorForRoom       (CuiWorkspace & ws, uint32 roomId);
	void                              setRoomId                 (uint32 roomId);

protected:

	void                              performActivate ();

private:
	                                 ~SwgCuiChatRoomsInvite ();
	                                  SwgCuiChatRoomsInvite (const SwgCuiChatRoomsInvite &);
	SwgCuiChatRoomsInvite &           operator=             (const SwgCuiChatRoomsInvite &);
	
	UIButton *                        m_buttonInvite;
	UIButton *                        m_buttonInviteGroup;
	UIButton *                        m_buttonCancel;
	UITextbox *                       m_textbox;
	UIText *                          m_textRoom;

	uint32                            m_roomId;
};

//======================================================================

#endif
