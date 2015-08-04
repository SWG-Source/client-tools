//======================================================================
//
// UIPie.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIPie.h"

#include "UICanvas.h"
#include "UIImageStyle.h"
#include "UIPalette.h"
#include "UIPieStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <cmath>
#include <vector>


//----------------------------------------------------------------------

const char * const UIPie::TypeName = "Pie";

const UILowerString UIPie::PropertyName::PieInnerRadiusType       = UILowerString ("PieInnerRadiusType");
const UILowerString UIPie::PropertyName::PieSegmentStretchType    = UILowerString ("PieSegmentStretchType");
const UILowerString UIPie::PropertyName::PieWinding               = UILowerString ("PieWinding");
const UILowerString UIPie::PropertyName::PieInnerRadius           = UILowerString ("PieInnerRadius");
const UILowerString UIPie::PropertyName::PieInnerRadiusPackInfo   = UILowerString ("PieInnerRadiusPackInfo");
const UILowerString UIPie::PropertyName::PieAngleStart            = UILowerString ("PieAngleStart");
const UILowerString UIPie::PropertyName::PieAngleClipFinish       = UILowerString ("PieAngleClipFinish");
const UILowerString UIPie::PropertyName::PieUseTextureOnce        = UILowerString ("PieUseTextureOnce");
const UILowerString UIPie::PropertyName::PieValue                 = UILowerString ("PieValue");
const UILowerString UIPie::PropertyName::Style                    = UILowerString ("Style");
const UILowerString UIPie::PropertyName::PieColor                 = UILowerString ("PieColor");
const UILowerString UIPie::PropertyName::PieOpacity               = UILowerString ("PieOpacity");
const UILowerString UIPie::PropertyName::SquaredOff               = UILowerString ("SquaredOff");


//----------------------------------------------------------------------

const Unicode::String UIPie::PieInnerRadiusTypeNames::Center            = Unicode::narrowToWide ("Center");
const Unicode::String UIPie::PieInnerRadiusTypeNames::Fixed             = Unicode::narrowToWide ("Fixed");
const Unicode::String UIPie::PieInnerRadiusTypeNames::Proportional      = Unicode::narrowToWide ("Proportional");
const Unicode::String UIPie::PieInnerRadiusTypeNames::Absolute          = Unicode::narrowToWide ("Absolute");
const Unicode::String UIPie::PieSegmentStretchTypeNames::None           = Unicode::narrowToWide ("None");
const Unicode::String UIPie::PieSegmentStretchTypeNames::Stretch        = Unicode::narrowToWide ("Stretch");
const Unicode::String UIPie::PieSegmentStretchTypeNames::Shrink         = Unicode::narrowToWide ("Shrink");
const Unicode::String UIPie::PieSegmentStretchTypeNames::Leftover       = Unicode::narrowToWide ("Leftover");
const Unicode::String UIPie::PieWindingNames::ccw                       = Unicode::narrowToWide ("ccw");
const Unicode::String UIPie::PieWindingNames::cw                        = Unicode::narrowToWide ("cw");

//======================================================================================
#define _TYPENAME UIPie

namespace UIPieNamespace
{
	const float MY_PI = 3.1415926535897932384626433832795f;
	const float SQRT_2_PLUS_EPSILON = 1.42f;  

	typedef std::vector<UITriangle> TriangleVector;
	TriangleVector s_triangleScratchBuf;
	TriangleVector s_uvsScratchBuf;

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Style, "", T_object),
		_DESCRIPTOR(PieAngleStart, "", T_float),
		_DESCRIPTOR(PieAngleClipFinish, "", T_float),
		_DESCRIPTOR(PieValue, "", T_float),
		_DESCRIPTOR(PieSegmentStretchType, "", T_string), // ENUM
		_DESCRIPTOR(PieWinding, "", T_string), // ENUM
		_DESCRIPTOR(PieInnerRadius, "", T_int),
		_DESCRIPTOR(PieInnerRadiusType, "", T_string), // ENUM
		_DESCRIPTOR(PieInnerRadiusPackInfo, "", T_point)
	_GROUPEND(Basic, 2, 0);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(PieColor, "", T_color),
		_DESCRIPTOR(PieOpacity, "", T_float),
		_DESCRIPTOR(SquaredOff, "", T_bool)
	_GROUPEND(Appearance, 2, 1);
	//================================================================
}
using namespace UIPieNamespace;
//======================================================================================

