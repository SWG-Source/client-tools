// Generated from vertex_program/modules/fog_setup.inc by asm2hlsl. Included inside main().

// ======================================================================
// inputs:
//
//   none
//
// outputs:
//
//   r9.xyz   = vertex in world space
//   r11.xyz  = unit vector from vertex to camera
//   r11.w    = distance squared from vertex to camera
//
// ======================================================================
// ----------------------------------------------------------------------
// transform vertex position into world space
	r9.xyz = (float3(dot(vPosition, c[4]), dot(vPosition, c[5]), dot(vPosition, c[6]))).xyz;
// ----------------------------------------------------------------------
//  calculate the direction to the viewer
	r11.xyz = (cCameraPosition - r9).xyz;
	r11.w = ((dot((r11).xyz, (r11).xyz)).xxxx).x;
	r0.w = ((rsqrt(abs(r11.w))).xxxx).x;
	r11.xyz = (r11 * r0.w).xyz;
