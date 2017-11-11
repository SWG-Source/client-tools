//======================================================================
//
// SwgCuiChatRooms.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiChatRooms_H
#define INCLUDED_SwgCuiChatRooms_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "UINotification.h"

class ChatRoomDataNode;
class CuiChatRoomDataNode;
class CuiMessageBox;
class UIButton;
class UIImageStyle;
class UIPage;
class UISliderbar;
class UIText;
class UITreeView;
struct ChatRoomData;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiChatRooms :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                    SwgCuiChatRooms           (UIPage & page);

	virtual void                performActivate           ();
	virtual void                performDeactivate         ();

	virtual void                OnButtonPressed           (UIWidget *context );
	virtual void                OnGenericSelectionChanged (UIWidget * context);

	const CuiChatRoomDataNode * getSelectedChatRoom       () const;

	void                        updateRoomNode            (const CuiChatRoomDataNode & node);

	void                        selectChatRoom            (uint32 roomId);

	void                        onMessageBoxClosedConfirmDestroy (const CuiMessageBox & box);

private:
	                       ~SwgCuiChatRooms ();
	                        SwgCuiChatRooms (const SwgCuiChatRooms &);
	SwgCuiChatRooms &       operator= (const SwgCuiChatRooms &);

	void                    onRoomsModified (const uint32 &);

	void                    updateTree    ();
	void                    updateText    ();
	void                    updateButtons ();

	void                    join       ();

	void                    refresh    ();
	void                    leave      ();
	void                    create     ();
	void                    destroy    (uint32 roomId, bool confirmed);
	void                    who        ();

	UIButton *              m_buttonJoin;
	UIButton *              m_buttonLeave;
	UIButton *              m_buttonRefresh;
	UIButton *              m_buttonDone;
	UIButton *              m_buttonCreate;
	UIButton *              m_buttonDelete;
	UIButton *              m_buttonWho;

	UIText *                m_textPath;
	UIText *                m_textTitle;
	UIText *                m_textCreator;
	UIText *                m_textOwner;
	UIText *                m_textMembers;
	
	UITreeView *            m_tree;

	UISliderbar *           m_slider;

	MessageDispatch::Callback *  m_callback;

	bool                    m_sliderDirection;

	Unicode::String         m_iconJoinPath;

	uint32                  m_roomIdToDestroy;
};

//======================================================================

#endif
