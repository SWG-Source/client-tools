#include "_precompile.h"
#include "UIWidget.h"

#include "UIButton.h"
#include "UICanvas.h"
#include "UIClock.h"
#include "UICursor.h"
#include "UICursorSet.h"
#include "UIDeformer.h"
#include "UIEffector.h"
#include "UIEventCallback.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPacking.h"
#include "UIPage.h"
#include "UIPalette.h"
#include "UIPopupMenuStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIRectangleStyle.h"
#include "UIScriptEngine.h"
#include "UITextStyle.h"
#include "UITooltipStyle.h"
#include "UIUtils.h"
#include "UIWatcher.h"
#include "UIWidgetBoundaries.h"
#include "UIWidgetRectangleStyles.h"
#include <cassert>
#include <list>
#include <vector>

const char * const UIWidget::TypeName                       = "Widget";

const UILowerString UIWidget::PropertyName::AbsorbsInput            = UILowerString ("AbsorbsInput");
const UILowerString UIWidget::PropertyName::AbsorbsTab              = UILowerString ("AbsorbsTab");
const UILowerString UIWidget::PropertyName::AcceptsMoveFromChildren = UILowerString ("AcceptsMoveFromChildren");
const UILowerString UIWidget::PropertyName::Activated               = UILowerString ("Activated");
const UILowerString UIWidget::PropertyName::AutoRegister            = UILowerString ("AutoRegister");
const UILowerString UIWidget::PropertyName::BackgroundColor         = UILowerString ("BackgroundColor");
const UILowerString UIWidget::PropertyName::BackgroundColorA        = UILowerString ("BackgroundColorA");
const UILowerString UIWidget::PropertyName::BackgroundColorB        = UILowerString ("BackgroundColorB");
const UILowerString UIWidget::PropertyName::BackgroundColorG        = UILowerString ("BackgroundColorG");
const UILowerString UIWidget::PropertyName::BackgroundColorR        = UILowerString ("BackgroundColorR");
const UILowerString UIWidget::PropertyName::BackgroundOpacity       = UILowerString ("BackgroundOpacity");
const UILowerString UIWidget::PropertyName::BackgroundScrolls       = UILowerString ("BackgroundScrolls");
const UILowerString UIWidget::PropertyName::BackgroundTint          = UILowerString ("BackgroundTint");
const UILowerString UIWidget::PropertyName::BackgroundTintA         = UILowerString ("BackgroundTintA");
const UILowerString UIWidget::PropertyName::BackgroundTintB         = UILowerString ("BackgroundTintB");
const UILowerString UIWidget::PropertyName::BackgroundTintG         = UILowerString ("BackgroundTintG");
const UILowerString UIWidget::PropertyName::BackgroundTintR         = UILowerString ("BackgroundTintR");
const UILowerString UIWidget::PropertyName::Color                   = UILowerString ("Color");
const UILowerString UIWidget::PropertyName::ColorB                  = UILowerString ("ColorB");
const UILowerString UIWidget::PropertyName::ColorG                  = UILowerString ("ColorG");
const UILowerString UIWidget::PropertyName::ColorR                  = UILowerString ("ColorR");
const UILowerString UIWidget::PropertyName::ContextCapable          = UILowerString ("ContextCapable");
const UILowerString UIWidget::PropertyName::ContextCapableAlternate = UILowerString ("ContextCapableAlternate");
const UILowerString UIWidget::PropertyName::ContextToParent         = UILowerString ("ContextToParent");
const UILowerString UIWidget::PropertyName::Cursor                  = UILowerString ("Cursor");
const UILowerString UIWidget::PropertyName::CursorSet               = UILowerString ("CursorSet");
const UILowerString UIWidget::PropertyName::CustomDragWidget        = UILowerString ("CustomDragWidget");
const UILowerString UIWidget::PropertyName::DragAccepts             = UILowerString ("DragAccepts");
const UILowerString UIWidget::PropertyName::DragBadCursor           = UILowerString ("DragBadCursor");
const UILowerString UIWidget::PropertyName::DragGoodCursor          = UILowerString ("DragGoodCursor");
const UILowerString UIWidget::PropertyName::DragType                = UILowerString ("DragType");
const UILowerString UIWidget::PropertyName::Dragable                = UILowerString ("Dragable");
const UILowerString UIWidget::PropertyName::DropToParent          = UILowerString ("DropToParent");
const UILowerString UIWidget::PropertyName::Enabled               = UILowerString ("Enabled");
const UILowerString UIWidget::PropertyName::Focus                 = UILowerString ("Focus");
const UILowerString UIWidget::PropertyName::ForwardMoveToParent   = UILowerString ("ForwardMoveToParent");
const UILowerString UIWidget::PropertyName::GetsInput             = UILowerString ("GetsInput");
const UILowerString UIWidget::PropertyName::LocalTooltip          = UILowerString ("LocalTooltip");
const UILowerString UIWidget::PropertyName::Location              = UILowerString ("Location");
const UILowerString UIWidget::PropertyName::LocationX             = UILowerString ("LocationX");
const UILowerString UIWidget::PropertyName::LocationY             = UILowerString ("LocationY");
const UILowerString UIWidget::PropertyName::LockDiagonal          = UILowerString ("LockDiagonal");
const UILowerString UIWidget::PropertyName::MaximumSize           = UILowerString ("MaximumSize");
const UILowerString UIWidget::PropertyName::MaximumSizeX          = UILowerString ("MaximumSizeX");
const UILowerString UIWidget::PropertyName::MaximumSizeY          = UILowerString ("MaximumSizeY");
const UILowerString UIWidget::PropertyName::MinimumScrollExtent   = UILowerString ("MinimumScrollExtent");
const UILowerString UIWidget::PropertyName::MinimumSize           = UILowerString ("MinimumSize");
const UILowerString UIWidget::PropertyName::MinimumSizeX          = UILowerString ("MinimumSizeX");
const UILowerString UIWidget::PropertyName::MinimumSizeY          = UILowerString ("MinimumSizeY");
const UILowerString UIWidget::PropertyName::OnActivate            = UILowerString ("OnActivate");
const UILowerString UIWidget::PropertyName::OnActivateEffector    = UILowerString ("OnActivateEffector");
const UILowerString UIWidget::PropertyName::OnDeactivate          = UILowerString ("OnDeactivate");
const UILowerString UIWidget::PropertyName::OnDeactivateEffector  = UILowerString ("OnDeactivateEffector");
const UILowerString UIWidget::PropertyName::OnDisable             = UILowerString ("OnDisable");
const UILowerString UIWidget::PropertyName::OnDisableEffector     = UILowerString ("OnDisableEffector");
const UILowerString UIWidget::PropertyName::OnEnable              = UILowerString ("OnEnable");
const UILowerString UIWidget::PropertyName::OnEnableEffector      = UILowerString ("OnEnableEffector");
const UILowerString UIWidget::PropertyName::OnHide                = UILowerString ("OnHide");
const UILowerString UIWidget::PropertyName::OnHideEffector        = UILowerString ("OnHideEffector");
const UILowerString UIWidget::PropertyName::OnHoverIn             = UILowerString ("OnHoverIn");
const UILowerString UIWidget::PropertyName::OnHoverInEffector     = UILowerString ("OnHoverInEffector");
const UILowerString UIWidget::PropertyName::OnHoverOut            = UILowerString ("OnHoverOut");
const UILowerString UIWidget::PropertyName::OnHoverOutEffector    = UILowerString ("OnHoverOutEffector");
const UILowerString UIWidget::PropertyName::OnRunScript           = UILowerString ("OnRunScript");
const UILowerString UIWidget::PropertyName::OnShow                = UILowerString ("OnShow");
const UILowerString UIWidget::PropertyName::OnShowEffector        = UILowerString ("OnShowEffector");
const UILowerString UIWidget::PropertyName::OnSizeChanged         = UILowerString ("OnSizeChanged");
const UILowerString UIWidget::PropertyName::Opacity               = UILowerString ("Opacity");
const UILowerString UIWidget::PropertyName::OpacityRelativeMin    = UILowerString ("OpacityRelativeMin");
const UILowerString UIWidget::PropertyName::PackLocation          = UILowerString ("PackLocation");
const UILowerString UIWidget::PropertyName::PackSize              = UILowerString ("PackSize");
const UILowerString UIWidget::PropertyName::PalShade              = UILowerString ("PalShade");
const UILowerString UIWidget::PropertyName::PopupStyle            = UILowerString ("PopupStyle");
const UILowerString UIWidget::PropertyName::ResizeInset           = UILowerString ("ResizeInset");
const UILowerString UIWidget::PropertyName::Rotation              = UILowerString ("Rotation");
const UILowerString UIWidget::PropertyName::ScrollExtent          = UILowerString ("ScrollExtent");
const UILowerString UIWidget::PropertyName::ScrollLocation        = UILowerString ("ScrollLocation");
const UILowerString UIWidget::PropertyName::ScrollSizeLine        = UILowerString ("ScrollSizeLine");
const UILowerString UIWidget::PropertyName::ScrollSizePage        = UILowerString ("ScrollSizePage");
const UILowerString UIWidget::PropertyName::Selectable            = UILowerString ("Selectable");
const UILowerString UIWidget::PropertyName::Shear                 = UILowerString ("Shear");
const UILowerString UIWidget::PropertyName::ShrinkWrap            = UILowerString ("ShrinkWrap");
const UILowerString UIWidget::PropertyName::Size                  = UILowerString ("Size");
const UILowerString UIWidget::PropertyName::SizeIncrement         = UILowerString ("SizeIncrement");
const UILowerString UIWidget::PropertyName::SizeX                 = UILowerString ("SizeX");
const UILowerString UIWidget::PropertyName::SizeY                 = UILowerString ("SizeY");
const UILowerString UIWidget::PropertyName::TabRoot               = UILowerString ("TabRoot");
const UILowerString UIWidget::PropertyName::TextOpacityRelativeApply  = UILowerString ("TextOpacityRelativeApply");
const UILowerString UIWidget::PropertyName::TextOpacityRelativeMin  = UILowerString ("TextOpacityRelativeMin");
const UILowerString UIWidget::PropertyName::Tooltip               = UILowerString ("Tooltip");
const UILowerString UIWidget::PropertyName::TooltipDelay          = UILowerString ("TooltipDelay");
const UILowerString UIWidget::PropertyName::TooltipStyle          = UILowerString ("TooltipStyle");
const UILowerString UIWidget::PropertyName::UserDragScrollable    = UILowerString ("UserDragScrollable");
const UILowerString UIWidget::PropertyName::UserMovable           = UILowerString ("UserMovable");
const UILowerString UIWidget::PropertyName::UserResizable         = UILowerString ("UserResizable");
const UILowerString UIWidget::PropertyName::Visible               = UILowerString ("Visible");

const UILowerString UIWidget::MethodName::EffectorExecute         = UILowerString ("EffectorExecute");
const UILowerString UIWidget::MethodName::EffectorCancel          = UILowerString ("EffectorCancel");
const UILowerString UIWidget::MethodName::RunScript               = UILowerString ("RunScript");

const UILowerString UIWidget::CategoryName::Basics                 = UILowerString ("Basics");
const UILowerString UIWidget::CategoryName::Appearance             = UILowerString ("Appearance");
const UILowerString UIWidget::CategoryName::AdvancedAppearance     = UILowerString ("Advanced Appearance");
const UILowerString UIWidget::CategoryName::Behavior               = UILowerString ("Behavior");
const UILowerString UIWidget::CategoryName::AdvancedBehavior       = UILowerString ("Advanced Behavior");

typedef UIWidget::AttributeBitFlags AttributeBitFlags;
typedef UIWidget::VisualState VisualState;

//----------------------------------------------------------------------
#define _TYPENAME UIWidget

namespace UIWidgetNamespace
{
	const long RESIZE_MARGIN = 8;

	UIString const s_emptyString;

	bool s_hasDragMovedYet = false;

	//-----------------------------------------------------------------
/*
	else if(categoryName == CategoryName::Appearance)
	{		
		UIPalette::GetPropertyNamesForType (TUIWidget, In);
		/////
		mRectangleStyles->GetPropertyNames (In);
	}
*/
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(AutoRegister,"false",T_string),
		_DESCRIPTOR(Location,"",T_point),
		_DESCRIPTOR_FLAGS(LocationX,"",T_int,F_SUB_PROPERTY),
		_DESCRIPTOR_FLAGS(LocationY,"",T_int,F_SUB_PROPERTY),
		_DESCRIPTOR(LockDiagonal,"",T_bool),
		_DESCRIPTOR(MaximumSize,"",T_string),
		_DESCRIPTOR(MinimumScrollExtent,"",T_string),
		_DESCRIPTOR(MinimumSize,"",T_string),
		_DESCRIPTOR(PackLocation,"",T_string),
		_DESCRIPTOR(PackSize,"",T_string),
		_DESCRIPTOR(ResizeInset,"",T_int),
		_DESCRIPTOR(Rotation,"",T_string),
		_DESCRIPTOR(ScrollExtent,"",T_string),
		_DESCRIPTOR(ScrollSizeLine,"",T_string),
		_DESCRIPTOR(ScrollSizePage,"",T_string),
		_DESCRIPTOR(Size,"",T_string),
		_DESCRIPTOR(SizeIncrement,"",T_string)
	_GROUPEND(Basic, 1, 0);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(BackgroundColor,"",T_color),
		_DESCRIPTOR(BackgroundOpacity,"",T_string),
		_DESCRIPTOR(BackgroundScrolls,"",T_string),
		_DESCRIPTOR(BackgroundTint,"",T_color),
		_DESCRIPTOR(Color,"",T_color),
		_DESCRIPTOR(Enabled,"",T_string),
		_DESCRIPTOR(Opacity,"",T_string),
		_DESCRIPTOR(OpacityRelativeMin,"",T_string),
		_DESCRIPTOR(TextOpacityRelativeApply,"",T_string),
		_DESCRIPTOR(TextOpacityRelativeMin,"",T_string),
		_DESCRIPTOR(Tooltip,"",T_string),
		_DESCRIPTOR(TooltipStyle,"",T_object),
		_DESCRIPTOR(TooltipDelay,"",T_string),
		_DESCRIPTOR(Visible,"",T_string),
		_DESCRIPTOR(PalShade,"",T_float),
	_GROUPEND(Appearance, 1, 1);
	//================================================================

	//================================================================
	// Behavior category
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(AbsorbsInput,"",T_string),
		_DESCRIPTOR(Activated,"",T_string),
		_DESCRIPTOR(Selectable,"",T_string),
		_DESCRIPTOR(UserDragScrollable,"",T_string),
		_DESCRIPTOR(UserMovable,"",T_string),
		_DESCRIPTOR(UserResizable,"",T_string),
		_DESCRIPTOR(GetsInput,"",T_string)
	_GROUPEND(Behavior, 1, 2);
	//================================================================

	//================================================================
	// AdvancedBehavior category
	_GROUPBEGIN(AdvancedBehavior)
		_DESCRIPTOR(ContextCapable,"",T_string),
		_DESCRIPTOR(ContextCapableAlternate,"",T_string),
		_DESCRIPTOR(ContextToParent,"",T_string),
		_DESCRIPTOR(DragAccepts,"",T_string),
		_DESCRIPTOR(DragType,"",T_string),
		_DESCRIPTOR(Dragable,"",T_string),
		_DESCRIPTOR(DropToParent,"",T_string),
		_DESCRIPTOR(AbsorbsTab,"",T_string),
		_DESCRIPTOR(OnActivate,"",T_string),
		_DESCRIPTOR(OnActivateEffector,"",T_string),
		_DESCRIPTOR(OnDeactivate,"",T_string),
		_DESCRIPTOR(OnDeactivateEffector,"",T_string),
		_DESCRIPTOR(OnDisable,"",T_string),
		_DESCRIPTOR(OnDisableEffector,"",T_string),
		_DESCRIPTOR(OnEnable,"",T_string),
		_DESCRIPTOR(OnEnableEffector,"",T_string),
		_DESCRIPTOR(OnHide,"",T_string),
		_DESCRIPTOR(OnHideEffector,"",T_string),
		_DESCRIPTOR(OnHoverIn,"",T_string),
		_DESCRIPTOR(OnHoverInEffector,"",T_string),
		_DESCRIPTOR(OnHoverOut,"",T_string),
		_DESCRIPTOR(OnShow,"",T_string),
		_DESCRIPTOR(OnShowEffector,"",T_string),
		_DESCRIPTOR(AcceptsMoveFromChildren,"",T_bool),
		_DESCRIPTOR(ForwardMoveToParent,"",T_bool)
	_GROUPEND(AdvancedBehavior, 1, 3);
	//================================================================

	//================================================================
	// AdvancedAppearance category
	_GROUPBEGIN(AdvancedAppearance)
		_DESCRIPTOR(LocalTooltip,"",T_string),
		_DESCRIPTOR(TabRoot,"",T_string),
		_DESCRIPTOR(Shear,"",T_string),
		_DESCRIPTOR(ShrinkWrap,"",T_string),
		_DESCRIPTOR(CustomDragWidget,"",T_string),
		_DESCRIPTOR(DragBadCursor,"",T_object),
		_DESCRIPTOR(DragGoodCursor,"",T_object),
		_DESCRIPTOR(Cursor,"",T_object),
		_DESCRIPTOR(CursorSet,"",T_object),
		_DESCRIPTOR(PopupStyle,"",T_object)
	_GROUPEND(AdvancedAppearance, 1, 4);
	//================================================================
}

