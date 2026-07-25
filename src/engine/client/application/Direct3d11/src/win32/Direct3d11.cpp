// ======================================================================
//
// Direct3d11.cpp
// copyright (c) 2026 Galaxies Reborn
//
// The entry point clientGraphics reaches this DLL through. Graphics::install
// LoadLibrary's gl11_{r,o,d}.dll, checks GetGlApiStructSize against its own
// sizeof(Gl_api), calls GetApi, then verify, then install.
//
// This is the project skeleton: the contract, the size guard and an install
// that refuses. Device bring-up, resources, shaders and draw paths arrive in
// later commits. Until then install returning false is the honest answer, and
// it leaves rasterMajor=5 as the working renderer.
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11.h"

#include "SetupDll.h"

#include "clientGraphics/Gl_dll.def"

// ======================================================================

namespace Direct3d11Namespace
{
	bool     verify();

	Gl_api   ms_glApi;
}

using namespace Direct3d11Namespace;

// ======================================================================

extern "C" __declspec(dllexport) Gl_api const * GetApi();

// Reported out of band rather than as a Gl_api member, and checked by
// Graphics::install before it calls through any slot. Gl_api has three binary
// layouts -- five slots are #ifdef _DEBUG and four more are
// #if PRODUCTION == 0 -- selected by the same DEBUG_LEVEL that picks the
// _r/_o/_d suffix in this DLL's name. A client and a backend built at
// different levels would otherwise load cleanly and then call the wrong
// function through every shifted slot.
extern "C" __declspec(dllexport) unsigned int GetGlApiStructSize();

// ======================================================================

Gl_api const * GetApi()
{
	ms_glApi.verify  = verify;
	ms_glApi.install = Direct3d11::install;
	return &ms_glApi;
}

// ----------------------------------------------------------------------

unsigned int GetGlApiStructSize()
{
	return static_cast<unsigned int>(sizeof(Gl_api));
}

// ----------------------------------------------------------------------
/**
 * Report whether this backend can run at all.
 *
 * Graphics::install calls this before install(). It stays true even while
 * install() refuses, so that the load path itself -- LoadLibrary, the size
 * guard, GetApi -- is exercised and can be verified.
 */

bool Direct3d11Namespace::verify()
{
	return true;
}

// ----------------------------------------------------------------------
/**
 * Bring up the device.
 *
 * Not yet implemented. Returning false makes Graphics::install fail loudly
 * instead of handing the engine a table of null slots.
 */

bool Direct3d11::install(Gl_install *gl_install)
{
	UNREF(gl_install);

	WARNING(true, ("Direct3d11: the DX11 backend is not implemented yet, so install is refusing. Set [ClientGraphics] rasterMajor=5 to run on DX9."));
	return false;
}

// ======================================================================
