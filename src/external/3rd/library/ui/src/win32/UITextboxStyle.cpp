#include "_precompile.h"
#include "UITextboxStyle.h"

#include "UICursor.h"
#include "UIGridStyle.h"
#include "UIManager.h"
#include "UIPalette.h"
#include "UIPropertyDescriptor.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UIUtils.h"
#include <vector>

//----------------------------------------------------------------------

const char * const UITextboxStyle::TypeName = "TextboxStyle";

const UILowerString UITextboxStyle::PropertyName::CaratColor          = UILowerString ("CaratColor");
const UILowerString UITextboxStyle::PropertyName::CaratWidth          = UILowerString ("CaratWidth");
const UILowerString UITextboxStyle::PropertyName::Cursor              = UILowerString ("Cursor");
const UILowerString UITextboxStyle::PropertyName::DisabledGridStyle   = UILowerString ("DisabledGridStyle");
const UILowerString UITextboxStyle::PropertyName::DisabledTextStyle   = UILowerString ("DisabledTextStyle");
const UILowerString UITextboxStyle::PropertyName::NormalGridStyle     = UILowerString ("NormalGridStyle");
const UILowerString UITextboxStyle::PropertyName::NormalTextStyle     = UILowerString ("NormalTextStyle");
const UILowerString UITextboxStyle::PropertyName::SelectedGridStyle   = UILowerString ("SelectedGridStyle");
const UILowerString UITextboxStyle::PropertyName::SelectedTextStyle   = UILowerString ("SelectedTextStyle");
const UILowerString UITextboxStyle::PropertyName::SelectionColor      = UILowerString ("SelectionColor");
const UILowerString UITextboxStyle::PropertyName::SelectionOpacity    = UILowerString ("SelectionOpacity");
const UILowerString UITextboxStyle::PropertyName::TextColor           = UILowerString ("TextColor");
const UILowerString UITextboxStyle::PropertyName::SoundKeyclick       = UILowerString ("SoundKeyclick");
const UILowerString UITextboxStyle::PropertyName::Padding             = UILowerString ("TextPadding");

//======================================================================================
#define _TYPENAME UITextboxStyle

namespace UITextboxStyleNamespace
{
	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(CaratColor, "", T_color),
		_DESCRIPTOR(CaratWidth, "", T_int),
		_DESCRIPTOR(Cursor, "", T_object),
		_DESCRIPTOR(DisabledGridStyle, "", T_object),
		_DESCRIPTOR(DisabledTextStyle, "", T_object),
		_DESCRIPTOR(NormalGridStyle, "", T_object),
		_DESCRIPTOR(NormalTextStyle, "", T_object),
		_DESCRIPTOR(Padding, "", T_rect),
		_DESCRIPTOR(SelectedGridStyle, "", T_object),
		_DESCRIPTOR(SelectedTextStyle, "", T_object),
		_DESCRIPTOR(SelectionColor, "", T_color),
		_DESCRIPTOR(SelectionOpacity, "", T_float),
		_DESCRIPTOR(TextColor, "", T_color),
		_DESCRIPTOR(SoundKeyclick, "", T_string),
	_GROUPEND(Appearance, 3, int(UIPropertyCategories::C_Appearance));
	//================================================================
}
using namespace UITextboxStyleNamespace;
//======================================================================================

UITextboxStyle::UITextboxStyle() :
mCaratColor         ( 0, 0, 0 ),
mCaratWidth         (1L),
mSelectionColor     ( 0xFF, 0xFF, 0xFF ),
mSelectionOpacity   (0.5f),
mTextPadding        ( 1, 1, 1, 1 ),
mCursor             (0),
mTextColor          (0x00, 0x00, 0x00),
mSoundKeyclick      ()
{

	for( int i = 0; i < UIWidget::LastState; ++i )
	{
		mGridStyles[i] = 0;
		mTextStyles[i] = 0;
	}
}

//----------------------------------------------------------------------

UITextboxStyle::~UITextboxStyle()
{
	for( int i = 0; i < UIWidget::LastState; ++i )
	{
		SetTextStyle( (VisualState)i, 0 );
		SetGridStyle( (VisualState)i, 0 );
	}

	SetMouseCursor(0);
}

//----------------------------------------------------------------------

bool UITextboxStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUITextboxStyle) || UIWidgetStyle::IsA( Type );
}

