#include "_precompile.h"

#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UICanvas.h"
#include "UIEventCallback.h"
#include "UIImageStyle.h"
#include "UIMessage.h"
#include "UIPropertyDescriptor.h"
#include "UIRectangleStyle.h"
#include "UISliderbar.h"
#include "UISliderbarStyle.h"
#include "UIUtils.h"
#include "UIWidgetRectangleStyles.h"

#include <vector>

//----------------------------------------------------------------------

const char * const UISliderbar::TypeName                 = "Sliderbar";

const UILowerString  UISliderbar::PropertyName::LowerLimit               = UILowerString ("LowerLimit");
const UILowerString  UISliderbar::PropertyName::UpperLimit               = UILowerString ("UpperLimit");
const UILowerString  UISliderbar::PropertyName::UpperLimitAllowed        = UILowerString ("UpperLimitAllowed");
const UILowerString  UISliderbar::PropertyName::UpperLimitAllowedEnabled = UILowerString ("UpperLimitAllowedEnabled");
const UILowerString  UISliderbar::PropertyName::Value                    = UILowerString ("Value");

//======================================================================================
#define _TYPENAME UISliderbar

namespace UISliderbarNamespace
{
	const unsigned long INCREMENT_TICK_TIME = 1;
	const unsigned long INCREMENT_TICK_TIME_DELAY = 20;


	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(LowerLimit, "", T_int),
		_DESCRIPTOR(UpperLimit, "", T_int),
		_DESCRIPTOR(Value, "", T_int),
	_GROUPEND(Basic, 4, int(UIPropertyCategories::C_Basic));
	//================================================================
	//================================================================
	// Behavior category.
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(UpperLimitAllowed, "", T_int),
		_DESCRIPTOR(UpperLimitAllowedEnabled, "", T_bool),
	_GROUPEND(Behavior, 4, int(UIPropertyCategories::C_Behavior));
	//================================================================
}
using namespace UISliderbarNamespace;
//======================================================================================

UISliderbar::UISliderbar  () :
UISliderbase              (),
mValue                    (0),
mUpperLimit	              (0),
mUpperLimitAllowed        (0),
mLowerLimit	              (0),
mUpperLimitAllowedEnabled (false),
mIncrementValue           (1)
{

}

//----------------------------------------------------------------------

UISliderbar::~UISliderbar()
{

}

//----------------------------------------------------------------------
												
bool UISliderbar::IsA( const UITypeID Type ) const
{
	return (Type == TUISliderbar) || UISliderbase::IsA( Type );
}

//----------------------------------------------------------------------

const char *UISliderbar::GetTypeName() const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UISliderbar::Clone() const
{
	return new UISliderbar;
}

//----------------------------------------------------------------------

void UISliderbar::SetValue( long In, bool notify )
{
	In = std::max (mLowerLimit,        In);

	if (mUpperLimitAllowedEnabled)
		In = std::min (mUpperLimitAllowed, In);

	In = std::min (mUpperLimit,        In);

	if( mValue != In )
	{
		mValue = In;

		UI_UNREF (notify);
		if (notify)
			SendCallback( &UIEventCallback::OnSliderbarChanged, UISliderbase::PropertyName::OnSliderChange );
	}
}

//----------------------------------------------------------------------

void UISliderbar::SetUpperLimitAllowed (long In, bool notify)
{
	In = std::max (mLowerLimit, In);
	In = std::min (mUpperLimit, In);

	mUpperLimitAllowed = In;

	if (!mUpperLimitAllowedEnabled)
		return;

	SetValue (std::min (mValue, mUpperLimitAllowed), notify);
}

//----------------------------------------------------------------------

void UISliderbar::SetUpperLimit(long In, bool notify )
{
	In = std::max (mLowerLimit, In);

	mUpperLimit        = In;

	//-- update the value to the new range
	SetValue (mValue, notify);
}

//----------------------------------------------------------------------

void UISliderbar::SetLowerLimit( const long In, bool notify )
{
	if( In > mUpperLimit )
	{
		mUpperLimit        = In;
	}

	mLowerLimit = In;

	//-- update the value to the new range
	SetValue (mValue, notify);
}

//----------------------------------------------------------------------

void UISliderbar::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UISliderbase::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
}

//----------------------------------------------------------------------

