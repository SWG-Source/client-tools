#include "_precompile.h"
#include "UIPage.h"

#include "UIButton.h"
#include "UICanvas.h"
#include "UIData.h"
#include "UIDataSourceContainer.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPopupMenu.h"
#include "UIRenderHelper.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIText.h"

#include <cassert>
#include <list>
#include <map>

//======================================================================================

const char *UIPage::TypeName = "Page";

const UILowerString UIPage::PropertyName::DoNotPackChildren = UILowerString ("DoNotPackChildren");
const UILowerString UIPage::PropertyName::ParentSize = UILowerString ("ParentSize");

const UILowerString UIPage::MethodName::Pack = UILowerString ("Pack");
const UILowerString UIPage::MethodName::Wrap = UILowerString ("Wrap");




//======================================================================================
#define _TYPENAME UIPage

namespace UIPageNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(DoNotPackChildren, "", T_bool),
		_DESCRIPTOR(ParentSize, "Force size to the size of the parent page.", T_bool),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIPageNamespace;
//======================================================================================

UIPage::UIPage () :
UIWidget                 (),
mObjects                 (new UIObjectList),
mTabOrder                (new UIObjectList),
mScratchVector           (0),
mWidgetWithKeyboardFocus (0),
mWidgetWithMouseLock     (0),
mWidgetUnderMouse        (0),
mWidgets                 (new UIWidgetVector),
mPageAttributeFlags(PA_Initialize),
mCodeData(0)
{
	UIWidget::SetSelectable (false);
}

//======================================================================================

UIPage::~UIPage ()
{
	while( !mObjects->empty() )
	{
		UIBaseObject * const ChildToRemove = mObjects->front ();
		ChildToRemove->SetParent (0);
		ChildToRemove->Detach( this );
		mObjects->pop_front();
	}
	
	if( mWidgetWithKeyboardFocus )
	{
		UIPage::SelectChild (0);
	}
	
	if( mWidgetWithMouseLock )
	{
		mWidgetWithMouseLock->Detach( this );
		mWidgetWithMouseLock = 0;
	}
	
	if( mWidgetUnderMouse )
	{
		mWidgetUnderMouse->Detach( this );
		mWidgetUnderMouse = 0;
	}

	mTabOrder->clear ();

	delete mObjects;
	mObjects = 0;

	delete mTabOrder;
	mTabOrder = 0;

	delete mWidgets;
	mWidgets = 0;

	delete mScratchVector;
	mScratchVector = 0;

	mCodeData = NULL;
	mWidgetWithKeyboardFocus = NULL;
}

//======================================================================================

bool UIPage::IsA( const UITypeID QueriedType ) const
{
	return QueriedType == TUIPage || UIWidget::IsA( QueriedType );
}		

//======================================================================================

const char *UIPage::GetTypeName () const
{
	return TypeName;
}

//======================================================================================

UIBaseObject *UIPage::Clone () const
{
	return new UIPage;
}

//----------------------------------------------------------------------

bool UIPage::CanSelectForTab () const
{
	if( !UIWidget::CanSelect() )
		return false;

	if (HasPageAttribute(PA_WidgetVectorDirty))
		BuildWidgetVector();

	for( UIWidgetVector::const_iterator i = mWidgets->begin(); i != mWidgets->end(); ++i )
	{
		const UIWidget * const wid = (*i);
		if (wid->IsA (TUIPage))
		{
			if (static_cast<const UIPage *>(wid)->CanSelectForTab ())
				return true;
		}
		else if( wid->CanSelect() )
			return true;
	}

	return false;
}

//======================================================================================

bool UIPage::CanSelect () const
{
	if( !UIWidget::CanSelect() )
		return false;

	if (HasPageAttribute(PA_WidgetVectorDirty))
		BuildWidgetVector();

	for( UIWidgetVector::const_iterator i = mWidgets->begin(); i != mWidgets->end(); ++i )
	{
		const UIWidget * const wid = (*i);
		if( wid->CanSelect() )
			return true;
	}

	return IsSelectable ();
}

//======================================================================================

void UIPage::SetSelected( const bool NewSelected )
{
	UIWidget::SetSelected( NewSelected );

	if( !NewSelected )
	{
		if (HasPageAttribute(PA_WidgetVectorDirty))
			BuildWidgetVector();

		for( UIWidgetVector::const_iterator i = mWidgets->begin(); i != mWidgets->end(); ++i )
			(*i)->SetSelected( NewSelected );
		
		CleanUpFocusHandles();		
	}
}

//----------------------------------------------------------------------

void UIPage::ReleaseMouseLock (const UIPoint & point)
{
	if (mWidgetWithMouseLock)
	{
		UIWidget * const widgetUnder = GetWidgetFromPoint (point, false);
		
		if (widgetUnder != mWidgetWithMouseLock)
		{
			UIMessage exitMessage;
			exitMessage.Type = UIMessage::MouseExit;
			UI_IGNORE_RETURN(mWidgetWithMouseLock->ProcessMessage (exitMessage));	
		}

		if (mWidgetWithMouseLock)
		{
			mWidgetWithMouseLock->Detach( this );
			mWidgetWithMouseLock = 0;
		}
	}
	
	for( UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		UIBaseObject * const o = *i;
		
		if(o && o->IsA( TUIPage ) )
		{
			UIPage * const page = static_cast<UIPage *>(o);
			
			page->ReleaseMouseLock (point - page->GetLocation ());
		}
	}
}

//=============================================================================

void UIPage::GiveWidgetMouseLock( UIWidget *child )
{
	if( mWidgetWithMouseLock )
	{
		mWidgetWithMouseLock->Detach( this );       
	}

	mWidgetWithMouseLock = child;

	if( mWidgetWithMouseLock )
	{
		mWidgetWithMouseLock->Attach( this );
	}
}

//======================================================================================

