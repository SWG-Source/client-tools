// ======================================================================
//
// CuiMessageBox.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiMessageBox.h"

#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UIComposite.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIProgressbar.h"
#include "UIRunner.h"
#include "UIScrollbar.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedRandom/Random.h"

#include <algorithm>

// ======================================================================

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiMessageBox::Messages::Closed::Payload &, CuiMessageBox::Messages::Closed > 
			closed;
	}
	
	const UILowerString BUTTON_TYPE_PROP = UILowerString ("BUTTON_TYPE");
	
	int messageBoxCount = 0;
	
	typedef std::vector<CuiMessageBox *>       MessageBoxVector_t;
	MessageBoxVector_t *                       ms_messageBoxVector = 0;
	bool                                       s_needsReleaseMouseLock = false;
	
	bool ms_installed = false;
}

//-----------------------------------------------------------------

CuiMessageBox::CuiMessageBox (bool modal, CallbackFunc callbackFunc) :
UIEventCallback       (),
m_killme              (false),
m_modal               (modal),
m_mainPage            (0),
m_innerPage           (0),
m_inputPage			  (0),
m_text                (0),
m_inputText			  (0),
m_scrollbar           (0),
m_sampleButton        (0),
m_progressbar         (0),
m_buttons             (new ButtonVector),
m_transitionTarget    (0),
m_emitter             (new MessageDispatch::Emitter ()),
m_completedButtonType (GBT_None),
m_completedButtonName (),
m_transceiverClosed   (0),
m_buttonClose         (0),
m_callbackFunc        (callbackFunc),
m_runner              (0),
m_composite           (0),
m_compositeButtons    (0),
m_compositeMargins    (new UIRect),
m_image				  (0),
m_verifyPage          (0)
{
	NOT_NULL (m_buttons);

	UIPage * const root          = NON_NULL (UIManager::gUIManager().GetRootPage());
	root->ReleaseMouseLock (UIPoint::zero);

	UIPage * const templatePage  = NON_NULL (GET_UI_OBJ ((*root), UIPage, "/MsgBox"));
	m_mainPage                   = NON_NULL (static_cast<UIPage *>(templatePage->Clone ())); //lint !e1774 //stfu noob
	m_mainPage->CopyPropertiesFrom (*templatePage);

	// this instance of the messagebox must have a unique name
	char buf[64];
	IGNORE_RETURN (_itoa (messageBoxCount++, buf, 10));
	m_mainPage->SetName (m_mainPage->GetName () + "_" + buf);

	const UIPage * const templateInnerPage = NON_NULL (GET_UI_OBJ ((*templatePage), UIPage, "InnerPage"));
	m_innerPage                            = NON_NULL (static_cast<UIPage *>(templateInnerPage->DuplicateObject ())); //lint !e1774 //stfu noob

	IGNORE_RETURN (m_mainPage->AddChild (m_innerPage));

	m_composite                            = NON_NULL (GET_UI_OBJ ((*m_innerPage),    UIComposite,    "comp"));
	m_compositeButtons                     = NON_NULL (GET_UI_OBJ ((*m_composite),    UIComposite,    "cButtons"));

	m_sampleButton                         = NON_NULL (GET_UI_OBJ ((*m_compositeButtons),  UIButton,    "b"));
	m_sampleButton->SetVisible (false);
	m_sampleButton->Attach (0);
	m_compositeButtons->Clear ();
	m_compositeButtons->SetVisible (false);

	m_text                                 = NON_NULL (GET_UI_OBJ ((*m_composite),    UIText,      "pText.text"));
	m_scrollbar                            = NON_NULL (GET_UI_OBJ ((*m_composite),    UIScrollbar, "pText.scroll"));

	m_buttonClose                          = NON_NULL (GET_UI_OBJ ((*m_innerPage),    UIButton,    "bg.mmc.close"));
	m_buttonClose->AddCallback (this);

	m_runner                               = NON_NULL (GET_UI_OBJ ((*m_composite),   UIRunner,    "runner"));
	m_runner->SetVisible (false);

	m_inputPage							   = NON_NULL (GET_UI_OBJ ((*m_composite),   UIPage,      "inputText"));
	m_inputText							   = NON_NULL (GET_UI_OBJ ((*m_composite),    UIText,     "inputText.text"));

	m_image								   = NON_NULL (GET_UI_OBJ ((*m_composite),  UIImage,  "verifyPage.image"));
	m_verifyPage						   = NON_NULL (GET_UI_OBJ ((*m_composite),   UIPage,      "verifyPage"));

	m_inputPage->SetVisible(false);
	m_inputText->SetVisible(false);
	m_inputText->AddCallback(this);

	m_verifyPage->SetVisible(false);

	m_composite->SetProperty        (UIWidget::PropertyName::PackLocation, Unicode::emptyString);
	m_composite->SetProperty        (UIWidget::PropertyName::PackSize, Unicode::emptyString);
	m_compositeButtons->SetProperty (UIWidget::PropertyName::PackLocation, Unicode::emptyString);
	m_compositeButtons->SetProperty (UIWidget::PropertyName::PackSize, Unicode::emptyString);
	m_text->GetParentWidget ()->SetProperty (UIWidget::PropertyName::PackLocation, Unicode::emptyString);
	m_text->GetParentWidget ()->SetProperty (UIWidget::PropertyName::PackSize, Unicode::emptyString);
	m_text->SetProperty             (UIWidget::PropertyName::PackLocation, Unicode::emptyString);
	m_text->SetProperty             (UIWidget::PropertyName::PackSize, Unicode::emptyString);

	IGNORE_RETURN (root->AddChild  (m_mainPage));
	IGNORE_RETURN (root->MoveChild (m_mainPage, UIBaseObject::Top));

	m_mainPage->Link ();

	m_mainPage->SetEnabled (true);
	m_mainPage->SetVisible (true);
	
	m_image->SetVisible (false);

	m_innerPage->SetFocus ();
	m_buttonClose->SetFocus ();

	CuiMediator::incrementPointerInputActiveCount  (1);
	CuiMediator::incrementKeyboardInputActiveCount (1);

	DEBUG_FATAL (m_innerPage != m_composite->GetParent (), ("data error"));
	*m_compositeMargins                = m_composite->GetRect ();
	const UISize & compositeParentSize = m_innerPage->GetSize ();
	m_compositeMargins->right  = compositeParentSize.x - m_compositeMargins->right;
	m_compositeMargins->bottom = compositeParentSize.y - m_compositeMargins->bottom;

	//-- force an input map reset since we are assuming full screen control with the new messaagebox
	CuiManager::getIoWin ().resetInputMaps ();
}

