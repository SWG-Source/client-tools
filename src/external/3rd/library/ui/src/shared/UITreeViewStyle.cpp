//======================================================================
//
// UITreeViewStyle.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UITreeViewStyle.h"

#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIPalette.h"
#include "UIPropertyDescriptor.h"
#include "UIText.h"
#include "UIUtils.h"

#include <cassert>
#include <vector>

//======================================================================

const char * const UITreeViewStyle::TypeName = "TreeViewStyle";

const UILowerString UITreeViewStyle::PropertyName::CellHeight                     = UILowerString ("CellHeight");
const UILowerString UITreeViewStyle::PropertyName::CellPadding                    = UILowerString ("CellPadding");
const UILowerString UITreeViewStyle::PropertyName::DefaultTextColor               = UILowerString ("DefaultTextColor");
const UILowerString UITreeViewStyle::PropertyName::DefaultTextColorHighlight      = UILowerString ("DefaultTextColorHighlight");
const UILowerString UITreeViewStyle::PropertyName::DefaultTextColorLowlight       = UILowerString ("DefaultTextColorLowlight");
const UILowerString UITreeViewStyle::PropertyName::DefaultTextStyle               = UILowerString ("DefaultTextStyle");
const UILowerString UITreeViewStyle::PropertyName::GridColor                      = UILowerString ("GridColor");
const UILowerString UITreeViewStyle::PropertyName::ImageStyleCollapsed            = UILowerString ("ImageStyleCollapsed");
const UILowerString UITreeViewStyle::PropertyName::ImageStyleExpanded             = UILowerString ("ImageStyleExpanded");
const UILowerString UITreeViewStyle::PropertyName::Indentation                    = UILowerString ("Indentation");
const UILowerString UITreeViewStyle::PropertyName::SelectionColorBackground       = UILowerString ("SelectionColorBackground");
const UILowerString UITreeViewStyle::PropertyName::SelectionColorRect             = UILowerString ("SelectionColorRect");
const UILowerString UITreeViewStyle::PropertyName::SelectionTextColor             = UILowerString ("SelectionTextColor");
const UILowerString UITreeViewStyle::PropertyName::SelectionTextColorHighlight    = UILowerString ("SelectionTextColorHighlight");
const UILowerString UITreeViewStyle::PropertyName::SelectionTextColorLowlight     = UILowerString ("SelectionTextColorLowlight");

//======================================================================================
#define _TYPENAME UITreeViewStyle

namespace UITreeViewStyleNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(CellPadding, "", T_point),
		_DESCRIPTOR(CellHeight, "", T_int),
		_DESCRIPTOR(Indentation, "", T_int),
	_GROUPEND(Basic, 3, 0);
	//================================================================

	//================================================================
	// Text category
	_GROUPBEGIN(Text)
		_DESCRIPTOR(DefaultTextStyle, "", T_object),
	_GROUPEND(Text, 3, 1);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(GridColor, "", T_color),
		_DESCRIPTOR(DefaultTextColor, "", T_color),
		_DESCRIPTOR(DefaultTextColorHighlight, "", T_color),
		_DESCRIPTOR(DefaultTextColorLowlight, "", T_color),
		_DESCRIPTOR(SelectionTextColor, "", T_color),
		_DESCRIPTOR(SelectionTextColorHighlight, "", T_color),
		_DESCRIPTOR(SelectionTextColorLowlight, "", T_color),
		_DESCRIPTOR(SelectionColorBackground, "", T_color),
		_DESCRIPTOR(SelectionColorRect, "", T_color),
		_DESCRIPTOR(ImageStyleCollapsed, "", T_object),
		_DESCRIPTOR(ImageStyleExpanded, "", T_object),
	_GROUPEND(Appearance, 3, 2);
	//================================================================
}
using namespace UITreeViewStyleNamespace;
//======================================================================================

UITreeViewStyle::UITreeViewStyle () :
UIWidgetStyle             (),
mGridColor                (UIColor::black),
mCellHeight               (16L),
mCellPadding              (2L, 2L),
mDefaultRendererText      (new UIText),
mDefaultRendererImage     (new UIImage),
mSelectionColorBackground (UIColor (0xff, 0xff, 0x88)),
mSelectionColorRect       (UIColor (0xff, 0x55, 0xcc)),
mIndentation              (16L)
{
	mDefaultRendererText->Attach (this);
	mDefaultRendererImage->Attach (this);

	{
	for (int i = 0; i < IST_count; ++i)
		mImageStyles [i] = 0;
	}

	for (int i = 0; i < DTC_count; ++i)
	{
		mDefaultTextColors   [i] = UIColor::white;
		mSelectionTextColors [i] = UIColor::cyan;
	}

	mDefaultTextColors   [DTC_highlight] = UIColor (0xaa,0xff,0xff);
	mDefaultTextColors   [DTC_lowlight]  = UIColor (0xaa,0xaa,0xaa);

	mSelectionTextColors [DTC_highlight] = UIColor (0xaa,0xff,0xff);
	mSelectionTextColors [DTC_lowlight]  = UIColor (0xaa,0xaa,0xaa);
}

