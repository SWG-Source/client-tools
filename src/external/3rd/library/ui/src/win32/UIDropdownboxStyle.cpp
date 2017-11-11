#include "_precompile.h"

#include "UIDropdownboxStyle.h"
#include "UIImageStyle.h"
#include "UIListboxStyle.h"
//#include "UIScrollbarStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

const char *UIDropdownboxStyle::TypeName = "DropdownboxStyle";

//const UILowerString UIDropdownboxStyle::PropertyName::ScrollbarStyle			 = "Scrollbar.Style";
const UILowerString UIDropdownboxStyle::PropertyName::CloseSound					 = UILowerString ("CloseSound");
const UILowerString UIDropdownboxStyle::PropertyName::DisabledBackground   = UILowerString ("Disabled.Background");
const UILowerString UIDropdownboxStyle::PropertyName::DisabledDownArrow       = UILowerString ("Disabled.DownArrow");
const UILowerString UIDropdownboxStyle::PropertyName::DisabledStartCap     = UILowerString ("Disabled.StartCap");
const UILowerString UIDropdownboxStyle::PropertyName::DropBy               = UILowerString ("DropBy");
const UILowerString UIDropdownboxStyle::PropertyName::ListboxStyle				 = UILowerString ("Listbox.Style");
const UILowerString UIDropdownboxStyle::PropertyName::NormalBackground     = UILowerString ("Normal.Background");
const UILowerString UIDropdownboxStyle::PropertyName::NormalDownArrow         = UILowerString ("Normal.DownArrow");
const UILowerString UIDropdownboxStyle::PropertyName::NormalStartCap       = UILowerString ("Normal.StartCap");
const UILowerString UIDropdownboxStyle::PropertyName::OpenSound						 = UILowerString ("OpenSound");
const UILowerString UIDropdownboxStyle::PropertyName::Padding              = UILowerString ("Padding");
const UILowerString UIDropdownboxStyle::PropertyName::PressedBackground    = UILowerString ("Pressed.Background");
const UILowerString UIDropdownboxStyle::PropertyName::PressedDownArrow        = UILowerString ("Pressed.DownArrow");
const UILowerString UIDropdownboxStyle::PropertyName::PressedStartCap      = UILowerString ("Pressed.StartCap");
const UILowerString UIDropdownboxStyle::PropertyName::SelectedBackground   = UILowerString ("Selected.Background");
const UILowerString UIDropdownboxStyle::PropertyName::SelectedDownArrow       = UILowerString ("Selected.DownArrow");
const UILowerString UIDropdownboxStyle::PropertyName::SelectedStartCap     = UILowerString ("Selected.StartCap");

typedef UIDropdownboxStyle::Image Image;
typedef UIDropdownboxStyle::PropertyName PropertyName;

//======================================================================================
#define _TYPENAME UIDropdownboxStyle

namespace UIDropdownboxStyleNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(NormalStartCap, "", T_object),
		_DESCRIPTOR(NormalBackground, "", T_object),
		_DESCRIPTOR(NormalDownArrow, "", T_object),
		_DESCRIPTOR(DisabledStartCap, "", T_object),
		_DESCRIPTOR(DisabledBackground, "", T_object),
		_DESCRIPTOR(DisabledDownArrow, "", T_object),
		_DESCRIPTOR(SelectedStartCap, "", T_object),
		_DESCRIPTOR(SelectedBackground, "", T_object),
		_DESCRIPTOR(SelectedDownArrow, "", T_object),
		_DESCRIPTOR(PressedStartCap, "", T_object),
		_DESCRIPTOR(PressedBackground, "", T_object),
		_DESCRIPTOR(PressedDownArrow, "", T_object),
		_DESCRIPTOR(DropBy, "", T_int),
		_DESCRIPTOR(Padding, "", T_rect),
		_DESCRIPTOR(ListboxStyle, "", T_object),
	_GROUPEND(Basic, 3, 0);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(OpenSound, "", T_string),
		_DESCRIPTOR(CloseSound, "", T_string),
	_GROUPEND(Appearance, 1, 1);
	//================================================================
}
using namespace UIDropdownboxStyleNamespace;
//======================================================================================

