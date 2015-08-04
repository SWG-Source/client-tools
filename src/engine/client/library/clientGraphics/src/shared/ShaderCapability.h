// ======================================================================
//
// ShaderCapability.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ShaderCapability_H
#define INCLUDED_ShaderCapability_H

// ======================================================================

inline int ShaderCapability(int major, int minor, bool allowInvalid=false)
{
	if ((major == 2 && minor == 0) || (major == 1 && minor == 4) || (major == 1 && minor == 1) || (major == 0 && minor == 3) || (major == 0 && minor == 2) || (major == 0 && minor == 0) || allowInvalid)
		return (major << 8) | minor;

	DEBUG_FATAL(true, ("Invalid shader capability specified %d.%d", major, minor));
	return 0;
}

// ----------------------------------------------------------------------

inline int GetShaderCapabilityMajor(int shaderCapability)
{
	return (shaderCapability >> 8) & 0xff;
}

// ----------------------------------------------------------------------

inline int GetShaderCapabilityMinor(int shaderCapability)
{
	return shaderCapability & 0xff;
}

// ======================================================================

#endif