//----------------------------------------------------------------------

UITreeViewStyle::~UITreeViewStyle ()
{
	mDefaultRendererText->Detach (this);
	mDefaultRendererImage->Detach (this);

	for (int i = 0; i < IST_count; ++i)
		SetImageStyle (static_cast<ImageStyleType>(i), 0);
}

//----------------------------------------------------------------------

bool UITreeViewStyle::IsA( const UITypeID type ) const
{
	return type == TUITreeViewStyle || UIWidgetStyle::IsA (type);
}

//----------------------------------------------------------------------
void UITreeViewStyle::GetLinkPropertyNames( UIPropertyNameVector & in) const
{
	in.push_back (PropertyName::DefaultTextStyle         );
	in.push_back (PropertyName::ImageStyleCollapsed );
	in.push_back (PropertyName::ImageStyleExpanded );

	UIWidgetStyle::GetLinkPropertyNames (in);
}

//----------------------------------------------------------------------

void UITreeViewStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Text, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UITreeViewStyle::GetPropertyNames( UIPropertyNameVector & in, bool forCopy) const
{
	UIPalette::GetPropertyNamesForType (TUITreeViewStyle, in);

	in.push_back (PropertyName::CellPadding              );
	in.push_back (PropertyName::CellHeight               );
	in.push_back (PropertyName::GridColor                );
	in.push_back (PropertyName::DefaultTextStyle         );
	in.push_back (PropertyName::DefaultTextColor         );
	in.push_back (PropertyName::DefaultTextColorHighlight);
	in.push_back (PropertyName::DefaultTextColorLowlight);
	in.push_back (PropertyName::SelectionTextColor         );
	in.push_back (PropertyName::SelectionTextColorHighlight);
	in.push_back (PropertyName::SelectionTextColorLowlight);
	in.push_back (PropertyName::SelectionColorBackground );
	in.push_back (PropertyName::SelectionColorRect       );
	in.push_back (PropertyName::ImageStyleCollapsed );
	in.push_back (PropertyName::ImageStyleExpanded );
	in.push_back (PropertyName::Indentation );

	UIWidgetStyle::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UITreeViewStyle::SetProperty( const UILowerString & Name, const UIString &Value )
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
		return UIUtils::ParseColor (Value, mDefaultTextColors [DTC_normal]);
	}
	else if (Name == PropertyName::DefaultTextColorHighlight)
	{
		return UIUtils::ParseColor (Value, mDefaultTextColors [DTC_highlight]);
	}
	else if (Name == PropertyName::DefaultTextColorLowlight)
	{
		return UIUtils::ParseColor (Value, mDefaultTextColors [DTC_lowlight]);
	}
	else if (Name == PropertyName::SelectionTextColor)
	{
		return UIUtils::ParseColor (Value, mSelectionTextColors [DTC_normal]);
	}
	else if (Name == PropertyName::SelectionTextColorHighlight)
	{
		return UIUtils::ParseColor (Value, mSelectionTextColors [DTC_highlight]);
	}
	else if (Name == PropertyName::SelectionTextColorLowlight)
	{
		return UIUtils::ParseColor (Value, mSelectionTextColors [DTC_lowlight]);
	}
	else if ( Name == PropertyName::SelectionColorBackground )
	{
		return UIUtils::ParseColor (Value, mSelectionColorBackground);
	}
	else if ( Name == PropertyName::SelectionColorRect )
	{
		return UIUtils::ParseColor (Value, mSelectionColorRect);
	}
	else if ( Name == PropertyName::Indentation )
	{
		return UIUtils::ParseLong (Value, mIndentation);
	}
	else if ( Name == PropertyName::ImageStyleCollapsed )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUIImageStyle );

		if( object || Value.empty() )
		{
			SetImageStyle( IST_collapsed, static_cast<UIImageStyle *>(object) );
			return true;
		}
	}
	else if ( Name == PropertyName::ImageStyleExpanded )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUIImageStyle );

		if( object || Value.empty() )
		{
			SetImageStyle( IST_expanded, static_cast<UIImageStyle *>(object) );
			return true;
		}
	}
	else
	{
		UIPalette::SetPropertyForObject (*this, Name, Value);
	}

	return UIWidgetStyle::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UITreeViewStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
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
		mDefaultRendererText->SetParent (const_cast<UITreeViewStyle *>(this));
		const bool retval = mDefaultRendererText->GetProperty (UIText::PropertyName::Style, Value);
		mDefaultRendererText->SetParent (0);

		return retval;
	}
	else if (Name == PropertyName::DefaultTextColor)
	{
		return UIUtils::FormatColor (Value, mDefaultTextColors [DTC_normal]);
	}
	else if (Name == PropertyName::DefaultTextColorHighlight)
	{
		return UIUtils::FormatColor (Value, mDefaultTextColors [DTC_highlight]);
	}
	else if (Name == PropertyName::DefaultTextColorLowlight)
	{
		return UIUtils::FormatColor (Value, mDefaultTextColors [DTC_lowlight]);
	}
	else if (Name == PropertyName::SelectionTextColor)
	{
		return UIUtils::FormatColor (Value, mSelectionTextColors [DTC_normal]);
	}
	else if (Name == PropertyName::SelectionTextColorHighlight)
	{
		return UIUtils::FormatColor (Value, mSelectionTextColors [DTC_highlight]);
	}
	else if (Name == PropertyName::SelectionTextColorLowlight)
	{
		return UIUtils::FormatColor (Value, mSelectionTextColors [DTC_lowlight]);
	}
	else if ( Name == PropertyName::SelectionColorBackground )
	{
		return UIUtils::FormatColor (Value, mSelectionColorBackground);
	}
	else if ( Name == PropertyName::SelectionColorRect )
	{
		return UIUtils::FormatColor (Value, mSelectionColorRect);
	}
	else if ( Name == PropertyName::Indentation )
	{
		return UIUtils::FormatLong (Value, mIndentation);
	}

	else if ( Name == PropertyName::ImageStyleCollapsed )
	{
		if( mImageStyles [IST_collapsed] )
		{
			GetPathTo( Value, mImageStyles [IST_collapsed] );
			return true;
		}
	}
	else if ( Name == PropertyName::ImageStyleExpanded )
	{
		if( mImageStyles [IST_expanded] )
		{
			GetPathTo( Value, mImageStyles [IST_expanded] );
			return true;
		}
	}

	return UIWidgetStyle::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

