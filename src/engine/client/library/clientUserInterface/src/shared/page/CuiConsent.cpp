//======================================================================
//
// CuiConsent.cpp
// copyright(c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiConsent.h"

#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ConsentResponseMessage.h"

#include "clientUserInterface/CuiConsentManager.h"

#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "sharedObject/Object.h"

// =====================================================================

CuiConsent::CuiConsent (UIPage & page)
: CuiMediator      ("CuiConsent", page),
  UIEventCallback  (),
  m_questionText   (0),
  m_yesButton      (0),
  m_noButton       (0),
  m_callback       (new MessageDispatch::Callback),
  m_question       (),
  m_id             (0),
  m_responseSent   (false),
  m_clientOnly     (false)
{
	IGNORE_RETURN(setState(MS_closeable));

	getCodeDataObject(TUIText,   m_questionText, "text");
	getCodeDataObject(TUIButton, m_yesButton,    "buttonYes");
	getCodeDataObject(TUIButton, m_noButton,     "buttonNo");

	m_questionText->Clear();
}

//-----------------------------------------------------------------

CuiConsent::~CuiConsent()
{
	delete m_callback;
	m_callback = 0;
}

//-----------------------------------------------------------------

void CuiConsent::performActivate()
{
	m_yesButton->AddCallback(this);
	m_noButton->AddCallback (this);
}

//-----------------------------------------------------------------

void CuiConsent::performDeactivate()
{
	m_yesButton->RemoveCallback(this);
	m_noButton->RemoveCallback (this);
}

//-----------------------------------------------------------------

bool CuiConsent::close()
{
	if(!m_responseSent)
	{
		if(!m_clientOnly)
		{
			const ConsentResponseMessage msg(Game::getPlayer()->getNetworkId(), m_id, false);
			GameNetwork::send(msg, true);
		}
		else
		{
			CuiConsentManager::handleResponse(m_id, false);
		}
	}
	deactivate();
	return true;
}

//-----------------------------------------------------------------

void CuiConsent::OnButtonPressed(UIWidget *context)
{
	if(context == getPage().FindDefaultButton(false))
	{
	}
	if(context == getPage().FindCancelButton(false))
	{
	}
	if(context == m_yesButton)
	{
		if(!m_clientOnly)
		{
			const ConsentResponseMessage msg(Game::getPlayer()->getNetworkId(), m_id, true);
			GameNetwork::send(msg, true);
		}
		else
		{
			CuiConsentManager::handleResponse(m_id, true);
		}
		deactivate();
		m_responseSent = true;
	}
	else if(context == m_noButton)
	{
		if(!m_clientOnly)
		{
			const ConsentResponseMessage msg(Game::getPlayer()->getNetworkId(), m_id, false);
			GameNetwork::send(msg, true);
		}
		else
		{
			CuiConsentManager::handleResponse(m_id, false);
		}
		deactivate();
		m_responseSent = true;
	}
}

//-----------------------------------------------------------------

CuiConsent * CuiConsent::createInto(UIPage& parent)
{
	UIPage* const page = NON_NULL(safe_cast<UIPage*>(parent.GetObjectFromPath("/PDA.Consent", TUIPage)));
	UIPage* const dupe = NON_NULL(safe_cast<UIPage*>(page->DuplicateObject()));
	IGNORE_RETURN(parent.AddChild (dupe));
	IGNORE_RETURN(parent.MoveChild(dupe, UIBaseObject::Top));
	dupe->Link  ();

	CuiConsent* const creation = new CuiConsent(*dupe);
	return creation;
}

//-----------------------------------------------------------------

void CuiConsent::setQuestion(const Unicode::String& question)
{
	m_question = question;
	m_questionText->SetText(question);
}

//-----------------------------------------------------------------

int CuiConsent::getId()
{
	return m_id;
}

//-----------------------------------------------------------------

void CuiConsent::setId(int id)
{
	m_id = id;
}

//-----------------------------------------------------------------

void CuiConsent::setClientOnly(bool clientOnly)
{
	m_clientOnly = clientOnly;
}

//=================================================================
