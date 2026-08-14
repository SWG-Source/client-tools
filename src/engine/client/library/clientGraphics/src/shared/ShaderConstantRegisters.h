// ======================================================================
//
// ShaderConstantRegisters.h
// Copyright 2001-2002 Sony Online Entertainment Inc.
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================
// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **
//
// These values are encoded into the shipped shader data files. They are not a
// backend's private numbering: every .vsh and .psh in the TRE set was compiled
// against them, so a change here silently changes what every one of those
// programs reads. Nothing recompiles them. Nothing warns.
//
// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **
// ======================================================================
//
// Promoted here from the Direct3d9 backend, where these three enumerations lived
// as Direct3d9_VertexShaderConstantRegisters.h,
// Direct3d9_PixelShaderConstantRegisters.h and
// Direct3d9_VertexShaderVertexRegisters.h. Values are byte-identical to those
// files; only their location changed.
//
// They belong to clientGraphics rather than to a backend for two reasons. They
// describe an asset format, and an asset format outlives the renderer that
// happened to read it first -- a second backend needs the same numbers, and two
// copies of an ABI is a divergence waiting to happen. And ShaderBuilder, the only
// tool that validates a .vsh and writes a .psh, already reaches across a layering
// boundary to include them by relative path, so deleting the DX9 backend would
// have broken the tool with no compile-time warning until someone built it.
//
// The correspondence to a D3D constant buffer is exact and worth stating: a
// constant declared `register(cN)` in the shader source lands at byte offset
// 16 * N in the auto-generated $Globals buffer. So these register numbers ARE the
// byte offsets, divided by sixteen, and the reflection guard in the DX11 backend
// asserts precisely that.
//
// ======================================================================

#ifndef INCLUDED_ShaderConstantRegisters_H
#define INCLUDED_ShaderConstantRegisters_H

// ======================================================================
// Vertex shader constant registers.

enum VertexShaderConstantRegisters
{
	VSCR_objectWorldCameraProjectionMatrix = 0,
	VSCR_objectWorldMatrix = 4,
	VSCR_cameraPosition = 8,
	VSCR_viewportData = 9,
	VSCR_fog = 10,
	VSCR_material = 11,
	VSCR_lightData = 16,
	VSCR_textureFactor = 44,
	VSCR_textureFactor2 = 45,
	// CR_userConstant = 46,
	VSCR_textureScroll = 47,
	VSCR_currentTime = 48,
	VSCR_unitX = 49,
	VSCR_unitY = 50,
	VSCR_unitZ = 51,
	VCSR_userConstant0 = 52,
	VCSR_userConstant1 = 53,
	VCSR_userConstant2 = 54,
	VCSR_userConstant3 = 55,
	VCSR_userConstant4 = 56,
	VCSR_userConstant5 = 57,
	VCSR_userConstant6 = 58,
	VCSR_userConstant7 = 59,
	VCSR_extendedLightData = 60,
	VSCR_MAX = 68,

	// c95 is not reachable from VSCR_MAX and never was. The shipped
	// registers.inc aliases four scalars onto its components -- c0_0, c0_5, c1_0
	// and cLog2e map to c95.x through c95.w -- and nine assembly modules consume
	// them, so a constant buffer sized from VSCR_MAX alone leaves every 0.5 bias
	// and every 1.0 constant in those programs reading zero.
	//
	// Named here rather than left as a literal in a backend, because it is the
	// same kind of thing as every value above it: an asset-encoded register.
	VSCR_literalConstants = 95,

	// Rows a vertex constant buffer must have to cover everything the corpus
	// addresses. Derived, so it cannot drift from the values above.
	VSCR_CBUFFER_ROWS = VSCR_literalConstants + 1
};

// ======================================================================
// Pixel shader constant registers.

enum PixelShaderConstantRegisters
{
	PSCR_dot3LightDirection,
	PSCR_dot3LightDiffuseColor,
	PSCR_dot3LightSpecularColor,
	PSCR_dot3LightTangentMinusDiffuseColor,
	PSCR_dot3LightTangentMinusBackColor,
	PSCR_textureFactor,
	PSCR_textureFactor2,
	PSCR_materialSpecularColor,

	// keep this constant last, so that we can use multiple user constant registers
	PSCR_userConstant,

	PSCR_MAX,

	// Rows a pixel constant buffer must have.
	//
	// There IS a tail beyond the enumeration, and this used to say there was not.
	// PSCR_userConstant is the FIRST of a run -- its own comment says "so that we can
	// use multiple user constant registers" -- and the shipped
	// pixel_program/include/pixel_shader_constants.inc declares that run as
	//
	//     float4 userConstants[17] : register(c8);
	//
	// which occupies c8 through c24. Sizing the buffer at PSCR_MAX made it nine rows,
	// so every user constant past the first was outside the buffer: the bloom passes
	// tripped the range FATAL in Direct3d11.cpp and 2d_blur and 2d_downsample read
	// registers that were not there.
	PSCR_userConstantCount = 17,
	PSCR_CBUFFER_ROWS = PSCR_userConstant + PSCR_userConstantCount
};

// ======================================================================
// Vertex shader input registers -- the vN slots a vertex declaration feeds.
//
// Note these are D3D9 input register numbers. Shader model 4 has no location
// semantics on vertex inputs at all: an input layout is matched to the shader by
// semantic NAME. The numbering survives here because the .vsh sources are
// annotated with it and a converter has to know what each annotation meant.

enum VertexShaderVertexRegisters
{
	VSVR_position = 0,
	VSVR_normal = 3,
	VSVR_pointSize = 4,
	VSVR_color0 = 5,
	VSVR_color1 = 6,
	VSVR_textureCoordinateSet0 = 7,
	VSVR_textureCoordinateSet1 = 8,
	VSVR_textureCoordinateSet2 = 9,
	VSVR_textureCoordinateSet3 = 10,
	VSVR_textureCoordinateSet4 = 11,
	VSVR_textureCoordinateSet5 = 12,
	VSVR_textureCoordinateSet6 = 13,
	VSVR_textureCoordinateSet7 = 14
};

// ======================================================================

#endif
