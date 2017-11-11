#include "_precompile.h"

#include "UISliderplaneStyle.h"
#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

const char *UISliderplaneStyle::TypeName = "SliderplaneStyle";

const UILowerString UISliderplaneStyle::PropertyName::Layout     = UILowerString ("Layout");
const UILowerString UISliderplaneStyle::PropertyName::StartCap   = UILowerString ("StartCap");
const UILowerString UISliderplaneStyle::PropertyName::Background = UILowerString ("Background");
const UILowerString UISliderplaneStyle::PropertyName::EndCap     = UILowerString ("EndCap");
const UILowerString UISliderplaneStyle::PropertyName::Slider     = UILowerString ("Slider");

//======================================================================================
#define _TYPENAME UISliderplaneStyle

namespace UISliderplaneStyleNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Layout, "", T_string), // ENUM
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================

	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(StartCap, "", T_object),
		_DESCRIPTOR(Background, "", T_object),
		_DESCRIPTOR(EndCap, "", T_object),
	_GROUPEND(Appearance, 3, int(UIPropertyCategories::C_Appearance));
	//================================================================

	//================================================================
	// AdvancedAppearance category.
	_GROUPBEGIN(AdvancedAppearance)
		_DESCRIPTOR(Slider, "", T_object),
	_GROUPEND(AdvancedAppearance, 3, int(UIPropertyCategories::C_AdvancedAppearance));
	//================================================================
}
using namespace UISliderplaneStyleNamespace;
//======================================================================================

typedef UISliderplaneStyle::Image Image;

//----------------------------------------------------------------------

UISliderplaneStyle::UISliderplaneStyle()
{
	for( int i= 0; i < LastImage; ++i )
		mImages[i] = 0;

	mLayout = L_horizontal;
}

UISliderplaneStyle::~UISliderplaneStyle()
{
	for( int i = 0; i < LastImage; ++i )
		SetImage( static_cast<Image>( i ), 0 );
}

bool UISliderplaneStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUISliderplaneStyle) || UIWidgetStyle::IsA( Type );
}

const char *UISliderplaneStyle::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UISliderplaneStyle::Clone( void ) const
{
	return new UISliderplaneStyle;
}

void UISliderplaneStyle::SetImage( const Image i, UIImageStyle *NewImage )
{
	if( NewImage )
		NewImage->Attach( this );

	if( mImages[i] )
		mImages[i]->Detach( this );

	mImages[i] = NewImage;
}

UIImageStyle *UISliderplaneStyle::GetImage( const Image i ) const
{
	return mImages[i];
}

UIStyle::Layout UISliderplaneStyle::GetLayout( void ) const
{
	return mLayout;
}

void UISliderplaneStyle::SetLayout( Layout NewLayout )
{
	mLayout = NewLayout;
}

void UISliderplaneStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::StartCap      );
	In.push_back( PropertyName::Background    );
	In.push_back( PropertyName::EndCap        );
	In.push_back( PropertyName::Slider        );
	In.push_back( PropertyName::Layout        );

	UIWidgetStyle::GetLinkPropertyNames( In );}

//----------------------------------------------------------------------

void UISliderplaneStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedAppearance, category, o_groups);
}

//----------------------------------------------------------------------

void UISliderplaneStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::StartCap      );
	In.push_back( PropertyName::Background    );
	In.push_back( PropertyName::EndCap        );
	In.push_back( PropertyName::Slider        );
	In.push_back( PropertyName::Layout        );

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UISliderplaneStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	Image ImageID = LookupImageIDByName( Name );

	if( ImageID != LastImage )
	{
		UIBaseObject *NewImage = GetObjectFromPath( Value, TUIImageStyle );

		if( NewImage || Value.empty() )
		{
			SetImage( ImageID, reinterpret_cast<UIImageStyle *>( NewImage ) );
			return true;
		}
	}
	else if( Name == PropertyName::Layout )
		return UIUtils::ParseLayout( Value, mLayout );

	return UIWidgetStyle::SetProperty( Name, Value );
}

bool UISliderplaneStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
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
	else if( Name == PropertyName::Layout )
		return UIUtils::FormatLayout( Value, mLayout );

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

Image UISliderplaneStyle::LookupImageIDByName( const UILowerString & Name ) const
{
	if( Name == PropertyName::StartCap )
		return StartCap;
	else if( Name == PropertyName::Background )
		return Background;
	else if( Name == PropertyName::EndCap )
		return EndCap;
	else if( Name == PropertyName::Slider )
		return Slider;
	else
		return LastImage;
}

//----------------------------------------------------------------------
