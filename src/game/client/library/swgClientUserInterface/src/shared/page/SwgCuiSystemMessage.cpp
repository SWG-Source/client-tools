//======================================================================
//
// SwgCuiSystemMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSystemMessage.h"

#include "UIComposite.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiConsoleHelper.h"
#include "clientUserInterface/CuiInstantMessageManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiSystemMessageManagerData.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/PortalProperty.h"
#include <list>

//#include "DejaLib.h"

//======================================================================

namespace
{
	const float timeout_length      = 4.0f;
	const float timeout_length_noob = 6.0f;
	const char * const s_newbieHallTemplate = "object/building/general/shared_newbie_hall.iff";
}

//----------------------------------------------------------------------

SwgCuiSystemMessage::SwgCuiSystemMessage         (UIPage & page, Type type) :
CuiMediator    ("SwgCuiSystemMessage", page),
m_composite    (0),
m_sampleText   (0),
m_timeout      (0.0f),
m_callback     (new MessageDispatch::Callback),
m_moveToTop    (false),
m_type         (type),
m_updatePack   (false)
{
	getCodeDataObject (TUIComposite, m_composite,  "composite");
	getCodeDataObject (TUIText,      m_sampleText, "sampleText");

	m_sampleText->SetPreLocalized (false);
	m_sampleText->SetVisible      (false);
	m_composite->Clear            ();

	m_callback->connect (*this, &SwgCuiSystemMessage::onInstantMessageReceived,        static_cast<CuiInstantMessageManager::Messages::MessageReceived *>     (0));
	m_callback->connect (*this, &SwgCuiSystemMessage::onSystemMessageReceived,         static_cast<CuiSystemMessageManager::Messages::Received*>     (0));
}

//----------------------------------------------------------------------

SwgCuiSystemMessage::~SwgCuiSystemMessage ()
{
	m_callback->connect    (*this, &SwgCuiSystemMessage::onInstantMessageReceived,        static_cast<CuiInstantMessageManager::Messages::MessageReceived *>     (0));
	m_callback->disconnect (*this, &SwgCuiSystemMessage::onSystemMessageReceived,         static_cast<CuiSystemMessageManager::Messages::Received*>     (0));

	m_sampleText = 0;
	m_composite  = 0; 

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiSystemMessage::performActivate      ()
{
	m_moveToTop = true;

	if (!m_composite->GetChildrenRef ().empty ())
		setIsUpdating (true);
}

//----------------------------------------------------------------------

void SwgCuiSystemMessage::performDeactivate    ()
{
	getPage ().SetEnabled (false);
	setIsUpdating (false);
}

//----------------------------------------------------------------------

void SwgCuiSystemMessage::update (float deltaTimeSecs)
{
	if(m_updatePack)
	{
		m_composite->Pack ();
		m_composite->ScrollToBottom ();

		bool removed = false;
		const UISize scrollLoc = m_composite->GetScrollLocation ();

		UIBaseObject::UIObjectList olist;
		m_composite->GetChildren (olist);
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIBaseObject * const obj = *it;
			if (obj->IsA (TUIWidget))
			{
				UIWidget * const wid = safe_cast<UIWidget *>(obj);

				//--if the widget is partially obscured, dump it from the display
				if (wid->GetLocation ().y < scrollLoc.y)
				{
					m_composite->RemoveChild (wid);
					removed = true;
				}
			}
		}

		if (removed)
		{
			m_composite->Pack ();
			m_composite->ScrollToBottom ();
		}

		m_moveToTop = true;

		m_updatePack = false;
	}

	CuiMediator::update (deltaTimeSecs);

	checkStatus ();
}

//----------------------------------------------------------------------