UIDropdownboxStyle::UIDropdownboxStyle()
{
	for( int i= 0; i < LastImage; ++i )
		mImages[i] = 0;

	//mScrollbarStyle = 0;
	mListboxStyle   = 0;

	mDropBy = 100;

	mPadding.left   = 0;
	mPadding.top    = 0;
	mPadding.right  = 0;
	mPadding.bottom = 0;
}

UIDropdownboxStyle::~UIDropdownboxStyle()
{
	for( int i = 0; i < LastImage; ++i )
	{
		if( mImages[i] )
			mImages[i]->Detach( this );
	}

//	SetScrollbarStyle(0);
	SetListboxStyle(0);
}

const char *UIDropdownboxStyle::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UIDropdownboxStyle::Clone( void ) const
{
	return new UIDropdownboxStyle;
}

bool UIDropdownboxStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUIDropdownboxStyle) || UIWidgetStyle::IsA( Type );
}

void UIDropdownboxStyle::SetImage( const Image i, UIImageStyle *NewImage )
{
	if( NewImage )
		NewImage->Attach( this );

	if( mImages[i] )
		mImages[i]->Detach( this );

	mImages[i] = NewImage;
}

UIImageStyle *UIDropdownboxStyle::GetImage( const Image i ) const
{
	static const Image remap[LastImage] =
	{
		NormalStartCap,
		NormalBackground,
		NormalDownArrow,

		NormalStartCap,
		NormalBackground,
		NormalDownArrow,

		NormalStartCap,
		NormalBackground,
		NormalDownArrow,

		NormalStartCap,
		NormalBackground,
		NormalDownArrow,
	};

	if( mImages[i] )
		return mImages[i];
	else
		return mImages[remap[i]];
}
/*
void UIDropdownboxStyle::SetScrollbarStyle( UIScrollbarStyle *NewScrollbarStyle )
{
	if( NewScrollbarStyle )
		NewScrollbarStyle->Attach( this );

	if( mScrollbarStyle )
		mScrollbarStyle->Detach( this );

	mScrollbarStyle = NewScrollbarStyle;
}

UIScrollbarStyle *UIDropdownboxStyle::GetScrollbarStyle( void ) const
{
	return mScrollbarStyle;
}
*/

void UIDropdownboxStyle::SetListboxStyle( UIListboxStyle *NewListboxStyle )
{
	if( NewListboxStyle )
		NewListboxStyle->Attach( this );

	if( mListboxStyle )
		mListboxStyle->Detach( this );

	mListboxStyle = NewListboxStyle;
}

UIListboxStyle *UIDropdownboxStyle::GetListboxStyle( void ) const
{
	return mListboxStyle;
}

void UIDropdownboxStyle::SetDropBy( const long In )
{
	mDropBy = In;
}

long UIDropdownboxStyle::GetDropBy( void ) const
{
	return mDropBy;
}

void UIDropdownboxStyle::SetPadding( const UIRect &In )
{
	mPadding = In;
}

void UIDropdownboxStyle::GetPadding( UIRect &Out ) const
{
	Out = mPadding;
}

//----------------------------------------------------------------------

void UIDropdownboxStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	GetPropertyNames (In, false);
}

//----------------------------------------------------------------------

void UIDropdownboxStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIDropdownboxStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::NormalStartCap    	);
	In.push_back( PropertyName::NormalBackground  	);
	In.push_back( PropertyName::NormalDownArrow    );

	In.push_back( PropertyName::DisabledStartCap  	);
	In.push_back( PropertyName::DisabledBackground	);
	In.push_back( PropertyName::DisabledDownArrow  );

	In.push_back( PropertyName::SelectedStartCap  	);
	In.push_back( PropertyName::SelectedBackground	);
	In.push_back( PropertyName::SelectedDownArrow  );

	In.push_back( PropertyName::PressedStartCap   	);
	In.push_back( PropertyName::PressedBackground 	);
	In.push_back( PropertyName::PressedDownArrow   );

	In.push_back( PropertyName::DropBy            	);
	In.push_back( PropertyName::Padding            );

