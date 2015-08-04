#include "_precompile.h"

#include "UIProgressbarStyle.h"
#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

const char *UIProgressbarStyle::TypeName = "ProgressbarStyle";

//----------------------------------------------------------------------

const UILowerString UIProgressbarStyle::PropertyName::Background    = UILowerString ("Background");
const UILowerString UIProgressbarStyle::PropertyName::BarBackground = UILowerString ("Bar.Background");
const UILowerString UIProgressbarStyle::PropertyName::BarEndCap			= UILowerString ("Bar.EndCap");
const UILowerString UIProgressbarStyle::PropertyName::BarStartCap   = UILowerString ("Bar.StartCap");
const UILowerString UIProgressbarStyle::PropertyName::EndCap        = UILowerString ("EndCap");
const UILowerString UIProgressbarStyle::PropertyName::Inverted			= UILowerString ("Inverted");
const UILowerString UIProgressbarStyle::PropertyName::Layout  			= UILowerString ("Layout");
const UILowerString UIProgressbarStyle::PropertyName::StartCap      = UILowerString ("StartCap");
const UILowerString UIProgressbarStyle::PropertyName::StepSize      = UILowerString ("StepSize");

//======================================================================================
#define _TYPENAME UIProgressbarStyle

namespace UIProgressbarStyleNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(StepSize, "", T_int),
		_DESCRIPTOR(Inverted, "", T_bool),
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================
	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(StartCap, "", T_object),
		_DESCRIPTOR(Background, "", T_object),
		_DESCRIPTOR(EndCap, "", T_object),
		_DESCRIPTOR(BarStartCap, "", T_object),
		_DESCRIPTOR(BarBackground, "", T_object),
		_DESCRIPTOR(BarEndCap, "", T_object),
		_DESCRIPTOR(Layout, "", T_string), // ENUM
	_GROUPEND(Appearance, 3, int(UIPropertyCategories::C_Appearance));
	//================================================================


}
using namespace UIProgressbarStyleNamespace;
//======================================================================================

typedef UIProgressbarStyle::Image Image;

UIProgressbarStyle::UIProgressbarStyle()
{
	for( int i= 0; i < LastImage; ++i )
		mImages[i] = 0;

	mStepSize = 1;
	mLayout   = L_horizontal;
	mInverted = false;
}

UIProgressbarStyle::~UIProgressbarStyle()
{
	for( int i = 0; i < LastImage; ++i )
		SetImage( static_cast<Image>( i ), 0 );
}

bool UIProgressbarStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUIProgressbarStyle) || UIWidgetStyle::IsA( Type );
}

const char *UIProgressbarStyle::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UIProgressbarStyle::Clone( void ) const
{
	return new UIProgressbarStyle;
}

void UIProgressbarStyle::SetImage( const Image i, UIImageStyle *NewImage )
{
	if( NewImage )
		NewImage->Attach( this );

	if( mImages[i] )
		mImages[i]->Detach( this );

	mImages[i] = NewImage;
}

UIImageStyle *UIProgressbarStyle::GetImage( const Image i ) const
{
	return mImages[i];
}

void UIProgressbarStyle::SetStepSize( const long In )
{
	mStepSize = In;
}

long UIProgressbarStyle::GetStepSize( void ) const
{
	return mStepSize;
}

void UIProgressbarStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::StartCap      );
	In.push_back( PropertyName::Background    );
	In.push_back( PropertyName::EndCap        );
	In.push_back( PropertyName::BarStartCap   );
	In.push_back( PropertyName::BarBackground );
	In.push_back( PropertyName::BarEndCap		 );

	UIWidgetStyle::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIProgressbarStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIProgressbarStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::StepSize      );
	In.push_back( PropertyName::StartCap      );
	In.push_back( PropertyName::Background    );
	In.push_back( PropertyName::EndCap        );
	In.push_back( PropertyName::BarStartCap   );
	In.push_back( PropertyName::BarBackground );
	In.push_back( PropertyName::BarEndCap		 );
	In.push_back( PropertyName::Layout			   );
	In.push_back( PropertyName::Inverted		   );

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UIProgressbarStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	Image ImageID = LookupImageIDByName( Name );

	if( ImageID != LastImage )
	{
		UIBaseObject *NewImage = GetObjectFromPath( Value.c_str(), TUIImageStyle );

		if( NewImage || Value.empty() )
		{
			SetImage( ImageID, reinterpret_cast<UIImageStyle *>( NewImage ) );
			return true;
		}
	}
	else if( Name == PropertyName::StepSize )
		return UIUtils::ParseLong( Value, mStepSize );
	else if( Name == PropertyName::Layout )
		return UIUtils::ParseLayout( Value, mLayout );
	else if( Name == PropertyName::Inverted )
		return UIUtils::ParseBoolean( Value, mInverted );

	return UIWidgetStyle::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIProgressbarStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	Image ImageID = LookupImageIDByName( Name );

	if( ImageID != LastImage )
	{
		if( mImages[ImageID] )
		{
			GetPathTo( Value, mImages[ImageID] );
			return true;
		}
	}
	else if( Name == PropertyName::StepSize )
		return UIUtils::FormatLong( Value, mStepSize );
	else if( Name == PropertyName::Layout )
		return UIUtils::FormatLayout( Value, mLayout );
	else if( Name == PropertyName::Inverted )
		return UIUtils::FormatBoolean( Value, mInverted );

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

UIProgressbarStyle::Image UIProgressbarStyle::LookupImageIDByName( const UILowerString & Name ) const
{
	if( Name == PropertyName::StartCap )
		return StartCap;
	else if( Name == PropertyName::Background )
		return Background;
	else if( Name == PropertyName::EndCap )
		return EndCap;
	else if( Name == PropertyName::BarStartCap )
		return BarStartCap;
	else if( Name == PropertyName::BarBackground )
		return BarBackground;
	else if( Name == PropertyName::BarEndCap )
		return BarEndCap;
	else
		return LastImage;
}

//----------------------------------------------------------------------
