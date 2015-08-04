//======================================================================
//
// SwgCuiPersistentMessageComposer.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiPersistentMessageComposer.h"

//======================================================================


#include "UIButton.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "clientGame/AttachmentData.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiStringIdsPersistentMessage.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/OutOfBand.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ChatPersistentMessageToServer.h"
#include "swgClientUserInterface/SwgCuiAttachmentList.h"
#include <algorithm>

//----------------------------------------------------------------------

namespace
{
	namespace PopupItems
	{
		const std::string del     = "popup_del";
	};

	namespace Properties
	{
		const UILowerString PopupChildIndex ("PopupChildIndex");
	}

	typedef stdvector<SwgCuiPersistentMessageComposer *>::fwd ComposerVector;
	ComposerVector s_composers;

	class MyCallback : 
	public MessageDispatch::Callback
	{
	public:
		void onAttach (const ClientObject & obj)
		{
			if (!s_composers.empty ())
			{
				SwgCuiPersistentMessageComposer * const composer = s_composers.back ();
				NOT_NULL (composer);

				SwgCuiPersistentMessageComposer * const topComposer = composer->findTopComposer ();
				NOT_NULL (topComposer);

				topComposer->addAttachment (obj);
			}
		}
	};

	MyCallback * s_callback = 0;
};

//----------------------------------------------------------------------

std::string      SwgCuiPersistentMessageComposer::ms_nextRecepients;
Unicode::String  SwgCuiPersistentMessageComposer::ms_nextSubject;
Unicode::String  SwgCuiPersistentMessageComposer::ms_nextBody;
Unicode::String  SwgCuiPersistentMessageComposer::ms_nextOob;

//----------------------------------------------------------------------

SwgCuiPersistentMessageComposer::SwgCuiPersistentMessageComposer (UIPage & page) :
CuiMediator        ("SwgCuiPersistentMessageComposer", page),
UIEventCallback    (),
m_buttonOk         (0),
m_buttonCancel     (0),
m_comboboxRecepient(0),	
m_textboxSubject   (0),
m_textBody         (0),
m_volumeAttach     (0),
m_attachments      (new SwgCuiAttachmentList)
{
	getCodeDataObject (TUIButton,     m_buttonOk,         "buttonOk");
	getCodeDataObject (TUIButton,     m_buttonCancel,     "buttonCancel");
	getCodeDataObject (TUIComboBox,   m_comboboxRecepient,"comborecipient");
	getCodeDataObject (TUITextbox,    m_textboxSubject,   "textboxSubject");
	getCodeDataObject (TUIText,       m_textBody,         "textBody");
	getCodeDataObject (TUIVolumePage, m_volumeAttach,     "volumeAttach");
	getCodeDataObject (TUIText,       m_textMaxReached,   "textmaxreached");

	m_textBody->SetPreLocalized (true);
	m_textBody->SetEditable     (true);
	m_textBody->SetEnabled      (true);

	m_textBody->SetPreLocalized (true);

	m_textBody->SetMaximumCharacters(ChatPersistentMessageToServerNamespace::MAX_MESSAGE_SIZE);

	m_textBody->Clear     ();
	m_volumeAttach->Clear ();

	// populate recipient combo box with predefined recipients
	m_comboboxRecepient->Clear();

	std::set<std::string> const & predefinedRecipients = CuiPersistentMessageManager::getPredefinedRecipients();
	for (std::set<std::string>::const_iterator iterRecipient = predefinedRecipients.begin(); iterRecipient != predefinedRecipients.end(); ++iterRecipient)
		m_comboboxRecepient->AddItem(Unicode::narrowToWide(*iterRecipient), *iterRecipient);

	m_comboboxRecepient->SetSelectedIndex(-1);
	m_comboboxRecepient->SetScrollbarOpacity(0.7f);

	// maximum 100 characters in the recipient combo box text field
	UITextbox * textboxRecepient = static_cast<UITextbox *> (m_comboboxRecepient->GetChild("ComboTextbox"));
	if (textboxRecepient)
	{
		textboxRecepient->SetProperty(UITextbox::PropertyName::MaxLength, Unicode::narrowToWide("100"));
		textboxRecepient->SetMaximumCharacters(100);
	}

	if (!ms_nextRecepients.empty ())
		m_comboboxRecepient->SetProperty(UIComboBox::PropertyName::SelectedText, Unicode::narrowToWide(ms_nextRecepients));

	if (!ms_nextSubject.empty ())
		m_textboxSubject->SetText (ms_nextSubject);

	if (!ms_nextBody.empty ())
		m_textBody->SetLocalText (ms_nextBody);

	if (!ms_nextOob.empty ())
		processOob (ms_nextOob);	
	
	m_textMaxReached->SetVisible(m_textBody->IsAtMaximumCharacters());

	// disable Send button if squelched
	m_buttonOk->SetEnabled(!Game::isPlayerSquelched());

	setState (MS_closeable);

	registerMediatorObject (*m_textBody,     this);
	registerMediatorObject (*m_volumeAttach, this);
	registerMediatorObject (*m_buttonOk,     this);
	registerMediatorObject (*m_buttonCancel, this);
	registerMediatorObject (getPage (),      this);
}

