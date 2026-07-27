// Generated from vertex_program/modules/dot3_3x2spec.inc by asm2hlsl. Included inside main().

// ======================================================================
// inputs:
//
//   none
//
// outputs:
//
//   oT0      = texture coordinate for NRML map
//   oT1      = direction to light (-1 .. 1)
//   oT2      = half angle vector (-1 .. 1)
//
// ======================================================================
// build transform from object space to texture space
	r0.xyz = (vsInput.textureCoordinateSetDOT3).xyz;
	r2.xyz = (vNormal).xyz;
	r2.w = ((dot((r2).xyz, (r2).xyz)).xxxx).w;
	r2.w = ((rsqrt(abs(r2.w))).xxxx).x;
	r2.xyz = (r2 * r2.w).xyz;
	r1.xyz = (r2.zxy * r0.yzx).xyz;
	r1.xyz = (r2.yzx * r0.zxy + -(r1.xyz)).xyz;
	r1.xyz = (r1 * vsInput.textureCoordinateSetDOT3.w).xyz;
// pass through normal map coordinates
	vsOutput.texcoord0 = asmTexcoord(vsInput.textureCoordinateSetNRML);
//m3x3 r3.xyz, vNormal, r0
//mad oT0.xyz, r3, c0_5, c0_5
// transform the light direction into texture space
	vsOutput.texcoord1.xyz = (float3(dot((cLightData_dot3_0_direction).xyz, (r0).xyz), dot((cLightData_dot3_0_direction).xyz, (r1).xyz), dot((cLightData_dot3_0_direction).xyz, (r2).xyz))).xyz;
// calculate the half angle (H = L + V / |L + V|)
	r3.xyz = (cLightData_dot3_0_cameraPosition - vPosition).xyz;
	r3.w = ((dot((r3).xyz, (r3).xyz)).xxxx).w;
	r3.w = ((rsqrt(abs(r3.w))).xxxx).x;
	r3.xyz = (r3 * r3.w).xyz;
	r3.xyz = (r3 + cLightData_dot3_0_direction).xyz;
	r3.w = ((dot((r3).xyz, (r3).xyz)).xxxx).w;
	r3.w = ((rsqrt(abs(r3.w))).xxxx).x;
	r3.xyz = (r3 * r3.w).xyz;
// transform half angle into texture space
	vsOutput.texcoord2.xyz = (float3(dot((r3).xyz, (r0).xyz), dot((r3).xyz, (r1).xyz), dot((r3).xyz, (r2).xyz))).xyz;
