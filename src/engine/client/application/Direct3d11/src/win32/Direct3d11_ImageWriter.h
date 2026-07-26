// ======================================================================
//
// Direct3d11_ImageWriter.h
// copyright (c) 2026 Galaxies Reborn
//
// Reading frames back out of the renderer: screen shots, image writing, and
// back buffer locking.
//
// All of it reads the scene target rather than the swap chain. A FLIP_DISCARD
// back buffer cannot be mapped, and after Present its contents are undefined.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_ImageWriter_H
#define INCLUDED_Direct3d11_ImageWriter_H

// ======================================================================

// GlScreenShotFormat and Gl_imageFormat cannot be forward declared: an unscoped
// enumeration has no known size until it is defined, so the declaration is ill formed
// and MSVC only warns about it.
#include "clientGraphics/Graphics.def"

struct Gl_pixelRect;
class  Rectangle2d;

// ======================================================================

class Direct3d11_ImageWriter
{
public:

	static bool  screenShot(GlScreenShotFormat format, int quality, char const *fileName);
	static bool  writeImage(char const *fileName, int width, int height, int pitch, int const *pixelsARGB, bool alphaExtend, Gl_imageFormat imageFormat, Rectangle2d const *subRect);

	static bool  lockBackBuffer(Gl_pixelRect &pixels, RECT const *lockRect);
	static bool  unlockBackBuffer();

private:

	Direct3d11_ImageWriter();
	Direct3d11_ImageWriter(Direct3d11_ImageWriter const &);
	Direct3d11_ImageWriter &operator =(Direct3d11_ImageWriter const &);
};

// ======================================================================

#endif
