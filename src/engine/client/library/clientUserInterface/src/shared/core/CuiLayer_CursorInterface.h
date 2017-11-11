//======================================================================
//
// CuiLayer_CursorInterface.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiLayer_CursorInterface_H
#define INCLUDED_CuiLayer_CursorInterface_H

#include "UICursorInterface.h"
#include "clientUserInterface/CuiLayer.h"

//======================================================================

class Texture;

//----------------------------------------------------------------------

class CuiLayer::CursorInterface :
public UICursorInterface
{
public:
	CursorInterface ();
	~CursorInterface ();

	void setCursor (UICursor * cursor);
	void render    (unsigned long theTime, UICursor * cursor, UICanvas & destinationCanvas, const UIPoint & position);
private:

	UICursor *               mCursor;
	const Texture *          mLastTexture;
	const Texture *          mBlankCursorTexture;
	bool                     mCursorVisible;
};

//======================================================================

#endif
