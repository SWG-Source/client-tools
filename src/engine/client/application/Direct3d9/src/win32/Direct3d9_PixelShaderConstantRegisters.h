// ======================================================================
//
// Direct3d9_PixelShaderConstantRegisters.h
// Copyright 2001-2002 Sony Online Entertainment Inc.
//
// ======================================================================
//
// The pixel shader constant registers moved to clientGraphics/ShaderConstantRegisters.h. They describe
// an asset format rather than anything specific to a Direct3D 9 backend, and a
// second backend needs the same numbers -- two copies of an ABI is a divergence
// waiting to happen.
//
// This file remains only so existing includes keep resolving. It will go when
// the Direct3d9 backend does.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_PixelShaderConstantRegisters_H
#define INCLUDED_Direct3d9_PixelShaderConstantRegisters_H

// ======================================================================

#include "clientGraphics/ShaderConstantRegisters.h"

// ======================================================================

#endif
