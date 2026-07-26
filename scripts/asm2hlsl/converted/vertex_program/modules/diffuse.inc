// Generated from vertex_program/modules/diffuse.inc by asm2hlsl. Included inside main().

// ======================================================================
//
// inputs:
//
//   r7       = cell/ambient lighting
//   r9.xyz   = vertex in world space
//   r10.xyz  = normal in world space (re-normalized)
//
// outputs:
//
//   oD0      = diffuse light
//
// ======================================================================
// ----------------------------------------------------------------------
//  calculate diffuse/specular parallel light 1
// calculate diffuse and specular lighting
	r0 = (dot((r10).xyz, (cLightData_parallelSpecular_0_direction).xyz)).xxxx;
	r7 = cExtLtData_parallelSpec_0_tangentColor + r7;
	r1 = max(r0, c0_0);
	r7 = cExtLtData_parallelSpec_0_tangentMinusDiffuse * -(r1.y) + r7;
	r1 = min(r0, c0_0);
	r7 = cExtLtData_parallelSpec_0_tangentMinusBack * r1.y + r7;
// ----------------------------------------------------------------------
// calculate diffuse parallel light 1
	r0 = (dot((r10).xyz, (cLightData_parallel_0_direction).xyz)).xxxx;
	r0 = max(r0, c0_0);
	r7 = r0 * cLightData_parallel_0_diffuseColor + r7;
// ----------------------------------------------------------------------
//  calculate diffuse parallel light 2
	r0 = (dot((r10).xyz, (cLightData_parallel_1_direction).xyz)).xxxx;
	r0 = max(r0, c0_0);
	r7 = r0 * cLightData_parallel_1_diffuseColor + r7;
// ----------------------------------------------------------------------
//  calculate diffuse/specular point light 1
// calculate unit vector from vertex to light and set up for attenuation calculation
	r0.xyz = (cLightData_pointSpecular_0_position - r9).xyz;
	r1 = (dot((r0).xyz, (r0).xyz)).xxxx;
	r2 = (rsqrt(abs(r1.w))).xxxx;
	r0.xyz = (r0 * r2).xyz;
// calculate attenuation
	r1 = float4(1.0f, (r1).y * (r2).y, (r1).z, (r2).w);
	r1 = (dot((r1).xyz, (cLightData_pointSpecular_0_attenuation).xyz)).xxxx;
	r1 = ((1.0f / (r1.w))).xxxx;
// calculate diffuse and specular lighting
	r0 = (dot((r10).xyz, (r0).xyz)).xxxx;
	r0 = max(r0, c0_0);
	r0 = r0 * r1;
	r7 = cLightData_pointSpecular_0_diffuseColor * r0.y + r7;
// ----------------------------------------------------------------------
//  calculate diffuse point light 1
// calculate unit vector from vertex to light and set up for attenuation calculation
	r0.xyz = (cLightData_point_0_position - r9).xyz;
	r1 = (dot((r0).xyz, (r0).xyz)).xxxx;
	r2 = (rsqrt(abs(r1.w))).xxxx;
	r0.xyz = (r0 * r2).xyz;
// calculate attenuation
	r1 = float4(1.0f, (r1).y * (r2).y, (r1).z, (r2).w);
	r1 = (dot((r1).xyz, (cLightData_point_0_attenuation).xyz)).xxxx;
	r1 = ((1.0f / (r1.w))).xxxx;
// calculate diffuse and  lighting
	r0 = (dot((r10).xyz, (r0).xyz)).xxxx;
	r0 = max(r0, c0_0);
	r0 = r0 * r1;
	r7 = cLightData_point_0_diffuseColor * r0 + r7;
// ----------------------------------------------------------------------
//  calculate diffuse point light 2
// calculate unit vector from vertex to light and set up for attenuation calculation
	r0.xyz = (cLightData_point_1_position - r9).xyz;
	r1 = (dot((r0).xyz, (r0).xyz)).xxxx;
	r2 = (rsqrt(abs(r1.w))).xxxx;
	r0.xyz = (r0 * r2).xyz;
// calculate attenuation
	r1 = float4(1.0f, (r1).y * (r2).y, (r1).z, (r2).w);
	r1 = (dot((r1).xyz, (cLightData_point_1_attenuation).xyz)).xxxx;
	r1 = ((1.0f / (r1.w))).xxxx;
// calculate diffuse and  lighting
	r0 = (dot((r10).xyz, (r0).xyz)).xxxx;
	r0 = max(r0, c0_0);
	r0 = r0 * r1;
	r7 = cLightData_point_1_diffuseColor * r0 + r7;
// ----------------------------------------------------------------------
//  calculate diffuse point light 3
// calculate unit vector from vertex to light and set up for attenuation calculation
	r0.xyz = (cLightData_point_2_position - r9).xyz;
	r1 = (dot((r0).xyz, (r0).xyz)).xxxx;
	r2 = (rsqrt(abs(r1.w))).xxxx;
	r0.xyz = (r0 * r2).xyz;
// calculate attenuation
	r1 = float4(1.0f, (r1).y * (r2).y, (r1).z, (r2).w);
	r1 = (dot((r1).xyz, (cLightData_point_2_attenuation).xyz)).xxxx;
	r1 = ((1.0f / (r1.w))).xxxx;
// calculate diffuse and  lighting
	r0 = (dot((r10).xyz, (r0).xyz)).xxxx;
	r0 = max(r0, c0_0);
	r0 = r0 * r1;
	r7 = cLightData_point_2_diffuseColor * r0 + r7;
// ----------------------------------------------------------------------
//  calculate diffuse point light 4
// calculate unit vector from vertex to light and set up for attenuation calculation
	r0.xyz = (cLightData_point_3_position - r9).xyz;
	r1 = (dot((r0).xyz, (r0).xyz)).xxxx;
	r2 = (rsqrt(abs(r1.w))).xxxx;
	r0.xyz = (r0 * r2).xyz;
// calculate attenuation
	r1 = float4(1.0f, (r1).y * (r2).y, (r1).z, (r2).w);
	r1 = (dot((r1).xyz, (cLightData_point_3_attenuation).xyz)).xxxx;
	r1 = ((1.0f / (r1.w))).xxxx;
// calculate diffuse and  lighting
	r0 = (dot((r10).xyz, (r0).xyz)).xxxx;
	r0 = max(r0, c0_0);
	r0 = r0 * r1;
	r7 = cLightData_point_3_diffuseColor * r0 + r7;
// ----------------------------------------------------------------------
//  store final diffuse and specular lighting colors, add in material emissive color
	r7 = r7 + cMaterial_emissiveColor;
	vsOutput.color0 = r7;
// ======================================================================
