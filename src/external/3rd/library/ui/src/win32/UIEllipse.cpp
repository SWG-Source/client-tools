#include "_precompile.h"
#include "UIEllipse.h"

#include "UICanvas.h"
#include "UIImage.h"
#include "UIImageFrame.h"
#include "UIImageStyle.h"
#include "UILowerString.h"
#include "UIManager.h"
#include "UIOutputStream.h"
#include "UIPropertyDescriptor.h"
#include "UIStyle.h"
#include "UIUtils.h"

#include <math.h>

const char * UIEllipse::TypeName                           = "Ellipse";
UILowerString UIEllipse::PropertyName::ArcBegin            = UILowerString("ArcBegin");
UILowerString UIEllipse::PropertyName::ArcEnd              = UILowerString("ArcEnd");
UILowerString UIEllipse::PropertyName::InnerRadiusColor    = UILowerString("InnerRadiusColor");
UILowerString UIEllipse::PropertyName::InnerRadiusOpacity  = UILowerString("InnerRadiusOpacity");
UILowerString UIEllipse::PropertyName::Thickness           = UILowerString("Thickness");
UILowerString UIEllipse::PropertyName::OuterThickness      = UILowerString("ThicknessOuter");
UILowerString UIEllipse::PropertyName::CircleSegments      = UILowerString("CircleSegments");
UILowerString UIEllipse::PropertyName::LineSegments        = UILowerString("LineSegments");

#define _TYPENAME UIEllipse

namespace UIEllipseNamespace
{
	float const c_opacityMin = 1.0f / 512.f;
	int const c_circleSegments = 48;
	float const c_piTwo = 6.28318530717958647692f;

	UIFloatPoint s_UIEllipseNullUVs[4] =
	{
		UIFloatPoint(0.0f, 0.0f),
		UIFloatPoint(1.0f, 0.0f),
		UIFloatPoint(0.0f, 1.0f),
		UIFloatPoint(1.0f, 1.0f),
	};


	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(ArcBegin, "", T_float),
		_DESCRIPTOR(ArcEnd, "", T_float),
		_DESCRIPTOR(Thickness, "", T_float),
		_DESCRIPTOR(OuterThickness, "", T_float),
		_DESCRIPTOR(CircleSegments, "", T_int),
		_DESCRIPTOR(LineSegments, "", T_int),
	_GROUPEND(Basic, 3, 0);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(InnerRadiusColor, "", T_color),
		_DESCRIPTOR(InnerRadiusOpacity, "", T_float),
	_GROUPEND(Appearance, 3, 1);
	//================================================================
}

using namespace UIEllipseNamespace;


//======================================================================================
UIEllipse::UIEllipse() : 
UIImage(),
mArcBegin(0.0f),
mArcEnd(1.0f),
mThickness(1.0f),
mOuterThickness(0.0f),
mHasInnerRadiusColor(false),
mInnerRadiusColor(0xFF, 0xFF, 0xFF),
mInnerRadiusOpacity(1.0f),
mPercent(1.0f),
mPointsDirty(true),
mLineSegments(0),
mCircleSegments(c_circleSegments)
{
	mPoints.reserve(mCircleSegments);
}

//======================================================================================
UIEllipse::~UIEllipse()
{
}
												
//======================================================================================
bool UIEllipse::IsA(const UITypeID Type) const
{
	return(Type == TUIEllipse) || UIImage::IsA(Type);
}

//======================================================================================
const char *UIEllipse::GetTypeName(void) const
{
	return TypeName;
}

//======================================================================================
UIBaseObject *UIEllipse::Clone(void) const
{
	return new UIEllipse;
}

//======================================================================================

void UIEllipse::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIImage::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIEllipse::GetPropertyNames(UIPropertyNameVector &In, bool forCopy) const
{
	In.push_back(PropertyName::ArcBegin);
	In.push_back(PropertyName::ArcEnd);
	In.push_back(PropertyName::InnerRadiusColor);
	In.push_back(PropertyName::InnerRadiusOpacity);
	In.push_back(PropertyName::Thickness);
	In.push_back(PropertyName::OuterThickness);
	In.push_back(PropertyName::CircleSegments);
	In.push_back(PropertyName::LineSegments);
	
	UIImage::GetPropertyNames(In, forCopy);
}
												
//======================================================================================

