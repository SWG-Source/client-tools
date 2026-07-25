// ======================================================================
//
// Direct3d11.h
// copyright (c) 2026 Galaxies Reborn
//
// The Direct3D 11 raster backend. Loaded by clientGraphics as gl11_{r,o,d}.dll
// when [ClientGraphics] rasterMajor is 11, and reached only through the Gl_api
// function table this file's GetApi returns.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_H
#define INCLUDED_Direct3d11_H

// ======================================================================

struct Gl_install;

// ======================================================================

class Direct3d11
{
public:

	static bool install(Gl_install *gl_install);

private:

	Direct3d11();
	Direct3d11(Direct3d11 const &);
	Direct3d11 &operator =(Direct3d11 const &);
};

// ======================================================================

#endif