bool UIPage::ProcessMessage( const UIMessage &msg )
{
	if( UIWidget::ProcessMessage( msg ) )
		return true;
	
	//-- drag messages are sent directly to the child widgets involved, do not propogate downwards
	if (msg.Type >= UIMessage::DragFirst && msg.Type <= UIMessage::DragLast)
		return false;

	//-- context request messages are sent directly to the child widget under the mouse
	if (msg.Type == UIMessage::ContextRequest)
	{
		UIBaseObject * const dso = GetChild ("testpopupds");
		if (dso && dso->IsA (TUIDataSourceContainer))
		{
			UIDataSourceContainer * const dsc = static_cast<UIDataSourceContainer *>(dso);
			UIPopupMenuStyle * const popupStyle = FindPopupStyle ();

			if (popupStyle)
			{
				UIPopupMenu * const pop = new UIPopupMenu(this);
				pop->SetName ("wtf");
				pop->SetStyle (popupStyle);
				pop->SetDataSourceContainer (dsc);
				pop->SetLocation (msg.MouseCoords);
				UIManager::gUIManager ().PushContextWidget (*pop);
			} //lint !e429 //custodial pop
		}

		return false;
	}

	if (msg.IsIMEMessage())
	{
		if( mWidgetWithKeyboardFocus && mWidgetWithKeyboardFocus->WantsMessage( msg ) )
		{
			if (mWidgetWithKeyboardFocus->ProcessMessage( msg ))
				return true;
		}
	}

	if( msg.IsKeyMessage() )
	{
		if (!mWidgetWithKeyboardFocus || !mWidgetWithKeyboardFocus->WantsMessage (msg))
		{
			if (msg.Keystroke != UIMessage::Escape || mWidgetWithKeyboardFocus)
				MoveKeyboardFocus( true );
		}

		if( msg.Keystroke == UIMessage::Tab )
		{
			// Be careful modifying this code.  Any changes you make must handle:
			// focus transfer into and out of arbitrarily nested pages, pages with no controls
			// to select, pages with only one control to select, focus looping at the root page.
			// If focus transfer results in the focus going to the currently selected control
			// you should not call that control's SetSelected function.
			if( msg.Type == UIMessage::KeyUp )
				return true;
			
			if( !mWidgetWithKeyboardFocus )
			{
				MoveKeyboardFocus( !(msg.Modifiers.LeftShift || msg.Modifiers.RightShift) );
				
				if( mWidgetWithKeyboardFocus )
				{
					if( mWidgetWithKeyboardFocus->IsA( TUIPage ) )
						return mWidgetWithKeyboardFocus->ProcessMessage( msg );
					else
					{
						mWidgetWithKeyboardFocus->SetSelected( true );
						return true;
					}
				}
			}
			else if (mWidgetWithKeyboardFocus->IsA( TUIPage ) || mWidgetWithKeyboardFocus->HasAttribute(UIWidget::BF_AbsorbsTab))
				return mWidgetWithKeyboardFocus->ProcessMessage( msg );
			
			ProcessTab( msg );
			return true;
		}
		else
		{
			if( !mWidgetWithKeyboardFocus || !mWidgetWithKeyboardFocus->WillDraw() || !mWidgetWithKeyboardFocus->GetsInput () || !mWidgetWithKeyboardFocus->IsEnabled ())
			{
				if (msg.Keystroke != UIMessage::Escape || mWidgetWithKeyboardFocus)
					MoveKeyboardFocus( true );
			}
			
			if( mWidgetWithKeyboardFocus && mWidgetWithKeyboardFocus->WantsMessage( msg ) )
			{
				if (mWidgetWithKeyboardFocus->ProcessMessage( msg ))
					return true;
			}
			
			if( msg.Type != UIMessage::KeyUp )
			{
				if( msg.Keystroke == UIMessage::Enter )
				{
					UIButton *theDefaultButton = FindDefaultButton(true);
					
					if( theDefaultButton )
					{						
						theDefaultButton->Press();
						return true;
					}
				}
				else if( msg.Keystroke == UIMessage::Escape )
				{
					if (!msg.Modifiers.isShiftDown () &&
						!msg.Modifiers.isControlDown () &&
						!msg.Modifiers.isAltDown ())
					{
						UIButton *theCancelButton = FindCancelButton(true);
						
						if( theCancelButton )
						{
							if( msg.Type != UIMessage::KeyUp )
							{
								theCancelButton->Press();
								return true;
							}
						}
					}
				}			
			}
		}
			
		return false;
	}

	//----------------------------------------------------------------------

	else if( msg.IsMouseMessage() )
	{
		UIWidgetList::iterator i;

		if( mWidgetWithMouseLock)
		{
			UIMessage TranslatedMessage;
			UIPoint		p (mWidgetWithMouseLock->GetLocation());
			
			//-----------------------------------------------------------------
			//-- widget with mouse lock may not be a direct child of the page

			UIBaseObject * parent = mWidgetWithMouseLock;
			while ((parent = parent->GetParent ()) != 0 && parent != this)
			{
				if (parent->IsA (TUIWidget))
					p += static_cast<UIWidget *>(parent)->GetLocation ();
			}

			TranslatedMessage = msg;
			TranslatedMessage.MouseCoords = (msg.MouseCoords - p) + GetScrollLocation();
			
			const bool Processed = mWidgetWithMouseLock->ProcessMessage( TranslatedMessage );

			// We need to check mWidgetWithMouseLock here again, as 
			// ProcessMessage could cause the control to go away
			if( mWidgetWithMouseLock && 
				  (msg.Type == UIMessage::LeftMouseUp || 
					msg.Type == UIMessage::MiddleMouseUp ||
					msg.Type == UIMessage::RightMouseUp ||
					msg.Type == UIMessage::MouseExit
					) )
			{
				if( !mWidgetWithMouseLock->HitTest( TranslatedMessage.MouseCoords ) )
				{
					UIMessage MouseOverMessage( TranslatedMessage );

					MouseOverMessage.Type = UIMessage::MouseExit;

					if( mWidgetUnderMouse )
					{
						UI_IGNORE_RETURN(mWidgetUnderMouse->ProcessMessage( MouseOverMessage ));
						mWidgetUnderMouse->Detach( this );
						mWidgetUnderMouse = 0;
					}
				}

				mWidgetWithMouseLock->Detach( this );
				mWidgetWithMouseLock = 0;
			}

			return Processed;
		}

		bool childWasHit = false;
		if (!ProcessMouseMessageUsingControlSet( msg, *mObjects, childWasHit ))
		{
			if (UIWidget::ProcessUserMessage (msg))
				return true;
			
			if (msg.IsMouseButtonMessage())
			{
				if (!IsUserModifying())
				{
					if (IsUserMovable() || IsUserDragScrollable() || IsUserResizable())
						return IsAbsorbsInput();
				}
			}

			return false;
		}
		else
			return true;
	}
	 
	return IsAbsorbsInput ();
}

//----------------------------------------------------------------------

void UIPage::SetWidgetWithMouseLock (UIWidget * w)
{
	if(mWidgetWithMouseLock != w)
	{
		if( w )
			w->Attach( this );
		
		if( mWidgetWithMouseLock )
			mWidgetWithMouseLock->Detach( this );
		
		mWidgetWithMouseLock = w;
	}
}

//======================================================================================

