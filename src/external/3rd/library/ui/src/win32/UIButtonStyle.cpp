#include "_precompile.h"

#include "UIButtonStyle.h"
#include "UIImageStyle.h"
#include "UIPalette.h"
#include "UIText.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <cassert>
#include <vector>

//----------------------------------------------------------------------

const char *UIButtonStyle::TypeName = "ButtonStyle";

//----------------------------------------------------------------------

const UILowerString  UIButtonStyle::PropertyName::DisabledIconColor     = UILowerString ("DisabledIconColor");
const UILowerString  UIButtonStyle::PropertyName::DisabledTextColor     = UILowerString ("DisabledTextColor");
const UILowerString  UIButtonStyle::PropertyName::DisabledTextMargin    = UILowerString ("DisabledTextMargin");
const UILowerString  UIButtonStyle::PropertyName::DisabledTextStyle     = UILowerString ("DisabledTextStyle");
const UILowerString  UIButtonStyle::PropertyName::MouseOverIconColor    = UILowerString ("MouseOverIconColor");
const UILowerString  UIButtonStyle::PropertyName::MouseOverSound        = UILowerString ("MouseOverSound");
const UILowerString  UIButtonStyle::PropertyName::MouseOverTextColor    = UILowerString ("MouseOverTextColor");
const UILowerString  UIButtonStyle::PropertyName::MouseOverTextMargin   = UILowerString ("MouseOverTextMargin");
const UILowerString  UIButtonStyle::PropertyName::MouseOverTextStyle    = UILowerString ("MouseOverTextStyle");
const UILowerString  UIButtonStyle::PropertyName::NormalIconColor       = UILowerString ("NormalIconColor");
const UILowerString  UIButtonStyle::PropertyName::NormalTextColor       = UILowerString ("NormalTextColor");
const UILowerString  UIButtonStyle::PropertyName::NormalTextMargin      = UILowerString ("NormalTextMargin");
const UILowerString  UIButtonStyle::PropertyName::NormalTextStyle       = UILowerString ("NormalTextStyle");
const UILowerString  UIButtonStyle::PropertyName::PressSound            = UILowerString ("PressSound");
const UILowerString  UIButtonStyle::PropertyName::PressedIconColor      = UILowerString ("PressedIconColor");
const UILowerString  UIButtonStyle::PropertyName::PressedTextColor      = UILowerString ("PressedTextColor");
const UILowerString  UIButtonStyle::PropertyName::PressedTextMargin     = UILowerString ("PressedTextMargin");
const UILowerString  UIButtonStyle::PropertyName::PressedTextStyle      = UILowerString ("PressedTextStyle");
const UILowerString  UIButtonStyle::PropertyName::ReleaseSound          = UILowerString ("ReleaseSound");
const UILowerString  UIButtonStyle::PropertyName::SelectedIconColor     = UILowerString ("SelectedIconColor");
const UILowerString  UIButtonStyle::PropertyName::SelectedTextColor     = UILowerString ("SelectedTextColor");
const UILowerString  UIButtonStyle::PropertyName::SelectedTextMargin    = UILowerString ("SelectedTextMargin");
const UILowerString  UIButtonStyle::PropertyName::SelectedTextStyle     = UILowerString ("SelectedTextStyle");
const UILowerString  UIButtonStyle::PropertyName::TextAlignment         = UILowerString ("TextAlignment");
const UILowerString  UIButtonStyle::PropertyName::TextAlignmentVertical = UILowerString ("TextAlignmentVertical");
const UILowerString  UIButtonStyle::PropertyName::TextCapital           = UILowerString ("TextCapital");

//======================================================================================
#define _TYPENAME UIButtonStyle

namespace UIButtonStyleNamespace
{
	//================================================================
	// Text category
	_GROUPBEGIN(Text)
		_DESCRIPTOR(NormalTextStyle,"Text Style.",T_object),
		_DESCRIPTOR(NormalTextMargin,"",T_rect),
		_DESCRIPTOR(SelectedTextStyle,"Text Style.",T_object),
		_DESCRIPTOR(SelectedTextMargin,"",T_rect),
		_DESCRIPTOR(DisabledTextStyle,"Text Style.",T_object),
		_DESCRIPTOR(DisabledTextMargin,"",T_rect),
		_DESCRIPTOR(PressedTextStyle,"Text Style.",T_object),
		_DESCRIPTOR(PressedTextMargin,"",T_rect),
		_DESCRIPTOR(MouseOverTextStyle,"Text Style.",T_object),
		_DESCRIPTOR(MouseOverTextMargin,"",T_rect),
		_DESCRIPTOR(TextCapital,"",T_bool),
		_DESCRIPTOR(TextAlignment,"",T_string),
		_DESCRIPTOR(TextAlignmentVertical,"",T_string)
	_GROUPEND(Text, 3, 0);
	//================================================================
	//================================================================
	// Icon category
	_GROUPBEGIN(Icon)
		_DESCRIPTOR(NormalIconColor,"",T_color),
		_DESCRIPTOR(SelectedIconColor,"",T_color),
		_DESCRIPTOR(DisabledIconColor,"",T_color),
		_DESCRIPTOR(PressedIconColor,"",T_color),
		_DESCRIPTOR(MouseOverIconColor,"",T_color)
	_GROUPEND(Icon, 3, 1);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(NormalTextColor,"Text Color.",T_color),
		_DESCRIPTOR(SelectedTextColor,"Text Color.",T_color),
		_DESCRIPTOR(DisabledTextColor,"Text Color.",T_color),
		_DESCRIPTOR(PressedTextColor,"Text Color.",T_color),
		_DESCRIPTOR(MouseOverTextColor,"Text Color.",T_color),
		_DESCRIPTOR(PressSound,"",T_string),
		_DESCRIPTOR(ReleaseSound,"",T_string),
		_DESCRIPTOR(MouseOverSound,"",T_string)
	_GROUPEND(Appearance, 3, 2);
	//================================================================
}
using namespace UIButtonStyleNamespace;