bool UIEllipse::SetProperty(const UILowerString & Name, const UIString &Value)
{
	mPointsDirty = true;

	if (Name == PropertyName::ArcBegin)
	{
		float ArcBegin = 0.0f;

		if (false == UIUtils::ParseFloat(Value, ArcBegin))
		{
			return false;
		}

		SetArcBegin(ArcBegin);
		return true;
	}
	else if (Name == PropertyName::ArcEnd)
	{
		float ArcEnd = 0.0f;

		if (false == UIUtils::ParseFloat(Value, ArcEnd))
		{
			return false;
		}

		SetArcEnd(ArcEnd);
		return true;
	}
	else if (Name == PropertyName::Thickness)
	{
		float Thickness = 0.0f;
		
		if (false == UIUtils::ParseFloat(Value, Thickness))
		{
			return false;
		}
		
		SetThickness(Thickness);
		return true;		
	}
	else if (Name == PropertyName::OuterThickness)
	{
		float OuterThickness = 0.0f;
		
		if (false == UIUtils::ParseFloat(Value, OuterThickness))
		{
			return false;
		}
		
		SetOuterThickness(OuterThickness);
		return true;		
	}
	else if (Name == PropertyName::InnerRadiusColor)
	{
		mHasInnerRadiusColor = UIUtils::ParseColor(Value, mInnerRadiusColor);
		return mHasInnerRadiusColor;
	}
	else if (Name == PropertyName::InnerRadiusOpacity)
	{
		return UIUtils::ParseFloat(Value, mInnerRadiusOpacity);
	}
	else if (Name == PropertyName::LineSegments)
	{
		return UIUtils::ParseInteger(Value, mLineSegments);
	}
	else if (Name == PropertyName::CircleSegments)
	{
		return UIUtils::ParseInteger(Value, mCircleSegments);
	}

	return UIImage::SetProperty(Name, Value);
}

//======================================================================================
bool UIEllipse::GetProperty(const UILowerString & Name, UIString &Value) const
{
	if (Name == PropertyName::ArcBegin)
	{
		return UIUtils::FormatFloat(Value, mArcBegin);
	}
	else if (Name == PropertyName::ArcEnd)
	{
		return UIUtils::FormatFloat(Value, mArcEnd);
	}
	else if (Name == PropertyName::Thickness)
	{
		return UIUtils::FormatFloat(Value, mThickness);		
	}
	else if (Name == PropertyName::OuterThickness)
	{
		return UIUtils::FormatFloat(Value, mOuterThickness);		
	}
	else if (Name == PropertyName::InnerRadiusColor)
	{
		return UIUtils::FormatColor(Value, mInnerRadiusColor);
	}
	else if (Name == PropertyName::InnerRadiusOpacity)
	{
		return UIUtils::FormatFloat(Value, mInnerRadiusOpacity);
	}
	else if (Name == PropertyName::LineSegments)
	{
		return UIUtils::FormatInteger(Value, mLineSegments);
	}
	else if (Name == PropertyName::CircleSegments)
	{
		return UIUtils::FormatInteger(Value, mCircleSegments);
	}

	return UIImage::GetProperty(UILowerString(Name), Value);
}

//======================================================================================
bool UIEllipse::CanSelect(void) const
{
	return false;
}

//======================================================================================
bool UIEllipse::WantsMessage(const UIMessage &) const
{
	return false;
}

//======================================================================================
bool UIEllipse::ProcessMessage(const UIMessage &)
{
	return true;
}