void UITreeViewStyle::Link()
{
	UIWidgetStyle::Link ();

	mDefaultRendererText->SetParent (this);
	mDefaultRendererText->Link ();
	mDefaultRendererText->SetParent (0);

	mDefaultRendererImage->SetParent (this);
	mDefaultRendererImage->Link ();
	mDefaultRendererImage->SetParent (0);

	for (int i = 0; i < IST_count; ++i)
	{
		if (mImageStyles [i])
			mImageStyles [i]->Link ();
	}
}

//-----------------------------------------------------------------

void UITreeViewStyle::SetGridColor (const UIColor & color)
{
	mGridColor = color;
}

//-----------------------------------------------------------------

void UITreeViewStyle::SetCellPadding (const UISize & size)
{
	mCellPadding = size;
}

//-----------------------------------------------------------------

void UITreeViewStyle::SetCellHeight (long height)
{
	mCellHeight = height;
}

//----------------------------------------------------------------------

void UITreeViewStyle::SetImageStyle (ImageStyleType type, UIImageStyle * style)
{
	UIImageStyle *& stylePtr = mImageStyles [type];

	AttachMember (stylePtr, style);
}

//----------------------------------------------------------------------

UIImage & UITreeViewStyle::GetDefaultImage (ImageStyleType type)
{
	UIImageStyle * const style = GetImageStyle (type);
	return GetDefaultImage (style);
}

//----------------------------------------------------------------------

UIImage & UITreeViewStyle::GetDefaultImage (UIImageStyle * style)
{
	mDefaultRendererImage->SetStyle (style);
	const long size = mIndentation == 0 ? mCellHeight : (std::min (mIndentation, mCellHeight));
	mDefaultRendererImage->SetSize (UISize (size, size));
	mDefaultRendererImage->SetLocation ((mIndentation - size) / 2, (mCellHeight - size) / 2);
	return *mDefaultRendererImage;
}

//----------------------------------------------------------------------

UIText & UITreeViewStyle::GetDefaultText (DefaultTextColors index, bool selected)
{
	mDefaultRendererText->SetTextColor (GetDefaultTextColor (index, selected));
	return *mDefaultRendererText;
}

//----------------------------------------------------------------------

UIImageStyle * UITreeViewStyle::GetImageStyle (ImageStyleType type)
{
	assert (type >= 0 && type < IST_count);
	return mImageStyles [type];
}

//----------------------------------------------------------------------

const UIColor & UITreeViewStyle::GetDefaultTextColor (DefaultTextColors index, bool selected)
{
	assert (index >= DTC_normal && index < DTC_count);

	if (selected)
		return mDefaultTextColors [static_cast<size_t>(index)];
	else
		return mSelectionTextColors [static_cast<size_t>(index)];
}

//======================================================================
