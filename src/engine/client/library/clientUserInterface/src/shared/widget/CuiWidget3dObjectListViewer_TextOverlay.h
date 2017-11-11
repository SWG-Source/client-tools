//======================================================================
//
// CuiWidget3dObjectListViewer_TextOverlay.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiWidget3dObjectListViewer_TextOverlay_H
#define INCLUDED_CuiWidget3dObjectListViewer_TextOverlay_H

//======================================================================

#include "clientUserInterface/CuiWidget3dObjectListViewer.h"

//----------------------------------------------------------------------

class CuiWidget3dObjectListViewer::TextOverlay
{
public:
	Unicode::String        str;
	UITextStyle *          style;
	long                   width;
	UIColor                color;
	
	TextOverlay  ();
	~TextOverlay ();

	void            recompute ();
	void            render    (const UISize & size, UICanvas & canvas, CuiWidget3dObjectListViewer::TextOverlayTypes tot);
	void            setStyle  (UITextStyle * style);
	void            setText   (const Unicode::String & str, const UIColor & color);

	void            setFadeOut (bool b);
	void            update     (float deltaTimeSecs);

private:

	float                 m_opacity;
	bool                  m_fadeOut;

};

//======================================================================

#endif