using namespace UIWidgetNamespace;

//-----------------------------------------------------------------

UIWidget::UIWidget () :
UIBaseObject         (),
mOpacity             (1.0f),
mOpacityRelativeMin  (0.0f),
mTextOpacityRelativeMin  (0.0f),
mColor               (0xFF,0xFF,0xFF),
mLocation            (),
mSize                (),
mScrollLocation      (),
mScrollExtent        (),
mRotation            (0.0f),
mCursor              (0),
mDragBadCursor       (0),
mDragGoodCursor      (0),
mTooltip(NULL),
mLocalTooltip(NULL),
mTooltipStyle        (0),
mCallbacks           (0),
mAttributeBits       (static_cast<int>(BF_Visible) | static_cast<int>(BF_Enabled) | static_cast<int>(BF_GetsInput) | static_cast<int>(BF_DropFlagOk) | static_cast<int>(BF_AbsorbsInput) | static_cast<int>(BF_BackgroundScrolls) | static_cast<int>(BF_TooltipDelay)),
mBackgroundColor     (0x00, 0x00, 0x00, 0x00),
mBackgroundTint      (0xff, 0xff, 0xff, 0x00),
mRectangleStyles     (0),
mMinimumSize         (0, 0),
mMaximumSize         (16384L, 16384L),
mMinimumScrollExtent (0, 0),
mSizeIncrement       (1L,1L),
mCurrentUserModificationType     (UMT_NONE),
mUserModificationStartPoint      (),
mUserModificationStartWidgetRect (),
mCursorSet           (0),
mBoundaries          (0),
mCustomDragWidget    (0),
mPopupStyle          (0),
mScrollSizePage      (0L,0L),
mScrollSizeLine      (1L,1L),
mDeformer(NULL),
mTooltipCallback(NULL),
mDepthOverride(false),
mDepth(1.0f),
mAutoRegister(false),
mPalShade(1.0f),
mLockDiagonal(false),
mResizeInset(0),
mNotModifyingUseDefaultCursor(0),
mForwardMoveToParent(false),
mAcceptsMoveFromChildren(false)
{
	mRectangleStyles = new UIWidgetRectangleStyles;
	mPackSizes [0] = mPackSizes [1] = 0;
	mPackLocations [0] = mPackLocations [1] = 0;
}

//-----------------------------------------------------------------

UIWidget::~UIWidget ()
{
	SetMouseCursor      (0);
	SetDragGoodCursor   (0);
	SetDragBadCursor    (0);
	SetTooltipStyle     (0);
	SetCustomDragWidget (0);
	SetPopupStyle       (0);

	mCursor = 0;
	mDragBadCursor = 0;
	mDragGoodCursor = 0;
	mTooltipStyle = 0;

	for (size_t i = 0; i < 2; ++i)
	{
		if (mPackSizes [i])
		{
			delete mPackSizes [i];
			mPackSizes [i] = 0;
		}
		if (mPackLocations [i])
		{
			delete mPackLocations [i];
			mPackLocations [i] = 0;
		}
	}

	delete mRectangleStyles;
	mRectangleStyles = 0;

	delete mCallbacks;
	mCallbacks = 0;

	SetCursorSet (0);
	mCursorSet = 0;

	delete mBoundaries;
	mBoundaries = 0;

	delete mDeformer;
	mDeformer = 0;

	delete mTooltip;
	mTooltip = 0;

	delete mLocalTooltip;
	mLocalTooltip = 0;
}

//-----------------------------------------------------------------

void UIWidget::AddCallback( UIEventCallback *NewCallback )
{
	DEBUG_DESTROYED();
	

	if (!mCallbacks)
		mCallbacks = new EventCallbackList;

	mCallbacks->push_back( NewCallback );
}

//-----------------------------------------------------------------

void UIWidget::RemoveCallback( UIEventCallback *OldCallback )
{
	DEBUG_DESTROYED();
	
	if (!mCallbacks)
		return;

	for( EventCallbackList::iterator i = mCallbacks->begin(); i != mCallbacks->end(); ++i )
	{
		if( *i == OldCallback )
		{
			mCallbacks->erase(i);
			break;
		}
	}

	if (mCallbacks->empty ())
	{
		delete mCallbacks;
		mCallbacks = 0;
	}
}

//----------------------------------------------------------------------

bool UIWidget::HasCallback    (UIEventCallback * callback) const
{
	DEBUG_DESTROYED();
	
	return mCallbacks && (std::find (mCallbacks->begin (), mCallbacks->end (), callback) != mCallbacks->end ());
}

//-----------------------------------------------------------------

void UIWidget::SetRect( const UIRect &NewRect )
{
	DEBUG_DESTROYED();
	
	UISize NewSize (NewRect.Size ());
	SetSize(NewSize);
	SetLocation(NewRect.left, NewRect.top);
}

//-----------------------------------------------------------------

void UIWidget::GetWorldRect( UIRect &Out ) const
{
	DEBUG_DESTROYED();
	
	const UIBaseObject *Parent = GetParent();

	GetRect( Out );

	while( Parent )
	{
		if( Parent->IsA( TUIWidget ) )
		{
			const UIWidget *w = static_cast<const UIWidget *>( Parent );
			Out += w->GetLocation();
			Out -= w->GetScrollLocation();
		}
		Parent = Parent->GetParent();
	}
}

//-----------------------------------------------------------------

UIRect UIWidget::GetWorldRect () const
{
	DEBUG_DESTROYED();
	
	UIRect rc;

	GetWorldRect( rc );
	return rc;
}

//-----------------------------------------------------------------

void UIWidget::GetWorldLocation( UIPoint &Out ) const
{
	DEBUG_DESTROYED();
	
	const UIBaseObject *Parent = GetParent();

	Out = GetLocation();

	while( Parent )
	{
		if( Parent->IsA( TUIWidget ) )
		{
			Out += static_cast<const UIWidget *>( Parent )->GetLocation();
			Out -= static_cast<const UIWidget *>( Parent )->GetScrollLocation();
		}
		Parent = Parent->GetParent();
	}
}

//----------------------------------------------------------------------

UIPoint  UIWidget::GetLocationRelativeTo     (const UIWidget & otherWidget) const
{
	DEBUG_DESTROYED();
	
	const UIPoint & myWorldLocation    = GetWorldLocation ();
	const UIPoint & otherWorldLocation = otherWidget.GetWorldLocation ();

	return myWorldLocation - otherWorldLocation;
}

//-----------------------------------------------------------------

UIPoint	UIWidget::GetWorldLocation () const
{
	DEBUG_DESTROYED();
	
	UIPoint pt;

	GetWorldLocation( pt );
	return pt;
}

//-----------------------------------------------------------------

void UIWidget::SetLocation( UIScalar const x, UIScalar const y, bool const center )
{
	DEBUG_DESTROYED();
	
	UIPoint newLocation;
	
	if (center)
	{
		newLocation = UIPoint(x,y) - (GetSize() / 2);
	}
	else
	{
		newLocation.Set(x, y);
	}


	if (newLocation != mLocation)
	{
		UIPoint const oldLocation(mLocation);
		mLocation = newLocation;
		ResetPackLocationInfo();
		OnLocationChanged(newLocation, oldLocation);
	}
}

//-----------------------------------------------------------------

void UIWidget::SetSize(UISize const & NewSize)
{
	DEBUG_DESTROYED();
	
	UISize DesiredSize (std::min (mMaximumSize.x, std::max (mMinimumSize.x, NewSize.x)),
						std::min (mMaximumSize.y, std::max (mMinimumSize.y, NewSize.y)));

	//-- handle diagonal lock here
	if(mLockDiagonal && (DesiredSize.x != DesiredSize.y))
	{
		if(DesiredSize.x == mSize.x) // only changed y, set x to y
		{
			DesiredSize.x = DesiredSize.y;
		}
		else if(DesiredSize.y == mSize.y) // only changed x, set y to x
		{
			DesiredSize.y = DesiredSize.x;
		}
		else if((DesiredSize.x > mSize.x) && (DesiredSize.y > mSize.y)) // both got bigger, use min increment
		{
			DesiredSize.x = std::min(DesiredSize.x, DesiredSize.y);
			DesiredSize.y = DesiredSize.x;
		}
		else if((DesiredSize.x < mSize.x) && (DesiredSize.y < mSize.y)) // both got smaller, use min increment
		{
			DesiredSize.x = std::max(DesiredSize.x, DesiredSize.y);
			DesiredSize.y = DesiredSize.x;
		}
		else // A random change that doesn't seem to correspond to anything; use the larger change
		{
			int xdist = DesiredSize.x - mSize.x;
			if(xdist < 0) xdist = -xdist;
			int ydist = DesiredSize.y - mSize.y;
			if(ydist < 0) ydist = -ydist;
			if(xdist > ydist)
				DesiredSize.y = DesiredSize.x;
			else
				DesiredSize.x = DesiredSize.y;
		}
	}
	
	//-- handle discrete sizing increments here
	if (mSizeIncrement.x > 1)
		DesiredSize.x -= (DesiredSize.x - mSize.x) % mSizeIncrement.x;

	if (mSizeIncrement.y > 1)
		DesiredSize.y -= (DesiredSize.y - mSize.y) % mSizeIncrement.y;

	if (DesiredSize != mSize)
	{
		UISize const oldSize(mSize);
		mSize = DesiredSize;

		SetScrollExtent (mSize);
		ResetPackSizeInfo ();

		if (mBoundaries)
			mBoundaries->onSizeChange (oldSize, mSize);

		// Internal.
		OnSizeChanged(DesiredSize, oldSize);

		// External.
		SendCallback(&UIEventCallback::OnSizeChanged, PropertyName::OnSizeChanged);
	}
}

//-----------------------------------------------------------------

void UIWidget::SetWidth( const long NewWidth )
{
	DEBUG_DESTROYED();
	
	SetSize ( UISize (NewWidth, mSize.y));
}

//-----------------------------------------------------------------

void UIWidget::SetHeight( const long NewHeight )
{
	DEBUG_DESTROYED();
	
	SetSize ( UISize (mSize.x, NewHeight));
}

//----------------------------------------------------------------------

void UIWidget::SetCustomDragWidget (UIWidget * widget)
{
	DEBUG_DESTROYED();
	
	AttachMember (mCustomDragWidget, widget);
}

//-----------------------------------------------------------------

void UIWidget::SetMouseCursor( UICursor *NewCursor )
{
	DEBUG_DESTROYED();
	
	AttachMember (mCursor, NewCursor);
}

//-----------------------------------------------------------------

void UIWidget::SetDragGoodCursor( UICursor *NewCursor )
{
	DEBUG_DESTROYED();
	
	AttachMember (mDragGoodCursor, NewCursor);
}

//-----------------------------------------------------------------

void UIWidget::SetDragBadCursor( UICursor *NewCursor )
{
	DEBUG_DESTROYED();
	
	AttachMember (mDragBadCursor, NewCursor);
}

//-----------------------------------------------------------------

void UIWidget::SetTooltip(const UIString &NewTooltip, bool const setLocalTooltip)
{
	DEBUG_DESTROYED();
	
	if (NewTooltip.empty()) 
	{
		delete mTooltip;
		mTooltip = 0;
	}
	else
	{
		if (mTooltip) 
		{
			*mTooltip = NewTooltip;
		}
		else
		{
			mTooltip = new UIString(NewTooltip);
		}
	}

	if (setLocalTooltip) 
	{
		UIString localToolTip;
		UIManager::gUIManager().CreateLocalizedString(GetTooltip(), localToolTip);
		SetLocalTooltip(localToolTip, false);
	}
}

//-----------------------------------------------------------------

void UIWidget::SetLocalTooltip(const UIString &NewTooltip, bool const setToolTip)
{
	DEBUG_DESTROYED();
	
	if (NewTooltip.empty()) 
	{
		delete mLocalTooltip;
		mLocalTooltip = 0;
	}
	else
	{
		if (mLocalTooltip) 
		{
			*mLocalTooltip = NewTooltip;
		}
		else
		{
			mLocalTooltip = new UIString(NewTooltip);
		}
	}

	if (setToolTip) 
	{
		SetTooltip(NewTooltip, false);
	}
}

//-----------------------------------------------------------------

void UIWidget::SetTooltipStyle( UITooltipStyle *NewStyle )
{
	DEBUG_DESTROYED();
	
	AttachMember (mTooltipStyle, NewStyle);
}

//-----------------------------------------------------------------

VisualState UIWidget::GetVisualState () const
{
	DEBUG_DESTROYED();
	
	if( !IsEnabled() )
		return Disabled;
	else if( IsSelected() )
		return Selected;
	else
		return Normal;
}

//-----------------------------------------------------------------

void UIWidget::SetEnabled( bool const NewEnabled )
{
	DEBUG_DESTROYED();
	
	if (!SetAttribute (BF_Enabled, NewEnabled))
		return;

	SetAttribute (BF_UnderMouseUpdated, false);

	if (NewEnabled)
	{
		CancelEffector( PropertyName::OnDisable );
		SendCallback( &UIEventCallback::OnEnable, PropertyName::OnEnable );
	}
	else
	{
		CancelEffector( PropertyName::OnEnable );
		SendCallback( &UIEventCallback::OnDisable, PropertyName::OnDisable );
		SetUnderMouse (false);

		if (IsA (TUIButton))
			SetActivated (false);
	}
}

//----------------------------------------------------------------------

void UIWidget::SetActivated (bool b)
{
	DEBUG_DESTROYED();
	
	if (!SetAttribute (BF_Activated, b))
		return;

	if (b)
	{
		CancelEffector( PropertyName::OnDeactivate );
		SendCallback( &UIEventCallback::OnActivate, PropertyName::OnActivate );
	}
	else
	{
		CancelEffector( PropertyName::OnActivate );
		SendCallback( &UIEventCallback::OnDeactivate, PropertyName::OnDeactivate );
	}
}

//-----------------------------------------------------------------

void UIWidget::SetVisible( bool const NewVisible )
{
	DEBUG_DESTROYED();
	
	if (!SetAttribute (BF_Visible, NewVisible))
		return;

	SetAttribute (BF_UnderMouseUpdated, false);

	if( NewVisible )
	{
		CancelEffector( PropertyName::OnHide );
		SendCallback( &UIEventCallback::OnShow, PropertyName::OnShow );
	}
	else
	{
		CancelEffector( PropertyName::OnShow );
		SendCallback( &UIEventCallback::OnHide, PropertyName::OnHide );
		SetUnderMouse (false);
	}

	UIPage * parentPage = UI_ASOBJECT(UIPage, GetParent());
	if (parentPage) 
	{
		parentPage->SetPackDirty(true);
	}
}

//-----------------------------------------------------------------

void UIWidget::ForceVisible( bool NewForceVisible )
{
	DEBUG_DESTROYED();
	
	if (SetAttribute (BF_ForceVisible, NewForceVisible))
		SetAttribute (BF_UnderMouseUpdated, false);
}

//-----------------------------------------------------------------

void UIWidget::SetSelected( const bool NewSelected )
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_Selected, NewSelected));
}

//----------------------------------------------------------------------

void UIWidget::SetSelectable ( const bool b)
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_Selectable, b));
}

//----------------------------------------------------------------------

void UIWidget::SetTabRoot ( const bool b)
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_TabRoot, b));
}

//----------------------------------------------------------------------

void UIWidget::SetUnderMouse( bool b)
{
	DEBUG_DESTROYED();
	
	if (!SetAttribute (BF_UnderMouse, b))
		return;

	if( b )
	{
		CancelEffector( PropertyName::OnHoverIn );
		SendCallback( &UIEventCallback::OnHoverIn, PropertyName::OnHoverIn );
	}
	else
	{
		CancelEffector( PropertyName::OnHoverOut );
		SendCallback( &UIEventCallback::OnHoverOut, PropertyName::OnHoverOut );
	}
}

//-----------------------------------------------------------------

void UIWidget::SetGetsInput( bool b )
{
	DEBUG_DESTROYED();
	
	if (SetAttribute (BF_GetsInput, b))
		SetAttribute (BF_UnderMouseUpdated, false);
}

//-----------------------------------------------------------------

void UIWidget::SetDragable( bool b )
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_Dragable, b));
}

//-----------------------------------------------------------------

void UIWidget::SetTransient( bool b )
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_Transient, b));
}

//----------------------------------------------------------------------

void UIWidget::SetDropToParent ( bool b )
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_DropToParent, b));
}

//----------------------------------------------------------------------

void UIWidget::SetContextToParent ( bool b )
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_ContextToParent, b));
}

//-----------------------------------------------------------------

void UIWidget::SetScrollLocation( const UIPoint &NewScroll )
{
	DEBUG_DESTROYED();
	
	mScrollLocation = NewScroll;
}

//-----------------------------------------------------------------

void UIWidget::SetDropFlagOk ( bool b)
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_DropFlagOk, b));
}

//----------------------------------------------------------------------

void UIWidget::SetUserMovable (bool b)
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_UserMovable, b));
}

//----------------------------------------------------------------------

void UIWidget::SetUserDragScrollable (bool b)
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_UserDragScrollable, b));
}

