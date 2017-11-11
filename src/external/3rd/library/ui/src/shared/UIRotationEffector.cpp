// ======================================================================
//
// UIRotationEffector.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UIRotationEffector.h"

#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <cassert>
#include <cmath>
#include <vector>

// ======================================================================

const char * const UIRotationEffector::TypeName                     = "RotationEffector";

//----------------------------------------------------------------------

const UILowerString UIRotationEffector::PropertyName::Period         = UILowerString ("Period");
const UILowerString UIRotationEffector::PropertyName::Cycling        = UILowerString ("Cycling");
const UILowerString UIRotationEffector::PropertyName::Target         = UILowerString ("Target");

//======================================================================================
#define _TYPENAME UIRotationEffector

namespace UIRotationEffectorNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Period, "", T_float),
		_DESCRIPTOR(Cycling, "", T_bool),
		_DESCRIPTOR(Target, "", T_float),
	_GROUPEND(Basic, 2, 0);
	//================================================================
}
using namespace UIRotationEffectorNamespace;
//======================================================================================

UIRotationEffector::UIRotationEffector   () :
UIEffector (),
mPeriod    (1.0f),
mStep      (0.0f),
mCycling   (true),
mTarget    (0.0f)
{
	SetPeriod (1.0f);
}

//----------------------------------------------------------------------

UIRotationEffector::UIRotationEffector   (const float period) :
UIEffector (),
mPeriod    (1.0f),
mStep      (0.0f)
{
	SetPeriod (period);
}

//----------------------------------------------------------------------

const char    *UIRotationEffector::GetTypeName       () const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject  *UIRotationEffector::Clone             () const
{
	return new UIRotationEffector;
}

//----------------------------------------------------------------------

void UIRotationEffector::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIEffector::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void           UIRotationEffector::GetPropertyNames  (UIPropertyNameVector & in, bool forCopy) const
{
	in.push_back (PropertyName::Period);
	in.push_back (PropertyName::Cycling);
	in.push_back (PropertyName::Target);

	UIEffector::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool           UIRotationEffector::SetProperty       (const UILowerString & Name, const UIString &Value)
{
	if (Name == PropertyName::Period)
	{
		float f = 0.0f;
		if (UIUtils::ParseFloat (Value, f))
			SetPeriod (f);
	}
	else if (Name == PropertyName::Cycling)
	{
		return UIUtils::ParseBoolean (Value, mCycling);
	}
	else if (Name == PropertyName::Target)
	{
		return UIUtils::ParseFloat (Value, mTarget);
	}

	return UIEffector::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UIRotationEffector::GetProperty       (const UILowerString & Name, UIString &Value) const
{
	if (Name == PropertyName::Period)
	{
		return UIUtils::FormatFloat (Value, mPeriod);
	}
	else if (Name == PropertyName::Cycling)
	{
		return UIUtils::FormatBoolean (Value, mCycling);
	}
	else if (Name == PropertyName::Target)
	{
		return UIUtils::FormatFloat (Value, mTarget);
	}

	return UIEffector::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

UIRotationEffector::EffectResult   UIRotationEffector::Effect            (UIBaseObject *theObject)
{
	assert (theObject);

	if (theObject->IsA (TUIWidget))
	{
		UIWidget * const w = static_cast<UIWidget *>(theObject);
		double wtf = 0.0f;
		const float curRotation = static_cast<float>(modf (w->GetRotation (), &wtf));
		const float rotation    = static_cast<float>(modf (mStep + curRotation, &wtf));

		if (!mCycling)
		{
			if (curRotation == mTarget)
			{
				w->SetRotation (mTarget);
				return Stop;
			}

			//- forward
			else if (mStep > 0.0f)
			{
				if ((curRotation < mTarget || curRotation > rotation) && rotation >= mTarget)
				{
					w->SetRotation (mTarget);
					return Stop;
				}
			}
			//- backward
			else
			{
				if ((curRotation > mTarget || curRotation < rotation) && rotation <= mTarget)
				{
					w->SetRotation (mTarget);
					return Stop;
				}
			}
		}

		w->SetRotation (rotation);

		return Continue;
	}

	return Stop;
}

//----------------------------------------------------------------------

void UIRotationEffector::SetPeriod         (const float period)
{
	mPeriod = period;

	if (mPeriod == 0.0f)
		mStep = 0.0f;
	else
	{
		//-- clock ticks are in 1/60th of a second?
		mStep = 1.0f / (mPeriod * 60.0f);
	}
}

//----------------------------------------------------------------------

