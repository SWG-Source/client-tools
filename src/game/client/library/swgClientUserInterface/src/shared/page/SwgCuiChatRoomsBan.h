//======================================================================
//
// SwgCuiChatRoomsBan.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiChatRoomsBan_H
#define INCLUDED_SwgCuiChatRoomsBan_H

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

class SwgCuiChatRoomsBan :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                          SwgCuiChatRoomsBan        (UIPage & page);
	void                              OnButtonPressed           (UIWidget *context );
	static SwgCuiChatRoomsBan *       createInto                (UIPage & parent);
	static SwgCuiChatRoomsBan *       getMediatorForRoom        (CuiWorkspace & ws, uint32 roomId, const std::string & name);
	static SwgCuiChatRoomsBan *       findMediatorForRoom       (CuiWorkspace & ws, uint32 roomId);
	void                              setRoomId                 (uint32 roomId, const std::string & name);

protected:

	void                              performActivate ();

private:
	                                 ~SwgCuiChatRoomsBan ();
	                                  SwgCuiChatRoomsBan (const SwgCuiChatRoomsBan &);
	SwgCuiChatRoomsBan &              operator=             (const SwgCuiChatRoomsBan &);
	
	UIButton *                        m_buttonOk;
	UIButton *                        m_buttonCancel;
	UITextbox *                       m_textbox;
	UIText *                          m_textRoom;

	uint32                            m_roomId;
};

//======================================================================

#endif
