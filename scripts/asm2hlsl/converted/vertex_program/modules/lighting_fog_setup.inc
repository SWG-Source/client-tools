// Generated from vertex_program/modules/lighting_fog_setup.inc by asm2hlsl. Included inside main().

// ======================================================================
// inputs:
//
//   none
//
// outputs:
//
//   r9.xyz   = vertex in world space
//   r10.xyz  = normal in world space (re-normalized)
//   r11.xyz  = unit vector from vertex to camera
//   r11.w    = distance squared from vertex to camera
//
// ======================================================================
// ----------------------------------------------------------------------
// transform vertex position into world space
	r9.xyz = (float3(dot(vPosition, c[4]), dot(vPosition, c[5]), dot(vPosition, c[6]))).xyz;
// ----------------------------------------------------------------------
//  rotate vertex normal into world space and renormalize
	r10.xyz = (float3(dot((vNormal).xyz, (c[4]).xyz), dot((vNormal).xyz, (c[5]).xyz), dot((vNormal).xyz, (c[6]).xyz))).xyz;
	r10.w = ((dot((r10).xyz, (r10).xyz)).xxxx).w;
	r10.w = ((rsqrt(abs(r10.w))).xxxx).x;
	r10.xyz = (r10 * r10.w).xyz;
// ----------------------------------------------------------------------
//  calculate the direction to the viewer
	r11.xyz = (cCameraPosition - r9).xyz;
	r11.w = ((dot((r11).xyz, (r11).xyz)).xxxx).w;
	r0.w = ((rsqrt(abs(r11.w))).xxxx).x;
	r11.xyz = (r11 * r0.w).xyz;
