#include "_precompile.h"
#include "UIVolumePage.h"

#include "UIButton.h"
#include "UICanvas.h"
#include "UICheckbox.h"
#include "UIMessage.h"
#include "UIPropertyDescriptor.h"
#include "UIRectangleStyle.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIUtils.h"

#include <cassert>
#include <list>
#include <vector>

#include <math.h>

//----------------------------------------------------------------------

const char *UIVolumePage::TypeName = "VolumePage";

//======================================================================================

const UILowerString UIVolumePage::PropertyName::CellSize             = UILowerString ("CellSize");
const UILowerString UIVolumePage::PropertyName::CellPadding          = UILowerString ("CellPadding");
const UILowerString UIVolumePage::PropertyName::CellMax              = UILowerString ("CellMax");
const UILowerString UIVolumePage::PropertyName::CellCount            = UILowerString ("CellCount");
const UILowerString UIVolumePage::PropertyName::CellCountFixed       = UILowerString ("CellCountFixed");
const UILowerString UIVolumePage::PropertyName::CellForceDragable    = UILowerString ("CellForceDragable");
const UILowerString UIVolumePage::PropertyName::CellSelectable       = UILowerString ("CellSelectable");
const UILowerString UIVolumePage::PropertyName::CellDragAccepts      = UILowerString ("CellDragAccepts");
const UILowerString UIVolumePage::PropertyName::MultiSelection       = UILowerString ("MultiSelection");
const UILowerString UIVolumePage::PropertyName::CellAlignmentV       = UILowerString ("CellAlignmentV");
const UILowerString UIVolumePage::PropertyName::CellAlignmentH       = UILowerString ("CellAlignmentH");
const UILowerString UIVolumePage::PropertyName::CellSelectionOverlay = UILowerString ("CellSelectionOverlay");
const UILowerString UIVolumePage::PropertyName::CellSelectionOverlayOpacity = UILowerString ("CellSelectionOverlayOpacity");
const UILowerString UIVolumePage::PropertyName::DoNotOverrideMaximumSize = UILowerString ("DoNotOverrideMaximumSize");

//======================================================================================
#define _TYPENAME UIVolumePage

namespace UIVolumePageNamespace
{
	/**
	* Change the visibility of all child widgets
	*/
	void changeVisibility (UIWidget & parent, bool visible)
	{
		bool changed = false;

		UIBaseObject::UIObjectList olist;
		parent.GetChildren (olist);
		for( UIBaseObject::UIObjectList::iterator i = olist.begin(); i != olist.end(); ++i )
		{
			UIWidget * const theWidget = UI_ASOBJECT(UIWidget, *i);
			if (theWidget && (theWidget->IsVisible() != visible)) 
			{
				theWidget->SetVisible(visible);
				changed = true;
			}
		}

		if (changed)
		{
			UIPage * const parentPage = UI_ASOBJECT(UIPage, &parent);
			if (parentPage) 
			{
				parentPage->SetPackDirty(true);
			}
		}
	}

	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(CellPadding, "", T_point),
		_DESCRIPTOR(CellSize, "", T_point),
		_DESCRIPTOR(CellMax, "", T_int),
		_DESCRIPTOR_FLAGS(CellCount, "", T_point, F_READ_ONLY), // ReadOnly
		_DESCRIPTOR(CellCountFixed, "", T_point),
		_DESCRIPTOR(CellForceDragable, "", T_bool),
		_DESCRIPTOR(CellSelectable, "", T_bool),
		_DESCRIPTOR(MultiSelection, "", T_bool),
		_DESCRIPTOR(CellAlignmentV, "", T_string), // ENUM
		_DESCRIPTOR(CellAlignmentH, "", T_string), // ENUM
		_DESCRIPTOR(DoNotOverrideMaximumSize, "", T_bool),
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================

	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(CellSelectionOverlay, "", T_object),
		_DESCRIPTOR(CellSelectionOverlayOpacity, "", T_float),
	_GROUPEND(Appearance, 3, int(UIPropertyCategories::C_Appearance));
	//================================================================
}
using namespace UIVolumePageNamespace;
//======================================================================================

UIVolumePage::UIVolumePage () :
UIPage                            (),
mCellSize                         (32,32),
mCellPadding                      (2,2),
mCellCount                        (),
mCellCountFixed                   (),
mCellMax                          (16384),
mWidgetCount                      (0),
mPackingRecursionGuard            (false),
mCallbackForwardingRecursionGuard (false),
mLastSelectedChild                (0),
mMultiSelection                   (true),
mDoMultiSelection                 (false),
mCellForceDragable                (false),
mCellSelectable                   (true),
mCellAlignmentVertical            (VA_Top),
mCellAlignmentHorizontal          (HA_Center),
mCellSelectionOverlay             (0),
mCellSelctionOverlayOpacity       (0.5f),
mDoNotOverrideMaximumSize         (false),
mMouseDown                        (false)
{
	AddCallback (this);
}

//======================================================================================

