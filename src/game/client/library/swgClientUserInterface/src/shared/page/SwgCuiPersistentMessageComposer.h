//======================================================================
//
// SwgCuiPersistentMessageComposer.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiPersistentMessageComposer_H
#define INCLUDED_SwgCuiPersistentMessageComposer_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class AttachmentData;
class ClientObject;
class UIButton;
class UIComboBox;
class UIPage;
class UIText;
class UITextbox;
class UIVolumePage;
class SwgCuiAttachmentList;

//----------------------------------------------------------------------

class SwgCuiPersistentMessageComposer :
public CuiMediator,
public UIEventCallback
{
public:

	typedef stdvector<AttachmentData>::fwd AttachmentVector;

	explicit                          SwgCuiPersistentMessageComposer (UIPage & page);

	virtual void                      performActivate      ();
	virtual void                      performDeactivate    ();

	virtual void                      OnButtonPressed      (UIWidget *context);
	virtual bool                      OnMessage            (UIWidget *context, const UIMessage & msg);
	virtual void                      OnPopupMenuSelection (UIWidget * context);

	void                              addAttachment        (const AttachmentData & ad);
	void                              addAttachment        (const ClientObject & obj);

	static SwgCuiPersistentMessageComposer * createInto    (UIPage & parent);

	static void                       setNextParameters    (const std::string & recepients, const Unicode::String & subject, const Unicode::String & body, const Unicode::String & oob);

	SwgCuiPersistentMessageComposer * findTopComposer      ();

	void                              processOob           (const Unicode::String & oob);

	void                              update               (float deltaTimeSecs);

private:

	                       ~SwgCuiPersistentMessageComposer ();
	                        SwgCuiPersistentMessageComposer (const SwgCuiPersistentMessageComposer &);
	SwgCuiPersistentMessageComposer &  operator=            (const SwgCuiPersistentMessageComposer &);

	void                    send ();

	void                    addIconForAttachment  (int index);
	void                    removeAttachment      (int index);
	void                    clearVolumeChildren   ();

	UIButton *              m_buttonOk;
	UIButton *              m_buttonCancel;

	UIComboBox *            m_comboboxRecepient;
	UITextbox *             m_textboxSubject;
	UIText *                m_textBody;
	UIText *                m_textMaxReached;

	UIVolumePage *          m_volumeAttach;

	SwgCuiAttachmentList *  m_attachments;

	static std::string      ms_nextRecepients;
	static Unicode::String  ms_nextSubject;
	static Unicode::String  ms_nextBody;
	static Unicode::String  ms_nextOob;
};

//======================================================================

#endif
