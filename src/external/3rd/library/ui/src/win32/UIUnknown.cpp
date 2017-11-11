#include "_precompile.h"

#include "UIPropertyDescriptor.h"
#include "UIUnknown.h"
#include "UICanvas.h"

#include <vector>

//-----------------------------------------------------------------

const UILowerString UIUnknown::PropertyName::TypeName = UILowerString ("Type");

//======================================================================================
#define _TYPENAME UIUnknown

namespace UIUnknownNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(TypeName, "", T_string),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIUnknownNamespace;
//======================================================================================

UIUnknown::UIUnknown( void )
{
}

//-----------------------------------------------------------------

UIUnknown::~UIUnknown( void )
{
}

//-----------------------------------------------------------------

bool UIUnknown::IsA( const UITypeID Type ) const
{ 
	return (Type == TUIUnknown) || UIWidget::IsA( Type );
}

//----------------------------------------------------------------------

void UIUnknown::Render (UICanvas & c) const
{
	UIWidget::Render (c);
}

//-----------------------------------------------------------------

const char *UIUnknown::GetTypeName( void ) const
{
	if( mTypeName.empty() )
		return "Unknown";
	else
		return mTypeName.c_str();
}

//-----------------------------------------------------------------

void UIUnknown::SetTypeName( const UINarrowString & NewName )
{
	mTypeName = NewName;
}

//-----------------------------------------------------------------

UIBaseObject *UIUnknown::Clone( void ) const
{
	return new UIUnknown;
}

//-----------------------------------------------------------------

void UIUnknown::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIUnknown::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::TypeName );

	UIWidget::GetPropertyNames( In, forCopy );
}

//-----------------------------------------------------------------

bool UIUnknown::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::TypeName )
	{
		SetTypeName( UIUnicode::wideToNarrow (Value) );
		return true;
	}

	return UIWidget::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIUnknown::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if(Name == PropertyName::TypeName )
	{
		Value = UIUnicode::narrowToWide (mTypeName);
		return true;
	}

	return UIWidget::GetProperty( Name, Value );
}

//-----------------------------------------------------------------
