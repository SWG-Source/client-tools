//======================================================================
//
// CuiLayer_CursorInterface.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLayer_CursorInterface.h"

#include "UICursor.h"
#include "UICanvas.h"
#include "UIImageStyle.h"
#include "UIImageFrame.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientUserInterface/CuiLayer_TextureCanvas.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/ConfigSharedFoundation.h"

//======================================================================

namespace CursorInterfaceNamespace
{
	const Texture * findTextureAndRectForCursor (unsigned long theTime, UICursor & cursor, UIRect & sourceRect)
	{
		UIImageStyle * const imageStyle = cursor.GetImageStyle ();
		
		if (imageStyle)
		{
			UIImageFragment * fragment = const_cast<UIImageFragment *>(imageStyle->GetFragmentSelf ());
			if (!fragment->IsReadyToRender ())
				fragment = 0;
			
			if (!fragment)
			{
				UIImageFrame * frame = 0;			
				if (!imageStyle->GetFrame (theTime, frame) || !frame)
					return 0;
				fragment = frame;
			}

			UICanvas * const sourceCanvas = fragment->GetCanvas ();
			
			if (sourceCanvas)
			{
				sourceRect   = fragment->GetSourceRect ();
				CuiLayer::TextureCanvas * const textureCanvas = safe_cast<CuiLayer::TextureCanvas *>(sourceCanvas);
				
				const StaticShader * const shader = textureCanvas->getShader ();
				
				if (shader)
				{
					const Texture * texture = 0;						
					if (shader->getTexture (TAG (M,A,I,N), texture) && texture)
					{
						return texture;
					}
				}
			}
		}
		return 0;
	}

	bool ms_acquiredFocus = true;
	void acquiredFocus()
	{
		ms_acquiredFocus = true;
	}
}

using namespace CursorInterfaceNamespace;

//----------------------------------------------------------------------

CuiLayer::CursorInterface::CursorInterface () :
UICursorInterface (),
mCursor (0),
mLastTexture (0),
mBlankCursorTexture(0),
mCursorVisible(true)
{
	mBlankCursorTexture = TextureList::fetch("texture/ui_cursor_blank.dds");
	Os::setAcquiredFocusHookFunction2(CursorInterfaceNamespace::acquiredFocus);
}

//----------------------------------------------------------------------

CuiLayer::CursorInterface::~CursorInterface ()
{
	Os::setAcquiredFocusHookFunction2(NULL);

	if (mCursor)
	{
		mCursor->Detach (0);
		mCursor = 0;
	}

	if (mLastTexture)
	{
		mLastTexture->release ();
		mLastTexture = 0;
	}

	if (mBlankCursorTexture)
	{
		mBlankCursorTexture->release();
		mBlankCursorTexture = 0;
	}
}

//----------------------------------------------------------------------

void CuiLayer::CursorInterface::setCursor (UICursor * cursor)
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

void CuiLayer::CursorInterface::render    (unsigned long theTime, UICursor * cursor, UICanvas & destinationCanvas, const UIPoint & position)
{
	ms_acquiredFocus |= Graphics::wasDeviceReset();

	bool failed = false;

	setCursor (cursor);

	// Once the hardware cursor has failed (e.g. D3D9 SetCursorProperties
	// rejects the texture on certain GPU/driver combos), permanently route
	// through the software cursor path so the user gets the textured cursor
	// (green combat reticle, etc.) rendered as a UI quad instead of a stale
	// or default Windows arrow.
	static bool s_hardwareCursorBroken = false;

	if (Graphics::getHardwareMouseCursorEnabled() && !s_hardwareCursorBroken)
	{
		//-- handle hardware cursor
		const Texture * texture = NULL;
		if (cursor)
		{
			UIRect sourceRect;
			texture = findTextureAndRectForCursor (theTime, *mCursor, sourceRect);
		}

		bool visible = true;
		if (!texture)
		{
			texture = mBlankCursorTexture;
			visible = false;
		}

		int hotSpotX = 0;
		int hotSpotY = 0;
		if (cursor)
		{
			UIPoint const hotSpot = mCursor->GetHotSpot();
			hotSpotX = hotSpot.x;
			hotSpotY = hotSpot.y;
		}

		if (mLastTexture != texture || ms_acquiredFocus || ConfigClientUserInterface::getAlwaysSetMouseCursor ())
		{
			if (Graphics::setMouseCursor(*texture, hotSpotX, hotSpotY))
			{
				// only commit the cache and ref-count swap on success
				if (texture)
					texture->fetch();
				if (mLastTexture)
					mLastTexture->release();
				mLastTexture = texture;
			}
			else
			{
				failed = true;
				s_hardwareCursorBroken = true;
			}
		}

		if (mCursorVisible != visible || ms_acquiredFocus || ConfigClientUserInterface::getAlwaysSetMouseCursor())
		{
			if (!Graphics::showMouseCursor(visible))
			{
				failed = true;
				s_hardwareCursorBroken = true;
			}
			mCursorVisible = visible;
		}

		// On first failure, hide the hardware cursor so it doesn't compete
		// with the software cursor we're about to render this same frame.
		if (s_hardwareCursorBroken)
			IGNORE_RETURN(Graphics::showMouseCursor(false));
	}

	if (!Graphics::getHardwareMouseCursorEnabled() || s_hardwareCursorBroken)
	{
		if (mCursor)
		{
			destinationCanvas.Flush();
			mCursor->Render (destinationCanvas, position - mCursor->GetHotSpot ());
		}
	}

	if (!failed && ms_acquiredFocus)
		ms_acquiredFocus = false;
}

//======================================================================
