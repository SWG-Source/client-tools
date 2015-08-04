//======================================================================
//
// CuiWidget3dObjectListViewer_TextOverlay.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer_TextOverlay.h"

#include "UITextStyle.h"
#include "UICanvas.h"

//======================================================================

CuiWidget3dObjectListViewer::TextOverlay::TextOverlay () :
str       (),
style     (0),
width     (0),
color     (UIColor::white),
m_opacity (1.0f),
m_fadeOut (false)
{
}

//----------------------------------------------------------------------

CuiWidget3dObjectListViewer::TextOverlay::~TextOverlay ()
{
	setStyle (0);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::TextOverlay::setStyle  (UITextStyle * _style)
{
	if (style != _style) 
	{
		if (style)
			style->Detach (0);

		style = _style;
		
		if (style)
			style->Attach (0);

		recompute();
	}

}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::TextOverlay::recompute ()
{
	if (style && !str.empty ())
	{
		long dummyHeight = 0L;
		style->MeasureText (str, width, dummyHeight);
	}
	else
		width = 0;

}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::TextOverlay::setText (const Unicode::String & _str, const UIColor & _color)
{
	if (str != _str)
	{
		str = _str;
		recompute ();
	}

	color = _color;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::TextOverlay::render    (const UISize & size, UICanvas & canvas, CuiWidget3dObjectListViewer::TextOverlayTypes tot)
{
	if (style && !str.empty ())
	{
		UIPoint pos;
		UITextStyle::Alignment align = UITextStyle::Right;

		bool twolines = false;

		if (tot == CuiWidget3dObjectListViewer::TOT_topRight)
			twolines = true;

		if (tot == CuiWidget3dObjectListViewer::TOT_bottom)
		{
			if (width >= size.x)
			{
				pos.y = size.y - style->GetLeading () * 2L;
				twolines = true;
			}
			else
			{
				pos.y = size.y - style->GetLeading ();
			}

			align = UITextStyle::Center;
		}
		else if (tot == CuiWidget3dObjectListViewer::TOT_topLeft)
			align = UITextStyle::Left;

		bool const dropShadowWasEnabled = UITextStyle::GetGlobalDropShadowEnabled ();
		UITextStyle::SetGlobalDropShadowEnabled(true);

		canvas.PushState ();
		canvas.ModifyOpacity (m_opacity);
		canvas.SetColor (color);
		if (twolines)
		{
			style->RenderText           (align, str, canvas, pos, &size.x, 2, true);
		}
		else
		{
			UITextStyle::RenderLineData rld (&canvas, align, pos, size.x, 0, 0, color);
			rld.truncateElipsis = true;
			rld.begin           = str.begin ();
			rld.nextLine        = str.end   ();
			rld.lineWidth       = width;

//			style->RenderPrewrappedLine (align, str.begin (), str.end (), width, &canvas, pos, size.x, inescape, true);
			style->RenderPrewrappedLine (rld);
		}

		canvas.PopState ();

		UITextStyle::SetGlobalDropShadowEnabled(dropShadowWasEnabled);
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::TextOverlay::setFadeOut (bool b)
{
	m_fadeOut = b;
	if (!m_fadeOut)
		m_opacity = 1.0f;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::TextOverlay::update     (float deltaTimeSecs)
{
	if (m_fadeOut)
		m_opacity -= deltaTimeSecs;

	if (m_opacity < 0)
	{
		m_opacity = 0.0f;
		m_fadeOut = false;
	}
}


//======================================================================
