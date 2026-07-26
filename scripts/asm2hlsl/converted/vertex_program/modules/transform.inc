// Generated from vertex_program/modules/transform.inc by asm2hlsl. Included inside main().

// ======================================================================
// inputs:
//
//   none
//
// outputs:
//
//   oPos     = vertex in projection space
//
// ======================================================================
// ----------------------------------------------------------------------
// transform vertex to projection space
	vsOutput.position = float4(dot(vPosition, c[0]), dot(vPosition, c[1]), dot(vPosition, c[2]), dot(vPosition, c[3]));
