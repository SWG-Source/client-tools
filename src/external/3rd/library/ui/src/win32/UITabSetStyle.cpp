#include "_precompile.h"

#include "UIPropertyDescriptor.h"
#include "UITabSetStyle.h"
#include "UIImageStyle.h"

#include <vector>

const char *UITabSetStyle::TypeName = "UITabSetStyle";

const UILowerString UITabSetStyle::PropertyName::ActiveTabStartCap			= UILowerString ("ActiveTabStartCap");
const UILowerString UITabSetStyle::PropertyName::ActiveTabBackground		= UILowerString ("ActiveTabBackground");
const UILowerString UITabSetStyle::PropertyName::ActiveTabEndCap				= UILowerString ("ActiveTabEndCap");
const UILowerString UITabSetStyle::PropertyName::InactiveTabStartCap		= UILowerString ("InactiveTabStartCap");
const UILowerString UITabSetStyle::PropertyName::InactiveTabBackground	= UILowerString ("InactiveTabBackground");
const UILowerString UITabSetStyle::PropertyName::InactiveTabEndCap			= UILowerString ("InactiveTabEndCap");
const UILowerString UITabSetStyle::PropertyName::BodyTopLeft						= UILowerString ("BodyTopLeft");
const UILowerString UITabSetStyle::PropertyName::BodyTop								= UILowerString ("BodyTop");
const UILowerString UITabSetStyle::PropertyName::BodyTopRight						= UILowerString ("BodyTopRight");
const UILowerString UITabSetStyle::PropertyName::BodyRight							= UILowerString ("BodyRight");
const UILowerString UITabSetStyle::PropertyName::BodyBottomRight				= UILowerString ("BodyBottomRight");
const UILowerString UITabSetStyle::PropertyName::BodyBottom							= UILowerString ("BodyBottom");
const UILowerString UITabSetStyle::PropertyName::BodyBottomLeft					= UILowerString ("BodyBottomLeft");
const UILowerString UITabSetStyle::PropertyName::BodyLeft								= UILowerString ("BodyLeft");
const UILowerString UITabSetStyle::PropertyName::BodyFill								= UILowerString ("BodyFill");
const UILowerString UITabSetStyle::PropertyName::Layout									= UILowerString ("Layout");

//======================================================================================
#define _TYPENAME UITabSetStyle

namespace UITabSetStyleNamespace
{
	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(ActiveTabStartCap, "", T_object),
		_DESCRIPTOR(ActiveTabBackground, "", T_object),
		_DESCRIPTOR(ActiveTabEndCap, "", T_object),
		_DESCRIPTOR(InactiveTabStartCap, "", T_object),
		_DESCRIPTOR(InactiveTabBackground, "", T_object),
		_DESCRIPTOR(InactiveTabEndCap, "", T_object),
		_DESCRIPTOR(BodyTopLeft, "", T_object),
		_DESCRIPTOR(BodyTop, "", T_object),
		_DESCRIPTOR(BodyTopRight, "", T_object),
		_DESCRIPTOR(BodyRight, "", T_object),
		_DESCRIPTOR(BodyBottomRight, "", T_object),
		_DESCRIPTOR(BodyBottom, "", T_object),
		_DESCRIPTOR(BodyBottomLeft, "", T_object),
		_DESCRIPTOR(BodyLeft, "", T_object),
		_DESCRIPTOR(BodyFill, "", T_object),
	_GROUPEND(Appearance, 3, int(UIPropertyCategories::C_Appearance));
	//================================================================
}
using namespace UITabSetStyleNamespace;
//======================================================================================

UITabSetStyle::UITabSetStyle( void )
{
	for( int i = 0; i < LastImage; ++i )
		mImages[i] = 0;
}

UITabSetStyle::~UITabSetStyle( void )
{
	for( int i = 0; i < LastImage; ++i )
	{
		if( mImages[i] )
			mImages[i]->Detach( this );
	}
}

bool UITabSetStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUITabSetStyle) || UIWidgetStyle::IsA( Type );
}

