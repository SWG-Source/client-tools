#include "_precompile.h"

#include "UIButton.h"
#include "UIPropertyDescriptor.h"
#include "UIScrollbar.h"
#include "UISliderbarStyle.h"
#include "UITabbedPane.h"
#include "UIUtils.h"

#include <cassert>
#include <cmath>
#include <vector>

const char * const UIScrollbar::TypeName                         = "Scrollbar";

const UILowerString UIScrollbar::PropertyName::AutoVisibility     = UILowerString ("AutoVisibility");
const UILowerString UIScrollbar::PropertyName::Control            = UILowerString ("Control");

//======================================================================================
#define _TYPENAME UIScrollbar

namespace UIScrollbarNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Control, "", T_object),
	_GROUPEND(Basic, 4, int(UIPropertyCategories::C_Basic));
	//================================================================
	//================================================================
	// Behavior category.
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(AutoVisibility, "", T_bool),
	_GROUPEND(Behavior, 4, int(UIPropertyCategories::C_Behavior));
	//================================================================

}
using namespace UIScrollbarNamespace;
//======================================================================================

UIScrollbar::UIScrollbar() :
UISliderbase     (),
mAttachedControl (0),
mState           (0)
{
}

//----------------------------------------------------------------------

UIScrollbar::~UIScrollbar()
{
	AttachToControl (0);
}

//----------------------------------------------------------------------

bool UIScrollbar::IsA( const UITypeID Type ) const
{
	return (Type == TUIScrollbar) || UISliderbase::IsA( Type );
}

//----------------------------------------------------------------------

const char *UIScrollbar::GetTypeName () const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UIScrollbar::Clone () const
{
	return new UIScrollbar;
}

//----------------------------------------------------------------------

bool UIScrollbar::CanSelect () const
{
	return false;
}


//----------------------------------------------------------------------

void UIScrollbar::AttachToControl( UIWidget *NewControl )
{
	AttachMember (mAttachedControl, NewControl);
}

//----------------------------------------------------------------------
void UIScrollbar::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::Control );

	UISliderbase::GetLinkPropertyNames( In );

}

//----------------------------------------------------------------------

void UIScrollbar::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UISliderbase::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
}

//----------------------------------------------------------------------

void UIScrollbar::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::AutoVisibility );
	In.push_back( PropertyName::Control );

	UISliderbase::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UIScrollbar::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::AutoVisibility )
	{
		bool b = false;

		if (UIUtils::ParseBoolean (Value, b))
		{
			if (b)
				mState |= BF_AutoVisibility;
			else
				mState &= ~BF_AutoVisibility;

			return true;
		}

		return false;
	}

	else if(Name == PropertyName::Control )
	{
		UIBaseObject *NewControl = GetObjectFromPath( Value, TUIWidget );

		if( NewControl || Value.empty() )
		{
			UIBaseObject * parent = GetParent ();

			//-- don't let the scrollbar attach to one of its parents
			while (parent)
			{
				if (parent == NewControl)
					return false;
				parent = parent->GetParent ();
			}

			AttachToControl( static_cast<UIWidget *>( NewControl ) );
			return true;
		}
	}

	return UISliderbase::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIScrollbar::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::AutoVisibility )
	{
		return (UIUtils::FormatBoolean (Value, (mState & BF_AutoVisibility) != 0));
	}
	else if( Name == PropertyName::Control )
	{
		if( mAttachedControl )
		{
			GetPathTo( Value, mAttachedControl );
			return true;
		}
	}

	return UISliderbase::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

long UIScrollbar::GetDesiredThumbSize          (long range) const
{
	if (!mAttachedControl)
		return 0;

	const UISliderbarStyle * const style = GetSliderbarStyle ();

	if (!style)
		return 0;

	long wLength  = 0L;
	long wVisible = 0L;

	UISize scrollExtent;
	mAttachedControl->GetScrollExtent (scrollExtent);

	if (style->GetLayout () == UIStyle::L_horizontal)
	{
		wVisible = mAttachedControl->GetWidth ();
		wLength  = scrollExtent.x;
	}
	else
	{
		wVisible = mAttachedControl->GetHeight ();
		wLength  = scrollExtent.y;
	}

	wLength = std::max (wVisible, wLength);

	if (!wLength)
		return 0;

	return range * wVisible / wLength;
}

//----------------------------------------------------------------------

/**
*/

