//======================================================================
//
// SwgCuiPersistentMessageBrowser.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiPersistentMessageBrowser_H
#define INCLUDED_SwgCuiPersistentMessageBrowser_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class  OutOfBandBase;
class  StringId;
class  SwgCuiAttachmentList;
class  UIButton;
class  UIPage;
class  UIPopupMenu;
class  UITable;
class  UITableModelDefault;
class  UIText;
class  UIVolumePage;
class  AttachmentData;
struct ChatAvatarId;
struct ChatPersistentMessageToClientData;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiPersistentMessageBrowser :
public CuiMediator,
public UIEventCallback
{
public:

	typedef stdvector<AttachmentData *>::fwd         Attachments;
	typedef ChatPersistentMessageToClientData        MessageData;
	typedef stdvector<const MessageData *>::fwd      DataVector;

	explicit                SwgCuiPersistentMessageBrowser (UIPage & page);

	virtual void            performActivate                ();
	virtual void            performDeactivate              ();

	virtual void            OnButtonPressed                (UIWidget * context);
	virtual void            OnGenericSelectionChanged      (UIWidget * context);
	virtual bool            OnMessage                      (UIWidget * context, const UIMessage & msg);	

	void                    onReceiveHeader                (const MessageData & data);
	void                    onReceiveBody                  (const MessageData & data);
	void                    onEmailListUpdated             (const bool & data);

	static SwgCuiPersistentMessageBrowser * createInto     (UIPage & parent);

	const MessageData *     getLastSelectedData            () const;
	const DataVector        getSelectedData                () const;

	void                    OnPopupMenuSelection           (UIWidget * context);
	void                    saveAttachment                 (int index);

	void                    update                         (float deltaTimeSecs);

private:
	                       ~SwgCuiPersistentMessageBrowser ();
	                        SwgCuiPersistentMessageBrowser (const SwgCuiPersistentMessageBrowser &);
	SwgCuiPersistentMessageBrowser &  operator=            (const SwgCuiPersistentMessageBrowser &);

	void                    update                         ();
	void                    updateBody                     ();
	void                    updateAttachments              ();
	void                    reply                          (const StringId & subjectPrefix, const ChatAvatarId & recipient);
	void                    deleteSelection                ();

	const MessageData *     getDataAtRow                   (const long visualRow) const;
	void                    appendMessageData              (const MessageData & messageData);
	void                    updateMessageData              (const MessageData & messageData);
	long                    findLogicalRowForMessageDataLinear (const MessageData & messageData) const;

	void                    handleDoubleClick              (int index);
	UIPopupMenu *           createContextMenu              (int index);


	UITable *               m_table;

	UIButton *              m_buttonOk;
	UIButton *              m_buttonNew;
	UIButton *              m_buttonDelete;
	UIButton *              m_buttonReply;
	UIButton *              m_buttonForward;
	UIText *                m_textBody;
	
	MessageDispatch::Callback *  m_callback;
	UITableModelDefault *   m_model;

	UIVolumePage *          m_volumeAttach;
	SwgCuiAttachmentList *  m_attachments;

	long                    m_lastSelectedRowVisual;
	long                    m_lastScrollPosY;
};

//======================================================================

#endif