bool UIPage::ProcessMouseMessageUsingControlSet( const UIMessage &msg, UIObjectList &ControlSet, bool & childWasHit )
{
	const UIPoint & ScrollLocation = GetScrollLocation();

	for( UIObjectList::iterator i = ControlSet.begin(); i != ControlSet.end(); ++i )
	{
		UIBaseObject * const o = *i;

		if (NULL == o)
			continue;

		if( o->IsA( TUIWidget ) )
		{
			UIWidget * const w = static_cast<UIWidget *>( o );

			if (!w->IsEnabled ())
				continue;

			const UIPoint p ((msg.MouseCoords - w->GetLocation ()) + ScrollLocation);

			if( w->WantsMessage( msg ) && ( ( msg.Type == UIMessage::MouseExit && w == mWidgetUnderMouse ) || ( w->WillDraw() && w->HitTest( p ) ) ) )
			{
				bool Processed;
				childWasHit = true;
				
				UIMessage TranslatedMessage   = msg;
				TranslatedMessage.MouseCoords = p;
				
				w->Attach(0);
				Processed = w->ProcessMessage( TranslatedMessage );
				UI_DEBUG_REPORT_LOG_PRINT((w->GetRefCount () <= 1), ("UI Widget %s has an invalid reference count (did you close a page during a callback?).\n", w->GetFullPath().c_str()));
				assert (w->GetRefCount () > 1); //lint !e1776
				w->Detach(0);
				
				
				if( (msg.Type == UIMessage::LeftMouseDown) || 
					(msg.Type == UIMessage::MiddleMouseDown) ||
					(msg.Type == UIMessage::RightMouseDown) )
				{
					if( Processed && w != mWidgetWithMouseLock)
					{
						SetWidgetWithMouseLock (w);						
					}
				}
				
				if (!Processed && w->IsAbsorbsInput ())
				{
					if (UIManager::gUIManager ().IsContextMessage (msg, true) &&  w->IsContextCapable (true))// || w->IsContextToParent ()))
						Processed = true;
					if (UIManager::gUIManager ().IsContextMessage (msg, false) && w->IsContextCapable (false))// || w->IsContextToParent ()))
						Processed = true;
				}

				if (Processed || w->IsAbsorbsInput ())
				{
					if( msg.Type == UIMessage::MouseMove )
					{
						if( mWidgetUnderMouse != w )
						{
							UIMessage MouseOverMessage( TranslatedMessage );
							
							if( mWidgetUnderMouse )
							{
								MouseOverMessage.Type        = UIMessage::MouseExit;
								MouseOverMessage.MouseCoords = UIPoint::zero;
								UI_IGNORE_RETURN(mWidgetUnderMouse->ProcessMessage( MouseOverMessage ));
								if( mWidgetUnderMouse )
								{
									mWidgetUnderMouse->Detach( this );
									mWidgetUnderMouse            = 0;
								}
							}
							
							mWidgetUnderMouse            = w;
							MouseOverMessage.Type        = UIMessage::MouseEnter;
							MouseOverMessage.MouseCoords = p;
							UI_IGNORE_RETURN(mWidgetUnderMouse->ProcessMessage( MouseOverMessage ));						
							mWidgetUnderMouse->Attach( this );
						}
					}
					
					if (msg.MovesFocus() && w != mWidgetWithKeyboardFocus)
					{
						if (w->IsA (TUIPage))
						{
							if (static_cast<UIPage *>(w)->CanSelectForTab ())
								SelectChild (w);
						}
						else if (w->CanSelect ())
							SelectChild (w);
					}
					return Processed;
				}
			}
		}
	}

	if (childWasHit)
		return false;

	if( msg.Type == UIMessage::MouseMove )
	{
		if( mWidgetUnderMouse )
		{
			UIMessage MouseOverMessage( msg );

			const UIPoint & p = mWidgetUnderMouse->GetLocation();
			MouseOverMessage.MouseCoords = (msg.MouseCoords - p) + ScrollLocation;
			MouseOverMessage.Type = UIMessage::MouseExit;

			UI_IGNORE_RETURN(mWidgetUnderMouse->ProcessMessage( MouseOverMessage ));
			mWidgetUnderMouse->Detach( this );
			mWidgetUnderMouse = 0;
		}
	}
	return false;
}

//----------------------------------------------------------------------

void UIPage::ForcePackChildren()
{
	// Force pack.
	Pack();

	// Depth first.
	for( UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i )
	{	
		UIPage * const page = UI_ASOBJECT(UIPage, *i);
		if (page)
			page->ForcePackChildren();
	}
}

//======================================================================================

void UIPage::Pack()
{
	// Prevent packing from stacking.
	if (HasPageAttribute(PA_IsPacking)) 
	{
		SetPackDirty(true);
	}
	else
	{
		SetPageAttribute(PA_IsPacking, true);

		SetSizeToParentSize();

		if (GetShrinkWrap()) 
		{
			if (IsUserModifying()) 
			{
				// Apply packing.
				ApplyPackingToChildWidgets();
				
				SetPackDirty(true);
			}
			else
			{
				// Set packing complete.
				SetPackDirty(false);

				// Shrink wrap.
				WrapChildren();
			}
		}
		else
		{
			// Set packing complete.
			SetPackDirty(false);

			// Apply packing.
			ApplyPackingToChildWidgets();
		}

		SetPageAttribute(PA_IsPacking, false);
	}
}

//----------------------------------------------------------------------

void UIPage::PackIfDirty ()
{
	if (HasPageAttribute(PA_PackDirty))
		Pack();
}

//======================================================================================

void UIPage::ApplyPacking( UIWidget *theWidget) const
{	
	UI_IGNORE_RETURN(theWidget->PackSelfSize ());
	UI_IGNORE_RETURN(theWidget->PackSelfLocation ());
}

//======================================================================================

void UIPage::AddOverlay( UIWidget *NewOverlay )
{
	assert( NewOverlay ); //lint !e1776 //broken msvc++
	assert( NewOverlay != this ); //lint !e1776 //broken msvc++
	assert( !NewOverlay->IsA( TUICanvas ) ); //lint !e1776 //broken msvc++

	NewOverlay->Attach( this );
	mObjects->push_front( NewOverlay );
	UI_IGNORE_RETURN(SetPageAttribute(PA_WidgetVectorDirty, true));
}

//======================================================================================

void UIPage::RemoveOverlay( UIWidget *OverlayToRemove )
{
	UI_IGNORE_RETURN(RemoveChild( OverlayToRemove ));
}

//======================================================================================

void UIPage::SelectChild( UIBaseObject *ChildToSelect )
{
	UIWidget * const widgetToSelect = static_cast<UIWidget *>(ChildToSelect);

	if (mWidgetWithKeyboardFocus != widgetToSelect)
	{
		if (widgetToSelect)
		{
			if (!widgetToSelect->CanSelect ())
				return;

			widgetToSelect->Attach ( this );
		}

		if(mWidgetWithKeyboardFocus)
		{
			mWidgetWithKeyboardFocus->SetSelected (false);
			mWidgetWithKeyboardFocus->Detach      (this);
			mWidgetWithKeyboardFocus = 0;
		}

		mWidgetWithKeyboardFocus = widgetToSelect;

		if (NULL != mWidgetWithKeyboardFocus)
		{
			mWidgetWithKeyboardFocus->SetSelected    (true);
		}

		UIWidget::SelectChild( widgetToSelect );
	}
}

//-----------------------------------------------------------------

UIWidget * UIPage::GetFocusedLeafWidget ()
{
	if (mWidgetWithKeyboardFocus)
		return mWidgetWithKeyboardFocus->GetFocusedLeafWidget ();

	return UIWidget::GetFocusedLeafWidget ();
}

//======================================================================================

UIWidget *UIPage::GetWidgetFromPoint( const UIPoint &PointToTest, bool mustGetInput ) const
{
	if (HasPageAttribute(PA_PackDirty))
		const_cast<UIPage *>(this)->Pack ();

	const UIPoint & ScrollLocation = GetScrollLocation();

	for( UIObjectList::const_iterator i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		UIBaseObject * const o = *i;

		if( o->IsA( TUIWidget ) )
		{
			UIWidget * const w = static_cast<UIWidget *>( o );
			const UIPoint p ((PointToTest - w->GetLocation()) + ScrollLocation);

			if( w->WillDraw() && w->HitTest( p ) )
			{
				if (!mustGetInput || (w->GetsInput () && w->IsEnabled ()))
				{
					UIWidget * const leaf = w->GetWidgetFromPoint( p, mustGetInput );

					if (leaf && leaf->IsAbsorbsInput ())
						return leaf;

					if (w->IsAbsorbsInput ())
						return w;
				}
			}
		}
	}

	return UIWidget::GetWidgetFromPoint( PointToTest, mustGetInput );
}

