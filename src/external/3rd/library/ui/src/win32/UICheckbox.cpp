#include "_precompile.h"

#include "UICanvas.h"
#include "UICheckbox.h"
#include "UICheckboxStyle.h"
#include "UIEventCallback.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPalette.h"
#include "UITextStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

const char *UICheckbox::TypeName                   = "Checkbox";

//----------------------------------------------------------------------

const UILowerString UICheckbox::PropertyName::Alignment    = UILowerString ("Alignment");
const UILowerString UICheckbox::PropertyName::Checked      = UILowerString ("Checked");
const UILowerString UICheckbox::PropertyName::LocalText    = UILowerString ("LocalText");
const UILowerString UICheckbox::PropertyName::MaxLines     = UILowerString ("MaxLines");
const UILowerString UICheckbox::PropertyName::OnSet        = UILowerString ("OnSet");
const UILowerString UICheckbox::PropertyName::OnUnset      = UILowerString ("OnUnset");
const UILowerString UICheckbox::PropertyName::Radio        = UILowerString ("Radio");
const UILowerString UICheckbox::PropertyName::SetSound     = UILowerString ("SetSound");
const UILowerString UICheckbox::PropertyName::Style        = UILowerString ("Style");
const UILowerString UICheckbox::PropertyName::Text         = UILowerString ("Text");
const UILowerString UICheckbox::PropertyName::TextColor    = UILowerString ("TextColor");
const UILowerString UICheckbox::PropertyName::UnsetSound   = UILowerString ("UnsetSound");

//----------------------------------------------------------------------

const Unicode::String UICheckbox::AlignmentNames::Left  = Unicode::narrowToWide ("Left");
const Unicode::String UICheckbox::AlignmentNames::Right = Unicode::narrowToWide ("Right");

//----------------------------------------------------------------------
#define _TYPENAME UICheckbox

namespace UICheckboxNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Style, "", T_object),
		_DESCRIPTOR(Checked, "", T_bool),
		_DESCRIPTOR(Radio, "", T_bool),
		_DESCRIPTOR(MaxLines, "", T_int)
	_GROUPEND(Basic, 2, 0);
	//================================================================

	//================================================================
	// Text category
	_GROUPBEGIN(Text)
	  _DESCRIPTOR(Text, "", T_string)
	_GROUPEND(Text, 2, 1);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
	  _DESCRIPTOR(Alignment, "", T_string),
	  _DESCRIPTOR(TextColor, "", T_color),
	  _DESCRIPTOR(SetSound, "", T_string),
	  _DESCRIPTOR(UnsetSound, "", T_string)
	_GROUPEND(Appearance, 2, 2);
	//================================================================
	//================================================================
	// Advanced Behavior category
	_GROUPBEGIN(AdvancedBehavior)
		_DESCRIPTOR(OnSet, "", T_string),
		_DESCRIPTOR(OnUnset, "", T_string)
	_GROUPEND(AdvancedBehavior, 2, 3);
	//================================================================
}
using namespace UICheckboxNamespace;

//----------------------------------------------------------------------

UICheckbox::UICheckbox() :
UIWidget   (),
mStyle     (0),
mChecked   (false),
mText      (),
mLocalText (),
mTextColor (UIColor::white),
mRadio     (false),
mMaxLines  (1),
mAlignment (A_left)
{
}

//----------------------------------------------------------------------

UICheckbox::~UICheckbox( void )
{
	SetStyle(0);
}

//----------------------------------------------------------------------

bool UICheckbox::IsA( const UITypeID QueriedType ) const
{
	return (QueriedType == TUICheckbox) || UIWidget::IsA( QueriedType );
}

//----------------------------------------------------------------------

const char *UICheckbox::GetTypeName( void ) const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UICheckbox::Clone( void ) const
{
	return new UICheckbox;
}

//----------------------------------------------------------------------

void UICheckbox::SetText( const UIString &NewText )
{
	mText = NewText;
	UIManager::gUIManager ().CreateLocalizedString (mText, mLocalText);
}

//----------------------------------------------------------------------

void UICheckbox::GetText( UIString &Out ) const
{
	Out = mText;
}

//----------------------------------------------------------------------

void  UICheckbox::SetLocalText( const UIString & str)
{
	mLocalText = str;
}