//======================================================================================

UIButtonStyle::UIButtonStyle () :
UIWidgetStyle          (),
mTextAlignment         (UITextStyle::Center),
mTextAlignmentVertical (UIText::TAV_center),
mTextCapital           (false)
{
}

//======================================================================================

UIButtonStyle::~UIButtonStyle ()
{
	for( int i = 0; i < LastButtonState; ++i )
	{
		if( mStateInfo[i].mTextStyle )
			mStateInfo[i].mTextStyle->Detach( this );
	}
}

//======================================================================================

bool UIButtonStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUIButtonStyle) || UIWidgetStyle::IsA( Type );
}

//======================================================================================

const char *UIButtonStyle::GetTypeName () const
{
	return TypeName;
}

//======================================================================================

UIBaseObject *UIButtonStyle::Clone () const
{
	return new UIButtonStyle;
}

//======================================================================================

void UIButtonStyle::SetTextStyle( UIButtonState s, UITextStyle *NewStyle )
{
	AttachMember (mStateInfo[s].mTextStyle, NewStyle);
}

//======================================================================================

UITextStyle *UIButtonStyle::GetTextStyle( UIButtonState s ) const
{
	if( mStateInfo[s].mTextStyle )
		return mStateInfo[s].mTextStyle;

	return mStateInfo[Normal].mTextStyle;
}

//======================================================================================

const UIColor & UIButtonStyle::GetIconColor      (UIButtonState s) const
{
	return mStateInfo[s].mIconColor;
}

//----------------------------------------------------------------------

const UIColor & UIButtonStyle::GetTextStyleColor( UIButtonState s ) const
{
	return mStateInfo[s].mTextColor;
}

//----------------------------------------------------------------------

const UIRect & UIButtonStyle::GetTextMargin     (UIButtonState s) const
{
	return mStateInfo [s].mTextMargin;
}

//======================================================================================

void UIButtonStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::NormalTextStyle		);
	In.push_back( PropertyName::SelectedTextStyle	);
	In.push_back( PropertyName::DisabledTextStyle	);
	In.push_back( PropertyName::PressedTextStyle	);
	In.push_back( PropertyName::MouseOverTextStyle	);

	UIWidgetStyle::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIButtonStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Text, category, o_groups);
	GROUP_TEST_AND_PUSH(Icon, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIButtonStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	UIPalette::GetPropertyNamesForType (TUIButtonStyle, In);

	In.push_back( PropertyName::NormalTextStyle		);
	In.push_back( PropertyName::NormalTextColor		);
	In.push_back( PropertyName::NormalIconColor		);

	In.push_back( PropertyName::SelectedTextStyle	);
	In.push_back( PropertyName::SelectedTextColor	);
	In.push_back( PropertyName::SelectedIconColor		);

	In.push_back( PropertyName::DisabledTextStyle	);
	In.push_back( PropertyName::DisabledTextColor	);
	In.push_back( PropertyName::DisabledIconColor		);

	In.push_back( PropertyName::PressedTextStyle		);
	In.push_back( PropertyName::PressedTextColor		);
	In.push_back( PropertyName::PressedIconColor		);

	In.push_back( PropertyName::MouseOverTextStyle	);
	In.push_back( PropertyName::MouseOverTextColor	);
	In.push_back( PropertyName::MouseOverIconColor		);

	In.push_back( PropertyName::PressSound					);
	In.push_back( PropertyName::ReleaseSound				);
	In.push_back( PropertyName::MouseOverSound			);

	In.push_back( PropertyName::TextCapital );
	In.push_back( PropertyName::TextAlignment     );
	In.push_back( PropertyName::TextAlignmentVertical);

	In.push_back( PropertyName::NormalTextMargin);
	In.push_back( PropertyName::DisabledTextMargin);
	In.push_back( PropertyName::PressedTextMargin);
	In.push_back( PropertyName::MouseOverTextMargin);
	In.push_back( PropertyName::SelectedTextMargin );

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//======================================================================================

bool UIButtonStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	StateElement	Element;
	UIButtonState State;

	if( LookupPropertyStateAndElement( Name.c_str (), State, Element ) )
	{
		switch( Element )
		{
			case TextStyle:
			{
				UITextStyle *NewTextStyle = UITextStyleManager::GetInstance()->GetFontForLogicalFont(Value);

				if( NewTextStyle || Value.empty() )
				{
					SetTextStyle( State, NewTextStyle );
					return true;
				}
				break;
			}

			case TextColor:
				UIUtils::ParseColor( Value, mStateInfo[State].mTextColor );
				return true;

			case IconColor:
				UIUtils::ParseColor( Value, mStateInfo[State].mIconColor );
				return true;

			case TextMargin:
				UIUtils::ParseRect( Value, mStateInfo[State].mTextMargin );
				return true;
		}
	}

	else if( Name == PropertyName::TextAlignment )
	{
		UITextStyle::Alignment a;
		if (UITextStyle::ParseAlignment (Value, a))
		{
			mTextAlignment = static_cast<short>(a);
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::TextCapital)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetTextCapital (b);
			return true;
		}
		return false;
	}

	else if( Name == PropertyName::TextAlignmentVertical )
	{
		UIText::TextAlignmentVertical tav;
		if (UIText::ParseTextAlignmentVertical (Value, tav))
		{
			mTextAlignmentVertical = static_cast<short>(tav);
			return true;
		}
		return false;
	}
	else
	{
		UIPalette::SetPropertyForObject (*this, Name, Value);
	}

	return UIWidgetStyle::SetProperty( Name, Value );
}