//======================================================================================

void UIPage::GetWidgetList( UIWidgetList &Out ) const
{
	for( UIObjectList::const_iterator i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		UIBaseObject *o = *i;

		if( o->IsA( TUIWidget ) )
			Out.push_back( static_cast<UIWidget *>( o ) );
	}
}

//======================================================================================

bool UIPage::AddChild( UIBaseObject *ChildToAdd )
{
	assert( ChildToAdd ); //lint !e1776 //broken msvc++
	assert( ChildToAdd != this ); //lint !e1776 //broken msvc++
	assert( !ChildToAdd->IsA( TUICanvas ) ); //lint !e1776 //broken msvc++
	assert (std::find (mObjects->begin (), mObjects->end (), ChildToAdd) == mObjects->end ()); //lint !e1776 //broken msvc++

	if (!ChildToAdd || ChildToAdd == this) //lint !e774 //always true, not
		return false;

	mObjects->push_back( ChildToAdd );

	ChildToAdd->SetParent( this );
	ChildToAdd->Attach( this );

	if( ChildToAdd->IsA( TUIWidget ) )
	{
		UIWidget * const w = static_cast<UIWidget *>(ChildToAdd);
		mTabOrder->push_back( w );
		UI_IGNORE_RETURN(SetPageAttribute(PA_WidgetVectorDirty, true));
		w->ResetPackLocationInfo ();
		w->ResetPackSizeInfo ();

		if (!mScratchVector)
		{
			if (ChildToAdd->IsA (TUIButton))
				mScratchVector = new UIWidgetVector;
		}
	}
	
	return true;
}

//----------------------------------------------------------------------

bool UIPage::InsertChildBefore (UIBaseObject * ChildToAdd, const UIBaseObject * childToPrecede)
{
	assert( ChildToAdd ); //lint !e1776 //broken msvc++
	assert( ChildToAdd != this ); //lint !e1776 //broken msvc++
	assert( !ChildToAdd->IsA( TUICanvas ) ); //lint !e1776 //broken msvc++
	assert (std::find (mObjects->begin (), mObjects->end (), ChildToAdd) == mObjects->end ()); //lint !e1776 //broken msvc++ 
	
	if (!ChildToAdd || ChildToAdd == this) //lint !e774 //always true, not
		return false;

	if (!childToPrecede)
		mObjects->push_back (ChildToAdd);
	else
	{
		const UIObjectList::iterator it = std::find (mObjects->begin (), mObjects->end (), childToPrecede);
		assert (it != mObjects->end ()); //lint !e1776 //broken msvc++
		
		UI_IGNORE_RETURN(mObjects->insert (it, ChildToAdd));
	}

	ChildToAdd->SetParent( this );
	ChildToAdd->Attach( this );

	if( ChildToAdd->IsA( TUIWidget ) )
	{
		UIWidget * const w = static_cast<UIWidget *>(ChildToAdd);
		mTabOrder->push_back( w );
		UI_IGNORE_RETURN(SetPageAttribute(PA_WidgetVectorDirty, true));
		w->ResetPackLocationInfo ();
		w->ResetPackSizeInfo ();

		if (!mScratchVector)
		{
			if (ChildToAdd->IsA (TUIButton))
				mScratchVector = new UIWidgetVector;
		}
	}

	return true;
}

//----------------------------------------------------------------------

bool UIPage::InsertChildAfter (UIBaseObject * ChildToAdd, const UIBaseObject * childToSucceed)
{
	assert( ChildToAdd ); //lint !e1776 //broken msvc++
	assert( ChildToAdd != this ); //lint !e1776 //broken msvc++
	assert( !ChildToAdd->IsA( TUICanvas ) ); //lint !e1776 //broken msvc++
	assert (std::find (mObjects->begin (), mObjects->end (), ChildToAdd) == mObjects->end ()); //lint !e1776 //broken msvc++
	
	if (!ChildToAdd || ChildToAdd == this) //lint !e774 //always true, not
		return false;

	if (!childToSucceed)
		mObjects->push_front (ChildToAdd);
	else
	{		
		UIObjectList::iterator it = std::find (mObjects->begin (), mObjects->end (), childToSucceed);
		assert (it != mObjects->end ()); //lint !e1776 //broken msvc++
		
		if (++it == mObjects->end ())
			mObjects->push_back (ChildToAdd);
		else
			UI_IGNORE_RETURN(mObjects->insert (it, ChildToAdd));
	}
	
	ChildToAdd->SetParent( this );
	ChildToAdd->Attach( this );

	if( ChildToAdd->IsA( TUIWidget ) )
	{
		UIWidget * const w = static_cast<UIWidget *>(ChildToAdd);
		mTabOrder->push_back( w );
		UI_IGNORE_RETURN(SetPageAttribute(PA_WidgetVectorDirty, true));
		w->ResetPackLocationInfo ();
		w->ResetPackSizeInfo ();

		if (!mScratchVector)
		{
			if (ChildToAdd->IsA (TUIButton))
				mScratchVector = new UIWidgetVector;
		}
	}

	return true;
}

//======================================================================================

bool UIPage::RemoveChild( UIBaseObject *ChildToRemove )
{
	assert( ChildToRemove ); //lint !e1776 //broken msvc++
	assert( ChildToRemove != this ); //lint !e1776 //broken msvc++
	assert( !ChildToRemove->IsA( TUICanvas ) ); //lint !e1776 //broken msvc++
	assert( ChildToRemove->GetParent () == this ); //lint !e1776 //broken msvc++
	
	//-----------------------------------------------------------------
	//-- widget with keyboard focus may not be a direct child of this container

	{
		UIBaseObject * parent = ChildToRemove;

		while (parent)
		{
			if (parent == this)
				break;

			if (parent == mWidgetWithKeyboardFocus)
			{
				SelectChild (0);
			}

			parent = parent->GetParent ();
		}
	}
	
	if( NULL != mWidgetWithMouseLock && ChildToRemove == mWidgetWithMouseLock )
	{
		mWidgetWithMouseLock->Detach( this );
		mWidgetWithMouseLock = 0;
	}
	
	if (NULL != mWidgetUnderMouse && ChildToRemove == mWidgetUnderMouse)
	{
		mWidgetUnderMouse->Detach( this );
		mWidgetUnderMouse = 0;
	}

	for( UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		if( *i == ChildToRemove )
		{
			UI_IGNORE_RETURN(mObjects->erase(i));
			
			if( ChildToRemove->IsA( TUIWidget ) )
			{
				for( UIObjectList::iterator j = mTabOrder->begin(); j != mTabOrder->end(); ++j )
				{
					if( *j == ChildToRemove )
					{
						UI_IGNORE_RETURN(mTabOrder->erase(j));
						break;
					}
				}
				UI_IGNORE_RETURN(SetPageAttribute(PA_WidgetVectorDirty, true));
			}
			
			ChildToRemove->SetParent (0);
			ChildToRemove->Detach( this );		
			return true;
		}
	}

	return false;
}