//----------------------------------------------------------------------

void  UICheckbox::GetLocalText( UIString & out) const
{
	out = mLocalText;
}

//----------------------------------------------------------------------

void UICheckbox::SetStyle( UICheckboxStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach( this );

	if( mStyle )
		mStyle->Detach( this );

	mStyle = NewStyle;
}

//----------------------------------------------------------------------

UIStyle *UICheckbox::GetStyle( void ) const
{
	return mStyle;
};

//----------------------------------------------------------------------

void UICheckbox::SetChecked( const bool NewChecked, const bool sendCallback )
{
	if( mChecked != NewChecked )
		ToggleChecked( false, sendCallback );
}

//----------------------------------------------------------------------

void UICheckbox::ToggleChecked( bool doPlaySound, const bool sendCallback )
{
	if( mChecked )
	{
		if( doPlaySound )
		{
			UINarrowString SoundToPlay;

			if( !GetPropertyNarrow( UICheckbox::PropertyName::UnsetSound, SoundToPlay ) && mStyle )
				mStyle->GetPropertyNarrow( UICheckboxStyle::PropertyName::UnsetSound, SoundToPlay );

			if( !SoundToPlay.empty() )
				UIManager::gUIManager().PlaySound( SoundToPlay.c_str () );
		}

		mChecked = false;

		if ( sendCallback )
		{
			SendCallback( &UIEventCallback::OnCheckboxUnset,           PropertyName::OnUnset );
			SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null );
		}
	}
	else
	{
		if( doPlaySound )
		{
			UINarrowString SoundToPlay;

			if( !GetPropertyNarrow( UICheckbox::PropertyName::SetSound, SoundToPlay ) && mStyle )
				mStyle->GetPropertyNarrow( UICheckboxStyle::PropertyName::SetSound, SoundToPlay );

			if( !SoundToPlay.empty() )
				UIManager::gUIManager().PlaySound( SoundToPlay.c_str () );
		}

		mChecked = true;

		if ( sendCallback )
		{
			SendCallback( &UIEventCallback::OnCheckboxSet,             PropertyName::OnSet );
			SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null );
		}
	}

	if ( sendCallback )
	{
		Unicode::String bstr;
		UIUtils::FormatBoolean (bstr, mChecked);
		SendDataChangedCallback (PropertyName::Checked, bstr);
	}
}

//----------------------------------------------------------------------

bool UICheckbox::ProcessMessage( const UIMessage &msg )
{
	if( UIWidget::ProcessMessage( msg ) )
		return true;

	switch( msg.Type )
	{
		case UIMessage::KeyDown:
			if( msg.Keystroke != UIMessage::Space && (!mRadio || !mChecked))
				ToggleChecked( true );

			break;

		case UIMessage::LeftMouseDown:
		case UIMessage::RightMouseDown:
		case UIMessage::MiddleMouseDown:
			if (!mRadio || !mChecked)
				ToggleChecked( true );
			break;
	}
	return true;
}

//----------------------------------------------------------------------

void UICheckbox::Render( UICanvas &DestinationCanvas ) const
{
	UIWidget::Render (DestinationCanvas);

	UIImageStyle *SourceImage;

	if( !mStyle )
		return;

	if( !IsEnabled() )
		SourceImage = mStyle->GetImageStyle( mChecked ? UICheckboxStyle::DisabledChecked : UICheckboxStyle::DisabledUnchecked );
	else if( IsSelected() )
		SourceImage = mStyle->GetImageStyle( mChecked ? UICheckboxStyle::SelectedChecked : UICheckboxStyle::SelectedUnchecked );
	else
		SourceImage = mStyle->GetImageStyle( mChecked ? UICheckboxStyle::Checked : UICheckboxStyle::Unchecked );

	UIPoint imagePoint;

	if( SourceImage )
	{
		const long sourceImageWidth = abs (SourceImage->GetWidth ());
		if (mAlignment == A_right)
			imagePoint.x = GetWidth () - sourceImageWidth;

		SourceImage->Render( GetAnimationState(), DestinationCanvas, imagePoint);
	}

	if( !mLocalText.empty() )
	{
		UITextStyle *TextStyle = mStyle->GetTextStyle (IsEnabled());

		if (TextStyle)
		{
			const UIColor oldColor (DestinationCanvas.GetColor ());
			DestinationCanvas.ModifyColor (mTextColor);

			UIPoint TextOffset;
			mStyle->GetTextOffset (TextOffset);
	
			const long width = GetWidth () - TextOffset.x;


			UITextStyle::Alignment textAlignment = UITextStyle::Left;
			if (mAlignment == A_right)
			{
				TextOffset.x = 0;
				textAlignment = UITextStyle::Right;
			}

			TextStyle->RenderText (textAlignment, mLocalText, DestinationCanvas, TextOffset, &width, mMaxLines, true);

			DestinationCanvas.SetColor (oldColor);
		}
	}
}

