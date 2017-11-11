#include "_precompile.h"

#include "UIDeformerWave.h"

#include "UICanvas.h"
#include "UIClock.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <cfloat>
#include <cmath>
#include <vector>

const char *UIDeformerWave::TypeName = "DeformerWave";

const UILowerString UIDeformerWave::PropertyName::Strength = UILowerString("WaveStrength");
const UILowerString UIDeformerWave::PropertyName::StrengthEnd = UILowerString("WaveStrengthEnd");
const UILowerString UIDeformerWave::PropertyName::Rate = UILowerString("WaveFrequency");
const UILowerString UIDeformerWave::PropertyName::RateEnd = UILowerString("WaveFrequencyEnd");
const UILowerString UIDeformerWave::PropertyName::Duration = UILowerString("WaveDuration");
const UILowerString UIDeformerWave::PropertyName::Interpolate = UILowerString("WaveSingleShot");

#define _TYPENAME UIDeformerWave

namespace UIDeformerWaveNamespace
{
	float const c_PiRad = 0.017453292519943295769236907684886f;
	float const c_HeartbeatTime = 1.0f/60.0f;

	float Offset(float const time, float const rate, float const strength)
	{
		return sinf(rate * c_PiRad * time) * strength;
	}

	template<class T>const T linearInterpolate (const T& start, const T& end, const float t)
	{
		return static_cast<T> (static_cast<float> (end - start) * t) + start;
	}

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Strength, "", T_floatPoint),
		_DESCRIPTOR(StrengthEnd, "", T_floatPoint),
		_DESCRIPTOR(Rate, "", T_floatPoint),
		_DESCRIPTOR(RateEnd, "", T_floatPoint),
		_DESCRIPTOR(Duration, "", T_float),
		_DESCRIPTOR(Interpolate, "", T_bool),
	_GROUPEND(Basic, 3, 0);
	//================================================================
}

using namespace UIDeformerWaveNamespace;

//=============================================================================

UIDeformerWave::UIDeformerWave() :
UIDeformer(),
mStrength(10.f, 10.f),
mStrengthEnd(0.f, 0.f),
mRate(15.f, 15.f),
mRateEnd(0.f, 0.f),
mDuration(3.0f),
mTime(0),
mInterpolate(true)
{
}

//=============================================================================

UIDeformerWave::~UIDeformerWave()
{
	UIClock::gUIClock().StopListening(this);
}

//=============================================================================

const char *UIDeformerWave::GetTypeName() const
{
	return TypeName;
}

//=============================================================================

UIBaseObject *UIDeformerWave::Clone() const
{
	return new UIDeformerWave;
}

//=============================================================================

void UIDeformerWave::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIDeformer::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIDeformerWave::GetPropertyNames(UIPropertyNameVector &In, bool forCopy) const
{
	UIDeformer::GetPropertyNames(In, forCopy);

	In.push_back(PropertyName::Strength);
	In.push_back(PropertyName::StrengthEnd);
	In.push_back(PropertyName::Rate);
	In.push_back(PropertyName::RateEnd);
	In.push_back(PropertyName::Duration);
	In.push_back(PropertyName::Interpolate);
}

//=============================================================================

bool UIDeformerWave::SetProperty(const UILowerString & Name, const UIString &Value)
{
	if (Name == PropertyName::Strength)
	{
		return UIUtils::ParseFloatPoint(Value, mStrength);
	}
	if (Name == PropertyName::StrengthEnd)
	{
		return UIUtils::ParseFloatPoint(Value, mStrengthEnd);
	}
	else if (Name == PropertyName::Rate)
	{
		return UIUtils::ParseFloatPoint(Value, mRate);
	}
	else if (Name == PropertyName::RateEnd)
	{
		return UIUtils::ParseFloatPoint(Value, mRateEnd);
	}
	else if (Name == PropertyName::Interpolate)
	{
		bool interp = true;

		bool success = UIUtils::ParseBoolean(Value, interp);
		if (success)
		{
			SetInterpolate(interp);
		}

		return success;
	}
	else if (Name == PropertyName::Duration)
	{
		float o = 0.0f;

		if (!UIUtils::ParseFloat(Value, o))
			return false;

		SetDuration(o);
		return true;
	}
	else
		return UIDeformer::SetProperty(Name, Value);
}

