// ======================================================================
//
// Direct3d11_ShaderSource.h
// copyright (c) 2026 Galaxies Reborn
//
// Every edit this backend makes to shader text before it reaches the compiler, in one
// place, with the evidence for each.
//
// There are six, and they divide cleanly. Four are inherited from the x64 DX9 build --
// that build is the image this port is measured against, so its source patches are part
// of the target whether or not one likes them. Two are new, and exist because
// d3dcompiler_47 rejects constructs the June-2010 D3DX compiler accepted and offers no
// legacy switch to get them back.
//
// ----------------------------------------------------------------------
// Inherited from the DX9 x64 build
//
//   1. `point` is renamed. Modern HLSL reserves it as a geometry-shader input topology
//      and vertex_shader_constants.inc uses it as a field name on LightData
//      (`PointLight point[NumberOfPointLights]`). DX9 prepends a #define to every
//      include it serves; this passes the same rename as a global macro, which reaches
//      the program source as well. No visual effect.
//
//   2. c_ambient.inc's `mov r7, vColor0` becomes `add r7, vColor0, c16`. DX9's stated
//      reason: skinned meshes have no baked vColor0, so characters received zero
//      ambient and went black in scenes where the parallel sun is weak. CHANGES THE
//      IMAGE.
//
//   3. `lightData.ambient.ambientColor + diffuseSpecular.diffuse` is wrapped in
//      max(..., 0.85). DX9's stated reason: dot3 bump shaders skip the parallel-spec
//      light slot, so outdoor characters rendered dark. CHANGES THE IMAGE, and 0.85 is
//      a constant somebody tuned by eye.
//
//   4. pixel_program/include/pixel_shader_constants.inc is replaced wholesale with the
//      engine's own register layout. The TRE copy declares textureFactor at c3, where
//      the engine uploads dot3LightTangentMinusDiffuseColor -- a negative value for the
//      synthesised hemispheric suns -- so a shader recompiled from source multiplies
//      colour by a negative number, the result clamps to zero, and every humanoid
//      renders black. DX9 needed this the moment it started recompiling from PSRC
//      instead of using the precompiled PEXE blob. This backend has no PEXE option at
//      all, so it is not optional here.
//
// ----------------------------------------------------------------------
// New, and forced by the compiler rather than chosen
//
//   5. vertex_program/include/vertex_shader_constants.inc is replaced with a flattened
//      equivalent. The shipped copy writes `Material material : register(c11);` and
//      `LightData lightData : register(c16);`, binding a register to an aggregate.
//      d3dcompiler_47 rejects that with X3202 and D3DX rejects it with X4016; DX9 gets
//      around it by forcing the legacy D3DX9_31 compiler through
//      D3DXSHADER_USE_LEGACY_D3DX9_31_DLL, and D3DCompile has no equivalent. Without
//      the replacement, 192 of the corpus's vertex programs do not build at all.
//
//      The replacement declares one float4 per register and rebuilds the structs as
//      `static` values, so shader source is untouched -- including
//      `lightData.point[i]` with a loop index, which a macro-based flattening could not
//      express. Registers are taken from the C++ structs the engine actually uploads
//      (Direct3d9_LightManager.h), every member of which is a four-float type, so each
//      occupies exactly one register; the run ends at c43 and the shipped include puts
//      textureFactor at c44, which is the arithmetic check that the layout is the
//      engine's rather than a guess.
//
//   6. The `: register(vN)` clause is removed from vertex input declarations. Programs
//      declare inputs as `float4 position : POSITION0 : register(v0);`, and a location
//      semantic on a struct member is X3202 as well.
//
//      This was measured before being done. Across all 192 vertex programs that carry
//      such a clause, every (semantic, vN) pair agrees with the engine's own
//      assignment (VSVR_position 0, normal 3, pointSize 4, color0 5, color1 6,
//      textureCoordinateSet0..7 7..14) with four exceptions. So for all but those four
//      the semantic already carries the binding, D3D11's input layout matches on
//      semantic anyway, and dropping the clause loses nothing. Where the two disagree
//      the register is what DX9 honoured, so the SEMANTIC is rewritten to the one that
//      owns that register rather than the clause being silently dropped. One program
//      binds POSITION0 to v1, a register the engine never writes; DX9 read undefined
//      data there, it is not reachable at shader capability 2.0, and it is reported
//      rather than guessed at.
//
// ----------------------------------------------------------------------
// What this does not do
//
// It does not translate assembly. A program whose first line is `//asm` or a bare
// `ps.1.x` version directive is not HLSL, D3DCompile has no assembler, and D3D9 shader
// assembly is not a D3D11 shader model. Ninety-seven such programs are reachable at
// capability 2.0 (36 vertex, 61 pixel); they need translating to HLSL, which is a
// separate piece of work with its own verification, and until it exists the program
// classes refuse them by name.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_ShaderSource_H
#define INCLUDED_Direct3d11_ShaderSource_H

// ======================================================================

#include <d3dcommon.h>

// ======================================================================

class Direct3d11_ShaderSource
{
public:

	// The language a program's first line declares.
	enum Language
	{
		L_hlsl,
		L_assembly,
		L_unknown
	};

	// Read the marker line. Assembly is either an explicit "//asm" or a bare "ps.1.x" /
	// "vs.1.x" version directive, which is how the pixel programs write it.
	static Language     getLanguage(char const *source, int sourceLength);

	// The whole text this backend serves for an #include, or null to read the TRE copy.
	// Overrides 4 and 5 come out of here.
	static char const  *getIncludeOverride(char const *path, int &length);

	// Patch 2 applied to an include's TRE contents, plus the caller-independent parts.
	// Returns a newly allocated buffer the caller owns, or null when nothing changed.
	static char        *patchIncludeContents(char const *path, char const *contents, int length, int &patchedLength);

	// Patches 3 and 6, applied to a program's source. Returns a newly allocated buffer
	// the caller owns, or null when nothing changed. isVertexProgram gates patch 6,
	// which has no meaning for a pixel program.
	static char        *patchProgramSource(char const *name, char const *source, int length, bool isVertexProgram, int &patchedLength);

	// Patch 1, as a macro the caller appends to its define list.
	static D3D_SHADER_MACRO const &getPointRenameMacro();

private:

	Direct3d11_ShaderSource();
	Direct3d11_ShaderSource(Direct3d11_ShaderSource const &);
	Direct3d11_ShaderSource &operator =(Direct3d11_ShaderSource const &);
};

// ======================================================================

#endif