//----------------------------------------------------------------------

void UIWidget::SetAbsorbsInput (bool b)
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_AbsorbsInput, b));
}

//----------------------------------------------------------------------

void UIWidget::SetUserResizable (bool b)
{
	DEBUG_DESTROYED();
	
	UI_IGNORE_RETURN (SetAttribute (BF_UserResizable, b));
}

//----------------------------------------------------------------------

void UIWidget::SetContextCapable (bool b, bool primary)
{
	DEBUG_DESTROYED();
	
	if (primary)
		UI_IGNORE_RETURN (SetAttribute (BF_ContextCapable, b));
	else
		UI_IGNORE_RETURN (SetAttribute (BF_ContextCapableAlternate, b));
}

//-----------------------------------------------------------------

bool UIWidget::CanSelect () const
{
	DEBUG_DESTROYED();
	
	return WillDraw() && IsEnabled() && GetsInput ();
}

//-----------------------------------------------------------------

void UIWidget::SetFocus ()
{
	DEBUG_DESTROYED();
	
	if ( CanSelect () && GetParent() )
	{
		if( GetParent()->IsA( TUIWidget ) )
			static_cast<UIWidget *>( GetParent() )->SetFocus();

		GetParent()->SelectChild( this );
	}
}

//-----------------------------------------------------------------

void UIWidget::SetScrollExtent( const UISize &NewSize )
{
	DEBUG_DESTROYED();
	
	mScrollExtent.x = std::max (mSize.x, std::max (mMinimumScrollExtent.x, NewSize.x));
	mScrollExtent.y = std::max (mSize.y, std::max (mMinimumScrollExtent.y, NewSize.y));

	UIPoint loc (mScrollLocation);

	if (loc.x + mSize.x > mScrollExtent.x)
		loc.x = std::max (0L, mScrollExtent.x - mSize.x);

	if (loc.y + mSize.y > mScrollExtent.y)
		loc.y = std::max (0L, mScrollExtent.y - mSize.y);

	SetScrollLocation (loc);
}

//-----------------------------------------------------------------

void UIWidget::GetScrollSizes( UISize &PageSize, UISize &LineSize ) const
{
	DEBUG_DESTROYED();
	
	PageSize = GetSize();
	if (mScrollSizePage.x > 0L)
		PageSize.x = mScrollSizePage.x;
	if (mScrollSizePage.y > 0L)
		PageSize.y = mScrollSizePage.y;

	LineSize = mScrollSizeLine;
}

//-----------------------------------------------------------------

bool UIWidget::HitTest( const UIPoint &PointToTest ) const
{
	DEBUG_DESTROYED();
	
	if( PointToTest.x < 0 )
		return false;

	if( PointToTest.y < 0 )
		return false;

	if( PointToTest.x > mSize.x )
		return false;

	if( PointToTest.y > mSize.y )
		return false;

	if (mBoundaries)
	{
		assert (!mBoundaries->empty ()); //lint !e1924 //stfu
		return mBoundaries->hitTest (PointToTest);
	}

	return true;
}

//-----------------------------------------------------------------

UIWidget * UIWidget::GetWidgetFromPoint( const UIPoint & pt , bool mustGetInput) const
{
	DEBUG_DESTROYED();
	
	if (!mustGetInput || (GetsInput () && IsEnabled () && WillDraw()))
	{
		if (HitTest (pt))
			return const_cast<UIWidget *>(this);
	}

	return 0;
}

//-----------------------------------------------------------------

bool UIWidget::WantsMessage( const UIMessage & ) const
{
	DEBUG_DESTROYED();
	
	return WillDraw() && HasAttribute (BF_GetsInput);
}

//-----------------------------------------------------------------

bool UIWidget::ProcessMessage( const UIMessage &msg )
{
	DEBUG_DESTROYED();
	
	if (!HasAttribute(BF_UserModifying))
		mCurrentUserModificationType = UMT_NONE;

	if( !SendUIMessageCallback( msg ) )
		return true;

	switch( msg.Type )
	{
		case UIMessage::MouseEnter:
			SetUnderMouse (true);
			break;

		case UIMessage::MouseExit:
			SetUnderMouse (false);
			break;

		case UIMessage::DragEnd:
			SendCallback( &UIEventCallback::OnDrop, msg.DragObject, UILowerString::null );
			break;

		case UIMessage::MouseWheel:

			{
				UISize	ScrollSize;
				UISize  PageSize;
				UISize  LineSize;

				GetScrollExtent   ( ScrollSize );
				UIPoint ScrollLocation (GetScrollLocation ());
				GetScrollSizes    ( PageSize, LineSize );

				long delta = - msg.Data;
				//multiply so we don't scroll 1 pixel/click.  This should eventually become a per-widget setting
				delta *= 5;

				//-- vertical scrolling only

				if (mSize.y != ScrollSize.y)
				{
					long newLocation = 0;
					newLocation = std::min (ScrollSize.y - PageSize.y, ScrollLocation.y + delta * LineSize.y);
					newLocation = std::max (0L, newLocation);

					if (ScrollLocation.y != newLocation)
					{
						ScrollLocation.y = newLocation;
						ScrollToPoint( ScrollLocation );
					}
					return true;
				}

				//-- attempt a horizontal scroll
				else if (mSize.x != ScrollSize.x)
				{
					long newLocation = 0;
					newLocation = std::min (ScrollSize.x - PageSize.x, ScrollLocation.x + delta * LineSize.x);
					newLocation = std::max (0L, newLocation);

					if (ScrollLocation.x != newLocation)
					{
						ScrollLocation.x = newLocation;
						ScrollToPoint( ScrollLocation );
					}
					return true;
				}
			}
			break;

		case UIMessage::LeftMouseUp:
		case UIMessage::MiddleMouseUp:
		case UIMessage::RightMouseUp:
			if (!HitTest (msg.MouseCoords))
				SetUnderMouse (false);
			break;

		case UIMessage::KeyFirst:
		case UIMessage::KeyDown:
		case UIMessage::KeyUp:
		case UIMessage::KeyRepeat:
		case UIMessage::KeyLast:
		case UIMessage::MouseFirst:
		case UIMessage::LeftMouseDown:
		case UIMessage::MiddleMouseDown:
		case UIMessage::RightMouseDown:
		case UIMessage::MouseLastFocusChanger:
		case UIMessage::MouseMove:
		case UIMessage::MouseLast:
		case UIMessage::DragFirst:
		case UIMessage::DragStart:
		case UIMessage::DragCancel:
		case UIMessage::DragOver:
		case UIMessage::DragLast:
			break;

		case UIMessage::Character:
		case UIMessage::LeftMouseDoubleClick:
		case UIMessage::MiddleMouseDoubleClick:
		case UIMessage::RightMouseDoubleClick:
		case UIMessage::ContextRequest:
		case UIMessage::NumMessages:
			break;
	}

	if (HasAttribute(BF_UserModifying))
		return UIWidget::ProcessUserMessage (msg);

	return false;
}

//----------------------------------------------------------------------

bool UIWidget::ProcessUserMouseMove (const UIMessage & msg)
{
	DEBUG_DESTROYED();
	
	assert (msg.Type == UIMessage::MouseMove);

	//----------------------------------------------------------------------

	if (HasAttribute(BF_UserModifying))
	{
		if (mCurrentUserModificationType == UMT_NONE)
		{
			SetAttribute(BF_UserModifying, false);
			return false;
		}

		if (mCurrentUserModificationType == UMT_DRAGSCROLL)
		{
			ScrollToPoint ((mUserModificationStartWidgetRect.Location () - msg.MouseCoords) + mUserModificationStartPoint);
			return true;
		}

		UISize parentSize;
		if (GetParent () && GetParent ()->IsA (TUIWidget))
			parentSize = static_cast<UIWidget *>(GetParent ())->GetSize ();

		const UIPoint diff ((msg.MouseCoords - mUserModificationStartPoint) + (mLocation - mUserModificationStartWidgetRect.Location ()));

		UIPoint resizeDiff (diff);

		if (mSizeIncrement.x)
			resizeDiff.x -= (resizeDiff.x % mSizeIncrement.x);

		if (mSizeIncrement.y)
			resizeDiff.y -= (resizeDiff.y % mSizeIncrement.y);

		UIRect rect (mUserModificationStartWidgetRect);

		UIWidget::UserModificationType saveType = mCurrentUserModificationType;
		UIWidget::UserModificationType useThisType = mCurrentUserModificationType;
		if(mLockDiagonal)
		{
			switch (mCurrentUserModificationType)
			{
			case UMT_N:
			case UMT_NE:
				resizeDiff.x = -resizeDiff.y;
				useThisType = UMT_NE;
				break;
			case UMT_E:
				resizeDiff.y = resizeDiff.x;
				useThisType = UMT_SE;
				break;
			case UMT_SE:
			case UMT_S:
				resizeDiff.x = resizeDiff.y;
				useThisType = UMT_SE;
				break;
			case UMT_SW:
			case UMT_W:
				resizeDiff.y = -resizeDiff.x;
				useThisType = UMT_SW;
				break;
			case UMT_NW:
				resizeDiff.y = resizeDiff.x;
				useThisType = UMT_NW;
				break;
			}
			mCurrentUserModificationType = useThisType;
		}
		switch (mCurrentUserModificationType)
		{
		case UMT_N:
			rect.top    = std::max (0L, std::min (parentSize.y, rect.top + resizeDiff.y));
			break;
		case UMT_NE:
			rect.right  = std::max (0L, std::min (parentSize.x, rect.right  + resizeDiff.x));
			rect.top    = std::max (0L, std::min (parentSize.y, rect.top    + resizeDiff.y));
			break;
		case UMT_E:
			rect.right  = std::max (0L, std::min (parentSize.x, rect.right  + resizeDiff.x));
			break;
		case UMT_SE:
			rect.right  = std::max (0L, std::min (parentSize.x, rect.right  + resizeDiff.x));
			rect.bottom = std::max (0L, std::min (parentSize.y, rect.bottom + resizeDiff.y));
			break;
		case UMT_S:
			rect.bottom = std::max (0L, std::min (parentSize.y, rect.bottom + resizeDiff.y));
			break;
		case UMT_SW:
			rect.left   = std::max (0L, std::min (parentSize.x, rect.left   + resizeDiff.x));
			rect.bottom = std::max (0L, std::min (parentSize.y, rect.bottom + resizeDiff.y));
			break;
		case UMT_W:
			rect.left   = std::max (0L, std::min (parentSize.x, rect.left   + resizeDiff.x));
			break;
		case UMT_NW:
			rect.left   = std::max (0L, std::min (parentSize.x, rect.left   + resizeDiff.x));
			rect.top    = std::max (0L, std::min (parentSize.y, rect.top    + resizeDiff.y));
			break;
		case UMT_MOVE:
			if(diff.x!= 0 || diff.y != 0 )
				s_hasDragMovedYet = true;
			rect        += diff;

			if ((rect.top < 0L) && (rect.bottom < 0L))
			{
				rect.bottom -= rect.top;
				rect.top = 0L;
			}
			else if ((rect.top > parentSize.y) && (rect.bottom > parentSize.y))
			{
				rect.top -= rect.bottom - parentSize.y;
				rect.bottom = parentSize.y;
			}
			
			if ((rect.left < 0L) && (rect.right < 0L))
			{
				rect.right -= rect.left;
				rect.left     = 0L;
			}
			else if ((rect.right > parentSize.x) && (rect.left > parentSize.x))
			{
				rect.left -= rect.right - parentSize.x;
				rect.right = parentSize.x;
			}
			break;
		}

		if (rect != GetRect ())
		{
			//-- make sure we don't stretch it too much or too little

			if (rect.Width () < mMinimumSize.x)
			{
				if (mCurrentUserModificationType == UMT_W ||
					mCurrentUserModificationType == UMT_NW ||
					mCurrentUserModificationType == UMT_SW)
				{
					rect.left -= (mMinimumSize.x - rect.Width ());
				}
			}
			if (rect.Height () < mMinimumSize.y)
			{
				if (mCurrentUserModificationType == UMT_N ||
					mCurrentUserModificationType == UMT_NW ||
					mCurrentUserModificationType == UMT_NE)
				{
					rect.top -= (mMinimumSize.y - rect.Height ());
				}
			}

			if (rect.Width () > mMaximumSize.x)
			{
				if (mCurrentUserModificationType == UMT_E ||
					mCurrentUserModificationType == UMT_NE ||
					mCurrentUserModificationType == UMT_SE)
				{
					rect.right -= (rect.Width () - mMaximumSize.x);
				}
			}
			if (rect.Height () > mMaximumSize.y)
			{
				if (mCurrentUserModificationType == UMT_S ||
					mCurrentUserModificationType == UMT_SW ||
					mCurrentUserModificationType == UMT_SE)
				{
					rect.bottom -= (rect.Height () - mMaximumSize.y);
				}
			}

			if (mCallbacks)
			{
				EventCallbackList CallbacksToInvoke( *mCallbacks );

				for( EventCallbackList::iterator i = CallbacksToInvoke.begin(); i != CallbacksToInvoke.end(); ++i )
				{
					(*i)->OnWidgetRectChanging( this, rect);
				}
			}

			SetRect (rect);

			SendCallback( &UIEventCallback::OnWidgetRectChanged, PropertyName::OnEnable );
		}
		mCurrentUserModificationType = saveType;
		return true;
	}

	//----------------------------------------------------------------------

	else
	{
		if (IsUserResizable () || IsUserDragScrollable () || IsUserMovable () || GetAcceptsMoveFromChildren())
		{
			mCurrentUserModificationType = GetUserModificationType (msg.MouseCoords);
		}
		else
			mCurrentUserModificationType = UMT_NONE;

		return (mCurrentUserModificationType != UMT_NONE);
	}

	return false;
}

//----------------------------------------------------------------------

