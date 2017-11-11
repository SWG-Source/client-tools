#include "_precompile.h"

#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIRectangleStyle.h"
#include "UISliderbarStyle.h"
#include "UIUtils.h"
#include "UIWidgetRectangleStyles.h"
#include "UIButtonStyle.h"

#include <vector>

//----------------------------------------------------------------------

const char * const UISliderbarStyle::TypeName                                    = "SliderbarStyle";

const UILowerString  UISliderbarStyle::PropertyName::Layout                      = UILowerString ("Layout");
const UILowerString  UISliderbarStyle::PropertyName::ButtonStyleStart            = UILowerString ("ButtonStyleStart");
const UILowerString  UISliderbarStyle::PropertyName::ButtonStyleEnd              = UILowerString ("ButtonStyleEnd");
const UILowerString  UISliderbarStyle::PropertyName::ButtonStyleThumb            = UILowerString ("ButtonStyleThumb");
const UILowerString  UISliderbarStyle::PropertyName::ButtonStyleTrack            = UILowerString ("ButtonStyleTrack");
const UILowerString  UISliderbarStyle::PropertyName::ButtonStyleTrackDisabled    = UILowerString ("ButtonStyleDisabledTrack");
const UILowerString  UISliderbarStyle::PropertyName::ButtonStyleOverlay          = UILowerString ("ButtonStyleOverlay");
const UILowerString  UISliderbarStyle::PropertyName::ButtonLengthStart           = UILowerString ("ButtonLengthStart");
const UILowerString  UISliderbarStyle::PropertyName::ButtonLengthEnd             = UILowerString ("ButtonLengthEnd");
const UILowerString  UISliderbarStyle::PropertyName::ButtonLengthThumb           = UILowerString ("ButtonLengthThumb");
const UILowerString  UISliderbarStyle::PropertyName::ButtonMarginStart           = UILowerString ("ButtonMarginStart");
const UILowerString  UISliderbarStyle::PropertyName::ButtonMarginEnd             = UILowerString ("ButtonMarginEnd");
const UILowerString  UISliderbarStyle::PropertyName::ButtonMarginThumb           = UILowerString ("ButtonMarginThumb");
const UILowerString  UISliderbarStyle::PropertyName::ButtonMarginTrack           = UILowerString ("ButtonMarginTrack");
const UILowerString  UISliderbarStyle::PropertyName::ButtonMarginTrackDisabled   = UILowerString ("ButtonMarginDisabledTrack");
const UILowerString  UISliderbarStyle::PropertyName::ButtonMarginOverlay         = UILowerString ("ButtonMarginOverlay");

//======================================================================================
#define _TYPENAME UISliderbarStyle

namespace UISliderbarStyleNamespace
{
	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(ButtonLengthStart, "", T_int),
		_DESCRIPTOR(ButtonLengthEnd, "", T_int),
		_DESCRIPTOR(ButtonLengthThumb, "", T_int),
		_DESCRIPTOR(ButtonMarginStart, "", T_rect),
		_DESCRIPTOR(ButtonMarginEnd, "", T_rect),
		_DESCRIPTOR(ButtonMarginThumb, "", T_rect),
		_DESCRIPTOR(ButtonMarginTrack, "", T_rect),
		_DESCRIPTOR(ButtonMarginTrackDisabled, "", T_rect),
		_DESCRIPTOR(ButtonMarginOverlay, "", T_rect),
	_GROUPEND(Appearance, 3, int(UIPropertyCategories::C_Appearance));
	//================================================================

	//================================================================
	// AdvancedAppearance category.
	_GROUPBEGIN(AdvancedAppearance)
		_DESCRIPTOR(ButtonStyleStart, "", T_object),
		_DESCRIPTOR(ButtonStyleEnd, "", T_object),
		_DESCRIPTOR(ButtonStyleThumb, "", T_object),
		_DESCRIPTOR(ButtonStyleTrack, "", T_object),
		_DESCRIPTOR(ButtonStyleTrackDisabled, "", T_object),
		_DESCRIPTOR(ButtonStyleOverlay, "", T_object),
	_GROUPEND(AdvancedAppearance, 3, int(UIPropertyCategories::C_AdvancedAppearance));
	//================================================================
}
using namespace UISliderbarStyleNamespace;
//======================================================================================

UISliderbarStyle::UISliderbarStyle () :
UIWidgetStyle          (),
mLayout                (L_horizontal)
{
	{
		for (int i = 0; i < BT_numButtonTypes; ++i)
		{
			mButtonStyles [i] = 0;
			mButtonLengths [i] = 0;
		}
	}
}

//----------------------------------------------------------------------

UISliderbarStyle::~UISliderbarStyle()
{
	{
		for (int i = 0; i < BT_numButtonTypes; ++i)
		{
			SetButtonStyle (static_cast<ButtonType>(i), 0);
		}
	}
}

//----------------------------------------------------------------------

bool UISliderbarStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUISliderbarStyle) || UIWidgetStyle::IsA( Type );
}

//----------------------------------------------------------------------

const char *UISliderbarStyle::GetTypeName() const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UISliderbarStyle::Clone() const
{
	return new UISliderbarStyle;
}

//----------------------------------------------------------------------

void UISliderbarStyle::SetButtonStyle   (const ButtonType type, UIButtonStyle * style)
{
	if (AttachMember (mButtonStyles [type], style))
		SendNotification (UINotification::ObjectChanged, this);
}

//----------------------------------------------------------------------

void UISliderbarStyle::SetButtonMargin   (const ButtonType type, const UIRect & rect)
{
	if (mButtonMargins [type] != rect)
	{
		mButtonMargins [type] = rect;
		SendNotification (UINotification::ObjectChanged, this);
	}
}

