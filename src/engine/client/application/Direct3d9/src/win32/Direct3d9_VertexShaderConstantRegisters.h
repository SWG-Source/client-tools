// ======================================================================
//
// Direct3d9_VertexShaderConstantRegisters.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_VertexShaderConstantRegisters_H
#define INCLUDED_Direct3d9_VertexShaderConstantRegisters_H

// ======================================================================
// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

// These values are encoded into the vertex shader data files.  Changing these
// may invalidate all the vertex shaders and require them to be recompiled.

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

enum Direct3d9_VertexShaderConstantRegisters
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
	//CR_userConstant = 46,
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
	VSCR_MAX = 68
};

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

// These values are encoded into the vertex shader data files.  Changing these
// may invalidate all the vertex shaders and require them to be recompiled.

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

// ======================================================================

#endif