//======================================================================================
void UIEllipse::Render(UICanvas &DestinationCanvas) const
{
	UIWidget::Render(DestinationCanvas);

	DestinationCanvas.EnableFiltering(true);

	const long animationState = GetAnimationState();
	
	UIImageFrame * SourceFrame = 0;
	const UIImageFragment * SourceFragment = 0;

	UIImageStyle const * const style = UIImage::GetImageStyle();
	if(style)
	{
		if(!style->GetFrame(animationState, SourceFrame))
		{
			SourceFragment = style->GetFragmentSelf();
		}
		else
			SourceFragment = SourceFrame;
		
		if(SourceFragment && !SourceFragment->IsReadyToRender())
		{
			SourceFragment = 0;
			SourceFrame    = 0;
		}

		if(SourceFrame)
			style->Render(*SourceFrame, DestinationCanvas, UIPoint::zero, GetSize());
		else
			style->Render(animationState, DestinationCanvas, UIPoint::zero, GetSize());
	}

	float const ScaleX = GetWidth() * 0.5f;
	float const ScaleY = GetHeight() * 0.5f;

	float arcBegin = 0.0f;
	float arcEnd = 0.0f;
	float const arcDiff = fabsf(mArcEnd - mArcBegin);
	
	if (mArcBegin < mArcEnd)
	{
		arcBegin = mArcBegin;
		float const arcRange = arcDiff * mPercent;
		arcEnd = mArcBegin + arcRange;
	}
	else
	{
		arcEnd = mArcBegin;
		float const arcRange = arcDiff *(1.0f - mPercent);
		arcBegin = mArcEnd + arcRange;
	}

	if (mPointsDirty)
	{
		mPoints.clear();
		mLines.clear();

		if (mCircleSegments)
		{
			const float StepSize = c_piTwo *(1.0f / float(mCircleSegments));
			float	theta;

			float InnerRadiusX	= ScaleX - mThickness;
			float InnerRadiusY	= ScaleY - mThickness;

			float OuterRadiusX	= ScaleX - mOuterThickness;
			float OuterRadiusY	= ScaleY - mOuterThickness;
			

			UIUtils::clamp(0.0f, InnerRadiusX, ScaleX);
			UIUtils::clamp(0.0f, InnerRadiusY, ScaleY);
			UIUtils::clamp(InnerRadiusX, OuterRadiusX, ScaleX);
			UIUtils::clamp(InnerRadiusY, OuterRadiusY, ScaleY);
			
			float ct = 0.0f;
			float st = 0.0f;
			
			for(theta = arcBegin * c_piTwo; theta < arcEnd * c_piTwo; theta += StepSize)
			{
				ct = cosf(theta);
				st = sinf(theta);
				
				mPoints.push_back(UIFloatPoint(InnerRadiusX * ct + ScaleX, InnerRadiusY * st + ScaleY));
				mPoints.push_back(UIFloatPoint(OuterRadiusX * ct + ScaleX, OuterRadiusY * st + ScaleY));
			}
			
			ct = cosf(arcEnd * c_piTwo);
			st = sinf(arcEnd * c_piTwo);
			
			mPoints.push_back(UIFloatPoint(InnerRadiusX * ct + ScaleX, InnerRadiusY * st + ScaleY));
			mPoints.push_back(UIFloatPoint(OuterRadiusX * ct + ScaleX, OuterRadiusY * st + ScaleY));
		}

		if (mLineSegments)
		{
			const float StepSize = c_piTwo *(1.0f / float(mLineSegments * 2));
			float	theta = 0.0f;
			
			float InnerRadiusX	= ScaleX - mThickness;
			float InnerRadiusY	= ScaleY - mThickness;
			
			float OuterRadiusX	= ScaleX - mOuterThickness;
			float OuterRadiusY	= ScaleY - mOuterThickness;
			
			UIUtils::clamp(0.0f, InnerRadiusX, ScaleX);
			UIUtils::clamp(0.0f, InnerRadiusY, ScaleY);
			UIUtils::clamp(InnerRadiusX, OuterRadiusX, ScaleX);
			UIUtils::clamp(InnerRadiusY, OuterRadiusY, ScaleY);

		
			float ct = 0.0f;
			float st = 0.0f;
			
			for(theta = arcBegin * c_piTwo; theta < arcEnd * c_piTwo; theta += StepSize)
			{
				ct = cosf(theta);
				st = sinf(theta);
				
				UILine const & line = UILine(UIFloatPoint(InnerRadiusX * ct + ScaleX, InnerRadiusY * st + ScaleY),
                                             UIFloatPoint(OuterRadiusX * ct + ScaleX, OuterRadiusY * st + ScaleY));
				mLines.push_back(line);
			}
			
			ct = cosf(arcEnd * c_piTwo);
			st = sinf(arcEnd * c_piTwo);
			
			UILine const & lineEnd = UILine(UIFloatPoint(InnerRadiusX * ct + ScaleX, InnerRadiusY * st + ScaleY), 
	                                        UIFloatPoint(OuterRadiusX * ct + ScaleX, OuterRadiusY * st + ScaleY));
			mLines.push_back(lineEnd);
		}

		mPointsDirty = false;
	}

	unsigned PointCount = mPoints.size();
	if (PointCount >= 4)
	{
		float const InverseOpacity = GetOpacity() > c_opacityMin ? 1.0f / GetOpacity() : 0.0f;
		unsigned char const OuterAlpha = unsigned char(DestinationCanvas.GetOpacity() * 255.5f);
		unsigned char const InnerAlpha = unsigned char(DestinationCanvas.GetOpacity() * InverseOpacity * mInnerRadiusOpacity * 255.5f);

		if (mLineSegments)
		{
			static UIColor Colors1[2];
			static UIColor Colors2[2];

			if (mHasInnerRadiusColor)
			{
				Colors1[0] = mInnerRadiusColor;			
				Colors1[1] = mInnerRadiusColor;		
			}
			else
			{
				Colors1[0] = GetColor();
				Colors1[1] = GetColor();
			}
			
			Colors1[0].a = InnerAlpha;
			Colors1[1].a = InnerAlpha;
			
			Colors2[0]   = GetColor();
			Colors2[1]   = GetColor();
			Colors2[0].a = OuterAlpha;
			Colors2[1].a = OuterAlpha;

			static UIFloatPoint points[4];
			unsigned iPoint;
			for(iPoint = 0; iPoint <(PointCount >> 1) - 1; ++iPoint)
			{
				UIFloatPointVector::iterator iterPoint = mPoints.begin() +(iPoint << 1);
				
				points[0] = DestinationCanvas.Transform(*(iterPoint));
				points[1] = DestinationCanvas.Transform(*(iterPoint + 1));
				points[2] = DestinationCanvas.Transform(*(iterPoint + 2));
				points[3] = DestinationCanvas.Transform(*(iterPoint + 3));

				UILine const & line1 = UILine(points[0], points[2]);
				DestinationCanvas.RenderLines(GetCanvas(), 1, &line1, s_UIEllipseNullUVs, Colors2);

				UILine const & line2 = UILine(points[3], points[1]);
				DestinationCanvas.RenderLines(GetCanvas(), 1, &line2, s_UIEllipseNullUVs, Colors1);
			}

			if (mLines.size())
			{
				UIColor ColorsLine[2] = {Colors2[0], Colors1[0]};
				UILine line;
				
				for(UILineVector::iterator iterLine = mLines.begin(); iterLine != mLines.end(); ++iterLine)
				{
					DestinationCanvas.Transform(line, *iterLine);
					DestinationCanvas.RenderLines(GetCanvas(), 1, &line, s_UIEllipseNullUVs, ColorsLine);
				}
			}
		}
		else
		{
			static UIColor Colors[4];
			
			if (mHasInnerRadiusColor)
			{
				Colors[0]	 = mInnerRadiusColor;			
				Colors[2]	 = mInnerRadiusColor;		
			}
			else
			{
				Colors[0] = GetColor();
				Colors[2] = GetColor();
			}
			
			Colors[0].a = InnerAlpha;
			Colors[2].a = InnerAlpha;
			
			Colors[1]   = GetColor();
			Colors[1].a = OuterAlpha;
			Colors[3]   = GetColor();
			Colors[3].a = OuterAlpha;

			unsigned iPoint;
			for(iPoint = 0; iPoint <(PointCount >> 1) - 1; ++iPoint)
			{
				UIFloatPointVector::iterator iterPoint = mPoints.begin() +(iPoint << 1);
				
				static UIFloatPoint points[4];
				
				points[0] = DestinationCanvas.Transform(*(iterPoint));
				points[1] = DestinationCanvas.Transform(*(iterPoint + 1));
				points[2] = DestinationCanvas.Transform(*(iterPoint + 2));
				points[3] = DestinationCanvas.Transform(*(iterPoint + 3));
				
				DestinationCanvas.RenderQuad(GetCanvas(), points, s_UIEllipseNullUVs, Colors);
			}
		}
	}

	DestinationCanvas.EnableFiltering(false);
}

