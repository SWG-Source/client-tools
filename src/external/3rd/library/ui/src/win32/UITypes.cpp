//======================================================================
//
// UITypes.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UITypes.h"
#include <cmath>
#include <limits>

//======================================================================

const UIRect UIRect::zero;
const UIRect UIRect::empty(std::numeric_limits<UIScalar>::max(), std::numeric_limits<UIScalar>::max(), -std::numeric_limits<UIScalar>::max(), -std::numeric_limits<UIScalar>::max());

//----------------------------------------------------------------------

const UIColor UIColor::white   (0xff,0xff,0xff);
const UIColor UIColor::black   (0x00,0x00,0x00);
const UIColor UIColor::red     (0xff,0x00,0x00);
const UIColor UIColor::yellow  (0xff,0xff,0x00);
const UIColor UIColor::green   (0x00,0xff,0x00);
const UIColor UIColor::cyan    (0x00,0xff,0xff);
const UIColor UIColor::blue    (0x00,0x00,0xff);
const UIColor UIColor::violet  (0xff,0x00,0xff);

//----------------------------------------------------------------------

UIColor UIColor::lerp (const UIColor & start, const UIColor & end, float f)
{
	const float diff [4] = 
	{
		static_cast<float>(end.r) - static_cast<float>(start.r),
		static_cast<float>(end.g) - static_cast<float>(start.g),
		static_cast<float>(end.b) - static_cast<float>(start.b),
		static_cast<float>(end.a) - static_cast<float>(start.a)
	};

	UIColor result;
	result.r = static_cast<unsigned char>(std::max (0.0f, std::min (255.0f, static_cast<float>(start.r) + diff [0] * f)));
	result.g = static_cast<unsigned char>(std::max (0.0f, std::min (255.0f, static_cast<float>(start.g) + diff [1] * f)));
	result.b = static_cast<unsigned char>(std::max (0.0f, std::min (255.0f, static_cast<float>(start.b) + diff [2] * f)));
	result.a = static_cast<unsigned char>(std::max (0.0f, std::min (255.0f, static_cast<float>(start.a) + diff [3] * f)));

	return result;
}

//----------------------------------------------------------------------

const UIFloatPoint UIFloatPoint::zero (0.0f, 0.0f);
const UIFloatPoint UIFloatPoint::one  (1.0f, 1.0f);

//-----------------------------------------------------------------

float    UIFloatPoint::Magnitude () const
{
	return static_cast<float>(sqrt( x * x + y * y ));
};

//-----------------------------------------------------------------

float UIFloatPoint::DistanceSquared (const UIFloatPoint & rhs) const
{
	return (rhs - *this).Magnitude ();
}

//----------------------------------------------------------------------

UIPoint::UIPoint(UIFloatPoint const & floatPoint)
{
	Set(static_cast<UIScalar>(floatPoint.x), static_cast<UIScalar>(floatPoint.y));
}

//----------------------------------------------------------------------

const UIPoint UIPoint::zero (0L,0L);
const UIPoint UIPoint::one  (1L,1L);

//----------------------------------------------------------------------

float UIPoint::Magnitude () const
{
	return static_cast<float>(sqrt( static_cast<double>(x * x + y * y)));//lint !e790 // suspicious truncation integral to float
}

//-----------------------------------------------------------------

float UIPoint::DistanceSquared (const UIPoint & rhs) const
{
	return (rhs - *this).Magnitude ();
}

//----------------------------------------------------------------------

UIPoint & UIPoint::operator *= (const UIFloatPoint & rhs)
{
	x = static_cast<UIScalar>(x *  rhs.x);
	y = static_cast<UIScalar>(y *  rhs.y);

	return *this;
}

//----------------------------------------------------------------------

UIPoint UIPoint::lerp (const UIPoint & start, const UIPoint & end, float t)
{
	UIPoint result;

	result.x = start.x + static_cast<long>((end.x - start.x) * t);
	result.y = start.y + static_cast<long>((end.y - start.y) * t);

	return result;
}

//----------------------------------------------------------------------

UIFloatPoint UIFloatPoint::lerp (const UIFloatPoint & start, const UIFloatPoint & end, float t)
{
	UIFloatPoint result;

	result.x = start.x + ((end.x - start.x) * t);
	result.y = start.y + ((end.y - start.y) * t);

	return result;
}

//----------------------------------------------------------------------

void UIRect::Extend(UIRect const & rect)
{
	left = std::min(left, std::min(rect.left, rect.right));
	right = std::max(right, std::max(rect.left, rect.right));
	top = std::min(top, std::min(rect.top, rect.bottom));
	bottom = std::max(bottom, std::max(rect.top, rect.bottom));
}


//======================================================================
