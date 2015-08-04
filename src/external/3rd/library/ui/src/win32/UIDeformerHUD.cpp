#include "_precompile.h"

#include "UIDeformerHUD.h"

#include "UICanvas.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <cfloat>
#include <cmath>
#include <vector>

const char *UIDeformerHUD::TypeName = "DeformerHUD";

const UILowerString UIDeformerHUD::PropertyName::WarpScale = UILowerString("WarpScale");
const UILowerString UIDeformerHUD::PropertyName::WarpShift = UILowerString("WarpShift");

#define _TYPENAME UIDeformerHUD
namespace UIDeformerHUDNamespace
{
	float squareroot(float value)
	{
		int * ival = reinterpret_cast<int*>(&value);
		*ival = ((*ival) >> 1) + (0x3f800000 >> 1);
		return value;
	}

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(WarpScale, "", T_float),
		_DESCRIPTOR(WarpShift, "", T_float),
	_GROUPEND(Basic, 3, 0);
	//================================================================
}

using namespace UIDeformerHUDNamespace;

//======================================================================================

UIDeformerHUD::UIDeformerHUD() :
mWarpScale(10.0f),
mWarpShift(0.0f)
{
}

//======================================================================================

UIDeformerHUD::~UIDeformerHUD()
{
}

//======================================================================================

const char *UIDeformerHUD::GetTypeName() const
{
	return TypeName;
}

//======================================================================================

UIBaseObject *UIDeformerHUD::Clone() const
{
	return new UIDeformerHUD;
}

//======================================================================================

void UIDeformerHUD::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIDeformer::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIDeformerHUD::GetPropertyNames(UIPropertyNameVector &In, bool forCopy) const
{
	UIDeformer::GetPropertyNames(In, forCopy);

	In.push_back(PropertyName::WarpScale);
	In.push_back(PropertyName::WarpShift);
}

//======================================================================================

bool UIDeformerHUD::SetProperty(const UILowerString & Name, const UIString &Value)
{
	if (Name == PropertyName::WarpScale)
	{
		float o = 0.0f;
		
		if (!UIUtils::ParseFloat(Value, o))
			return false;

		mWarpScale = o;
		return true;
	}
	else if (Name == PropertyName::WarpShift)
	{
		float o = 0.0f;
		
		if (!UIUtils::ParseFloat(Value, o))
			return false;
		
		mWarpShift = fabsf(o);
		return true;
	}
	else
		return UIDeformer::SetProperty(Name, Value);
}

//======================================================================================

bool UIDeformerHUD::GetProperty(const UILowerString & Name, UIString &Value) const
{
	if (Name == PropertyName::WarpScale)
		return UIUtils::FormatFloat( Value, mWarpScale );
	else if (Name == PropertyName::WarpShift)
		return UIUtils::FormatFloat( Value, mWarpShift );
	else
		return UIDeformer::GetProperty(Name, Value);
}

//----------------------------------------------------------------------
bool UIDeformerHUD::Deform(UICanvas & canvas, UIFloatPoint const * points, UIFloatPoint * out, size_t count)
{
	UISize canvasSize;
	canvas.GetSize(canvasSize);

	float const canvasWidth = static_cast<float>(canvasSize.x);
	float const halfWidth = canvasWidth * 0.5f;

	if (halfWidth > 4.0f)
	{
		float ooWidth = 1.0f / canvasWidth;

		for (size_t cnt = 0; cnt < count; ++cnt, ++out, ++points)
		{
			float const xoffset = (points->x - halfWidth) * ooWidth;
			float offset = squareroot(xoffset * xoffset + mWarpShift);
			out->x = points->x;
			out->y = points->y + offset * mWarpScale;
		}
	}

	return true;
}

//----------------------------------------------------------------------
