//======================================================================
//
// UIRenderHelper.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIRenderHelper.h"
#include "UITextStyle.h"
#include "UIText.h"
#include <cassert>

//======================================================================
namespace UIRenderHelperNamespace
{
	void multiplyColor(unsigned char & value, float const multiplier)
	{
		int newvalue = static_cast<int>(static_cast<float>(value) * multiplier);
		newvalue = (newvalue < 0) ? 0 : ((newvalue > 255) ? 255 : newvalue);
		value = static_cast<unsigned char>(newvalue);
	}
}

using namespace UIRenderHelperNamespace;

inline void UIRenderHelper::RenderWidget (UICanvas & DestinationCanvas, UIWidget & w, int stage, bool textPass)
{
	UI_UNREF (stage);

	DestinationCanvas.RestoreState();

	UIDeformer * const deformer = w.GetDeformer();
	if (deformer)
	{
		DestinationCanvas.QueueDeformer(*deformer);
	}
	
	const float canvasOpacity        = DestinationCanvas.GetOpacity ();
	const float opacity              = w.GetOpacity () * canvasOpacity;
	const float opacityRelativeMin   = w.GetOpacityRelativeMin ();
	const float opacityRelativeMax   = 1.0f;
	const float opacityRelativeRange = opacityRelativeMax - opacityRelativeMin;
	const float threshold            = 0.20f;
	
	float actualOpacity = (opacity * opacityRelativeRange) + opacityRelativeMin;
	
	if (threshold && opacity < opacityRelativeMin && opacity < threshold)
		actualOpacity *= opacity / threshold;
	
	DestinationCanvas.SetOpacity (actualOpacity);
	
	const float oldTextOpacityRelativeMin = UITextStyle::getOpacityRelativeMin ();
	
	if (w.HasAttribute (UIWidget::BF_TextOpacityRelativeApply))
		UITextStyle::setOpacityRelativeMin(w.mTextOpacityRelativeMin);
	else if (!w.IsEnabled())
		UITextStyle::setOpacityRelativeMin(0.0f);
	
	if (DestinationCanvas.IsNotTransparent() || DestinationCanvas.IsDeforming())
	{
		DestinationCanvas.Flush();
		
		const UIPoint & translate  = w.GetLocation () - w.GetScrollLocation ();
		UIRect  rect               = w.GetRect ();

		if (DestinationCanvas.Clip (rect))
		{
			DestinationCanvas.Translate (translate);
			
			if( w.GetRotation() != 0.0f )
			{
				const UIPoint & CenteringTranslation = w.GetSize() / 2L;
				
				DestinationCanvas.Translate       (CenteringTranslation );
				DestinationCanvas.Rotate          (w.GetRotation() );
				DestinationCanvas.EnableFiltering (true );
				DestinationCanvas.Translate       (-CenteringTranslation );
			}
			
			UIColor widgetColor = w.GetColor();
			float const palShade = w.GetPalShade();

			multiplyColor(widgetColor.r, palShade);
			multiplyColor(widgetColor.g, palShade);
			multiplyColor(widgetColor.b, palShade);

			DestinationCanvas.ModifyColor(widgetColor);

			if(textPass)
			{
				w.RenderText (DestinationCanvas);
			}
			else if (w.IsA (TUIButton))
			{
				UIButton * const button = static_cast<UIButton *>(&w);

				if (stage == -1)
					button->Render (DestinationCanvas);
				else if (stage == 0)
					button->RenderBackground (DestinationCanvas);
				else if (stage == 1)
					button->RenderIcon (DestinationCanvas);
				else if (stage == 2)
					button->RenderText (DestinationCanvas);
			}
			else
				w.Render (DestinationCanvas);
		}
		
		UITextStyle::setOpacityRelativeMin (oldTextOpacityRelativeMin);
	}

	if (deformer)
	{
		DestinationCanvas.DequeueDeformer(*deformer);
	}
}

//-----------------------------------------------------------------

void UIRenderHelper::RenderObjects( UICanvas &DestinationCanvas, const UIWidgetVector &WidgetsToRender, UIWidgetVector * scratch)
{
	DestinationCanvas.PushState();
	
	if (scratch)
		scratch->clear ();

	bool needTextPass = false;
	UIWidgetVector drawLastUIText;
	{
		for( UIWidgetVector::const_reverse_iterator i = WidgetsToRender.rbegin(); i != WidgetsToRender.rend(); ++i ) //lint !e55 !e81 // bad type STL bug in lint
		{
			UIWidget * const w = *i;
			assert (w);
			
			if (w && w->WillDraw())
			{
				if (w->IsA (TUIButton))
				{
					if (scratch)
					{
						RenderWidget (DestinationCanvas, *w, 0);
						scratch->push_back (w);
					}
					else
						RenderWidget (DestinationCanvas, *w, -1);
				}
				else if (w->IsA(TUIText))
				{
					UIText * textView = static_cast<UIText*>(w);
					if (textView && textView->HasTextFlag(UIText::TF_drawLast))
					{
						drawLastUIText.push_back(w);
					}
					else
						RenderWidget (DestinationCanvas, *w, 0);
				}
				else
					RenderWidget (DestinationCanvas, *w, 0);

				if(w->IsA(TUI3DObjectListViewer))
					needTextPass = true;
			}
		}
	}

	if(needTextPass)
	{
		for( UIWidgetVector::const_reverse_iterator i = WidgetsToRender.rbegin(); i != WidgetsToRender.rend(); ++i ) //lint !e55 !e81 // bad type STL bug in lint
		{
			UIWidget * const w = *i;
			assert (w);
			if( w && w->IsA(TUI3DObjectListViewer))
			{
					RenderWidget(DestinationCanvas, *w, 0, true);
			}
		}
	}
	
	if (scratch)
	{
		{
			for (UIWidgetVector::const_iterator it = scratch->begin (); it != scratch->end (); ++it)
			{
				UIWidget * const w = *it;
				assert (w);
				if (w)
					RenderWidget (DestinationCanvas, *w, 1);
			}
		}
		
		for (UIWidgetVector::const_iterator it = scratch->begin (); it != scratch->end (); ++it)
		{
			UIWidget * const w = *it;
			assert (w);
			if (w)
				RenderWidget (DestinationCanvas, *w, 2);
		}
	}

	{
		for (UIWidgetVector::const_iterator it = drawLastUIText.begin (); it != drawLastUIText.end (); ++it)
		{
			UIWidget * const w = *it;
			assert (w);
			if (w)
				RenderWidget (DestinationCanvas, *w, 0);
		}
	}
	
	DestinationCanvas.PopState();
}

//======================================================================