UIPie::UIPie () :
UIWidget (),
mInnerRadiusType             (PIRT_center),
mSegmentStretchType          (PSST_none),
mWinding                     (PW_ccw),
mStyle                       (0),
mInnerRadius                 (0L),
mInnerRadiusPackInfo         (1,1),
mAngleStartRadians           (0),
mAngleClipFinishRadians      (-1.0f),
mAngleEndRadians             (1.2f),
mValue                       (0.7f),
mNumSegments                 (0),
mAnglePerSegment             (0.0f),
mLastScrollExtent            (),
mPackSizeInfoDirty           (false),
mPieColor                    (UIColor::white),
mPieOpacity                  (1.0f),
mRecomputeDirty              (true),
mSquaredOff                  (false),
mUseTextureOnce              (false)
{
}

//----------------------------------------------------------------------

UIPie::~UIPie ()
{
	SetStyle (0);
}

//----------------------------------------------------------------------

bool UIPie::IsA (const UITypeID Type) const
{
	return (Type == TUIPie || UIWidget::IsA (Type));
}

//----------------------------------------------------------------------

void UIPie::GetLinkPropertyNames  (UIPropertyNameVector & in) const
{
	in.push_back (PropertyName::Style);

	UIWidget::GetLinkPropertyNames (in);
}

//----------------------------------------------------------------------

