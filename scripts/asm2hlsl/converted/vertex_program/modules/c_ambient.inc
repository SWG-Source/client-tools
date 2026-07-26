// Generated from vertex_program/modules/c_ambient.inc by asm2hlsl. Included inside main().

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
//  calculate cell light
	r7 = vColor0 + cLightData_ambient_ambientColor;
