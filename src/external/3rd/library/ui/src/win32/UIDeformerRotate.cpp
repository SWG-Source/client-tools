#include "_precompile.h"
#include "UIDeformerRotate.h"

#include "UICanvas.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"
#include "UIClock.h"

#include <cfloat>
#include <cmath>
#include <vector>

//=============================================================================

const char *UIDeformerRotate::TypeName = "DeformerRotate";

const UILowerString UIDeformerRotate::PropertyName::HorizontalActive = UILowerString("HorizontalActive");
const UILowerString UIDeformerRotate::PropertyName::HorizontalRight = UILowerString("HorizontalRight");
const UILowerString UIDeformerRotate::PropertyName::VerticalActive = UILowerString("VerticalActive");
const UILowerString UIDeformerRotate::PropertyName::VerticalDown = UILowerString("VerticalDown");
const UILowerString UIDeformerRotate::PropertyName::FromTheCenter = UILowerString("FromTheCenter");
const UILowerString UIDeformerRotate::PropertyName::Duration = UILowerString("DurationInSeconds");
const UILowerString UIDeformerRotate::PropertyName::StartDelay = UILowerString("StartDelayInSeconds");
const UILowerString UIDeformerRotate::PropertyName::SlideIn = UILowerString("SlideIn");
const UILowerString UIDeformerRotate::PropertyName::RotateStart = UILowerString("RotateStartDegrees");
const UILowerString UIDeformerRotate::PropertyName::RotateStop = UILowerString("RotateStopDegrees");
const UILowerString UIDeformerRotate::PropertyName::RotateStopAndHold = UILowerString("RotateStopThenHold");

//=============================================================================
#define _TYPENAME UIDeformerRotate

namespace UIDeformerRotateNamespace
{
	float const c_pi180 = 3.14f / 180.f;
	float const c_startRotation = 4.71f;
	float const c_updateRate = 30.0f / 1000.0f;

	// RLS TODO: Move to UIUtils.h
	template<class T>inline const T clamp(const T &minT, const T &t, const T &maxT)
	{
		return (t < minT) ? minT : ((t > maxT) ? maxT : t);
	}

	template<class T> const T square(T const & value)
	{
		return value * value;
	}

	template<class T> const T nonLinearInterpolate(const T& start, const T& end, const float t)
	{
		float const tSquared = square(t);
		float const tCubed = tSquared * t;
		T const & diff = end - start;
		return static_cast<T>((diff * (tCubed * -2.0f)) + (diff * (tSquared * 3.0f)) + start);
	}

	inline float fastsqrt(float value)
	{
		int * const ival = reinterpret_cast<int*>(&value);
		*ival = ((*ival) >> 1) + (0x3f800000 >> 1);
		return value;
	}
	
	float DegreesToRadians(float const degrees)
	{
		return degrees * c_pi180;
	}

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(HorizontalActive, "", T_bool),
		_DESCRIPTOR(HorizontalRight, "", T_bool),
		_DESCRIPTOR(VerticalActive, "", T_bool),
		_DESCRIPTOR(VerticalDown, "", T_bool),
		_DESCRIPTOR(FromTheCenter, "", T_bool),
		_DESCRIPTOR(Duration, "", T_float),
		_DESCRIPTOR(StartDelay, "", T_float),
		_DESCRIPTOR(SlideIn, "", T_bool),
		_DESCRIPTOR(RotateStart, "", T_float),
		_DESCRIPTOR(RotateStop, "", T_float),
		_DESCRIPTOR(RotateStopAndHold, "", T_bool),
	_GROUPEND(Basic, 3, 0);
	//================================================================
}

using namespace UIDeformerRotateNamespace;

//=============================================================================

UIDeformerRotate::UIDeformerRotate() :
UIDeformer(),
mHorizontalActive(true),
mHorizontalRight(true),
mVerticalActive(true),
mVerticalDown(true),
mFromTheCenter(true),
mDuration(0.5f),
mTimeSeconds(0.0f),
mStartDelay(0.0f),
mSlideIn(false),
mTriggerEffect(false),
mRotateStart(0.0),
mRotateStop(360.0),
mRotateStopAndHold(true),
mTimerDirty(true)
{
}

//=============================================================================

UIDeformerRotate::~UIDeformerRotate()
{
}

//=============================================================================

const char *UIDeformerRotate::GetTypeName() const
{
	return TypeName;
}

//=============================================================================

UIBaseObject *UIDeformerRotate::Clone() const
{
	return new UIDeformerRotate;
}

//=============================================================================

void UIDeformerRotate::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIDeformer::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIDeformerRotate::GetPropertyNames(UIPropertyNameVector &In, bool forCopy) const
{
	UIDeformer::GetPropertyNames(In, forCopy);

	In.push_back(PropertyName::HorizontalActive);
	In.push_back(PropertyName::HorizontalRight); 
	In.push_back(PropertyName::VerticalActive);
	In.push_back(PropertyName::VerticalDown);
	In.push_back(PropertyName::FromTheCenter);
	In.push_back(PropertyName::Duration);
	In.push_back(PropertyName::StartDelay);
	In.push_back(PropertyName::SlideIn);
	In.push_back(PropertyName::RotateStart);
	In.push_back(PropertyName::RotateStop);
	In.push_back(PropertyName::RotateStopAndHold);
}