void UISliderbar::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::LowerLimit );
	In.push_back( PropertyName::UpperLimit );
	In.push_back( PropertyName::UpperLimitAllowed);
	In.push_back( PropertyName::UpperLimitAllowedEnabled);
	In.push_back( PropertyName::Value );

	UISliderbase::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UISliderbar::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Value )
	{	
		long NewValue;

		if( UIUtils::ParseLong( Value, NewValue ) )
		{
			SetValue( NewValue, true );
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::LowerLimit )
	{
		long l = 0;
		if (UIUtils::ParseLong( Value, l ))
		{
			SetLowerLimit (l);
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::UpperLimit )
	{
		long l = 0;
		if (UIUtils::ParseLong( Value, l ))
		{
			SetUpperLimit (l);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::UpperLimitAllowed)
	{
		long l = 0;
		if (UIUtils::ParseLong( Value, l ))
		{
			SetUpperLimitAllowed (l);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::UpperLimitAllowedEnabled)
	{
		bool b = false;
		if (UIUtils::ParseBoolean ( Value, b ))
		{
			SetUpperLimitAllowedEnabled (b);
			return true;
		}
		return false;
	}

	return UISliderbase::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UISliderbar::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Value )
		return UIUtils::FormatLong( Value, mValue );
	else if( Name == PropertyName::LowerLimit )
		return UIUtils::FormatLong( Value, mLowerLimit );
	else if( Name == PropertyName::UpperLimit )
		return UIUtils::FormatLong( Value, mUpperLimit );
	else if (Name == PropertyName::UpperLimitAllowed)
		return UIUtils::FormatLong( Value, mUpperLimitAllowed );
	else if (Name == PropertyName::UpperLimitAllowedEnabled)
		return UIUtils::FormatBoolean( Value, mUpperLimitAllowedEnabled );

	return UISliderbase::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

long UISliderbar::GetDesiredThumbSize (long) const
{
	return 0;
}

//----------------------------------------------------------------------

long UISliderbar::GetDesiredThumbPosition (long range) const
{
	if (range < 0)
		return 0;

	const float valueRange = static_cast<float>(mUpperLimit - mLowerLimit);

	if (valueRange > 0.0f)
	{
		const float value      = static_cast<float>(mValue - mLowerLimit);
		const float range_f    = static_cast<float>(range);
		return static_cast<long>( value * range_f / valueRange);
	}

	return 0;
}

//----------------------------------------------------------------------

void UISliderbar::IncrementValue (bool page, bool upDown, bool notify)
{
	UI_UNREF (page);

	if (upDown)
		SetValue (GetValue () + GetIncrementValue(), notify);
	else
		SetValue (GetValue () - GetIncrementValue(), notify);
}

//----------------------------------------------------------------------

void UISliderbar::UpdateValueFromThumbPosition (long position, long range, bool notify)
{
	if (position < 0L || range <= 0)
		SetValue (mLowerLimit, notify);
	else if (position >= range)
		SetValue (mUpperLimit, notify);
	else
	{
		if (range) 
		{
			const float range_f    = static_cast<float>(range);
			const float valueRange = static_cast<float>(mUpperLimit - mLowerLimit + 1) / static_cast<float>(mIncrementValue);
			const long value       = mLowerLimit + ( static_cast<long>(static_cast<float>(valueRange * (static_cast<float>(position) / range_f))) * mIncrementValue );
			SetValue (value, notify);
		}
	}
}

//----------------------------------------------------------------------

bool UISliderbar::GetDesiredDisabledTrackPosition (long range, long & pos) const
{
	if (!mUpperLimitAllowedEnabled)
		return false;

	if (mUpperLimitAllowed <= mLowerLimit || mUpperLimitAllowed >= mUpperLimit)
		return false;

	const float valueRange = static_cast<float>(mUpperLimit - mLowerLimit);

	if (valueRange > 0.0f)
	{
		const float value      = static_cast<float>(mUpperLimitAllowed - mLowerLimit);
		const float range_f    = static_cast<float>(range);
		pos = static_cast<long>(value * range_f / valueRange);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void UISliderbar::SetUpperLimitAllowedEnabled (bool b)
{
	mUpperLimitAllowedEnabled = b;
	if (b)
	{
		SetUpperLimitAllowed (mUpperLimitAllowed);
	}
}

//----------------------------------------------------------------------

void UISliderbar::SetIncrementValue(const long increment)
{
	mIncrementValue = increment;
}

//----------------------------------------------------------------------