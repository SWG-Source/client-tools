#include "_precompile.h"
#include "UIGBuffer.h"
#include "UIGBuffer_Command.h"
#include "UICanvas.h"
#include "UILowerString.h"
#include "UIUtils.h"
#include "UIRenderHelper.h"


namespace UIGBufferCommandNamespace
{
	UIFloatPoint s_UIGBufferNullUVs[4];
}

using namespace UIGBufferCommandNamespace;

//======================================================================================
UIGBuffer_Command::UIGBuffer_Command() :
mCommand(ID_Null),
mWidget(NULL)
{
}

//======================================================================================
UIGBuffer_Command::UIGBuffer_Command(UIWidget * widget) :
mCommand(ID_Widget),
mWidget(NULL)
{
	attach(widget);
}

//======================================================================================
UIGBuffer_Command::~UIGBuffer_Command()
{
	attach(NULL);
}

//======================================================================================
void UIGBuffer_Command::attach(UIWidget * widget)
{
	if (widget != mWidget)
	{
		if (mWidget)
		{
			mWidget->SetVisible(false);
			mWidget->Detach(NULL);
		}

		mWidget = widget;

		if (mWidget)
		{
			mWidget->Attach(NULL);
		}
	}
}


//======================================================================================
void UIGBuffer_Command::Render(UICanvas & canvas) const
{
	switch(mCommand)
	{
	case ID_Widget:
		{
			mWidget->SetVisible(true);
			UIRenderHelper::RenderWidget(canvas, *mWidget, 0);
			mWidget->SetVisible(false);
		}
	break;

	case ID_Quad:
		{
			UIColor uiColor = canvas.GetColor();
			float uiOpacity = canvas.GetOpacity();

			canvas.SetColor(mColor);
			canvas.SetOpacity(mOpacity);

			std::vector<UIFloatPoint> points;
			for (PointVector::const_iterator itPoint = mPoints.begin(); itPoint != mPoints.end(); ++itPoint)
			{
				points.push_back(canvas.TransformFP(static_cast<float>(itPoint->x), static_cast<float>(itPoint->y)));
			}

			canvas.RenderQuad(0, &(points.front()), s_UIGBufferNullUVs);

			canvas.SetColor(uiColor);
			canvas.SetOpacity(uiOpacity);
		}
	break;

	case ID_Line:
		{
			UIColor uiColor = canvas.GetColor();
			float uiOpacity = canvas.GetOpacity();

			canvas.SetColor(mColor);
			canvas.SetOpacity(mOpacity);

			std::vector<UIFloatPoint> points;
			for (PointVector::const_iterator itPoint = mPoints.begin(); itPoint != mPoints.end(); ++itPoint)
			{
				points.push_back(canvas.TransformFP(static_cast<float>(itPoint->x), static_cast<float>(itPoint->y)));
			}

			canvas.RenderLineStrip(0, 1, &(points.front()));

			canvas.SetColor(uiColor);
			canvas.SetOpacity(uiOpacity);
		}
		break;
	}
}

//======================================================================================
void UIGBuffer_Command::SetType(Id command)
{
	mCommand = command;
}

//======================================================================================
void UIGBuffer_Command::SetPoints(UIPoint const & p1, UIPoint const & p2, UIPoint const & p3, UIPoint const & p4)
{
	mPoints.push_back(p1);
	mPoints.push_back(p2);
	mPoints.push_back(p4);
	mPoints.push_back(p3);
}

//======================================================================================
void UIGBuffer_Command::SetPoints(UIPoint const & p1, UIPoint const & p2)
{
	mPoints.push_back(p1);
	mPoints.push_back(p2);
}

//======================================================================================
void UIGBuffer_Command::SetColor(UIColor const & c1)
{
	mColor = c1;
}

//======================================================================================
void UIGBuffer_Command::SetOpacity(float const o1)
{
	mOpacity = o1;
}