//======================================================================================
void UIPage::GetChildren( UIObjectList &Out ) const
{
	Out.insert( Out.end(), mObjects->begin(), mObjects->end() );	
}

//======================================================================================
unsigned long UIPage::GetChildCount () const
{
	return mObjects->size();
}

//======================================================================================
UIBaseObject *UIPage::GetChild( const char *ChildName ) const
{
	const char *pSeparator = strchr( ChildName, '.' );
	int					len;

	if( pSeparator )
		len = pSeparator - ChildName;
	else
		len = static_cast<int>(strlen( ChildName ));

	for ( ; ; )
	{
		const UIObjectList::const_iterator end = mObjects->end();
		for( UIObjectList::const_iterator i = mObjects->begin(); i != end; ++i )
		{
			UIBaseObject *o = *i;

			if( o->IsName( ChildName, static_cast<size_t>(len) ) )
			{
				if( pSeparator )
				{
					UIBaseObject *Child = o->GetChild( ChildName + len + 1 );

					if( Child )
						return Child;
				}
				else
					return o;
			}
		}

		if( pSeparator )
		{
			pSeparator = strchr( pSeparator + 1, '.' );

			if( pSeparator )
				len = pSeparator - ChildName;
			else
				len = static_cast<int>(strlen( ChildName ));
		}
		else
			return 0;
	}
}

//----------------------------------------------------------------------

void UIPage::Render( UICanvas &DestinationCanvas ) const
{
	UI_PROFILER_ENTER(mFullPath);

	if (HasPageAttribute(PA_PackDirty))
		const_cast<UIPage *>(this)->Pack ();

	if( !GetParent() )
	{
		if( !WillDraw() )
		{
			UI_PROFILER_LEAVE(mFullPath);
			return;
		}

		DestinationCanvas.PushState();
		DestinationCanvas.Translate( GetLocation() );
		UI_IGNORE_RETURN(DestinationCanvas.Clip( GetRect() ));
		DestinationCanvas.Translate( -GetScrollLocation() );
		DestinationCanvas.ModifyOpacity( GetOpacity() );
	}
 
	UIWidget::Render (DestinationCanvas);

	if (HasPageAttribute(PA_WidgetVectorDirty))
		BuildWidgetVector();

	if (GetDepthOverride()) 
	{
		UICanvasState state = DestinationCanvas.GetCurrentState();
		state.mDepth = GetDepth();
		state.DepthOverride = GetDepthOverride();
		DestinationCanvas.SetCurrentState(state);
	}

	if (!mWidgets->empty ())
	{
		if (IsA(TUIVolumePage))
		{
			UIRenderHelper::RenderObjects(DestinationCanvas, *mWidgets, NULL);
		}
		else
		{
			UIRenderHelper::RenderObjects(DestinationCanvas, *mWidgets, mScratchVector);
		}
	}

	if( !GetParent() )
		DestinationCanvas.PopState();

	UI_PROFILER_LEAVE(mFullPath);
}

//======================================================================================

void UIPage::MinimizeResources ()
{
	UIWidget::MinimizeResources();

	for( UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i )
		(*i)->MinimizeResources();
}

//======================================================================================

void UIPage::ProcessTab( const UIMessage &msg, bool allowParentCycling)
{
	const bool MoveFocusForward                      = !msg.Modifiers.isShiftDown ();
	UIWidget * const PreviousWidgetWithKeyboardFocus = mWidgetWithKeyboardFocus;
	
	MoveKeyboardFocus (MoveFocusForward);

	if( !mWidgetWithKeyboardFocus )
	{
		UIBaseObject * const parent     = GetParent();
		UIPage *       const parentPage = (parent && parent->IsA (TUIPage)) ? static_cast<UIPage *>(parent) : 0;

		if (parent && parentPage && !IsTabRoot () && parent->GetParent ())
		{
			// If we have a parent (and it is not a direct child of the root) our parent is responsible for looping our focus
			if (allowParentCycling)
				parentPage->ProcessTab( msg, true );
		}
		else
		{
			// If we don't have a parent we're responsible for looping our focus
			MoveKeyboardFocus( MoveFocusForward );

			if( mWidgetWithKeyboardFocus && mWidgetWithKeyboardFocus->IsA( TUIPage ) )
				static_cast<UIPage *>( mWidgetWithKeyboardFocus )->ProcessTab( msg, false);
		}
	}
	else if( PreviousWidgetWithKeyboardFocus != mWidgetWithKeyboardFocus )
	{
		if (mWidgetWithKeyboardFocus->IsA (TUIPage))
		{
			UIPage * const focusedPage = static_cast<UIPage *>(mWidgetWithKeyboardFocus);

			focusedPage->CleanUpFocusHandles ();
			focusedPage->ProcessTab (msg, false);//MoveKeyboardFocus( MoveFocusForward );
		}
	}
}

//======================================================================================

void UIPage::MoveKeyboardFocus( bool Forward )
{
	if( Forward )
		SelectChild (GetNextWidgetInTabOrder( mWidgetWithKeyboardFocus ));
	else
		SelectChild (GetPreviousWidgetInTabOrder( mWidgetWithKeyboardFocus ));
}	

//======================================================================================

UIWidget *UIPage::GetNextWidgetInTabOrder( const UIBaseObject *RelativeTo ) const
{	
	UIObjectList::const_iterator i;

	if( RelativeTo )
	{
		if( RelativeTo->IsA( TUIWidget ) && !static_cast<const UIWidget *>( RelativeTo )->WillDraw() )
			i = mTabOrder->begin();
		else
		{
			for( i = mTabOrder->begin(); i != mTabOrder->end(); ++i )
			{
				UIBaseObject * const obj = *i;
				if( obj == RelativeTo )
				{
					++i;
					break;
				}
			}

			if( i == mTabOrder->end() )
				return 0;
		}
	}
	else
	{
		i = mTabOrder->begin();
	}
	
	for( ; i != mTabOrder->end(); ++i )
	{
		UIBaseObject *o = *i;

		if (o->IsA (TUIWidget))
		{
			UIWidget * const wid = static_cast<UIWidget *>(o);

			if (wid->IsA (TUIPage))
			{
				if (static_cast<UIPage *>(wid)->CanSelectForTab ())
					return wid;
			}
			else if (wid->CanSelect ())
				return wid;
		}
	}

	return 0;
}

//======================================================================================

UIWidget *UIPage::GetPreviousWidgetInTabOrder( const UIBaseObject *RelativeTo ) const
{
	UIObjectList::reverse_iterator i;

	if( RelativeTo )
	{
		if( RelativeTo->IsA( TUIWidget ) && !static_cast<const UIWidget *>( RelativeTo )->WillDraw() )
			i = mTabOrder->rbegin();
		else
		{
			for( i = mTabOrder->rbegin(); i != mTabOrder->rend(); ++i )
			{
				UIBaseObject * const obj = *i;
				if( obj == RelativeTo )
				{
					++i;
					break;
				}
			}

			if( i == mTabOrder->rend() )
				return 0;
		}
	}
	else
	{
		i = mTabOrder->rbegin();
	}
	
	for( ; i != mTabOrder->rend(); ++i )
	{
		UIBaseObject *o = *i;

		if (o->IsA (TUIWidget))
		{
			UIWidget * const wid = static_cast<UIWidget *>(o);

			if (wid->IsA (TUIPage))
			{
				if (static_cast<UIPage *>(wid)->CanSelectForTab ())
					return wid;
			}
			else if (wid->CanSelect ())
				return wid;
		}
	}

	return 0;
}

