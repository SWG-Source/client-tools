// ======================================================================
//
// Direct3d11_ShaderCompiler.h
// copyright (c) 2026 Galaxies Reborn
//
// The single place HLSL becomes bytecode.
//
// One chokepoint, for three reasons. Every shader in the corpus has to be
// compiled with exactly the same flags or the numerical-equivalence harness is
// comparing different things. Swapping FXC for DXC later, if a shader model above
// 5 is ever wanted, is then one file rather than a search. And the flags
// themselves are load-bearing to a degree that is easy to underestimate:
//
//   D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY is mandatory. Without it the
//   corpus does not compile at all -- tex2D is a hard error, sampler
//   declarations are rejected, and the legacy POSITION0 output semantic is not
//   mapped to SV_Position. With it, a constant declared register(cN) lands in
//   the auto-generated $Globals buffer at byte offset exactly 16 * N, including
//   for constants the shader never reads. That single property is what lets
//   twenty years of shader assets keep their constant ABI untouched, and it is
//   the reason this port does not have to renumber anything.
//
//   D3DCOMPILE_PACK_MATRIX_ROW_MAJOR must never be passed. The engine uploads
//   matrix bytes in the order D3D9 wanted them, and HLSL's default column-major
//   packing reads those same bytes correctly. Forcing row-major transposes every
//   matrix in the corpus at once.
//
// Targets are pinned at vs_4_0 and ps_4_0. Nothing in the live corpus needs
// shader model 5 -- no tessellation, no compute, no unordered access, and the
// highest sampler slot in use is s4 -- and pinning one profile keeps every
// comparison against DX9 valid. The prior attempt used 5_0, which is not wrong
// but is one more difference to account for when a pixel diff disagrees.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_ShaderCompiler_H
#define INCLUDED_Direct3d11_ShaderCompiler_H

// ======================================================================

#include <d3d11_1.h>

// For D3D_SHADER_MACRO. It is a typedef rather than a plain struct tag, so it
// cannot be forward declared.
#include <d3dcommon.h>

// ======================================================================

class Direct3d11_ShaderCompiler
{
public:

	static void  install();
	static void  remove();

	// Drop every cached include. Needed by a development-time reload: without it
	// an edited .inc keeps compiling from the copy read the first time.
	static void  flushIncludeCache();

	// Returns a bytecode blob the caller owns, or null. The name is only for
	// diagnostics; make it the asset path so a failure names something findable.
	static ID3DBlob *compileVertexShader(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros);
	static ID3DBlob *compilePixelShader(char const *source, int sourceLength, char const *name, D3D_SHADER_MACRO const *macros);

	static char const *getVertexShaderTarget();
	static char const *getPixelShaderTarget();

private:

	Direct3d11_ShaderCompiler();
	Direct3d11_ShaderCompiler(Direct3d11_ShaderCompiler const &);
	Direct3d11_ShaderCompiler &operator =(Direct3d11_ShaderCompiler const &);
};

// ======================================================================

#endif
