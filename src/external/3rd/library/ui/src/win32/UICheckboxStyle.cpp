#include "_precompile.h"

#include "UICheckboxStyle.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

const char *UICheckboxStyle::TypeName = "CheckboxStyle";

//----------------------------------------------------------------------

const UILowerString UICheckboxStyle::PropertyName::NormalTextStyle					= UILowerString ("NormalTextStyle");
const UILowerString UICheckboxStyle::PropertyName::NormalUncheckedImage			= UILowerString ("NormalUncheckedImage");
const UILowerString UICheckboxStyle::PropertyName::NormalCheckedImage				= UILowerString ("NormalCheckedImage");
const UILowerString UICheckboxStyle::PropertyName::SelectedUncheckedImage		= UILowerString ("SelectedUncheckedImage");
const UILowerString UICheckboxStyle::PropertyName::SelectedCheckedImage			= UILowerString ("SelectedCheckedImage");
const UILowerString UICheckboxStyle::PropertyName::DisabledTextStyle				= UILowerString ("DisabledTextStyle");
const UILowerString UICheckboxStyle::PropertyName::DisabledUncheckedImage		= UILowerString ("DisabledUncheckedImage");
const UILowerString UICheckboxStyle::PropertyName::DisabledCheckedImage			= UILowerString ("DisabledCheckedImage");
const UILowerString UICheckboxStyle::PropertyName::TextOffset								= UILowerString ("TextOffset");
const UILowerString UICheckboxStyle::PropertyName::SetSound									= UILowerString ("SetSound");
const UILowerString UICheckboxStyle::PropertyName::UnsetSound								= UILowerString ("UnsetSound");

//----------------------------------------------------------------------
#define _TYPENAME UICheckboxStyle

namespace UICheckboxStyleNamespace
{
	//================================================================
	// Text category
	_GROUPBEGIN(Text)
	  _DESCRIPTOR(NormalTextStyle, "", T_object),
	  _DESCRIPTOR(DisabledTextStyle, "", T_object),
	  _DESCRIPTOR(TextOffset, "", T_point)
	_GROUPEND(Text, 3, 1);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
	  _DESCRIPTOR(NormalUncheckedImage, "", T_object),
	  _DESCRIPTOR(NormalCheckedImage, "", T_object),
	  _DESCRIPTOR(SelectedUncheckedImage, "", T_object),
	  _DESCRIPTOR(SelectedCheckedImage, "", T_object),
	  _DESCRIPTOR(DisabledUncheckedImage, "", T_object),
	  _DESCRIPTOR(DisabledCheckedImage, "", T_object),
	  _DESCRIPTOR(SetSound, "", T_string),
	  _DESCRIPTOR(UnsetSound, "", T_string)
	_GROUPEND(Appearance, 3, 2);
	//================================================================

}
using namespace UICheckboxStyleNamespace;

//----------------------------------------------------------------------

typedef UICheckboxStyle::PropertyName PropertyName;
typedef UICheckboxStyle::UICheckboxState UICheckboxState;

UICheckboxStyle::UICheckboxStyle( void )
{
	mNormalTextStyle   = 0;
	mDisabledTextStyle = 0;

	mTextOffset.x      = 0;
	mTextOffset.y      = 0;

	for( int i=0; i < LastCheckboxState; ++i )
		mImages[i] = 0;
}

UICheckboxStyle::~UICheckboxStyle( void )
{
	SetTextStyle( true, 0 );
	SetTextStyle( false, 0 );

	for( int i = 0; i < LastCheckboxState; ++i )
		SetImageStyle( static_cast<UICheckboxState>(i), 0 );
}

bool UICheckboxStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUICheckboxStyle) || UIWidgetStyle::IsA( Type );
}

const char *UICheckboxStyle::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UICheckboxStyle::Clone( void ) const
{
	return new UICheckboxStyle;
}

void UICheckboxStyle::SetTextStyle( bool Enabled, UITextStyle *NewStyle )
{
	UITextStyle **p;

	if( Enabled )
		p = &mNormalTextStyle;
	else
		p = &mDisabledTextStyle;

	if( NewStyle )
		NewStyle->Attach( this );

	if( (*p) )
		(*p)->Detach( this );

	*p = NewStyle;
}

UITextStyle *UICheckboxStyle::GetTextStyle( bool Enabled ) const
{
	if( Enabled )
		return mNormalTextStyle;
	else
		return mDisabledTextStyle ? mDisabledTextStyle : mNormalTextStyle;
}

void UICheckboxStyle::SetImageStyle( UICheckboxState s, UIImageStyle *NewImage )
{
	if( NewImage )
		NewImage->Attach( this );

	if( mImages[s] )
		mImages[s]->Detach( this );

	mImages[s] = NewImage;
}

