//======================================================================
//
// CuiConversationMenu.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiConversationMenu.h"

#include "clientGame/clientObject.h"
#include "clientGame/Game.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/CuiChatBubbleManager.h"
#include "clientUserInterface/CuiConversationManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Timer.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"


#include "UIClock.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UIText.h"

#include "UnicodeUtils.h"

//======================================================================

namespace CuiConversationMenuNamespace
{
	float const s_killWindowDistance = 8.0f;	
	Timer s_closeTimer(17.0f);
	bool s_closeTimerActive = false;
}

using namespace CuiConversationMenuNamespace;

//----------------------------------------------------------------------

CuiConversationMenu::CuiConversationMenu (UIPage & parent) :
UIEventCallback              (),
m_frame                      (0),
m_text                       (0),
m_popup                      (0),
m_valid                      (false),
m_responsesDirty             (false),
m_targetDirty                (false),
m_callback                   (new MessageDispatch::Callback),
m_lastHeadScreenPoint        (),
m_lastDesiredHeadScreenPoint (),
m_headPointsInitialized      (false),
m_lastUpdateSecs             (0.0f),
m_canSee                     (false),
m_requestPointer(false)
{
	UIText * const convoText       = GET_UI_OBJ (parent, UIText,      "/Convo.ground.text");
	UIPage * const convoFrame      = GET_UI_OBJ (parent, UIPage,      "/Convo.ground.frame");
	UIPopupMenu * const convoPopup = GET_UI_OBJ (parent, UIPopupMenu, "/Convo.ground.buttons");

	m_text   = safe_cast<UIText *>     (NON_NULL (convoText->DuplicateObject  ()));
	m_frame  = safe_cast<UIPage *>     (NON_NULL (convoFrame->DuplicateObject ()));
	m_popup  = safe_cast<UIPopupMenu *>(NON_NULL (convoPopup->DuplicateObject ()));
	m_popup->SetPopupButtonBehavior (false);

	parent.InsertChildAfter (m_text,  0);
	parent.InsertChildAfter (m_frame, 0);
	parent.InsertChildAfter (m_popup, 0);

	m_text->Link ();
	m_frame->Link ();
	m_popup->Link ();

	m_popup->SetSelectable (true);
	m_popup->AddCallback (this);

	m_text->Attach  (0);
	m_frame->Attach (0);
	m_popup->Attach (0);

	setValid (false);
	setVisible(false);

	m_callback->connect(*this, &CuiConversationMenu::onResponsesChanged, static_cast<CuiConversationManager::Messages::ResponsesChanged *>(0));
	m_callback->connect(*this, &CuiConversationMenu::onTargetChanged,    static_cast<CuiConversationManager::Messages::TargetChanged *>(0));
	m_callback->connect(*this, &CuiConversationMenu::onConversationEnded,    static_cast<CuiConversationManager::Messages::ConversationEnded *>(0));

	UIClock::gUIClock ().ListenPerFrame (this);
}

//----------------------------------------------------------------------

