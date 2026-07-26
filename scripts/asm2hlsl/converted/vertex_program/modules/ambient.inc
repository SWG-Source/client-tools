// Generated from vertex_program/modules/ambient.inc by asm2hlsl. Included inside main().

// ======================================================================
//
// inputs:
//
//   none
//
// outputs:
//
//   r7       = cell/ambient lighting
//
// ======================================================================
// ----------------------------------------------------------------------
//  calculate ambient light
	r7 = cLightData_ambient_ambientColor;