//-----------------------------------------------------------------

CuiMessageBox::~CuiMessageBox ()
{
	m_sampleButton->Detach (0);
	m_sampleButton = 0;

	CuiMediator::incrementPointerInputActiveCount  (-1);
	CuiMediator::incrementKeyboardInputActiveCount (-1);

	//-- close and emit closing message if needed, this usually will be a no-op
	closeMessageBox ();

	m_innerPage = 0;

	NOT_NULL (m_mainPage->GetParent ());

	IGNORE_RETURN (m_mainPage->GetParent ()->RemoveChild (m_mainPage));
	m_mainPage = 0;

	m_text = 0;
	m_scrollbar = 0;
	m_sampleButton = 0;

	delete m_buttons;
	m_buttons = 0;

	delete m_emitter;
	m_emitter = 0;

	delete m_transceiverClosed;
	m_transceiverClosed = 0;

	m_callbackFunc = 0;

	delete m_compositeMargins;
	m_compositeMargins = 0;
}

//----------------------------------------------------------------------

void CuiMessageBox::install ()
{
	DEBUG_FATAL (ms_installed, ("already installed\n"));

	ms_messageBoxVector = new MessageBoxVector_t ();

	ms_ButtonTypeLabels[CuiMessageBox::GBT_None]    = StringId ();
	ms_ButtonTypeLabels[CuiMessageBox::GBT_Ok]      = CuiStringIds::ok;
	ms_ButtonTypeLabels[CuiMessageBox::GBT_Cancel]  = CuiStringIds::cancel;
	ms_ButtonTypeLabels[CuiMessageBox::GBT_No]      = SharedStringIds::no;
	ms_ButtonTypeLabels[CuiMessageBox::GBT_Yes]     = SharedStringIds::yes;
	ms_ButtonTypeLabels[CuiMessageBox::GBT_Abort]   = CuiStringIds::abort;

	ms_installed = true;
}

