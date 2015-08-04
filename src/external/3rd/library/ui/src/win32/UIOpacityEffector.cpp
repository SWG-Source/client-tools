#include "_precompile.h"
#include "UIOpacityEffector.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <vector>

const char * const UIOpacityEffector::TypeName                    = "OpacityEffector";

//----------------------------------------------------------------------

const UILowerString UIOpacityEffector::PropertyName::Speed          = UILowerString ("Speed");
const UILowerString UIOpacityEffector::PropertyName::TargetOpacity  = UILowerString ("TargetOpacity");
const UILowerString UIOpacityEffector::PropertyName::TargetOpacity2 = UILowerString ("TargetOpacity2");
const UILowerString UIOpacityEffector::PropertyName::Background     = UILowerString ("Background");
const UILowerString UIOpacityEffector::PropertyName::Cycling        = UILowerString ("Cycling");

//======================================================================================
#define _TYPENAME UIOpacityEffector

namespace UIOpacityEffectorNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Speed, "", T_float),
		_DESCRIPTOR(TargetOpacity, "", T_float),
		_DESCRIPTOR(TargetOpacity2, "", T_float),
		_DESCRIPTOR(Background, "", T_bool),
		_DESCRIPTOR(Cycling, "", T_bool),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIOpacityEffectorNamespace;
//======================================================================================

UIOpacityEffector::UIOpacityEffector () : 
mBackground     (false),
mTargetOpacity  (0.0f),
mTargetOpacity2 (0.0f),
mOpacitySpeed   (0.0f),
mOpacityStep    (0.0f),
mCycling        (false)
{
}

//-----------------------------------------------------------------

UIOpacityEffector::UIOpacityEffector (const float TargetOpacity, const float TargetOpacity2, const float OpacitySpeed, bool isCycling) :
mBackground     (false),
mTargetOpacity  (TargetOpacity),
mTargetOpacity2 (TargetOpacity2),
mOpacitySpeed   (OpacitySpeed),
mOpacityStep    (OpacitySpeed / 60.0f),
mCycling        (isCycling)
{
}

//======================================================================================

const char *UIOpacityEffector::GetTypeName () const
{
	return TypeName;
}

//======================================================================================

UIBaseObject *UIOpacityEffector::Clone () const
{
	return new UIOpacityEffector;
}

//======================================================================================

void UIOpacityEffector::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIEffector::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIOpacityEffector::GetPropertyNames (UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::Speed);
	In.push_back( PropertyName::TargetOpacity);
	In.push_back( PropertyName::TargetOpacity2);
	In.push_back( PropertyName::Background);
	In.push_back( PropertyName::Cycling);
	
	UIEffector::GetPropertyNames( In, forCopy );
}

//======================================================================================

bool UIOpacityEffector::SetProperty (const UILowerString & Name, const UIString &Value)
{
	if( Name == PropertyName::Speed )
	{
		if( !UIUtils::ParseFloat( Value, mOpacitySpeed ) )
			return false;

		mOpacityStep = mOpacitySpeed / 60.0f;
		return true;
	}
	else if( Name == PropertyName::TargetOpacity )
		return UIUtils::ParseFloat( Value, mTargetOpacity );
	else if( Name == PropertyName::TargetOpacity2 )
		return UIUtils::ParseFloat( Value, mTargetOpacity2 );
	else if( Name == PropertyName::Background )
		return UIUtils::ParseBoolean( Value, mBackground);
	else if( Name == PropertyName::Cycling )
		return UIUtils::ParseBoolean( Value, mCycling);
	else
		return UIEffector::SetProperty( Name, Value );
}

//======================================================================================

bool UIOpacityEffector::GetProperty (const UILowerString & Name, UIString &Value) const
{
	if( Name == PropertyName::Speed )
		return UIUtils::FormatFloat( Value, mOpacitySpeed );
	else if( Name == PropertyName::TargetOpacity )
		return UIUtils::FormatFloat( Value, mTargetOpacity );
	else if( Name == PropertyName::TargetOpacity2 )
		return UIUtils::FormatFloat( Value, mTargetOpacity2 );
	else if( Name == PropertyName::Background )
		return UIUtils::FormatBoolean( Value, mBackground);
	else if( Name == PropertyName::Cycling )
		return UIUtils::FormatBoolean( Value, mCycling);
	else
		return UIEffector::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

UIEffector::EffectResult UIOpacityEffector::handleOpacityChange (UIWidget & widget, const float targetOpacity) const
{
	EffectResult ReturnCode = Stop;
	float currentOpacity = 0.0f;

	if (mBackground)
		currentOpacity = widget.GetBackgroundOpacity ();
	else
		currentOpacity = widget.GetOpacity ();

	if( currentOpacity < targetOpacity )
	{
		currentOpacity += mOpacityStep;

		if( currentOpacity > targetOpacity )
			currentOpacity = targetOpacity;
		else
			ReturnCode = Continue;
	}
	else if( currentOpacity >= targetOpacity )
	{
		currentOpacity -= mOpacityStep;

		if( currentOpacity < targetOpacity )
			currentOpacity = targetOpacity;
		else
			ReturnCode = Continue;
	}

	if (mBackground)
		widget.SetBackgroundOpacity( currentOpacity );
	else
		widget.SetOpacity( currentOpacity );

	return ReturnCode;
}

//======================================================================================

UIEffector::EffectResult UIOpacityEffector::Effect (UIBaseObject * const theObject)
{
	if (!theObject->IsA (TUIWidget))
		return Stop;

	UIWidget * const widget = static_cast<UIWidget *>(theObject);
	EffectResult ReturnCode = Stop;

	if (!mCycling)
		ReturnCode = handleOpacityChange (*widget, mTargetOpacity);
	else
	{
		static UIString val;
		static const UILowerString CycleProperty = UILowerString ("UIColorEffectorCycle");

		const bool isCycling = widget->GetProperty (CycleProperty, val);

		if (!isCycling)
		{
			ReturnCode = handleOpacityChange (*widget, mTargetOpacity);

			if (ReturnCode == Stop)
			{
				val.assign (1, '1');
				widget->SetProperty (CycleProperty, val);
			}
		}
		else
		{
			ReturnCode = handleOpacityChange (*widget, mTargetOpacity2);
			if (ReturnCode == Stop)
			{
				widget->RemoveProperty (CycleProperty);
			}
		}

		ReturnCode = Continue;
	}

	return ReturnCode;
}

//-----------------------------------------------------------------

void UIOpacityEffector::SetTargetOpacity (const float TargetOpacity)
{
	mTargetOpacity = TargetOpacity;
}

//-----------------------------------------------------------------

void UIOpacityEffector::SetTargetOpacity2 (const float TargetOpacity)
{
	mTargetOpacity2 = TargetOpacity;
}

//-----------------------------------------------------------------

void UIOpacityEffector::SetOpacitySpeed (const float OpacitySpeed)
{
	mOpacitySpeed = OpacitySpeed;
	mOpacityStep  = mOpacitySpeed / 60.0f;
}

//-----------------------------------------------------------------