//----------------------------------------------------------------------

const char *UITextboxStyle::GetTypeName( void ) const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UITextboxStyle::Clone( void ) const
{
	return new UITextboxStyle;
}

//----------------------------------------------------------------------

void UITextboxStyle::SetTextStyle( VisualState State, UITextStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach( this );

	if( mTextStyles[State] )
		mTextStyles[State]->Detach( this );

	mTextStyles[State] = NewStyle;
}

//----------------------------------------------------------------------

UITextStyle *UITextboxStyle::GetTextStyle( VisualState State ) const
{
	if( mTextStyles[State] )
		return mTextStyles[State];
	else
		return mTextStyles[UIWidget::Normal];
}

//----------------------------------------------------------------------

void UITextboxStyle::SetGridStyle( VisualState State, UIGridStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach( this );

	if( mGridStyles[State] )
		mGridStyles[State]->Detach( this );

	mGridStyles[State] = NewStyle;
}

//----------------------------------------------------------------------

UIGridStyle *UITextboxStyle::GetGridStyle( VisualState State ) const
{
	if( mGridStyles[State] )
		return mGridStyles[State];
	else
		return mGridStyles[UIWidget::Normal];
}

//----------------------------------------------------------------------

void UITextboxStyle::SetCaratColor( const UIColor &In )
{
	mCaratColor = In;
}

//----------------------------------------------------------------------

UIColor UITextboxStyle::GetCaratColor( void ) const
{
	return mCaratColor;
}

//----------------------------------------------------------------------

void UITextboxStyle::SetCaratWidth( const long In )
{
	mCaratWidth = In;
}

//----------------------------------------------------------------------

long UITextboxStyle::GetCaratWidth( void ) const
{
	return mCaratWidth;
}

//----------------------------------------------------------------------

void UITextboxStyle::SetMouseCursor( UICursor *NewCursor )
{
	if( NewCursor )
		NewCursor->Attach( this );

	if( mCursor )
		mCursor->Detach( this );

	mCursor = NewCursor;
}

//----------------------------------------------------------------------

void UITextboxStyle::SetTextPadding( const UIRect &In )
{
	mTextPadding = In;
}

//----------------------------------------------------------------------

void UITextboxStyle::GetTextPadding( UIRect &Out ) const
{
	Out = mTextPadding;
}

