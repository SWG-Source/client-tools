#include "_precompile.h"

#include "UIButtonStyle.h"
#include "UIPopupMenuStyle.h"
#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

const char * const UIPopupMenuStyle::TypeName = "PopupMenuStyle";

const UILowerString UIPopupMenuStyle::PropertyName::BackgroundOpacity = UILowerString ("BackgroundOpacity");
const UILowerString UIPopupMenuStyle::PropertyName::ButtonStyle       = UILowerString ("ButtonStyle");
const UILowerString UIPopupMenuStyle::PropertyName::ButtonStyleLabel  = UILowerString ("ButtonStyleLabel");
const UILowerString UIPopupMenuStyle::PropertyName::IconSubmenu       = UILowerString ("IconSubmenu");
const UILowerString UIPopupMenuStyle::PropertyName::ItemHeight        = UILowerString ("ItemHeight");

//======================================================================================
#define _TYPENAME UIPopupMenuStyle

namespace UIPopupMenuStyleNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(ButtonStyle, "", T_object),
		_DESCRIPTOR(ButtonStyleLabel, "", T_object),
		_DESCRIPTOR(ItemHeight, "", T_int),
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================
	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(BackgroundOpacity, "", T_float),
	_GROUPEND(Appearance, 3, int(UIPropertyCategories::C_Appearance));
	//================================================================

	//================================================================
	// AdvancedAppearance category.
	_GROUPBEGIN(AdvancedAppearance)
		_DESCRIPTOR(IconSubmenu, "", T_object),
	_GROUPEND(AdvancedAppearance, 3, int(UIPropertyCategories::C_AdvancedAppearance));
	//================================================================
}
using namespace UIPopupMenuStyleNamespace;
//======================================================================================

UIPopupMenuStyle::UIPopupMenuStyle() :
UIWidgetStyle      (),
mButtonStyle       (0),
mButtonStyleLabel  (0),
mItemHeight        (16L),
mBackgroundOpacity (1.0f),
mIconSubmenu       (0)
{

}

//----------------------------------------------------------------------

UIPopupMenuStyle::~UIPopupMenuStyle()
{
	SetButtonStyle      (0);
	SetButtonStyleLabel (0);
	SetIconSubmenu      (0);
}

//----------------------------------------------------------------------

bool UIPopupMenuStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUIPopupMenuStyle) || UIWidgetStyle::IsA( Type );
}

//----------------------------------------------------------------------

const char *UIPopupMenuStyle::GetTypeName( void ) const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UIPopupMenuStyle::Clone( void ) const
{
	return new UIPopupMenuStyle;
}

//----------------------------------------------------------------------
void UIPopupMenuStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::ButtonStyle);
	In.push_back( PropertyName::ButtonStyleLabel);
	In.push_back( PropertyName::IconSubmenu);

	UIWidgetStyle::GetLinkPropertyNames( In );}

//----------------------------------------------------------------------

void UIPopupMenuStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedAppearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIPopupMenuStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::ButtonStyle);
	In.push_back( PropertyName::ButtonStyleLabel);
	In.push_back( PropertyName::ItemHeight );
	In.push_back( PropertyName::BackgroundOpacity );
	In.push_back( PropertyName::IconSubmenu );

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UIPopupMenuStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::ButtonStyle )
	{
		UIButtonStyle * const NewStyle = static_cast<UIButtonStyle *>(GetObjectFromPath(Value, TUIButtonStyle));

		if (NewStyle || Value.empty())
		{
			SetButtonStyle (NewStyle);
			RemoveProperty (Name);
			return true;
		}
	}
	else if( Name == PropertyName::ButtonStyleLabel )
	{
		UIButtonStyle * const NewStyle = static_cast<UIButtonStyle *>(GetObjectFromPath(Value, TUIButtonStyle));

		if (NewStyle || Value.empty())
		{
			SetButtonStyleLabel (NewStyle);
			RemoveProperty      (Name);
			return true;
		}
	}


	else if ( Name == PropertyName::ItemHeight )
	{
		return UIUtils::ParseLong (Value, mItemHeight);
	}
	else if ( Name == PropertyName::BackgroundOpacity )
	{
		return UIUtils::ParseFloat (Value, mBackgroundOpacity);
	}
	else if ( Name == PropertyName::IconSubmenu )
	{
		UIImageStyle * const style = static_cast<UIImageStyle *>(GetObjectFromPath(Value, TUIImageStyle));

		if (style || Value.empty())
		{
			SetIconSubmenu (style);
			RemoveProperty (Name);
			return true;
		}
	}

	return UIWidgetStyle::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIPopupMenuStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{

	if( Name == PropertyName::ButtonStyle )
	{
		if( mButtonStyle )
		{
			GetPathTo( Value, mButtonStyle );
			return true;
		}
	}
	else if( Name == PropertyName::ButtonStyleLabel )
	{
		if( mButtonStyleLabel )
		{
			GetPathTo( Value, mButtonStyleLabel );
			return true;
		}
	}
	else if ( Name == PropertyName::ItemHeight )
	{
		return UIUtils::FormatLong (Value, mItemHeight);
	}
	else if ( Name == PropertyName::BackgroundOpacity )
	{
		return UIUtils::FormatFloat (Value, mBackgroundOpacity);
	}
	else if( Name == PropertyName::IconSubmenu )
	{
		if( mIconSubmenu )
		{
			GetPathTo( Value, mIconSubmenu);
			return true;
		}
	}

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

void UIPopupMenuStyle::SetButtonStyle (UIButtonStyle * style)
{
	AttachMember (mButtonStyle, style);
}

//----------------------------------------------------------------------

void UIPopupMenuStyle::SetButtonStyleLabel (UIButtonStyle * style)
{
	AttachMember (mButtonStyleLabel, style);
}

//-----------------------------------------------------------------

void UIPopupMenuStyle::Link( void )
{
	UIWidgetStyle::Link ();

	if (mButtonStyle)
		mButtonStyle->Link ();
	if (mButtonStyleLabel)
		mButtonStyleLabel->Link ();
}

//----------------------------------------------------------------------

void UIPopupMenuStyle::SetIconSubmenu       (UIImageStyle * icon)
{
	AttachMember (mIconSubmenu, icon);
}

//----------------------------------------------------------------------
