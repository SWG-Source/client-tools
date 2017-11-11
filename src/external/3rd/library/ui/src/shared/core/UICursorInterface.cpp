//======================================================================
//
// UICursorInterface.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UICursorInterface.h"
#include "UICursor.h"
#include "UICanvas.h"

//======================================================================

UICursorInterface::UICursorInterface ()
{
}

//----------------------------------------------------------------------

UICursorInterface::~UICursorInterface ()
{
}

//----------------------------------------------------------------------

UICursorInterfaceDefault::UICursorInterfaceDefault () :
UICursorInterface (),
mCursor (0)
{
}

//----------------------------------------------------------------------

UICursorInterfaceDefault::~UICursorInterfaceDefault ()
{
	setCursor (0);
}

//----------------------------------------------------------------------

void UICursorInterfaceDefault::setCursor (UICursor * cursor)
{
	if (cursor != mCursor)
	{
		if (cursor)
			cursor->Attach (0);

		if (mCursor)
			mCursor->Detach (0);

		mCursor = cursor;
	}
}

//----------------------------------------------------------------------

void UICursorInterfaceDefault::render    (unsigned long , UICursor * cursor, UICanvas & destinationCanvas, const UIPoint & position)
{
	setCursor (cursor);

	if (mCursor)
	{
		destinationCanvas.Flush();				
		mCursor->Render (destinationCanvas, position - mCursor->GetHotSpot ());
	}
}

//======================================================================