//=============================================================================

bool UIDeformerRotate::SetProperty(const UILowerString & Name, const UIString &Value)
{
	if (Name == PropertyName::HorizontalActive)
	{
		bool hz = false;
		
		if (!UIUtils::ParseBoolean(Value, hz))
			return false;
		
		mHorizontalActive = hz;
		return true;
	}
	else if (Name == PropertyName::HorizontalRight)
	{
		bool hz = false;
		
		if (!UIUtils::ParseBoolean(Value, hz))
			return false;
		
		mHorizontalRight = hz;
		return true;
	}
	else if (Name == PropertyName::VerticalActive)
	{
		bool vt = false;
		
		if (!UIUtils::ParseBoolean(Value, vt))
			return false;
		
		mVerticalActive = vt;
		return true;
	}
	else if (Name == PropertyName::VerticalDown)
	{
		bool vt = false;
		
		if (!UIUtils::ParseBoolean(Value, vt))
			return false;
		
		mVerticalDown = vt;
		return true;
	}
	else if (Name == PropertyName::FromTheCenter)
	{
		bool flip = false;
		
		if (!UIUtils::ParseBoolean(Value, flip))
			return false;
		
		mFromTheCenter = flip;
		return true;
	}
	else if (Name == PropertyName::Duration)
	{
		float duration = 0.0f;
		
		if (!UIUtils::ParseFloat(Value, duration))
			return false;
		
		mDuration = duration;
		return true;
	}
	else if (Name == PropertyName::StartDelay)
	{
		float startDelay = 0.0f;
		
		if (!UIUtils::ParseFloat(Value, startDelay))
			return false;
		
		mStartDelay = startDelay;
		return true;
	}
	else if (Name == PropertyName::SlideIn)
	{
		bool slide = false;
		
		if (!UIUtils::ParseBoolean(Value, slide))
			return false;
		
		mSlideIn = slide;
		return true;
	}
	else if (Name == PropertyName::RotateStart)
	{
		float rotate = 0.0f;
		
		if (!UIUtils::ParseFloat(Value, rotate))
			return false;
		
		mRotateStart = rotate;
		return true;
	}
	else if (Name == PropertyName::RotateStop)
	{
		float rotate = 0.0f;
		
		if (!UIUtils::ParseFloat(Value, rotate))
			return false;
		
		mRotateStop = rotate;
		return true;
	}
	else if (Name == PropertyName::RotateStopAndHold)
	{
		bool stopAndHold = false;
		
		if (!UIUtils::ParseBoolean(Value, stopAndHold))
			return false;
		
		mRotateStopAndHold = stopAndHold;
		return true;
	}
	else
		return UIDeformer::SetProperty(Name, Value);
}

//=============================================================================

bool UIDeformerRotate::GetProperty(const UILowerString & Name, UIString &Value) const
{
	if (Name == PropertyName::HorizontalActive)
		return UIUtils::FormatBoolean(Value, mHorizontalActive);
	else if (Name == PropertyName::HorizontalRight)
		return UIUtils::FormatBoolean(Value, mHorizontalRight);
	else if (Name == PropertyName::VerticalActive)
		return UIUtils::FormatBoolean(Value, mVerticalActive);
	else if (Name == PropertyName::VerticalDown)
		return UIUtils::FormatBoolean(Value, mVerticalDown);
	else if (Name == PropertyName::FromTheCenter)
		return UIUtils::FormatBoolean(Value, mFromTheCenter);
	else if (Name == PropertyName::Duration)
		return UIUtils::FormatFloat(Value, mDuration);
	else if (Name == PropertyName::StartDelay)
		return UIUtils::FormatFloat(Value, mStartDelay);
	else if (Name == PropertyName::SlideIn)
		return UIUtils::FormatBoolean(Value, mSlideIn);
	else if (Name == PropertyName::RotateStart)
		return UIUtils::FormatFloat(Value, mRotateStart);
	else if (Name == PropertyName::RotateStop)
		return UIUtils::FormatFloat(Value, mRotateStop);
	else if (Name == PropertyName::RotateStopAndHold)
		return UIUtils::FormatBoolean(Value, mRotateStopAndHold);
	else
		return UIDeformer::GetProperty(Name, Value);
}

//=============================================================================


