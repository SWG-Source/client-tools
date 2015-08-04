// ======================================================================
//
// SwgCuiHarassmentMessage.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHarassmentMessage.h"
#include "swgClientUserInterface/SwgCuiTicketSubmission.h"
#include "swgClientUserInterface/SwgCuiTicketSubmission_NewTicket.h"

#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "UIButton.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"




// ======================================================================
//
// SwgCuiHarassmentMessage
//
// ======================================================================

//-----------------------------------------------------------------
SwgCuiHarassmentMessage::SwgCuiHarassmentMessage(UIPage &page)
 : CuiMediator("SwgCuiHarassmentReport", page)
 , UIEventCallback()
 , m_cancelButton(NULL)
 , m_continueButton(NULL)
 , m_fromKBSearch(false)
{


	getCodeDataObject(TUIButton, m_cancelButton, "buttonCancel");
	registerMediatorObject(*m_cancelButton, true);

	getCodeDataObject(TUIButton, m_continueButton, "buttonContinue");
	registerMediatorObject(*m_continueButton, true);

	IGNORE_RETURN(setState(MS_closeable));
	IGNORE_RETURN(setState(MS_closeDeactivates));
}

//-----------------------------------------------------------------
SwgCuiHarassmentMessage::~SwgCuiHarassmentMessage()
{
	
	m_cancelButton = NULL;
	m_continueButton = NULL;
	
}

//-----------------------------------------------------------------
void SwgCuiHarassmentMessage::performActivate()
{
	
}

//-----------------------------------------------------------------
void SwgCuiHarassmentMessage::performDeactivate()
{
}

//-----------------------------------------------------------------
void SwgCuiHarassmentMessage::OnButtonPressed(UIWidget *context)
{
	if( context == m_continueButton )
	{
		if(m_fromKBSearch)
		{
			SwgCuiTicketSubmission::NewTicket::setTicketSubmissionEnabled(true);
			IGNORE_RETURN(CuiActionManager::performAction (CuiActions::ticketSubmission, Unicode::emptyString));
		}
		else
		{
			IGNORE_RETURN(CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_HarassmentReport));
		}
		close();
	}
	else if (context == m_cancelButton)
	{
		close();
	}
}

//----------------------------------------------------------------
void SwgCuiHarassmentMessage::setFromKBSearch(bool val)
{
	m_fromKBSearch = val;
}

//----------------------------------------------------------------

SwgCuiHarassmentMessage *SwgCuiHarassmentMessage::createInto(UIPage *parent)
{
	NOT_NULL (parent);
	UIPage * const page = NON_NULL(safe_cast<UIPage *>(parent->GetObjectFromPath("/PDA.Service.harassmentMessage", TUIPage)));
	UIPage * const dupe = NON_NULL(safe_cast<UIPage *>(page->DuplicateObject()));
	IGNORE_RETURN(parent->AddChild(dupe));
	IGNORE_RETURN(parent->MoveChild(dupe, UIBaseObject::Top));
	dupe->Link();
	dupe->Center();
	
	SwgCuiHarassmentMessage * const creation = new SwgCuiHarassmentMessage(*dupe);
	return creation;
}



// ======================================================================