//----------------------------------------------------------------------

void  CuiMessageBox::remove ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	// wipe out any outstanding messageboxes
	if (ms_messageBoxVector)
	{
		for (MessageBoxVector_t::iterator iter = ms_messageBoxVector->begin (); iter != ms_messageBoxVector->end (); ++iter)
		{
			//-- todo: simply remove it from the parent widget?
			CuiMessageBox::CuiManagerHelper::destroyMessageBox (*iter);
		}
		delete ms_messageBoxVector;
		ms_messageBoxVector = 0;
	}

	ms_installed = false;
}

//----------------------------------------------------------------------

void CuiMessageBox::update ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	//-- cleanup the vector of messageboxes
	
	NOT_NULL (ms_messageBoxVector);
	
	if (!ms_messageBoxVector->empty ())
	{
		CuiMessageBox * lastBox = 0;

		bool found = false;
		
		for (MessageBoxVector_t::iterator iter = ms_messageBoxVector->begin (); iter != ms_messageBoxVector->end (); ++iter)
		{
			CuiMessageBox * const box = *iter;

			if (box)
			{
				if (box->pleaseKillMe ())
				{
					found = true;
					//-- todo: simply remove it from the parent widget?
					CuiMessageBox::CuiManagerHelper::destroyMessageBox (box);
					*iter = 0;
				}
				else
					lastBox = box;
			}
			else
				found = true;
		}
		
		if ((found || s_needsReleaseMouseLock) && lastBox)
		{
			UIPage * const root = NON_NULL (UIManager::gUIManager ().GetRootPage ());
			root->ReleaseMouseLock (UIPoint::zero);
			lastBox->m_innerPage->SetFocus ();
		}

		//-- some messageboxes have been deleted, erase them from the vector
		if (found)
		{
			ms_messageBoxVector->erase (std::remove (ms_messageBoxVector->begin (), ms_messageBoxVector->end (), static_cast<CuiMessageBox *>(0)),
				ms_messageBoxVector->end ());
		}

		s_needsReleaseMouseLock = false;
	}
}

//-----------------------------------------------------------------

CuiMessageBox *  CuiMessageBox::createMessageBox (CallbackFunc callbackFunc, bool modal, bool runner, bool doLayout, bool inputField)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	CuiMessageBox * const box = new CuiMessageBox (modal, callbackFunc);

	UIManager::gUIManager ().AbortDragAndDrop (0);

	ms_messageBoxVector->push_back (box);
	s_needsReleaseMouseLock = true;

	//-- put the console back on top

	UIPage * const root = NON_NULL (UIManager::gUIManager().GetRootPage());

	UIPage * const console = GET_UI_OBJ((*root), UIPage, "/Console");
	NOT_NULL (console);
	IGNORE_RETURN (root->MoveChild (console, UIBaseObject::Top));

	box->setRunner (runner, false);

	box->enableInputField(inputField);

	if (doLayout)
		box->layout ();

	return box;
}

//----------------------------------------------------------------------

CuiMessageBox *  CuiMessageBox::createInfoBox    (const UIString & str, CallbackFunc callbackFunc, bool modal)
{
	return createMessageBox (str, GBT_Ok, callbackFunc, modal); 
}

//----------------------------------------------------------------------

CuiMessageBox *  CuiMessageBox::createWaitingBox    (const UIString & str, CallbackFunc callbackFunc, bool modal)
{
	return createMessageBox (str, GBT_Cancel, callbackFunc, modal, true); 
}

//----------------------------------------------------------------------

/*
* Create a messagebox with one button
*/

