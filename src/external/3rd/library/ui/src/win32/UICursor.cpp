#include "_precompile.h"

#include "UICursor.h"
#include "UICanvas.h"
#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

const char *UICursor::TypeName = "Cursor";
const UILowerString UICursor::PropertyName::ImageStyle = UILowerString ("ImageStyle");
const UILowerString UICursor::PropertyName::HotSpot    = UILowerString ("HotSpot");

//======================================================================================
#define _TYPENAME UICursor

namespace UICursorNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(ImageStyle, "", T_object),
		_DESCRIPTOR(HotSpot, "", T_point)
	_GROUPEND(Basic, 1, 0);
	//================================================================
}
using namespace UICursorNamespace;

//======================================================================================

UICursor::UICursor() : mHotSpot(0,0)
{
	mImage = 0;
}

UICursor::~UICursor()
{
	SetImageStyle( 0 );
}

bool UICursor::IsA( const UITypeID Type ) const
{
	return (Type == TUICursor) || UIBaseObject::IsA( Type );
}

const char *UICursor::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UICursor::Clone( void ) const
{
	return new UICursor;
}

void UICursor::Render( UICanvas &DestinationCanvas, const UIPoint &loc ) const
{
	if( !mImage )
		return;

	mImage->Render( 0, DestinationCanvas, loc );
}

void UICursor::SetImageStyle( UIImageStyle *NewImage )
{
	if( NewImage )
		NewImage->Attach( this );

	if( mImage )
		mImage->Detach( this );

	mImage = NewImage;
}

UISize UICursor::GetSize( void ) const
{
	if( mImage )
		return mImage->GetSize();
	else
		return UISize(0,0);
}

long UICursor::GetWidth( void ) const
{
	if( mImage )
		return mImage->GetWidth();
	else
		return 0;
}

long UICursor::GetHeight( void ) const
{
	if( mImage )
		return mImage->GetHeight();
	else
		return 0;
}

void UICursor::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::ImageStyle );

	UIBaseObject::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UICursor::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIBaseObject::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UICursor::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::ImageStyle );
	In.push_back( PropertyName::HotSpot );

	UIBaseObject::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UICursor::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::ImageStyle )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value, TUIImageStyle );

		if( NewStyle || Value.empty() )
		{
			SetImageStyle( reinterpret_cast<UIImageStyle *>( NewStyle ) );
			return true;
		}
	}
	else if( Name == PropertyName::HotSpot )
		return UIUtils::ParsePoint( Value, mHotSpot );

	return UIBaseObject::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UICursor::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::ImageStyle )
	{
		if( mImage )
		{
			GetPathTo( Value, mImage );
			return true;
		}
	}
	else if( Name == PropertyName::HotSpot )
		return UIUtils::FormatPoint( Value, mHotSpot );
	return UIBaseObject::GetProperty( Name, Value );
}

//----------------------------------------------------------------------