//======================================================================================

bool UIButtonStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	UIButtonState State;
	StateElement	Element;

	if( LookupPropertyStateAndElement( Name.c_str (), State, Element ) )
	{
		switch( Element )
		{
			case TextStyle:
				if( mStateInfo[State].mTextStyle )
				{
					Value = mStateInfo[State].mTextStyle->GetLogicalName();
					return true;
				}
				break;

			case TextColor:
				UIUtils::FormatColor( Value, mStateInfo[State].mTextColor );
				return true;

			case IconColor:
				UIUtils::FormatColor( Value, mStateInfo[State].mIconColor );
				return true;

			case TextMargin:
				UIUtils::FormatRect (Value, mStateInfo [State].mTextMargin);
				return true;

			default:
				assert(false);
		}
	}

	else if( Name == PropertyName::TextAlignment )
	{
		UITextStyle::FormatAlignment (Value, static_cast<UITextStyle::Alignment>(mTextAlignment));
		return true;
	}
	else if( Name == PropertyName::TextCapital )
	{
		return UIUtils::FormatBoolean (Value, GetTextCapital ());
	}
	else if( Name == PropertyName::TextAlignmentVertical )
	{
		UIText::FormatTextAlignmentVertical (Value, static_cast<UIText::TextAlignmentVertical>(mTextAlignmentVertical));
		return true;
	}

	return UIWidgetStyle::GetProperty( Name, Value );
}

//======================================================================================

bool UIButtonStyle::LookupPropertyStateAndElement( const char *Name, UIButtonState &State, StateElement &Element ) const
{
	int Offset;

	if( !_strnicmp( Name, UIButtonStyle::PropertyName::NormalTextStyle.c_str (), 6 ) )
	{
		State  = Normal;
		Offset = 6;
	}
	else if( !_strnicmp( Name, UIButtonStyle::PropertyName::SelectedTextStyle.c_str (), 8 ) )
	{
		State  = Selected;
		Offset = 8;
	}
	else if( !_strnicmp( Name, UIButtonStyle::PropertyName::DisabledTextStyle.c_str (), 8 ) )
	{
		State  = Disabled;
		Offset = 8;
	}
	else if( !_strnicmp( Name, UIButtonStyle::PropertyName::MouseOverTextStyle.c_str (), 9 ) )
	{
		State  = MouseOver;
		Offset = 9;
	}
	else if( !_strnicmp( Name, UIButtonStyle::PropertyName::PressedTextStyle.c_str (), 7 ) )
	{
		State  = Pressed;
		Offset = 7;
	}
	else
		return false;


	if( !_stricmp( Name + Offset, UIButtonStyle::PropertyName::NormalTextStyle.c_str () + 6 ) )
		Element = TextStyle;
	else if( !_stricmp( Name + Offset, UIButtonStyle::PropertyName::NormalTextColor.c_str () + 6 ) )
		Element = TextColor;
	else if( !_stricmp( Name + Offset, UIButtonStyle::PropertyName::NormalTextMargin.c_str () + 6 ) )
		Element = TextMargin;
	else if( !_stricmp( Name + Offset, UIButtonStyle::PropertyName::NormalIconColor.c_str () + 6 ) )
		Element = IconColor;
	else
		return false;

	return true;
}

//-----------------------------------------------------------------

void UIButtonStyle::Link ()
{
	UIWidgetStyle::Link ();

	for (int i = 0; i < LastButtonState; ++i)
	{
		StateInfo & sinfo = mStateInfo [i];
		if (sinfo.mTextStyle)
			sinfo.mTextStyle->Link ();
	}
}

//----------------------------------------------------------------------

void UIButtonStyle::SetTextCapital           (bool b)
{
	mTextCapital = b;
}

//-----------------------------------------------------------------
