//======================================================================
//
// CuiWorkspace.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiWorkspace.h"

#include "UICanvas.h"
#include "UIClock.h"
#include "UIComposite.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIOpacityEffector.h"
#include "UnicodeUtils.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiPopupHelp.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedMessageDispatch/Message.h"

#include <map>
#include <set>
#include <typeinfo>
#include <vector>
#include <list>

//----------------------------------------------------------------------

namespace
{
	//----------------------------------------------------------------------
	//-- workspace edge attraction
	
	const long EDGE_ATTRACTION_THRESHOLD = 4;
}

//-----------------------------------------------------------------

CuiWorkspace *    CuiWorkspace::ms_gameWorkspace;

//-----------------------------------------------------------------

class CuiWorkspaceGlassPane : public UIWidget
{
public:
	typedef std::vector<UIRect> RectVector;

	explicit CuiWorkspaceGlassPane (UIPage & page) : 
	UIWidget (),
	m_rects ()
	{
		SetName ("GlassPane");
		UIWidget::SetSize (page.GetSize ());
		IGNORE_RETURN (UIWidget::SetProperty (UIWidget::PropertyName::PackSize, Unicode::narrowToWide ("1,1")));
		IGNORE_RETURN (UIWidget::SetProperty (UIWidget::PropertyName::GetsInput, Unicode::narrowToWide ("false")));
		IGNORE_RETURN (page.AddChild (this));
		IGNORE_RETURN (page.MoveChild (this, UIBaseObject::Bottom));
		
	}
	
	virtual UIBaseObject * Clone () const { return 0; }
	virtual UIStyle * GetStyle () const { return 0; }

	void Render (UICanvas & canvas) const
	{
		static UIFloatPoint points [5];
		static const UIColor color(50,0,20,100);

		for (RectVector::iterator it = m_rects.begin (); it != m_rects.end (); ++it)
		{
			const UIRect & rect = *it;
			canvas.ClearTo (color, rect);
		}

		m_rects.clear ();
	}

	void pushRect (const UIRect & rect)
	{
		m_rects.push_back (rect);
	}

private:
	CuiWorkspaceGlassPane ();
	CuiWorkspaceGlassPane & operator= (const CuiWorkspaceGlassPane & rhs);
	CuiWorkspaceGlassPane (const CuiWorkspaceGlassPane & rhs);

	mutable RectVector m_rects;
};
 
//======================================================================

CuiWorkspace::CuiWorkspace (UIPage & page, bool toggleMediatorsEnabled) :
MessageDispatch::Receiver (),
UINotification            (), //lint !e1769
UIEventCallback           (), //lint !e1769
m_enabled                 (false),
m_page                    (page),
m_mediators               (new MediatorSet),
m_glassPane               (new CuiWorkspaceGlassPane (page)),
m_iconAnimations          (new IconAnimationVector),
m_iconMediatorMap         (new IconMediatorMap),
m_toggleMediatorsEnabled  (toggleMediatorsEnabled),
m_raiseMediator           (0),
m_defaultOpacityEnabled   (1.0f),
m_defaultOpacityDisabled  (1.0f),
m_opacityEffectorEnabled  (0),
m_opacityEffectorDisabled (0),
m_iteratingEnabledStates  (false),
m_glowFocused             (0),
m_mediatorFocused         (0),
m_showGlow(false)
{
	m_page.Attach (0);
	connectToMessage (CuiIoWin::Messages::POINTER_INPUT_TOGGLED);

	m_glowFocused = GET_UI_OBJ (page, UIWidget, "glowFocused");

	if (m_glowFocused)
	{
		m_glowFocused->Attach (0);
		m_glowFocused->SetVisible (false);
	}

	UIClock::gUIClock().ListenPerFrame( this );
}

//----------------------------------------------------------------------

