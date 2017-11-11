// ======================================================================
//
// UIColorEffector.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UIColorEffector.h"

#include "UIButton.h"
#include "UIPalette.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <vector>

// ======================================================================

const char * const UIColorEffector::TypeName                     = "ColorEffector";


//----------------------------------------------------------------------

const UILowerString  UIColorEffector::PropertyName::Speed          = UILowerString ("Speed");
const UILowerString  UIColorEffector::PropertyName::TargetColor    = UILowerString ("TargetColor");
const UILowerString  UIColorEffector::PropertyName::TargetColor2   = UILowerString ("TargetColor2");
const UILowerString  UIColorEffector::PropertyName::Cycling        = UILowerString ("Cycling");
const UILowerString  UIColorEffector::PropertyName::Background     = UILowerString ("Background");
const UILowerString  UIColorEffector::PropertyName::BackgroundTint = UILowerString ("BackgroundTint");
const UILowerString  UIColorEffector::PropertyName::Icon           = UILowerString ("Icon");
const UILowerString  UIColorEffector::PropertyName::RestoreColor   = UILowerString ("RestoreColor");
const UILowerString  UIColorEffector::PropertyName::ForceColor = UILowerString ("ForceColor");

//======================================================================================
#define _TYPENAME UIColorEffector

namespace UIColorEffectorNamespace
{
	//================================================================
	// Apoearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(Background, "", T_bool),
		_DESCRIPTOR(BackgroundTint, "", T_bool),
		_DESCRIPTOR(Icon, "", T_bool),
		_DESCRIPTOR(TargetColor, "", T_color),
		_DESCRIPTOR(TargetColor2, "", T_color),
		_DESCRIPTOR(Cycling, "", T_bool),
		_DESCRIPTOR(Speed, "", T_float)
	_GROUPEND(Appearance, 2, 0);
	//================================================================

	//================================================================
	// Behavior category
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(RestoreColor, "", T_bool),
		_DESCRIPTOR(ForceColor, "", T_bool)
	_GROUPEND(Behavior, 2, 1);
	//================================================================

}
using namespace UIColorEffectorNamespace;

//======================================================================================

UIColorEffector::UIColorEffector () : 
mBackground     (false),
mBackgroundTint (false),
mIcon           (false),
mTargetColor    (UIColor::white),
mTargetColor2   (UIColor::black),
mCycling        (false),
mSpeed          (0.0f),
mStep           (0),
mOriginalColor(),
mRestorelColor(false),
mForceColor(false)
{
}

//-----------------------------------------------------------------

UIColorEffector::UIColorEffector (const UIColor & targetColor, const float speed, bool background, bool backgroundTint, bool icon) :
mBackground     (background),
mBackgroundTint (backgroundTint && !background),
mIcon           (icon && !background && !backgroundTint),
mTargetColor    (),
mTargetColor2   (),
mCycling        (false),
mSpeed          (0.0f),
mStep           (0),
mOriginalColor(),
mRestorelColor(false),
mForceColor(false)
{
	SetTargetColor (targetColor);
	SetSpeed       (speed);
}

//======================================================================================

const char *UIColorEffector::GetTypeName () const
{
	return TypeName;
}

//======================================================================================

UIBaseObject *UIColorEffector::Clone () const
{
	return new UIColorEffector;
}

//======================================================================================

void UIColorEffector::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIEffector::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
}

//======================================================================================

void UIColorEffector::GetPropertyNames (UIPropertyNameVector &In, bool forCopy) const
{
	UIPalette::GetPropertyNamesForType (TUIColorEffector, In);

	In.push_back( PropertyName::Background);
	In.push_back( PropertyName::Icon);
	In.push_back( PropertyName::BackgroundTint);
	In.push_back( PropertyName::Cycling);
	In.push_back( PropertyName::Speed);
	In.push_back( PropertyName::TargetColor);
	In.push_back( PropertyName::TargetColor2);
	In.push_back( PropertyName::RestoreColor);
	In.push_back( PropertyName::ForceColor);
	
	UIEffector::GetPropertyNames( In, forCopy );
}