CuiMessageBox * CuiMessageBox::createMessageBox (const UIString & str, GenericButtonType type, CallbackFunc callbackFunc, bool modal, bool runner, bool inputField)
{
	CuiMessageBox * const box = NON_NULL (createMessageBox (callbackFunc, modal, runner, false, inputField));
	box->addButton (type, true, true, false);
	box->setText (str, false);
	box->layout ();

	if (type == GBT_Yes || type == GBT_Ok)
		CuiSoundManager::play (CuiSounds::dialog_warning);

	return box;
}

//----------------------------------------------------------------------

/*
* Create a messagebox with no buttons
*/

CuiMessageBox * CuiMessageBox::createMessageBox (const UIString & str, CallbackFunc callbackFunc, bool modal, bool runner, bool inputField)
{
	CuiMessageBox * const box = NON_NULL (createMessageBox (callbackFunc, modal, runner, false, inputField));
	box->setText (str, false);
	box->layout ();
	return box;
}

//----------------------------------------------------------------------

CuiMessageBox * CuiMessageBox::createDialogBox (const UIString & str, GenericButtonType buttonDefault, GenericButtonType buttonCancel, CallbackFunc callbackFunc, bool modal)
{
	CuiMessageBox * const box = NON_NULL (createMessageBox (callbackFunc, modal, false));

	if (buttonCancel != GBT_None)
	{
		box->addButton (buttonCancel, false, true, false);
	}

	box->addButton (buttonDefault, true, buttonCancel == GBT_None, false);

	box->setText (str, false);
	box->layout ();

	CuiSoundManager::play (CuiSounds::dialog_warning);

	return box;
}

//----------------------------------------------------------------------

CuiMessageBox *      CuiMessageBox::createOkCancelBox  (const UIString & str, CallbackFunc callbackFunc, bool modal)
{
	return createDialogBox (str, GBT_Ok, GBT_Cancel, callbackFunc, modal);
}

//----------------------------------------------------------------------

CuiMessageBox *      CuiMessageBox::createYesNoBox     (const UIString & str, CallbackFunc callbackFunc, bool modal)
{
	return createDialogBox (str, GBT_Yes, GBT_No, callbackFunc, modal);
}

//----------------------------------------------------------------------

const char * const CuiMessageBox::Messages::COMPLETED         = "CuiMessageBox::Messages::COMPLETED";
const char * const CuiMessageBox::Messages::CLOSED            = "CuiMessageBox::Messages::cLOSED";
const char * const CuiMessageBox::Messages::PROGRESS_FINISHED = "CuiMessageBox::Messages::PROGRESS_FINISHED";

StringId CuiMessageBox::ms_ButtonTypeLabels[CuiMessageBox::GBT_Max];

const char * const CuiMessageBox::ms_ButtonTypeNames[CuiMessageBox::GBT_Max] =
{
	"None",
	"OkButton",
	"CancelButton",
	"NoButton",
	"YesButton",
	"AbortButton"
};

//----------------------------------------------------------------------

CuiMessageBox::ButtonInfo::ButtonInfo () :
m_labelText (),
m_name (),
m_isDefault (false),
m_isCancel (false),
m_type (GBT_None)
{
	DEBUG_FATAL ((m_type < GBT_None || m_type >= GBT_Max), ("Invalid type.\n"));
	m_labelText = ms_ButtonTypeLabels [static_cast<size_t>(m_type)].localize ();
	m_name      = ms_ButtonTypeNames  [static_cast<size_t>(m_type)];
}

//----------------------------------------------------------------------

CuiMessageBox::ButtonInfo::ButtonInfo (CuiMessageBox::GenericButtonType type, bool isDefault, bool isCancel) :
m_labelText (),
m_name (),
m_isDefault (isDefault),
m_isCancel (isCancel),
m_type (type)
{
	DEBUG_FATAL ((type < GBT_None || type >= GBT_Max), ("Invalid type.\n"));
	m_labelText = ms_ButtonTypeLabels [static_cast<size_t>(type)].localize ();
	m_name      = ms_ButtonTypeNames  [static_cast<size_t>(type)];
}