//=============================================================================

bool UIDeformerWave::GetProperty(const UILowerString & Name, UIString &Value) const
{
	if (Name == PropertyName::Strength)
		return UIUtils::FormatFloatPoint(Value, mStrength);
	if (Name == PropertyName::StrengthEnd)
		return UIUtils::FormatFloatPoint(Value, mStrengthEnd);
	else if (Name == PropertyName::Rate)
		return UIUtils::FormatFloatPoint(Value, mRate);
	else if (Name == PropertyName::RateEnd)
		return UIUtils::FormatFloatPoint(Value, mRateEnd);
	else if (Name == PropertyName::Duration)
		return UIUtils::FormatFloat(Value, mDuration);
	else if (Name == PropertyName::Interpolate)
		return UIUtils::FormatBoolean(Value, mInterpolate);
	else
		return UIDeformer::GetProperty(Name, Value);
}

//=============================================================================

UIEffector::EffectResult UIDeformerWave::Effect(UIBaseObject *theObject)
{
	UIDeformer::Effect(theObject);

	EffectResult result = Continue;

	if (mInterpolate && mTime >= mDuration)
	{
		result = Stop;
		SetTarget(0);
	}

	return result;
}

//=============================================================================

bool UIDeformerWave::Deform(UICanvas & /*canvas*/, UIFloatPoint const * points, UIFloatPoint * out, size_t count)
{
	if (mInterpolate)
	{
		if (mDuration > 0.0f && mTime < mDuration)
		{
			float const t = mTime / mDuration;

			float const RateX = linearInterpolate(mRate.x, mRateEnd.x, t);
			float const RateY = linearInterpolate(mRate.y, mRateEnd.y, t);
			float const StrengthX = linearInterpolate(mStrength.x, mStrengthEnd.x, t);
			float const StrengthY = linearInterpolate(mStrength.y, mStrengthEnd.y, t);

			for (size_t cnt = 0; cnt < count; ++cnt, ++out, ++points)
			{
				out->x = points->x + Offset(mTime + points->x, RateX, StrengthX);
				out->y = points->y + Offset(mTime + points->y, RateY, StrengthY);
			}
		}
		else
		{
			memcpy(out, points, sizeof(UIFloatPoint) * count);
		}
	}
	else
	{
		mTime = fmodf(mTime, 10000.f); //-- Don't let time get too big.

		for (size_t cnt = 0; cnt < count; ++cnt, ++out, ++points)
		{
			out->x = points->x + Offset(mTime * points->x, mRate.x, mStrength.x);
			out->y = points->y + Offset(mTime * points->y, mRate.y, mStrength.y);
		}
	}

	return mTime < mDuration;
}

//=============================================================================

void UIDeformerWave::SetDuration(float const duration)
{
	if (mDuration > 0.0f)
	{
		mDuration = duration;
	}

	mTime = 0;
}

//=============================================================================

void UIDeformerWave::SetInterpolate(bool const interp)
{
	if (mInterpolate != interp)
	{
		mInterpolate = interp;
		mTime = 0;
	}
}

//=============================================================================

void UIDeformerWave::Notify(UINotificationServer * server, UIBaseObject * subject, Code code)
{
	UIDeformer::Notify(server, subject, code);

	if (GetTarget())
	{
		mTime += c_HeartbeatTime;
	}
}

//=============================================================================

void UIDeformerWave::OnTargetChanged(UIWidget * const newTarget, UIWidget * const oldTarget)
{
	UIDeformer::OnTargetChanged(newTarget, oldTarget);

	if (oldTarget != newTarget)
	{
		mTime = 0.0f;

		if (newTarget)
		{
			UIClock::gUIClock().Listen(this);
		}
		else
		{
			UIClock::gUIClock().StopListening(this);
		}
	}
}

//=============================================================================