//----------------------------------------------------------------------

SwgCuiPersistentMessageComposer::~SwgCuiPersistentMessageComposer ()
{
	m_buttonOk          = 0;
	m_buttonCancel      = 0;
	m_comboboxRecepient = 0;
	m_textboxSubject    = 0;
	m_textBody          = 0;

	delete m_attachments;
	m_attachments = 0;

	clearVolumeChildren ();
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::performActivate   ()
{
	CuiManager::requestPointer (true);

	Unicode::String recipient;
	m_comboboxRecepient->GetProperty(UIComboBox::PropertyName::SelectedText, recipient);

	if (recipient.empty ())
	{
		UITextbox * textboxRecepient = static_cast<UITextbox *> (m_comboboxRecepient->GetChild("ComboTextbox"));
		if (textboxRecepient)
			textboxRecepient->SetFocus ();
	}
	else if (m_textboxSubject->GetLocalText ().empty ())
		m_textboxSubject->SetFocus ();
	else
		m_textBody->SetFocus ();

	m_textBody->SetCursorPoint (0);
	m_textBody->ClearSelection ();

	m_volumeAttach->SetVisible (true);

	if (!s_callback)
	{
		s_callback = new MyCallback;
      s_callback->connect (*s_callback, &MyCallback::onAttach, static_cast<CuiPersistentMessageManager::Messages::AttachObject *>(0));
	}

	s_composers.push_back (this);
	CuiPersistentMessageManager::setCompositionWindowCount (static_cast<int>(s_composers.size ()));

	setIsUpdating (true);
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::performDeactivate ()
{
	CuiManager::requestPointer (false);

	s_composers.erase (std::remove (s_composers.begin (), s_composers.end (), this), s_composers.end ());
	if (s_composers.empty ())
	{
		delete s_callback;
		s_callback = 0;
	}

	CuiPersistentMessageManager::setCompositionWindowCount (static_cast<int>(s_composers.size ()));

	setIsUpdating (false);
}

//----------------------------------------------------------------------

void  SwgCuiPersistentMessageComposer::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonOk)
	{
		send ();
	}
	else if (context == m_buttonCancel)
	{
		if (getButtonClose ())
			getButtonClose ()->Press ();
	}
}

//----------------------------------------------------------------------

