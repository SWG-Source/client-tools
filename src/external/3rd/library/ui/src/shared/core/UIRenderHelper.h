//======================================================================
//
// UIRenderHelper.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIRenderHelper_H
#define INCLUDED_UIRenderHelper_H

#include "UICanvas.h"
#include "UIButton.h"
#include "UITextStyle.h"
#include <vector>

//======================================================================

class UIRenderHelper
{
public:

	typedef ui_stdvector<UIWidget *>::fwd UIWidgetVector;

	static void RenderWidget  (UICanvas & DestinationCanvas, UIWidget & w, int stage, bool textPass = false);
	static void RenderObjects (UICanvas & DestinationCanvas, const UIWidgetVector &WidgetsToRender, UIWidgetVector * scratch = 0);

};

//======================================================================

#endif
