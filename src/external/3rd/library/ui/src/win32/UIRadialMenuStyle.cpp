#include "_precompile.h"

#include "UIButtonStyle.h"
#include "UIImageStyle.h"
#include "UIPopupMenuStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIRadialMenuStyle.h"
#include "UIUtils.h"

#include <vector>

const char * const UIRadialMenuStyle::TypeName                         = "RadialMenuStyle";

//----------------------------------------------------------------------

const UILowerString  UIRadialMenuStyle::PropertyName::ItemHeight         = UILowerString ("ItemHeight");
const UILowerString  UIRadialMenuStyle::PropertyName::ItemMargin         = UILowerString ("ItemMargin");
const UILowerString  UIRadialMenuStyle::PropertyName::ItemRadius         = UILowerString ("ItemRadius");
const UILowerString  UIRadialMenuStyle::PropertyName::ButtonStyle_NW     = UILowerString ("ButtonStyleNW");
const UILowerString  UIRadialMenuStyle::PropertyName::ButtonStyle_N      = UILowerString ("ButtonStyleN");
const UILowerString  UIRadialMenuStyle::PropertyName::ButtonStyle_NE     = UILowerString ("ButtonStyleNE");
const UILowerString  UIRadialMenuStyle::PropertyName::ButtonStyle_E      = UILowerString ("ButtonStyleE");
const UILowerString  UIRadialMenuStyle::PropertyName::ButtonStyle_SE     = UILowerString ("ButtonStyleSE");
const UILowerString  UIRadialMenuStyle::PropertyName::ButtonStyle_S      = UILowerString ("ButtonStyleS");
const UILowerString  UIRadialMenuStyle::PropertyName::ButtonStyle_SW     = UILowerString ("ButtonStyleSW");
const UILowerString  UIRadialMenuStyle::PropertyName::ButtonStyle_W      = UILowerString ("ButtonStyleW");
const UILowerString  UIRadialMenuStyle::PropertyName::ItemPopupTimeout   = UILowerString ("ItemPopupTimeout");
const UILowerString  UIRadialMenuStyle::PropertyName::PopupStyle_NW     = UILowerString ("PopupStyleNW");
const UILowerString  UIRadialMenuStyle::PropertyName::PopupStyle_N      = UILowerString ("PopupStyleN");
const UILowerString  UIRadialMenuStyle::PropertyName::PopupStyle_NE     = UILowerString ("PopupStyleNE");
const UILowerString  UIRadialMenuStyle::PropertyName::PopupStyle_E      = UILowerString ("PopupStyleE");
const UILowerString  UIRadialMenuStyle::PropertyName::PopupStyle_SE     = UILowerString ("PopupStyleSE");
const UILowerString  UIRadialMenuStyle::PropertyName::PopupStyle_S      = UILowerString ("PopupStyleS");
const UILowerString  UIRadialMenuStyle::PropertyName::PopupStyle_SW     = UILowerString ("PopupStyleSW");
const UILowerString  UIRadialMenuStyle::PropertyName::PopupStyle_W      = UILowerString ("PopupStyleW");

//======================================================================================
#define _TYPENAME UIRadialMenuStyle

namespace UIRadialMenuStyleNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(ItemRadius, "", T_int),
		_DESCRIPTOR(ItemHeight, "", T_int),
		_DESCRIPTOR(ItemMargin, "", T_point),
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================
	//================================================================
	// Behavior category.
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(ItemPopupTimeout, "", T_int),
	_GROUPEND(Behavior, 3, int(UIPropertyCategories::C_Behavior));
	//================================================================
	//================================================================
	// AdvancedAppearance category.
	_GROUPBEGIN(AdvancedAppearance)
		_DESCRIPTOR(ButtonStyle_E, "", T_object),
		_DESCRIPTOR(ButtonStyle_N, "", T_object),
		_DESCRIPTOR(ButtonStyle_NE, "", T_object),
		_DESCRIPTOR(ButtonStyle_NW, "", T_object),
		_DESCRIPTOR(ButtonStyle_S, "", T_object),
		_DESCRIPTOR(ButtonStyle_SE, "", T_object),
		_DESCRIPTOR(ButtonStyle_SW, "", T_object),
		_DESCRIPTOR(ButtonStyle_W, "", T_object),
		_DESCRIPTOR(PopupStyle_E, "", T_object),
		_DESCRIPTOR(PopupStyle_N, "", T_object),
		_DESCRIPTOR(PopupStyle_NE, "", T_object),
		_DESCRIPTOR(PopupStyle_NW, "", T_object),
		_DESCRIPTOR(PopupStyle_S, "", T_object),
		_DESCRIPTOR(PopupStyle_SE, "", T_object),
		_DESCRIPTOR(PopupStyle_SW, "", T_object),
		_DESCRIPTOR(PopupStyle_W, "", T_object),
	_GROUPEND(AdvancedAppearance, 3, int(UIPropertyCategories::C_AdvancedAppearance));
	//================================================================
}
using namespace UIRadialMenuStyleNamespace;
//======================================================================================