//======================================================================================
void UIEllipse::SetAllColors(const UIColor &NewColor)
{
	if (GetColor() != NewColor || GetInnerRadiusColor() != NewColor)
	{
		SetColor(NewColor);
		SetInnerRadiusColor(NewColor);
		mPointsDirty = true;
	}
}

//======================================================================================
void UIEllipse::SetAllOpacity(float const Opacity)
{
	if (GetOpacity() != Opacity || GetInnerRadiusOpacity() != Opacity)
	{
		SetOpacity(Opacity);
		SetInnerRadiusOpacity(Opacity);
		mPointsDirty = true;
	}
}

//======================================================================================
void UIEllipse::SetPercent(float const Percent)
{
	if (mPercent != Percent)
	{
		mPercent = Percent;
		mPointsDirty = true;
	}
}

//======================================================================================
float UIEllipse::GetPercent() const
{
	return mPercent;
}

//======================================================================================
void UIEllipse::SetLineSegments(int const numSegs)
{
	if (mLineSegments != numSegs)
	{
		mLineSegments = numSegs;
		mPointsDirty = true;
	}
}

//======================================================================================
int UIEllipse::GetLineSegments() const
{
	return mLineSegments;
}

//======================================================================================

void UIEllipse::OnSizeChanged(UISize const & newSize, UISize const & oldSize)
{
	UIImage::OnSizeChanged(newSize, oldSize);
	mPointsDirty = true;
}

//======================================================================================

void UIEllipse::OnLocationChanged(UIPoint const & newLocation, UIPoint const & oldLocation)
{
	UIImage::OnLocationChanged(newLocation, oldLocation);
	mPointsDirty = true;
}

//======================================================================================

float UIEllipse::GetArcLength() const
{
	return fabsf(mArcEnd - mArcBegin);
}