UIImageStyle *UICheckboxStyle::GetImageStyle( UICheckboxState s ) const
{
	static UICheckboxState Remap[] =
	{
		Unchecked, Checked,
		Unchecked, Checked,
		Unchecked, Checked,
	};

	if( mImages[s] )
		return mImages[s];

	return mImages[Remap[s]];
}

void UICheckboxStyle::SetTextOffset( const UIPoint &In )
{
	mTextOffset = In;
}

void UICheckboxStyle::GetTextOffset( UIPoint &Out ) const
{
	Out = mTextOffset;
}
void UICheckboxStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::NormalTextStyle				);
	In.push_back( PropertyName::NormalUncheckedImage		);
	In.push_back( PropertyName::NormalCheckedImage			);

	In.push_back( PropertyName::SelectedUncheckedImage );
	In.push_back( PropertyName::SelectedCheckedImage		);

	In.push_back( PropertyName::DisabledTextStyle			);
	In.push_back( PropertyName::DisabledUncheckedImage );
	In.push_back( PropertyName::DisabledCheckedImage   );

	UIWidgetStyle::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UICheckboxStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Text, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UICheckboxStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::NormalTextStyle				);
	In.push_back( PropertyName::NormalUncheckedImage		);
	In.push_back( PropertyName::NormalCheckedImage			);

	In.push_back( PropertyName::SelectedUncheckedImage );
	In.push_back( PropertyName::SelectedCheckedImage		);

	In.push_back( PropertyName::DisabledTextStyle			);
	In.push_back( PropertyName::DisabledUncheckedImage );
	In.push_back( PropertyName::DisabledCheckedImage   );

	In.push_back( PropertyName::TextOffset							);

	In.push_back( PropertyName::SetSound								);
	In.push_back( PropertyName::UnsetSound							);

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UICheckboxStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	UICheckboxState State (Unchecked);

	if( LookupPropertyImageState( Name, State ) )
	{
		UIBaseObject *NewImage = GetObjectFromPath( Value, TUIImageStyle );

		if( NewImage || Value.empty() )
		{
			SetImageStyle( State, reinterpret_cast<UIImageStyle *>( NewImage ) );
			return true;
		}
	}
	else if( Name == PropertyName::NormalTextStyle )
	{
		UIBaseObject *NewStyle = UITextStyleManager::GetInstance()->GetFontForLogicalFont(Value);

		if( NewStyle || Value.empty() )
		{
			SetTextStyle( true, reinterpret_cast<UITextStyle *>( NewStyle ) );
			return true;
		}
	}
	else if( Name == PropertyName::DisabledTextStyle )
	{
		UITextStyle *NewStyle = UITextStyleManager::GetInstance()->GetFontForLogicalFont(Value);

		if( NewStyle || Value.empty() )
		{
			SetTextStyle( false, reinterpret_cast<UITextStyle *>( NewStyle ) );
			return true;
		}
	}
	else if( Name == PropertyName::TextOffset )
		return UIUtils::ParsePoint( Value, mTextOffset );

	return UIWidgetStyle::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UICheckboxStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	UICheckboxState State ( Unchecked );

	if( LookupPropertyImageState( Name, State ) )
	{
		if( mImages[State] )
		{
			GetPathTo( Value, mImages[State] );
			return true;
		}
	}
	else if( Name == PropertyName::NormalTextStyle )
	{
		if( mNormalTextStyle )
		{
			Value = mNormalTextStyle->GetLogicalName();
			return true;
		}
	}
	else if( Name == PropertyName::DisabledTextStyle )
	{
		if( mDisabledTextStyle )
		{
			Value = mDisabledTextStyle->GetLogicalName();
			return true;
		}
	}
	else if( Name == PropertyName::TextOffset )
		return UIUtils::FormatPoint( Value, mTextOffset );

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UICheckboxStyle::LookupPropertyImageState( const UILowerString & Name, UICheckboxState &State ) const
{
	if( Name == PropertyName::NormalUncheckedImage )
		State = Unchecked;
	else if( Name == PropertyName::NormalCheckedImage )
		State = Checked;
	else if( Name == PropertyName::SelectedUncheckedImage )
		State = SelectedUnchecked;
	else if( Name == PropertyName::SelectedCheckedImage )
		State = SelectedChecked;
	else if( Name == PropertyName::DisabledUncheckedImage )
		State = DisabledUnchecked;
	else if( Name == PropertyName::DisabledCheckedImage )
		State = DisabledChecked;
	else
		return false;

	return true;
}