bool UIDeformerRotate::Deform(UICanvas &, UIFloatPoint const * pointsIn, UIFloatPoint * pointsOut, size_t count)
{
	UIWidget * const target = GetTarget();

	if (!mTriggerEffect && mTimeSeconds >= mStartDelay)
	{
		mTriggerEffect = true;
		PlaySoundDelay();
	}

	bool shouldContinue = !mTriggerEffect;

	if (target && ((mRotateStopAndHold || mTimeSeconds < TotalTime()) && mTriggerEffect))
	{
		UIRect worldRect;
		target->GetWorldRect(worldRect);

		UISize const & canvasSize = worldRect.Size();
		UISize const & canvasLocation = worldRect.Location();
		UISize const & canvasEndLocation = canvasLocation + canvasSize;

		if (canvasSize.x > FLT_MIN && canvasSize.y > FLT_MIN) 
		{
			float const canvasWidth = static_cast<float>(canvasSize.x);
			float const canvasHeight = static_cast<float>(canvasSize.y);

			UIFloatPoint focus;
			
			if (mFromTheCenter)
			{
				focus.Set(canvasWidth * 0.5f, canvasHeight * 0.5f);
			}
			else
			{
				focus.Set(mHorizontalRight ? 0.0f : canvasWidth, mVerticalDown ? 0.0f : canvasHeight);
			}

			focus.x += canvasLocation.x;
			focus.y += canvasLocation.y;
			
			float const timeLerp = clamp(0.0f, mTimeSeconds / TotalTime(), 1.0f);
			float const rotationRadians = mSlideIn ? 0.0f : nonLinearInterpolate(c_startRotation + DegreesToRadians(mRotateStart),  c_startRotation + DegreesToRadians(mRotateStop), timeLerp);
			
			for (size_t cnt = 0; cnt < count; ++cnt, ++pointsOut, ++pointsIn)
			{
				updatePoint(timeLerp, rotationRadians, focus, pointsIn, pointsOut);
			}

			// Update the scaling stuff.
			if (mTimerDirty)
			{
				mTimerDirty = false;

				UIFloatPoint rectMin;
				UIFloatPoint const minPoint(canvasLocation);
				updatePoint(timeLerp, rotationRadians, focus, &minPoint, &rectMin);
				UIPoint rectMinCanvas(rectMin);
				
				UIFloatPoint rectMax;
				UIFloatPoint const maxPoint(canvasEndLocation);
				updatePoint(timeLerp, rotationRadians, focus, &maxPoint, &rectMax);
				UIPoint rectMaxCanvas(rectMax);
				
				float const scaleX = (rectMax.x - rectMin.x) / canvasWidth;
				float const scaleY = (rectMax.y - rectMin.y) / canvasHeight;
				mCanvasScale.Set(scaleX, scaleY);
			}
		}
	}
	else
	{
		for (size_t cnt = 0; cnt < count; ++cnt, ++pointsOut, ++pointsIn)
		{
			*pointsOut = *pointsIn;
		}
	}

	return shouldContinue;
}

//=============================================================================

UIEffector::EffectResult UIDeformerRotate::Effect(UIBaseObject *theObject)
{
	UIDeformer::Effect(theObject);
	
	EffectResult result = Continue;
	
	if (!mRotateStopAndHold && (mTimeSeconds >= TotalTime()))
	{
		result = Stop;
		SetTarget(0);
	}
	
	return result;
}

//=============================================================================

void UIDeformerRotate::Notify(UINotificationServer * server, UIBaseObject * subject, Code code)
{
	UIDeformer::Notify(server, subject, code);
	mTimerDirty = true;
	mTimeSeconds += (static_cast<float>(UIClock::gUIClock().GetLastFrameTime()) * c_updateRate);
}


//=============================================================================

void UIDeformerRotate::OnTargetChanged(UIWidget * const newTarget, UIWidget * const oldTarget)
{
	UIDeformer::OnTargetChanged(newTarget, oldTarget);
	
	if (oldTarget != newTarget)
	{
		mTimeSeconds = 0.0f;
		mTriggerEffect = false;
		mTimerDirty = true;
		
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

void UIDeformerRotate::updatePoint(float const timeLerp, float const rotationRadians, UIFloatPoint const & focus, UIFloatPoint const * const pointIn, UIFloatPoint * const pointOut) const
{
	if (mSlideIn)
	{
		if (timeLerp >= 1.0f)
		{
			*pointOut = *pointIn;
		}
		else
		{
			if (mHorizontalActive)
			{
				pointOut->x = nonLinearInterpolate(focus.x, pointIn->x, timeLerp);
			}
			else
			{
				pointOut->x = pointIn->x;
			}
			
			
			if (mVerticalActive)
			{
				pointOut->y = nonLinearInterpolate(focus.y, pointIn->y, timeLerp);
			}
			
			else
			{
				pointOut->y = pointIn->y;
			}
		}
	}
	else
	{
		UIFloatPoint const & offset = (focus - *pointIn);
		// float const distance = fastsqrt(square(offset.x) + square(offset.y));
		float const distance = sqrtf(square(offset.x) + square(offset.y));
		float const angle = rotationRadians - atan2f(offset.x, offset.y);
		
		if (mHorizontalActive)
		{
			pointOut->x = distance * cosf(angle) + focus.x;
		}
		else
		{
			pointOut->x = pointIn->x;
		}
		
		
		if (mVerticalActive)
		{
			pointOut->y = distance * sinf(angle) + focus.y;
		}
		
		else
		{
			pointOut->y = pointIn->y;
		}
	}
}

//=============================================================================

UIFloatPoint UIDeformerRotate::GetDeformedScale() const
{
	return mCanvasScale;
}

//=============================================================================