UIVolumePage::~UIVolumePage( void )
{
	RemoveCallback (this);

	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();

	for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
	{
		UIBaseObject * const obj = *i;
		if (obj->IsA (TUIWidget))
			static_cast<UIWidget *>(obj)->RemoveCallback (this);
	}
}


//======================================================================================

bool UIVolumePage::IsA( const UITypeID QueriedType ) const
{
	return QueriedType == TUIVolumePage || UIPage::IsA( QueriedType );
}

//======================================================================================

const char *UIVolumePage::GetTypeName( void ) const
{
	return TypeName;
}

//======================================================================================

UIBaseObject *UIVolumePage::Clone( void ) const
{
	return new UIVolumePage;
}

//-----------------------------------------------------------------

void UIVolumePage::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::CellSelectionOverlay );

	UIPage::GetLinkPropertyNames (In);
}

//----------------------------------------------------------------------

void UIVolumePage::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIPage::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIVolumePage::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	if (!forCopy)
	{
		In.push_back( PropertyName::CellPadding );
		In.push_back( PropertyName::CellSize );
		In.push_back( PropertyName::CellMax );
		In.push_back( PropertyName::CellCount );
		In.push_back( PropertyName::CellCountFixed );
		In.push_back( PropertyName::CellForceDragable );
		In.push_back( PropertyName::CellSelectable );
		In.push_back( PropertyName::CellDragAccepts );
		In.push_back( PropertyName::MultiSelection );
		In.push_back( PropertyName::CellAlignmentV );
		In.push_back( PropertyName::CellAlignmentH );
		In.push_back( PropertyName::CellSelectionOverlay );
		In.push_back( PropertyName::CellSelectionOverlayOpacity);
		In.push_back( PropertyName::DoNotOverrideMaximumSize);
	}
	
	UIPage::GetPropertyNames( In, forCopy );
}

//======================================================================================