//----------------------------------------------------------------------

CuiMessageBox::BoxMessage::BoxMessage (CuiMessageBox * box, const char * const theType) :
MessageDispatch::MessageBase (theType),
m_box (box)
{
}

//----------------------------------------------------------------------

CuiMessageBox::BoxMessage::~BoxMessage ()
{
	m_box = 0;
}

//----------------------------------------------------------------------

const CuiMessageBox * CuiMessageBox::BoxMessage::getMessageBox () const
{
	return m_box;
}

//----------------------------------------------------------------------

CuiMessageBox::CompletedMessage::CompletedMessage (CuiMessageBox * box, GenericButtonType buttonType) :
BoxMessage (box, Messages::COMPLETED),
m_buttonType (buttonType)
{
}

//----------------------------------------------------------------------

CuiMessageBox::GenericButtonType CuiMessageBox::CompletedMessage::getButtonType () const
{
	return m_buttonType;
}

//-----------------------------------------------------------------

void  CuiMessageBox::setText (const UIString & str, bool doLayout)
{
	m_text->SetText (str);

	m_text->SetEnabled (true);
	m_text->SetVisible (true);

	if (doLayout)
		layout ();
}

//-----------------------------------------------------------------

void  CuiMessageBox::setInputText (const UIString & str, bool doLayout)
{
	m_inputText->SetText (str);

	m_inputPage->SetEnabled(true);
	m_inputPage->SetVisible(true);

	m_inputText->SetEnabled (true);
	m_inputText->SetVisible (true);

	if (doLayout)
		layout ();
}


//-----------------------------------------------------------------

void CuiMessageBox::addButton (const ButtonInfo & info, bool doLayout)
{	
	UIButton * button = static_cast<UIButton*> (m_sampleButton->DuplicateObject ()); //lint !e1774 //stfu noob

	UIString typeStr;
	if (UIUtils::FormatInteger (typeStr, static_cast<int>(info.m_type)))
		IGNORE_RETURN (button->SetProperty (BUTTON_TYPE_PROP, typeStr));

	NOT_NULL (button);
	
	m_buttons->push_back (button);
	
	m_compositeButtons->AddChild   (button);
	m_compositeButtons->MoveChild  (button, UIBaseObject::Top);
	m_compositeButtons->SetVisible (true);

	button->Link ();

	button->SetVisible (true);
	button->SetEnabled (true);
	
	button->SetIsDefaultButton (info.m_isDefault);
	button->SetIsCancelButton (info.m_isCancel);
	button->SetText (info.m_labelText);

	if (info.m_isDefault)
		button->SetFocus ();

//	button->SetFocus ();

	if (info.m_isCancel)
		m_completedButtonType = info.m_type;

	button->SetName (info.m_name);
	
	button->AddCallback (this);

	m_buttonClose->SetVisible (false);

	if (doLayout)
		layout ();
}

//-----------------------------------------------------------------

void CuiMessageBox::addButton  (GenericButtonType type, bool isDefault, bool isCancel, bool doLayout)
{
	if (type == GBT_Max || type == GBT_None)
		return;

	addButton (ButtonInfo (type, isDefault, isCancel), doLayout);
}

//-----------------------------------------------------------------

void  CuiMessageBox::setButtons (const ButtonInfo * buttons, const int numButtons, bool doLayout)
{
	{
		for (size_t i = 0; i < m_buttons->size (); ++i)
		{
			if ((*m_buttons) [i]->GetParent () == m_innerPage)
			{
				(*m_buttons) [i]->RemoveCallback (this);
				IGNORE_RETURN (m_compositeButtons->RemoveChild ((*m_buttons) [i]));
				(*m_buttons) [i] = 0;
			}
		}
	}

	m_buttons->clear ();

	for (int i = 0; i < numButtons; ++i)
	{
		addButton (buttons [i]);
	}

	if (doLayout)
		layout ();
}

//-----------------------------------------------------------------