CuiConversationMenu::~CuiConversationMenu ()
{
	setVisible(false);

	UIClock::gUIClock ().StopListening (this);

	m_popup->RemoveCallback (this);

	m_text->Detach  (0);
	m_frame->Detach (0);
	m_popup->Detach (0);

	m_text  = 0;
	m_frame = 0;
	m_popup = 0;

	m_callback->disconnect(*this, &CuiConversationMenu::onResponsesChanged, static_cast<CuiConversationManager::Messages::ResponsesChanged *>(0));
	m_callback->disconnect(*this, &CuiConversationMenu::onTargetChanged,    static_cast<CuiConversationManager::Messages::TargetChanged *>(0));
	m_callback->disconnect(*this, &CuiConversationMenu::onConversationEnded,    static_cast<CuiConversationManager::Messages::ConversationEnded *>(0));

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void  CuiConversationMenu::OnPopupMenuSelection (UIWidget * context)
{
	if (context != m_popup || !context->IsA (TUIPopupMenu))
		return;

	if (!m_valid) 
		return;

	UIPopupMenu const * const pop = safe_cast<UIPopupMenu *>(context);

	long const index = pop->GetSelectedIndex ();

	if (index >= 0)
	{
		if (index >= static_cast<long>(CuiConversationManager::getResponses ().size ()))
			CuiConversationManager::stop ();
		else
			CuiConversationManager::respond (CuiConversationManager::getTarget (), index);
	}
}

//----------------------------------------------------------------------

bool CuiConversationMenu::OnMessage (UIWidget * context, const UIMessage & msg)
{
	if (m_popup == context)
	{
		if (msg.Type == UIMessage::KeyDown && msg.Keystroke == UIMessage::Escape)
		{
			if (m_valid) 
			{
				CuiConversationManager::stop ();
				return false;
			}
		}
	}
	
	return true;
}

//-----------------------------------------------------------------

void  CuiConversationMenu::onResponsesChanged (const bool &)
{
	m_responsesDirty = true;
}

//----------------------------------------------------------------------

void CuiConversationMenu::onTargetChanged (const bool &)
{
	m_targetDirty = true;

	if (isClientOnlyMode()) 
	{
		requestPointer(false);
	}
}

//----------------------------------------------------------------------

void CuiConversationMenu::setVisible (bool b)
{
	bool const focus   = b && !m_frame->IsVisible();
	bool const unfocus = !b && m_frame->IsVisible();

	m_text->SetVisible  (m_canSee && b);
	m_frame->SetVisible (b);
	m_popup->SetVisible (m_canSee && b);

	if (focus)
	{
		requestPointer(true);
		m_popup->SetFocus();
	}
	else if (unfocus)
	{
		requestPointer(false);
	}
}

//----------------------------------------------------------------------

void CuiConversationMenu::setValid (bool b)
{
	m_valid = b;
	if (!b)
		setVisible (false);
}

//----------------------------------------------------------------------

void CuiConversationMenu::updateResponses ()
{
	m_responsesDirty = false;

	m_popup->ClearItems ();
	
	const CuiConversationManager::StringVector & sv = CuiConversationManager::getResponses ();
	if (!sv.empty ())
	{
		int i = 0;
		for (CuiConversationManager::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it, ++i)
		{
			const Unicode::String & response = *it;
			char buf [64];
			_itoa (i, buf, 10);
			
			m_popup->AddItem (buf, response);
		}

		//-- @todo: make popup menus support separators
	}

	//-- @todo: localized stop conversing option...
	if (CuiConversationManager::getUseDefaultResponse() && !isClientOnlyMode()) 
	{
		m_popup->AddItem ("-1", CuiStringIds::convo_stop_conversing.localize());
	}

	m_popup->Link ();
	m_popup->SetFocus ();
}

//----------------------------------------------------------------------

bool CuiConversationMenu::updatePosition (const Camera & camera)
{
	if (!m_valid)
		return false;

	const ClientObject * const obj = safe_cast<ClientObject *>(CuiConversationManager::getTarget ().getObject ());

	if (!obj)
	{
		WARNING (true, ("Updating position for a valid CuiConversationMenu, but no object found."));
		m_valid = false;
		return false;
	}

	// don't adjust position for ship conversations
	if (obj->asShipObject())
		return false;


	Vector objectHeadPoint = CuiObjectTextManager::getCurrentObjectHeadPoint_o (*obj);
	objectHeadPoint.y *= 1.05f;
	Vector const & headPoint = obj->rotateTranslate_o2w (objectHeadPoint);

	if (CuiConversationManager::isClientOnlyMode())
	{
		if (headPoint.magnitudeBetween(camera.getPosition_w()) > s_killWindowDistance)
		{
#if END_CHAT_IN_DIALOG
			CuiChatBubbleManager::enqueueChat(CuiConversationManager::getTarget(),
				CuiConversationManager::getLastMessage().c_str(), 0, 0, 32);
#endif
			CuiConversationManager::stopClientOnlyConversation();
		}
		else if (s_closeTimerActive && s_closeTimer.updateZero(Clock::frameTime()))
		{
			CuiConversationManager::stopClientOnlyConversation();
		}
	}

	Vector headScreenVect;

	UIFloatPoint headScreenFloatPt;
	if (camera.projectInWorldSpace (headPoint, &headScreenFloatPt.x, &headScreenFloatPt.y, 0, false))
	{
		m_lastDesiredHeadScreenPoint = headScreenFloatPt;
		if (!m_headPointsInitialized)
		{
			m_lastHeadScreenPoint = m_lastDesiredHeadScreenPoint;
			m_headPointsInitialized = true;
		}

		//headScreenFloatPt = m_lastHeadScreenPoint;

		objectHeadPoint.y *= 0.75f;
		const Vector chinPoint (obj->getPosition_w () + objectHeadPoint);

		Vector chinScreenVect;

		if (camera.projectInWorldSpace (chinPoint, &chinScreenVect.x, &chinScreenVect.y, 0, false))
		{
			const UIPoint headScreenPt (static_cast<long>(headScreenFloatPt.x), static_cast<long>(headScreenFloatPt.y));
			const UIPoint chinScreenPt (static_cast<long>(chinScreenVect.x),    static_cast<long>(chinScreenVect.y));
			
			m_frame->SetSize (UIPoint (abs (chinScreenPt.y - headScreenPt.y) * 2L / 3L, abs (chinScreenPt.y - headScreenPt.y)));
			
			UIPoint convoFramePt ( headScreenPt );
			convoFramePt.x -= m_frame->GetWidth () / 2;
			m_frame->SetLocation (convoFramePt);
			
			{
				m_text->SetPreLocalized (true);
				m_text->SetLocalText (CuiConversationManager::getLastMessage ());
				m_text->SizeToContent ();
				UIPoint convoTextPt (convoFramePt);
				convoTextPt.x -= m_text->GetWidth ();
				convoTextPt.y -= ( m_text->GetHeight () - m_frame->GetHeight () ) / 2;
				m_text->SetLocation (convoTextPt);
			}
			
			{			
  				UIPoint convoButtonsPt (convoFramePt);
				convoButtonsPt.x += m_frame->GetWidth ();
				convoButtonsPt.y -= ( m_popup->GetHeight () - m_frame->GetHeight () ) / 2;
				m_popup->SetLocation (convoButtonsPt);
			}

			m_headPointsInitialized = true;

			setVisible (true);
		}
		else
		{
			m_headPointsInitialized = false;
			setVisible (false);
		}
	}
	else
	{
		m_headPointsInitialized = false;
		setVisible (false);
	}

	return true;
}

//----------------------------------------------------------------------

void CuiConversationMenu::Notify( UINotificationServer *, UIBaseObject *, Code  )
{
	const float currentTimeSecs = Game::getElapsedTime ();
	
	if (m_targetDirty)
	{
		m_headPointsInitialized = false;
		m_lastUpdateSecs        = currentTimeSecs;
		m_targetDirty           = false;
		setValid((CuiConversationManager::getTarget() != NetworkId::cms_invalid) && !Game::isHudSceneTypeSpace());
	}

	if (m_responsesDirty)
		updateResponses();

	if (m_valid)
	{
		if (m_headPointsInitialized)
		{
			const float deltaTime       = currentTimeSecs - m_lastUpdateSecs;
			const float lerp_factor     = 0.5f;
			const float t               = std::min (std::max (0.0f, deltaTime * lerp_factor), 1.0f);
			m_lastHeadScreenPoint       = UIFloatPoint::lerp (m_lastHeadScreenPoint, m_lastDesiredHeadScreenPoint, t);
		}

		m_lastUpdateSecs = currentTimeSecs;

		const Object * const targetObject = CuiConversationManager::getTarget ().getObject ();
		m_canSee = false;
		if (targetObject)
		{
			float timeFactor = 0.0f;
			m_canSee = CuiObjectTextManager::canSee (*targetObject, timeFactor);
		}
	}
}

//----------------------------------------------------------------------

bool CuiConversationMenu::isClientOnlyMode() const
{
	return CuiConversationManager::isClientOnlyMode();
}

//----------------------------------------------------------------------

void CuiConversationMenu::setCloseTimer(bool enabled, float timer)
{
	s_closeTimer.setExpireTime(timer);
	s_closeTimerActive = enabled;
}

//----------------------------------------------------------------------

void CuiConversationMenu::onConversationEnded(bool const & server)
{
	setCloseTimer(!server, CuiConversationManager::getClientOnlyWindowCloseTime());
}

//----------------------------------------------------------------------

void CuiConversationMenu::requestPointer(bool requested)
{
	if (m_requestPointer != requested) 
	{
		// flip state.
		m_requestPointer = requested;

		CuiManager::requestPointer(requested);
	}
}


//======================================================================
