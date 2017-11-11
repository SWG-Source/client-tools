#include "_precompile.h"

#include "UIActionListener.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UICanvasGenerator.h"
#include "UIClock.h"
#include "UICursor.h"
#include "UICursorInterface.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIEffector.h"
#include "UIIMEManager.h"
#include "UIList.h"
#include "UILocalizedStringFactory.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPacking.h"
#include "UIPage.h"
#include "UIPalette.h"
#include "UIPaletteRegistrySetup.h"
#include "UIPopupMenu.h"
#include "UIRenderHelper.h"
#include "UIScriptEngine.h"
#include "UISoundCanvas.h"
#include "UISystemDependancies.h"
#include "UIText.h"
#include "UITooltipStyle.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <unordered_map>
#include <list>
#include <map>
#include <set>
#include <vector>

#if WIN32
// Work around for buggy windows header #define of PlaySound
#undef PlaySound

#pragma warning (disable:4503) // truncated symbol name

#endif // WIN32

//----------------------------------------------------------------------

namespace UIManagerNamespace
{
	bool ms_effectorListLock = false;

	typedef std::multimap<UIManager::EffectToken, UIString> EffectTokenNameMap;
	EffectTokenNameMap ms_effectTokenNameMap;

	char const * const cms_effectTokenActive = "active";
	char const * const cms_effectTokenAssignment = "=";
	char const * const cms_effectTokenCancel = "cancel";
	char const * const cms_effectTokenEffect = "effect";
	char const * const cms_effectTokenPage = "page";
	char const * const cms_effectTokenReset = "reset";
	char const * const cms_effectTokenUi = "ui:";
	char const * const cms_effectTokenShowMediator = "showMediator";
	char const * const cms_effectTokenHideMediator = "hideMediator";


	typedef ui_stdvector<UIWidget *>::fwd WidgetVector; 
	WidgetVector ms_popContextWidgetsNextFrame;
	
	bool ms_isRefreshing = false;
	bool ms_drawWidgetBorders = false;

	void drawWidgetRect(UICanvas & canvas, UIWidget * widget);
	void drawWidgetBorders(UICanvas & canvas, UIWidget * widget);

	long const s_defaultToolTipCharacterWidth = 35;
}

using namespace UIManagerNamespace;

//----------------------------------------------------------------------

UIManager * volatile UIManager::gSingleton = 0;
//static     bool UIManager::isUIReady();
bool UIManager::gIsUIReady = false;



//----------------------------------------------------------------------

UIManager::ContextInfo::ContextInfo () :
message         (new UIMessage),
messageComplete (0),
delay           (15),
countdown       (0),
pending         (false),
requestPoint    (),
requestData     (0)
{
}

//----------------------------------------------------------------------

UIManager::ContextInfo::~ContextInfo ()
{
	delete message;
	message = 0;

	delete messageComplete;
	messageComplete = 0;
}

//----------------------------------------------------------------------

