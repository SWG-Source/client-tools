//======================================================================
//
// CuiInputBox.cpp
// copyright(c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiInputBox.h"

#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ConsentResponseMessage.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"

// =====================================================================

CuiInputBox::CuiInputBox (UIPage & page)
: CuiMediator        ("CuiInputBox", page),
  UIEventCallback    (),
  m_questionText     (0),
  m_answerText       (0),
  m_okayButton       (0),
  m_cancelButton     (0),
  m_responseSent     (false),
  m_callback         (new MessageDispatch::Callback),
  m_callbackFunction (0),
  m_object           ()
{
	IGNORE_RETURN(setState(MS_closeable));

	getCodeDataObject(TUIText,   m_questionText,  "question");
	getCodeDataObject(TUITextbox,m_answerText,    "text");
	getCodeDataObject(TUIButton, m_okayButton,    "buttonYes");
	getCodeDataObject(TUIButton, m_cancelButton,  "buttonNo");

	m_questionText->Clear();
	m_answerText->SetText(Unicode::emptyString);

	registerMediatorObject (*m_okayButton,   true);
	registerMediatorObject (*m_cancelButton, true);
}

//-----------------------------------------------------------------

CuiInputBox::~CuiInputBox()
{
	delete m_callback;
	m_callback = 0;
}

//-----------------------------------------------------------------

void CuiInputBox::performActivate()
{
}

//-----------------------------------------------------------------

void CuiInputBox::performDeactivate()
{
}

//-----------------------------------------------------------------

void CuiInputBox::OnButtonPressed(UIWidget *context)
{
	if(context == m_okayButton)
	{
		handleResult (true, m_answerText->GetLocalText());
		deactivate();
	}
	else if(context == m_cancelButton)
	{
		handleResult (false, m_answerText->GetLocalText());
		deactivate();
	}
}

//-----------------------------------------------------------------

bool CuiInputBox::close()
{
	if(!m_responseSent)
		handleResult (false, m_answerText->GetLocalText());

	deactivate();
	return true;
}

//-----------------------------------------------------------------

void CuiInputBox::setQuestionText   (const Unicode::String& question)
{
	m_questionText->SetText(question);
}

//-----------------------------------------------------------------

void CuiInputBox::setAnswerText     (const Unicode::String& defaultAnswer)
{
	m_answerText->SetText(defaultAnswer);
}

//-----------------------------------------------------------------

void CuiInputBox::setCallbackFunc (CuiInputBox::CallbackFunction f)
{
	m_callbackFunction = f;
}

//-----------------------------------------------------------------

void CuiInputBox::setObject (const NetworkId& object)
{
	m_object = object;
}

//----------------------------------------------------------------------

void CuiInputBox::handleResult (bool affirmative, Unicode::String const & s)
{
	if(m_callbackFunction)
		m_callbackFunction(affirmative, s, m_object);
	m_responseSent = true;
}

//=================================================================
