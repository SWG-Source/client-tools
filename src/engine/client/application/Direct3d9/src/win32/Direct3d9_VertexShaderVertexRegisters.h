// ======================================================================
//
// Direct3d9_VertexShaderVertexRegisters.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_VertexShaderVertexRegisters_H
#define INCLUDED_Direct3d9_VertexShaderVertexRegisters_H

#include <d3d9.h>

// ======================================================================

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

// These values are encoded into the vertex shader data files.  Changing these
// may invalidate all the vertex shaders and require them to be recompiled.

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

enum Direct3d9_VertexShaderVertexRegisters
{
	VSVR_position               = 0,
	VSVR_normal                 = 3,
	VSVR_pointSize              = 4,
	VSVR_color0                 = 5,
	VSVR_color1                 = 6,
	VSVR_textureCoordinateSet0  = 7,
	VSVR_textureCoordinateSet1  = 8,
	VSVR_textureCoordinateSet2  = 9,
	VSVR_textureCoordinateSet3  = 10,
	VSVR_textureCoordinateSet4  = 11,
	VSVR_textureCoordinateSet5  = 12,
	VSVR_textureCoordinateSet6  = 13,
	VSVR_textureCoordinateSet7  = 14,
};

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

// These values are encoded into the vertex shader data files.  Changing these
// may invalidate all the vertex shaders and require them to be recompiled.

// ** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **

// ======================================================================

#endif