void CuiMessageBox::layout ()
{
	long widthRequired = 0;

	if (m_inputPage->IsVisible())
	{
		widthRequired = m_inputPage->GetWidth();
	}

	if(m_verifyPage->IsVisible())
	{
		widthRequired = std::max(m_verifyPage->GetWidth(), widthRequired);
	}

	if (m_runner->IsVisible ())
	{
		widthRequired = m_runner->GetWidth ();
	}
	
	{
		for (size_t i = 0; i < m_buttons->size (); ++i)
		{
			UIButton * const button = (*m_buttons) [i];
			UISize dsize;
			button->GetDesiredSize (dsize);
			button->SetWidth (dsize.x);
		}
		
		m_compositeButtons->SetWidth (0);
		m_compositeButtons->Pack ();
		UISize extent;
		m_compositeButtons->GetScrollExtent (extent);
		m_compositeButtons->SetWidth        (extent.x);
		m_compositeButtons->SetLocation     (UIPoint::zero);
	}
	
	widthRequired = std::max (widthRequired, m_compositeButtons->GetWidth ());	
	
	/*
	const UISize BUTTON_MARGIN (32, 32);
	
	long buttonWidthRequired = static_cast<long>(m_buttons->size ()) * (m_sampleButton->GetWidth () + BUTTON_MARGIN.y);

	NOT_NULL (m_mainPage->GetParent ());

	const UISize & parentSize = static_cast<UIPage*>(m_mainPage->GetParent ())->GetSize (); //lint !e1774 //stfu noob

	const UISize MAX_BOX_SIZE (parentSize.x  - BUTTON_MARGIN.x * 4,
	                           parentSize.y  - BUTTON_MARGIN.y * 4);
	
	  */

	const Unicode::String & localText = m_text->GetLocalText ();

	if (!localText.empty ())
	{
		const long scrollbarWidth = m_scrollbar->GetWidth ();

		m_text->SizeToContent ();

		const UISize & MAX_TEXT_SIZE = m_mainPage->GetSize () / 3L;

		if (m_text->GetWidth () > MAX_TEXT_SIZE.x)
			m_text->SetWidth (MAX_TEXT_SIZE.x);
		else if ((m_text->GetWidth () + scrollbarWidth) < widthRequired)
			m_text->SetWidth (widthRequired - scrollbarWidth);

		UISize extent;
		m_text->GetScrollExtent (extent);

		const long height = std::min (extent.y, MAX_TEXT_SIZE.y);
		m_text->SetHeight (height);
		const UISize textSize = m_text->GetSize ();

		UIPage * const textParent = safe_cast<UIPage *>(m_text->GetParent ());
		NOT_NULL (textParent);
		textParent->SetSize (UISize (textSize.x + scrollbarWidth, textSize.y));
		m_composite->SetWidth (textParent->GetWidth ());
		textParent->SetLocation     (UIPoint::zero);

		widthRequired = std::max (m_composite->GetWidth (), widthRequired);
	}

	m_compositeButtons->SetWidth (widthRequired);
	m_composite->SetWidth        (widthRequired);
	m_composite->SetHeight       (0L);
	m_composite->Pack ();

	UISize extent;
	m_composite->GetScrollExtent (extent);
	m_composite->SetHeight (extent.y);

	DEBUG_FATAL (extent.x != widthRequired, ("logic error"));

	m_innerPage->SetWidth  (m_composite->GetWidth  () + m_compositeMargins->left + m_compositeMargins->right);
	m_innerPage->SetHeight (m_composite->GetHeight () + m_compositeMargins->top  + m_compositeMargins->bottom);

	m_composite->Center ();

	//-- trigger any ui scripts on the composite
	m_composite->SetEnabled (false);
	m_composite->SetEnabled (true);

	m_innerPage->Center ();
}

//-----------------------------------------------------------------

void  CuiMessageBox::setProgressbar (bool b)
{

	NOT_NULL (m_progressbar);
	m_progressbar->SetEnabled (b);
	m_progressbar->SetVisible (b);

	layout ();
}

//-----------------------------------------------------------------