//======================================================================================

bool UIPage::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == UIPage::MethodName::Pack)
	{
		Pack ();
		return true;
	}

	if (Name == UIPage::MethodName::Wrap)
	{
		WrapChildren();
		return true;
	}

	const size_t pSeparator = Name.get ().find ('.');
	
	if( pSeparator != static_cast<size_t>(std::string::npos))
	{
		const int len = static_cast<int>(pSeparator);

		for( UIObjectList::iterator o = mObjects->begin(); o != mObjects->end(); ++o )
		{
			UIBaseObject *theObject = *o;

			if( theObject->IsName( Name.c_str (), static_cast<size_t>(len) ) )
				return theObject->SetProperty( UILowerString(Name.c_str () + pSeparator + 1), Value );
		}
		// Fall through
	}

	if(Name == PropertyName::DoNotPackChildren)
	{
		bool bIgnorePacking = false;
		if(!UIUtils::ParseBoolean(Value, bIgnorePacking))
			return false;
	
		SetDoNotPackChildren(bIgnorePacking);
		return true;
	}
	else if(Name == PropertyName::ParentSize)
	{
		bool bParentSize = false;
		if(!UIUtils::ParseBoolean(Value, bParentSize))
			return false;
	
		SetPageAttribute(PA_ParentSize, bParentSize);
		return true;
	}


	
	return UIWidget::SetProperty( Name, Value );
}

//======================================================================================

bool UIPage::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if(Name == PropertyName::DoNotPackChildren)
	{
		return UIUtils::FormatBoolean(Value, GetDoNotPackChildren());
	}
	else if(Name == PropertyName::ParentSize)
	{
		return UIUtils::FormatBoolean(Value, HasPageAttribute(PA_ParentSize));
	}

	return UIWidget::GetProperty( Name, Value );
}

//======================================================================================

void UIPage::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIPage::GetPropertyNames( UIPropertyNameVector &In, bool forCopy) const
{
	In.push_back(PropertyName::DoNotPackChildren);
	In.push_back(PropertyName::ParentSize);

	UIWidget::GetPropertyNames( In, forCopy );
}

//======================================================================================

bool UIPage::CanChildMove( UIBaseObject *ObjectToMove, ChildMovementDirection MoveDirection )
{
	if( !ObjectToMove )
		return false;

	for( UIObjectList::const_iterator i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		if( *i == ObjectToMove )
		{
			if( i == mObjects->begin() )
			{
				if( (MoveDirection == Up) || (MoveDirection == Top) )
					return false;
			}
			else if( ++i == mObjects->end() )
			{
				if( (MoveDirection == Down) || (MoveDirection == Bottom) )
					return false;
			}
			return true;
		}			
	}	
	return false;
}

//-----------------------------------------------------------------

void UIPage::ChangeFocusOrder (UIWidget *insertBefore, UIWidget *child)
{
	assert (child); //lint !e1776 //broken msvc++
	assert (child != insertBefore); //lint !e1776 //broken msvc++
	UIObjectList::iterator insertIter = insertBefore ? std::find (mTabOrder->begin(), mTabOrder->end(), insertBefore) : mTabOrder->end ();
	UIObjectList::iterator childIter = std::find (mTabOrder->begin(), mTabOrder->end(), child);

	assert (childIter != mTabOrder->end ()); //lint !e1776 //broken msvc++
	
	UI_IGNORE_RETURN(mTabOrder->erase (childIter));
	UI_IGNORE_RETURN(mTabOrder->insert (insertIter, child));
}

//======================================================================================

bool UIPage::MoveChild( UIBaseObject *ObjectToMove, ChildMovementDirection MoveDirection )
{
	if( !ObjectToMove )
		return false;

	UI_IGNORE_RETURN(SetPageAttribute(PA_WidgetVectorDirty, true));

	UIObjectList::iterator i = std::find (mObjects->begin(), mObjects->end(), ObjectToMove);

	if (i == mObjects->end())
	{
		assert (false); //lint !e1776 //broken msvc++
		return false; //lint !e527 //unreachable, not
	}
	
	UIBaseObject *o = *i;
		
	if( (MoveDirection == Up) )
	{
		if( i != mObjects->begin() )
		{
			--i;
			UIBaseObject * const insertBefore = *i;
			UI_IGNORE_RETURN(mObjects->insert( i, o ));
			UI_IGNORE_RETURN(mObjects->erase( ++i ));

			if (ObjectToMove->IsA (TUIWidget) && insertBefore->IsA (TUIWidget))
				ChangeFocusOrder (static_cast<UIWidget *>(insertBefore), static_cast<UIWidget *>(ObjectToMove));

			return true;
		}
	}
	else if( (MoveDirection == Top) )
	{
		if( i != mObjects->begin() )
		{
			UIBaseObject * const insertBefore = mTabOrder->empty() ? 0 : mTabOrder->front ();

			UI_IGNORE_RETURN(mObjects->insert( mObjects->begin(), o ));
			UI_IGNORE_RETURN(mObjects->erase(i));

			if (ObjectToMove->IsA (TUIWidget) && 
				insertBefore != ObjectToMove && 
				(!insertBefore || insertBefore->IsA (TUIWidget)))
				ChangeFocusOrder (static_cast<UIWidget *>(insertBefore), static_cast<UIWidget *>(ObjectToMove));

			return true;
		}
	}
	else if( (MoveDirection == Down) )
	{		
		UIObjectList::iterator InsertPoint = i;
		++InsertPoint;

		if (InsertPoint != mObjects->end ())
			++InsertPoint;

		UIBaseObject * const insertBefore = InsertPoint != mObjects->end () ? *InsertPoint : 0;

		UI_IGNORE_RETURN(mObjects->insert( InsertPoint, o ));
		UI_IGNORE_RETURN(mObjects->erase( i ));

		if (ObjectToMove->IsA (TUIWidget) && (!insertBefore || insertBefore->IsA (TUIWidget)))
			ChangeFocusOrder (static_cast<UIWidget *>(insertBefore), static_cast<UIWidget *>(ObjectToMove));

		return true;
	}

	else if( (MoveDirection == Bottom) )
	{
		if( o != mObjects->back() )
		{
			UI_IGNORE_RETURN(mObjects->insert( mObjects->end(), o ));
			UI_IGNORE_RETURN(mObjects->erase( i )); 

			if (ObjectToMove->IsA (TUIWidget))
				ChangeFocusOrder (0, static_cast<UIWidget *>(ObjectToMove));

			return true;
		}
	}
	else
	{
		assert( false ); //lint !e1776 //broken msvc++
	}
	
	return false;
}

//======================================================================================

void UIPage::Link ()
{
	UIWidget::Link();

	bool hasUIData = false;
	
	for( UIObjectList::const_iterator i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		(*i)->Link();
		if (!hasUIData)
			hasUIData = (*i)->IsA(TUIData);
	}

	if (hasUIData)
	{
		mCodeData = static_cast<UIData *>(GetObjectFromPath("CodeData", TUIData));
	}
}