void UIPie::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIPie::GetPropertyNames  (UIPropertyNameVector & in, bool forCopy) const
{
	UIPalette::GetPropertyNamesForType (TUIPie, in);

	in.push_back (PropertyName::PieInnerRadiusType      );
	in.push_back (PropertyName::PieSegmentStretchType   );
	in.push_back (PropertyName::PieWinding              );
	in.push_back (PropertyName::PieInnerRadius          );
	in.push_back (PropertyName::PieInnerRadiusPackInfo  );
	in.push_back (PropertyName::PieAngleStart           );
	in.push_back (PropertyName::PieAngleClipFinish      );
	in.push_back (PropertyName::PieValue                );
	in.push_back (PropertyName::Style);
	in.push_back (PropertyName::PieColor);
	in.push_back (PropertyName::PieOpacity);	
	in.push_back (PropertyName::SquaredOff);	
	in.push_back (PropertyName::PieUseTextureOnce);

	UIWidget::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UIPie::SetProperty  (const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::PieInnerRadiusType      )
	{
		PieInnerRadiusType type = PIRT_center;
		if (FindInnerRadiusType (Value, type))
		{
			SetInnerRadiusType (type);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::PieSegmentStretchType   )
	{
		PieSegmentStretchType type = PSST_none;
		if (FindSegmentStretchType (Value, type))
		{
			SetSegmentStretchType (type);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::PieWinding              )
	{
		PieWinding type = PW_ccw;
		if (FindWinding (Value, type))
		{
			SetWinding (type);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::PieInnerRadius          )
	{
		long l = 0;
		if (UIUtils::ParseLong (Value, l))
		{
			SetInnerRadius (l);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::PieInnerRadiusPackInfo  )
	{
		return false;
	}
	else if (Name == PropertyName::PieAngleStart           )
	{
		float f = 0.0f;
		if (UIUtils::ParseFloat (Value, f))
		{
			SetAngleStartRadians (f);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::PieAngleClipFinish      )
	{
		float f = 0.0f;
		if (UIUtils::ParseFloat (Value, f))
		{
			SetAngleClipFinishRadians (f);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::PieValue                )
	{
		float f = 0.0f;
		if (UIUtils::ParseFloat (Value, f))
		{
			SetValue (f);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::Style)
	{
		UIPieStyle * const style = static_cast<UIPieStyle *>(GetObjectFromPath (Value, TUIPieStyle));
		if (style || Value.empty ())
		{
			SetStyle (style);
			RemoveProperty (Name);
			return true;
		}

		//-- fall through
	}

	else if (Name == PropertyName::PieColor    )
	{
		return UIUtils::ParseColor (Value, mPieColor);
	}

	else if (Name == PropertyName::PieOpacity  )
	{
		return UIUtils::ParseFloat (Value, mPieOpacity);
	}
	else if (Name == PropertyName::SquaredOff  )
	{
		return UIUtils::ParseBoolean (Value, mSquaredOff);
	}
	else if (Name == PropertyName::PieUseTextureOnce  )
	{
		return UIUtils::ParseBoolean (Value, mUseTextureOnce);
	}
	else
	{
		UIPalette::SetPropertyForObject (*this, Name, Value);
	}

	return UIWidget::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UIPie::GetProperty (const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::PieInnerRadiusType      )
	{
		Value = FindInnerRadiusTypeName (mInnerRadiusType);
		return true;
	}
	else if (Name == PropertyName::PieSegmentStretchType   )
	{
		Value = FindSegmentStretchTypeName (mSegmentStretchType);
		return true;
	}
	else if (Name == PropertyName::PieWinding              )
	{
		Value = FindWindingName (mWinding);
		return true;
	}
	else if (Name == PropertyName::PieInnerRadius          )
	{
		return UIUtils::FormatLong (Value, mInnerRadius);
	}
	else if (Name == PropertyName::PieInnerRadiusPackInfo  )
	{
		return UIUtils::FormatPoint (Value, mInnerRadiusPackInfo);
	}
	else if (Name == PropertyName::PieAngleStart           )
	{
		return UIUtils::FormatFloat (Value, mAngleStartRadians);
	}
	else if (Name == PropertyName::PieAngleClipFinish      )
	{
		return UIUtils::FormatFloat (Value, mAngleClipFinishRadians);
	}
	else if (Name == PropertyName::PieValue                )
	{
		return UIUtils::FormatFloat (Value, mValue);
	}
	else if (Name == PropertyName::Style)
	{
		if (mStyle)
		{
			GetPathTo (Value, mStyle);
			return true;
		}
		//-- fall through
	}
	else if (Name == PropertyName::PieColor    )
	{
		return UIUtils::FormatColor (Value, mPieColor);
	}

	else if (Name == PropertyName::PieOpacity  )
	{
		return UIUtils::FormatFloat (Value, mPieOpacity);
	}

	else if (Name == PropertyName::SquaredOff  )
	{
		return UIUtils::FormatBoolean(Value, mSquaredOff);
	}

	else if (Name == PropertyName::PieUseTextureOnce  )
	{
		return UIUtils::FormatBoolean(Value, mUseTextureOnce);
	}

	return UIWidget::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

UIStyle * UIPie::GetStyle () const
{
	return mStyle;
}

//----------------------------------------------------------------------

bool UIPie::ProcessMessage (const UIMessage & msg)
{
	return UIWidget::ProcessUserMessage (msg);
}

//----------------------------------------------------------------------

void UIPie::Render (UICanvas & canvas) const
{
	if (mRecomputeDirty)
		Recompute ();

	UIWidget::Render (canvas);

	UISize scrollExtent;
	GetScrollExtent (scrollExtent);

	if (mPackSizeInfoDirty)
	{
		const_cast<UIPie *>(this)->ResetPackSizes ();
		mPackSizeInfoDirty = false;
	}

	if (mLastScrollExtent != scrollExtent)
	{
		mLastScrollExtent = scrollExtent;
		const_cast<UIPie *>(this)->UpdateFromPackInfo ();
	}

	canvas.ModifyOpacity (mPieOpacity);
	canvas.ModifyColor   (mPieColor);

	if (mInnerRadiusType == PIRT_center)
	{
		RenderCenter (canvas);
	}
	else
		RenderRing   (canvas);
}

//----------------------------------------------------------------------

void UIPie::RenderCenter (UICanvas & canvas) const
{
	if (!mStyle || mNumSegments == 0)
		return;

	const UIImageStyle * const imageStyle = mStyle->GetImageStyle ();

	UISize scrollExtent;
	GetScrollExtent (scrollExtent);

	const UIFloatPoint centerPoint (static_cast<float>(scrollExtent.x / 2L), static_cast<float>(scrollExtent.y / 2L));
	float outerRadius = static_cast<float>(std::min (scrollExtent.x, scrollExtent.y) / 2L);
	const float saveOuterRadius = outerRadius;
	if(mSquaredOff)
		outerRadius *= SQRT_2_PLUS_EPSILON;  

	const UIFloatPoint * const fourUvs = mStyle->GetUvs ();

	float angle = mAngleStartRadians;

	UIFloatPoint lastOuterPoint = centerPoint;
	lastOuterPoint.x += static_cast<float>(outerRadius * cos (mAngleStartRadians));
	lastOuterPoint.y += static_cast<float>(outerRadius * sin (mAngleStartRadians));

	if(mSquaredOff)
	{
		lastOuterPoint.x = std::max(lastOuterPoint.x, 0.0f);
		lastOuterPoint.x = std::min(lastOuterPoint.x, centerPoint.x + saveOuterRadius);
		lastOuterPoint.y = std::max(lastOuterPoint.y, 0.0f);
		lastOuterPoint.y = std::min(lastOuterPoint.y, centerPoint.y + saveOuterRadius);
	}
	
	lastOuterPoint = canvas.Transform (lastOuterPoint);

	UITriangle tri;
	tri.p1 = canvas.Transform (centerPoint);

	UITriangle uvs;
	uvs.p1.y = fourUvs [0].y;
	uvs.p1.x = (fourUvs [1].x + fourUvs [2].x) * 0.5f;

	s_triangleScratchBuf.clear ();
	s_uvsScratchBuf.clear ();
	s_triangleScratchBuf.reserve (mNumSegments);
	s_uvsScratchBuf.reserve (mNumSegments);

	for (int i = 0; i < mNumSegments; ++i)
	{
		angle -= mAnglePerSegment;
		if(mAngleClipFinishRadians >= 0.0f)
		{		
			if(mAnglePerSegment > 0.0f)
			{
				if(angle < mAngleClipFinishRadians)
					angle = mAngleClipFinishRadians;
			}
			else
			{
				if(angle > mAngleClipFinishRadians)
					angle = mAngleClipFinishRadians;
			}
		}

		UIFloatPoint outerPoint = centerPoint;
		outerPoint.x += static_cast<float>(outerRadius * cos (angle));
		outerPoint.y += static_cast<float>(outerRadius * sin (angle));
		if(mSquaredOff)
		{
			outerPoint.x = std::max(outerPoint.x, 0.0f);
			outerPoint.x = std::min(outerPoint.x, centerPoint.x + saveOuterRadius);
			outerPoint.y = std::max(outerPoint.y, 0.0f);
			outerPoint.y = std::min(outerPoint.y, centerPoint.y + saveOuterRadius);
		}
		outerPoint = canvas.Transform (outerPoint);

		if (mWinding != PW_ccw)
		{
			tri.p2 = lastOuterPoint;
			tri.p3 = outerPoint;

			uvs.p2 = fourUvs [2];
			uvs.p3 = fourUvs [1];
		}
		else
		{
			tri.p2 = outerPoint;
			tri.p3 = lastOuterPoint;

			uvs.p2 = fourUvs [1];
			uvs.p3 = fourUvs [2];
		}
		if(mUseTextureOnce)
		{		
			float xStart = uvs.p2.x;
			float xDistance = (uvs.p3.x - uvs.p2.x) / mNumSegments;
			uvs.p2.x = xStart + xDistance * i;
			uvs.p3.x = xStart + xDistance * (i + 1);
		}
				

		lastOuterPoint = outerPoint;

		s_triangleScratchBuf.push_back (tri);
		s_uvsScratchBuf.push_back (uvs);
	}

	canvas.RenderTriangles (imageStyle->GetSourceCanvas (), mNumSegments, s_triangleScratchBuf.begin (), s_uvsScratchBuf.begin ());
}

//----------------------------------------------------------------------

void UIPie::RenderRing (UICanvas & canvas) const
{
	if (!mStyle || mNumSegments == 0)
		return;

	const UIImageStyle * const imageStyle = mStyle->GetImageStyle ();

	if (!imageStyle)
		return;

	UISize scrollExtent;
	GetScrollExtent (scrollExtent);

	const UIFloatPoint centerPoint (static_cast<float>(scrollExtent.x / 2L), static_cast<float>(scrollExtent.y / 2L));

	float outerRadius = static_cast<float>(std::min (scrollExtent.x, scrollExtent.y) / 2L);
	const float saveOuterRadius = outerRadius;
	const float innerRadius = static_cast<float>(mInnerRadius);

	if(mSquaredOff)
		outerRadius *= SQRT_2_PLUS_EPSILON;  

	const UIFloatPoint * const fourUvs = mStyle->GetUvs ();

	float angle = mAngleStartRadians;

	const float cos_angleStart = static_cast<float>(cos (mAngleStartRadians));
	const float sin_angleStart = static_cast<float>(sin (mAngleStartRadians));

	UIFloatPoint lastOuterPoint = centerPoint;
	lastOuterPoint.x += static_cast<float>(outerRadius * cos_angleStart);
	lastOuterPoint.y += static_cast<float>(outerRadius * sin_angleStart);
	lastOuterPoint = canvas.Transform (lastOuterPoint);
	
	if(mSquaredOff)
	{
		lastOuterPoint.x = std::max(lastOuterPoint.x, 0.0f);
		lastOuterPoint.x = std::min(lastOuterPoint.x, centerPoint.x + saveOuterRadius);
		lastOuterPoint.y = std::max(lastOuterPoint.y, 0.0f);
		lastOuterPoint.y = std::min(lastOuterPoint.y, centerPoint.y + saveOuterRadius);
	}
	
	UIFloatPoint lastInnerPoint = centerPoint;
	lastInnerPoint.x += static_cast<float>(innerRadius * cos_angleStart);
	lastInnerPoint.y += static_cast<float>(innerRadius * sin_angleStart);
	lastInnerPoint = canvas.Transform (lastInnerPoint);

	UITriangle tri;
	UITriangle uvs;

	s_triangleScratchBuf.clear ();
	s_uvsScratchBuf.clear ();
	s_triangleScratchBuf.reserve (mNumSegments);
	s_uvsScratchBuf.reserve (mNumSegments);

	static UIFloatPoint fourActualUvs[4];
	for(int c = 0; c < 4; c++)
		fourActualUvs[c] = fourUvs[c];
	for (int i = 0; i < mNumSegments; ++i)
	{
		angle -= mAnglePerSegment;
		if((mSegmentStretchType == PSST_leftover) && (i == (mNumSegments - 1)) )
			angle = mAngleStartRadians + (mAngleStartRadians - mAngleEndRadians);
		
		if(mAngleClipFinishRadians >= 0.0f)
		{		
			if(mAnglePerSegment > 0.0f)
			{
				if(angle < mAngleClipFinishRadians)
					angle = mAngleClipFinishRadians;
			}
			else
			{
				if(angle > mAngleClipFinishRadians)
					angle = mAngleClipFinishRadians;
			}
		}

		const float cos_angle = static_cast<float>(cos (angle));
		const float sin_angle = static_cast<float>(sin (angle));

		UIFloatPoint outerPoint = centerPoint;
		outerPoint.x += static_cast<float>(outerRadius * cos_angle);
		outerPoint.y += static_cast<float>(outerRadius * sin_angle);
		outerPoint = canvas.Transform (outerPoint);
	
		if(mSquaredOff)
		{
			outerPoint.x = std::max(outerPoint.x, 0.0f);
			outerPoint.x = std::min(outerPoint.x, centerPoint.x + saveOuterRadius);
			outerPoint.y = std::max(outerPoint.y, 0.0f);
			outerPoint.y = std::min(outerPoint.y, centerPoint.y + saveOuterRadius);
		}

		UIFloatPoint innerPoint = centerPoint;
		innerPoint.x += static_cast<float>(innerRadius * cos_angle);
		innerPoint.y += static_cast<float>(innerRadius * sin_angle);
		innerPoint = canvas.Transform (innerPoint);
			

		if(mUseTextureOnce)
		{		
			float xStart = fourUvs[1].x;
			float xDistance = (fourUvs[2].x - fourUvs[1].x) / mNumSegments;
			if (mWinding == PW_cw)
			{
				fourActualUvs[0].x = xStart + xDistance * i;
				fourActualUvs[1].x = xStart + xDistance * i;
				fourActualUvs[2].x = xStart + xDistance * (i + 1);
				fourActualUvs[3].x = xStart + xDistance * (i + 1);
			}
			else
			{
				fourActualUvs[2].x = xStart + xDistance * i;
				fourActualUvs[3].x = xStart + xDistance * i;
				fourActualUvs[0].x = xStart + xDistance * (i + 1);
				fourActualUvs[1].x = xStart + xDistance * (i + 1);
			}
		}
			
		if (mWinding == PW_ccw)
		{
			tri.p1 = lastInnerPoint;
			tri.p3 = lastOuterPoint;
			tri.p2 = outerPoint;
			uvs.p1 = fourActualUvs [3];
			uvs.p3 = fourActualUvs [2];
			uvs.p2 = fourActualUvs [1];

			s_triangleScratchBuf.push_back (tri);
			s_uvsScratchBuf.push_back (uvs);

			tri.p3 = outerPoint;
			tri.p2 = innerPoint;
			uvs.p3 = fourActualUvs [1];
			uvs.p2 = fourActualUvs [0];

			s_triangleScratchBuf.push_back (tri);
			s_uvsScratchBuf.push_back (uvs);
		}
		else
		{
			tri.p1 = lastInnerPoint;
			tri.p3 = outerPoint;
			tri.p2 = lastOuterPoint;
			uvs.p1 = fourActualUvs [0];
			uvs.p3 = fourActualUvs [2];
			uvs.p2 = fourActualUvs [1];

			s_triangleScratchBuf.push_back (tri);
			s_uvsScratchBuf.push_back (uvs);

			tri.p3 = innerPoint;
			tri.p2 = outerPoint;
			uvs.p3 = fourActualUvs [3];
			uvs.p2 = fourActualUvs [2];

			s_triangleScratchBuf.push_back (tri);
			s_uvsScratchBuf.push_back (uvs);
		}
		

		
		lastOuterPoint = outerPoint;
		lastInnerPoint = innerPoint;
	}

	canvas.RenderTriangles (imageStyle->GetSourceCanvas (), mNumSegments * 2, s_triangleScratchBuf.begin (), s_uvsScratchBuf.begin ());
}

//----------------------------------------------------------------------

void UIPie::Link ()
{
	UIWidget::Link ();

	if (mStyle)
	{
		mStyle->Link ();
		mRecomputeDirty = true;
	}
}

//----------------------------------------------------------------------

void UIPie::SetStyle (UIPieStyle * style)
{
	if (style != mStyle)
	{
		if (mStyle)
			mStyle->StopListening (this);

		if (style)
			style->Listen (this);
	}

	if (AttachMember (mStyle, style))
		mRecomputeDirty = true;
}

//----------------------------------------------------------------------

void UIPie::Notify (UINotificationServer *notifyingObject, UIBaseObject *, UINotification::Code )
{
	if (notifyingObject == mStyle)
	{
		mRecomputeDirty = true;
	}
}

//----------------------------------------------------------------------

void UIPie::SetAngleStartRadians (float rad)
{
	if (mAngleStartRadians != rad)
	{
		mAngleStartRadians = rad;
		mRecomputeDirty = true;
	}
}

//----------------------------------------------------------------------

void UIPie::SetAngleClipFinishRadians(float rad)
{
	mAngleClipFinishRadians = rad;
}

//----------------------------------------------------------------------

void UIPie::SetValue (float value)
{
	if (mValue != value)
	{
		mValue = value;
		mRecomputeDirty = true;
	}
}

//----------------------------------------------------------------------

void UIPie::SetInnerRadius               (long r, bool resetPacking)
{
	if (mInnerRadius != r)
	{
		mInnerRadius = r;
		if (resetPacking)
			mPackSizeInfoDirty = true;

		mRecomputeDirty = true;
	}
}

//----------------------------------------------------------------------

void  UIPie::Recompute () const
{
	mRecomputeDirty = false;

	if (mWinding == PW_ccw)
	{
		mAngleEndRadians = mAngleStartRadians + (MY_PI * 2.0f * mValue);
	}
	else
		mAngleEndRadians = mAngleStartRadians - (MY_PI * 2.0f * mValue);

	if (mStyle)
	{
		const float desiredAnglePerSegment = mStyle->GetImageStyleAngleRadians ();

		if (desiredAnglePerSegment > 0.0f)
		{
			const float desiredAngleDelta = mAngleEndRadians - mAngleStartRadians;
			if (mSegmentStretchType == PSST_none)
			{
				mNumSegments = static_cast<int>(fabs (desiredAngleDelta) / desiredAnglePerSegment);
				if((mNumSegments == 0) && (fabs(desiredAngleDelta) > 0.0001f))
					mNumSegments = 1;
				mAnglePerSegment = (desiredAngleDelta < 0.0f ? -1.0f: 1.0f) * desiredAnglePerSegment;
			}
			else if (mSegmentStretchType == PSST_stretch)
			{
				mNumSegments = static_cast<int>(fabs (desiredAngleDelta) / desiredAnglePerSegment);
				if((mNumSegments == 0) && (fabs(desiredAngleDelta) > 0.0001f))
					mNumSegments = 1;
				mAnglePerSegment = desiredAngleDelta / static_cast<float>(mNumSegments);
			}
			else if (mSegmentStretchType == PSST_shrink)
			{
				mNumSegments = static_cast<int>(fabs (desiredAngleDelta) / desiredAnglePerSegment) + 1;
				mAnglePerSegment = desiredAngleDelta / static_cast<float>(mNumSegments);
			}
			else if (mSegmentStretchType == PSST_leftover)
			{
				mNumSegments = static_cast<int>(fabs (desiredAngleDelta) / desiredAnglePerSegment) + 1;								
				mAnglePerSegment = (desiredAngleDelta < 0.0f ? -1.0f: 1.0f) * desiredAnglePerSegment;
			}
		}
	}
}


//----------------------------------------------------------------------

bool UIPie::FindInnerRadiusType    (const Unicode::String & str, PieInnerRadiusType & type)
{
	const int len = static_cast<int>(str.size ());
	if (Unicode::caseInsensitiveCompare (str, PieInnerRadiusTypeNames::Center, 0, len))
		type = PIRT_center;
	else if (Unicode::caseInsensitiveCompare (str, PieInnerRadiusTypeNames::Fixed, 0, len))
		type = PIRT_fixed;
	else if (Unicode::caseInsensitiveCompare (str, PieInnerRadiusTypeNames::Proportional, 0, len))
		type = PIRT_proportional;
	else if (Unicode::caseInsensitiveCompare (str, PieInnerRadiusTypeNames::Absolute, 0, len))
		type = PIRT_absolute;
	else
		return false;
	return true;
}

//----------------------------------------------------------------------

bool UIPie::FindSegmentStretchType (const Unicode::String & str, PieSegmentStretchType & type)
{
	const int len = static_cast<int>(str.size ());
	if (Unicode::caseInsensitiveCompare (str, PieSegmentStretchTypeNames::None, 0, len))
		type = PSST_none;
	else if (Unicode::caseInsensitiveCompare (str, PieSegmentStretchTypeNames::Stretch, 0, len))
		type = PSST_stretch;
	else if (Unicode::caseInsensitiveCompare (str, PieSegmentStretchTypeNames::Shrink, 0, len))
		type = PSST_shrink;
	else if (Unicode::caseInsensitiveCompare (str, PieSegmentStretchTypeNames::Leftover, 0, len))
		type = PSST_leftover;
	else
		return false;

	return true;
}

//----------------------------------------------------------------------

bool UIPie::FindWinding            (const Unicode::String & str, PieWinding & type)
{
	const int len = static_cast<int>(str.size ());
	if (Unicode::caseInsensitiveCompare (str, PieWindingNames::ccw, 0, len))
		type = PW_ccw;
	else if (Unicode::caseInsensitiveCompare (str, PieWindingNames::cw, 0, len))
		type = PW_cw;
	else
		return false;

	return true;
}

//----------------------------------------------------------------------

const Unicode::String & UIPie::FindInnerRadiusTypeName      (PieInnerRadiusType type)
{
	switch (type)
	{
	case PIRT_center:
		return PieInnerRadiusTypeNames::Center;
	case PIRT_fixed:
		return PieInnerRadiusTypeNames::Fixed;
	case PIRT_proportional:
		return PieInnerRadiusTypeNames::Proportional;
	default:
		return PieInnerRadiusTypeNames::Absolute;
	}
}

//----------------------------------------------------------------------

const Unicode::String & UIPie::FindSegmentStretchTypeName   (PieSegmentStretchType type)
{
	switch (type)
	{
	case PSST_none:
		return PieSegmentStretchTypeNames::None;
	case PSST_stretch:
		return PieSegmentStretchTypeNames::Stretch;
	case PSST_leftover:
		return PieSegmentStretchTypeNames::Leftover;
	default:
		return PieSegmentStretchTypeNames::Shrink;
	}
}

//----------------------------------------------------------------------

const Unicode::String & UIPie::FindWindingName       (PieWinding type)
{
	switch (type)
	{
	case PW_ccw:
		return PieWindingNames::ccw;
	default:
		return PieWindingNames::cw;
	}
}

//----------------------------------------------------------------------

void UIPie::SetInnerRadiusType (PieInnerRadiusType type)
{
	if (mInnerRadiusType != type)
	{
		mInnerRadiusType = type;
		mPackSizeInfoDirty = true;
		mRecomputeDirty = true;
	}
}

//----------------------------------------------------------------------

void UIPie::SetSegmentStretchType (PieSegmentStretchType type)
{
	if (mSegmentStretchType != type)
	{
		mSegmentStretchType = type;
		mRecomputeDirty = true;
	}
}

//----------------------------------------------------------------------

void  UIPie::SetWinding  (PieWinding type)
{
	if (mWinding != type)
	{
		mWinding = type;
		mRecomputeDirty = true;
	}
}

//----------------------------------------------------------------------

void UIPie::ResetPackSizes ()
{
	switch (mInnerRadiusType)
	{
	case PIRT_center:
		mInnerRadiusPackInfo.x = 1L;
		mInnerRadiusPackInfo.y = 1L;
		break;
	case PIRT_fixed:
		mInnerRadiusPackInfo.x = mInnerRadius;
		mInnerRadiusPackInfo.y = 1L;
		break;
	case PIRT_absolute:
		{
			UISize scrollExtent;
			GetScrollExtent (scrollExtent);
			const long outerRadius = std::min (scrollExtent.x, scrollExtent.y) / 2L;
			mInnerRadiusPackInfo.x = outerRadius - mInnerRadius;
			mInnerRadiusPackInfo.y = 1L;
		}
		break;
	case PIRT_proportional:
		{
			UISize scrollExtent;
			GetScrollExtent (scrollExtent);
			const long outerRadius = std::min (scrollExtent.x, scrollExtent.y) / 2L;
			mInnerRadiusPackInfo.x = mInnerRadius;
			mInnerRadiusPackInfo.y = outerRadius;
		}
		break;
	}
}

//----------------------------------------------------------------------

void UIPie::UpdateFromPackInfo  ()
{
	switch (mInnerRadiusType)
	{
	case PIRT_center:
		SetInnerRadius (0L, false);
		break;
	case PIRT_fixed:
		SetInnerRadius (mInnerRadiusPackInfo.x, false);
		break;
	case PIRT_absolute:
		{
			UISize scrollExtent;
			GetScrollExtent (scrollExtent);
			const long outerRadius = std::min (scrollExtent.x, scrollExtent.y) / 2L;
			SetInnerRadius (outerRadius - mInnerRadiusPackInfo.x, false);
		}
		break;
	case PIRT_proportional:
		{
			if (mInnerRadiusPackInfo.y)
			{
				UISize scrollExtent;
				GetScrollExtent (scrollExtent);
				const long outerRadius = std::min (scrollExtent.x, scrollExtent.y) / 2L;
				SetInnerRadius (outerRadius * mInnerRadiusPackInfo.x / mInnerRadiusPackInfo.y, false);
			}
		}
		break;
	}
}

//======================================================================