bool UIVolumePage::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::CellPadding)
	{
		UIPoint p;
		if (UIUtils::ParsePoint (Value, p))
		{
			SetCellPadding (p);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::CellSize)
	{
		UIPoint p;
		if (UIUtils::ParsePoint (Value, p))
		{
			SetCellSize (p);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::CellCountFixed)
	{
		UIPoint p;
		if (UIUtils::ParsePoint (Value, p))
		{
			SetCellCountFixed (p);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::CellMax)
	{
		return UIUtils::ParseLong (Value, mCellMax);
	}
	else if (Name == PropertyName::CellForceDragable)
	{
		return UIUtils::ParseBoolean (Value, mCellForceDragable);
	}
	else if (Name == PropertyName::CellSelectable)
	{
		return UIUtils::ParseBoolean (Value, mCellSelectable);
	}
	else if (Name == PropertyName::MultiSelection)
	{
		bool b;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetMultiSelection (b);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::CellAlignmentV)
	{
		VerticalAlignment va = VA_Top;
		if (FindVerticalAlignment (Unicode::wideToNarrow (Value).c_str (), va))
		{
			if (va != mCellAlignmentVertical)
			{
				mCellAlignmentVertical = va;
				SetPackDirty (true);
			}

			return true;
		}

		return false;
	}
	else if (Name == PropertyName::CellAlignmentH)
	{
		HorizontalAlignment ha = HA_Left;
		if (FindHorizontalAlignment (Unicode::wideToNarrow (Value).c_str (), ha))
		{
			if (ha != mCellAlignmentHorizontal)
			{
				mCellAlignmentHorizontal = ha;
				SetPackDirty (true);
			}

			return true;
		}

		return false;
	}
	else if (Name == PropertyName::CellCount)
	{
		return UIUtils::ParsePoint (Value, mCellCount);
		//-- cellcount is read-only ??
	}
	else if (Name == PropertyName::CellSelectionOverlay)
	{
		UIRectangleStyle * const rs = static_cast<UIRectangleStyle *>( GetObjectFromPath( Value, TUIRectangleStyle ) );

		if( rs || Value.empty() )
		{
			SetCellSelectionOverlay( rs );
			return true;
		}
	}

	//----------------------------------------------------------------------

	else if (Name == PropertyName::CellSelectionOverlayOpacity)
	{
		return UIUtils::ParseFloat (Value, mCellSelctionOverlayOpacity);
	}

	//----------------------------------------------------------------------

	else if (Name == PropertyName::DoNotOverrideMaximumSize)
	{
		return UIUtils::ParseBoolean (Value, mDoNotOverrideMaximumSize);
	}

	return UIPage::SetProperty( Name, Value );
}

//======================================================================================

bool UIVolumePage::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::CellPadding)
	{
		return UIUtils::FormatPoint (Value, mCellPadding);
	}
	else if (Name == PropertyName::CellSize)
	{
		return UIUtils::FormatPoint (Value, mCellSize);
	}
	else if (Name == PropertyName::CellCountFixed)
	{
		return UIUtils::FormatPoint (Value, mCellCountFixed);
	}
	else if (Name == PropertyName::CellMax)
	{
		return UIUtils::FormatLong (Value, mCellMax);
	}
	else if (Name == PropertyName::MultiSelection)
	{
		return UIUtils::FormatBoolean (Value, mMultiSelection);
	}
	else if (Name == PropertyName::CellCount)
	{
		return UIUtils::FormatPoint (Value, mCellCount);
	}
	else if (Name == PropertyName::CellForceDragable)
	{
		return UIUtils::FormatBoolean (Value, mCellForceDragable);
	}
	else if (Name == PropertyName::CellSelectable)
	{
		return UIUtils::FormatBoolean (Value, mCellSelectable);
	}
	else if (Name == PropertyName::CellAlignmentV)
	{
		return FormatVerticalAlignment (mCellAlignmentVertical, Value);
	}
	else if (Name == PropertyName::CellAlignmentH)
	{
		return FormatHorizontalAlignment (mCellAlignmentHorizontal, Value);
	}
	else if (Name == PropertyName::CellSelectionOverlay)
	{
		if( mCellSelectionOverlay )
		{
			GetPathTo( Value, mCellSelectionOverlay );
			return true;
		}
	}
	//----------------------------------------------------------------------

	else if (Name == PropertyName::CellSelectionOverlayOpacity)
	{
		return UIUtils::FormatFloat (Value, mCellSelctionOverlayOpacity);
	}

	//----------------------------------------------------------------------

	else if (Name == PropertyName::DoNotOverrideMaximumSize)
	{
		return UIUtils::FormatBoolean (Value, mDoNotOverrideMaximumSize);
	}
	return UIPage::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

UIWidget * UIVolumePage::FindCell (const UIPoint & thePoint, const bool clip)
{

	const UIPoint & ScrollLocation = GetScrollLocation();
	const UIObjectList & olist = GetChildrenRef ();

	UIPoint cell;

	for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
	{
		UIBaseObject * const o = *i;

		if (o->IsA (TUIWidget))
		{
			UIWidget * const w = static_cast<UIWidget *>( o );
			UIPoint p (thePoint - w->GetLocation() + ScrollLocation);

			if (clip)
			{
				if (cell.x == 0L)
					p.x = std::max (0L, p.x);
				else if (cell.x == mCellCount.x - 1)
					p.x = std::min (w->GetWidth (), p.x);

				if (cell.y == 0L)
					p.y = std::max (0L, p.y);
				else if (cell.y == mCellCount.y - 1)
					p.y = std::min (w->GetHeight (), p.y);
			}

			if( w->WillDraw() && w->HitTest( p ) )
			{
				return w;
			}

			++cell.x;

			if (cell.x >= mCellCount.x)
			{
				cell.x = 0L;
				++cell.y;
			}
		}
	}

	return 0;
}

//----------------------------------------------------------------------

UIWidget * UIVolumePage::FindCell                (int index)
{
	int count = -1;

	//----------------------------------------------------------------------
	//-- find the selected child's position

	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();
	for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
	{
		UIBaseObject * const theObject = *i;
		if (!theObject->IsA (TUIWidget))
			continue;

		++count;
		UIWidget * const widget = static_cast<UIWidget *>(theObject);

		if (count == index)
			return widget;
	}

	return 0;
}

//----------------------------------------------------------------------

int UIVolumePage::FindCellIndex           (const UIWidget & child) const
{
	int count = -1;

	//----------------------------------------------------------------------
	//-- find the selected child's position

	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();
	for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
	{
		UIBaseObject * const theObject = *i;
		if (!theObject->IsA (TUIWidget))
			continue;

		++count;
		UIWidget * const widget = static_cast<UIWidget *>(theObject);

		if (&child == widget)
			return count;
	}

	return -1;
}

//======================================================================================

bool UIVolumePage::ProcessMessage (const UIMessage &msg)
{
	bool processed = false;

	if (mCellSelectable)
	{
		UIWidget * Context = 0;

		if (msg.Type == UIMessage::LeftMouseDown || msg.Type == UIMessage::RightMouseDown)
		{
			Context = FindCell (msg.MouseCoords, false);
		}
		else if (msg.Type == UIMessage::KeyDown || msg.Type == UIMessage::KeyRepeat)
		{
			int index = 0;
			const int lastSelectedIndex = GetLastSelectedIndex ();

			if (lastSelectedIndex >= 0)
			{
				index = lastSelectedIndex;
				const UIPoint cell (FindChildCell (index));

				if (msg.Keystroke == UIMessage::UpArrow)
				{
					if (mCellCount.y != 1L)
					{
						if (cell.y > 0)
							index -= mCellCount.x;
					}
					else
						--index;
				}
				else if (msg.Keystroke == UIMessage::DownArrow)
				{
					if (mCellCount.y != 1L)
					{
						if (cell.y < mCellCount.y - 1)
							index += mCellCount.x;
					}
					else
						++index;
				}
				else if (msg.Keystroke == UIMessage::LeftArrow)
				{
					--index;
				}
				else if (msg.Keystroke == UIMessage::RightArrow)
				{
					++index;
				}
				else
					return false;

				index = std::max (std::min (index, static_cast<int>(mWidgetCount)), 0);
			}
			else
			{
				if (msg.Keystroke != UIMessage::UpArrow &&
					msg.Keystroke == UIMessage::DownArrow &&
					msg.Keystroke == UIMessage::LeftArrow &&
					msg.Keystroke == UIMessage::RightArrow)
					return false;
			}

			Context = FindCell (index);
		}

		if (Context)
		{
			bool selectionChanged = true;

			if ((mMultiSelection || !mLastSelectedChild || mLastSelectedChild == Context) && (msg.Modifiers.LeftControl || msg.Modifiers.RightControl))
			{
				//@todo: removeSelection function

				bool oldActivated = Context->IsActivated ();
				Context->SetActivated (!oldActivated);

				//----------------------------------------------------------------------
				//-- if removing a selection, we must arbitrarily reset the last selected child
				if (oldActivated)
				{
					Context = 0;

					const UIObjectList & olist = GetChildrenRef ();

					for (UIObjectList::const_reverse_iterator rit = olist.rbegin (); rit != olist.rend (); ++rit)
					{
						if ((*rit)->IsA (TUIWidget) && static_cast<UIWidget *>(*rit)->IsActivated ())
						{
							Context = static_cast<UIWidget *>(*rit);
							break;
						}
					}
				}
			}
			else if (mMultiSelection && (msg.Modifiers.LeftShift || msg.Modifiers.RightShift))
			{
				//@todo: AddSelection function
				Context->SetActivated (true);
				mDoMultiSelection = true;
			}
			else
			{
				// SetSelection handles the selection changed callback
				selectionChanged = false;
				SetSelection (Context);
			}

			if (selectionChanged)
			{
				mLastSelectedChild = Context;
				SendCallback( &UIEventCallback::OnVolumePageSelectionChanged, UILowerString::null);
				mDoMultiSelection = false;
			}

			processed = true;

			if (Context)
				EnsureChildVisible (*Context);
		}
	}

	if (UIPage::ProcessMessage( msg ))
		return true;

	//-- handle mousebutton-dragging

	if (msg.Type == UIMessage::LeftMouseDown)
	{
		if (mCellSelectable)
		{
			mMouseDown = true;
			return true;;
		}
	}
	else if (msg.Type == UIMessage::LeftMouseUp)
	{
		mMouseDown = false;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragStart)
	{
		mMouseDown = false;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseMove && mMouseDown)
	{
		if (mCellSelectable)
		{
			UIWidget * const cell = FindCell (msg.MouseCoords, true);

			if (cell)
			{
				SetSelection (cell);
			}

		}

		//@todo: handle auto-scrolling
		return true;
	}

	return processed;
}

//----------------------------------------------------------------------

/**
* Force all children visible.  This gives complex widgets a chance to
* unpause or reactivate themselves
*/

void UIVolumePage::OnShow( UIWidget *Context )
{
	if (Context == this)
		changeVisibility (*this, true);
}


//----------------------------------------------------------------------

/**
* Force all children invisible.  This gives complex widgets a chance to
* pause or deactivate themselves.  This also will clear the current selection.
*/

void UIVolumePage::OnHide( UIWidget *Context )
{
	if (Context == this)
	{
		changeVisibility (*this, false);


		const UIBaseObject::UIObjectList & olist = GetChildrenRef ();
		for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
		{
			UIBaseObject * const theObject = *i;
			if (!theObject->IsA (TUIWidget))
				continue;

			UIWidget * const widget = static_cast<UIWidget *>(theObject);
			widget->SetActivated (false);
		}

		mLastSelectedChild = 0;
		SendCallback( &UIEventCallback::OnVolumePageSelectionChanged, UILowerString::null );
	}
}

//----------------------------------------------------------------------

bool UIVolumePage::OnMessage( UIWidget *Context, const UIMessage & msg )
{
	assert (Context);

	if (mCallbackForwardingRecursionGuard)
		return true;

	if (Context != this && Context->GetParent () == this)
	{
		//----------------------------------------------------------------------
		//-- a UIVolumePage always forwards messages from its children to any
		//-- listeners subscribed to the volume page

		mCallbackForwardingRecursionGuard = true;
		const bool keepGoing = SendUIMessageCallback( msg, Context );
		mCallbackForwardingRecursionGuard = false;

		return keepGoing || msg.Type == UIMessage::LeftMouseDown;
	}

	return true;
}

//----------------------------------------------------------------------

void UIVolumePage::OnHoverIn  (UIWidget *context)
{
	if (context != this && context->GetParent () == this)
	{
		if (mCallbackForwardingRecursionGuard)
			return;

		mCallbackForwardingRecursionGuard = true;
		SendCallback( &UIEventCallback::OnHoverIn, context, UIWidget::PropertyName::OnHoverIn);
		mCallbackForwardingRecursionGuard = false;
	}
}

//----------------------------------------------------------------------

void UIVolumePage::OnHoverOut (UIWidget *context)
{
	if (context != this && context->GetParent () == this)
	{
		if (mCallbackForwardingRecursionGuard)
			return;

		mCallbackForwardingRecursionGuard = true;
		SendCallback( &UIEventCallback::OnHoverOut, context, UIWidget::PropertyName::OnHoverOut);
		mCallbackForwardingRecursionGuard = false;
	}
}

//----------------------------------------------------------------------

void UIVolumePage::GetScrollSizes( UISize &PageSize, UISize &LineSize ) const
{
	PageSize = GetSize ();
	LineSize = mCellSize + mCellPadding;
}

//======================================================================================

void UIVolumePage::Pack()
{
	if(GetDoNotPackChildren())
		return;

	if (mPackingRecursionGuard)
		return;

	mWidgetCount = 0;

	//-- compute the widget count
	{
		const UIBaseObject::UIObjectList & olist = GetChildrenRef ();
		for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
		{
			UIBaseObject * const theObject = *i;
			if (!theObject->IsA (TUIWidget))
				continue;

			UIWidget * const wid = static_cast<UIWidget *>(theObject);

			if (!wid->IsVisible ())
				continue;

			++mWidgetCount;
		}
	}

	if (mCellCountFixed.x)
		mCellSize.x = (GetWidth ()  - mCellPadding.x * (mCellCountFixed.x + 1L)) / mCellCountFixed.x;
	if (mCellCountFixed.y)
		mCellSize.y = (GetHeight () - mCellPadding.y * (mCellCountFixed.y + 1L)) / mCellCountFixed.y;

	UIPoint pt (mCellPadding);

	const UISize paddedCellSize (mCellSize + mCellPadding);

	if (paddedCellSize.x == 0L || paddedCellSize.y == 0L)
		return;

	const UISize size (GetSize ());
	long maxWidth = 0L;

	if (mCellCountFixed.y)
		maxWidth = mCellPadding.x + ((mWidgetCount / (mCellCountFixed.y)) * paddedCellSize.x);
	else
		maxWidth = size.x;

	const Unicode::String empty;

	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();
	int scrollSizeX = -1;
	for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
	{
		UIBaseObject * const theObject = *i;
		if (!theObject->IsA (TUIWidget))
			continue;

		UIWidget * const widget = static_cast<UIWidget *>(theObject);

		if (!widget->IsVisible ())
			continue;

		//-- three calls are needed to ensure that both increases and decreases in size are honored

		widget->SetLocation (pt);

		if (mCellForceDragable)
			widget->SetDragable (true);

		UIString cellDragAccepts;

		if (GetProperty (PropertyName::CellDragAccepts, cellDragAccepts) && !cellDragAccepts.empty ())
		{
			UIString oldDragAccepts;
			widget->GetProperty (UIWidget::PropertyName::DragAccepts, oldDragAccepts);

			if (oldDragAccepts.find (cellDragAccepts) == oldDragAccepts.npos)
			{
				if (!oldDragAccepts.empty ())
					oldDragAccepts.append (1, ',');

				widget->SetProperty (UIWidget::PropertyName::DragAccepts, oldDragAccepts + cellDragAccepts);
			}
		}

		UISize actualSize = widget->GetSize();
		UISize maxSize = widget->GetMaximumSize();
		int widthToAdd = actualSize.x;
		//Use paddedCellSize unless the maxSize is smaller than cellSize, in which case use maxSize,
		//but don't decrease the actual size to make it smaller (i.e. no overlap)
		if((actualSize.x < mCellSize.x) && (mCellSize.x < maxSize.x))
			widthToAdd = mCellSize.x;
		else
			widthToAdd = actualSize.x;
		pt.x += (widthToAdd + mCellPadding.x);

		if (pt.x > scrollSizeX)
			scrollSizeX = pt.x;

		if (scrollSizeX > maxWidth)
			scrollSizeX = maxWidth;

		if (pt.x + paddedCellSize.x > maxWidth)
		{
			pt.x = mCellPadding.x;
			pt.y += mCellPadding.y + mCellSize.y;
		}
	}

	if (mWidgetCount)
	{
		mCellCount.x = std::max (1L, (maxWidth - mCellPadding.x) / paddedCellSize.x);
		mCellCount.y = mWidgetCount / mCellCount.x;

		if (mWidgetCount % mCellCount.x)
			++mCellCount.y;
	}
	else
		mCellCount.y = mCellCount.x = 0;

	const UISize scrollSize (scrollSizeX + mCellPadding.x, mCellCount.y * paddedCellSize.y + mCellPadding.y);

	mPackingRecursionGuard = true;
	SetMinimumScrollExtent (scrollSize);
	SetScrollExtent        (scrollSize);
	mPackingRecursionGuard = false;

	if (mCellAlignmentVertical == VA_Bottom)
	{
		if (scrollSize.y < GetHeight ())
		{
			const long diffy = GetHeight () - scrollSize.y;

			for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
			{
				UIBaseObject * const theObject = *i;
				if (!theObject->IsA (TUIWidget))
					continue;

				UIWidget * const widget = static_cast<UIWidget *>(theObject);

				const UIPoint loc (widget->GetLocation ());

				widget->SetLocation (loc.x, loc.y + diffy);
			}
		}
	}

	SetPackDirty (false);
}

//======================================================================================

bool UIVolumePage::AddChild( UIBaseObject *ChildToAdd )
{
	if (mWidgetCount >= mCellMax)
		return false;


	const bool retval = UIPage::AddChild (ChildToAdd);

	if (retval && ChildToAdd->IsA (TUIWidget))
	{
		static_cast<UIWidget *>(ChildToAdd)->AddCallback (this);
		ChildToAdd->SetProperty (UIWidget::PropertyName::PackSize,     UIString ());
		ChildToAdd->SetProperty (UIWidget::PropertyName::PackLocation, UIString ());
		SetPackDirty (true);
	}

	return retval;
}

//----------------------------------------------------------------------

bool UIVolumePage::InsertChildBefore (UIBaseObject * ChildToAdd, const UIBaseObject * childToPrecede)
{
	if (mWidgetCount >= mCellMax)
		return false;

	const bool retval =  UIPage::InsertChildBefore (ChildToAdd, childToPrecede);

	if (retval && ChildToAdd->IsA (TUIWidget))
	{
		static_cast<UIWidget *>(ChildToAdd)->AddCallback (this);
		ChildToAdd->SetProperty (UIWidget::PropertyName::PackSize,     UIString ());
		ChildToAdd->SetProperty (UIWidget::PropertyName::PackLocation, UIString ());

		SetPackDirty (true);
	}

	return retval;
}

//----------------------------------------------------------------------

bool UIVolumePage::InsertChildAfter (UIBaseObject * ChildToAdd, const UIBaseObject * childToSucceed)
{
	if (mWidgetCount >= mCellMax)
		return false;

	const bool retval =  UIPage::InsertChildAfter (ChildToAdd, childToSucceed);

	if (retval && ChildToAdd->IsA (TUIWidget))
	{
		static_cast<UIWidget *>(ChildToAdd)->AddCallback (this);
		ChildToAdd->SetProperty (UIWidget::PropertyName::PackSize,     UIString ());
		ChildToAdd->SetProperty (UIWidget::PropertyName::PackLocation, UIString ());
		SetPackDirty (true);
	}

	return retval;
}

//======================================================================================

bool UIVolumePage::RemoveChild( UIBaseObject *ChildToRemove )
{
	bool selectionChanged = false;

	if (ChildToRemove->IsA (TUIWidget))
	{
		UIWidget * const widget = static_cast<UIWidget *>(ChildToRemove);
		widget->RemoveCallback (this);

		if (widget == mLastSelectedChild)
			mLastSelectedChild = 0;

		if (widget->IsActivated ())
		{
			selectionChanged = true;
			widget->SetActivated (false);
		}
	}

	if (UIPage::RemoveChild (ChildToRemove))
	{
		SetPackDirty (true);

		if (selectionChanged)
			SendCallback( &UIEventCallback::OnVolumePageSelectionChanged, UILowerString::null );

		return true;
	}

	assert (false);
	return false;
}

//======================================================================================

void UIVolumePage::Render( UICanvas &DestinationCanvas ) const
{

	const UIPoint scrollLocation (GetScrollLocation());
	const UIPoint size (GetSize ());
	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();

	//----------------------------------------------------------------------
	//-- first make sure all the widgets are the right size

	{
		for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
		{
			UIBaseObject * const theObject = *i;
			if (!theObject->IsA (TUIWidget))
				continue;

			UIWidget * const widget = static_cast<UIWidget *>(theObject);

			const UIRect rect (widget->GetLocation (), mCellSize);

			if (widget->GetSize () != mCellSize)
			{
				if(mDoNotOverrideMaximumSize)
				{				
					widget->SetMinimumSize (mCellSize);
					widget->SetSize(mCellSize);
				}
				else
				{
					widget->SetMaximumSize (mCellSize);
					widget->SetMinimumSize (mCellSize);
					widget->SetMaximumSize (mCellSize);
				}
			}
		}
	}

	UIPage::Render (DestinationCanvas);

	for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
	{
		UIBaseObject * const theObject = *i;
		if (!theObject->IsA (TUIWidget))
			continue;

		UIWidget * const widget = static_cast<UIWidget *>(theObject);

		const UIRect rect (widget->GetRect ());

		if (widget->IsActivated ())
		{
			if (mCellSelectable)
			{
				if (mCellSelectionOverlay)
				{
					UISize scrollExtent;
					widget->GetScrollExtent (scrollExtent);
					
					const float oldOpacity = DestinationCanvas.GetOpacity ();
					
					DestinationCanvas.ModifyOpacity (mCellSelctionOverlayOpacity);
					DestinationCanvas.Translate     (widget->GetLocation ());
					mCellSelectionOverlay->Render   (GetAnimationState (), DestinationCanvas, scrollExtent);
					DestinationCanvas.Translate     (-widget->GetLocation ());
					DestinationCanvas.SetOpacity    (oldOpacity);
				}
				else
				{
					//-- wtf
					const UIPoint & loc = widget->GetLocation ();
					const UIFloatPoint pos (static_cast<float>(loc.x), static_cast<float>(loc.y));
					const UISize size (widget->GetSize ());
					
					const UIFloatPoint points [5] =
					{
						pos + UIFloatPoint (0.0f,                        0.0f),
							pos + UIFloatPoint (0.0f,                        static_cast<float>(size.y)),
							pos + UIFloatPoint (static_cast<float>(size.x), static_cast<float>(size.y)),
							pos + UIFloatPoint (static_cast<float>(size.x), 0.0f),
							pos + UIFloatPoint (0.0f, 0.0f),
					};
					
					const UIColor OldColor   = DestinationCanvas.GetColor();
					DestinationCanvas.SetColor (UIColor (255, 255, 0));
					DestinationCanvas.RenderLineStrip (0, 5, points);
					DestinationCanvas.SetColor (OldColor);
				}
			}
		}
	}
}

//======================================================================================

bool UIVolumePage::MoveChild( UIBaseObject *ObjectToMove, ChildMovementDirection MoveDirection )
{
	if (UIPage::MoveChild (ObjectToMove, MoveDirection))
	{
		SetPackDirty (true);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

void UIVolumePage::SetSize( const UISize & size )
{
	const UISize oldSize = GetSize ();
	UIPage::SetSize (size);
	if (oldSize != GetSize ())
		SetPackDirty (true);
}

//----------------------------------------------------------------------

void UIVolumePage::SetCellCountFixed (const UISize & count)
{
	if (count != mCellCountFixed)
	{
		mCellCountFixed = count;
		SetPackDirty (true);
	}
}

//----------------------------------------------------------------------

void UIVolumePage::GetSelection (UIWidget::UIWidgetVector & selection) const
{
	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();
	for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
	{
		UIBaseObject * const theObject = *i;
		if (!theObject->IsA (TUIWidget))
			continue;

		UIWidget * const widget = static_cast<UIWidget *>(theObject);

		if (widget->IsActivated ())
			selection.push_back (widget);
	}
}

//----------------------------------------------------------------------

void UIVolumePage::SetMultiSelection (bool b)
{
	mMultiSelection = b;

	if (!mMultiSelection)
	{
		//----------------------------------------------------------------------
		//-- deselect all but the last selected child

		const UIBaseObject::UIObjectList & olist = GetChildrenRef ();
		for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
		{
			UIBaseObject * const theObject = *i;
			if (!theObject->IsA (TUIWidget))
				continue;

			UIWidget * const widget = static_cast<UIWidget *>(theObject);

			if (widget->IsActivated () && widget != mLastSelectedChild)
				widget->SetActivated (false);
		}

		SendCallback( &UIEventCallback::OnVolumePageSelectionChanged, UILowerString::null );
	}
}

//----------------------------------------------------------------------

int UIVolumePage::GetLastSelectedIndex () const
{
	if (!mLastSelectedChild)
		return -1;

	return FindCellIndex (*mLastSelectedChild);
}

//----------------------------------------------------------------------

void UIVolumePage::SetSelectionIndex (int index)
{
	SetSelection (FindCell (index));
}

//----------------------------------------------------------------------

void UIVolumePage::SetSelection (UIWidget * const child, bool const clearPreviousSelections)
{
	bool changed = false;

	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();
	for( UIObjectList::const_iterator i = olist.begin(); i != olist.end(); ++i )
	{
		UIBaseObject * const theObject = *i;
		if (!theObject->IsA (TUIWidget))
			continue;

		UIWidget * const widget = static_cast<UIWidget *>(theObject);

		bool activate = widget == child;

		if (activate != widget->IsActivated())
		{
			if (activate)
			{
				widget->SetActivated(true);
			}

			else if (clearPreviousSelections)
			{
				widget->SetActivated(false);
			}

			changed = true;
		}

		if (widget == child)
			mLastSelectedChild = widget;
	}

	if (changed)
		SendCallback( &UIEventCallback::OnVolumePageSelectionChanged, UILowerString::null );
}

//----------------------------------------------------------------------


bool  UIVolumePage::FindVerticalAlignment   (const char * name, VerticalAlignment & va)
{
	assert (name);

	if (!_stricmp (name, "top"))
		va = VA_Top;
	else if (!_stricmp (name, "center"))
		va = VA_Center;
	else if (!_stricmp (name, "bottom"))
		va = VA_Bottom;
	else
		return false;

	return true;
}

//----------------------------------------------------------------------

bool UIVolumePage::FindHorizontalAlignment (const char * name, HorizontalAlignment & ha)
{
	assert (name);

	if (!_stricmp (name, "left"))
		ha = HA_Left;
	else if (!_stricmp (name, "center"))
		ha = HA_Center;
	else if (!_stricmp (name, "right"))
		ha = HA_Right;
	else
		return false;

	return true;
}

//----------------------------------------------------------------------

bool UIVolumePage::FormatVerticalAlignment   (VerticalAlignment va, UIString & Value)
{
	const char * narrowValue = 0;

	switch (va)
	{
	case VA_Top:
		narrowValue = "top";
		break;
	case VA_Center:
		narrowValue = "center";
		break;
	case VA_Bottom:
		narrowValue = "bottom";
		break;
	default:
		return false;
	}

	assert (narrowValue);
	Value = Unicode::narrowToWide (narrowValue);

	return true;
}

//----------------------------------------------------------------------

bool UIVolumePage::FormatHorizontalAlignment (HorizontalAlignment ha, UIString & Value)
{
	const char * narrowValue = 0;

	switch (ha)
	{
	case HA_Left:
		narrowValue = "left";
		break;
	case HA_Center:
		narrowValue = "center";
		break;
	case HA_Right:
		narrowValue = "right";
		break;
	default:
		return false;
	}

	assert (narrowValue);
	Value = Unicode::narrowToWide (narrowValue);

	return true;
}
//----------------------------------------------------------------------

void UIVolumePage::Clear ()
{
	const bool wasEmpty = GetChildrenRef ().empty ();
	UIPage::Clear ();

	if (!wasEmpty)
		SendCallback( &UIEventCallback::OnVolumePageSelectionChanged, UILowerString::null );
}

//----------------------------------------------------------------------

void UIVolumePage::CenterChild (const UIWidget & child)
{
	if (child.GetParent () != this)
		return;

	UIPoint childPos = child.GetLocation () + (child.GetSize () - GetSize ()) / 2L;
	ScrollToPoint (childPos);
}

//----------------------------------------------------------------------

void UIVolumePage::EnsureChildVisible (const UIWidget & child)
{
	if (child.GetParent () != this)
		return;

	UIRect childRect  = child.GetRect ();
	childRect.left   -= mCellPadding.x;
	childRect.right  += mCellPadding.x;
	childRect.top    -= mCellPadding.y;
	childRect.bottom += mCellPadding.y;

	EnsureRectVisible (childRect);

}

//----------------------------------------------------------------------

void UIVolumePage::SetCellSelectionOverlay (UIRectangleStyle * rs)
{
	mCellSelectionOverlay = rs;
}

//----------------------------------------------------------------------

UIPoint UIVolumePage::FindChildCell (int index) const
{
	UIPoint cell;

	if (mCellCount.x)
	{
		cell.x = index % mCellCount.x;
		cell.y = index / mCellCount.x;
	}

	return cell;
}

//----------------------------------------------------------------------

void UIVolumePage::SetCellSize          (const UISize & count)
{
	if (mCellSize != count)
	{
		mCellSize = count;
		SetPackDirty (true);
	}
}

//----------------------------------------------------------------------

void UIVolumePage::SetCellPadding       (const UISize & count)
{
	if (mCellPadding != count)
	{
		mCellPadding = count;
		SetPackDirty (true);
	}
}

//----------------------------------------------------------------------

void UIVolumePage::CopyPropertiesFrom(const UIBaseObject & rhs)
{
	UIPage::CopyPropertiesFrom(rhs);
	if (rhs.IsA(TUIVolumePage))
	{
		UIVolumePage const & rhs_volumePage = static_cast<UIVolumePage const &>(rhs);
		
		SetCellPadding(rhs_volumePage.GetCellPadding());
		SetCellSize(rhs_volumePage.GetCellSize());
		SetCellCountFixed(rhs_volumePage.GetCellCountFixed());
		mCellMax = rhs_volumePage.mCellMax;
		mCellForceDragable = rhs_volumePage.mCellForceDragable;
		mCellSelectable = rhs_volumePage.mCellSelectable;
		SetMultiSelection(rhs_volumePage.GetMultiSelection());
		mCellAlignmentVertical = rhs_volumePage.mCellAlignmentVertical;
		mCellAlignmentHorizontal = rhs_volumePage.mCellAlignmentHorizontal;
		mCellCount = rhs_volumePage.mCellCount;
		SetCellSelectionOverlay(const_cast<UIRectangleStyle *>(rhs_volumePage.GetCellSelectionOverlay()));
		mCellSelctionOverlayOpacity = rhs_volumePage.mCellSelctionOverlayOpacity;
		mDoNotOverrideMaximumSize = rhs_volumePage.mDoNotOverrideMaximumSize;
	}

	SetPackDirty(true);
}

//----------------------------------------------------------------------

bool UIVolumePage::OptimizeChildSpacing(int const count)
{
	bool optimized = false;

	if (count > 0) 
	{
		UISize const & volumeSize = GetSize();
		if (volumeSize.x > 0 && volumeSize.y > 0)
		{
			float countSqrt = sqrtf(static_cast<float>(count));

			UIScalar const widgetWidth = static_cast<UIScalar>(volumeSize.x / floor(countSqrt));
			UIScalar const widgetHeight = static_cast<UIScalar>(volumeSize.y / countSqrt);

			UISize const cellSize(widgetWidth, widgetHeight);

			SetCellSize(cellSize);
			SetCellPadding(UISize::zero);

			optimized = true;
		}
	}

	if (optimized)
	{
		SetPackDirty(true);
	}

	return optimized;
}

//-----------------------------------------------------------------