CuiWorkspace::~CuiWorkspace ()
{
	UIClock::gUIClock().StopListening( this );

	setEffectors (0, 0);

	{
		for (MediatorSet::iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
		{
			CuiMediator * mediator = *it;
			mediator->getPage ().RemoveCallback (this);

			if (mediator->getContainingWorkspace () == this)
				mediator->setContainingWorkspace (0);

			mediator->release ();
		}
	}

	if (m_mediatorFocused)
	{
		m_mediatorFocused->release ();
		m_mediatorFocused = 0;
	}
	
	if (m_raiseMediator)
	{
		m_raiseMediator->release ();
		m_raiseMediator = 0;
	}

	if (m_glowFocused)
	{
		m_glowFocused->Detach (0);
		m_glowFocused->SetVisible (false);
	}

	m_page.Detach (0);
	delete m_mediators;
	m_mediators = 0;

	{
		for (IconMediatorMap::iterator it = m_iconMediatorMap->begin (); it != m_iconMediatorMap->end (); ++it)
		{
			(*it).first->RemoveCallback (this);
			(*it).first->Detach (0);
		}
	}

	delete m_iconMediatorMap;
	m_iconMediatorMap = 0;
	delete m_iconAnimations;
	m_iconAnimations = 0;
	
	m_glassPane = 0; //m_glassPane is deleted by the ui system
}

//----------------------------------------------------------------------

void CuiWorkspace::updateMediatorEnabledStates (bool force)
{
	const bool pointerVisible = m_enabled && CuiManager::getPointerInputActive () && (CuiManager::getPointerToggledOn () || !CuiManager::getIoWin ().getRadialMenuActiveHack ());

	m_iteratingEnabledStates = true;

	//----------------------------------------------------------------------
	//-- activate handles and enable all mediators
	if (force || pointerVisible)
	{
		for (MediatorSet::iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
		{
			CuiMediator * const mediator = NON_NULL (*it);
			if (mediator->wasVisible () && !mediator->hasState (CuiMediator::MS_closeNextFrame))
			{
				if (pointerVisible || (force && mediator->isStickyVisible ()))
					mediator->activate ();
			}

			if (m_toggleMediatorsEnabled)
				mediator->setEnabled (pointerVisible);
		}
	}
	//----------------------------------------------------------------------
	//-- deactivate handles
	else
	{			
		//-- disable all mediators
		for (MediatorSet::iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
		{
			CuiMediator * const mediator = NON_NULL (*it);
			if (!m_enabled || !mediator->isStickyVisible ())
			{
				if (mediator->isActive ())
				{
					mediator->deactivate ();
					mediator->setWasVisible (true);
				}
			}

			if (m_toggleMediatorsEnabled)
				mediator->setEnabled (false);
		}
	}

	m_iteratingEnabledStates = false;
}

//----------------------------------------------------------------------

void CuiWorkspace::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	if (message.isType (CuiIoWin::Messages::POINTER_INPUT_TOGGLED))
	{
		if (CuiManager::getIoWin ().getRadialMenuActiveHack ())
			return;
		
		updateMediatorEnabledStates (false);
	}
}

//----------------------------------------------------------------------

void CuiWorkspace::addMediator    (CuiMediator & mediator)
{
	DEBUG_FATAL (m_mediators->find (&mediator) != m_mediators->end (), ("don't addMediator 2x, moron."));
	DEBUG_FATAL (m_iteratingEnabledStates, ("Can't addMediator [%s] during m_iteratingEnabledStates", mediator.getMediatorDebugName ().c_str ()));

	UIPage & mediatorPage = mediator.getPage ();

	if (mediatorPage.GetParent () != &m_page)
	{
		mediatorPage.Attach (0);
		if (mediatorPage.GetParent ())
			mediatorPage.GetParent ()->RemoveChild (&mediatorPage);
		m_page.InsertChildAfter (&mediatorPage, 0);
		mediator.getPage ().Link ();
		mediator.getPage ().Pack ();
		mediatorPage.Detach (0);
	}

	DEBUG_FATAL (mediatorPage.GetParent () != &m_page, ("bogus mediator parent in workspace\n"));

	m_mediators->insert (&mediator);
	mediator.setContainingWorkspace (this);

	mediator.getPage ().AddCallback (this);
	mediator.getPage ().SetTabRoot  (true);

	mediator.fetch ();

	if (m_toggleMediatorsEnabled)
		mediator.setEnabled (CuiManager::getPointerInputActive () && (CuiManager::getPointerToggledOn () || !CuiManager::getIoWin ().getRadialMenuActiveHack ()));
	
	if (mediatorPage.IsEnabled ())
		mediatorPage.SetOpacity (m_defaultOpacityEnabled);
	else
		mediatorPage.SetOpacity (m_defaultOpacityDisabled);
	
	if (m_opacityEffectorEnabled)
	{
		UIManager::gUIManager ().CancelEffectorsFor (&mediatorPage);
		mediatorPage.SetPropertyNarrow (UIWidget::PropertyName::OnEnableEffector, m_opacityEffectorEnabled->GetFullPath ());
	}

	if (m_opacityEffectorDisabled)
	{
		UIManager::gUIManager ().CancelEffectorsFor (&mediatorPage);
		mediatorPage.SetPropertyNarrow (UIWidget::PropertyName::OnDisableEffector, m_opacityEffectorDisabled->GetFullPath ());
	}

	mediatorPage.SetPropertyBoolean (UIWidget::PropertyName::TextOpacityRelativeApply, true);
	mediatorPage.SetPropertyFloat   (UIWidget::PropertyName::TextOpacityRelativeMin,   1.0f);
}

//----------------------------------------------------------------------

void CuiWorkspace::removeMediator (CuiMediator & mediator)
{
	DEBUG_FATAL (m_iteratingEnabledStates, ("Can't removeMediator [%s] during m_iteratingEnabledStates", mediator.getMediatorDebugName ().c_str ()));

	for (IconMediatorMap::iterator it = m_iconMediatorMap->begin (); it != m_iconMediatorMap->end (); )
	{
		if ((*it).second == &mediator)
		{
			(*it).first->RemoveCallback (this);
			(*it).first->Detach (0);
			m_iconMediatorMap->erase (it++);
		}
		else
			++it;
	}

	if (m_raiseMediator == &mediator)
	{
		m_raiseMediator->release ();
		m_raiseMediator = 0;
	}

	if (m_mediatorFocused == &mediator)
	{
		m_mediatorFocused->release ();
		m_mediatorFocused = 0;
	}

	m_mediators->erase (&mediator);

	mediator.getPage ().RemoveCallback (this);
	mediator.setContainingWorkspace (0);

	mediator.release ();

	updateGlow ();
}

//-----------------------------------------------------------------

bool CuiWorkspace::iconify (CuiMediator & mediator)
{
	if (!hasMediator (mediator))
		return false;

	if (!mediator.isIconifiable ())
		return false;
	
	CuiWorkspaceIcon * const icon = dynamic_cast<CuiWorkspaceIcon *>(mediator.getIcon ());

	if (!icon)
	{
		WARNING (true, ("Mediator has no icon, can't be minimized."));
		return false;
	}
	
	if (icon->GetParent ())
	{
		if (icon->GetParent () != &m_page)
		{
			icon->Attach (0);
			IGNORE_RETURN (icon->GetParent ()->RemoveChild (icon));
			IGNORE_RETURN (m_page.AddChild (icon));
			icon->Detach (0);
		}
	}
	else
		IGNORE_RETURN (m_page.AddChild (icon));
	
	icon->setWorkspace (this);
	icon->SetVisible (true);
	mediator.getPage ().SetVisible (false);

	icon->AddCallback (this);

	mediator.setIconified (true);

	const std::pair<IconMediatorMap::iterator, bool> insert_retval = m_iconMediatorMap->insert (std::make_pair(icon, &mediator));

	//-- increment refcount if this is a newly added icon
	if (insert_retval.second)
		icon->Attach (0);

	const IconAnimation anim (mediator.getPage ().GetRect (), icon->GetRect ());
	m_iconAnimations->push_back (anim);
	return true;
}

//----------------------------------------------------------------------

void CuiWorkspace::update (float )
{
	updateGlowRect();
}

//----------------------------------------------------------------------

void CuiWorkspace::updateGlowRect () const
{
	if (m_glowFocused)
	{
		if (m_mediatorFocused)
		{
			if(m_showGlow || m_mediatorFocused->getShowFocusedGlowRect())
			{
				m_glowFocused->SetVisible (true);
				m_glowFocused->SetSize     (m_mediatorFocused->getPage ().GetSize () + UISize (4L,4L));
				m_glowFocused->SetLocation (m_mediatorFocused->getPage ().GetLocation () - UIPoint (2L, 2L));
			}
			else
			{
				m_glowFocused->SetVisible (false);
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiWorkspace::updateGlow ()
{
	if (!m_showGlow) 
	{
		if (m_mediatorFocused)
		{
			m_mediatorFocused->release ();
			m_mediatorFocused = 0;
		}
		
		const UIBaseObject::UIObjectList & olist = m_page.GetChildrenRef ();
		
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIBaseObject * const obj = *it;
			if (obj->IsA (TUIPage))
			{
				UIPage * const page = safe_cast<UIPage *>(obj);
				if (page->IsVisible ())
				{
					
					CuiMediator * const mediator = findMediatorByPage (*page);
					if (mediator && mediator->getShowFocusedGlowRect() && mediator->isActive () && !mediator->isStickyVisible () && typeid (*mediator) != typeid(CuiPopupHelp))
					{
						m_mediatorFocused = mediator;
						m_mediatorFocused->fetch ();
						if (m_glowFocused)
						{
							UIPage * const mediatorParent = safe_cast<UIPage *>(mediator->getPage ().GetParent ());
							if (mediatorParent && mediatorParent->IsA (TUIPage))
							{
								updateGlowRect ();
								
								m_glowFocused->GetParent ()->RemoveChild (m_glowFocused);
								
								UIPage * const mediatorParentPage = safe_cast<UIPage *>(mediatorParent);
								mediatorParentPage->InsertChildAfter (m_glowFocused, &mediator->getPage ());
							}
							else
								m_glowFocused->SetVisible (false);
						}
						return;
					}
				}
			}
		}
		
		if (m_glowFocused)
			m_glowFocused->SetVisible (false);
	}
}

//-----------------------------------------------------------------

void CuiWorkspace::Notify( UINotificationServer *, UIBaseObject *, Code  )
{	
	if (m_raiseMediator)
	{
		UIPage * const parent = dynamic_cast<UIPage *>(m_raiseMediator->getPage ().GetParent ());
		if (parent && parent == &m_page)
		{
			parent->MoveChild (&m_raiseMediator->getPage (), UIBaseObject::Top);
			m_raiseMediator->raisePopupHelp ();
		}

		m_raiseMediator->getPage ().SetFocus ();

		m_raiseMediator->release ();
		m_raiseMediator = 0;

		updateGlow ();
	}

	{
		for (IconAnimationVector::iterator it = m_iconAnimations->begin (); it != m_iconAnimations->end ();)
		{
			IconAnimation & anim = *it;
			
			
			const UIRect diff (
				(anim.endRect.left   - anim.curRect.left)   / 3,
				(anim.endRect.top    - anim.curRect.top)    / 3,
				(anim.endRect.right  - anim.curRect.right)  / 3,
				(anim.endRect.bottom - anim.curRect.bottom) / 3);
			
			if (diff.left == 0 && diff.right == 0 && diff.top == 0 && diff.bottom == 0)
			{				
				//-- one last frame
				m_glassPane->pushRect (anim.curRect);
				it = m_iconAnimations->erase (it);
			}
			else
			{
				anim.curRect.left   += diff.left;
				anim.curRect.right  += diff.right;
				anim.curRect.top    += diff.top;
				anim.curRect.bottom += diff.bottom;
				
				m_glassPane->pushRect (anim.curRect);
				
				++it;
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiWorkspace::moveIcon (CuiWorkspaceIcon & icon, const UIPoint & pos)
{
	const UISize iconSize (icon.GetSize ());
	//UIRect theRect (pos.x, pos.y, pos.x + frameRect.Width (), pos.y + frameRect.Height ());

	const UISize workspaceSize (m_page.GetSize ());
	const UIPoint workspaceCenter (workspaceSize.x / 2, workspaceSize.y / 2);

	//----------------------------------------------------------------------
	//-- workspace edge resistance

	UIPoint thePos (pos);

	//----------------------------------------------------------------------
	//
	//   h = workspace height
	//   s = slope (h/w)
	//
	//                y <= h - x * s
	//             && y <= x * s
	//
	//           0,0
	//             *----------*
	//             |\        /|
	//             | \      / | 
	//  y <= h-x*s |  \    /  |    y >  h - x * s
	// && y >  x*s |   \  /   | && y <= x * s
	//             |    \/    |
	//             |    /\    |
	//             |   /  \   |
	//             |  /    \  |
	//             | /      \ |
	//             |/        \|
	//             *----------* h,h
	//
	//                y > h - x * s
	//             && y > x * s


	const float slope = static_cast<float>(workspaceSize.y) / static_cast<float>(workspaceSize.x);

	const long x_slope         = static_cast<long> (static_cast<float>(thePos.x) * slope);
	const long h_minus_x_slope = workspaceSize.y - x_slope;

	thePos.x = std::max (0L, std::min (thePos.x, workspaceSize.x - iconSize.x));
	thePos.y = std::max (0L, std::min (thePos.y, workspaceSize.y - iconSize.y));

	if (thePos.y > x_slope)
	{
		if (thePos.y <= h_minus_x_slope)
			thePos.x = 0;
		else
			thePos.y = (workspaceSize.y - iconSize.y);
	}
	else
	{
		if (thePos.y <= h_minus_x_slope)
			thePos.y = 0;
		else
			thePos.x = (workspaceSize.x - iconSize.x);
	}

	thePos.x = (thePos.x / iconSize.x) * iconSize.x;
	thePos.y = (thePos.y / iconSize.y) * iconSize.y;
 
	icon.SetLocation (thePos);
} //lint !e1762 //not this time

//-----------------------------------------------------------------

void CuiWorkspace::setGameWorkspace(CuiWorkspace * workspace)
{
	ms_gameWorkspace = workspace;
}

//-----------------------------------------------------------------

CuiWorkspace* CuiWorkspace::getGameWorkspace()
{
	return ms_gameWorkspace;
}

//-----------------------------------------------------------------


void CuiWorkspace::restoreFromIcon (UIWidget * icon, CuiMediator & mediator)
{	
	if (!icon)
	{
		if (m_iconMediatorMap->find (mediator.getIcon ()) != m_iconMediatorMap->end ())
			icon = mediator.getIcon ();
	}

	if (icon)
	{
		icon->SetVisible (false);
		icon->RemoveCallback (this);
	}

	mediator.setIconified (false);

	//@todo a better way to accomplish this
	mediator.deactivate ();
	mediator.activate ();

} //lint !e1762 //naaa

//-----------------------------------------------------------------

bool CuiWorkspace::OnMessage( UIWidget *context, const UIMessage & msg )
{
	if (context->GetParent () == &m_page)
	{
		if (msg.Type == UIMessage::LeftMouseDown ||
			msg.Type == UIMessage::MiddleMouseDown ||
			msg.Type == UIMessage::RightMouseDown)
		{
			UIPage * const page = dynamic_cast<UIPage *> (context);
			
			if (page)
			{
				if (m_raiseMediator)
					m_raiseMediator->release ();
				
				m_raiseMediator = findMediatorByPage (*page);
				
				if (m_raiseMediator)
					m_raiseMediator->fetch ();
			}
		}
		return true;
	}
	
	
	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		const IconMediatorMap::iterator it = m_iconMediatorMap->find (context);
		
		if (it != m_iconMediatorMap->end ())
		{
			restoreFromIcon ((*it).first, *NON_NULL ((*it).second));
			return false;
		}
	}

	return true;
} //lint !e1762 //naaa

//----------------------------------------------------------------------

const UIRect CuiWorkspace::findClosestWidgetEdges  (const UIWidget * context, const UIRect & targetRect) const
{
	const UISize & workspaceSize = m_page.GetSize ();
	UIRect closestEdges (0, 0, workspaceSize.x, workspaceSize.y);
	
	//-----------------------------------------------------------------
	
	for (MediatorSet::iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
	{
		const UIPage & page = (*it)->getPage ();
		
		if (!page.IsVisible () || !page.GetsInput () || page.GetSize () == workspaceSize || &page == context)
			continue;
		
		const UIRect rect (page.GetRect ());
		
		if ((rect.left < targetRect.right && rect.right > targetRect.left) ||
			(rect.right > targetRect.left && rect.left < targetRect.right))
		{
			// rect is above the targetrect
			if (rect.bottom > closestEdges.top && abs (rect.bottom - targetRect.top) < abs (closestEdges.top - targetRect.top))
				closestEdges.top = rect.bottom;
			
			//-- rect is below the targetrect
			if (rect.top < closestEdges.bottom  && abs (rect.top - targetRect.bottom) < abs (closestEdges.bottom - targetRect.bottom))
				closestEdges.bottom = rect.top;
		}
		
		if ((rect.top < targetRect.bottom && rect.bottom > targetRect.top) ||
			(rect.bottom > targetRect.top && rect.top < targetRect.bottom))
		{
			//-- rect is left of the targetRect
			if (rect.right > closestEdges.left && abs (rect.right - targetRect.left) < abs (closestEdges.left - targetRect.left))
				closestEdges.left = rect.right;
			
			//-- rect is right of the targetRect
			if (rect.left < closestEdges.right && abs (rect.left - targetRect.right) < abs (closestEdges.right - targetRect.right))
				closestEdges.right = rect.left;
		}
	}

	return closestEdges;
}

//----------------------------------------------------------------------

const UIRect CuiWorkspace::pushRectToWindowEdges (const UIRect & targetRect, const UIRect & closestEdges) const
{

	UIRect pushedRect (targetRect);
	
	//-----------------------------------------------------------------
	
	if (abs (pushedRect.right - closestEdges.right) < EDGE_ATTRACTION_THRESHOLD)
	{
		pushedRect.left  = closestEdges.right - pushedRect.Width ();
		pushedRect.right = closestEdges.right;
	}
	
	if (abs (pushedRect.left - closestEdges.left) < EDGE_ATTRACTION_THRESHOLD)
	{
		pushedRect.right = closestEdges.left + pushedRect.Width ();
		pushedRect.left  = closestEdges.left;
	}
	
	if (abs (pushedRect.bottom - closestEdges.bottom) < EDGE_ATTRACTION_THRESHOLD)
	{
		pushedRect.top    = closestEdges.bottom - pushedRect.Height ();
		pushedRect.bottom = closestEdges.bottom;
	}
	
	if (abs (pushedRect.top - closestEdges.top) < EDGE_ATTRACTION_THRESHOLD)
	{
		pushedRect.bottom = closestEdges.top + pushedRect.Height ();
		pushedRect.top    = closestEdges.top;
	}
	

	return pushedRect;
}

//----------------------------------------------------------------------

const UIRect CuiWorkspace::autoPositionMovingRect (UIWidget * context, const UIRect & targetRect, bool squash) const
{
	const UIRect & closestEdges = findClosestWidgetEdges (context, targetRect);
	const UIRect & finalRect    = pushRectToWindowEdges  (targetRect, closestEdges);
	UIRect resultRect (finalRect);

	if (squash)
	{
		resultRect.left   = std::max (finalRect.left,   targetRect.left);
		resultRect.top    = std::max (finalRect.top,    targetRect.top);
		resultRect.right  = std::min (finalRect.right,  targetRect.right);
		resultRect.bottom = std::min (finalRect.bottom, targetRect.bottom);
	}
	else
		resultRect = finalRect;

	if (context)
	{
		context->SetRect (resultRect);
		UIRect actualRect = context->GetRect ();


		context->SetRect (actualRect);

		if (m_mediatorFocused && &m_mediatorFocused->getPage () == context)
			updateGlowRect ();
	}

	return resultRect;
}

//-----------------------------------------------------------------

void CuiWorkspace::OnWidgetRectChanging (UIWidget * context, UIRect & targetRect)
{
	//-- move only
	if (context->GetWidth () == targetRect.Width () &&
		context->GetHeight () == targetRect.Height ())
	{		
		targetRect = autoPositionMovingRect (context, targetRect, false);
	}

	//-- resize
	else
	{
	}
}

//----------------------------------------------------------------------

void CuiWorkspace::OnWidgetRectChanged (UIWidget * context)
{
	NOT_NULL (context);

	if (m_mediatorFocused && &m_mediatorFocused->getPage () == context)
	{
		updateGlowRect ();
		m_mediatorFocused->saveSettings ();
	}
	else if (context->IsA (TUIPage))
	{
		CuiMediator * const mediator = findMediatorByPage   (*safe_cast<UIPage *>(context));
		if (mediator)
			mediator->saveSettings ();
	}
}

//-----------------------------------------------------------------

void CuiWorkspace::restore (CuiMediator & mediator)
{
	if (!mediator.restore ())
	{
		UIRect rect (mediator.getRestoreRect ());
		mediator.getPage ().SetRect (rect);
	}
}

//-----------------------------------------------------------------

void CuiWorkspace::maximize (CuiMediator & mediator)
{
	if (!mediator.isMaximizable ())
		return;

	if (!mediator.maximize ())
	{
		const UISize & size = m_page.GetSize ();
		mediator.getPage ().SetSize (size);
		mediator.getPage ().SetLocation (UIPoint::zero);
	}
}

//-----------------------------------------------------------------

void CuiWorkspace::close (CuiMediator & mediator)
{
	if (!mediator.isCloseable ())
		return;

	//-- give the mediator a chance to close itself, otherwise delete it

	if (!mediator.close ())
	{
		//-- this mediator wants to stick around in a deactivated state
		if (mediator.isCloseDeactivates ())
		{
			mediator.deactivate ();
		}
		else
		{
			//-- don't assume that the mediator is still part of the workspace after a close () call
			
			UIWidget * const parent = dynamic_cast<UIWidget *>(mediator.getPage ().GetParent ());
			if (parent)
				parent->RemoveChild (&mediator.getPage ());
			
			removeMediator (mediator);
		}
	}
}

//----------------------------------------------------------------------

void CuiWorkspace::setEnabled (bool b)
{
	m_enabled = b;
	updateMediatorEnabledStates (b);
}

//----------------------------------------------------------------------

void CuiWorkspace::cancelFocusForMediator (const CuiMediator & mediator)
{
	if (m_raiseMediator == &mediator)
	{
		m_raiseMediator->release ();
		m_raiseMediator = 0;
	}
}

//----------------------------------------------------------------------

void CuiWorkspace::focusMediator (CuiMediator & mediator, bool b)
{
	if (mediator.getPage ().GetParent () != &m_page)
	{
		WARNING (true, ("don't do that"));
		return;
	}
	
	if (b)
	{		
		for (MediatorSet::const_iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
		{
			if (*it != &mediator)
				focusMediator (*NON_NULL(*it), false);
		}

		m_page.ReleaseMouseLock (UIPoint::zero);

		mediator.fetch ();
		if (m_raiseMediator)
			m_raiseMediator->release ();

		m_raiseMediator = &mediator;

//		m_page.MoveChild (&mediator.getPage (), UIBaseObject::Top);
		mediator.getPage ().SetFocus ();

	}
}

//-----------------------------------------------------------------

bool CuiWorkspace::hasMediator (const CuiMediator & mediator) const
{
	return m_mediators->find (const_cast<CuiMediator *>(&mediator)) != m_mediators->end ();
}

//-----------------------------------------------------------------

CuiMediator * CuiWorkspace::findMediatorByType   (const type_info & info)
{
	for (MediatorSet::const_iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
	{
		CuiMediator * const mediator = NON_NULL (*it);
		const type_info & mediator_info = typeid (*mediator);
		if (mediator_info == info)
			return mediator;
	}

	return 0;
}

//----------------------------------------------------------------------

CuiMediator * CuiWorkspace::findMediatorByPage   (const UIPage & page)
{
	for (MediatorSet::const_iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
	{
		CuiMediator * const mediator = NON_NULL (*it);
		if (&mediator->getPage () == &page)
			return mediator;
	}
	
	return 0;
}


//----------------------------------------------------------------------

CuiMediator * CuiWorkspace::focusTopMediator     (const CuiMediator * ignoreMediator)
{
	const UIBaseObject::UIObjectList & olist = m_page.GetChildrenRef ();

	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		const UIBaseObject * const obj = *it;
		if (obj && obj->IsA (TUIPage))
		{
			const UIPage * const page = safe_cast<const UIPage *> (obj);
			CuiMediator * const mediator = findMediatorByPage (*page);
			if (mediator && mediator != ignoreMediator && mediator->isActive ())
			{
				focusMediator (*mediator, true);
				return mediator;
			}
		}
	}

	return 0;
}

//----------------------------------------------------------------------

void CuiWorkspace::positionMediator     (CuiMediator & mediator, const UIPoint & pt, bool squash)
{
	UIRect targetRect (pt, mediator.getPage ().GetSize ());
	autoPositionMovingRect (&mediator.getPage (), targetRect, squash);
	/*
	mediator.getPage ().SetLocation (pt);

	UIRect rect (mediator.getPage ().GetRect ());

	if (rect.bottom > m_page.GetHeight ())
		rect -= UIPoint (0L, rect.bottom - m_page.GetHeight ());
	if (rect.right > m_page.GetWidth ())
		rect -= UIPoint (rect.right - m_page.GetWidth (), 0L);

	if (rect.left < 0)
		rect += UIPoint (-rect.left, 0L);
	if (rect.top < 0)
		rect += UIPoint (0L, -rect.top);

	mediator.getPage ().SetLocation (rect.Location ());
	*/
}

//----------------------------------------------------------------------

void CuiWorkspace::positionMediator     (CuiMediator & mediator)
{
	positionMediator (mediator, UIManager::gUIManager ().GetLastMouseCoord ());
}

//----------------------------------------------------------------------

void CuiWorkspace::updateOpacities      (float opacityEnabled, float opacityDisabled)
{
	for (MediatorSet::iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
	{
		CuiMediator * const mediator = NON_NULL (*it);

		UIPage & page = mediator->getPage ();

		if (page.IsEnabled ())
			page.SetOpacity (opacityEnabled);
		else
			page.SetOpacity (opacityDisabled);
	}
	
	m_defaultOpacityEnabled    = opacityEnabled;
	m_defaultOpacityDisabled   = opacityDisabled;
	
	if (m_opacityEffectorEnabled)
		m_opacityEffectorEnabled->SetTargetOpacity (m_defaultOpacityEnabled);
	
	if (m_opacityEffectorDisabled)
		m_opacityEffectorDisabled->SetTargetOpacity (m_defaultOpacityDisabled);
}

//----------------------------------------------------------------------

void CuiWorkspace::setEffectors         (UIOpacityEffector * enableEffector, UIOpacityEffector * disableEffector)
{
	if (enableEffector)
		enableEffector->Attach (0);
	if (disableEffector)
		disableEffector->Attach (0);

	if (m_opacityEffectorEnabled)
		m_opacityEffectorEnabled->Detach (0);

	if (m_opacityEffectorDisabled)
		m_opacityEffectorDisabled->Detach (0);

	m_opacityEffectorEnabled   = enableEffector;
	m_opacityEffectorDisabled  = disableEffector;

	if (m_opacityEffectorEnabled)
		m_opacityEffectorEnabled->SetTargetOpacity (m_defaultOpacityEnabled);
	
	if (m_opacityEffectorDisabled)
		m_opacityEffectorDisabled->SetTargetOpacity (m_defaultOpacityDisabled);
}

//----------------------------------------------------------------------

void CuiWorkspace::resetAllToDefaultSizeAndLocation ()
{
	for (MediatorSet::iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
	{
		CuiMediator * mediator = *it;
		if(mediator)
		{
			mediator->resetToDefaultSizeAndLocation();
		}
	}
}

//----------------------------------------------------------------------

void CuiWorkspace::saveAllSettings() const
{
	if (m_mediators == NULL)
		return;

	for (MediatorSet::iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
	{
		CuiMediator * mediator = *it;
		if (mediator != NULL)
		{
			mediator->saveSettings();
		}
	}
}

//----------------------------------------------------------------------

void CuiWorkspace::setShowGlow(bool glow)
{
	m_showGlow = glow;
}

//----------------------------------------------------------------------

bool CuiWorkspace::getShowGlow() const
{
	return m_showGlow;
}

//----------------------------------------------------------------------

CuiMediator * CuiWorkspace::getFocusMediator() const
{
	return m_mediatorFocused;
}

//----------------------------------------------------------------------

void CuiWorkspace::focusGlowMediatorByPage(UIPage & page)
{
	if (m_showGlow) 
	{
		if (m_mediatorFocused)
		{
			m_mediatorFocused->release();
			m_mediatorFocused->getPage().SetTooltip(Unicode::emptyString);
		}

		m_mediatorFocused = NULL;

		UIPage const * currentPage = &page;
		UIPage const * root = UIManager::gUIManager().GetRootPage();
		do
		{
			for (MediatorSet::const_iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
			{
				CuiMediator * const mediator = NON_NULL (*it);
				if (&mediator->getPage() == currentPage)
				{
					m_mediatorFocused = mediator;
					break;
				}
			}
			currentPage = static_cast<UIPage const *>(currentPage->GetParent(TUIPage));
		}
		while (currentPage && (currentPage != root));

		if (m_mediatorFocused)
		{
			m_mediatorFocused->fetch();

			UIString const & tooltip = Unicode::narrowToWide(m_mediatorFocused->getMediatorDebugName() + " - " + page.GetFullPath().c_str());
			
			m_mediatorFocused->getPage().SetTooltip(tooltip);
			m_mediatorFocused->getPage().SetTooltipDelay(false);

			page.SetTooltip(tooltip);
			page.SetTooltipDelay(false);
		}
	}
}

//----------------------------------------------------------------------

const UIComposite & CuiWorkspace::getComposite () const
{
	FATAL(!m_page.IsA(TUIComposite), ("[%s] is not a composite!", m_page.GetName().c_str()));

	return *safe_cast<UIComposite *>(&m_page);
}

//----------------------------------------------------------------------

UIComposite & CuiWorkspace::getComposite ()
{
	FATAL(!m_page.IsA(TUIComposite), ("[%s] is not a composite!", m_page.GetName().c_str()));

	return *safe_cast<UIComposite *>(&m_page);
}

//======================================================================