bool UIWidget::ProcessUserMessage (const UIMessage & msg)
{
	DEBUG_DESTROYED();
	
	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::MouseMove)
	{
		return ProcessUserMouseMove (msg);
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseUp || msg.Type == UIMessage::MouseExit)
	{
		SetAttribute(BF_UserModifying, false);
		mCurrentUserModificationType = UMT_NONE;
	}

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::LeftMouseDown)
	{
		if (IsUserResizable () || IsUserDragScrollable () || IsUserMovable () || GetAcceptsMoveFromChildren())
		{
			mCurrentUserModificationType = GetUserModificationType (msg.MouseCoords);

			if (mCurrentUserModificationType != UMT_NONE)
			{
				SetAttribute(BF_UserModifying, true);
				s_hasDragMovedYet = false;
				mUserModificationStartPoint = msg.MouseCoords;
				GetRect (mUserModificationStartWidgetRect);

				if (mCurrentUserModificationType == UMT_DRAGSCROLL)
				{
					mUserModificationStartWidgetRect.left = GetScrollLocation ().x;
					mUserModificationStartWidgetRect.top  = GetScrollLocation ().y;
				}

				return true;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

UICursor * UIWidget::GetUserModificationCursor (UserModificationType type) const
{
	DEBUG_DESTROYED();
	
	if (type < 0 || type >= UMT_NONE)
		return 0;

	UICursor * cursor = 0;

	if (mCursorSet)
		cursor =  mCursorSet->GetCursor (type);

	if (!cursor && GetParent () && GetParent ()->IsA (TUIWidget))
		cursor = static_cast<const UIWidget *>(GetParent ())->GetUserModificationCursor (type);

	return cursor;
}
//----------------------------------------------------------------------

UIWidget::UserModificationType    UIWidget::GetUserModificationType (const UIPoint & pt) const
{
	DEBUG_DESTROYED();
	
	UIWidget * const widget = GetWidgetFromPoint ( pt, true);

	bool moveForwardedFromChild = false;

	if (widget)
	{
		UIWidget * parent = widget;

		while (parent)
		{
			if (parent == this)
				break;

			if (parent->IsSelectable ())
				return UMT_NONE;

			parent = parent->GetParentWidget ();
		}

		if (widget != this)
		{
			moveForwardedFromChild = GetAcceptsMoveFromChildren() && (widget->FindTopMoveParentWidget() == this);

			if (widget->IsUserMovable() && !moveForwardedFromChild)
				return UMT_NONE;

			if (widget->IsUserDragScrollable () || widget->IsUserResizable ())
				return UMT_NONE;

			if (!widget->IsA (TUIPage))
				return UMT_NONE;

			if (widget->FindFirstDragableWidget ())
				return UMT_NONE;

		}

		if (widget->FindFirstContextCapableWidget (false))
			return UMT_NONE;

	}

	UserModificationType type = UMT_MOVE;

	UISize extent;
	GetScrollExtent (extent);

	if (IsUserResizable ())
	{
		const bool canResizeX = (mMinimumSize.x != mSize.x || mMaximumSize.x != mSize.x);
		const bool canResizeY = (mMinimumSize.y != mSize.y || mMaximumSize.y != mSize.y);

		if(mResizeInset == 0)
		{
			//Old and simpler logic handles this case
			if ((pt.x < RESIZE_MARGIN) && canResizeX)
			{
				if (pt.y < RESIZE_MARGIN)
					type = UMT_NW;
				else if (pt.y > (mSize.y - RESIZE_MARGIN))
					type = UMT_SW;
				else
					type = UMT_W;
			}
			else if ((pt.x >= (mSize.x - RESIZE_MARGIN)) && canResizeX)
			{
				if (pt.y < RESIZE_MARGIN)
					type = UMT_NE;
				else if (pt.y > (mSize.y - RESIZE_MARGIN))
					type = UMT_SE;
				else
					type = UMT_E;
			}
			else if ((pt.y < RESIZE_MARGIN) && canResizeY)
				type = UMT_N;
			else if ((pt.y >= (mSize.y - RESIZE_MARGIN)) && canResizeY)
				type = UMT_S;
		}
		else
		{
			if( (pt.x < mResizeInset) || (pt.x > (mSize.x - mResizeInset)) ||
				(pt.y < mResizeInset) || (pt.y > (mSize.y - mResizeInset)) )
				type = UMT_NONE;
			else if ((pt.x > mResizeInset) && (pt.x < RESIZE_MARGIN + mResizeInset) && canResizeX)
			{
				if ((pt.y > mResizeInset) && (pt.y < RESIZE_MARGIN + mResizeInset))
					type = UMT_NW;
				else if ( (pt.y < mSize.y - mResizeInset) && (pt.y > (mSize.y - RESIZE_MARGIN - mResizeInset)) )
					type = UMT_SW;
				else
					type = UMT_W;
			}
			else if ( (pt.x < (mSize.x - mResizeInset)) && (pt.x >= (mSize.x - RESIZE_MARGIN - mResizeInset)) && canResizeX)
			{
				if ((pt.y > mResizeInset) && (pt.y < RESIZE_MARGIN + mResizeInset))
					type = UMT_NE;
				else if ( (pt.y < mSize.y - mResizeInset) && (pt.y > (mSize.y - RESIZE_MARGIN - mResizeInset)) )
					type = UMT_SE;
				else
					type = UMT_E;
			}
			else if ((pt.y > mResizeInset) && (pt.y < RESIZE_MARGIN + mResizeInset) && canResizeY)
				type = UMT_N;
			else if ((pt.y < mSize.y - mResizeInset) && (pt.y >= (mSize.y - RESIZE_MARGIN - mResizeInset)) && canResizeY)
				type = UMT_S;
		}
	}

	if (extent != mSize && IsUserDragScrollable ())
	{
		return UMT_DRAGSCROLL;
	}

	if (!moveForwardedFromChild && !IsUserMovable () && type == UMT_MOVE)
		return UMT_NONE;

	return type;

}

//-----------------------------------------------------------------

void UIWidget::ProcessChildNotificationMessage( UIWidget *, const UIMessage & )
{
}

//-----------------------------------------------------------------

void UIWidget::FillHorizontallyAndCap( UICanvas &DestinationCanvas, long width, UIImageStyle *StartCap, UIImageStyle *Background, UIImageStyle *EndCap ) const
{
	DEBUG_DESTROYED();
	
	unsigned long		AnimationState = static_cast<unsigned long>(GetAnimationState());
	const UIPoint & Size = GetSize ();

	DestinationCanvas.PushState();
	DestinationCanvas.Clip( UIRect( 0, 0, width, Size.y ) );

	DestinationCanvas.PushState();
	DestinationCanvas.Clip( UIRect( 0, 0, width - (EndCap ? EndCap->GetWidth() : 0), Size.y ) );

	if( StartCap )
		DestinationCanvas.Translate( StartCap->GetWidth(), 0 );

	if( Background )
	{
		int BackgroundWidth	= Background->GetWidth();

		if( BackgroundWidth > 0 )
		{
			long PixelOffset = 0;

			while( PixelOffset < width )
			{
				Background->Render( AnimationState, DestinationCanvas, UIPoint(PixelOffset,0) );
				PixelOffset += BackgroundWidth;
			}
		}
	}

	DestinationCanvas.PopState();

	if( StartCap )
		StartCap->Render( AnimationState, DestinationCanvas, UIPoint::zero );

	if( EndCap )
		EndCap->Render( AnimationState, DestinationCanvas, UIPoint(width - EndCap->GetWidth(),0) );

	DestinationCanvas.PopState();
}

//-----------------------------------------------------------------

void UIWidget::FillVerticallyAndCap( UICanvas &DestinationCanvas, long height, UIImageStyle *StartCap, UIImageStyle *Background, UIImageStyle *EndCap ) const
{
	DEBUG_DESTROYED();
	
	unsigned long		AnimationState = static_cast<unsigned long>(GetAnimationState());
	const UIPoint & Size = GetSize ();

	DestinationCanvas.PushState();
	DestinationCanvas.Clip( UIRect( 0, 0, Size.x, height ) );

	DestinationCanvas.PushState();
	DestinationCanvas.Clip( UIRect( 0, 0, Size.x, height - (EndCap ? EndCap->GetHeight() : 0) ) );

	if( StartCap )
		DestinationCanvas.Translate( 0, StartCap->GetHeight() );

	if( Background )
	{
		int BackgroundHeight	= Background->GetHeight();

		if( BackgroundHeight > 0 )
		{
			long PixelOffset = 0;

			while( PixelOffset < height )
			{
				Background->Render( AnimationState, DestinationCanvas, UIPoint(0, PixelOffset) );
				PixelOffset += BackgroundHeight;
			}
		}
	}

	DestinationCanvas.PopState();

	if( StartCap )
		StartCap->Render( AnimationState, DestinationCanvas, UIPoint::zero );

	if( EndCap )
		EndCap->Render( AnimationState, DestinationCanvas, UIPoint(0,height - EndCap->GetHeight()) );

	DestinationCanvas.PopState();
}

//-----------------------------------------------------------------

void UIWidget::FillVerticallyAndCapInverted( UICanvas &DestinationCanvas, long height, UIImageStyle *StartCap, UIImageStyle *Background, UIImageStyle *EndCap ) const
{
	DEBUG_DESTROYED();
	
	unsigned long		AnimationState = static_cast<unsigned long>(GetAnimationState());
	const UIPoint & Size = GetSize ();

	DestinationCanvas.PushState();
	DestinationCanvas.Clip( UIRect( 0, 0, Size.x, height ) );

	DestinationCanvas.PushState();
	DestinationCanvas.Clip( UIRect( 0, 0, Size.x, height - (EndCap ? EndCap->GetHeight() : 0) ) );

	if( StartCap )
		DestinationCanvas.Translate( 0, StartCap->GetHeight() );

	if( Background )
	{
		int BackgroundHeight = Background->GetHeight();

		if( BackgroundHeight > 0 )
		{
			long PixelOffset = height;

			while( PixelOffset > 0 )
			{
				PixelOffset -= BackgroundHeight;
				Background->Render( AnimationState, DestinationCanvas, UIPoint(0, PixelOffset) );
			}
		}
	}

	DestinationCanvas.PopState();

	if( StartCap )
		StartCap->Render( AnimationState, DestinationCanvas, UIPoint::zero );

	if( EndCap )
		EndCap->Render( AnimationState, DestinationCanvas, UIPoint(0,height - EndCap->GetHeight()) );

	DestinationCanvas.PopState();
}

//-----------------------------------------------------------------

bool UIWidget::SendUIMessageCallback( const UIMessage &msg, UIWidget * delegateContext )
{
	DEBUG_DESTROYED();
	
	if (mCallbacks)
	{
		EventCallbackList CallbacksToInvoke( *mCallbacks );

		for( EventCallbackList::iterator i = CallbacksToInvoke.begin(); i != CallbacksToInvoke.end(); ++i )
		{
			if( !(*i)->OnMessage( delegateContext ? delegateContext : this, msg ) )
				return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------

void UIWidget::SendDataChangedCallback(const UILowerString & property, const UIString & value)
{
	DEBUG_DESTROYED();
	
	if (mCallbacks)
	{
		EventCallbackList CallbacksToInvoke( *mCallbacks );

		for( EventCallbackList::iterator i = CallbacksToInvoke.begin(); i != CallbacksToInvoke.end(); ++i )
		{
			(*i)->OnWidgetDataChanged( this, property.get (), value );
		}
	}
}

//-----------------------------------------------------------------

void UIWidget::SendTreeRowExpansionToggledCallback(int const row)
{
	DEBUG_DESTROYED();
	
	if (mCallbacks)
	{
		EventCallbackList CallbacksToInvoke( *mCallbacks );

		for( EventCallbackList::iterator i = CallbacksToInvoke.begin(); i != CallbacksToInvoke.end(); ++i )
		{
			(*i)->OnTreeRowExpansionToggled(this, row);
		}
	}
}

//----------------------------------------------------------------------

/**
* This function was inlined but is no more because strange esp corruption was occuring
* in msvc 6.0
*
*/

void UIWidget::SendCallback( UIWidget::CallbackFunc f, const UILowerString & ScriptPropertyName )
{
	DEBUG_DESTROYED();
	
	SendCallback( f, this, ScriptPropertyName );
}

//-----------------------------------------------------------------

void UIWidget::SendCallback( UIWidget::CallbackFunc f, UIWidget *Context, const UILowerString & ScriptPropertyName )
{
	DEBUG_DESTROYED();
	
	assert( Context ); //lint !e1924 // c-style case MSVC bug

	if (mCallbacks)
	{
		EventCallbackList CallbacksToInvoke( *mCallbacks );

		for( EventCallbackList::iterator i = CallbacksToInvoke.begin(); i != CallbacksToInvoke.end(); ++i )
		{
			UIEventCallback * const callback = *i;
			(callback->*f)( Context );
		}
	}

	if( !ScriptPropertyName.empty () )
	{
		UIManager &theManager = UIManager::gUIManager();
		UIStyle		*theStyle		= Context->GetStyle();
		UILowerString  EffectorPropertyName;
		UIString   EffectorName;
		UIString   Script;

		EffectorPropertyName.set (ScriptPropertyName.get () + std::string ("Effector"));

		// Execute local script
		if( GetProperty( ScriptPropertyName, Script ) )
			theManager.ExecuteScript( Script, this );
		else if( theStyle )
		{
			// Execute style script
			if( theStyle->GetProperty( ScriptPropertyName, Script ) )
				theManager.ExecuteScript( Script, this );
		}

		// Execute local effector
		if( GetProperty( EffectorPropertyName, EffectorName ) )
		{
			UIEffector *theEffector = static_cast<UIEffector *>( Context->GetObjectFromPath( EffectorName, TUIEffector ) );

			if( theEffector )
				Context->ExecuteEffector (*theEffector);
		}
		else if( theStyle )
		{
			// Execute style effector
			if( theStyle->GetProperty( EffectorPropertyName, EffectorName ) )
			{
				UIEffector *theEffector = static_cast<UIEffector *>( Context->GetObjectFromPath( EffectorName, TUIEffector ) );

				if( theEffector )
					Context->ExecuteEffector (*theEffector);
			}
		}
	}
}

//-----------------------------------------------------------------

void UIWidget::CancelEffector( const UILowerString & ScriptPropertyName )
{
	DEBUG_DESTROYED();
	
	if( !ScriptPropertyName.empty () )
	{
		UILowerString	EffectorPropertyName (ScriptPropertyName.get()  + std::string ("Effector"));
		UIString   EffectorName;

		// Cancel local effector
		if( GetProperty( EffectorPropertyName, EffectorName ) )
		{
			UIEffector *theEffector = static_cast<UIEffector *>( GetObjectFromPath( EffectorName, TUIEffector ) );

			if( theEffector )
				CancelEffector (*theEffector);
		}

		UIStyle *theStyle = GetStyle();

		if( theStyle )
		{
			// Cancel style effector
			if( theStyle->GetProperty( EffectorPropertyName, EffectorName ) )
			{
				UIEffector *theEffector = static_cast<UIEffector *>( GetObjectFromPath( EffectorName, TUIEffector ) );

				if( theEffector )
					CancelEffector (*theEffector);
			}
		}
	}
}

//----------------------------------------------------------------------

void UIWidget::CancelEffector  (UIEffector & effector)
{
	DEBUG_DESTROYED();
	
	if (UIManager::isUIReady())
	{
		UIManager::gUIManager ().CancelEffector( &effector, this );
	}
}

//----------------------------------------------------------------------

void UIWidget::ExecuteEffector (UIEffector & effector)
{
	DEBUG_DESTROYED();
	
	if (UIManager::isUIReady())
	{
		UIManager::gUIManager ().ExecuteEffector( &effector, this );
	}
}

//-----------------------------------------------------------------

void UIWidget::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	DEBUG_DESTROYED();
	
	In.push_back( PropertyName::Cursor );
	In.push_back( PropertyName::CursorSet );
	In.push_back( PropertyName::CustomDragWidget);
	In.push_back( PropertyName::DragBadCursor );
	In.push_back( PropertyName::DragGoodCursor );
	In.push_back( PropertyName::PopupStyle );
	In.push_back( PropertyName::Size );
	In.push_back( PropertyName::Tooltip );
	In.push_back( PropertyName::TooltipStyle );
	In.push_back( PropertyName::TooltipDelay );
	In.push_back( PropertyName::PalShade);

	mRectangleStyles->GetLinkPropertyNames (In);

	UIBaseObject::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIWidget::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	DEBUG_DESTROYED();
	
	UIBaseObject::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedBehavior, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedAppearance, category, o_groups);

	UIWidgetRectangleStyles::GetStaticPropertyGroups(o_groups, category);
}

//----------------------------------------------------------------------

void UIWidget::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	DEBUG_DESTROYED();
	
	if (!forCopy)
	{
		UIPalette::GetPropertyNamesForType (TUIWidget, In);
		In.push_back( PropertyName::AbsorbsInput);
		In.push_back( PropertyName::Activated );
		In.push_back( PropertyName::BackgroundColor );
		In.push_back( PropertyName::BackgroundOpacity );
		In.push_back( PropertyName::BackgroundScrolls);
		In.push_back( PropertyName::BackgroundTint );
		In.push_back( PropertyName::Color );
		In.push_back( PropertyName::ContextCapable);
		In.push_back( PropertyName::ContextCapableAlternate);
		In.push_back( PropertyName::ContextToParent);
		In.push_back( PropertyName::DragAccepts );
		In.push_back( PropertyName::DragType );
		In.push_back( PropertyName::Dragable );
		In.push_back( PropertyName::DropToParent );
		In.push_back( PropertyName::Enabled );
		In.push_back( PropertyName::GetsInput );
		In.push_back( PropertyName::LocalTooltip );
		In.push_back( PropertyName::Location );
		In.push_back( PropertyName::MaximumSize );
		In.push_back( PropertyName::MinimumScrollExtent );
		In.push_back( PropertyName::MinimumSize );
		In.push_back( PropertyName::OnActivate );
		In.push_back( PropertyName::OnActivateEffector );
		In.push_back( PropertyName::OnDeactivate );
		In.push_back( PropertyName::OnDeactivateEffector );
		In.push_back( PropertyName::OnDisable );
		In.push_back( PropertyName::OnDisableEffector );
		In.push_back( PropertyName::OnEnable );
		In.push_back( PropertyName::OnEnableEffector );
		In.push_back( PropertyName::OnHide );
		In.push_back( PropertyName::OnHideEffector );
		In.push_back( PropertyName::OnHoverIn );
		In.push_back( PropertyName::OnHoverInEffector );
		In.push_back( PropertyName::OnHoverOut );
		In.push_back( PropertyName::OnHoverOutEffector );
		In.push_back( PropertyName::OnShow );
		In.push_back( PropertyName::OnShowEffector );
		In.push_back( PropertyName::Opacity );
		In.push_back( PropertyName::OpacityRelativeMin );
		In.push_back( PropertyName::TextOpacityRelativeApply);
		In.push_back( PropertyName::TextOpacityRelativeMin);
		In.push_back( PropertyName::PackLocation );
		In.push_back( PropertyName::PackSize );
		In.push_back( PropertyName::PalShade);
		In.push_back( PropertyName::Rotation );
		In.push_back( PropertyName::ScrollExtent );
		In.push_back( PropertyName::ScrollLocation );
		In.push_back( PropertyName::ScrollSizeLine );
		In.push_back( PropertyName::ScrollSizePage );
		In.push_back( PropertyName::Selectable );
		In.push_back( PropertyName::TabRoot );
		In.push_back( PropertyName::Shear );
		In.push_back( PropertyName::ShrinkWrap );
		In.push_back( PropertyName::Size );
		In.push_back( PropertyName::SizeIncrement );
		In.push_back( PropertyName::Tooltip );
		In.push_back( PropertyName::UserDragScrollable );
		In.push_back( PropertyName::UserMovable );
		In.push_back( PropertyName::UserResizable );
		In.push_back( PropertyName::Visible );
		In.push_back( PropertyName::AbsorbsTab );
		In.push_back( PropertyName::AutoRegister );
		In.push_back( PropertyName::ForwardMoveToParent );
		In.push_back( PropertyName::AcceptsMoveFromChildren );
	}

	In.push_back( PropertyName::CustomDragWidget);
	In.push_back( PropertyName::DragBadCursor );
	In.push_back( PropertyName::DragGoodCursor );
	In.push_back( PropertyName::Cursor );
	In.push_back( PropertyName::CursorSet );
	In.push_back( PropertyName::PopupStyle );
	In.push_back( PropertyName::TooltipStyle );
	In.push_back( PropertyName::TooltipDelay );

	mRectangleStyles->GetPropertyNames (In);

	UIBaseObject::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

void UIWidget::GetCategories(UIPropertyCategories::CategoryMask &o_categories) const
{
	DEBUG_DESTROYED();
	
	o_categories.setHasBasic(true);
	o_categories.setHasAppearance(true);
	o_categories.setHasAdvancedAppearance(true);
	o_categories.setHasBehavior(true);
	o_categories.setHasAdvancedBehavior(true);

	UIBaseObject::GetCategories(o_categories);
}

//----------------------------------------------------------------------

void UIWidget::GetPropertiesInCategory (UIPropertyCategories::Category category, UIPropertyNameVector & In) const
{
	DEBUG_DESTROYED();
	
	if(category == UIPropertyCategories::C_Basic)
	{						
		In.push_back( PropertyName::Location );
		In.push_back( PropertyName::LockDiagonal );
		In.push_back( PropertyName::MaximumSize );
		In.push_back( PropertyName::MinimumScrollExtent );
		In.push_back( PropertyName::MinimumSize );
		
		In.push_back( PropertyName::PackLocation );
		In.push_back( PropertyName::PackSize );
		In.push_back( PropertyName::ResizeInset );
		In.push_back( PropertyName::Rotation );
		In.push_back( PropertyName::ScrollExtent );
		In.push_back( PropertyName::ScrollLocation );
		In.push_back( PropertyName::ScrollSizeLine );
		In.push_back( PropertyName::ScrollSizePage );
		In.push_back( PropertyName::Size );
		In.push_back( PropertyName::SizeIncrement );
	}
	else if(category == UIPropertyCategories::C_Appearance)
	{		
		UIPalette::GetPropertyNamesForType (TUIWidget, In);
		In.push_back( PropertyName::BackgroundColor );
		In.push_back( PropertyName::BackgroundOpacity );
		In.push_back( PropertyName::BackgroundScrolls);
		In.push_back( PropertyName::BackgroundTint );
		In.push_back( PropertyName::Color );
		In.push_back( PropertyName::Enabled );		
		In.push_back( PropertyName::Opacity );
		In.push_back( PropertyName::OpacityRelativeMin );
		In.push_back( PropertyName::TextOpacityRelativeApply);
		In.push_back( PropertyName::TextOpacityRelativeMin);
		In.push_back( PropertyName::Tooltip );
		In.push_back( PropertyName::TooltipStyle );
		In.push_back( PropertyName::TooltipDelay );
		In.push_back( PropertyName::Visible );
		mRectangleStyles->GetPropertyNames (In);
	}
	else if(category == UIPropertyCategories::C_Behavior)
	{		
		In.push_back( PropertyName::AbsorbsInput);
		In.push_back( PropertyName::Activated );		
		In.push_back( PropertyName::Selectable );
		In.push_back( PropertyName::UserDragScrollable );
		In.push_back( PropertyName::UserMovable );
		In.push_back( PropertyName::UserResizable );
		In.push_back( PropertyName::GetsInput );
	}
	else if(category == UIPropertyCategories::C_AdvancedAppearance)
	{
		In.push_back( PropertyName::LocalTooltip );
		In.push_back( PropertyName::TabRoot );
		In.push_back( PropertyName::Shear );
		In.push_back( PropertyName::ShrinkWrap );			
		In.push_back( PropertyName::CustomDragWidget);
		In.push_back( PropertyName::DragBadCursor );
		In.push_back( PropertyName::DragGoodCursor );
		In.push_back( PropertyName::Cursor );
		In.push_back( PropertyName::CursorSet );
		In.push_back( PropertyName::PopupStyle );
		In.push_back( PropertyName::PalShade);
	}
	else if(category == UIPropertyCategories::C_AdvancedBehavior)
	{		
		In.push_back( PropertyName::ContextCapable);
		In.push_back( PropertyName::ContextCapableAlternate);
		In.push_back( PropertyName::ContextToParent);
		In.push_back( PropertyName::DragAccepts );
		In.push_back( PropertyName::DragType );
		In.push_back( PropertyName::Dragable );
		In.push_back( PropertyName::DropToParent );
		In.push_back( PropertyName::AbsorbsTab );
		In.push_back( PropertyName::OnActivate );
		In.push_back( PropertyName::OnActivateEffector );
		In.push_back( PropertyName::OnDeactivate );
		In.push_back( PropertyName::OnDeactivateEffector );
		In.push_back( PropertyName::OnDisable );
		In.push_back( PropertyName::OnDisableEffector );
		In.push_back( PropertyName::OnEnable );
		In.push_back( PropertyName::OnEnableEffector );
		In.push_back( PropertyName::OnHide );
		In.push_back( PropertyName::OnHideEffector );
		In.push_back( PropertyName::OnHoverIn );
		In.push_back( PropertyName::OnHoverInEffector );
		In.push_back( PropertyName::OnHoverOut );
		In.push_back( PropertyName::OnHoverOutEffector );
		In.push_back( PropertyName::OnShow );
		In.push_back( PropertyName::OnShowEffector );
		In.push_back( PropertyName::ForwardMoveToParent );
		In.push_back( PropertyName::AcceptsMoveFromChildren);
	}


	UIBaseObject::GetPropertiesInCategory(category, In);
}

//-----------------------------------------------------------------

bool UIWidget::SetProperty( const UILowerString & Name, const UIString &Value )
{
	DEBUG_DESTROYED();
	
	//-----------------------------------------------------------------
	//-- Color
	//-----------------------------------------------------------------

	if( Name == PropertyName::Color )
	{
		UIColor color;
		if( !UIUtils::ParseColor( Value, color ) )
			return false;

		SetColor (color);
		return true;
	}

	//-----------------------------------------------------------------

	if( Name == PropertyName::ColorR )
	{
		UIColor color = mColor;
		color.r = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetColor (color);
		return true;

	}

	//-----------------------------------------------------------------

	if( Name == PropertyName::ColorG )
	{
		UIColor color = mColor;
		color.g = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetColor (color);
		return true;

	}

	//-----------------------------------------------------------------

	if( Name == PropertyName::ColorB )
	{
		UIColor color = mColor;
		color.b = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetColor (color);
		return true;

	}
	
	//-----------------------------------------------------------------
	
	if (Name == PropertyName::PalShade)
	{
		float shade = static_cast<float>(atof(Unicode::wideToNarrow(Value).c_str()));
		SetPalShade(shade);
		return true;
	}

	//-----------------------------------------------------------------
	//-----------------------------------------------------------------
	//-----------------------------------------------------------------

	else if( Name == PropertyName::Cursor )
	{
		UIBaseObject *NewCursor = GetObjectFromPath( Value, TUICursor );

		if( NewCursor || Value.empty() )
		{
			SetMouseCursor (static_cast<UICursor *>( NewCursor ) );
			RemoveProperty (Name);
			return true;
		}
		// Fall through
	}
	else if( Name == PropertyName::Dragable )
	{
		bool localIsDragable;

		if( !UIUtils::ParseBoolean( Value, localIsDragable ) )
			return false;

		SetDragable( localIsDragable );
		return true;
	}
	else if( Name == PropertyName::DropToParent )
	{
		bool localDropToParent;

		if( !UIUtils::ParseBoolean( Value, localDropToParent ) )
			return false;

		SetAttribute( BF_DropToParent, localDropToParent );
		return true;
	}
	else if( Name == PropertyName::ContextToParent )
	{
		bool local;

		if( !UIUtils::ParseBoolean( Value, local ) )
			return false;

		SetAttribute( BF_ContextToParent, local );
		return true;
	}
	else if( Name == PropertyName::ContextCapable )
	{
		bool local;

		if( !UIUtils::ParseBoolean( Value, local ) )
			return false;

		SetContextCapable (local, true);
		return true;
	}
	else if( Name == PropertyName::ContextCapableAlternate )
	{
		bool local;

		if( !UIUtils::ParseBoolean( Value, local ) )
			return false;

		SetContextCapable (local, false);
		return true;
	}
	else if( Name == PropertyName::DragBadCursor )
	{
		UIBaseObject *NewCursor = GetObjectFromPath( Value, TUICursor );

		if( NewCursor || Value.empty() )
		{
			SetDragBadCursor( static_cast<UICursor *>( NewCursor ) );
			RemoveProperty (Name);
			return true;
		}
		// Fall through
	}
	else if( Name ==  PropertyName::DragGoodCursor )
	{
		UIBaseObject *NewCursor = GetObjectFromPath( Value, TUICursor );

		if( NewCursor || Value.empty() )
		{
			SetDragGoodCursor( static_cast<UICursor *>( NewCursor ) );
			RemoveProperty (Name);
			return true;
		}
		// Fall through
	}

	//----------------------------------------------------------------------
	else if( Name == PropertyName::CursorSet )
	{
		UIBaseObject * const cursorSet = GetObjectFromPath( Value, TUICursorSet );

		if( cursorSet || Value.empty() )
		{
			SetCursorSet (static_cast<UICursorSet *>(cursorSet));
			RemoveProperty (Name);
			return true;
		}
		// Fall through
	}
	//----------------------------------------------------------------------
	else if( Name == PropertyName::Activated )
	{
		bool b;

		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetActivated( b );
		return true;
	}

	else if( Name == PropertyName::Enabled )
	{
		bool local;

		if( !UIUtils::ParseBoolean( Value, local ) )
			return false;

		SetEnabled( local );
		return true;
	}

	else if( Name == PropertyName::GetsInput )
	{
		bool local;
		if( !UIUtils::ParseBoolean( Value, local ) )
			return false;

		SetGetsInput (local);

		return true;
	}

	else if( Name == PropertyName::UserMovable )
	{
		bool local;
		if( !UIUtils::ParseBoolean( Value, local ) )
			return false;

		SetUserMovable (local);
		return true;
	}
	else if( Name == PropertyName::UserDragScrollable )
	{
		bool local;
		if( !UIUtils::ParseBoolean( Value, local ) )
			return false;

		SetUserDragScrollable (local);
		return true;
	}
	else if( Name == PropertyName::AbsorbsInput )
	{
		bool local;
		if( !UIUtils::ParseBoolean( Value, local ) )
			return false;

		SetAbsorbsInput (local);
		return true;
	}
	else if( Name == PropertyName::UserResizable )
	{
		bool local;
		if( !UIUtils::ParseBoolean( Value, local ) )
			return false;

		SetUserResizable (local);
		return true;
	}

	//-----------------------------------------------------------------
	//-- Location
	//-----------------------------------------------------------------

	else if( Name == PropertyName::Location )
		return UIUtils::ParsePoint( Value, mLocation );

	//-----------------------------------------------------------------

	else if (Name == PropertyName::LocationX)
	{
		UISize location = mLocation;
		location.x = atoi(Unicode::wideToNarrow(Value).c_str());
		SetLocation(location);
		return true;
	}

	//-----------------------------------------------------------------

	else if (Name == PropertyName::LocationY)
	{
		UISize location = mLocation;
		location.y = atoi(Unicode::wideToNarrow(Value).c_str());
		SetLocation(location);
		return true;
	}

	//-----------------------------------------------------------------

	else if( Name == PropertyName::Opacity )
	{
		float o = 0.0f;
		if (!UIUtils::ParseFloat( Value, o ))
			return false;

		SetOpacity (o);
		return true;
	}
	else if( Name == PropertyName::OpacityRelativeMin )
	{
		float o = 0.0f;
		if (!UIUtils::ParseFloat( Value, o ))
			return false;

		SetOpacityRelativeMin (o);
		return true;
	}
	else if( Name == PropertyName::TextOpacityRelativeApply )
	{
		bool b = false;
		if (!UIUtils::ParseBoolean( Value, b ))
			return false;

		SetAttribute (BF_TextOpacityRelativeApply, b);
		return true;
	}
	else if( Name == PropertyName::TextOpacityRelativeMin )
	{
		return UIUtils::ParseFloat( Value, mTextOpacityRelativeMin );
	}
	else if( Name == PropertyName::Rotation )
	{
		if (Value.empty())
		{
			mRotation=0;
			return true;
		}
		else
		{
			return UIUtils::ParseFloat( Value, mRotation );
		}
	}
	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	//-- Size
	//----------------------------------------------------------------------

	else if( Name == PropertyName::Size )
	{
		// Don't just parse into mSize, as derived classes override SetSize
		UISize NewSize;
		if( !UIUtils::ParsePoint( Value, NewSize ) )
			return false;

		SetSize( NewSize );
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::SizeX )
	{
		// Don't just parse into mSize, as derived classes override SetSize
		UISize size = mSize;
		size.x = atoi(Unicode::wideToNarrow(Value).c_str());
		SetSize(size);
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::SizeY )
	{
		// Don't just parse into mSize, as derived classes override SetSize
		UISize size = mSize;
		size.y = atoi(Unicode::wideToNarrow(Value).c_str());
		SetSize(size);
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::SizeIncrement )
	{
		UISize NewSize;
		if( !UIUtils::ParsePoint( Value, NewSize ) )
			return false;

		SetSizeIncrement( NewSize );
		return true;
	}

	//----------------------------------------------------------------------
	//-- Maximum Size
	//----------------------------------------------------------------------

	else if( Name == PropertyName::LockDiagonal )
	{
		bool lockDiagonal;
		if( !UIUtils::ParseBoolean( Value, lockDiagonal ) )
			return false;

		SetLockDiagonal(lockDiagonal);
		return true;
	}

	//----------------------------------------------------------------------
	else if( Name == PropertyName::MinimumScrollExtent )
	{
		// Don't just parse into mMinimumSize, as SetMinimumSize does special work
		UISize NewSize;
		if( !UIUtils::ParsePoint( Value, NewSize ) )
			return false;

		SetMinimumScrollExtent( NewSize );
		return true;
	}

	//----------------------------------------------------------------------
	//-- Minimum Size
	//----------------------------------------------------------------------

	else if( Name == PropertyName::MinimumSize )
	{
		// Don't just parse into mMinimumSize, as SetMinimumSize does special work
		UISize NewSize;
		if( !UIUtils::ParsePoint( Value, NewSize ) )
			return false;

		SetMinimumSize( NewSize );
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::MinimumSizeX )
	{
		UISize size = mMinimumSize;
		size.x = atoi(Unicode::wideToNarrow(Value).c_str());
		SetMinimumSize(size);
		return true;
	}

	//----------------------------------------------------------------------
	else if( Name == PropertyName::MinimumSizeY )
	{
		UISize size = mMinimumSize;
		size.y = atoi(Unicode::wideToNarrow(Value).c_str());
		SetMinimumSize(size);
		return true;
	}

	//----------------------------------------------------------------------
	//-- Maximum Size
	//----------------------------------------------------------------------

	else if( Name == PropertyName::MaximumSize )
	{
		// Don't just parse into mMaximumSize, as SetMaximumSize does special work
		UISize NewSize;
		if( !UIUtils::ParsePoint( Value, NewSize ) )
			return false;

		SetMaximumSize( NewSize );
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::MaximumSizeX )
	{
		UISize size = mMaximumSize;
		size.x = atoi(Unicode::wideToNarrow(Value).c_str());
		SetMaximumSize(size);
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::MaximumSizeY )
	{
		UISize size = mMaximumSize;
		size.y = atoi(Unicode::wideToNarrow(Value).c_str());
		SetMaximumSize(size);
		return true;
	}

	else if( Name == PropertyName::ResizeInset )
	{
		int resizeInset;
		if( !UIUtils::ParseInteger( Value, resizeInset ) )
			return false;

		SetResizeInset(static_cast<unsigned char>(resizeInset));
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::ScrollExtent )
	{
		UIPoint extent;
		if( UIUtils::ParsePoint( Value, extent ) )
		{
			SetScrollExtent (extent);
			return true;
		}

		return false;
	}
	else if( Name == PropertyName::ScrollLocation )
		return UIUtils::ParsePoint( Value, mScrollLocation );
	else if( Name == PropertyName::Tooltip )
	{
		SetTooltip (Value);
		return true;
	}
	else if( Name == PropertyName::LocalTooltip )
	{
		SetLocalTooltip (Value);
		return true;
	}
	else if( Name == PropertyName::TooltipStyle )
	{
		UIBaseObject * const NewStyle = GetObjectFromPath( Value, TUITooltipStyle );

		if( NewStyle || Value.empty() )
		{
			SetTooltipStyle( static_cast<UITooltipStyle *>(NewStyle) );
			RemoveProperty (Name);
			return true;
		}
		// Fall through
	}
	else if( Name == PropertyName::TooltipDelay )
	{
		bool tooltipDelay = true;
		
		if (!UIUtils::ParseBoolean(Value, tooltipDelay))
			return false;

		SetTooltipDelay(tooltipDelay);
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::PopupStyle )
	{
		UIPopupMenuStyle * const style = static_cast<UIPopupMenuStyle *>(GetObjectFromPath (Value, TUIPopupMenuStyle));

		if (style || Value.empty())
		{
			SetPopupStyle (style);
			RemoveProperty (Name);
			return true;
		}
		// Fall through
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::Visible )
	{
		bool localIsVisible;

		if( !UIUtils::ParseBoolean( Value, localIsVisible ) )
			return false;

		SetVisible( localIsVisible );
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::AbsorbsTab )
	{
		bool localAbsorbsTab;

		if( !UIUtils::ParseBoolean( Value, localAbsorbsTab ) )
			return false;

		SetAttribute( UIWidget::BF_AbsorbsTab, localAbsorbsTab );
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::Selectable )
	{
		bool b;

		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetSelectable ( b );
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::TabRoot )
	{
		bool b;

		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetTabRoot ( b );
		return true;
	}

	//----------------------------------------------------------------------

	else if (Name == PropertyName::CustomDragWidget)
	{
		UIWidget * const widget = static_cast<UIWidget *>(GetObjectFromPath( Value, TUIWidget ));

		if (widget || Value.empty())
		{
			SetCustomDragWidget (widget);
			RemoveProperty (Name);
			return true;
		}
		// Fall through
	}

	//-----------------------------------------------------------------

	else if ( Name == PropertyName::PackSize )
	{
		const size_t comma_pos = Value.find_first_of (',');

		UIString tokens [2];

		tokens [0] = Value.substr (0, comma_pos);

		if (comma_pos != UIString::npos)
			tokens [1] = Value.substr (comma_pos+1);

		bool found = false;

		for (size_t i = 0; i < 2; ++i)
		{
			const UIPacking::SizeInfo * sinfo = 0;

			const UINarrowString nstr = Unicode::wideToNarrow (tokens [i]);
			if (tokens [i].empty () == false)
			{
				sinfo = UIManager::gUIManager ().GetPackingManager ().getSizeInfo (nstr);
			}

			if (sinfo)
			{
				// only change the packsize if the new specification is different

				if (mPackSizes [i] == 0 || mPackSizes [i]->m_type != sinfo->m_type)
				{
					if (mPackSizes [i] == 0)
					{
						mPackSizes [i] = new UIPacking::SizeInfo;
					}

					*mPackSizes [i] = *sinfo;
					found = true;
				}
			}
			else {

				// try to make it a user-defined thang

				if (nstr.empty () == false && isdigit (nstr [0]))
				{
					if (mPackSizes [i] == 0)
					{
						mPackSizes [i]               = new UIPacking::SizeInfo;
					}

					mPackSizes [i]->m_type       = UIPacking::SizeInfo::UserDefined;
					mPackSizes [i]->m_userProportion = static_cast<float> (atof (nstr.c_str ()));
				}

				// just delete it
				else if (mPackSizes [i])
				{
					delete mPackSizes [i];
					mPackSizes [i] = 0;
				}
			}
		}

		if (found)
			ResetPackSizeInfo ();
	}
	//-----------------------------------------------------------------
	else if ( Name == PropertyName::PackLocation )
	{
		const size_t comma_pos = Value.find_first_of (',');

		UIString tokens [2];

		tokens [0] = Value.substr (0, comma_pos);

		if (comma_pos != UIString::npos)
			tokens [1] = Value.substr (comma_pos+1);

		bool found = false;

		for (size_t i = 0; i < 2; ++i)
		{
			const UIPacking::LocationInfo * sinfo = 0;

			const UINarrowString nstr = Unicode::wideToNarrow (tokens [i]);
			if (tokens [i].empty () == false)
			{
				sinfo = UIManager::gUIManager ().GetPackingManager ().getLocationInfo (nstr);
			}

			if (sinfo)
			{
				// only change the packsize if the new specification is different

				if (mPackLocations [i] == 0 || sinfo->rulesMatch (*mPackLocations [i]) == false)
				{

					if (mPackLocations [i] == 0)
					{
						mPackLocations [i] = new UIPacking::LocationInfo;
					}

					*mPackLocations [i] = *sinfo;
					found = true;
				}
			}
			else {

				// try to make it a user-defined thang

				if (nstr.empty () == false && isdigit (nstr [0]))
				{
					if (mPackLocations [i] == 0)
					{
						mPackLocations [i]               = new UIPacking::LocationInfo;
					}

					mPackLocations [i]->m_type       = UIPacking::LocationInfo::UserDefined;
					mPackLocations [i]->m_userProportion = static_cast<float> (atof (nstr.c_str ()));
				}

				// just delete it
				else if (mPackLocations [i])
				{
					delete mPackLocations [i];
					mPackLocations [i] = 0;
				}
			}
		}

		if (found)
			ResetPackLocationInfo ();

	}

	//----------------------------------------------------------------------
	//-- background color

	else if( Name ==  PropertyName::BackgroundColor )
	{
		UIColor color;
		if( !UIUtils::ParseColor( Value, color ) )
			return false;

		color.a = mBackgroundColor.a;
		SetBackgroundColor (color);
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name ==  PropertyName::BackgroundColorA )
	{
		UIColor color = mBackgroundColor;
		color.a = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetBackgroundColor (color);
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name ==  PropertyName::BackgroundColorR )
	{
		UIColor color = mBackgroundColor;
		color.r = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetBackgroundColor (color);
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name ==  PropertyName::BackgroundColorG )
	{
		UIColor color = mBackgroundColor;
		color.g = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetBackgroundColor (color);
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name ==  PropertyName::BackgroundColorB )
	{
		UIColor color = mBackgroundColor;
		color.b = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetBackgroundColor (color);
		return true;
	}

	//----------------------------------------------------------------------
	//-- background tint

	else if( Name == PropertyName::BackgroundTint )
	{
		UIColor color;
		if( !UIUtils::ParseColor( Value, color ) )
			return false;

		SetBackgroundTint (color);
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::BackgroundTintA )
	{
		UIColor color = mBackgroundTint;
		color.a = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetBackgroundTint(color);
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::BackgroundTintR )
	{
		UIColor color = mBackgroundTint;
		color.r = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetBackgroundTint(color);
		return true;

	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::BackgroundTintG )
	{
		UIColor color = mBackgroundTint;
		color.g = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetBackgroundTint(color);
		return true;

	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::BackgroundTintB )
	{
		UIColor color = mBackgroundTint;
		color.b = static_cast<unsigned char>(atoi(Unicode::wideToNarrow(Value).c_str()));
		SetBackgroundTint(color);
		return true;
	}

	//----------------------------------------------------------------------
	//----------------------------------------------------------------------

	else if( Name == PropertyName::BackgroundScrolls )
	{
		bool b;
		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetAttribute (BF_BackgroundScrolls, b);
		return true;
	}

	//----------------------------------------------------------------------
	//-- background opacity

	else if( Name == PropertyName::BackgroundOpacity )
	{
		float Opacity;

		if( !UIUtils::ParseFloat( Value, Opacity ) )
			return false;

		SetBackgroundOpacity (Opacity);

		return true;
	}

	//----------------------------------------------------------------------

	else if(  Name == PropertyName::Focus )
	{
		bool b;

		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetFocus();
		return true;
	}
	else if(  Name == PropertyName::ScrollSizePage )
	{
		return UIUtils::ParsePoint( Value, mScrollSizePage );
	}
	else if(  Name == PropertyName::ScrollSizeLine )
	{
		return UIUtils::ParsePoint( Value, mScrollSizeLine );
	}
	else if( Name == MethodName::EffectorExecute )
	{
		UIEffector * const effector = static_cast<UIEffector *>(GetObjectFromPath( Value, TUIEffector ));

		if (effector)
		{
			ExecuteEffector(*effector);
			return true;
		}
		return false;
	}
	else if( Name == MethodName::EffectorCancel )
	{
		UIEffector * const effector = static_cast<UIEffector *>(GetObjectFromPath( Value, TUIEffector ));

		if (effector)
		{
			CancelEffector(*effector);
			return true;
		}
		return false;
	}
	else if( Name == MethodName::RunScript )
	{
		return RunScript();
	}
	else if (Name == PropertyName::ShrinkWrap )
	{
		bool shrink = false;

		if (!UIUtils::ParseBoolean(Value, shrink))
			return false;

		SetShrinkWrap(shrink);
		return true;
	}
	else if (Name == PropertyName::AutoRegister)
	{
		bool reg = false;

		if (!UIUtils::ParseBoolean(Value, reg))
			return false;

		SetAutoRegister(reg);
		return true;
	}
	else if( Name == PropertyName::ForwardMoveToParent )
	{
		bool b;
		if(!UIUtils::ParseBoolean(Value, b))
			return false;

		SetForwardMoveToParent(b);
		return true;
	}
	else if( Name == PropertyName::AcceptsMoveFromChildren )
	{
		bool b;
		if (!UIUtils::ParseBoolean(Value, b))
			return false;
		SetAcceptsMoveFromChildren(b);
		return true;
	}
	else
	{
		if (mRectangleStyles->SetProperty (*this, Name, Value))
			return true;

		UIPalette::SetPropertyForObject (*this, Name, Value);
	}
	//-----------------------------------------------------------------

	return UIBaseObject::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIWidget::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	DEBUG_DESTROYED();
	
	//----------------------------------------------------------------------

	if( Name == PropertyName::Color )
		return UIUtils::FormatColor( Value, mColor );
	else if( Name == PropertyName::ColorR )
		return UIUtils::FormatInteger( Value, mColor.r );
	else if( Name == PropertyName::ColorG )
		return UIUtils::FormatInteger( Value, mColor.g );
	else if( Name == PropertyName::ColorB )
		return UIUtils::FormatInteger( Value, mColor.b );
	else if( Name == PropertyName::PalShade )
		return UIUtils::FormatFloat(Value, mPalShade);

	//----------------------------------------------------------------------

	else if( Name == PropertyName::Cursor )
	{
		if( mCursor )
		{
			GetPathTo( Value, mCursor );
			return true;
		}
	}
	//----------------------------------------------------------------------
	else if( Name == PropertyName::CursorSet )
	{
		if( mCursorSet )
		{
			GetPathTo( Value, mCursorSet );
			return true;
		}
	}
	else if( Name == PropertyName::Dragable )
		return UIUtils::FormatBoolean( Value, IsDragable() );
	else if( Name == PropertyName::DropToParent )
		return UIUtils::FormatBoolean( Value, HasAttribute (BF_DropToParent) );
	else if( Name == PropertyName::ContextToParent)
		return UIUtils::FormatBoolean( Value, HasAttribute (BF_ContextToParent) );
	else if( Name == PropertyName::ContextCapable)
		return UIUtils::FormatBoolean( Value, IsContextCapable (true));
	else if( Name == PropertyName::ContextCapableAlternate)
		return UIUtils::FormatBoolean( Value, IsContextCapable (false));
	else if( Name == PropertyName::DragBadCursor )
	{
		if( mDragBadCursor )
		{
			GetPathTo( Value, mDragBadCursor );
			return true;
		}
	}
	else if( Name == PropertyName::DragGoodCursor )
	{
		if( mDragGoodCursor )
		{
			GetPathTo( Value, mDragGoodCursor );
			return true;
		}
	}
	else if( Name == PropertyName::Activated )
		return UIUtils::FormatBoolean( Value, IsActivated() );
	else if( Name == PropertyName::Enabled )
		return UIUtils::FormatBoolean( Value, IsEnabled() );
	else if( Name == PropertyName::GetsInput )
		return UIUtils::FormatBoolean( Value, HasAttribute (BF_GetsInput));
	else if( Name == PropertyName::UserMovable )
		return UIUtils::FormatBoolean( Value, IsUserMovable() );
	else if( Name == PropertyName::UserDragScrollable )
		return UIUtils::FormatBoolean( Value, IsUserDragScrollable() );
	else if( Name == PropertyName::AbsorbsInput )
		return UIUtils::FormatBoolean( Value, IsAbsorbsInput() );
	else if( Name == PropertyName::UserResizable )
		return UIUtils::FormatBoolean( Value, IsUserResizable() );

	//----------------------------------------------------------------------

	else if( Name == PropertyName::Location )
		return UIUtils::FormatPoint( Value, mLocation );
	else if( Name == PropertyName::LocationX )
		return UIUtils::FormatInteger( Value, mLocation.x );
	else if( Name == PropertyName::LocationY )
		return UIUtils::FormatInteger( Value, mLocation.y );

	//----------------------------------------------------------------------

	else if( Name == PropertyName::Opacity )
		return UIUtils::FormatFloat( Value, mOpacity );
	else if( Name == PropertyName::OpacityRelativeMin )
		return UIUtils::FormatFloat( Value, mOpacityRelativeMin );
	else if( Name == PropertyName::TextOpacityRelativeMin )
		return UIUtils::FormatFloat( Value, mTextOpacityRelativeMin );
	else if( Name == PropertyName::TextOpacityRelativeApply )
		return UIUtils::FormatBoolean( Value, HasAttribute (BF_TextOpacityRelativeApply) );
	else if( Name == PropertyName::Rotation )
	{
		if( mRotation == 0.0f )
		{
			Value.erase();
			return true;
		}
		else
			return UIUtils::FormatFloat( Value, mRotation );
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::Size )
		return UIUtils::FormatPoint( Value, mSize );
	else if( Name == PropertyName::SizeX )
		return UIUtils::FormatInteger( Value, mSize.x );
	else if( Name == PropertyName::SizeY )
		return UIUtils::FormatInteger( Value, mSize.y );

	//----------------------------------------------------------------------

	else if( Name == PropertyName::LockDiagonal )
		return UIUtils::FormatBoolean( Value, mLockDiagonal );

	else if( Name == PropertyName::ResizeInset )
		return UIUtils::FormatInteger( Value, mResizeInset );
	//----------------------------------------------------------------------

	else if( Name == PropertyName::SizeIncrement )
		return UIUtils::FormatPoint( Value, mSizeIncrement );
	else if( Name == PropertyName::MinimumScrollExtent )
		return UIUtils::FormatPoint( Value, mMinimumScrollExtent );

	//----------------------------------------------------------------------

	else if( Name == PropertyName::MinimumSize )
		return UIUtils::FormatPoint( Value, mMinimumSize );
	else if( Name == PropertyName::MinimumSizeX )
		return UIUtils::FormatInteger( Value, mMinimumSize.x );
	else if( Name == PropertyName::MinimumSizeY )
		return UIUtils::FormatInteger( Value, mMinimumSize.y );

	//----------------------------------------------------------------------

	else if( Name == PropertyName::MaximumSize )
		return UIUtils::FormatPoint( Value, mMaximumSize );
	else if( Name == PropertyName::MaximumSizeX )
		return UIUtils::FormatInteger( Value, mMaximumSize.x );
	else if( Name == PropertyName::MaximumSizeY )
		return UIUtils::FormatInteger( Value, mMaximumSize.y );

	//----------------------------------------------------------------------
	//-- scrollextents must be gotten via the virtual accessor
	else if( Name == PropertyName::ScrollExtent )
	{
		UIPoint point;
		GetScrollExtent (point);
		return UIUtils::FormatPoint( Value, point );
	}
	else if( Name == PropertyName::ScrollLocation )
		return UIUtils::FormatPoint( Value, mScrollLocation );
	else if( Name == PropertyName::Tooltip )
	{
		Value = GetTooltip();
		return true;
	}
	else if( Name == PropertyName::LocalTooltip )
	{
		Value = GetLocalTooltip();
		return true;
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::TooltipStyle )
	{
		if (mTooltipStyle)
		{
			GetPathTo( Value, mTooltipStyle );
			return true;
		}
	}

	//----------------------------------------------------------------------
	else if (Name == PropertyName::TooltipDelay)
		return UIUtils::FormatBoolean(Value, GetTooltipDelay());

	//----------------------------------------------------------------------

	else if( Name == PropertyName::PopupStyle)
	{
		if (mPopupStyle)
		{
			GetPathTo (Value, mPopupStyle);
			return true;
		}
	}

	//----------------------------------------------------------------------

	else if (Name ==PropertyName::CustomDragWidget)
	{
		if (mCustomDragWidget)
		{
			GetPathTo( Value, mCustomDragWidget );
			return true;
		}
	}

	//----------------------------------------------------------------------

	else if( Name == PropertyName::Visible )
		return UIUtils::FormatBoolean( Value, IsVisible() );

	else if( Name == PropertyName::AbsorbsTab )
		return UIUtils::FormatBoolean( Value, HasAttribute(BF_AbsorbsTab) );

	else if( Name == PropertyName::Selectable )
		return UIUtils::FormatBoolean( Value, IsSelectable () );

	else if( Name == PropertyName::TabRoot )
		return UIUtils::FormatBoolean( Value, IsTabRoot () );

	//----------------------------------------------------------------------

	else if( Name == PropertyName::BackgroundColor )
		return UIUtils::FormatColor( Value, mBackgroundColor );
	else if( Name == PropertyName::BackgroundColorA )
		return UIUtils::FormatInteger( Value, mBackgroundColor.a );
	else if( Name == PropertyName::BackgroundColorR )
		return UIUtils::FormatInteger( Value, mBackgroundColor.r );
	else if( Name == PropertyName::BackgroundColorG )
		return UIUtils::FormatInteger( Value, mBackgroundColor.g );
	else if( Name == PropertyName::BackgroundColorB )
		return UIUtils::FormatInteger( Value, mBackgroundColor.b );

	//----------------------------------------------------------------------

	else if( Name == PropertyName::BackgroundTint )
		return UIUtils::FormatColor( Value, mBackgroundTint );
	else if( Name == PropertyName::BackgroundTintA )
		return UIUtils::FormatInteger( Value, mBackgroundTint.a );
	else if( Name == PropertyName::BackgroundTintR )
		return UIUtils::FormatInteger( Value, mBackgroundTint.r );
	else if( Name == PropertyName::BackgroundTintG )
		return UIUtils::FormatInteger( Value, mBackgroundTint.g );
	else if( Name == PropertyName::BackgroundTintB )
		return UIUtils::FormatInteger( Value, mBackgroundTint.b );

	//----------------------------------------------------------------------

	else if( Name == PropertyName::BackgroundScrolls )
		return UIUtils::FormatBoolean (Value, HasAttribute (BF_BackgroundScrolls));

	else if( Name == PropertyName::BackgroundOpacity )
		return UIUtils::FormatFloat( Value, static_cast<float>(mBackgroundColor.a) / 255.0f );
	else if(  Name == PropertyName::ScrollSizePage )
	{
		return UIUtils::FormatPoint( Value, mScrollSizePage );
	}
	else if(  Name == PropertyName::ScrollSizeLine )
	{
		return UIUtils::FormatPoint( Value, mScrollSizeLine );
	}
	else if (Name == PropertyName::ShrinkWrap)
		return UIUtils::FormatBoolean(Value, GetShrinkWrap());
	else if (Name == PropertyName::AutoRegister)
		return UIUtils::FormatBoolean(Value, GetAutoRegister());
	else if (Name == PropertyName::ForwardMoveToParent)
		return UIUtils::FormatBoolean(Value, GetForwardMoveToParent());
	else if (Name == PropertyName::AcceptsMoveFromChildren)
		return UIUtils::FormatBoolean(Value, GetAcceptsMoveFromChildren());	
	else
	{
		if (mRectangleStyles->GetProperty (*this, Name, Value))
			return true;
	}

	return UIBaseObject::GetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool  UIWidget::PackSelfSize()
{
	DEBUG_DESTROYED();
	
	bool retval = false;

	UISize pSize;
	
	if (GetParent () && GetParent ()->IsA (TUIWidget))
		static_cast<UIWidget *>(GetParent ())->GetScrollExtent (pSize);
	else
		return false;
	
	using namespace UIPacking;
	
	UISize newSize = mSize;
	
	Rational oldProps [2];
	
	if (mPackSizes [0])
	{
		newSize.x = mPackSizes [0]->performPacking ( mLocation.x, mLocation.x + mSize.x, pSize.x);
		retval = true;
		oldProps [0] = mPackSizes [0]->m_proportion;
	}
	
	if (mPackSizes [1])
	{
		newSize.y = mPackSizes [1]->performPacking (mLocation.y, mLocation.y + mSize.y, pSize.y);
		retval = true;
		oldProps [1] = mPackSizes [1]->m_proportion;
	}
	
	//-- a change in size can cause a widget's pack location proportions to
	//-- change as well.  This occurs if the widget packs from its center or far edge
	
	Rational oldLocProps [2];
	
	if (mPackLocations [0])
		oldLocProps [0] = mPackLocations [0]->m_proportion;
	
	if (mPackLocations [1])
		oldLocProps [1] = mPackLocations [1]->m_proportion;
	
	
	//-- restore previous packing proportions
	//-- sizing a widget as a result of a pack should not change its proportions
	
	if (retval) 
	{
		SetSize (newSize);
		
		if (mPackSizes [0])
			mPackSizes [0]->m_proportion = oldProps [0];
		
		if (mPackSizes [1])
			mPackSizes [1]->m_proportion = oldProps [1];
		
		if (mPackLocations [0])
			mPackLocations [0]->m_proportion = oldLocProps [0];
		
		if (mPackLocations [1])
			mPackLocations [1]->m_proportion = oldLocProps [1];
	}

	return retval;
}

//-----------------------------------------------------------------

bool  UIWidget::PackSelfLocation ()
{
	DEBUG_DESTROYED();
	
	bool retval = false;

	UISize pSize;
	
	if (GetParent () && GetParent ()->IsA (TUIWidget))
		static_cast<UIWidget *>(GetParent ())->GetScrollExtent (pSize);
	else
		return false;
	
	using namespace UIPacking;
	
	UIPoint newLocation = mLocation;
	
	
	Rational oldLocProps [2];
	
	if (mPackLocations [0])
	{
		newLocation.x = mPackLocations [0]->performPacking (mLocation.x,mLocation.x + mSize.x,pSize.x);
		retval = true;
		oldLocProps [0] = mPackLocations [0]->m_proportion;
	}
	
	if (mPackLocations [1])
	{
		newLocation.y = mPackLocations [1]->performPacking (mLocation.y,mLocation.y + mSize.y,pSize.y);
		retval = true;
		oldLocProps [1] = mPackLocations [1]->m_proportion;
	}
	
	//-- restore previous packing proportions
	//-- moving a widget as a result of a pack should not change its proportions
	
	if (retval)
	{
		SetLocation (newLocation);
		
		if (mPackLocations [0])
			mPackLocations [0]->m_proportion = oldLocProps [0];
		
		if (mPackLocations [1])
			mPackLocations [1]->m_proportion = oldLocProps [1];
	}

	return retval;
}

//-----------------------------------------------------------------

void UIWidget::ResetPackSizeInfo ()
{
	DEBUG_DESTROYED();
	
	// reset packing proportions
	if (GetParent () && GetParent ()->IsA (TUIWidget))
	{
		UISize pSize;
		static_cast<UIWidget *>(GetParent ())->GetScrollExtent (pSize);

		//-- do pack sizes first
		if (mPackSizes [0])
			mPackSizes [0]->resetInfo (mSize.x, pSize.x);

		if (mPackSizes [1])
			mPackSizes [1]->resetInfo (mSize.y, pSize.y);
	}
}

//-----------------------------------------------------------------

void UIWidget::ResetPackLocationInfo ()
{
	DEBUG_DESTROYED();
	
	// reset packing proportions
	if (GetParent () && GetParent ()->IsA (TUIWidget))
	{
		UISize pSize;
		static_cast<UIWidget *>(GetParent ())->GetScrollExtent (pSize);

		//-- next do pack locations
		if (mPackLocations [0])
		{
			mPackLocations [0]->resetInfo (mLocation.x, mLocation.x + mSize.x, pSize.x);
		}
		if (mPackLocations [1])
		{
			mPackLocations [1]->resetInfo (mLocation.y, mLocation.y + mSize.y, pSize.y);
		}
	}
}

//-----------------------------------------------------------------

void  UIWidget::ScrollToBottom ()
{
	DEBUG_DESTROYED();
	
	UIPoint  ScrollExtent;
	GetScrollExtent ( ScrollExtent );
	SetScrollLocation (UIPoint (0, ScrollExtent.y - mSize.y));
}

//----------------------------------------------------------------------

void UIWidget::ScrollToPoint (const UIPoint & pt)
{
	DEBUG_DESTROYED();
	
	UIPoint  ScrollExtent;
	UISize   ScrollPageSize;
	UISize   ScrollLineSize;

	GetScrollExtent ( ScrollExtent );
	GetScrollSizes  ( ScrollPageSize, ScrollLineSize );

	SetScrollLocation ( pt );

	UIPoint thePoint (pt);

	thePoint.x = std::min (thePoint.x, ScrollExtent.x - mSize.x);
	thePoint.y = std::min (thePoint.y, ScrollExtent.y - mSize.y);

	thePoint.x = std::max (0L, thePoint.x);
	thePoint.y = std::max (0L, thePoint.y);

	SetScrollLocation (thePoint);
}

//----------------------------------------------------------------------

void  UIWidget::RenderDefault ( UICanvas & canvas, const UIWidgetRectangleStyles * fallback) const
{
	DEBUG_DESTROYED();
	
	//----------------------------------------------------------------------
	//-- only draw the background if the style has no center piece (which prob. means it has nothing else)

	if (mBackgroundColor.a != 0 )
	{
		const UIColor preBackgroundColor (canvas.GetColor ());
		const UIRectangleStyle * const style = mRectangleStyles->GetAppropriateStyle (*this, fallback);
		const float OldOpacity    = canvas.GetOpacity();
		static const float oo_255 = static_cast<float>(1.0f / 255.0f);

		canvas.ModifyColor   (mBackgroundTint);
		canvas.ModifyOpacity (static_cast<float> (mBackgroundColor.a) * oo_255);

		if (!style || !style->GetImage (UIRectangleStyle::I_Center))
		{
			const UIColor OldColor   = canvas.GetColor();
			canvas.ModifyColor( mBackgroundColor );
			//-- blt only into size to avoid clipping to scroll extent

			const UIPoint location (GetScrollLocation ());

			canvas.BltFrom( 0, UIPoint::zero, location, GetSize() );
			canvas.SetColor( OldColor );
		}

		if (style)
		{
			if (HasAttribute (BF_BackgroundScrolls))
			{
				UISize scrollExtent;
				GetScrollExtent (scrollExtent);
				style->Render (GetAnimationState (), canvas, scrollExtent);
			}
			else
			{
				const UIPoint & scrollLocation = GetScrollLocation ();
				canvas.Translate (scrollLocation);
				style->Render    (GetAnimationState (), canvas, GetSize ());
				canvas.Translate (-scrollLocation);
			}
		}

		canvas.SetColor (preBackgroundColor);
		canvas.SetOpacity( OldOpacity );
	}
}

//-----------------------------------------------------------------

void UIWidget::Render( UICanvas & canvas) const
{
	DEBUG_DESTROYED();
	
	RenderDefault (canvas, 0);
}

//----------------------------------------------------------------------

void UIWidget::SetMinimumScrollExtent (const UISize & size)
{
	DEBUG_DESTROYED();
	
	//----------------------------------------------------------------------
	//-- clamp minimumSize to maximumSize

	mMinimumScrollExtent.x = std::max (0L, size.x);
	mMinimumScrollExtent.y = std::max (0L, size.y);

	//-- force a size update if neede
	SetScrollExtent (mScrollExtent);
}

//----------------------------------------------------------------------

void UIWidget::SetMinimumSize (const UISize & size)
{
	DEBUG_DESTROYED();
	
	//----------------------------------------------------------------------
	//-- clamp minimumSize to maximumSize

	mMinimumSize.x = std::min (std::max (0L, size.x), mMaximumSize.x);
	mMinimumSize.y = std::min (std::max (0L, size.y), mMaximumSize.y);

	//-- force a size update if needed
	SetSize (mSize);
}

//----------------------------------------------------------------------

void UIWidget::SetMaximumSize (const UISize & size)
{
	DEBUG_DESTROYED();
	
	//----------------------------------------------------------------------
	//-- clamp maximumSize to minimumSize

	mMaximumSize.x = std::max (size.x, mMinimumSize.x);
	mMaximumSize.y = std::max (size.y, mMinimumSize.y);

	//-- force a size update if needed
	SetSize (mSize);
}

//----------------------------------------------------------------------

void UIWidget::SetLockDiagonal (const bool b)
{
	DEBUG_DESTROYED();

	mLockDiagonal = b;
}

//----------------------------------------------------------------------

void UIWidget::SetResizeInset (const unsigned char c)
{
	DEBUG_DESTROYED();

	mResizeInset = c;
}

//-----------------------------------------------------------------

UIWidget * UIWidget::GetFocusedLeafWidget ()
{
	DEBUG_DESTROYED();
	
	return IsSelected () ? this : 0;
}

//-----------------------------------------------------------------

/**
* The implementor of this function assumes that the DragType has already been found
* as a valid type for dropping on this widget.  At this point the implementor may
* override the return value and return false if some areas within a widget (e.g. a 3d
* viewer or a gridbag) are not droppable.  Also, the implementor may use this
* as an opportunity to change the draggood cursor.  Changing the dragbad cursor
* is not reliable, as this function is not always called on bad drags, only
* potentially good ones.
*
* By default, this method returns true if the BF_DropFlagOk flag is set.
*/
bool UIWidget::IsDropOk (const UIWidget & , const UIString & , const UIPoint & )
{
	DEBUG_DESTROYED();
	
	return HasAttribute (BF_DropFlagOk);
}

//----------------------------------------------------------------------
/**
* Override to return special widgets for dragging.  By default the method
* returns this pointer.  Subclasses should return the value of this base
* class method as a default return value.
*
* A null return is valid, in which case no drag occurs.
*/

UIWidget * UIWidget::GetCustomDragWidget (const UIPoint & pt, UIPoint & offset )
{
	DEBUG_DESTROYED();
	
	offset = -pt;

	if (mCustomDragWidget)
	{
		if (mCustomDragWidget->GetParent () == 0)
			offset = -(mCustomDragWidget->GetSize () / 2L);
		else
			offset -= (mCustomDragWidget->GetWorldLocation () + (mCustomDragWidget->GetSize () / 2L)) - GetWorldLocation ();

		//-- offsets for custom widgets must be negative
		offset.x = std::min (0L, offset.x);
		offset.y = std::min (0L, offset.y);

		offset.x = -std::min (mCustomDragWidget->GetWidth (),  -offset.x);
		offset.y = -std::min (mCustomDragWidget->GetHeight (), -offset.y);

		return mCustomDragWidget;
	}

	return this;
}

//----------------------------------------------------------------------

void  UIWidget::SetSizeIncrement (const UISize & size)
{
	DEBUG_DESTROYED();
	
	mSizeIncrement = size;
}

//----------------------------------------------------------------------

namespace
{
	typedef std::list<UIString> UIStringList;
	bool ParseList( const UIString &In, UIStringList &Out )
	{
		UIString CurrentItem;
		UIString::value_type QuoteCharacter = 0;

		for( UIString::const_iterator i = In.begin(); i != In.end(); ++i )
		{
			if( QuoteCharacter )
			{
				if( *i == QuoteCharacter )
					QuoteCharacter = 0;
				else
					CurrentItem.append( 1, *i );
			}
			else if( (*i == '\'') || (*i == '\"') )
			{
				QuoteCharacter = *i;
			}
			else if( isspace( *i ) || *i == ',' )
			{
				if( !CurrentItem.empty() )
				{
					Out.push_back( CurrentItem );
					CurrentItem.erase();
				}
			}
			else
			{
				CurrentItem.append( 1, *i );
			}
		}

		if( !CurrentItem.empty() )
			Out.push_back( CurrentItem );

		return true;
	}
}

//----------------------------------------------------------------------

bool  UIWidget::AcceptsDragType (const Unicode::String & dragType)
{
	DEBUG_DESTROYED();
	
	Unicode::String DragAccepts;
	if (GetProperty( UIWidget::PropertyName::DragAccepts, DragAccepts ))
	{
		UIStringList AcceptedDragTypes;

		if( ParseList( DragAccepts, AcceptedDragTypes ) )
		{
			for( UIStringList::iterator i = AcceptedDragTypes.begin(); i != AcceptedDragTypes.end(); ++i )
			{
				if(Unicode::caseInsensitiveCompare (dragType, *i))
					return true;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

/**
* Get the first Widget-class parent of this widget
*/
UIWidget* UIWidget::GetParentWidget ()
{
	DEBUG_DESTROYED();
	
	UIBaseObject * theParent = GetParent ();
	while (theParent && !theParent->IsA (TUIWidget))
		theParent = theParent->GetParent ();

	return static_cast<UIWidget *>(theParent);
}

//----------------------------------------------------------------------

/**
* Get the first Widget-class parent of this widget
*/

const UIWidget *UIWidget::GetParentWidget () const
{
	DEBUG_DESTROYED();
	
	const UIBaseObject * theParent = GetParent ();
	while (theParent && !theParent->IsA (TUIWidget))
		theParent = theParent->GetParent ();

	return static_cast<const UIWidget *>(theParent);
}

//----------------------------------------------------------------------

bool UIWidget::SetAttribute (AttributeBitFlags attribute, bool value)
{
	DEBUG_DESTROYED();
	
	const int bit = static_cast<int>(attribute);

	if( value )
	{
		if ((mAttributeBits & bit) == 0)
		{
			mAttributeBits |=  static_cast<int>(attribute);
			return true;
		}
	}
	else
	{
		if ((mAttributeBits & bit) != 0)
		{
			mAttributeBits &= ~static_cast<int>(attribute);
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

const UICursor *UIWidget::GetMouseCursor() const
{
	DEBUG_DESTROYED();
	
	if (mCurrentUserModificationType == UMT_NONE)
		return mCursor;

	if ((!HasAttribute(BF_UserModifying) || !s_hasDragMovedYet) && mNotModifyingUseDefaultCursor)
		return mCursor;

	return GetUserModificationCursor (mCurrentUserModificationType);
}

//----------------------------------------------------------------------

UICursor  *UIWidget::GetMouseCursor()
{
	DEBUG_DESTROYED();
	
	if (mCurrentUserModificationType == UMT_NONE)
		return mCursor;
	
	if ((!HasAttribute(BF_UserModifying) || !s_hasDragMovedYet) && mNotModifyingUseDefaultCursor)
		return mCursor;

	return GetUserModificationCursor (mCurrentUserModificationType);
}

//----------------------------------------------------------------------

UIWidget * UIWidget::FindFirstDragableWidget ()
{
	DEBUG_DESTROYED();
	
	if (IsDragable ())
		return this;

	if (GetParent () && GetParent ()->IsA (TUIWidget))
		return static_cast<UIWidget *>(GetParent ())->FindFirstDragableWidget ();

	return 0;
}

//----------------------------------------------------------------------

UIWidget * UIWidget::FindFirstContextCapableWidget (bool primary)
{
	DEBUG_DESTROYED();
	
	if (IsContextCapable (primary))
		return this;

	if (IsContextToParent () && GetParent () && GetParent ()->IsA (TUIWidget))
		return static_cast<UIWidget *>(GetParent ())->FindFirstContextCapableWidget (primary);

	return 0;
}

//----------------------------------------------------------------------

UIWidget * UIWidget::FindTopMoveParentWidget()
{
	DEBUG_DESTROYED();

	UIBaseObject * parent = GetParent();

	if(parent && parent->IsA(TUIWidget) && GetForwardMoveToParent())
	{
		UIWidget * parentWidget = static_cast<UIWidget *>(parent);
		UIWidget * topmost = parentWidget->FindTopMoveParentWidget ();
		
		return topmost ? topmost : parentWidget;
	}
	
	return 0;
}

//----------------------------------------------------------------------

void UIWidget::SetCursorSet (UICursorSet * cursorSet)
{
	DEBUG_DESTROYED();
	
	AttachMember (mCursorSet, cursorSet);
}

//-----------------------------------------------------------------

void UIWidget::CopyPropertiesFrom( const UIBaseObject & rhs)
{
	DEBUG_DESTROYED();
	
	UIBaseObject::CopyPropertiesFrom (rhs);

	if (rhs.IsA (TUIWidget))
	{
		UIWidget const & rhs_widget = static_cast<UIWidget const &>(rhs);

		SetColor(rhs_widget.GetColor());
		SetDragable(rhs_widget.IsDragable());
		SetUserDragScrollable(rhs_widget.IsUserDragScrollable());
		SetDropToParent(rhs_widget.IsDropToParent());
		SetAttribute(BF_ContextToParent, rhs_widget.HasAttribute(BF_ContextToParent));
		SetContextCapable(rhs_widget.IsContextCapable(true), true);
		SetContextCapable(rhs_widget.IsContextCapable(false), false);
		SetActivated(rhs_widget.IsActivated());
		SetEnabled(rhs_widget.IsEnabled());
		SetGetsInput(rhs_widget.GetsInput());
		SetUserMovable(rhs_widget.IsUserMovable());
		SetAbsorbsInput(rhs_widget.IsAbsorbsInput());
		SetUserResizable(rhs_widget.IsUserResizable());
		SetLocation(rhs_widget.GetLocation());
		SetOpacity(rhs_widget.GetOpacity());
		SetOpacityRelativeMin(rhs_widget.GetOpacityRelativeMin());
		SetAttribute (BF_TextOpacityRelativeApply, rhs_widget.HasAttribute(BF_TextOpacityRelativeApply));
		mTextOpacityRelativeMin = rhs_widget.mTextOpacityRelativeMin;
		SetRotation(rhs_widget.GetRotation());
		SetSize(rhs_widget.GetSize());
		SetSizeIncrement(rhs_widget.GetSizeIncrement());
		SetMinimumScrollExtent(rhs_widget.GetMinimumScrollExtent());
		SetMinimumSize(rhs_widget.GetMinimumSize());
		SetMaximumSize(rhs_widget.GetMaximumSize());
		SetLockDiagonal(rhs_widget.GetLockDiagonal());
		SetResizeInset(rhs_widget.GetResizeInset());
		UISize scrollExtent;
		rhs_widget.GetScrollExtent(scrollExtent);
		SetScrollExtent(scrollExtent);
		SetScrollLocation(rhs_widget.GetScrollLocation());
		SetTooltip(rhs_widget.GetTooltip());
		SetVisible(rhs_widget.IsVisible());
		SetAttribute(BF_AbsorbsTab, rhs_widget.HasAttribute(BF_AbsorbsTab));
		SetSelectable(rhs_widget.IsSelectable());
		SetTabRoot(rhs_widget.IsTabRoot());
		SetBackgroundColor(rhs_widget.GetBackgroundColor());
		SetBackgroundTint(rhs_widget.GetBackgroundTint());
		SetAttribute (BF_BackgroundScrolls, rhs_widget.HasAttribute(BF_BackgroundScrolls));
		SetBackgroundOpacity(rhs_widget.GetBackgroundOpacity());
		mScrollSizePage = rhs_widget.mScrollSizePage;
		mScrollSizeLine = rhs_widget.mScrollSizeLine;
		UIPalette::CopyPaletteEntriesForObject(TUIWidget, *this, rhs);
		SetShrinkWrap(rhs_widget.GetShrinkWrap());
		SetAutoRegister(rhs_widget.GetAutoRegister());
		SetForwardMoveToParent(rhs_widget.GetForwardMoveToParent());
		SetAcceptsMoveFromChildren(rhs_widget.GetAcceptsMoveFromChildren());
		mPalShade = rhs_widget.mPalShade;

		//-- do pack sizes first
		if (mPackSizes [0] && rhs_widget.mPackSizes [0])
			*mPackSizes [0] = *rhs_widget.mPackSizes [0];
		if (mPackSizes [1] && rhs_widget.mPackSizes [1])
			*mPackSizes [1] = *rhs_widget.mPackSizes [1];

		//-- next do pack locations
		if (mPackLocations [0] && rhs_widget.mPackLocations [0])
			*mPackLocations [0] = *rhs_widget.mPackLocations [0];
		if (mPackLocations [1] && rhs_widget.mPackLocations [1])
			*mPackLocations [1] = *rhs_widget.mPackLocations [1];
	}
}

//----------------------------------------------------------------------

UIBaseObject * UIWidget::DuplicateObject () const
{
	DEBUG_DESTROYED();
	
	if (IsTransient ())
		return 0;

	UIWidget * const widget = static_cast<UIWidget *>(UIBaseObject::DuplicateObject ());

	if (widget)
	{
		//-- do pack sizes first
		if (mPackSizes [0] && widget->mPackSizes [0])
			*widget->mPackSizes [0] = *mPackSizes [0];
		if (mPackSizes [1] && widget->mPackSizes [1])
			*widget->mPackSizes [1] = *mPackSizes [1];

		//-- next do pack locations
		if (mPackLocations [0] && widget->mPackLocations [0])
			*widget->mPackLocations [0] = *mPackLocations [0];
		if (mPackLocations [1] && widget->mPackLocations [1])
			*widget->mPackLocations [1] = *mPackLocations [1];
	}

	return widget;
}

//-----------------------------------------------------------------

void UIWidget::SetLocation( const UIPoint &Location, bool const center )
{
	DEBUG_DESTROYED();
	
	SetLocation(Location.x, Location.y, center);
}

//----------------------------------------------------------------------

void UIWidget::SetBackgroundColor( const UIColor &In )
{
	DEBUG_DESTROYED();
	
	const unsigned char a = mBackgroundColor.a;
	mBackgroundColor = In;
	mBackgroundColor.a = a;
}

//----------------------------------------------------------------------

void UIWidget::SetBackgroundTint( const UIColor &In )
{
	DEBUG_DESTROYED();
	
	mBackgroundTint = In;
}

//-----------------------------------------------------------------

void UIWidget::Center(bool const Horizontal, bool const Vertical)
{
	DEBUG_DESTROYED();
	
	UIWidget const * const parent = GetParentWidget ();
	if (parent)
	{
		UIPoint newLocaltion((parent->GetSize() - mSize) / 2);

		if (!Horizontal)
		{
			newLocaltion.x = mLocation.x;
		}
		
		if (!Vertical)
		{
			newLocaltion.y = mLocation.y;
		}

		SetLocation(newLocaltion);
	}
}

//-----------------------------------------------------------------

void UIWidget::Center(UIPoint const & pos, bool const Horizontal, bool const Vertical)
{
	DEBUG_DESTROYED();
	
	UIPoint const & halfSize = mSize / 2;

	UIPoint location(pos - halfSize);

	if (!Horizontal)
	{
		location.x = pos.x;
	}
	
	if (!Vertical)
	{
		location.y = pos.y;
	}

	SetLocation(location);
}

//-----------------------------------------------------------------

void UIWidget::addBoundary    (UIBoundary & boundary)
{
	DEBUG_DESTROYED();
	
	if (!mBoundaries)
		mBoundaries = new UIWidgetBoundaries (*this);

	mBoundaries->addBoundary (boundary);
}

//-----------------------------------------------------------------

void UIWidget::removeBoundary (UIBoundary & boundary)
{
	DEBUG_DESTROYED();
	
	if (mBoundaries)
	{
		mBoundaries->removeBoundary (boundary);
		if (mBoundaries->empty ())
		{
			delete mBoundaries;
			mBoundaries = 0;
		}
	}
}

//----------------------------------------------------------------------

UIBoundary * UIWidget::findBoundary   (const std::string & name)
{
	DEBUG_DESTROYED();
	
	if (mBoundaries)
		return mBoundaries->findBoundary (name);

	return 0;
}

//----------------------------------------------------------------------

const UIBoundary * UIWidget::findBoundary   (const std::string & name) const
{
	DEBUG_DESTROYED();
	
	if (mBoundaries)
		return mBoundaries->findBoundary (name);

	return 0;
}

//----------------------------------------------------------------------

void UIWidget::SetBackgroundOpacity (float f)
{
	DEBUG_DESTROYED();
	
	mBackgroundColor.a = static_cast<unsigned char>(std::min (std::max (0.0f, f), 1.0f) * 255.0f);
}

//----------------------------------------------------------------------

void UIWidget::EnsureRectVisible (const UIRect & rect)
{
	DEBUG_DESTROYED();
	
	const UIPoint & scrollLocation = GetScrollLocation ();

	//-- force a scroll extent update

	UISize  scrollExtent;
	GetScrollExtent (scrollExtent);

	UIRect scrollRect (scrollLocation, GetSize ());

	bool found = false;

	if (rect.right > scrollRect.right)
	{
		scrollRect += UIPoint (rect.right - scrollRect.right, 0L);
		found = true;
	}
	else if (rect.bottom > scrollRect.bottom)
	{
		scrollRect += UIPoint (0L, rect.bottom - scrollRect.bottom);
		found = true;
	}

	if (rect.left < scrollRect.left)
	{
		scrollRect -= UIPoint (scrollRect.left - rect.left, 0L);
		found = true;
	}
	else if (rect.top < scrollRect.top)
	{
		scrollRect -= UIPoint (0L, scrollRect.top - rect.top);
		found = true;
	}

	if (found)
		ScrollToPoint (scrollRect.Location ());

}

//----------------------------------------------------------------------

long UIWidget::GetAnimationState () const
{
	DEBUG_DESTROYED();
	
	return static_cast<long>(UIClock::gUIClock().GetTime());
}

//----------------------------------------------------------------------

void UIWidget::SetPopupStyle (UIPopupMenuStyle * style)
{
	DEBUG_DESTROYED();
	
	AttachMember (mPopupStyle, style);
}

//----------------------------------------------------------------------

UIPopupMenuStyle * UIWidget::FindPopupStyle ()
{
	DEBUG_DESTROYED();
	
	if (mPopupStyle)
		return mPopupStyle;
	else
	{
		UIWidget * const parent = GetParentWidget ();

		if (parent)
			return parent->FindPopupStyle ();
	}

	return 0;
}

//----------------------------------------------------------------------

void UIWidget::SetScrollSizes (const UISize & page, const UISize & line)
{
	DEBUG_DESTROYED();
	
	mScrollSizePage = page;
	mScrollSizeLine = line;
}

//----------------------------------------------------------------------

const UIPacking::SizeInfo * UIWidget::getPackSize           (int index) const
{
	DEBUG_DESTROYED();
	
	if (index < 0 || index > 1)
		return 0;

	return mPackSizes [index];
}

//----------------------------------------------------------------------

UIString const & UIWidget::GetLocalTooltip() const
{
	DEBUG_DESTROYED();
	
	return mLocalTooltip ? *mLocalTooltip : s_emptyString;
}

//----------------------------------------------------------------------

const UIString & UIWidget::GetLocalTooltip(const UIPoint & point) const
{
	DEBUG_DESTROYED();
	
	if (mTooltipCallback) 
	{
		UIString const & NewTooltip = mTooltipCallback->getTooltip(point);

		if (NewTooltip.empty()) 
		{
			delete mLocalTooltip;
			mLocalTooltip = 0;
		}
		else
		{
			if (mLocalTooltip) 
			{
				*mLocalTooltip = NewTooltip;
			}
			else
			{
				mLocalTooltip = new UIString(NewTooltip);
			}
		}
	}

	return GetLocalTooltip();
}

//----------------------------------------------------------------------

void UIWidget::SetDeformer(UIDeformer * deformer)
{
	DEBUG_DESTROYED();
	
	if (!mDeformer && deformer)
	{
		mDeformer = new UIWatcher<UIDeformer>;
	}

	if (mDeformer && ((*mDeformer) != deformer))
	{
		*mDeformer = deformer;
	}
}


//----------------------------------------------------------------------

UIDeformer * UIWidget::GetDeformer() const
{
	DEBUG_DESTROYED();
	
	if (mDeformer)
	{
		return *mDeformer;
	}

	return NULL;
}

//----------------------------------------------------------------------

UIDeformer * UIWidget::GetDeformer()
{
	DEBUG_DESTROYED();
	
	if (mDeformer)
	{
		return *mDeformer;
	}
	
	return NULL;
}

//----------------------------------------------------------------------

bool UIWidget::RunScript()
{
	DEBUG_DESTROYED();
	
	SendCallback( &UIEventCallback::OnRunScript, PropertyName::OnRunScript );
	return true;
}

//----------------------------------------------------------------------

void UIWidget::OnSizeChanged(UISize const & /*newSize*/, UISize const & /*oldSize*/)
{
}

//----------------------------------------------------------------------

void UIWidget::OnLocationChanged(UIPoint const & /*newLocation*/, UIPoint const & /*oldLocation*/)
{
}

//----------------------------------------------------------------------

void UIWidget::SetColor(const UIColor &NewColor, bool applyToChildren )
{
	DEBUG_DESTROYED();
	
	mColor.Set(NewColor);

	if (applyToChildren)
	{
		UIBaseObject::UIObjectList children;
		GetChildren(children);

		for (UIObjectList::iterator itChild = children.begin(); itChild != children.end(); ++itChild)
		{
			UIBaseObject * const object = *itChild;
			UIWidget * const widget = (object && object->IsA(TUIWidget)) ? static_cast<UIWidget*>(object) : NULL;
			if (widget)
			{
				widget->SetColor(NewColor, true);
			}
		}
	}
}


//----------------------------------------------------------------------

const UIString & UIWidget::GetTooltip () const
{
	DEBUG_DESTROYED();
	
	return mTooltip ? *mTooltip : s_emptyString;
}

//----------------------------------------------------------------------

bool UIWidget::GetChildRectForShrinkWrap(UIRect & childRect) const
{
	DEBUG_DESTROYED();
	
	bool hasValidChildren = false;

	UIBaseObject::UIObjectList children;
	GetChildren(children);
	
	// Stretch a rect around the children.
	for (UIObjectList::iterator itChild = children.begin(); itChild != children.end(); ++itChild)
	{
		UIBaseObject * const object = *itChild;
		UIWidget * const widget = UI_ASOBJECT(UIWidget, object);

		if (widget && widget->WillDraw())
		{
			hasValidChildren = true;
			childRect.Extend(widget->GetWorldRect()); // extend in world space.
		}
	}

	return hasValidChildren;
}

//----------------------------------------------------------------------

void UIWidget::WrapChildren()
{
	DEBUG_DESTROYED();
	
	// Do not wrap children if the user is modifying.
	if (!IsUserModifying()) 
	{
		UIWidget const * const parentWidget = GetParentWidget();
		if (parentWidget) 
		{
			UIRect newRect(UIRect::empty);
			if (GetChildRectForShrinkWrap(newRect))
			{
				// Set the size.
				UISize const & newSize = newRect.Size();
				SetSize(newSize);
				SetScrollExtent(newSize);

				// Set the location.
				UIPoint const & newLocation = newRect.Location() - parentWidget->GetWorldLocation();
				SetLocation(newLocation);
			}
			else
			{
				// Resize to the minimum size if there are no visible child objects.
				SetSize(GetMinimumSize());
			}
		}
	}
}

//----------------------------------------------------------------------

void UIWidget::SetPalShade(float shade)
{
	mPalShade = shade;
}

//----------------------------------------------------------------------

float UIWidget::GetPalShade() const
{
	return mPalShade;
}

//----------------------------------------------------------------------

void UIWidget::SetNotModifyingUseDefaultCursor(bool b)
{
	mNotModifyingUseDefaultCursor = b;
}

//----------------------------------------------------------------------

bool UIWidget::GetNotModifyingUseDefaultCursor() const
{
	return mNotModifyingUseDefaultCursor;
}

//----------------------------------------------------------------------

bool UIWidget::GetForwardMoveToParent() const
{
	return mForwardMoveToParent;
}

//----------------------------------------------------------------------

void UIWidget::SetForwardMoveToParent (bool b)
{
	mForwardMoveToParent = b;
}

//----------------------------------------------------------------------

bool UIWidget::GetAcceptsMoveFromChildren() const
{
	return mAcceptsMoveFromChildren;
}

//----------------------------------------------------------------------

void UIWidget::SetAcceptsMoveFromChildren (bool b)
{
	mAcceptsMoveFromChildren = b;
}

//----------------------------------------------------------------------