//	In.push_back( PropertyName::ScrollbarStyle			);
	In.push_back( PropertyName::ListboxStyle				);

	In.push_back( PropertyName::OpenSound					);
	In.push_back( PropertyName::CloseSound					);

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

bool UIDropdownboxStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	Image ImageID;

	if( LookupPropertyImage( Name, ImageID ) )
	{
		UIBaseObject *NewImage = GetObjectFromPath( Value.c_str(), TUIImageStyle );

		if( NewImage || Value.empty() )
		{
			SetImage( ImageID, reinterpret_cast<UIImageStyle *>( NewImage ) );
			return true;
		}
	}
	else if( Name == PropertyName::DropBy )
		return UIUtils::ParseLong( Value, mDropBy );
	else if( Name == PropertyName::Padding )
		return UIUtils::ParseRect( Value, mPadding );
	/*
	else if( !_stricmp( Name, PropertyName::ScrollbarStyle ) )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value.c_str(), TUIScrollbarStyle );

		if( NewStyle || Value.empty() )
		{
			SetScrollbarStyle( reinterpret_cast<UIScrollbarStyle *>( NewStyle ) );
			return true;
		}
	}
	*/
	else if( Name == PropertyName::ListboxStyle )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value.c_str(), TUIListboxStyle );

		if( NewStyle || Value.empty() )
		{
			SetListboxStyle( reinterpret_cast<UIListboxStyle *>( NewStyle ) );
			return true;
		}
	}

	return UIWidgetStyle::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIDropdownboxStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	Image ImageID;

	if( LookupPropertyImage( Name, ImageID ) )
	{
		if( mImages[ImageID] )
		{
			GetPathTo( Value, mImages[ImageID] );
			return true;
		}
	}
	else if( Name ==  PropertyName::DropBy )
		return UIUtils::FormatLong( Value, mDropBy );
	else if( Name == PropertyName::Padding )
		return UIUtils::FormatRect( Value, mPadding );
	/*
	else if( !_stricmp( Name, PropertyName::ScrollbarStyle ) )
	{
		if( mScrollbarStyle )
		{
			GetPathTo( Value, mScrollbarStyle );
			return true;
		}
	}
	*/
	else if( Name == PropertyName::ListboxStyle )
	{
		if( mListboxStyle )
		{
			GetPathTo( Value, mListboxStyle );
			return true;
		}
	}

	return UIWidgetStyle::GetProperty( Name, Value );
}

bool UIDropdownboxStyle::LookupPropertyImage( const UILowerString & Name, Image &ImageID ) const
{
	if( Name == PropertyName::NormalStartCap )
		ImageID = NormalStartCap;
	else if( Name == PropertyName::NormalBackground )
		ImageID = NormalBackground;
	else if( Name == PropertyName::NormalDownArrow )
		ImageID = NormalDownArrow;
	else if( Name == PropertyName::DisabledStartCap )
		ImageID = DisabledStartCap;
	else if( Name == PropertyName::DisabledBackground )
		ImageID = DisabledBackground;
	else if( Name == PropertyName::DisabledDownArrow )
		ImageID = DisabledDownArrow;
	else if( Name == PropertyName::SelectedStartCap )
		ImageID = SelectedStartCap;
	else if( Name == PropertyName::SelectedBackground )
		ImageID = SelectedBackground;
	else if( Name == PropertyName::SelectedDownArrow )
		ImageID = SelectedDownArrow;
	else if( Name == PropertyName::PressedStartCap )
		ImageID = PressedStartCap;
	else if( Name == PropertyName::PressedBackground )
		ImageID = PressedBackground;
	else if( Name == PropertyName::PressedDownArrow )
		ImageID = PressedDownArrow;
	else
		return false;

	return true;
}

