#include "_precompile.h"

#include "UIButtonStyle.h"
#include "UITabbedPaneStyle.h"
#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

const char * const  UITabbedPaneStyle::TypeName = "TabbedPaneStyle";
const UILowerString  UITabbedPaneStyle::PropertyName::ButtonStyle        = UILowerString ("ButtonStyle");
const UILowerString  UITabbedPaneStyle::PropertyName::ButtonStyleLeft    = UILowerString ("ButtonStyleLeft");
const UILowerString  UITabbedPaneStyle::PropertyName::ButtonStyleRight   = UILowerString ("ButtonStyleRight");

//======================================================================================
#define _TYPENAME UITabbedPaneStyle

namespace UITabbedPaneStyleNamespace
{
	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(ButtonStyle, "", T_object),
		_DESCRIPTOR(ButtonStyleLeft, "", T_object),
		_DESCRIPTOR(ButtonStyleRight, "", T_object),
	_GROUPEND(Appearance, 3, int(UIPropertyCategories::C_Appearance));
	//================================================================
}
using namespace UITabbedPaneStyleNamespace;
//======================================================================================

UITabbedPaneStyle::UITabbedPaneStyle() :
UIWidgetStyle     ()
{
	mButtonStyle [BST_left]   = 0;
	mButtonStyle [BST_middle] = 0;
	mButtonStyle [BST_right]  = 0;
}

//----------------------------------------------------------------------

UITabbedPaneStyle::~UITabbedPaneStyle()
{
	SetButtonStyle (BST_left,   0);
	SetButtonStyle (BST_middle, 0);
	SetButtonStyle (BST_right,  0);
}

//----------------------------------------------------------------------

bool UITabbedPaneStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUITabbedPaneStyle) || UIWidgetStyle::IsA( Type );
}
//----------------------------------------------------------------------

const char *UITabbedPaneStyle::GetTypeName( void ) const
{
	return TypeName;
}
//----------------------------------------------------------------------

UIBaseObject *UITabbedPaneStyle::Clone( void ) const
{
	return new UITabbedPaneStyle;
}

//----------------------------------------------------------------------

void UITabbedPaneStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::ButtonStyle      );
	In.push_back( PropertyName::ButtonStyleLeft  );
	In.push_back( PropertyName::ButtonStyleRight );
	UIWidgetStyle::GetLinkPropertyNames (In);
}

//----------------------------------------------------------------------

void UITabbedPaneStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UITabbedPaneStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::ButtonStyle      );
	In.push_back( PropertyName::ButtonStyleLeft  );
	In.push_back( PropertyName::ButtonStyleRight );

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}
//----------------------------------------------------------------------

bool UITabbedPaneStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::ButtonStyle )
	{
		UIButtonStyle * const NewStyle = static_cast<UIButtonStyle *>(GetObjectFromPath(Value, TUIButtonStyle));

		if (NewStyle || Value.empty())
		{
			SetButtonStyle (BST_middle, NewStyle);
		}
	}
	else if( Name == PropertyName::ButtonStyleLeft )
	{
		UIButtonStyle * const NewStyle = static_cast<UIButtonStyle *>(GetObjectFromPath(Value, TUIButtonStyle));

		if (NewStyle || Value.empty())
		{
			SetButtonStyle (BST_left, NewStyle);
		}
	}
	else if( Name == PropertyName::ButtonStyleRight )
	{
		UIButtonStyle * const NewStyle = static_cast<UIButtonStyle *>(GetObjectFromPath(Value, TUIButtonStyle));

		if (NewStyle || Value.empty())
		{
			SetButtonStyle (BST_right, NewStyle);
		}
	}
	return UIWidgetStyle::SetProperty( Name, Value );
}
//----------------------------------------------------------------------

bool UITabbedPaneStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::ButtonStyle )
	{
		if( mButtonStyle [BST_middle] )
		{
			GetPathTo( Value, mButtonStyle [BST_middle] );
			return true;
		}
	}
	if( Name == PropertyName::ButtonStyleLeft )
	{
		if( mButtonStyle [BST_left] )
		{
			GetPathTo( Value, mButtonStyle [BST_left] );
			return true;
		}
	}
	if( Name == PropertyName::ButtonStyleRight )
	{
		if( mButtonStyle [BST_right] )
		{
			GetPathTo( Value, mButtonStyle [BST_right] );
			return true;
		}
	}

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

void UITabbedPaneStyle::SetButtonStyle (ButtonStyleType type, UIButtonStyle * style)
{
	if( style )
		style->Attach( this );

	if( mButtonStyle [type] )
		mButtonStyle [type] ->Detach( this );

	mButtonStyle [type] = style;
}

//-----------------------------------------------------------------

void UITabbedPaneStyle::Link ()
{
	UIWidgetStyle::Link ();

	for (int i = 0; i < BST_numTypes; ++i)
	{
		if (mButtonStyle [i])
			mButtonStyle [i]->Link ();
	}
}

//----------------------------------------------------------------------