bool SwgCuiPersistentMessageComposer::OnMessage        (UIWidget *context, const UIMessage & msg)
{
	if (context == &getPage ())
	{
		if (msg.Type == UIMessage::KeyDown)
		{
			 if (msg.Keystroke == UIMessage::Enter && msg.Modifiers.isControlDown ())
			 {
				send ();
				return false;
			 }
		}
	}
	else if (context == m_textBody)
	{
		if (msg.Type == UIMessage::Character)
		{
			m_textMaxReached->SetVisible(m_textBody->IsAtMaximumCharacters());
		}
	}
	//----------------------------------------------------------------------

	else if (context == m_volumeAttach)
	{
		if (msg.Type == UIMessage::DragOver)
		{
			NOT_NULL (msg.DragSource);
			NOT_NULL (msg.DragObject);

			m_volumeAttach->SetDropFlagOk (false);

			if (msg.DragSource->GetParent () == m_volumeAttach)
			{
				return false;
			}

			const CuiDragInfo cdi (*msg.DragObject);

			if (cdi.type == CuiDragInfoTypes::CDIT_object)
			{
				const ClientObject * const obj = cdi.getClientObject ();
				NOT_NULL (obj);

				//-- only allow dropping waypoints for now					
				if (!dynamic_cast<const ClientWaypointObject *>(obj))
					return false;

				m_volumeAttach->SetDropFlagOk(true);
			}
			else if (cdi.type == CuiDragInfoTypes::CDIT_attachmentData)
			{
				m_volumeAttach->SetDropFlagOk(true);
			}
			return false;
		}
		else if (msg.Type == UIMessage::DragEnd)
		{
			NOT_NULL (msg.DragSource);
			NOT_NULL (msg.DragObject);

			if (msg.DragSource->GetParent () == m_volumeAttach)
				return true;

			const CuiDragInfo cdi (*msg.DragObject);

			if (cdi.type == CuiDragInfoTypes::CDIT_object)
			{
				const ClientObject * const obj = cdi.getClientObject ();
				NOT_NULL (obj);
				addAttachment (*obj);
			}
			else if (cdi.type == CuiDragInfoTypes::CDIT_attachmentData)
			{
				const AttachmentData * const ad = cdi.getAttachmentData ();
				NOT_NULL (ad);
				addAttachment (*ad);
			}
	
			return false;
		}
	}

	//----------------------------------------------------------------------

	else if (context->GetParent () == m_volumeAttach)
	{
		if (msg.Type == UIMessage::ContextRequest)
		{
			int index = 0;
			context->GetPropertyInteger (UIBaseObject::PropertyName::Name, index);

			UIPopupMenu * const pop = new UIPopupMenu(&getPage());
			pop->AddCallback (this);
			pop->SetStyle    (context->FindPopupStyle ());
			pop->AddItem     (PopupItems::del, CuiStringIdsPersistentMessage::popup_delete.localize ());
			pop->SetVisible  (true);
			pop->SetLocation (context->GetWorldLocation () + msg.MouseCoords);
			pop->SetPropertyInteger (Properties::PopupChildIndex, index);
			UIManager::gUIManager ().PushContextWidget (*pop);
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::OnPopupMenuSelection (UIWidget * context)
{
	NOT_NULL (context);
	UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);

	int index = 0;
	if (context->GetPropertyInteger (Properties::PopupChildIndex, index))
	{
		const std::string & selection = pop->GetSelectedName ();

		if (selection == PopupItems::del)
		{
			removeAttachment (index);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::addAttachment    (const AttachmentData & ad)
{
	if (ad.isValid ())
	{
		//-- only add waypoints for now, expand later to include tokens
		if (ad.getType () != AttachmentData::T_waypoint)
			return;

		m_attachments->push_back (ad);
		addIconForAttachment (m_attachments->size () - 1);
	}
	else
		WARNING (true, ("SwgCuiPersistentMessageComposer error adding object attachment"));
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::addIconForAttachment (int index)
{
	const SwgCuiAttachmentList::Renderable * const renderable = m_attachments->get (index);
	NOT_NULL (renderable);
	
	if (renderable)
	{
		static int iconCount = 0;
		UIWidget * const icon = renderable->fetchIcon (true);
		NOT_NULL (icon);
		icon->SetPropertyInteger (UIBaseObject::PropertyName::Name, m_attachments->size () - 1);
		icon->SetDropToParent (true);
		m_volumeAttach->AddChild (icon);
	}
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::removeAttachment (int index)
{
	if (index < 0 || index >= m_attachments->size ())
		return;

	clearVolumeChildren  ();
	m_attachments->erase (index);
	clearVolumeChildren  ();
	for (int i = 0 ; i < m_attachments->size (); ++i)
		addIconForAttachment (i);
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::clearVolumeChildren ()
{
	SwgCuiAttachmentList::Renderable::releaseIconsForPage (*m_volumeAttach);
	m_volumeAttach->Clear ();
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::addAttachment    (const ClientObject & obj)
{	
	const AttachmentData ad (obj);
	addAttachment (ad);
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::setNextParameters (const std::string & recepients, const Unicode::String & subject, const Unicode::String & body, const Unicode::String & oob)
{
	ms_nextRecepients = recepients;
	ms_nextSubject    = subject;
	ms_nextBody       = body;	
	ms_nextOob        = oob;
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::send ()
{
	Unicode::String recepients;
	m_comboboxRecepient->GetProperty(UIComboBox::PropertyName::SelectedText, recepients);

	const Unicode::String & subject     = m_textboxSubject->GetLocalText ();
	const Unicode::String & body        = m_textBody->GetLocalText ();

	if (recepients.empty ())
		CuiMessageBox::createInfoBox (CuiStringIdsPersistentMessage::err_no_recipients.localize ());
//	else if (subject.empty ())
//		CuiMessageBox::createInfoBox (CuiStringIdsPersistentMessage::pm_err_no_subject.localize ());
	else
	{
		std::string narrow_recepients (Unicode::wideToNarrow (recepients));
		if (!CuiPersistentMessageManager::validateRecepientsString (narrow_recepients))
		{
			CuiMessageBox::createInfoBox (CuiStringIdsPersistentMessage::err_bad_recipients.localize ());
		}
		else
		{
			Unicode::String oob;
			m_attachments->constructOob (oob);
			CuiPersistentMessageManager::sendMessageToMultiple (narrow_recepients, subject, body, oob);
			m_buttonCancel->Press ();
		}
	}
}

//----------------------------------------------------------------------

SwgCuiPersistentMessageComposer * SwgCuiPersistentMessageComposer::findTopComposer ()
{
	return this;
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::processOob  (const Unicode::String & oob)
{
	const OutOfBandPackager::OutOfBandBaseVector & bv = OutOfBandPackager::unpack (oob, OutOfBandPackager::OT_waypointData);

	for (OutOfBandPackager::OutOfBandBaseVector::const_iterator it = bv.begin (); it != bv.end (); ++it)
	{
		OutOfBandBase * const oobBase = *it;
		const int position = oobBase->getPosition ();
		const int typeId = oobBase->getTypeId ();
		
		//-- negative 3 is the index for attachments
		if (position == -3)
		{
			if (typeId == OutOfBandPackager::OT_waypointData)
			{
				AttachmentData ad;
				ad.set (*oobBase);
				addAttachment (ad);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageComposer::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	// disable Send button if squelched
	if (m_buttonOk)
		m_buttonOk->SetEnabled(!Game::isPlayerSquelched());
}

//======================================================================