void SwgCuiSystemMessage::checkStatus ()
{
	const UIBaseObject::UIObjectList & olist = m_composite->GetChildrenRef ();

	if (olist.empty ())
	{
		setIsUpdating (false);
		return;
	}

	if (m_moveToTop)
	{
		UIPage * const parent = dynamic_cast<UIPage *>(getPage ().GetParent ());
		if (parent)
		{
			UIWidget * const focusedLeaf = parent->GetFocusedLeafWidget ();
			if (focusedLeaf)
				focusedLeaf->Attach (0);

			parent->MoveChild (&getPage (), UIBaseObject::Top);
			if (focusedLeaf)
			{
//				focusedLeaf->SetFocus ();
				focusedLeaf->Detach (0);
			}
		}
		m_moveToTop = false;
	}

	const float curTime = Game::getElapsedTime ();

	if (curTime > m_timeout)
	{
		UIWidget * const widget = NON_NULL (safe_cast<UIWidget *>(olist.front ()));
		if (!widget->IsEnabled ())
		{
			if (widget->GetOpacity () == 0.0f)
			{
				m_composite->RemoveChild (olist.front ());

				if (olist.empty ())
					setIsUpdating (false);
				else
				{
					//-- newbs get longer messages
					if (m_type == T_noob)
						m_timeout = curTime + (timeout_length_noob * CuiPreferences::getSystemMessageDuration ());
					else
						m_timeout = curTime + (timeout_length *      CuiPreferences::getSystemMessageDuration ());
				}
				
				m_composite->ScrollToBottom ();
				m_composite->Pack ();
			}
		}
		else
		{
			widget->SetEnabled (false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSystemMessage::appendText           (const Unicode::String & str)
{
	//DEJA_CONTEXT("SwgCuiSystemMessage::appendText");

	bool wasEmpty = m_composite->GetChildrenRef ().empty ();

	UIText * const text = safe_cast<UIText *>(m_sampleText->DuplicateObject ());

	m_composite->AddChild (text);
	text->Link       ();

	text->SetVisible    (true);
	text->SetText       (ClientTextManager::colorAndFilterText(str, ClientTextManager::TT_systemMessage, false));
	text->SetWidth      (0);
	text->SizeToContent ();
	text->SetOpacity    (0.0f);
	text->SetEnabled    (false);
	text->SetEnabled    (true);

	text->SetTextUnroll     (true);
	text->SetTextUnrollOnce (true);

	if (text->GetWidth () > m_composite->GetWidth ())
		text->SetWidth (m_composite->GetWidth ());
	
	text->SetLocation ((m_composite->GetWidth () - text->GetWidth ()) / 2L, text->GetLocation ().y);


	setIsUpdating (true);
	
	if (wasEmpty)
	{
		const float curTime = Game::getElapsedTime ();
		//-- newbs get longer messages
		if (m_type == T_noob)
			m_timeout = curTime + (timeout_length_noob * CuiPreferences::getSystemMessageDuration ());
		else
			m_timeout = curTime + (timeout_length *      CuiPreferences::getSystemMessageDuration ());
	}
	else
	{
		m_updatePack = true;
	}
}

//----------------------------------------------------------------------

void SwgCuiSystemMessage::onInstantMessageReceived (const CuiInstantMessageManagerElement & elem)
{
	UNREF (elem);
}

//----------------------------------------------------------------------

void SwgCuiSystemMessage::onSystemMessageReceived  (const CuiSystemMessageManagerData & msg)
{
	if (!CuiPreferences::getShowSystemMessages ())
		return;

	const Object * const player = Game::getPlayer ();

	if (!player)
		return;

	if ((msg.flags & CuiSystemMessageManagerData::F_chatBoxOnly) != 0)
		return;

	const CellProperty * const cellProperty = player->getParentCell ();

	if (cellProperty)
	{
		const PortalProperty * const portalProperty = cellProperty->getPortalProperty ();

		if (portalProperty)
		{
			const Object & portalOwner = portalProperty->getOwner ();

			const char * const otname = portalOwner.getObjectTemplateName ();

			if (otname && !strcmp (otname, s_newbieHallTemplate))
			{
				if (m_type == T_noob)
					appendText (msg.translated);

				return;
			}
		}
	}

	if (m_type == T_normal)
		appendText (msg.translated);
}

//======================================================================
