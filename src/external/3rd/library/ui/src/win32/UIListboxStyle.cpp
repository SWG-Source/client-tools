#include "_precompile.h"

#include "UIListboxStyle.h"
#include "UIImageStyle.h"
#include "UIGridStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

const char *UIListboxStyle::TypeName								= "ListboxStyle";

//----------------------------------------------------------------------

const UILowerString UIListboxStyle::PropertyName::GridStyle = UILowerString ("GridStyle");
const UILowerString UIListboxStyle::PropertyName::Layout		= UILowerString ("Layout");

//======================================================================================
#define _TYPENAME UIListboxStyle

namespace UIListboxStyleNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(GridStyle, "", T_object),
		_DESCRIPTOR(Layout, "", T_string), // ENUM
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIListboxStyleNamespace;
//======================================================================================

UIListboxStyle::UIListboxStyle()
{
	mGridStyle = 0;
	mLayout    = UIStyle::L_vertical;
}

UIListboxStyle::~UIListboxStyle()
{
	SetGridStyle( 0 );
}

bool UIListboxStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUIListboxStyle) || UIWidgetStyle::IsA( Type );
}

const char *UIListboxStyle::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UIListboxStyle::Clone( void ) const
{
	return new UIListboxStyle;
}

void UIListboxStyle::SetGridStyle( UIGridStyle *NewGridStyle )
{
	if( NewGridStyle )
		NewGridStyle->Attach( this );

	if( mGridStyle )
		mGridStyle->Detach( this );

	mGridStyle = NewGridStyle;
}

UIGridStyle *UIListboxStyle::GetGridStyle( void ) const
{
	return mGridStyle;
}

void UIListboxStyle::SetLayout( const Layout NewLayout )
{
	mLayout = NewLayout;
}

UIStyle::Layout UIListboxStyle::GetLayout( void ) const
{
	return mLayout;
}

void UIListboxStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::GridStyle );
	In.push_back( PropertyName::Layout );

	UIWidgetStyle::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIListboxStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIListboxStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::GridStyle );
	In.push_back( PropertyName::Layout );

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UIListboxStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::GridStyle )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value.c_str(), TUIGridStyle );

		if( NewStyle || Value.empty() )
		{
			SetGridStyle( reinterpret_cast<UIGridStyle *>( NewStyle ) );
			return true;
		}
	}
	else if( Name == PropertyName::Layout )
	{
		UIUtils::ParseLayout( Value, mLayout );
		return true;
	}

	return UIWidgetStyle::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIListboxStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::GridStyle )
	{
		if( mGridStyle )
		{
			GetPathTo( Value, mGridStyle );
			return true;
		}
	}
	else if( Name == PropertyName::Layout )
		return UIUtils::FormatLayout( Value, mLayout );

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------