void  CuiMessageBox::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonClose)
	{
		m_completedButtonType = GBT_Close;
		m_completedButtonName = context->GetName();
	}
	else
	{
		NOT_NULL (context);
		NOT_NULL (m_emitter);
		
		long type = static_cast<long>(GBT_None);
		
		IGNORE_RETURN (context->GetPropertyLong (BUTTON_TYPE_PROP, type));
		
		m_completedButtonType = static_cast<GenericButtonType>(type);
		m_completedButtonName = context->GetName();
	}

	m_emitter->emitMessage (CompletedMessage (this, m_completedButtonType));
	closeMessageBox ();

} //lint !e818 //stfu noob

//-----------------------------------------------------------------

void   CuiMessageBox::closeMessageBox ()
{
	if (m_mainPage->IsVisible ())
	{
		NOT_NULL (m_emitter);

		m_mainPage->SetVisible (false);
		m_mainPage->SetEnabled (false);
		m_killme = true;
		
		m_emitter->emitMessage (BoxMessage (this, Messages::CLOSED));

		if (m_transceiverClosed)
			m_transceiverClosed->emitMessage (*this);

		if (m_callbackFunc)
			m_callbackFunc (*this);

		if (m_transitionTarget)
			IGNORE_RETURN (CuiMediatorFactory::activate (m_transitionTarget));
	}
}

//-----------------------------------------------------------------

void CuiMessageBox::setModal (bool b)
{
	NOT_NULL (m_mainPage->GetParent ());
	m_modal = b;
	layout ();
}

//-----------------------------------------------------------------

const std::string & CuiMessageBox::getName () const
{
	return m_mainPage->GetName ();
}

//-----------------------------------------------------------------

void CuiMessageBox::setTransitionTarget (const char * target)
{
	m_transitionTarget = target;
}

//----------------------------------------------------------------------

void CuiMessageBox::connectToMessages (MessageDispatch::Receiver & receiver) const
{
	NOT_NULL (m_emitter);
	receiver.connectToEmitter (*m_emitter, Messages::COMPLETED);
	receiver.connectToEmitter (*m_emitter, Messages::CLOSED);
	receiver.connectToEmitter (*m_emitter, Messages::PROGRESS_FINISHED);
}

//----------------------------------------------------------------------

void CuiMessageBox::disconnectFromMessages (MessageDispatch::Receiver & receiver) const
{
	NOT_NULL (m_emitter);
	receiver.disconnectFromEmitter (*m_emitter, Messages::COMPLETED);
	receiver.disconnectFromEmitter (*m_emitter, Messages::CLOSED);
	receiver.disconnectFromEmitter (*m_emitter, Messages::PROGRESS_FINISHED);
}

//----------------------------------------------------------------------

MessageDispatch::Emitter &  CuiMessageBox::getEmitter ()
{
	return *NON_NULL (m_emitter);
}

//----------------------------------------------------------------------

CuiMessageBox::Transceivers::Closed & CuiMessageBox::getTransceiverClosed ()
{
	if (!m_transceiverClosed)
		m_transceiverClosed = new Transceivers::Closed;

	return *m_transceiverClosed;
}

//----------------------------------------------------------------------

int CuiMessageBox::getModalMessageBoxCount ()
{
	if (ms_messageBoxVector)
		return static_cast<int>(ms_messageBoxVector->size ());
	return 0;
}

//----------------------------------------------------------------------

void CuiMessageBox::setCallbackFunc (CallbackFunc func)
{
	m_callbackFunc = func;
}

//----------------------------------------------------------------------

void CuiMessageBox::setRunner       (bool b, bool doLayout)
{
	if (b)
	{
		m_runner->SetVisible  (true);
		m_runner->ToggleAll   (false);
		m_runner->SetProgress (0);
		m_runner->SetPaused   (false);
	}
	else
	{
		m_runner->SetVisible (false);
		m_runner->SetPaused  (true);
	}

	if (doLayout)
		layout ();

}

//----------------------------------------------------------------------

void CuiMessageBox::enableInputField(bool b)
{
	m_inputPage->SetVisible(b);
	m_inputText->SetVisible(b);
	m_inputText->SetLocalText(UIString());
}

