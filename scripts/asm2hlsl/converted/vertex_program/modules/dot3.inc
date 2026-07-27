// Generated from vertex_program/modules/dot3.inc by asm2hlsl. Included inside main().

// ======================================================================
// inputs:
//
//   none
//
// outputs:
//
//   oT0      = texture coordinate for NRML map
//   oT1      = directon to light (packed into 0..1 range)
//
// ======================================================================
// ----------------------------------------------------------------------
// transform the light direction into texture space, pack into 0..1 range
	r0.xyz = (vsInput.textureCoordinateSetDOT3).xyz;
	r2.xyz = (vNormal).xyz;
	r2.w = ((dot((r2).xyz, (r2).xyz)).xxxx).w;
	r2.w = ((rsqrt(abs(r2.w))).xxxx).x;
	r2.xyz = (r2 * r2.w).xyz;
	r1.xyz = (r2.zxy * r0.yzx).xyz;
	r1.xyz = (r2.yzx * r0.zxy + -(r1.xyz)).xyz;
	r1.xyz = (r1 * vsInput.textureCoordinateSetDOT3.w).xyz;
	r0.xyz = (float3(dot((cLightData_dot3_0_direction).xyz, (r0).xyz), dot((cLightData_dot3_0_direction).xyz, (r1).xyz), dot((cLightData_dot3_0_direction).xyz, (r2).xyz))).xyz;
	vsOutput.texcoord0.xyz = (r0 * c0_5 + c0_5).xyz;
	vsOutput.texcoord1 = asmTexcoord(vsInput.textureCoordinateSetNRML);