const char *UITabSetStyle::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UITabSetStyle::Clone( void ) const
{
	return new UITabSetStyle;
}


void UITabSetStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	UIWidgetStyle::GetLinkPropertyNames (In);
}

//----------------------------------------------------------------------

void UITabSetStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UITabSetStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::ActiveTabStartCap			);
	In.push_back( PropertyName::ActiveTabBackground		);
	In.push_back( PropertyName::ActiveTabEndCap				);

	In.push_back( PropertyName::InactiveTabStartCap		);
	In.push_back( PropertyName::InactiveTabBackground	);
	In.push_back( PropertyName::InactiveTabEndCap			);

	In.push_back( PropertyName::BodyTopLeft						);
	In.push_back( PropertyName::BodyTop								);
	In.push_back( PropertyName::BodyTopRight						);
	In.push_back( PropertyName::BodyRight							);
	In.push_back( PropertyName::BodyBottomRight				);
	In.push_back( PropertyName::BodyBottom							);
	In.push_back( PropertyName::BodyBottomLeft					);
	In.push_back( PropertyName::BodyLeft								);
	In.push_back( PropertyName::BodyFill								);

	In.push_back( PropertyName::Layout									);

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UITabSetStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	Image ImageID;

	if( LookupImageByName( Name, ImageID ) )
	{
		UIImageStyle *NewImage = reinterpret_cast<UIImageStyle *>( GetObjectFromPath( Value, TUIImageStyle ) );

		if( NewImage || Value.empty() )
			SetImage( ImageID, NewImage );

		return true;
	}
	else if( Name == PropertyName::Layout )
	{
		return true;
	}

	return UIWidgetStyle::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UITabSetStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	Image ImageID;

	if( LookupImageByName( Name, ImageID ) )
	{
		if( mImages[ImageID] )
		{
			GetPathTo( Value, mImages[ImageID] );
			return true;
		}
	}
	else if( Name == PropertyName::Layout )
	{
		return true;
	}

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

void UITabSetStyle::SetImage( const Image ImageID, UIImageStyle *NewImageStyle )
{
	if (NewImageStyle == mImages [ImageID])
		return;

	if( NewImageStyle )
		NewImageStyle->Detach( this );

	if( mImages[ImageID] )
		mImages[ImageID]->Detach( this );

	mImages[ImageID] = NewImageStyle;
}

UIImageStyle *UITabSetStyle::GetImage( const Image ImageID ) const
{
	return mImages[ImageID];
}

bool UITabSetStyle::LookupImageByName( const UILowerString & Name, Image &theImage ) const
{
	if( Name == PropertyName::ActiveTabStartCap )
		theImage = ActiveTabStartCap;
	else if( Name == PropertyName::ActiveTabBackground )
		theImage = ActiveTabBackground;
	else if( Name == PropertyName::ActiveTabEndCap )
		theImage = ActiveTabEndCap;
	else if( Name == PropertyName::InactiveTabStartCap )
		theImage = InactiveTabStartCap;
	else if( Name == PropertyName::InactiveTabBackground )
		theImage = InactiveTabBackground;
	else if( Name == PropertyName::InactiveTabEndCap )
		theImage = InactiveTabEndCap;
	else if( Name == PropertyName::BodyTopLeft )
		theImage = BodyTopLeft;
	else if( Name == PropertyName::BodyTop )
		theImage = BodyTop;
	else if( Name == PropertyName::BodyTopRight )
		theImage = BodyTopRight;
	else if( Name == PropertyName::BodyRight )
		theImage = BodyRight;
	else if( Name == PropertyName::BodyBottomRight )
		theImage = BodyBottomRight;
	else if( Name == PropertyName::BodyBottom )
		theImage = BodyBottom;
	else if( Name == PropertyName::BodyBottomLeft )
		theImage = BodyBottomLeft;
	else if( Name == PropertyName::BodyLeft )
		theImage = BodyLeft;
	else if( Name == PropertyName::BodyFill )
		theImage = BodyFill;
	else
		return false;

	return true;
}