//----------------------------------------------------------------------
void UITextboxStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::Cursor							);
	In.push_back( PropertyName::DisabledGridStyle	);
	In.push_back( PropertyName::DisabledTextStyle	);
	In.push_back( PropertyName::NormalGridStyle		);
	In.push_back( PropertyName::NormalTextStyle		);
	In.push_back( PropertyName::SelectedGridStyle	);
	In.push_back( PropertyName::SelectedTextStyle	);

	UIWidgetStyle::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UITextboxStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UITextboxStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	UIPalette::GetPropertyNamesForType (TUITextboxStyle, In);

	In.push_back( PropertyName::CaratColor					);
	In.push_back( PropertyName::CaratWidth					);
	In.push_back( PropertyName::Cursor							);
	In.push_back( PropertyName::DisabledGridStyle	);
	In.push_back( PropertyName::DisabledTextStyle	);
	In.push_back( PropertyName::NormalGridStyle		);
	In.push_back( PropertyName::NormalTextStyle		);
	In.push_back( PropertyName::Padding						);
	In.push_back( PropertyName::SelectedGridStyle	);
	In.push_back( PropertyName::SelectedTextStyle	);
	In.push_back( PropertyName::SelectionColor			);
	In.push_back( PropertyName::SelectionOpacity		);
	In.push_back( PropertyName::TextColor							);
	In.push_back( PropertyName::SoundKeyclick);

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UITextboxStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	VisualState State = LookupTextStyleStateByName( Name );

	if( State != UIWidget::LastState )
	{
		UITextStyle * NewTextStyle = UITextStyleManager::GetInstance()->GetFontForLogicalFont(Value);
		
		if( NewTextStyle || Value.empty() )
		{
			SetTextStyle( State, NewTextStyle );
			return true;
		}
		// Fall through
	}

	State = LookupGridStyleStateByName( Name );

	if( State != UIWidget::LastState )
	{
		UIGridStyle *NewGridStyle = reinterpret_cast<UIGridStyle *>( GetObjectFromPath( Value, TUIGridStyle ) );

		if( NewGridStyle || Value.empty() )
		{
			SetGridStyle( State, NewGridStyle );
			return true;
		}
		// Fall through
	}

	if( Name == PropertyName::CaratColor )
		return UIUtils::ParseColor( Value, mCaratColor );
	else if( Name == PropertyName::CaratWidth )
		return UIUtils::ParseLong( Value, mCaratWidth );
	else if( Name == PropertyName::Cursor )
	{
		UIBaseObject *NewCursor = GetObjectFromPath( Value, TUICursor );

		if( NewCursor || Value.empty() )
		{
			SetMouseCursor( reinterpret_cast<UICursor *>( NewCursor ) );
			return true;
		}
		// Fall through
	}
	else if( Name == PropertyName::SelectionColor )
		return UIUtils::ParseColor( Value, mSelectionColor );
	else if( Name == PropertyName::SelectionOpacity )
		return UIUtils::ParseFloat( Value, mSelectionOpacity );
	else if( Name == PropertyName::Padding )
		return UIUtils::ParseRect( Value, mTextPadding );
	else if( Name == PropertyName::TextColor )
		return UIUtils::ParseColor( Value, mTextColor);
	else if( Name == PropertyName::SoundKeyclick )
	{
		mSoundKeyclick = Unicode::wideToNarrow (Value);
		return true;
	}
	else
	{
		UIPalette::SetPropertyForObject (*this, Name, Value);
	}

	return UIWidgetStyle::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UITextboxStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	VisualState State = LookupTextStyleStateByName( Name );

	if( State != UIWidget::LastState )
	{
		if( mTextStyles[State] )
		{
			Value = mTextStyles[State]->GetLogicalName();
			return true;
		}
		// Fall through
	}

	State = LookupGridStyleStateByName( Name );

	if( State != UIWidget::LastState )
	{
		if( mGridStyles[State] )
		{
			GetPathTo( Value, mGridStyles[State] );
			return true;
		}
		// Fall through
	}

	if( Name == PropertyName::CaratColor )
		return UIUtils::FormatColor( Value, mCaratColor );
	else if( Name == PropertyName::CaratWidth )
		return UIUtils::FormatLong( Value, mCaratWidth );
	else if(  Name == PropertyName::Cursor )
	{
		if( mCursor )
		{
			GetPathTo( Value, mCursor );
			return true;
		}
		else
			return UIBaseObject::GetProperty( Name, Value );
	}
	else if( Name == PropertyName::SelectionColor )
		return UIUtils::FormatColor( Value, mSelectionColor );
	else if( Name == PropertyName::SelectionOpacity )
		return UIUtils::FormatFloat( Value, mSelectionOpacity );
	else if( Name == PropertyName::Padding )
		return UIUtils::FormatRect( Value, mTextPadding );
	else if( Name == PropertyName::TextColor )
		return UIUtils::FormatColor( Value, mTextColor);
	else if( Name == PropertyName::SoundKeyclick )
	{
		Value = Unicode::narrowToWide (mSoundKeyclick);
		return true;
	}

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

UITextboxStyle::VisualState UITextboxStyle::LookupGridStyleStateByName( const UILowerString & Name ) const
{
	if( Name == PropertyName::NormalGridStyle)
		return UIWidget::Normal;
	else if( Name == PropertyName::SelectedGridStyle)
		return UIWidget::Selected;
	else if( Name == PropertyName::DisabledGridStyle)
		return UIWidget::Disabled;
	else
		return UIWidget::LastState;
}

//----------------------------------------------------------------------

UITextboxStyle::VisualState UITextboxStyle::LookupTextStyleStateByName( const UILowerString & Name ) const
{
	if( Name == PropertyName::NormalTextStyle)
		return UIWidget::Normal;
	else if( Name == PropertyName::SelectedTextStyle)
		return UIWidget::Selected;
	else if( Name == PropertyName::DisabledTextStyle)
		return UIWidget::Disabled;
	else
		return UIWidget::LastState;
}

//----------------------------------------------------------------------

void UITextboxStyle::PlaySoundKeyclick () const
{
	if (!mSoundKeyclick.empty () && UIManager::isUIReady())
		UIManager::gUIManager ().PlaySound (mSoundKeyclick.c_str ());
}

//----------------------------------------------------------------------
