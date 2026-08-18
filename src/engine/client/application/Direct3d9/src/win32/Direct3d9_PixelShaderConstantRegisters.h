// ======================================================================
//
// Direct3d9_PixelShaderConstantRegisters.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_PixelShaderConstantRegisters_H
#define INCLUDED_Direct3d9_PixelShaderConstantRegisters_H

// ======================================================================

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

// These values are encoded into the pixel shader data files.  Changing these
// may invalidate all the pixel shaders and require them to be recompiled.

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

enum Direct3d9_PixelShaderConstantRegisters
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

	PSCR_MAX
};

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

// These values are encoded into the pixel shader data files.  Changing these
// may invalidate all the pixel shaders and require them to be recompiled.

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

// ======================================================================

#endif