//======================================================================================

void UIPage::BuildWidgetVector () const
{
	UI_IGNORE_RETURN(mWidgets->erase(mWidgets->begin(), mWidgets->end()));

	for( UIObjectList::const_iterator i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		UIBaseObject *o = *i;

		if( o->IsA( TUIWidget ) )
			mWidgets->push_back( static_cast<UIWidget *>( o ) );
	}

	UI_IGNORE_RETURN(SetPageAttribute(PA_WidgetVectorDirty, false));
}

//======================================================================================

void UIPage::CleanUpFocusHandles ()
{
	if( mWidgetWithKeyboardFocus )
	{
		SelectChild (0);
	}

	if( mWidgetWithMouseLock )
	{
		mWidgetWithMouseLock->Detach( this );
		mWidgetWithMouseLock = 0;
	}

	if( mWidgetUnderMouse )
	{
		mWidgetUnderMouse->Detach( this );
		mWidgetUnderMouse = 0;
	}
}

//======================================================================================

UIButton *UIPage::FindDefaultButton (bool useFocusedChild)
{
	UIObjectList::iterator i;

	if(useFocusedChild && mWidgetWithKeyboardFocus && mWidgetWithKeyboardFocus->IsA( TUIPage ) )
	{
		UIButton * const button = static_cast<UIPage *>( mWidgetWithKeyboardFocus )->FindDefaultButton(useFocusedChild);
		if (button)
			return button;
	}
	
	for( i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		UIBaseObject *theObject = *i;

		if( theObject->IsA( TUIButton ) )
		{
			UIButton * const theButton = static_cast<UIButton *>( theObject );

			if( theButton->IsDefaultButton() && theButton->IsVisible() && theButton->GetsInput() && theButton->IsEnabled() )
				return theButton;
		}
	}

	for( i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		UIBaseObject *theObject = *i;

		if( theObject->IsA( TUIPage ) )
		{
			UIPage * const thePage = static_cast<UIPage *>( theObject );

			if( thePage->IsVisible() && thePage->GetsInput() && thePage->IsEnabled() )
			{
				UIButton *theButton = thePage->FindDefaultButton(useFocusedChild);

				if( theButton )
					return theButton;
			}
		}
	}

	return 0;
}

//======================================================================================

UIButton *UIPage::FindCancelButton (bool useFocusedChild)
{
	if( useFocusedChild && mWidgetWithKeyboardFocus && mWidgetWithKeyboardFocus->IsA( TUIPage ) )
	{
		UIButton * const button = static_cast<UIPage *>( mWidgetWithKeyboardFocus )->FindCancelButton(useFocusedChild);
		if (button)
			return button;
	}

	UIObjectList::iterator i;

	for( i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		UIBaseObject *theObject = *i;

		if( theObject->IsA( TUIButton ) )
		{
			UIButton *theButton = static_cast<UIButton *>( theObject );

			if( theButton->IsCancelButton() && theButton->IsVisible() && theButton->GetsInput() && theButton->IsEnabled() )
				return theButton;
		}
	}

	for( i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		UIBaseObject *theObject = *i;

		if( theObject->IsA( TUIPage ) )
		{
			UIPage *thePage = static_cast<UIPage *>( theObject );

			if( thePage->IsVisible() && thePage->GetsInput() && thePage->IsEnabled() )
			{
				UIButton *theButton = thePage->FindCancelButton(useFocusedChild);

				if( theButton )
					return theButton;
			}
		}
	}

	return 0;
}

//-----------------------------------------------------------------

void	UIPage::ResetLocalizedStrings (void)
{
	for( UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i )
	{
		(*i)->ResetLocalizedStrings ();
	}
}

//-----------------------------------------------------------------

void UIPage::SetScrollExtent( const UISize & newSize)
{
	UISize oldExtent;
	GetScrollExtent (oldExtent);
	UIWidget::SetScrollExtent (newSize);
	UISize extent;
	GetScrollExtent (extent);
	if (extent != oldExtent)
		SetPackDirty (true);
}

//----------------------------------------------------------------------

void UIPage::SetUnderMouse( bool b)
{
	UIWidget::SetUnderMouse (b);
	
	if (!b)
	{
		if (!IsEnabled () || !WillDraw () || !GetsInput ())
			UI_IGNORE_RETURN(UIWidget::SetAttribute(BF_UnderMouseUpdated, true));

		const UIObjectList::iterator end = mObjects->end ();
		for (UIObjectList::iterator it   = mObjects->begin(); it != end; ++it)
		{
			UIBaseObject * const obj = *it;

			if (obj->IsA (TUIWidget))
				static_cast<UIWidget *>(obj)->SetUnderMouse (false);
		}
	}
}

//----------------------------------------------------------------------

void UIPage::UpdateUnderMouse (const UIPoint & pt)
{
	if (HasPageAttribute(PA_PackDirty))
		Pack ();

	UIWidget::SetUnderMouse (true);

	bool found = false;

	const UIObjectList::iterator end = mObjects->end ();
	for (UIObjectList::iterator it = mObjects->begin(); it != end; ++it )
	{
		UIBaseObject * const obj = *it;

		if (obj->IsA (TUIWidget))
		{
			UIWidget * const widget = static_cast<UIWidget *>(obj);

			if (widget->HasAttribute(BF_UnderMouseUpdated))
				continue;

			if (found || !widget->WillDraw () || !widget->IsEnabled () || !widget->GetsInput ())
			{
				widget->SetUnderMouse (false);
			}
			else
			{
				const UIPoint translated (pt + GetScrollLocation () - widget->GetLocation ());
				
				if (!widget->HitTest (translated))
					widget->SetUnderMouse (false);
				else
				{				
					found = found || widget->IsAbsorbsInput ();
					
					if (widget->IsA (TUIPage))
						static_cast<UIPage *>(widget)->UpdateUnderMouse (translated);
					else
						widget->SetUnderMouse (true);
				}
			}
		}
	}
}

//-----------------------------------------------------------------

void UIPage::SetPackDirty (bool b) const
{
	UI_IGNORE_RETURN(SetPageAttribute(PA_PackDirty, b));
}

//----------------------------------------------------------------------

void UIPage::Clear ()
{
	UIObjectList listCopy (*mObjects);

	for (UIObjectList::const_iterator it = listCopy.begin (); it != listCopy.end (); ++it)
		UI_IGNORE_RETURN(RemoveChild (*it));
}

//----------------------------------------------------------------------

UIPage * UIPage::DuplicateInto          (UIPage & parent, const char * const path)
{
	UIPage * const page = static_cast<UIPage *>(parent.GetObjectFromPath (path, TUIPage));
	if (page)
	{
		UIPage * const dupe = static_cast<UIPage *>(page->DuplicateObject ());
		assert (dupe); //lint !e1776 //msvc bug
		UI_IGNORE_RETURN (parent.AddChild (dupe));
		UI_IGNORE_RETURN (parent.MoveChild (dupe, UIBaseObject::Top));
		dupe->Link();
		dupe->Pack();
		return dupe;
	}
	else
	{
		UI_REPORT_LOG_PRINT_ALWAYS(("UIPage::DuplicateInto - Can't find the page at %s.", path));
	}

	return 0;
}

//----------------------------------------------------------------------

