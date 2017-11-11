//======================================================================
//
// UIPieStyle.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIPieStyle.h"

#include "UIImageStyle.h"
#include "UICanvas.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <cmath>
#include <vector>

//======================================================================

const char * const UIPieStyle::TypeName = "PieStyle";

const UILowerString UIPieStyle::PropertyName::ImageStyle           = UILowerString ("ImageStyle");
const UILowerString UIPieStyle::PropertyName::ImageBottomWidth     = UILowerString ("ImageBottomWidth");
const UILowerString UIPieStyle::PropertyName::ImageSegmentAngle    = UILowerString ("ImageSegmentAngle");
const UILowerString UIPieStyle::PropertyName::ImageUvs             = UILowerString ("ImageUvs");
const UILowerString UIPieStyle::PropertyName::ImageAutoAngle       = UILowerString ("ImageAutoAngle");

//======================================================================================
#define _TYPENAME UIPieStyle

namespace UIPieStyleNamespace
{

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(ImageStyle, "", T_object),
		_DESCRIPTOR(ImageBottomWidth, "", T_int),
		_DESCRIPTOR(ImageSegmentAngle, "", T_float),
		_DESCRIPTOR(ImageAutoAngle, "", T_bool)
	_GROUPEND(Appearance, 3, 0);
	//================================================================
}
using namespace UIPieStyleNamespace;
//======================================================================================

UIPieStyle::UIPieStyle () :
UIWidgetStyle           (),
mImageStyle             (0),
mImageStyleAngleRadians (0),
mImageBottomWidth       (0L),
mImageAutoAngle         (true),
mImageStyleAngleDirty   (false)
{
	mUvs [0] = mUvs [1] = mUvs [2] = mUvs [3] = UIFloatPoint::zero;
}

//----------------------------------------------------------------------

UIPieStyle::~UIPieStyle ()
{
	SetImageStyle (0);
}

//----------------------------------------------------------------------

bool UIPieStyle::IsA         (const UITypeID type) const
{
	return type == TUIPieStyle || UIWidgetStyle::IsA (type);
}

//----------------------------------------------------------------------

void UIPieStyle::GetLinkPropertyNames ( UIPropertyNameVector & in) const
{
	in.push_back (PropertyName::ImageStyle        );

	UIWidgetStyle::GetLinkPropertyNames (in);
}

//----------------------------------------------------------------------

void UIPieStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIPieStyle::GetPropertyNames ( UIPropertyNameVector & in, bool forCopy) const
{
	in.push_back (PropertyName::ImageStyle        );
	in.push_back (PropertyName::ImageBottomWidth  );
	in.push_back (PropertyName::ImageSegmentAngle );
	in.push_back (PropertyName::ImageUvs          );
	in.push_back (PropertyName::ImageAutoAngle );

	UIWidgetStyle::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UIPieStyle::SetProperty      ( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::ImageStyle)
	{
		UIImageStyle * const style = static_cast<UIImageStyle *>(GetObjectFromPath (Value, TUIImageStyle));
		if (style || Value.empty ())
		{
			SetImageStyle (style);
			RemoveProperty (Name);
			return true;
		}
		//- fall through
	}
	else if (Name == PropertyName::ImageBottomWidth)
	{
		long bw = 0L;
		if (UIUtils::ParseLong (Value, bw))
		{
			SetImageBottomWidth (bw);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::ImageSegmentAngle)
	{
		if (!mImageAutoAngle)
		{
			if (UIUtils::ParseFloat (Value, mImageStyleAngleRadians))
			{
				mImageStyleAngleDirty = true;
				return true;
			}
		}
		return false;
	}
	else if (Name == PropertyName::ImageUvs)
		return false;
	else if (Name == PropertyName::ImageAutoAngle)
	{
		if (UIUtils::ParseBoolean (Value, mImageAutoAngle))
		{
			mImageStyleAngleDirty = true;
			return true;
		}
		return false;
	}

	return UIWidgetStyle::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UIPieStyle::GetProperty      ( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::ImageStyle)
	{
		if (mImageStyle)
		{
			GetPathTo (Value, mImageStyle);
			return true;
		}
		//-- fall through
	}
	else if (Name == PropertyName::ImageBottomWidth)
	{
		return UIUtils::FormatLong (Value, mImageBottomWidth);
	}
	else if (Name == PropertyName::ImageSegmentAngle)
	{
		return UIUtils::FormatFloat (Value, mImageStyleAngleRadians);
	}
	else if (Name == PropertyName::ImageUvs)
	{
		Unicode::String tmpStr;
		UIUtils::FormatFloatPoint (tmpStr, mUvs [0]);
		Value += tmpStr;
		Value.append (1, ';');
		UIUtils::FormatFloatPoint (tmpStr, mUvs [1]);
		Value += tmpStr;
		Value.append (1, ';');
		UIUtils::FormatFloatPoint (tmpStr, mUvs [2]);
		Value += tmpStr;
		Value.append (1, ';');
		UIUtils::FormatFloatPoint (tmpStr, mUvs [3]);
		Value += tmpStr;
		return true;
	}
	else if (Name == PropertyName::ImageAutoAngle)
	{
		return UIUtils::FormatBoolean (Value, mImageAutoAngle);
	}

	return UIWidgetStyle::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

void UIPieStyle::Link ()
{
	UIWidgetStyle::Link ();

	if (mImageStyle)
	{
		mImageStyle->Link ();
//		RecomputeImageStyleAngle ();
	}
}

//----------------------------------------------------------------------

void UIPieStyle::SetImageStyle (UIImageStyle * style)
{
	if (AttachMember (mImageStyle, style))
	{
		SetImageBottomWidth (mImageBottomWidth);
		mImageStyleAngleDirty = true;
	}
}

//----------------------------------------------------------------------

void UIPieStyle::RecomputeImageStyleAngle () const
{
	mImageStyleAngleDirty = false;

	if (mImageStyle)
	{
		const float width  = static_cast<float>(abs (mImageStyle->GetWidth  ()));
		const float height = static_cast<float>(abs (mImageStyle->GetHeight ()));
		const float bottom = static_cast<float>(mImageBottomWidth);

		const float adjacent = height;
		float opposite = (width - bottom) * 0.5f;

		if (width == bottom)
			opposite = width / 2L;

		if (mImageAutoAngle)
			mImageStyleAngleRadians = 2.0f * static_cast<float>(atan2 (opposite, adjacent));

		const UICanvas * const sourceCanvas = mImageStyle->GetSourceCanvas ();

		if (sourceCanvas)
		{
			const UIRect &         sourceRect      = mImageStyle->GetSourceRect ();
			const float deltaWidthBottom = static_cast<float>((sourceRect.Width () - mImageBottomWidth) / 2L);
			UISize canvasSize;
			sourceCanvas->GetSize (canvasSize);
			const UIFloatPoint fCanvasSize (canvasSize);

			if (fCanvasSize.x > 0.0f && fCanvasSize.y > 0.0f)
			{
				const UIFloatPoint bottomPointLeft  (static_cast<float>(sourceRect.left)  + deltaWidthBottom, static_cast<float>(sourceRect.bottom));
				const UIFloatPoint bottomPointRight (static_cast<float>(sourceRect.right) - deltaWidthBottom, bottomPointLeft.y);

				//-- uvs go from bottom left, top left, top right, bottom right

				mUvs [0].x = bottomPointLeft.x / fCanvasSize.x;
				mUvs [0].y = bottomPointLeft.y / fCanvasSize.y;

				const UIFloatPoint topPointLeft (static_cast<float>(sourceRect.left), static_cast<float>(sourceRect.top));
				mUvs [1].x = topPointLeft.x / fCanvasSize.x;
				mUvs [1].y = topPointLeft.y / fCanvasSize.y;

				const UIFloatPoint topPointRight (static_cast<float>(sourceRect.right), static_cast<float>(sourceRect.top));
				mUvs [2].x = topPointRight.x / fCanvasSize.x;
				mUvs [2].y = mUvs [1].y;

				mUvs [3].x = bottomPointRight.x / fCanvasSize.x;
				mUvs [3].y = mUvs [0].y;
			}
		}
	}
	else if (mImageAutoAngle)
		mImageStyleAngleRadians = 0.0f;

	const_cast<UIPieStyle *>(this)->FireStyleChanged ();
}

//----------------------------------------------------------------------

void UIPieStyle::SetImageBottomWidth (long l)
{
	if (mImageStyle)
	{
		const long width  = abs (mImageStyle->GetWidth  ());

		mImageBottomWidth = std::max (0L, std::min (width, l));
	}
	else
		mImageBottomWidth = l;

	mImageStyleAngleDirty = true;
}

//======================================================================