bool UIManager::ContextInfo::update (long Ticks)
{
	if (!pending)
		return false;
	
	countdown -= Ticks;
	
	if (countdown < 0 )
	{
		pending = false;
		
		UIWidget * activeWidget = static_cast<UIWidget *>(UIManager::gUIManager ().GetRootPage ()->GetWidgetFromPoint (requestPoint, true));
		
		if (activeWidget)
		{
			activeWidget = activeWidget->FindFirstContextCapableWidget (requestData ? false : true);
			
			if (activeWidget)
			{
				UIMessage contextMessage;
				contextMessage.Type        = UIMessage::ContextRequest;
				contextMessage.Data        = requestData;
				contextMessage.MouseCoords = requestPoint - activeWidget->GetWorldLocation ();
				
				//-- see if this resulted in a popup context widget occuring
				if (activeWidget->ProcessMessage (contextMessage))
				{
					return true;
				}
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

void UIManager::ContextInfo::checkDragActivate (UIManager & manager, const UIPoint & pt)
{
	if (pending)
	{
		const UIPoint & Delta  = pt - requestPoint;
		const float mag = Delta.Magnitude ();
		
		if (mag > manager.GetDragThreshold ())
		{
			//-- if attempting to drag, cancel the context request
			if (manager.mMouseDownControl && manager.mAttemptToDrag )
				pending = false;
			//-- if not attempting to drag, force the context request to fire
			else
			{
				UIWidget * widget = manager.mMouseDownControl;
				
				if (widget)
				{
					widget = widget->FindFirstContextCapableWidget (requestData ? false : true);
					
					if (widget)
						countdown = 0L;
				}
			}
		}
	}
}

//----------------------------------------------------------------------

bool UIManager::ContextInfo::checkRequest (bool start, const UIMessage & msg)
{
	if (start)
	{
		if (msg.Type == message->Type)
		{
			requestPoint = msg.MouseCoords;
			pending      = true;
			countdown    = delay;
			return true;
		}
	}
	else
	{
		if (messageComplete && msg.Type == messageComplete->Type)
		{
			requestPoint = msg.MouseCoords;
			pending      = true;
			countdown    = 0;
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

void UIManager::ContextInfo::reset ()
{
	pending = false;
}

//-----------------------------------------------------------------

struct UIManager::StringTokenMapping
{
	typedef std::unordered_map<UIString, UINarrowString>  Container;
	Container c;
};

//-----------------------------------------------------------------

struct UIManager::CanvasShaderMap
{
	typedef std::map<UIString, UICanvas *, UIStringCompareNoCase>    CanvasMap;
	typedef std::map<UIString, CanvasMap,  UIStringCompareNoCase>    Container;

	Container c;
};


//-----------------------------------------------------------------

struct UIManager::ClockSet
{
	typedef std::set<UIClock *>                                  Container;
	Container c;
};

//-----------------------------------------------------------------
  
struct UIManager::CanvasFactoryVector
{
	typedef std::vector<const UICanvasFactory *>                 Container;
	Container c;
};

//-----------------------------------------------------------------

struct UIManager::LocalizedStringFactoryVector
{
	typedef std::vector<const UILocalizedStringFactory *>        Container;
	Container c;
};


//======================================================================================

UIManager::UIManager () : 
mSoundCanvas              (0),
mRootPage                 (0),
mScriptEngine             (0),
mTicksToTooltip           (40),
mCountdownToHoverPress    (0),
mTicksToHoverPress        (50),
mHoverPressComplete       (false),
mLastMouseCoord           (0,0),
mLastMouseDownCoord       (0,0),
mMouseDownControl         (0),
mDraggedControl           (0),
mDraggedSource            (0),
mLastWidgetUnderMouse     (0),
mDragThreshold            (5.0f),
mDragEchoOffset           (0,0),
mShowDragEcho             (true),
mDragGood                 (false),
mAttemptToDrag            (false),
mLastHeartbeatCall        (0),
mDrawCursor               (true),
mUpdateUnderMouse         (true),
mCanvasShaders            (new CanvasShaderMap),
mCanvasFactories          (new CanvasFactoryVector),
mClocks                   (new ClockSet),
mLocalizedStringFactories (new LocalizedStringFactoryVector),
mEffectors                (new EffectorMap),
mInDestructor             (false),
mClockInitalized          (false),
mPackingManager           (new UIPacking::Manager ()),
mStringTokenMapping       (new StringTokenMapping),
mHeartbeatNotificationServer (new UINotificationServer),
mContextInfo              (),
mContextInfoAlternate     (),
mDragStartMessage         (new UIMessage),
mDragEndMessage           (new UIMessage),
mDragModifier             (new UIMessageModifierData),
mDragCounter              (0L),
mContextPage              (0),
mCurrentTick              (0L),
mActionListener           (0),
mKeystrokePopsContext     (true),
mLocale(L_usa),
mIsScripting(false),
mRenderLastList(0)
{
	assert (mCanvasShaders);
	assert (mCanvasFactories);
	assert (mClocks);
	assert (mLocalizedStringFactories);
	assert (mEffectors);
	assert (mPackingManager);
	assert (mStringTokenMapping);
	assert (mHeartbeatNotificationServer);

	mContextInfo.message->Type             = UIMessage::RightMouseDown;

	mContextInfo.messageComplete           = new UIMessage;
	mContextInfo.messageComplete->Type     = UIMessage::RightMouseUp;

	mContextInfoAlternate.message->Type    = UIMessage::LeftMouseDown;
	mContextInfoAlternate.requestData      = 1;
	SetDragButton (0);

	UIPaletteRegistrySetup::install ();

	// Setup the effect token map.
	ms_effectTokenNameMap.insert(std::make_pair(EFTKN_Active, UIUnicode::narrowToWide(cms_effectTokenActive)));
	ms_effectTokenNameMap.insert(std::make_pair(EFTKN_Assignment, UIUnicode::narrowToWide(cms_effectTokenAssignment)));
	ms_effectTokenNameMap.insert(std::make_pair(EFTKN_Cancel, UIUnicode::narrowToWide(cms_effectTokenCancel)));
	ms_effectTokenNameMap.insert(std::make_pair(EFTKN_Effect, UIUnicode::narrowToWide(cms_effectTokenEffect)));
	ms_effectTokenNameMap.insert(std::make_pair(EFTKN_Page, UIUnicode::narrowToWide(cms_effectTokenPage)));
	ms_effectTokenNameMap.insert(std::make_pair(EFTKN_Reset, UIUnicode::narrowToWide(cms_effectTokenReset)));
	ms_effectTokenNameMap.insert(std::make_pair(EFTKN_Ui, UIUnicode::narrowToWide(cms_effectTokenUi)));
	ms_effectTokenNameMap.insert(std::make_pair(EFTKN_ShowMediator, UIUnicode::narrowToWide(cms_effectTokenShowMediator)));
	ms_effectTokenNameMap.insert(std::make_pair(EFTKN_HideMediator, UIUnicode::narrowToWide(cms_effectTokenHideMediator)));

	mRenderLastList = new std::vector<UIWidget *>;
}

//======================================================================================

UIManager::~UIManager ()
{ 
	UIBaseObject::garbageCollect();

	UIClock * const clock = UIClock::GetSingleton ();
	if (clock)
		clock->StopListening (this);

	mInDestructor = true;

	std::for_each (ms_popContextWidgetsNextFrame.begin (), ms_popContextWidgetsNextFrame.end (), UIBaseObject::DetachFunctor (0));
	ms_popContextWidgetsNextFrame.clear ();

	SetRootPage( 0 );

	SetLastWidgetUnderMouse (0);

	{
		for (CanvasShaderMap::Container::iterator it = mCanvasShaders->c.begin (); it != mCanvasShaders->c.end (); ++it)
		{
			CanvasShaderMap::CanvasMap & canvases = (*it).second;
			for( CanvasShaderMap::CanvasMap::iterator sit = canvases.begin(); sit != canvases.end(); ++sit )
			{
				UICanvas * const canvas = (*sit).second;
				if (NULL != canvas)
					canvas->Detach(0);
			}
		}
	}
	
	for( EffectorMap::iterator j = mEffectors->begin(); j != mEffectors->end(); ++j )
	{
		(*j).first->Detach(0);
		(*j).second->Detach(0);
	}

	mCanvasShaders->c.clear();

	delete mCanvasShaders;
	mCanvasShaders = 0;

	delete mCanvasFactories;
	mCanvasFactories = 0;

	delete mClocks;
	mClocks = 0;

	delete mLocalizedStringFactories;
	mLocalizedStringFactories = 0;

	delete mEffectors;
	mEffectors = 0;

	delete mPackingManager;
	mPackingManager = 0;

	delete mStringTokenMapping;
	mStringTokenMapping = 0;

	delete mHeartbeatNotificationServer;
	mHeartbeatNotificationServer = 0;

	delete mDragStartMessage;
	mDragStartMessage = 0;
	delete mDragEndMessage;
	mDragEndMessage = 0;

	delete mDragModifier;
	mDragModifier = 0;

	mRenderLastList->clear();
	delete mRenderLastList;
	mRenderLastList = 0;

	UIBaseObject::garbageCollect();
}

//======================================================================================

void UIManager::SendHeartbeats ()
{
	if( !mClockInitalized )
	{
		// Listen to the global clock so we can run effectors
		UIClock::gUIClock().Listen( this );
		mClockInitalized = true;
	}

	if( mRootPage )
	{
		const long CurrentTime = UISystemDependancies::Get().GetTickCount();
		mCurrentTick = CurrentTime;

		static const float factor = (60.0f / 1000.0f);
		const double fticks        = floor( static_cast<float>(CurrentTime - mLastHeartbeatCall) * factor);

		const long Ticks          = static_cast<long>(fticks);

		if (Ticks <= 0)
			return;

		for( ClockSet::Container::iterator i = mClocks->c.begin(); i != mClocks->c.end(); ++i )
			(*i)->Advance( Ticks );
		
		mLastHeartbeatCall += static_cast<long>(static_cast<float>(Ticks) * 1000.0f / 60.0f);
		
		if (mContextInfo.update (Ticks))
		{
			mContextInfoAlternate.reset ();	
			
			ResetTooltipCountdown ();
			ResetHoverPressCountdown ();
			
			mRootPage->ReleaseMouseLock (mContextInfo.requestPoint);
		}
		else if (mContextInfoAlternate.update (Ticks))
		{
			ResetTooltipCountdown ();
			ResetHoverPressCountdown ();
			mRootPage->ReleaseMouseLock (mContextInfo.requestPoint);
		}
		else
		{
			UIWidget * const control = mRootPage->GetWidgetFromPoint( mLastMouseCoord, true );
			
			if (control != mLastWidgetUnderMouse)
			{
				SetLastWidgetUnderMouse (control);
				ResetTooltipCountdown ();
				ResetHoverPressCountdown ();
			}
			else
			{
				if (mMouseDownControl)
					ResetTooltipCountdown ();
				else
					mCountdownToTooltip = std::max (0L, mCountdownToTooltip - Ticks);
				
				//-- handle hoverpress
				
				if (!mHoverPressComplete && mDraggedControl)
				{
					mCountdownToHoverPress = std::max (0L, mCountdownToHoverPress - Ticks);
					
					if (mCountdownToHoverPress == 0)
					{
						
						if (control && control != mDraggedControl)
						{
							if (control->IsA (TUIButton))
							{
								UIButton * const button = static_cast<UIButton *>(control);
								if (button->IsAutoPressByDrag ())
									button->Press ();

								mHoverPressComplete = true;
							}
						}
					}
				}
			}
		}
	}

	//-- yes, this is expensive, but only necessary when dragging.
	//-- it is necessary because the contents and hotspots of a widget can move around underneath, e.g. 3d views
	//-- @todo: optimize by keeping pointer to drag target around (assumes that widgets don't change size underneath)

	if (mDraggedControl)
		 GetDragTarget( mLastMouseCoord, mDragGood );

	mHeartbeatNotificationServer->SendNotification (UINotification::ObjectChanged, 0);
}

//======================================================================================

void UIManager::ResetHeartbeat ()
{
	mLastHeartbeatCall = UISystemDependancies::Get().GetTickCount();
}

//======================================================================================

void UIManager::ResetTooltipCountdown ()
{
	mCountdownToTooltip	= mTicksToTooltip;
}

//----------------------------------------------------------------------

void UIManager::ResetHoverPressCountdown ()
{
	mCountdownToHoverPress = mTicksToHoverPress;
	mHoverPressComplete = false;
}

//======================================================================================

void UIManager::SetSize( long width, long height )
{
	if( mRootPage )
	{	
		if( (mRootPage->GetWidth() != width) || (mRootPage->GetHeight() != height) )
		{
			mRootPage->Pack();
			mRootPage->SetSize( UISize( width, height ) );
		}
	}
}

//======================================================================================

void UIManager::Render( UICanvas &DestinationCanvas ) const
{
	if( !mRootPage )
		return;

	mRootPage->Render( DestinationCanvas );

	//Draw the render last list
	for(std::vector<UIWidget *>::iterator i = mRenderLastList->begin(); i != mRenderLastList->end(); ++i)
	{
		(*i)->Render( DestinationCanvas );
	}

#ifdef _DEBUG
	if (ms_drawWidgetBorders)
	{
		UIColor const saveColor = DestinationCanvas.GetColor();

		DestinationCanvas.SetColor(UIColor::red);
		drawWidgetBorders(DestinationCanvas, mRootPage);
		DestinationCanvas.SetColor(saveColor);
	}
#endif

	if( !mDrawCursor )
	{
		if ( mCursorInterface )
			mCursorInterface->render( 0, 0, DestinationCanvas, mLastMouseCoord );
		return;
	}
		
	// Draw the item being dragged
	if( mDraggedControl && mShowDragEcho )
	{
		DestinationCanvas.Flush();

		DestinationCanvas.PushState();
		DestinationCanvas.Translate( mLastMouseCoord + mDragEchoOffset );
		DestinationCanvas.Clip( UIRect( 0, 0, mDraggedControl->GetWidth(), mDraggedControl->GetHeight() ) );

		DestinationCanvas.SetOpacity( 0.5f );
		DestinationCanvas.Translate( -mDraggedControl->GetScrollLocation() );
		DestinationCanvas.SetColor( mDraggedControl->GetColor() );
		
		mDraggedControl->Render( DestinationCanvas );				
		DestinationCanvas.PopState();
	}
	
	UIWidget * const ObjectUnderCursor = (mMouseDownControl && !mDraggedControl) ? mMouseDownControl : mRootPage->GetWidgetFromPoint( mLastMouseCoord, true );
	
	//-----------------------------------------------------------------
	//-- draw the cursor
	UICursor *Cursor            = 0;
	UICursor *NormalCursor      = 0;
	
	{
		UIBaseObject *ObjectToQuery = ObjectUnderCursor;
		
		// Search for Cursor
		while( ObjectToQuery && !Cursor )
		{
			if( ObjectToQuery->IsA( TUIWidget ) )
			{
				if( mDraggedControl )
				{
					if( mDragGood )
						Cursor = static_cast<UIWidget *>( ObjectToQuery )->GetDragGoodCursor();
					else
						Cursor = static_cast<UIWidget *>( ObjectToQuery )->GetDragBadCursor();
				}
		
				if (!NormalCursor)
					NormalCursor = static_cast<UIWidget *>( ObjectToQuery )->GetMouseCursor();
			}
			
			if( !Cursor )
				ObjectToQuery = ObjectToQuery->GetParent();
		}
		
		if (!Cursor)
			Cursor = NormalCursor;

		const unsigned long theTime = UIClock::gUIClock ().GetTime();
		mCursorInterface->render    (theTime, Cursor, DestinationCanvas, mLastMouseCoord);
	}		
	
	if (ObjectUnderCursor && (mCountdownToTooltip == 0 || ObjectUnderCursor->GetTooltipDelay() == false))
	{
		const UIPoint & worldLocation = ObjectUnderCursor->GetWorldLocation ();
		UIPoint tooltipPt = mLastMouseCoord - worldLocation;
		UIWidget * const TooltipObjectUnderCursor = ObjectUnderCursor->GetWidgetFromPoint (tooltipPt, false);

		UIBaseObject   * ObjectToQuery = TooltipObjectUnderCursor;
		UITooltipStyle * TooltipStyle	= 0;
		static UIString Tooltip;
		Tooltip.clear ();

		// Search for tooltip contents
		while( ObjectToQuery )
		{
			if( ObjectToQuery->IsA( TUIWidget ) )
			{
				UIWidget *WidgetToQuery = static_cast<UIWidget *>( ObjectToQuery );

				Tooltip = WidgetToQuery->GetLocalTooltip(tooltipPt);
				if (Tooltip.empty () && mActionListener)
					mActionListener->constructTooltipForObject (*WidgetToQuery, Tooltip);

				TooltipStyle = WidgetToQuery->GetTooltipStyle();

				if( TooltipStyle )
					break;

				if( !Tooltip.empty() )
					break;

				tooltipPt += WidgetToQuery->GetLocation ();
			}

			ObjectToQuery = ObjectToQuery->GetParent();
		}

		// Do not reset ObjectToQuery to ObjectUnderCursor here

		if( !Tooltip.empty() || TooltipStyle )
		{
			// Search for tooltip style
			while( ObjectToQuery && !TooltipStyle )
			{
				if( ObjectToQuery->IsA( TUIWidget ) )
					TooltipStyle = static_cast<UIWidget *>( ObjectToQuery )->GetTooltipStyle();

				ObjectToQuery = ObjectToQuery->GetParent();
			}

			if( TooltipStyle )
			{
				UISize	TooltipSize;
				UIPoint TooltipLocation;

				DestinationCanvas.Flush();

				// Determine tooltip size
				if (!Tooltip.empty())
					TooltipSize = TooltipStyle->Measure (Tooltip);
				else
					TooltipSize = TooltipStyle->Measure ();

				TooltipStyle->SetCharacterWidth(s_defaultToolTipCharacterWidth);

				// Determine tooltip location
				TooltipLocation = mLastMouseCoord;

				UIPoint cursorOffset;
				if( Cursor )
				{
					cursorOffset -= Cursor->GetHotSpot ();
					cursorOffset += Cursor->GetSize ();
				}

				TooltipLocation += cursorOffset;

				const UISize & rootSize = mRootPage->GetSize ();

				if ((TooltipLocation.x + TooltipSize.x) > (rootSize.x - 16L))
					TooltipLocation.x -= TooltipSize.x + cursorOffset.x;

				TooltipLocation.x = std::max (0L, TooltipLocation.x);

				if ((TooltipLocation.y + TooltipSize.y) > (rootSize.y - 16L))
					TooltipLocation.y -= TooltipSize.y + cursorOffset.y;

				TooltipLocation.y = std::max (0L, TooltipLocation.y);

				DestinationCanvas.Translate( TooltipLocation );

				if( Tooltip.empty() )
					TooltipStyle->Render( DestinationCanvas, mTooltipAnimationState );
				else
					TooltipStyle->Render( DestinationCanvas, Tooltip, mTooltipAnimationState );

				DestinationCanvas.Translate( -TooltipLocation );
			}
		}
	}
}

//======================================================================================

bool UIManager::ProcessMessage (const UIMessage &Msg)
{
	if( !mRootPage )
		return false;

	if( Msg.Type == UIMessage::MouseMove )
	{
		if( mLastMouseCoord == Msg.MouseCoords && !ms_isRefreshing)
			return false;
		
		const UIPoint & delta = mLastMouseCoord - Msg.MouseCoords;

		//-- reset the tooltip timeout only if the tooltip isnt already displaying
		const float threshold = mDragThreshold * 2L;
		if ((mCountdownToTooltip != 0 || delta.Magnitude () > threshold) && !ms_isRefreshing)
			ResetTooltipCountdown ();
		
		//----------------------------------------------------------------------
		
		mContextInfo.checkDragActivate          (*this, Msg.MouseCoords);
		mContextInfoAlternate.checkDragActivate (*this, Msg.MouseCoords);
		
		//----------------------------------------------------------------------
		
		if (mContextPage)
		{
			const bool retval = mContextPage->ProcessMessage (Msg);
			mLastMouseCoord = Msg.MouseCoords;
			return retval;
		}

		//----------------------------------------------------------------------

		if( Msg.Modifiers.and (*mDragModifier) )
		{
			if( mDraggedControl )
			{
				if( mDraggedControl->GetRefCount() == 1 )
				{
					// We're the only person holding a reference to the dragged object
					// this indicates that the UI system has detached from it somehow
					// (maybe it's a template instance and the data changed), we should 
					// stop the drag.

					//-- jww - stop this behavior for now to support custom drag widgets

//					mDraggedControl->Detach(0);
//					mDraggedControl = 0;
				}
			}

			//----------------------------------------------------------------------

			if( mDraggedControl && mDraggedSource )
			{
				UIWidget *DragTarget = GetDragTarget( Msg.MouseCoords, mDragGood );
				
				if( DragTarget )
				{
					UIMessage NotificationMessage;

					NotificationMessage.Type       = UIMessage::DragOver;
					NotificationMessage.DragSource = mDraggedSource;
					NotificationMessage.DragObject = mDraggedControl;
					
					DragTarget->ProcessMessage( NotificationMessage );
				}
			}

			//----------------------------------------------------------------------

			else if( mMouseDownControl && mAttemptToDrag )
			{

				UIPoint Delta = Msg.MouseCoords - mLastMouseDownCoord;

				if( Delta.Magnitude() > mDragThreshold )
				{
					UIWidget * const DraggedControl = mMouseDownControl->FindFirstDragableWidget ();
					
					if( DraggedControl )
					{
						UIPoint dragWidgetOffset;

						UIWidget * const customDragWidget = DraggedControl->GetCustomDragWidget (Msg.MouseCoords - DraggedControl->GetWorldLocation (), dragWidgetOffset);
						
						if (customDragWidget)
						{							
							UIWidget * const parentWidget = DraggedControl->GetParentWidget();
							
							if( parentWidget )
							{

								UIMessage NotificationMessage;
								
								NotificationMessage.MouseCoords = Msg.MouseCoords - parentWidget->GetWorldLocation ();
								NotificationMessage.Type        = UIMessage::DragStart;
								NotificationMessage.DragSource  = DraggedControl;
								NotificationMessage.DragObject  = customDragWidget;
								
								++mDragCounter;

								//-- notify the widget being dragged, then notify it's parent

								DraggedControl->ProcessMessage( NotificationMessage );
								parentWidget->ProcessMessage( NotificationMessage );

								mRootPage->ReleaseMouseLock (Msg.MouseCoords);
							}
							
							SetDragEcho( DraggedControl, customDragWidget, dragWidgetOffset );
							//ShowDragEcho (true);
						}

						//-- nobody should be messing with mMouseDownControl before we get here...
						assert (mMouseDownControl);

						mMouseDownControl->Detach (0);
						mMouseDownControl = 0;

					}
					else
					{
						mAttemptToDrag = false;
					}
				}
			}
		}

		//-- set the last mouse coords last
		mLastMouseCoord = Msg.MouseCoords;
		
	}
	
	//-----------------------------------------------------------------
	//--
	//-----------------------------------------------------------------

	else if (Msg.IsMouseMessage ())
	{
		mContextInfo.reset ();
		mContextInfoAlternate.reset ();

		bool processed = false;
		
		//----------------------------------------------------------------------
		
		bool poppedContext = false;

		if (mContextPage)
		{
			if (!mContextPage->ProcessMessage (Msg))
			{
				if (mContextPage)
				{
					poppedContext  = true;
					PopContextWidgets (0);
					mContextInfo.reset ();
					mContextInfoAlternate.reset ();
				}
			}

			if (Msg.Type != UIMessage::LeftMouseUp && 
				Msg.Type != UIMessage::MiddleMouseUp && 
				Msg.Type != UIMessage::RightMouseUp)
				processed = true;
		}
		
		//-----------------------------------------------------------------

		if (!poppedContext)
		{
			if (!mContextInfo.checkRequest (!mMouseDownControl, Msg))
				mContextInfoAlternate.checkRequest (!mMouseDownControl, Msg);
		}

		//----------------------------------------------------------------------		
		
		if (Msg.Type == UIMessage::LeftMouseDown || Msg.Type == UIMessage::MiddleMouseDown || Msg.Type == UIMessage::RightMouseDown)
		{
			UIWidget * const ControlUnderMouse = mRootPage->GetWidgetFromPoint( Msg.MouseCoords, true );
			
			if( ControlUnderMouse )
				ControlUnderMouse->Attach(0);
			
			if( mMouseDownControl )
				mMouseDownControl->Detach(0);			

			mMouseDownControl   = ControlUnderMouse;
			mLastMouseDownCoord = Msg.MouseCoords;
		}		

		//----------------------------------------------------------------------		
		
		else if (mMouseDownControl && (Msg.Type == UIMessage::LeftMouseUp || Msg.Type == UIMessage::MiddleMouseUp || Msg.Type == UIMessage::RightMouseUp))
		{
			mMouseDownControl->Detach (0);
			mMouseDownControl = 0;
			
			//-- send the message on down the tree to make the rootpage loose it's mouselocked widget
//			mRootPage->ProcessMessage( Msg );
			
			//-- now force an enter/exit series to get the hovering states right
//			UIMessage updateMsg = Msg;
//			updateMsg.Type = UIMessage::MouseMove;
//			mRootPage->ProcessMessage (updateMsg);
		}

		//-----------------------------------------------------------------

		if( Msg.Type == mDragEndMessage->Type)
		{
			
			if( mDraggedControl && mDraggedSource )
			{
				UIWidget *DragTarget = GetDragTarget( Msg.MouseCoords, mDragGood );
				
				// Notify parent of source that drop is done
				UIMessage			NotificationMessage;			
				NotificationMessage.Type       = mDragGood ? UIMessage::DragEnd : UIMessage::DragCancel;
				NotificationMessage.DragObject = mDraggedControl;
				NotificationMessage.DragSource = mDraggedSource;
				NotificationMessage.DragTarget = DragTarget;
				
				mDraggedSource->ProcessMessage( NotificationMessage );

				UIWidget * const parentWidget = mDraggedSource->GetParentWidget();

				if (parentWidget)
					parentWidget->ProcessMessage( NotificationMessage );
				
				// Notify destination that drop is done
				if( DragTarget && DragTarget != mDraggedSource )
					DragTarget->ProcessMessage( NotificationMessage );
				
				SetDragEcho( 0, 0, UIPoint::zero );

				processed = true;

			}
		}
		
		//----------------------------------------------------------------------
		
		else if( Msg.Type == mDragStartMessage->Type )
		{
			if (mMouseDownControl)
				mAttemptToDrag = mMouseDownControl->IsDragable ();
			else
				mAttemptToDrag = false;
		}

		//----------------------------------------------------------------------
		
		if ((!mMouseDownControl || !mMouseDownControl->FindFirstContextCapableWidget (true)) &&
			(!mContextInfo.pending || mContextInfo.countdown > 0))
			mContextInfo.reset ();

		if ((!mMouseDownControl || !mMouseDownControl->FindFirstContextCapableWidget (false)) &&
			(!mContextInfoAlternate.pending || mContextInfoAlternate.countdown > 0))
			mContextInfoAlternate.reset ();

		//----------------------------------------------------------------------

		if (processed)
			return true;
	}

	//----------------------------------------------------------------------
	//-- 
	//-----------------------------------------------------------------
	
	else if (Msg.Type == UIMessage::KeyDown || Msg.Type == UIMessage::KeyRepeat)
	{
		mContextInfo.reset ();
		mContextInfoAlternate.reset ();
		
		//----------------------------------------------------------------------

		if (mContextPage)
		{
			if (Msg.Keystroke == UIMessage::Escape)
			{
				PopContextWidgets (0);
				return true;
			}

			mKeystrokePopsContext = true;

			if (!mRootPage->ProcessMessage (Msg))
			{
				if (mKeystrokePopsContext)
				{
					PopContextWidgets (0);
					return true;
				}
				return false;
			}
			
			return true;
		}

		//-----------------------------------------------------------------
		
		if (Msg.Keystroke == UIMessage::Escape)
		{
			bool stop = false;
			
			if (mDraggedControl || mDraggedSource)
			{
				UIWidget * const DragTarget = GetDragTarget( Msg.MouseCoords, mDragGood );
				AbortDragAndDrop (DragTarget);
				stop = true;
			}
			
			if (mMouseDownControl)
			{
				//-- send the exit message down the tree to make the rootpage loose it's mouselocked widget
				
				UIMessage exitMsg;
				exitMsg.MouseCoords = mLastMouseCoord;
				exitMsg.Type = UIMessage::MouseExit;
				mRootPage->ProcessMessage (exitMsg);
				
				mMouseDownControl->Detach (0);
				mMouseDownControl = 0;

// TMS - 08/01/06
// If a widget has the mouse captured we still want the ESC key to 
// go to the rest of the game.  This lets us drop target while firing
// with the left mouse button (which puts the hud on mouse drag)
//				stop = true; 
			}
			
			if (stop)
			{				
				//-- now force an enter/exit series to get the hovering states right
				UIMessage updateMsg;
				updateMsg.MouseCoords = mLastMouseCoord;
				updateMsg.Type = UIMessage::MouseMove;
				mRootPage->ProcessMessage (updateMsg);
				return true;
			}
		}

		return mRootPage->ProcessMessage (Msg);
	}
	//----------------------------------------------------------------------
	
	else if (Msg.Type == UIMessage::Character)
	{
		if (mContextPage)
		{
			if (mRootPage->ProcessMessage(Msg)) 
			{
				PopContextWidgets(0);
			}

			return true;
		}
	}

	if (Msg.Type > UIMessage::IMEFirst && Msg.Type < UIMessage::IMELast)
	{
		if (Msg.Type == UIMessage::IMEChangeCandidate)
		{
			UIPage *debugPage = (UIPage *) mRootPage->GetChild("Debug");

			if (debugPage)
			{
				UIPage *page = (UIPage *) debugPage->GetChild("IME");

				if (page)
				{
					UIText *text = (UIText *) page->GetChild("CandidateInfo");

					if (text)
					{
						char buf[32];

						_snprintf(buf, sizeof(buf), "%d/%d", UIManager::getUIIMEManager()->GetSelectedCandidate() + 1, UIManager::getUIIMEManager()->GetNumCandidates());

						text->SetLocalText(Unicode::narrowToWide(buf));
					}

					UIList *listbox = (UIList *) page->GetChild("CandidateListbox");

					listbox->Clear();

					std::vector<Unicode::String> &candidates = UIManager::getUIIMEManager()->GetCandidateList();
					for (unsigned int i = 0; i < candidates.size(); i++)
					{
						unsigned short num[2];

						num[0] = (unsigned short) (L'0' + ((i + 1) % 10));
						num[1] = 0;
			
						UIString str = candidates[i];
						
						unsigned short buf[512];
						_snwprintf(buf, sizeof(buf) - 1, L"%s\\>032%s", num, str.c_str());
						
						listbox->AddRow(Unicode::String(buf), "Candidate");

					}

					if (listbox)
					{
						listbox->SelectRow(UIManager::getUIIMEManager()->GetSelectedCandidateIndex());
					}
				}
			}
		}

		return mRootPage->ProcessMessage( Msg );
	}
	
	//----------------------------------------------------------------------
	
	if( !mDraggedControl || !mDragGood || Msg.Type == UIMessage::MouseMove)
	{
		// While dragging we don't forward messages
		return mRootPage->ProcessMessage( Msg );
	}
	
	return false;
}

//======================================================================================

void UIManager::DrawCursor( bool NewDrawFlag )
{
	mDrawCursor = NewDrawFlag;

	if (NewDrawFlag)
		mUpdateUnderMouse = true;
	else if (mMouseDownControl)
	{
		mMouseDownControl->Detach (0);
		mMouseDownControl = 0;
	}
}

//======================================================================================

void UIManager::SetRootPage( UIPage *NewRootPage )
{
	// mMouseDownControl is not relevant to the new root page
	if( mMouseDownControl )
	{
		mMouseDownControl->Detach(0);
		mMouseDownControl = 0;
	}

	// mDraggedControl is not relevant to the new root page
	if( mDraggedControl )
	{
		mDraggedControl->Detach(0);
		mDraggedControl = 0;
	}

	// mDraggedControl is not relevant to the new root page
	if( mDraggedSource )
	{
		mDraggedSource->Detach(0);
		mDraggedSource = 0;
	}

	if( NewRootPage )
		NewRootPage->Attach(0);

	if( mRootPage )
		mRootPage->Detach(0);

	mRootPage = NewRootPage;

	ResetHeartbeat();
	ResetTooltipCountdown();	
	ResetHoverPressCountdown ();

	if (mRootPage)
	{
		UIPalette * const palette = UIPalette::GetInstance ();
		if (palette)
			palette->Reset ();
	}
}

//======================================================================================

int  UIManager::GetCanvasCount () const
{
	int count = 0;
	for (CanvasShaderMap::Container::const_iterator it = mCanvasShaders->c.begin (); it != mCanvasShaders->c.end (); ++it)
	{
		const CanvasShaderMap::CanvasMap & canvases = (*it).second;
		count += canvases.size ();
	}
	return count;
}

//-----------------------------------------------------------------

void UIManager::GetCanvases (std::vector<UICanvas *> & canvasVector) const
{
	canvasVector.clear   ();

	for (CanvasShaderMap::Container::const_iterator it = mCanvasShaders->c.begin (); it != mCanvasShaders->c.end (); ++it)
	{
		const CanvasShaderMap::CanvasMap & canvases = (*it).second;

		canvasVector.reserve (canvasVector.size () + canvases.size ());

		for( CanvasShaderMap::CanvasMap::const_iterator sit = canvases.begin(); sit != canvases.end(); ++sit )
		{
			canvasVector.push_back ((*sit).second);
		}
	}
}

//----------------------------------------------------------------------

UICanvas * UIManager::GetCanvas (const UIString & textureName)
{
	static const Unicode::String empty;

	const size_t colon_pos = textureName.find (':');
	if (colon_pos != textureName.npos)
		return GetCanvas (textureName.substr (0, colon_pos), textureName.substr (colon_pos + 1));
	else
		return GetCanvas (empty, textureName);
}

//-----------------------------------------------------------------

UICanvas * UIManager::GetCanvas (const UIString & shaderName, const UIString & textureName)
{
	const CanvasShaderMap::Container::iterator shader_it = mCanvasShaders->c.find (shaderName);

	if (shader_it != mCanvasShaders->c.end())
	{
		const UIString & name = shader_it->first;
		UI_UNREF (name);
		const CanvasShaderMap::CanvasMap & canvases = shader_it->second;
		const CanvasShaderMap::CanvasMap::const_iterator canvas_it = canvases.find (textureName);
		if (canvas_it != canvases.end ())
			return (*canvas_it).second;
	}

	const UINarrowString textureNameCopy (Unicode::wideToNarrow (textureName));
	const UINarrowString shaderNameCopy  (Unicode::wideToNarrow (shaderName));

	for( CanvasFactoryVector::Container::iterator it = mCanvasFactories->c.begin(); it != mCanvasFactories->c.end(); ++it )
	{
		UICanvas * const theNewCanvas = (*it)->CreateCanvas( shaderNameCopy, textureNameCopy );

		//-- add canvas even if it is null, to prevent future CreateCanvas calls
		AddCanvas( shaderName, textureName, theNewCanvas );
		return theNewCanvas;
	}

	return 0;
}

//======================================================================================

void UIManager::AddCanvas( const Unicode::String & shaderName, const UIString & Name, UICanvas *Value )
{
	if( Value )
		Value->Attach(0);

	CanvasShaderMap::Container::iterator shader_it = mCanvasShaders->c.find (shaderName);

	if (shader_it == mCanvasShaders->c.end())
	{
		const std::pair<CanvasShaderMap::Container::const_iterator, bool> retval = mCanvasShaders->c.insert (std::make_pair (shaderName, CanvasShaderMap::CanvasMap ()));
		if (!retval.second)
		{
			assert (false);
			return;
		}
		
		shader_it = mCanvasShaders->c.find (shaderName);
	}
	
	assert (shader_it != mCanvasShaders->c.end());
	
	CanvasShaderMap::CanvasMap & canvases = (*shader_it).second;
	
	const CanvasShaderMap::CanvasMap::iterator canvas_it = canvases.find (Name);
	if (canvas_it != canvases.end ())
	{		
		(*canvas_it).second->Detach(0);
		canvases.erase( canvas_it );
	}
	
	canvases.insert (std::make_pair (Name, Value));
	
	if (NULL != Value)
	{
		if (!shaderName.empty ())
			Value->SetName( Unicode::wideToNarrow (shaderName + Unicode::narrowToWide (":") + Name) );
		else
			Value->SetName( Unicode::wideToNarrow (Name) );
	}
}

//======================================================================================

void UIManager::AddCanvasFactory( const UICanvasFactory *theNewFactory )
{
	mCanvasFactories->c.push_back( theNewFactory );
}

//======================================================================================

void UIManager::RefreshGraphics ()
{
	for (CanvasShaderMap::Container::iterator it = mCanvasShaders->c.begin (); it != mCanvasShaders->c.end (); ++it)
	{
		CanvasShaderMap::CanvasMap & canvases = (*it).second;
		for( CanvasShaderMap::CanvasMap::iterator sit = canvases.begin(); sit != canvases.end(); /*inline*/)
		{
			UICanvas * const canvas = (*sit).second;
			if (canvas)
			{
				canvas->Refresh();
				++sit;
			}
			else
			{
				canvases.erase(sit++);
			}
		}
	}

	// Force an update.
	GetRootPage()->Link();
}

//======================================================================================

void UIManager::SetDragEcho( UIWidget * source, UIWidget *NewEcho, const UIPoint & offset )
{
	assert ((source && NewEcho) || (!source && !NewEcho));

	if( source )
	{
		source->Attach (0);
	}

	if (NewEcho)
	{
		NewEcho->Attach(0);
		mDragEchoOffset = offset;

		mDragEchoOffset.x = std::min (mDragEchoOffset.x, 0L);
		mDragEchoOffset.x = std::max (mDragEchoOffset.x, -NewEcho->GetWidth ());
		mDragEchoOffset.y = std::min (mDragEchoOffset.y, 0L);
		mDragEchoOffset.y = std::max (mDragEchoOffset.y, -NewEcho->GetHeight ());
	}

	if( mDraggedControl )
		mDraggedControl->Detach(0);

	if (mDraggedSource)
		mDraggedSource->Detach (0);

	mDraggedSource  = source;
	mDraggedControl = NewEcho;
	mDragGood       = false;
}

//======================================================================================

void UIManager::ShowDragEcho( bool NewShow )
{
	mShowDragEcho = NewShow;
}

//======================================================================================

UIWidget *UIManager::GetDragTarget( const UIPoint &Ref, bool & dragOk )
{
	assert (mDraggedControl);
	dragOk = false;

	UIString  DragType;
	mDraggedControl->GetProperty( UIWidget::PropertyName::DragType, DragType );

	if (DragType.empty ())
		return 0;

	UIWidget *DragTarget   = mRootPage->GetWidgetFromPoint( Ref, true );
	
	while (DragTarget)
	{
		if (DragTarget->AcceptsDragType (DragType))
			break;

		if (DragTarget->IsDropToParent ())
			DragTarget = DragTarget->GetParentWidget ();
		else
			DragTarget = 0;
	}

	if (DragTarget)
	{
		const UIPoint transformedPoint (Ref - DragTarget->GetWorldLocation ());
		dragOk = DragTarget->IsDropOk (*mDraggedControl, DragType, transformedPoint);
	}

	return DragTarget;
}

//======================================================================================

void UIManager::AddClock( UIClock *NewClock )
{
	mClocks->c.insert( NewClock );
}

//======================================================================================

void UIManager::RemoveClock( UIClock *OldClock )
{
	mClocks->c.erase( OldClock );
}

//======================================================================================

void UIManager::SetSoundCanvas( UISoundCanvas *NewSoundCanvas )
{
	mSoundCanvas = NewSoundCanvas;
}

//======================================================================================

void UIManager::PlaySound( const char *FileName ) const
{
	if( mSoundCanvas )
		mSoundCanvas->Play( FileName );
}

//----------------------------------------------------------------------

void UIManager::PlaySoundGenericNegative () const
{
	if (mSoundCanvas)
		mSoundCanvas->PlaySoundGenericNegative ();
}

//======================================================================================

void UIManager::SetScriptEngine( UIScriptEngine *NewScriptEngine )
{
	mScriptEngine = NewScriptEngine;
}

//======================================================================================

void UIManager::ExecuteScript( const UIString &Script, UIBaseObject *Context )
{
	if( mScriptEngine )
	{
		UINarrowString ErrorMessage;

		mIsScripting = true;
		mScriptEngine->Execute( Script, Context, ErrorMessage );
		mIsScripting = false;
	}
}

//======================================================================================

void UIManager::ExecuteEffector( UIEffector *Effector, UIBaseObject *Context, bool const allowDuplicates)
{
	assert (!ms_effectorListLock);

	assert( Effector );
	assert( Context );

	assert( Effector->GetRefCount() > 0 );
	assert( Context->GetRefCount() > 0 );

	if (!allowDuplicates && HasEffector(Effector, Context))
		return;

	if(Effector->OnCreate(Context) == UIEffector::Continue)
	{
		Effector->Attach(0);
		Context->Attach(0);
		
		mEffectors->insert( EffectorMap::value_type( Effector, Context ) );
	}
}

//======================================================================================

bool UIManager::HasEffector( UIEffector *Effector, UIBaseObject *Context )
{
	assert(Effector);
	std::pair<EffectorMap::iterator, EffectorMap::iterator> range = mEffectors->equal_range(Effector);

	if (Context == NULL && range.first != range.second)
		return true;

	for (; range.first != range.second; ++range.first)
	{
		UIBaseObject * const runningObject = (*range.first).second;
		if (Context == runningObject)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------

void UIManager::CancelEffector( UIEffector *Effector, UIBaseObject *Context )
{
	if( mInDestructor )
		return;

	assert (!ms_effectorListLock);
	
	std::pair<EffectorMap::iterator, EffectorMap::iterator> range = mEffectors->equal_range(Effector);
	if (range.first == range.second)
		return;

	ms_effectorListLock = true;

	for (; range.first != range.second; )
	{
		UIEffector   * const runningEffector = (*(range.first)).first;
		UIBaseObject * const runningObject   = (*(range.first)).second;

		if(!Context || runningObject == Context)
		{
			runningEffector->OnDestroy(Context);
			runningEffector->Detach(0);
			runningObject->Detach(0);
			mEffectors->erase( (range.first)++ );
		}
		else
			++(range.first);
	}

	ms_effectorListLock = false;
}

//======================================================================================

void UIManager::CancelEffectorsFor(UIBaseObject *Context, bool const childObjects, UITypeID const type)
{
	if( mInDestructor )
		return;

	if (ms_effectorListLock)
		return;

	ms_effectorListLock = true;

	for( EffectorMap::iterator i = mEffectors->begin(); i != mEffectors->end(); )
	{
		UIEffector   * const runningEffector = (*i).first;
		UIBaseObject * const runningObject   = (*i).second;

		if( runningObject == Context && runningEffector->IsA(type) )
		{
			runningEffector->OnDestroy(Context);
			runningEffector->Detach(0);
			runningObject->Detach(0);
			mEffectors->erase( i++ );
		}
		else
			++i;
	}

	ms_effectorListLock = false;

	if(childObjects)
	{
		UIBaseObject::UIObjectList children;
		
		Context->GetChildren(children);
		
		for(UIBaseObject::UIObjectList::iterator itChild = children.begin(); itChild != children.end(); ++itChild)
		{
			CancelEffectorsFor(*itChild, true);
		}
	}
}

//======================================================================================

void UIManager::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, UINotification::Code NotificationCode )
{	
	UI_UNREF (NotificationCode);
	UI_UNREF (ContextObject);
	UI_UNREF (NotifyingObject);

	ms_effectorListLock = true;

	for( EffectorMap::iterator i = mEffectors->begin(); i != mEffectors->end(); )
	{
		UIEffector   * const runningEffector = (*i).first;
		UIBaseObject * const runningObject   = (*i).second;

		assert(runningEffector);
		assert(runningObject);

		if (runningEffector->GetRefCount() <= 1 || runningObject->GetRefCount() <= 1)
		{
			runningEffector->Detach(0);
			runningObject->Detach(0);
			mEffectors->erase( i++ );
		}
		else if (runningEffector->Effect(runningObject) != UIEffector::Continue)
		{
			runningEffector->Detach(0);
			runningObject->Detach(0);
			mEffectors->erase( i++ );
		}
		else
			++i;
	}

	ms_effectorListLock = false;

	static long LastUpdateTime = mCurrentTick;
	
	if (mUpdateUnderMouse)
	{
		if (LastUpdateTime != mCurrentTick && !mMouseDownControl)
		{
			mRootPage->UpdateUnderMouse (mLastMouseCoord);
			LastUpdateTime = mCurrentTick ;
		}

		if (!mDrawCursor)
			mUpdateUnderMouse = false;
	}

	for (WidgetVector::iterator it = ms_popContextWidgetsNextFrame.begin (); it != ms_popContextWidgetsNextFrame.end (); ++it)
	{
		UIWidget * const wid = *it;
		if (!wid)
			PopContextWidgets (0);
		else
		{
			if (wid->GetRefCount () > 1)
				PopContextWidgets (wid);
			wid->Detach (0);
		}
	}

	ms_popContextWidgetsNextFrame.clear ();
}

//-----------------------------------------------------------------

void  UIManager::AddLocalizedStringFactory (const UILocalizedStringFactory * factory)
{
	mLocalizedStringFactories->c.push_back (factory);
}

//-----------------------------------------------------------------

bool UIManager::GetLocalizedString (const UINarrowString & name, UIString &dest) const
{
	for (LocalizedStringFactoryVector::Container::const_iterator iter = mLocalizedStringFactories->c.begin (); iter != mLocalizedStringFactories->c.end (); ++iter)
	{
		if ((*iter)->GetLocalizedString (name, dest))
			return true;
	}

	return false;
}

//-----------------------------------------------------------------

bool UIManager::CreateLocalizedString (const UIString & source, UIString & dest) const
{
	// localize text that starts with @
	
	if (source.empty () == false && source[0] == '@')
	{
		const UIString & tokensource = source;//source.substr (1);

		StringTokenMapping::Container::iterator iter = mStringTokenMapping->c.find (tokensource);

		if (iter == mStringTokenMapping->c.end ())
		{
			const std::pair <StringTokenMapping::Container::iterator, bool> retval = mStringTokenMapping->c.insert (std::make_pair (tokensource, Unicode::wideToNarrow (tokensource)));
			iter = retval.first;
		}

		if (GetLocalizedString ((*iter).second, dest))
			return true;
		
		// localized string not found -- indicate with special syntax
		dest.erase ();
		dest.append (1, '[');
		dest.append (source);
		dest.append (1, ']');
	}
	else
	{
		dest = source;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void UIManager::fitWidgetOnScreen (UIWidget & context, ContextWidgetAlignment alignment, bool fit)
{
	const UIPoint location (context.GetLocation ());
	const UISize size      (context.GetSize ());
	
	UIPoint offset;
	
	switch (alignment)
	{
		
	case CWA_NW:
		break;
	case CWA_N:
		offset.x = -context.GetWidth () / 2;
		break;
	case CWA_NE:
		offset.x = -context.GetWidth ();
		break;
	case CWA_E:
		offset.y = -context.GetHeight () / 2;
		offset.x = -context.GetWidth ();
		break;
	case CWA_SE:
		offset.y = -context.GetHeight ();
		offset.x = -context.GetWidth ();
		break;
	case CWA_S:
		offset.y = -context.GetHeight ();
		offset.x = -context.GetWidth () / 2;
		break;
	case CWA_SW:
		offset.y = -context.GetHeight ();
		break;
	case CWA_W:
		offset.y = -context.GetHeight () / 2;
		break;
	case CWA_Center:
		offset.y = -context.GetHeight () / 2;
		offset.x = -context.GetWidth () / 2;
		break;
	}
	
	context.SetLocation (location + offset);
	
	if (fit)
	{
		UIWidget * const parent = context.GetParentWidget ();
		
		if (parent)
		{
			UIRect rect (context.GetRect ());
			
			if (rect.bottom >= parent->GetHeight ())
				rect.top = rect.top - (rect.bottom - parent->GetHeight ());
			
			if (rect.right >= parent->GetWidth ())
				rect.left = rect.left - (rect.right - parent->GetWidth ());
			
			rect.top  = std::max (0L, rect.top);
			rect.left = std::max (0L, rect.left);
			
			context.SetLocation (rect.Location ());
		}
	}
}

//----------------------------------------------------------------------

void  UIManager::PushContextWidget           (UIWidget & context, ContextWidgetAlignment alignment, bool fit)
{
	context.Attach (0);

	PopContextWidgets (&context);

	if (!mContextPage)
	{
		mContextPage = new UIPage;
		mContextPage->SetAbsorbsInput (false);
		mContextPage->Attach (0);
		mContextPage->SetName ("InvisibleContextPage");
	}
	
	assert (mContextPage);
	
	if (mContextPage->GetParent () == 0)
	{
		mContextPage->SetSize (mRootPage->GetSize ());
		mContextPage->SetTransient (true);
		
		mRootPage->ReleaseMouseLock (UIPoint ());
		mRootPage->AddChild (mContextPage);
		mRootPage->MoveChild (mContextPage, UIBaseObject::Top);		
	}
	
	if (mContextPage->AddChild(&context))
	{
		mContextPage->MoveChild(&context, UIBaseObject::Top);
	}
	
	if (mMouseDownControl)
	{
		mMouseDownControl->Detach (0);
		mMouseDownControl = 0;
	}

	context.SetVisible(true);
	context.SetEnabled(true);
	context.Link ();
	context.SetFocus();
	
	fitWidgetOnScreen(context, alignment, fit);
}

//----------------------------------------------------------------------

void UIManager::PopContextWidgets           (UIWidget * context)
{
	static bool doubleEntryIntoPopContextWidgets = false;

	if (doubleEntryIntoPopContextWidgets)
		return;

	doubleEntryIntoPopContextWidgets = true;

	if (!mContextPage)
	{
		doubleEntryIntoPopContextWidgets = false;
		return;
	}

	UIBaseObject::UIObjectList olist;
	mContextPage->GetChildren (olist);
	
	const UIBaseObject::UIObjectList & olistRef = mContextPage->GetChildrenRef ();

	bool found = context == 0;

	for (UIBaseObject::UIObjectList::reverse_iterator it = olist.rbegin (); it != olist.rend (); ++it)
	{
		UIBaseObject * const obj = *it;

		if (!obj->IsA (TUIWidget))
			continue;

		UIWidget * const wid = static_cast<UIWidget *>(obj);

		if (obj == context)
			found = true;

		if (found)
		{
			wid->SetVisible (false);
			wid->SetEnabled (false);
			mContextPage->RemoveChild (wid);
			wid->Detach (0);
		}
	}

	if (mContextPage->GetParent () && olistRef.empty ())
	{
		mContextPage->GetParent ()->RemoveChild (mContextPage);
		mContextPage->Detach (0);
		mContextPage = 0;
	}

	doubleEntryIntoPopContextWidgets = false;
}

//----------------------------------------------------------------------

void UIManager::PopContextWidgetsNextFrame  (UIWidget * context)
{
	if (context)
		context->Attach (0);
	ms_popContextWidgetsNextFrame.push_back (context);
}

//----------------------------------------------------------------------

UIPopupMenu * UIManager::FindAssociatedPopupMenu(UIPage * const page) const
{
	if ((page != 0) && (mContextPage != 0))
	{
		UIBaseObject::UIObjectList objectlist;
		mContextPage->GetChildren(objectlist);
	
		UIBaseObject::UIObjectList::reverse_iterator ii = objectlist.rbegin();
		UIBaseObject::UIObjectList::reverse_iterator iiEnd = objectlist.rend();

		for (; ii != iiEnd; ++ii)
		{
			UIBaseObject * const object = *ii;

			if (object->IsA(TUIPopupMenu))
			{
				UIPopupMenu * const popupMenu = static_cast<UIPopupMenu *>(object);

				if ((popupMenu != 0) && (popupMenu->GetOwningPage() == page))
				{
					return popupMenu;
				}
			}
		}
	}
	return 0;
}

//-----------------------------------------------------------------

void UIManager::SetContextMessage (const UIMessage & msg, long holdTime, bool primary)
{
	ContextInfo * info = 0;

	if (primary)
		info = &mContextInfo;
	else
	{
		info = &mContextInfoAlternate;

		*info->message       = msg;
		info->delay          = holdTime;
	}
}

//----------------------------------------------------------------------

bool UIManager::IsContextMessage            (const UIMessage & msg, bool primary)
{
	if (primary)
		return (msg.Type == mContextInfo.message->Type);
	else
		return (msg.Type == mContextInfoAlternate.message->Type);
}

//----------------------------------------------------------------------

void UIManager::SetDragButton (long buttonNumber)
{
	memset (&mDragStartMessage->Modifiers, 0, sizeof (UIMessage::ModifierData));
	memset (mDragModifier,               0, sizeof (UIMessage::ModifierData));

	switch (buttonNumber)
	{
	case 0:
		mDragStartMessage->Type = UIMessage::LeftMouseDown;
		mDragEndMessage->Type   = UIMessage::LeftMouseUp;
		mDragModifier->LeftMouseDown = true;
		break;
	case 1:
		mDragStartMessage->Type = UIMessage::RightMouseDown;
		mDragEndMessage->Type   = UIMessage::RightMouseUp;
		mDragModifier->RightMouseDown = true;
		break;
	case 2:
		mDragStartMessage->Type = UIMessage::MiddleMouseDown;
		mDragEndMessage->Type   = UIMessage::MiddleMouseUp;		
		mDragModifier->MiddleMouseDown = true;
		break;
	default:
		mDragStartMessage->Type = UIMessage::NumMessages;
		mDragEndMessage->Type   = UIMessage::NumMessages;
		break;
	}
}

//----------------------------------------------------------------------

void UIManager::RefreshMousePosition ()
{
	UIMessage msg;
	msg.Type	      = UIMessage::MouseMove;
	msg.MouseCoords   = mLastMouseCoord;
	ms_isRefreshing = true;
	ProcessMessage (msg);
	ms_isRefreshing = false;
}

//----------------------------------------------------------------------

void UIManager::SetSize( const UIPoint &pt )
{
	SetSize( pt.x, pt.y );
}

//----------------------------------------------------------------------

const UIPoint & UIManager::getDragEchoOffset () const
{
	return mDragEchoOffset;
}

//----------------------------------------------------------------------

UIBaseObject     *UIManager::GetObjectFromPath         (const char * str ) const
{
	if (!mRootPage)
		return 0;

	return mRootPage->GetObjectFromPath (str);
}

//----------------------------------------------------------------------

UIBaseObject     *UIManager::GetObjectFromPath         (const char * str, UITypeID type) const
{
	if (!mRootPage)
		return 0;

	return mRootPage->GetObjectFromPath (str, type);
}

//----------------------------------------------------------------------

UIWidget * UIManager::GetFocusedLeafWidget ()
{
	if (!mRootPage)
		return 0;

	return mRootPage->GetFocusedLeafWidget ();
}

//----------------------------------------------------------------------

void UIManager::SetActionListener (UIActionListener * listener)
{
	mActionListener = listener;
}

//----------------------------------------------------------------------

bool UIManager::NotifyActionListener (const UIBaseObject & obj)
{
	if (mActionListener)
		return mActionListener->performActionForObject (obj);

	return false;
}

//----------------------------------------------------------------------

void UIManager::SetCursorInterface          (UICursorInterface * cursorInterface)
{
	mCursorInterface = cursorInterface;
}

//----------------------------------------------------------------------

void UIManager::SetLastWidgetUnderMouse (UIWidget * const widget)
{
	if (widget)
		widget->Attach (0);
	if (mLastWidgetUnderMouse)
		mLastWidgetUnderMouse->Detach (0);
	mLastWidgetUnderMouse = widget;
}

//----------------------------------------------------------------------

const UIWidget *  UIManager::GetTopContextWidget () const
{
	if (mContextPage)
	{
		const UIBaseObject::UIObjectList & olist = mContextPage->GetChildrenRef ();
		if (!olist.empty ())
		{
			const UIBaseObject * const obj = olist.front ();
			if (obj->IsA (TUIWidget))
				return static_cast<const UIWidget *>(obj);
		}
	}
	
	return 0;
}

//----------------------------------------------------------------------

void UIManager::SetTooltipDelaySecs (float secs)
{
	mTicksToTooltip = static_cast<long> (60.0f * secs);
}

//----------------------------------------------------------------------

void UIManager::AbortDragAndDrop (UIWidget * DragTarget)
{
	UIMessage NotificationMessage;
	
	NotificationMessage.Type       = UIMessage::DragCancel;
	NotificationMessage.DragSource = mDraggedSource;
	NotificationMessage.DragObject = mDraggedControl;
	NotificationMessage.DragTarget = DragTarget;
	
	if( DragTarget && mDraggedSource != DragTarget)
		DragTarget->ProcessMessage (NotificationMessage);
	
	if (mDraggedSource)
	{
		mDraggedSource->ProcessMessage (NotificationMessage);
		
		UIWidget * const parentWidget = mDraggedSource->GetParentWidget ();
		
		if (parentWidget)
			parentWidget->ProcessMessage( NotificationMessage );					
	}
	
	SetDragEcho (0, 0, UIPoint::zero);
}

//----------------------------------------------------------------------

void UIManager::SetKeystrokePopsContext     (bool b)
{
	mKeystrokePopsContext = b;
}

//----------------------------------------------------------------------

void UIManager::ClearMouseDownControl()
{
	if (mMouseDownControl != NULL)
	{
		mMouseDownControl->Detach(0);
	}

	mMouseDownControl = 0;
}

//----------------------------------------------------------------------

UIString const & UIManager::getEffectToken(EffectToken const token) const
{
	EffectTokenNameMap::iterator itToken = ms_effectTokenNameMap.find(token);

	if (itToken != ms_effectTokenNameMap.end())
	{
		return itToken->second;
	}

	return Unicode::emptyString;
}

//----------------------------------------------------------------------

void UIManager::replaceCanvasTexturesByName(UINarrowString const & currentTextureName, UINarrowString const & desiredTextureName)
{
	
	UINarrowString newTextureName;
	
	if (desiredTextureName.find("texture/") == std::string::npos)
		newTextureName = "texture/";

	newTextureName += desiredTextureName;

	if (desiredTextureName.find(".dds") == std::string::npos)
		newTextureName += ".dds";


	// Replace texture per shader.

	UIString const currentTextureNameWide(UIUnicode::narrowToWide(currentTextureName));

	for (CanvasShaderMap::Container::iterator itShaderToCanvasMap = mCanvasShaders->c.begin (); itShaderToCanvasMap != mCanvasShaders->c.end (); ++itShaderToCanvasMap)
	{
		CanvasShaderMap::CanvasMap & canvases = (*itShaderToCanvasMap).second;
		CanvasShaderMap::CanvasMap::iterator itCanvas = canvases.find(currentTextureNameWide);
		if (itCanvas != canvases.end())
		{
			UICanvas * const canvas = (*itCanvas).second;
			if (NULL != canvas)
			{
				canvas->Reload(newTextureName);
			}
		}
	}
}

//----------------------------------------------------------------------

void UIManager::setUIIMEManager(UIIMEManager *manager)
{
	mIMEManager = manager;
}

//----------------------------------------------------------------------

UIIMEManager *UIManager::getUIIMEManager()
{
	return mIMEManager;
}

//----------------------------------------------------------------------

char const * UIManager::GetLocaleString() const
{
	char * localName = NULL;

	switch(mLocale) 
	{
	case L_usa:
		localName = "en";
		break;
	case L_japan:
		localName = "ja";
		break;
	default:
		__assume(0);
	}

	return localName;
}

//----------------------------------------------------------------------

void UIManager::SetLocaleByString(std::string const & locale)
{
	Locale const newLocale = (!locale.empty() && (tolower(locale[0]) == 'j')) ? L_japan : L_usa;
	SetLocale(newLocale);
}

//----------------------------------------------------------------------

void UIManager::createSingleton()
{
	if ( !gSingleton )
	{
		gSingleton = new UIManager;
		gIsUIReady = true;
	}
}

//-----------------------------------------------------------------

void UIManager::AddToRenderLastList(UIWidget *widget)
{
	mRenderLastList->push_back(widget);
}

//-----------------------------------------------------------------

void UIManager::RemoveFromRenderLastList(UIWidget *widget)
{
	std::vector<UIWidget *>::iterator i = std::find(mRenderLastList->begin(), mRenderLastList->end(), widget);
	if(i != mRenderLastList->end())
	{
		mRenderLastList->erase(i);
	}
}

//-----------------------------------------------------------------

UICanvas * UIManager::CreateCanvas(UINarrowString const & textureName)
{
	UICanvas * canvas = NULL;
	
	for( CanvasFactoryVector::Container::iterator it = mCanvasFactories->c.begin(); it != mCanvasFactories->c.end() && canvas == NULL; ++it )
	{
		canvas = (*it)->CreateCanvas("", textureName.c_str());
	}
	
	return canvas;
}

//-----------------------------------------------------------------

void UIManager::garbageCollect()
{
	UIBaseObject::garbageCollect();
}

//-----------------------------------------------------------------

void UIManager::enableDrawWidgetBorders(bool enable)
{
	ms_drawWidgetBorders = enable;
}

//-----------------------------------------------------------------

void UIManagerNamespace::drawWidgetRect(UICanvas & canvas, UIWidget * widget)
{
#ifndef _DEBUG
	UI_UNREF(canvas);
	UI_UNREF(widget);
#else
	float const wx = static_cast<float>(widget->GetWorldLocation().x) + 1;
	float const wy = static_cast<float>(widget->GetWorldLocation().y) + 1;
	float const sx = static_cast<float>(widget->GetSize().x) - 1;
	float const sy = static_cast<float>(widget->GetSize().y) - 1;

	UIFloatPoint const p[] =
	{
		UIFloatPoint(wx, wy),
		UIFloatPoint(wx + sx, wy),
		UIFloatPoint(wx + sx, wy + sy),
		UIFloatPoint(wx, wy + sy),
	};

	UILine const lines[] =
	{
		UILine(p[0], p[1]),
		UILine(p[1], p[2]),
		UILine(p[2], p[3]),
		UILine(p[3], p[0]),
	};

	canvas.RenderLines(0, 4, lines, 0);
#endif
}

void UIManagerNamespace::drawWidgetBorders(UICanvas & canvas, UIWidget * widget)
{
#ifndef _DEBUG
	UI_UNREF(canvas);
	UI_UNREF(widget);
#else
	if (widget->IsVisible())
	{
		if (widget->GetChildCount() > 0)
		{
			UIBaseObject::UIObjectList children;
			widget->GetChildren(children);

			for (UIBaseObject::UIObjectList::iterator i = children.begin(); i != children.end(); ++i)
			{
				if ((*i)->IsA(TUIWidget))
				{
					UIWidget * childWidget = static_cast<UIWidget *>(*i);

					drawWidgetBorders(canvas, childWidget);
				}
			}
		}

		drawWidgetRect(canvas, widget);
	}
#endif
}
