// Generated from vertex_program/modules/env_t1.inc by asm2hlsl. Included inside main().

// ======================================================================
// inputs:
//
//   r11.xyz  = unit vector from vertex to camera
//
// outputs:
//
//   oT1      = r11 reflected around normal
//
// ======================================================================
// normal * (2 * normal.dot(incident)) - incident;
	r0 = (dot((vNormal).xyz, (r11).xyz)).xxxx;
	r0 = r0 + r0;
	vsOutput.texcoord1 = asmTexcoord(vNormal * r0 + -(r11));