//----------------------------------------------------------------------

bool UICheckbox::CanSelect( void ) const
{
	return false;
}
void UICheckbox::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::Style			);
	In.push_back( PropertyName::Text				);

	UIWidget::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UICheckbox::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Text, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedBehavior, category, o_groups);
}

//----------------------------------------------------------------------

void UICheckbox::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	UIPalette::GetPropertyNamesForType (TUICheckbox, In);

	In.push_back (PropertyName::Alignment);
	In.push_back (PropertyName::MaxLines);
	In.push_back( PropertyName::Checked		);
	In.push_back( PropertyName::OnSet			);
	In.push_back( PropertyName::OnUnset		);
	In.push_back( PropertyName::Radio );
	In.push_back( PropertyName::SetSound		);
	In.push_back( PropertyName::Style			);
	In.push_back( PropertyName::Text				);
	In.push_back( PropertyName::TextColor );
	In.push_back( PropertyName::UnsetSound);

	UIWidget::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UICheckbox::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == UICheckbox::PropertyName::Checked )
	{
		UIUtils::ParseBoolean( Value, mChecked );
		return true;
	}
	else if( Name == PropertyName::Style )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value, TUICheckboxStyle );

		if( NewStyle || Value.empty() )
		{
			SetStyle( reinterpret_cast<UICheckboxStyle *>(NewStyle) );
			return true;
		}
	}
	else if( Name == PropertyName::Text )
	{
		SetText( Value);
		return true;
	}
	else if( Name == PropertyName::LocalText )
	{
		SetLocalText( Value);
		return true;
	}
	else if( Name == PropertyName::TextColor )
	{
		return UIUtils::ParseColor (Value, mTextColor);
	}
	else if( Name == PropertyName::Radio )
	{
		return UIUtils::ParseBoolean (Value, mRadio);
	}
	else if (Name == PropertyName::MaxLines)
	{
		return UIUtils::ParseInteger (Value, mMaxLines);
	}
	else if (Name == PropertyName::Alignment)
	{
		if (Unicode::caseInsensitiveCompare (Value, AlignmentNames::Right))
			mAlignment = A_right;
		else if (Unicode::caseInsensitiveCompare (Value, AlignmentNames::Left))
			mAlignment = A_left;
		else
			return false;

		return true;
	}
	else
		UIPalette::SetPropertyForObject (*this, Name, Value);

	return UIWidget::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UICheckbox::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == UICheckbox::PropertyName::Checked )
		return UIUtils::FormatBoolean( Value, mChecked );
	else if( Name == PropertyName::Style )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}
	else if( Name == PropertyName::Text )
	{
		GetText (Value);
		return true;
	}
	else if( Name == PropertyName::LocalText )
	{
		GetLocalText (Value);
		return true;
	}
	else if( Name == PropertyName::TextColor )
	{
		return UIUtils::FormatColor (Value, mTextColor);
	}
	else if( Name == PropertyName::Radio )
	{
		return UIUtils::FormatBoolean (Value, mRadio);
	}
	else if (Name == PropertyName::MaxLines)
	{
		return UIUtils::FormatInteger (Value, mMaxLines);
	}
	else if (Name == PropertyName::Alignment)
	{
		if (mAlignment == A_right)
			Value = AlignmentNames::Right;
		else
			Value = AlignmentNames::Left;

		return true;
	}

	return UIWidget::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

void UICheckbox::SetTextColor (const UIColor & color)
{
	mTextColor = color;
}

//----------------------------------------------------------------------

void UICheckbox::SetRadio (bool b)
{
	mRadio = b;
}

//----------------------------------------------------------------------