//======================================================================================

bool UIColorEffector::SetProperty (const UILowerString & Name, const UIString &Value)
{
	if( Name == PropertyName::Speed )
	{
		if( !UIUtils::ParseFloat( Value, mSpeed ) )
			return false;

		SetSpeed (mSpeed);
		return true;
	}
	else if( Name == PropertyName::TargetColor )
		return UIUtils::ParseColor( Value, mTargetColor );
	else if( Name == PropertyName::TargetColor2 ) 
		return UIUtils::ParseColor( Value, mTargetColor2 );
	else if( Name == PropertyName::Cycling )
		return UIUtils::ParseBoolean( Value, mCycling );
	else if( Name == PropertyName::Background )
	{
		if (UIUtils::ParseBoolean( Value, mBackground))
		{
			if (mBackground && mBackgroundTint)
				mBackgroundTint = false;
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::BackgroundTint )
	{
		if (UIUtils::ParseBoolean( Value, mBackgroundTint))
		{
			if (mBackground && mBackgroundTint)
				mBackground = false;
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::Icon )
	{
		if (UIUtils::ParseBoolean( Value, mIcon))
		{
			if (mIcon)
			{
				mBackgroundTint = false;
				mBackground     = false;
			}

			return true;
		}
		return false;
	}
	else if (Name == PropertyName::RestoreColor)
	{
		if (UIUtils::ParseBoolean(Value, mRestorelColor))
		{
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::ForceColor)
	{
		if (UIUtils::ParseBoolean(Value, mForceColor))
		{
			return true;
		}
		return false;
	}
	else
	{
		UIPalette::SetPropertyForObject (*this, Name, Value);
	}

	return UIEffector::SetProperty( Name, Value );
}

//======================================================================================

bool UIColorEffector::GetProperty (const UILowerString & Name, UIString &Value) const
{
	if( Name == PropertyName::Speed )
		return UIUtils::FormatFloat ( Value, mSpeed );
	else if( Name == PropertyName::TargetColor ) 
		return UIUtils::FormatColor ( Value, mTargetColor );
	else if( Name == PropertyName::TargetColor2 )
		return UIUtils::FormatColor( Value, mTargetColor2 );
	else if( Name == PropertyName::Cycling )
		return UIUtils::FormatBoolean( Value, mCycling );
	else if( Name == PropertyName::Background )
		return UIUtils::FormatBoolean ( Value, mBackground);
	else if( Name == PropertyName::BackgroundTint )
		return UIUtils::FormatBoolean ( Value, mBackgroundTint);
	else if( Name == PropertyName::Icon )
		return UIUtils::FormatBoolean ( Value, mIcon);
	else if(Name == PropertyName::RestoreColor)
		return UIUtils::FormatBoolean(Value, mRestorelColor);
	else if(Name == PropertyName::ForceColor)
		return UIUtils::FormatBoolean(Value, mForceColor);
	else
		return UIEffector::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

UIEffector::EffectResult UIColorEffector::handleColorChange (UIWidget * const widget, const UIColor & targetColor)
{
	EffectResult ReturnCode = Stop;
	
	UIColor const & CurrentColor = GetCurrentColor(widget);

	static int current [3];
	current [0] = CurrentColor.r;
	current [1] = CurrentColor.g;
	current [2] = CurrentColor.b;
	

	static int target [3];
	target [0] = targetColor.r;
	target [1] = targetColor.g;
	target [2] = targetColor.b;

	for (int i = 0; i < 3; ++i)
	{		
		if( current [i] < target [i])
		{
			current [i] += mStep;
			
			if( current [i] > target [i] )
				current [i] = target [i];
			else
				ReturnCode = Continue;
		}
		else if( current [i] > target [i] )
		{
			current [i] -= mStep;
			
			if( current [i] < target [i] )
				current [i] = target [i];
			else
				ReturnCode = Continue;
		}
	}

	UIColor newColor;
	newColor.r = static_cast<unsigned char>(std::max (0, std::min (255, current [0])));
	newColor.g = static_cast<unsigned char>(std::max (0, std::min (255, current [1])));
	newColor.b = static_cast<unsigned char>(std::max (0, std::min (255, current [2])));
	newColor.a = CurrentColor.a;

	SetCurrentColor(widget, newColor);
	
	return ReturnCode;
}

//======================================================================================

UIEffector::EffectResult UIColorEffector::Effect (UIBaseObject * const theObject)
{
	if (!theObject->IsA (TUIWidget) || !mStep)
		return Stop;

	EffectResult retval = Stop;

	UIWidget * const widget  = static_cast<UIWidget *>( theObject );

	if (!mCycling)
		retval = handleColorChange (widget, mTargetColor);
	else
	{
		static UIString val;
		static const UILowerString CycleProperty = UILowerString ("UIColorEffectorCycle");

		const bool isCycling = widget->GetProperty (CycleProperty, val);

		if (!isCycling)
		{
			retval = handleColorChange (widget, mTargetColor);

			if (retval == Stop)
			{
				val.assign (1, '1');
				widget->SetProperty (CycleProperty, val);
			}
		}
		else
		{
			retval = handleColorChange (widget, mTargetColor2);
			if (retval == Stop)
			{
				widget->RemoveProperty (CycleProperty);
			}
		}

		retval = Continue;
	}


	return retval;
}

//-----------------------------------------------------------------

void UIColorEffector::SetTargetColor (const UIColor & TargetColor)
{
	mTargetColor = TargetColor;
}

//----------------------------------------------------------------------

void UIColorEffector::SetTargetColor2   (const UIColor & targetColor)
{
	mTargetColor2 = targetColor;
}

//-----------------------------------------------------------------

void UIColorEffector::SetSpeed (const float speed)
{
	mSpeed = speed;
	mStep  = static_cast<unsigned char>(255.0f * mSpeed / 60.0f);
}

//-----------------------------------------------------------------

UIColor UIColorEffector::GetCurrentColor(UIWidget const * const widget) const
{
	UIColor CurrentColor;
	
	if (widget)
	{
		if (mBackground)
		{
			CurrentColor = widget->GetBackgroundColor();
		}
		else if (mBackgroundTint)
		{
			CurrentColor = widget->GetBackgroundTint();
		}
		else if (mIcon)
		{
			if (widget->IsA (TUIButton))
				CurrentColor = static_cast<UIButton const *>(widget)->GetIconColor();
		}
		else
			CurrentColor = widget->GetColor();
	}

	return CurrentColor;
}

//-----------------------------------------------------------------

void UIColorEffector::SetCurrentColor(UIWidget * const widget, UIColor const & newColor)
{
	if (widget)
	{
		if (mForceColor) 
		{
			widget->SetColor(newColor, true);
		}
		else
		{
			if (mBackground)
				widget->SetBackgroundColor(newColor);
			else if (mBackgroundTint)
				widget->SetBackgroundTint (newColor);
			else if (mIcon)
			{
				if (widget->IsA (TUIButton))
					static_cast<UIButton *>(widget)->SetIconColor (newColor);
			}
			else
				widget->SetColor(newColor);
		}
	}
}

//-----------------------------------------------------------------

UIEffector::EffectResult UIColorEffector::OnCreate(UIBaseObject *theObject)
{
	if (mRestorelColor)
	{
		mOriginalColor = GetCurrentColor(UI_ASOBJECT(UIWidget, theObject));
	}

	return UIEffector::OnCreate(theObject);;
}

//-----------------------------------------------------------------

void UIColorEffector::OnDestroy(UIBaseObject *theObject)
{
	UIEffector::OnDestroy(theObject);

	if (theObject && mRestorelColor)
	{
		UIWidget * const theWidget = UI_ASOBJECT(UIWidget, theObject);
		
		if (theWidget) 
		{
			SetCurrentColor(theWidget, mOriginalColor);
			
			if (mForceColor) 
			{
				theWidget->Link();

				UIPalette * const palette = UIPalette::GetInstance();
				if (palette)
				{
					palette->ApplyPalette(*theWidget);
				}
			}
		}
	}
}

// ======================================================================