//----------------------------------------------------------------------

void CuiMessageBox::ensureFocus     ()
{
	typedef std::vector<CuiMessageBox *>       MessageBoxVector_t;
	if (!ms_messageBoxVector->empty ())
	{
		CuiMessageBox * const box = ms_messageBoxVector->back ();
		if (box)
		{
			box->m_innerPage->SetFocus ();
			box->m_buttonClose->SetFocus ();

			for (ButtonVector::const_iterator it = box->m_buttons->begin (); it != box->m_buttons->end (); ++it)
			{
				UIButton * const butt = *it;
				if (butt->IsDefaultButton ())
				{
					butt->SetFocus ();
				}
			}
		}
	}
}


CuiMessageBox * CuiMessageBox::createOkCancelBoxWithInput(const UIString &str, CuiMessageBox::CallbackFunc callbackFunc, bool modal)
{
	CuiMessageBox * const box = NON_NULL (createMessageBox (callbackFunc, modal, false, true, true));


	box->addButton (GBT_Cancel, false, true, false);


	box->addButton (GBT_Ok, true, false, false);

	box->setText (str, false);
	box->layout ();

	CuiSoundManager::play (CuiSounds::dialog_warning);

	return box;
}

bool CuiMessageBox::OnMessage(UIWidget * context, const UIMessage & msg)
{
	UNREF(context);


	if(msg.Type == UIMessage::KeyDown && msg.Keystroke == UIMessage::Enter && m_inputText->IsVisible())
	{
		// This will only return the GBT_Ok since we don't know what the default button is currently...
		m_completedButtonType =  GBT_Ok;
		m_emitter->emitMessage (CompletedMessage (this, m_completedButtonType));
		closeMessageBox ();
		return true;
	}

	return true;
}

void CuiMessageBox::randomizePosition()
{
	UIPoint newLocation;

	newLocation.x = Random::random(200, 700);
	newLocation.y = Random::random(200, 500);

	m_innerPage->SetLocation(newLocation);
}

void CuiMessageBox::enableImage(bool enable)
{
	m_image->SetEnabled(enable);
	m_image->SetVisible(enable);

	layout();
}

void CuiMessageBox::generateVerificationImage(int totalCopies, bool rotate, bool scale)
{
	m_verifyPage->SetVisible(true);
	UISize maxSize = m_verifyPage->GetSize();
	UIPoint startPosition = m_image->GetLocation();

	long farX = maxSize.x - (maxSize.x / 10); // Farthest we'll ever go is 10% from the right edge.

	long equalIncrement = farX / totalCopies;
	
	for(int i = 0; i < totalCopies; ++i)
	{
		UIImage * newImage = dynamic_cast<UIImage *>(m_image->DuplicateObject());

		newImage->SetEnabled(true);
		newImage->SetVisible(true);
		newImage->SetName("verify");
		
		
		UIPoint newPosition = startPosition;

		newPosition.x += Random::random(-(equalIncrement/4), equalIncrement) * i;
		newPosition.y += Random::random(-35, 35);

		while(newPosition.x < 0)
		{
			newPosition.x = startPosition.x;
			newPosition.x += Random::random(-(equalIncrement/4), equalIncrement) * i;
		}

		newImage->SetLocation(newPosition);

		if(rotate)
			newImage->SetRotation(Random::randomReal(0.0f, 1.0f));

		if(scale)
		{
			UISize currentSize = newImage->GetSize();

			float xScalar = Random::randomReal(0.75f, 1.25f);
			float yScalar = Random::randomReal(0.85f, 1.15f);

			float newWidth = static_cast<float>(currentSize.x) * xScalar;
			float newHeight = static_cast<float>(currentSize.y) * yScalar;

			newImage->SetSize(UISize(static_cast<UIScalar>(newWidth), static_cast<UIScalar>(newHeight)));
		}

		float opacity = Random::randomReal(0.45f, 1.0f);

		newImage->SetOpacity(opacity);

		m_verifyPage->AddChild(newImage);
	}

	layout();
}

// ======================================================================