long UIScrollbar::GetDesiredThumbPosition      (long range) const
{
	if (!mAttachedControl)
		return 0;

	const UISliderbarStyle * const style = GetSliderbarStyle ();

	if (!style)
		return 0;

	long wLength  = 0L;
	long wPos     = 0L;

	UISize scrollExtent;
	mAttachedControl->GetScrollExtent (scrollExtent);

	if (style->GetLayout () == UIStyle::L_horizontal)
	{
		wLength  = scrollExtent.x - mAttachedControl->GetWidth ();
		wPos     = mAttachedControl->GetScrollLocation ().x;
	}
	else
	{
		wLength  = scrollExtent.y - mAttachedControl->GetHeight ();
		wPos     = wLength - mAttachedControl->GetScrollLocation ().y;
	}

	wLength = std::max (0L,      wLength);
	wPos    = std::max (0L,      wPos);
	wPos    = std::min (wLength, wPos);

	if (!wLength)
		return 0;

	return range * wPos / wLength;
}

//----------------------------------------------------------------------

/**
* vertical scrollbars are inverted wrt vertical sliders
*/

void UIScrollbar::IncrementValue               (bool page, bool upDown, bool)
{
	if (!mAttachedControl)
		return;

	const UISliderbarStyle * const style = GetSliderbarStyle ();

	if (!style)
		return;

	UISize pageSize;
	UISize lineSize;

	UIPoint scrollLocation = mAttachedControl->GetScrollLocation ();

	if (mAttachedControl->IsA (TUITabbedPane))
	{
		UITabbedPane * const tabs = static_cast<UITabbedPane *> (mAttachedControl);
		tabs->GetScrollSizes (pageSize, lineSize, upDown, !upDown);
	}
	else
		mAttachedControl->GetScrollSizes (pageSize, lineSize);

	lineSize.x = std::max (1L, lineSize.x);
	lineSize.y = std::max (1L, lineSize.y);

	if (page)
	{
		if (style->GetLayout () == UIStyle::L_horizontal)
			scrollLocation.x += upDown ? pageSize.x : -pageSize.x;
		else
			scrollLocation.y += upDown ? -pageSize.y : pageSize.y;
	}
	else
	{
		if (style->GetLayout () == UIStyle::L_horizontal)
			scrollLocation.x += upDown ? lineSize.x : -lineSize.x;
		else
			scrollLocation.y += upDown ? -lineSize.y : lineSize.y;
	}

	mAttachedControl->ScrollToPoint (scrollLocation);
}

//----------------------------------------------------------------------

void UIScrollbar::UpdateValueFromThumbPosition (long position, long range, bool)
{
	if (!mAttachedControl || !range)
		return;

	const UISliderbarStyle * const style = GetSliderbarStyle ();

	if (!style)
		return;


	UIPoint pt = mAttachedControl->GetScrollLocation ();

	UISize scrollExtent;
	mAttachedControl->GetScrollExtent (scrollExtent);

	UISize pageSize;
	UISize lineSize;

	mAttachedControl->GetScrollSizes (pageSize, lineSize);
	lineSize.x = std::max (1L, lineSize.x);
	lineSize.y = std::max (1L, lineSize.y);

	if (style->GetLayout () == UIStyle::L_horizontal)
	{
		const long wLength  = std::max (0L, scrollExtent.x - mAttachedControl->GetWidth ());
		pt.x     = position * wLength / range;
		pt.x     = (pt.x / lineSize.x) * lineSize.x;
	}
	else
	{
		const long wLength  = std::max (0L, scrollExtent.y - mAttachedControl->GetHeight ());
		pt.y     = wLength - (position * wLength / range);
		pt.y     += (lineSize.y / 2);
		pt.y     = (pt.y / lineSize.y) * lineSize.y;
	}

	mAttachedControl->ScrollToPoint (pt);
}

//----------------------------------------------------------------------

void UIScrollbar::UpdatePerFrame               ()
{
//	if ((mState & BF_AutoVisibility) != 0)
	{
		if (mAttachedControl)
		{
			UISize scrollExtent;
			mAttachedControl->GetScrollExtent (scrollExtent);
			const bool enabled = scrollExtent != mAttachedControl->GetSize ();
			SetEnabled (enabled);
			UIButton * const thumb = GetThumb ();
			if (thumb)
				thumb->SetVisible (enabled);
		}
		else
			SetEnabled (false);
	}
}

//----------------------------------------------------------------------