bool UIPage::SetPageAttribute(PageAttributeFlags attribute, bool value) const
{
	const int bit = static_cast<int>(attribute);
	
	if( value )
	{
		if ((mPageAttributeFlags & bit) == 0)
		{
			mPageAttributeFlags |=  static_cast<int>(attribute);
			return true;
		}
	}
	else
	{
		if ((mPageAttributeFlags & bit) != 0)
		{
			mPageAttributeFlags &= ~static_cast<int>(attribute);
			return true;
		}
	}
	
	return false;
}

//----------------------------------------------------------------------

const UIBaseObject::UIObjectList & UIPage::GetChildrenRef () const
{
	return *mObjects;
}

//----------------------------------------------------------------------

UIPage::UIWidgetVector & UIPage::GetWidgetVector () const
{
	if (HasPageAttribute(PA_WidgetVectorDirty))
		BuildWidgetVector();
	
	return *mWidgets;
} //lint !e1763 //indirectly modifies

//----------------------------------------------------------------------

const UIWidget * UIPage::GetWidgetWithMouseLock() const
{
	return mWidgetWithMouseLock;
}

//----------------------------------------------------------------------

void UIPage::SetDoNotPackChildren(bool bSet) const
{
	UI_IGNORE_RETURN(SetPageAttribute(PA_DoNotPackChildren, bSet));
}

//----------------------------------------------------------------------

bool UIPage::GetDoNotPackChildren() const
{
	return HasPageAttribute(PA_DoNotPackChildren);
}


//----------------------------------------------------------------------

UIBaseObject * UIPage::GetCodeDataObject(const UIData * theData, UITypeID id, const char * name, bool ) const
{
	if (theData == NULL)
	{
		UI_REPORT_LOG_PRINT_ALWAYS(("Missing the data command."));
		return NULL;
	}
	
	if (name == NULL)
	{
		UI_REPORT_LOG_PRINT_ALWAYS(("Missing the name parameter."));
		return NULL;
	}
	
	UIBaseObject * result = 0;
	
	UINarrowString path;
	
	if (theData->GetPropertyNarrow(UILowerString(name), path))
	{
		result = GetObjectFromPath(path.c_str());
		
		if (!result)
		{
			UI_REPORT_LOG_PRINT_ALWAYS(("Can't find the object from the path."));
			return 0;
		}
		
		if (!result->IsA(id))
		{
			UI_REPORT_LOG_PRINT_ALWAYS(("Ids don't match."));
			result = 0;
		}
	}
	else
	{
		result = GetChild(name);
	}
	
	return result;
}

//----------------------------------------------------------------------

void UIPage::OnSizeChanged(UISize const & newSize, UISize const & oldSize)
{
	UI_IGNORE_RETURN(SetPageAttribute(PA_PackDirty, true));
	UIWidget::OnSizeChanged(newSize, oldSize);
}

//----------------------------------------------------------------------

void UIPage::WrapChildren()
{
	if (IsUserModifying()) 
	{
		SetPackDirty(true);
	}
	else
	{
		if (GetShrinkWrap()) 
		{
			// Pack all children.
			{
				for( UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i )
				{	
					UIPage * const page = UI_ASOBJECT(UIPage, *i);
					if (page)
					{
						page->Pack();
					}
				}
			}
			
			// Pack self.
			if(!HasPageAttribute(PA_DoNotPackChildren))
			{
				for (UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i)
				{	
					UIWidget * const theWidget = UI_ASOBJECT(UIWidget, *i);
					
					if (theWidget)
					{
						ApplyPacking(theWidget);
					}
				}
			}
			
			// Now wrap around it.
			UIWidget::WrapChildren();
		}
	}
}

//----------------------------------------------------------------------

UIBaseObject * UIPage::GetPreviousChild(UIBaseObject const * const child) const
{
	if (mObjects->size() > 1) 
	{
		for (UIBaseObject::UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i)
		{
			if(*i == child)
			{
				if (i == mObjects->begin())
				{
					return NULL;
				}
				else
				{
					return *(--i);
				}
			}
		}
	}

	return NULL;
}

//----------------------------------------------------------------------

UIBaseObject * UIPage::GetNextChild(UIBaseObject const * const child) const
{
	if (mObjects->size() > 1) 
	{
		for (UIBaseObject::UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i)
		{
			if(*i == child)
			{
				if (*i == mObjects->back())
				{
					return NULL;
				}
				else
				{
					return *(++i);
				}
			}
		}
	}
	
	return NULL;
}

//----------------------------------------------------------------------

void UIPage::SetTextStyle(UITextStyle const * const style, bool const overrideBold)
{
	bool parentIsSetDirty = false;

	for (UIBaseObject::UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i)
	{
		UIBaseObject * obj = *i;
		if (obj && obj->IsA(TUIWidget))
		{
			UIText * text = UI_ASOBJECT(UIText, obj);
			if (text != NULL && style != text->GetTextStyle()) 
			{
				if (!parentIsSetDirty) 
				{
					UIPage * const parentPage = UI_ASOBJECT(UIPage, text->GetParent());
					if (parentPage) 
					{
						parentPage->SetPackDirty(true);
					}
				}

				text->SetStyle(const_cast<UITextStyle *>(style));
			}
			else
			{
				UIPage * const page = UI_ASOBJECT(UIPage, obj);
				if (page)
				{
					page->SetTextStyle(style, overrideBold);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void UIPage::ApplyPackingToChildWidgets()
{
	if (!HasPageAttribute(PA_DoNotPackChildren))
	{
		bool const ignoreHidden = HasPageAttribute(PA_PackIgnoresHiddenObjects);

		for (UIObjectList::iterator i = mObjects->begin(); i != mObjects->end(); ++i)
		{	
			UIWidget * const theWidget = UI_ASOBJECT(UIWidget, *i);
			if (theWidget && (!ignoreHidden || theWidget->IsVisible()))
			{
				UIPage * const page = UI_ASOBJECT(UIPage, theWidget);
				if (page)
				{
					page->SetSizeToParentSize();
				}

				ApplyPacking(theWidget);
			}
		}
	}
}

//----------------------------------------------------------------------

UIData const * UIPage::GetCodeData() const
{
	return mCodeData;
}

//----------------------------------------------------------------------

void UIPage::SetSizeToParentSize()
{
	if (HasPageAttribute(PA_ParentSize)) 
	{
		UIWidget const * const parent = UI_ASOBJECT(UIWidget, GetParent());
		if (parent)
		{
			SetMaximumSize(parent->GetSize());
			SetMinimumSize(parent->GetSize());
			SetMaximumSize(parent->GetSize());
			SetScrollExtent(parent->GetSize());
			SetSize(parent->GetSize());
		}
	}
}

//----------------------------------------------------------------------

void UIPage::GetVisibleChildren(UISmartObjectVector & wv)
{
	UIObjectList const & olist = GetChildrenRef();

	wv.clear();
	wv.reserve(olist.size ());
		
	for (UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const obj = *it;
		
		if (obj->IsA(TUIWidget))
		{
			UIWidget * const wid = static_cast<UIWidget *>(obj);
			
			if (wid->WillDraw())
				wv.push_back(UIBaseObjectPointer(obj));
		}
	}
}

//-----------------------------------------------------------------