UIRadialMenuStyle::UIRadialMenuStyle() :
UIWidgetStyle (),
mItemHeight (16L),
mItemMargin (8L,4L),
mItemPopupTimeout (10L),
mItemRadius       (64L)
{
	for (size_t i = 0; i < static_cast<size_t>(BST_NumButtonStyles); ++i)
	{
		mButtonStyles [i] = 0;
		mPopupStyles  [i] = 0;
	}
}

//----------------------------------------------------------------------

UIRadialMenuStyle::~UIRadialMenuStyle()
{
	for (size_t i = 0; i < static_cast<size_t>(BST_NumButtonStyles); ++i)
	{
		SetButtonStyle (static_cast<ButtonStyleType>(i), 0);
		SetPopupStyle  (static_cast<ButtonStyleType>(i), 0);
	}
}

//----------------------------------------------------------------------

bool UIRadialMenuStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUIRadialMenuStyle) || UIWidgetStyle::IsA( Type );
}

//----------------------------------------------------------------------

const char *UIRadialMenuStyle::GetTypeName( void ) const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UIRadialMenuStyle::Clone( void ) const
{
	return new UIRadialMenuStyle;
}

//----------------------------------------------------------------------
void UIRadialMenuStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back (PropertyName::ButtonStyle_E  );
	In.push_back (PropertyName::ButtonStyle_N  );
	In.push_back (PropertyName::ButtonStyle_NE );
	In.push_back (PropertyName::ButtonStyle_NW );
	In.push_back (PropertyName::ButtonStyle_S  );
	In.push_back (PropertyName::ButtonStyle_SE );
	In.push_back (PropertyName::ButtonStyle_SW );
	In.push_back (PropertyName::ButtonStyle_W  );
	In.push_back (PropertyName::PopupStyle_E  );
	In.push_back (PropertyName::PopupStyle_N  );
	In.push_back (PropertyName::PopupStyle_NE );
	In.push_back (PropertyName::PopupStyle_NW );
	In.push_back (PropertyName::PopupStyle_S  );
	In.push_back (PropertyName::PopupStyle_SE );
	In.push_back (PropertyName::PopupStyle_SW );
	In.push_back (PropertyName::PopupStyle_W  );

	UIWidgetStyle::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIRadialMenuStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedAppearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIRadialMenuStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back (PropertyName::ButtonStyle_E  );
	In.push_back (PropertyName::ButtonStyle_N  );
	In.push_back (PropertyName::ButtonStyle_NE );
	In.push_back (PropertyName::ButtonStyle_NW );
	In.push_back (PropertyName::ButtonStyle_S  );
	In.push_back (PropertyName::ButtonStyle_SE );
	In.push_back (PropertyName::ButtonStyle_SW );
	In.push_back (PropertyName::ButtonStyle_W  );
	In.push_back (PropertyName::PopupStyle_E  );
	In.push_back (PropertyName::PopupStyle_N  );
	In.push_back (PropertyName::PopupStyle_NE );
	In.push_back (PropertyName::PopupStyle_NW );
	In.push_back (PropertyName::PopupStyle_S  );
	In.push_back (PropertyName::PopupStyle_SE );
	In.push_back (PropertyName::PopupStyle_SW );
	In.push_back (PropertyName::PopupStyle_W  );

	In.push_back( PropertyName::ItemRadius );
	In.push_back( PropertyName::ItemHeight );
	In.push_back( PropertyName::ItemMargin );
	In.push_back( PropertyName::ItemPopupTimeout );

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UIRadialMenuStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	bool isButton = false;

	const ButtonStyleType bst = LookupButtonStyleType (Name, isButton);

	if (bst != BST_NumButtonStyles)
	{
		if (isButton)
		{
			UIButtonStyle * const NewStyle = static_cast<UIButtonStyle *>(GetObjectFromPath(Value, TUIButtonStyle));

			if (NewStyle || Value.empty())
			{
				SetButtonStyle (bst, NewStyle);
			}
		}
		else
		{
			UIPopupMenuStyle * const NewStyle = static_cast<UIPopupMenuStyle *>(GetObjectFromPath(Value, TUIPopupMenuStyle));

			if (NewStyle || Value.empty())
			{
				SetPopupStyle (bst, NewStyle);
			}
		}
	}

	else if ( Name == PropertyName::ItemHeight )
	{
		return UIUtils::ParseLong (Value, mItemHeight);
	}
	else if ( Name == PropertyName::ItemRadius )
	{
		if (UIUtils::ParseLong (Value, mItemRadius))
		{
			FireStyleChanged ();
			return true;
		}
		return false;
	}
	else if ( Name == PropertyName::ItemMargin )
	{
		return UIUtils::ParsePoint (Value, mItemMargin);
	}
	else if ( Name == PropertyName::ItemPopupTimeout )
	{
		long tmp;
		if (UIUtils::ParseLong (Value, tmp))
		{
			mItemPopupTimeout = tmp < 0 ? 0 : static_cast<unsigned long>(tmp);
			return true;
		}

		return false;
	}

	return UIWidgetStyle::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIRadialMenuStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{

	bool isButton = false;
	const ButtonStyleType bst = LookupButtonStyleType (Name, isButton);

	if (bst != BST_NumButtonStyles)
	{
		if (isButton)
		{
			if (mButtonStyles [static_cast<size_t>(bst)])
			{
				GetPathTo( Value, mButtonStyles [static_cast<size_t>(bst)]);
				return true;
			}
		}
		else
		{
			if (mPopupStyles [static_cast<size_t>(bst)])
			{
				GetPathTo( Value, mPopupStyles [static_cast<size_t>(bst)]);
				return true;
			}
		}
	}

	else if ( Name == PropertyName::ItemHeight )
	{
		return UIUtils::FormatLong (Value, mItemHeight);
	}
	else if ( Name == PropertyName::ItemRadius )
	{
		return UIUtils::FormatLong (Value, mItemRadius);
	}
	else if ( Name == PropertyName::ItemMargin )
	{
		return UIUtils::FormatPoint (Value, mItemMargin);
	}
	else if ( Name == PropertyName::ItemPopupTimeout )
	{
		return UIUtils::FormatLong (Value, mItemPopupTimeout);
	}

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

void UIRadialMenuStyle::SetButtonStyle (ButtonStyleType type, UIButtonStyle * style)
{
	const size_t index = static_cast<size_t>(type);

	AttachMember (mButtonStyles [index], style);

	FireStyleChanged ();
}

//-----------------------------------------------------------------

void UIRadialMenuStyle::Link( void )
{
	UIWidgetStyle::Link ();

	for (size_t i = 0; i < static_cast<size_t>(BST_NumButtonStyles); ++i)
	{
		if (mButtonStyles [i])
			mButtonStyles [i]->Link ();
		if (mPopupStyles [i])
			mPopupStyles [i]->Link ();
	}
}

//----------------------------------------------------------------------

UIRadialMenuStyle::ButtonStyleType  UIRadialMenuStyle::LookupButtonStyleType (const UILowerString & Name, bool & isButton) const
{
	isButton = true;

	if (Name == PropertyName::ButtonStyle_N)
		return BST_N;
	if (Name == PropertyName::ButtonStyle_NE)
		return BST_NE;
	if (Name == PropertyName::ButtonStyle_NW)
		return BST_NW;
	if (Name == PropertyName::ButtonStyle_S)
		return BST_S;
	if (Name == PropertyName::ButtonStyle_SE)
		return BST_SE;
	if (Name == PropertyName::ButtonStyle_SW)
		return BST_SW;
	if (Name == PropertyName::ButtonStyle_E)
		return BST_E;
	if (Name == PropertyName::ButtonStyle_W)
		return BST_W;

	isButton = false;

	if (Name == PropertyName::PopupStyle_N)
		return BST_N;
	if (Name == PropertyName::PopupStyle_NE)
		return BST_NE;
	if (Name == PropertyName::PopupStyle_NW)
		return BST_NW;
	if (Name == PropertyName::PopupStyle_S)
		return BST_S;
	if (Name == PropertyName::PopupStyle_SE)
		return BST_SE;
	if (Name == PropertyName::PopupStyle_SW)
		return BST_SW;
	if (Name == PropertyName::PopupStyle_E)
		return BST_E;
	if (Name == PropertyName::PopupStyle_W)
		return BST_W;

	return BST_NumButtonStyles;
}

//----------------------------------------------------------------------

void UIRadialMenuStyle::SetPopupStyle (ButtonStyleType type, UIPopupMenuStyle * style)
{
	const size_t index = static_cast<size_t>(type);

	AttachMember (mPopupStyles [index], style);

	FireStyleChanged ();
}

//----------------------------------------------------------------------
