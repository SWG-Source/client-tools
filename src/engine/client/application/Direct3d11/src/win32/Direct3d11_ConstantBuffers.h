// ======================================================================
//
// Direct3d11_ConstantBuffers.h
// copyright (c) 2026 Galaxies Reborn
//
// The shader constant register file, and the per-draw slice that must not live
// in it.
//
// The setters below deliberately keep D3D9's signature -- a register index, a
// pointer, and a count of float4 rows -- because that is what lets the light
// manager and the static shader data port across with their upload code
// unchanged. What changes is everything behind them.
//
// D3D9's setters had no CPU shadow, no comparison and no coalescing: every call
// went straight to SetVertexShaderConstantF, and the driver's register file made
// that cheap. D3D11 has no register file. A constant buffer is a resource, and
// updating one costs either a rename or a partial update, so the same call
// pattern issued naively becomes the dominant per-draw cost of the renderer.
//
// So constants are shadowed here, marked dirty per row, and flushed at most once
// per stage per draw -- and the two matrices that genuinely change per draw are
// moved out of the register file entirely:
//
//   b0  $Globals      the register file, rows 0..95, flushed only when dirty
//   b1  reserved      the pixel epilogue (alpha test reference, fog colour),
//                     which arrives with the alpha test work
//   b3  SwgPerObject  objectWorldCameraProjection and objectWorld, 128 bytes,
//                     taken from a ring so a draw costs an offset, not an upload
//
// Leaving the two matrices in b0 is what makes the arithmetic fail. They occupy
// c0..c7, so touching them dirties the front of the register file, and a
// per-draw flush of the reflected extent then moves on the order of 1.5 KB per
// draw where D3D9 moved 128 bytes. At a few thousand draws a frame that is
// megabytes of constant traffic per frame against DX9's hundreds of kilobytes.
// The corresponding asset change -- dropping the register(c0) and register(c4)
// annotations from vertex_shader_constants.inc and declaring those two matrices
// in a cbuffer at b3 -- lands with the shader corpus conversion. Until then this
// backend writes them to b3 and no shader reads them, which is visible as
// untransformed geometry rather than as silently wrong constants.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_ConstantBuffers_H
#define INCLUDED_Direct3d11_ConstantBuffers_H

// ======================================================================

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_ConstantBuffers
{
public:
	// b0 and b1 are the register files; b3 is the per-object ring. b2 is left
	// alone: the prior DX11 effort documented it as its own, and colliding with
	// a convention someone may still have shaders built against costs nothing
	// to avoid.
	static constexpr int VERTEX_GLOBALS_SLOT = 0;
	static constexpr int PIXEL_GLOBALS_SLOT = 0;
	static constexpr int PIXEL_EPILOGUE_SLOT = 1;
	static constexpr int PER_OBJECT_SLOT = 3;

public:
	static void install();
	static void remove();

	// Reset the per-object ring. Called once per frame.
	static void beginFrame();

	// D3D9's shape, kept on purpose: index is a c-register number, and
	// numberOfConstants counts float4 rows, so the caller's struct layout is the
	// register layout exactly as before.
	static void setVertexShaderConstants(int index, void const *data, int numberOfConstants);
	static void setPixelShaderConstants(int index, void const *data, int numberOfConstants);

	// One component of one pixel register. The engine's pixel layout packs unrelated
	// values into single registers -- the material's specular power shares register 0
	// with the dot3 light direction -- so writing it as a whole row would clobber the
	// other three components. DX9 had a dedicated setSpecularPower for the same reason.
	static void setPixelShaderConstantComponent(int index, int component, float value);

	// The two per-draw matrices, in the order and byte layout D3D9 uploaded them
	// to c0..c7. Sixteen floats each, row-major in memory exactly as D3DXMATRIX
	// was, because HLSL's default column-major packing reads those same bytes
	// correctly and transposing here would break every shader at once.
	static void setPerObjectTransforms(float const *objectWorldCameraProjection, float const *objectWorldMatrix);

	// Push whatever is dirty. Called from the draw path; cheap when nothing is.
	static void flush();

	// Constants the backend owns rather than receives. Seeded at install and
	// re-established if the device state is ever rebuilt, mirroring the only four
	// rows D3D9's restoreDevice puts back.
	static void seedBackendOwnedConstants();

	static void setViewportData(int x, int y, int width, int height);
	static void setFog(bool enabled, float density, float red, float green, float blue);

	// A pass's fog colour override, as a ShaderImplementation::Pass::FogMode. FM_Black and
	// FM_White are what additive and multiplicative passes need so that fog contributes nothing or
	// multiplies by one; without them every blended layer fogs with the scene colour and those
	// contributions stack, which is what made distant multi-pass terrain patchy.
	static void setFogColorMode(int fogMode);
	static void setCurrentTime(float currentTime);

	// The alpha test, which D3D11 has no state for at all. The code injected into every
	// pixel program applies it as clip(alpha * scale + bias); the pair is derived inside
	// this class because both the material reference and the engine fade opacity move it,
	// and DX9 combined the two the same way.
	static void setAlphaTest(int compareFunction, int reference);
	static void setAlphaFadeOpacity(float opacity);

private:
	Direct3d11_ConstantBuffers();
	Direct3d11_ConstantBuffers(Direct3d11_ConstantBuffers const &);
	Direct3d11_ConstantBuffers &operator=(Direct3d11_ConstantBuffers const &);
};

// ======================================================================

#endif
