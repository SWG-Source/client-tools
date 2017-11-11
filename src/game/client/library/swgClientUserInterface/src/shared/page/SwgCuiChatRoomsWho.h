//======================================================================
//
// SwgCuiChatRoomsWho.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiChatRoomsWho_H
#define INCLUDED_SwgCuiChatRoomsWho_H

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

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiChatRoomsWho :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                       SwgCuiChatRoomsWho        (UIPage & page);

	void                           performActivate           ();
	void                           performDeactivate         ();

	void                           OnButtonPressed           (UIWidget *context );
	void                           OnGenericSelectionChanged (UIWidget *context );

	static SwgCuiChatRoomsWho *    createInto                (UIPage & parent);
	static SwgCuiChatRoomsWho *    getMediatorForRoom        (uint32 roomId);

	void                           resetTable                ();
	void                           resetInfo                 ();

	void                           onRoomModified            (const uint32 & roomId);

	void                           setRoomId                 (uint32 roomId);

	ChatAvatarId                   getSelectedAvatar         () const;

private:
	                              ~SwgCuiChatRoomsWho ();
	                               SwgCuiChatRoomsWho (const SwgCuiChatRoomsWho &);
	SwgCuiChatRoomsWho &           operator=             (const SwgCuiChatRoomsWho &);
	
	void                           invite     ();
	void                           uninvite   ();
	void                           kick       ();
	void                           op         (bool b);
	void                           ban        ();
	void                           unban      ();

	UIButton *                     m_buttonChatRoom;
	UIButton *                     m_buttonDeOp;
	UIButton *                     m_buttonInvite;
	UIButton *                     m_buttonKick;
	UIButton *                     m_buttonOk;
	UIButton *                     m_buttonOp;
	UIButton *                     m_buttonUninvite;
	UIButton *                     m_buttonBan;
	UIButton *                     m_buttonUnban;

	UITable *                      m_table;

	UIText *                       m_textName;
	UIText *                       m_textCreator;
	UIText *                       m_textModerated;
	UIText *                       m_textOwner;
	UIText *                       m_textPrivate;
	UIText *                       m_textTitle;

	uint32                         m_roomId;

	Unicode::String                m_pathIconPresent;
	Unicode::String                m_pathIconInvited;
	Unicode::String                m_pathIconModerator;
	Unicode::String                m_pathIconBanned;
	Unicode::String                m_pathIconInvitedBanned; // for error display

	MessageDispatch::Callback *    m_callback;
};

//======================================================================

#endif
