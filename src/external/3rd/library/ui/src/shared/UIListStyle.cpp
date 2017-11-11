//======================================================================
//
// UIListStyle.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIListStyle.h"

#include "UIImage.h"
#include "UIPalette.h"
#include "UIText.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

//======================================================================

const char * const UIListStyle::TypeName = "ListStyle";

const UILowerString  UIListStyle::PropertyName::BackgroundColor          = UILowerString ("BackgroundColor");
const UILowerString  UIListStyle::PropertyName::BackgroundOpacity        = UILowerString ("BackgroundOpacity");
const UILowerString  UIListStyle::PropertyName::CellHeight               = UILowerString ("CellHeight");
const UILowerString  UIListStyle::PropertyName::CellPadding              = UILowerString ("CellPadding");
const UILowerString  UIListStyle::PropertyName::DefaultTextColor         = UILowerString ("DefaultTextColor");
const UILowerString  UIListStyle::PropertyName::DefaultTextStyle         = UILowerString ("DefaultTextStyle");
const UILowerString  UIListStyle::PropertyName::GridColor                = UILowerString ("GridColor");
const UILowerString  UIListStyle::PropertyName::Margin                   = UILowerString ("Margin");
const UILowerString  UIListStyle::PropertyName::SelectionColorBackground = UILowerString ("SelectionColorBackground");
const UILowerString  UIListStyle::PropertyName::SelectionColorRect       = UILowerString ("SelectionColorRect");
const UILowerString  UIListStyle::PropertyName::SelectionTextColor       = UILowerString ("SelectionTextColor");

//======================================================================================
#define _TYPENAME UIListStyle

namespace UIListStyleNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(CellHeight, "", T_int),
		_DESCRIPTOR(CellPadding, "", T_point),
		_DESCRIPTOR(Margin, "", T_rect),
	_GROUPEND(Basic, 3, 0);
	//================================================================

	//================================================================
	// Text category
	_GROUPBEGIN(Text)
		_DESCRIPTOR(DefaultTextStyle, "", T_object),
	_GROUPEND(Text, 3, 0);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(BackgroundColor, "", T_color),
		_DESCRIPTOR(BackgroundOpacity, "", T_float),
		_DESCRIPTOR(DefaultTextColor, "", T_color),
		_DESCRIPTOR(GridColor, "", T_color),
		_DESCRIPTOR(SelectionColorBackground, "", T_color),
		_DESCRIPTOR(SelectionColorRect, "", T_color),
		_DESCRIPTOR(SelectionTextColor, "", T_color),
	_GROUPEND(Appearance, 3, 1);
	//================================================================
}
using namespace UIListStyleNamespace;
//======================================================================================

UIListStyle::UIListStyle () :
UIWidgetStyle             (),
mGridColor                (UIColor::black),
mCellHeight               (16L),
mCellPadding              (2L, 2L),
mDefaultRendererText      (new UIText),
mDefaultRendererImage     (new UIImage),
mSelectionColorBackground (UIColor (0xff, 0xff, 0x88)),
mSelectionColorRect       (UIColor (0xff, 0x55, 0xcc)),
mSelectionTextColor       (UIColor (0x00, 0x00, 0xff)),
mDefaultTextColor         (UIColor::white),
mMargin                   (),
mBackgroundColor          (UIColor (0x33, 0x55, 0x66)),
mBackgroundOpacity        (0.8f)
{

	mDefaultRendererText->Attach (this);
	mDefaultRendererImage->Attach (this);
}

//----------------------------------------------------------------------

UIListStyle::~UIListStyle ()
{
	mDefaultRendererText->Detach (this);
	mDefaultRendererImage->Detach (this);
}

//----------------------------------------------------------------------

bool UIListStyle::IsA( const UITypeID type ) const
{
	return type == TUIListStyle || UIWidgetStyle::IsA (type);
}

//----------------------------------------------------------------------

void UIListStyle::GetLinkPropertyNames( UIPropertyNameVector & in) const
{
	in.push_back (PropertyName::DefaultTextStyle         );

	UIWidgetStyle::GetPropertyNames (in, false);
}

//----------------------------------------------------------------------

void UIListStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Text, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIListStyle::GetPropertyNames( UIPropertyNameVector & in, bool forCopy) const
{
	UIPalette::GetPropertyNamesForType (TUIListStyle, in);

	in.push_back (PropertyName::BackgroundColor          );
	in.push_back (PropertyName::BackgroundOpacity        );
	in.push_back (PropertyName::CellHeight               );
	in.push_back (PropertyName::CellPadding              );
	in.push_back (PropertyName::DefaultTextColor         );
	in.push_back (PropertyName::DefaultTextStyle         );
	in.push_back (PropertyName::GridColor                );
	in.push_back (PropertyName::Margin                   );
	in.push_back (PropertyName::SelectionColorBackground );
	in.push_back (PropertyName::SelectionColorRect       );
	in.push_back (PropertyName::SelectionTextColor );

	UIWidgetStyle::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UIListStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::CellPadding              )
	{
		UIPoint pt;
		if (UIUtils::ParsePoint (Value, pt))
		{
			SetCellPadding (pt);
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::CellHeight               )
	{
		long height = -1;
		if (UIUtils::ParseLong (Value, height))
		{
			SetCellHeight (height);
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::GridColor      )
	{
		return UIUtils::ParseColor (Value, mGridColor);
	}

	else if (Name == PropertyName::DefaultTextStyle)
	{
		mDefaultRendererText->SetParent (this);
		mDefaultRendererText->SetProperty (UIText::PropertyName::Style, Value);
		mDefaultRendererText->SetParent (0);
		// fall through
	}
	else if (Name == PropertyName::DefaultTextColor)
	{
		return UIUtils::ParseColor (Value, mDefaultTextColor);
	}
	else if (Name == PropertyName::SelectionColorBackground )
	{
		return UIUtils::ParseColor (Value, mSelectionColorBackground);
	}
	else if (Name == PropertyName::SelectionColorRect )
	{
		return UIUtils::ParseColor (Value, mSelectionColorRect);
	}
	else if (Name == PropertyName::SelectionTextColor )
	{
		UIColor color;

		if (UIUtils::ParseColor (Value, color))
		{
			SetSelectionTextColor (color);
			return true;
		}
	}

	else if (Name == PropertyName::Margin )
	{
		UIRect rect;

		if (UIUtils::ParseRect (Value, rect))
		{
			SetMargin (rect);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::BackgroundColor )
	{
		UIColor color;

		if (UIUtils::ParseColor (Value, color))
		{
			SetBackgroundColor (color);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::BackgroundOpacity )
	{
		float f;

		if (UIUtils::ParseFloat (Value, f))
		{
			SetBackgroundOpacity (f);
			return true;
		}
		return false;
	}
	else
	{
		UIPalette::SetPropertyForObject (*this, Name, Value);
	}

	return UIWidgetStyle::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UIListStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::CellPadding              )
	{
		return UIUtils::FormatPoint (Value, mCellPadding);
	}

	else if (Name == PropertyName::CellHeight               )
	{
		return UIUtils::FormatLong (Value, mCellHeight);
	}

	else if (Name == PropertyName::GridColor      )
	{
		return UIUtils::FormatColor (Value, mGridColor);
	}

	else if (Name == PropertyName::DefaultTextStyle)
	{
		mDefaultRendererText->SetParent (const_cast<UIListStyle *>(this));
		const bool retval = mDefaultRendererText->GetProperty (UIText::PropertyName::Style, Value);
		mDefaultRendererText->SetParent (0);

		return retval;
	}
	else if (Name == PropertyName::DefaultTextColor)
	{
		return UIUtils::FormatColor (Value, mDefaultTextColor);
	}
	else if (Name == PropertyName::SelectionColorBackground )
	{
		return UIUtils::FormatColor (Value, mSelectionColorBackground);
	}
	else if (Name == PropertyName::SelectionColorRect )
	{
		return UIUtils::FormatColor (Value, mSelectionColorRect);
	}
	else if (Name == PropertyName::SelectionTextColor )
	{
		return UIUtils::FormatColor (Value, mSelectionTextColor);
	}
	else if (Name == PropertyName::Margin )
	{
		return UIUtils::FormatRect (Value, mMargin);
	}
	else if (Name == PropertyName::BackgroundColor )
	{
		return UIUtils::FormatColor (Value, mBackgroundColor);
	}
	else if (Name == PropertyName::BackgroundOpacity )
	{
		return UIUtils::FormatFloat (Value, mBackgroundOpacity);
	}
	return UIWidgetStyle::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

void UIListStyle::Link()
{
	UIWidgetStyle::Link ();

	mDefaultRendererText->SetParent (this);
	mDefaultRendererText->Link ();
	mDefaultRendererText->SetParent (0);

	mDefaultRendererImage->SetParent (this);
	mDefaultRendererImage->Link ();
	mDefaultRendererImage->SetParent (0);
}

//-----------------------------------------------------------------

void UIListStyle::SetGridColor (const UIColor & color)
{
	if (mGridColor != color)
	{
		mGridColor = color;
		FireStyleChanged ();
	}
}

//-----------------------------------------------------------------

void UIListStyle::SetCellPadding (const UISize & size)
{
	if (size != mCellPadding)
	{
		mCellPadding = size;
		FireStyleChanged ();
	}
}

//-----------------------------------------------------------------

void UIListStyle::SetCellHeight (long height)
{
	if (height != mCellHeight)
	{
		mCellHeight = height;
		FireStyleChanged ();
	}
}

//----------------------------------------------------------------------

void UIListStyle::SetMargin (const UIRect & rect)
{
	if (rect != mMargin)
	{
		mMargin = rect;
		FireStyleChanged ();
	}
}

//----------------------------------------------------------------------

void UIListStyle::SetBackgroundOpacity (float f)
{
	if (mBackgroundOpacity != f)
	{
		mBackgroundOpacity = f;
		FireStyleChanged ();
	}
}

//----------------------------------------------------------------------

void UIListStyle::SetBackgroundColor (const UIColor & color)
{
	if (mBackgroundColor != color)
	{
		mBackgroundColor = color;
		FireStyleChanged ();
	}
}

//----------------------------------------------------------------------

void UIListStyle::SetSelectionTextColor (const UIColor & color)
{
	if (mSelectionTextColor != color)
	{
		mSelectionTextColor = color;
		FireStyleChanged ();
	}
}

//======================================================================
