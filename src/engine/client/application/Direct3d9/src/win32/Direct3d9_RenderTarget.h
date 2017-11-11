// ======================================================================
//
// Direct3d9_RenderTarget.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_RenderTarget_H
#define INCLUDED_Direct3d9_RenderTarget_H

// ======================================================================

class Texture;
#include "clientGraphics/Texture.def"

// ======================================================================

class Direct3d9_RenderTarget
{
public:

	static void install();
	static void remove();

	static void setRenderTargetToPrimary();
	static void setRenderTarget(Texture *texture, CubeFace cubeFace, int mipmapLevel);
	static bool copyRenderTargetToNonRenderTargetTexture();

	static void lostDevice();
	static void restoreDevice();
};

// ======================================================================

#endif
