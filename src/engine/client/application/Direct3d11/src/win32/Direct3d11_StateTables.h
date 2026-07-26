// ======================================================================
//
// Direct3d11_StateTables.h
// copyright (c) 2026 Galaxies Reborn
//
// Translation from the engine's render-state enumerations to D3D11.
//
// Every table here is a copy of the DX9 backend's, INDEX FOR INDEX, including
// the entries that look wrong. That is not laziness, it is the contract: shader
// effect assets store the integer index, not the name, so whatever DX9's table
// does with an index is what that asset has meant for twenty years of authoring.
//
// The clearest example is Compare. Direct3d9_ShaderImplementationData.cpp:31-41
// maps index 5 -- named C_GreaterOrEqual in the engine's enum -- to
// D3DCMP_NOTEQUAL, and index 6, named C_NotEqual, to D3DCMP_GREATEREQUAL. The
// two are swapped with respect to their names. An asset authored against DX9
// asking for "C_GreaterOrEqual" has always got a not-equal test, and any content
// that looks right today looks right because of that. "Fixing" the names to
// match changes the meaning of every affected pass.
//
// The prior DX11 attempt un-swapped those two indices, found that stencil
// shadows regressed, and reverted -- keeping a wrong translation to mask the
// symptom instead of recognising the table as the contract.
//
// verifyTables() asserts the swap is present, so a future tidy-up that
// "corrects" it fails immediately and loudly rather than silently changing what
// shipped assets mean.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_StateTables_H
#define INCLUDED_Direct3d11_StateTables_H

// ======================================================================

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_StateTables
{
public:

	// Asserts every table's length and the Compare swap. Called from install.
	static void                      verifyTables();

	static D3D11_COMPARISON_FUNC     getCompare(int engineCompare);
	static D3D11_BLEND               getBlend(int engineBlend);
	static D3D11_BLEND_OP            getBlendOperation(int engineBlendOperation);
	static D3D11_STENCIL_OP          getStencilOperation(int engineStencilOperation);
	static D3D11_TEXTURE_ADDRESS_MODE getTextureAddress(int engineTextureAddress);

	// Filter selection is not a single table in D3D11: min, mag and mip are three
	// fields of one D3D11_FILTER value, where D3D9 set them separately. This
	// composes them, and reports whether anisotropy was asked for so the caller
	// can set MaxAnisotropy.
	static D3D11_FILTER              getFilter(int engineMinFilter, int engineMagFilter, int engineMipFilter, bool &isAnisotropic);

	// True when the engine's mip filter means "no mipmapping at all", which D3D11
	// expresses by clamping the LOD range rather than by a filter bit.
	static bool                      isMipFilterDisabled(int engineMipFilter);

private:

	Direct3d11_StateTables();
	Direct3d11_StateTables(Direct3d11_StateTables const &);
	Direct3d11_StateTables &operator =(Direct3d11_StateTables const &);
};

// ======================================================================

#endif