//----------------------------------------------------------------------

void UISliderbarStyle::SetButtonLength  (const ButtonType type, const long length)
{
	if (mButtonLengths [type] != length)
	{
		mButtonLengths [type] = length;
		SendNotification (UINotification::ObjectChanged, this);
	}
}

//----------------------------------------------------------------------

UIStyle::Layout UISliderbarStyle::GetLayout() const
{
	return mLayout;
}

//----------------------------------------------------------------------

void UISliderbarStyle::SetLayout( Layout NewLayout )
{
	if (NewLayout != mLayout)
	{
		mLayout = NewLayout;
		SendNotification (UINotification::ObjectChanged, this);
	}
}

//----------------------------------------------------------------------

void UISliderbarStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::ButtonStyleStart );
	In.push_back( PropertyName::ButtonStyleEnd  );
	In.push_back( PropertyName::ButtonStyleThumb);
	In.push_back( PropertyName::ButtonStyleTrack);
	In.push_back( PropertyName::ButtonStyleTrackDisabled);
	In.push_back( PropertyName::ButtonStyleOverlay);
	In.push_back( PropertyName::ButtonLengthStart);
	In.push_back( PropertyName::ButtonLengthEnd );
	In.push_back( PropertyName::ButtonLengthThumb);
	In.push_back( PropertyName::ButtonMarginStart);
	In.push_back( PropertyName::ButtonMarginEnd );
	In.push_back( PropertyName::ButtonMarginThumb);
	In.push_back( PropertyName::ButtonMarginTrack);
	In.push_back( PropertyName::ButtonMarginTrackDisabled);
	In.push_back( PropertyName::ButtonMarginOverlay);

	UIWidgetStyle::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UISliderbarStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedAppearance, category, o_groups);
}

//----------------------------------------------------------------------

void UISliderbarStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::Layout );

	In.push_back( PropertyName::ButtonStyleStart );
	In.push_back( PropertyName::ButtonStyleEnd  );
	In.push_back( PropertyName::ButtonStyleThumb);
	In.push_back( PropertyName::ButtonStyleTrack);
	In.push_back( PropertyName::ButtonStyleTrackDisabled);
	In.push_back( PropertyName::ButtonStyleOverlay);
	In.push_back( PropertyName::ButtonLengthStart);
	In.push_back( PropertyName::ButtonLengthEnd );
	In.push_back( PropertyName::ButtonLengthThumb);
	In.push_back( PropertyName::ButtonMarginStart);
	In.push_back( PropertyName::ButtonMarginEnd );
	In.push_back( PropertyName::ButtonMarginThumb);
	In.push_back( PropertyName::ButtonMarginTrack);
	In.push_back( PropertyName::ButtonMarginTrackDisabled);
	In.push_back( PropertyName::ButtonMarginOverlay);

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UISliderbarStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Layout )
		return UIUtils::ParseLayout( Value, mLayout );
	else
	{
		PropertyType ptype;
		ButtonType   btype;

		if (FindPropertyInfo (Name.c_str (), ptype, btype))
		{
			switch (ptype)
			{
			case PT_style:
				{
					UIButtonStyle * const bstyle = static_cast<UIButtonStyle *>(GetObjectFromPath (Value, TUIButtonStyle));

					if (bstyle|| Value.empty ())
						SetButtonStyle (btype, bstyle);
				}
				break;
			case PT_length:
				{
					long length = 0L;
					if (UIUtils::ParseLong (Value, length))
					{
						SetButtonLength (btype, length);
						return true;
					}
					return false;
				}
			case PT_margin:
				{
					UIRect rect;
					if (UIUtils::ParseRect (Value, rect))
					{
						SetButtonMargin (btype, rect);
						return true;
					}
					return false;
				}
			}
		}
	}

	return UIWidgetStyle::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UISliderbarStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if ( Name == PropertyName::Layout )
		return UIUtils::FormatLayout( Value, mLayout );
	else
	{

		PropertyType ptype;
		ButtonType   btype;

		if (FindPropertyInfo (Name.c_str (), ptype, btype))
		{
			switch (ptype)
			{
			case PT_style:
				{
					UIButtonStyle * const bstyle = mButtonStyles [btype];

					if (bstyle)
					{
						GetPathTo (Value, bstyle);
						return true;
					}
				}
				break;
			case PT_length:
				return UIUtils::FormatLong (Value, mButtonLengths [btype]);
			case PT_margin:
				return UIUtils::FormatRect (Value, mButtonMargins [btype]);
			}
		}
	}

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UISliderbarStyle::FindPropertyInfo (const char * const str, PropertyType & ptype, ButtonType & btype)
{
	static const char * const prefix_button     = "button";
	static const int          prefix_button_len = 6;

	static const std::string prefixes [PT_numPropertyTypes] =
	{
		"style",
		"length",
		"margin",
	};

	static const std::string buttonTypeNames [BT_numButtonTypes] =
	{
		"start",
		"end",
		"thumb",
		"track",
		"disabledTrack",
		"overlay"
	};

	if (!_strnicmp (str, prefix_button, prefix_button_len))
	{
		int pos = prefix_button_len;

		for (int i = 0; i < PT_numPropertyTypes; ++i)
		{
			const std::string & prefix = prefixes [i];
			if (!_strnicmp (str + pos, prefix.c_str (), prefix.size ()))
			{
				pos += prefix.size ();
				ptype = static_cast<PropertyType>(i);

				for (int j = 0; j < BT_numButtonTypes; ++j)
				{
					const std::string & btName = buttonTypeNames [j];
					if (!_strnicmp (str + pos, btName.c_str (), btName.size ()))
					{
						btype = static_cast<ButtonType>(j);
						return true;
					}
				}
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------
