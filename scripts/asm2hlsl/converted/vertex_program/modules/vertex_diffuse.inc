// Generated from vertex_program/modules/vertex_diffuse.inc by asm2hlsl. Included inside main().

// ======================================================================
// inputs:
//
//   r7       = cell/ambient lighting
//
// outputs:
//
//   oD0      = diffuse light
//
// ======================================================================
	vsOutput.color0.rgb = (r7 * vColor0).xyz;
	vsOutput.color0.a = (vColor0).x;
