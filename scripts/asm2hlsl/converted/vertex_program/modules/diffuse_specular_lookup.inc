// Generated from vertex_program/modules/diffuse_specular_lookup.inc by asm2hlsl. Included inside main().

// ======================================================================
//
// inputs:
//
//   r7       = cell/ambient lighting
//   r9.xyz   = vertex in world space
//   r10.xyz  = normal in world space (re-normalized)
//   r11.xyz  = unit vector from vertex to camera
//
// outputs:
//
//   oD0      = diffuse light
//   oD1      = specular light
//
// ======================================================================
// ----------------------------------------------------------------------
//  calculate diffuse/specular parallel light 1
// calculate the half angle (H = L + V / |L + V|)
	r1.xyz = (cLightData_parallelSpecular_0_direction + r11).xyz;
	r1.w = ((dot((r1).xyz, (r1).xyz)).xxxx).w;
	r1.w = ((rsqrt(abs(r1.w))).xxxx).x;
	r1.xyz = (r1 * r1.w).xyz;
//H dot N into a UV2 for the specular lookup
	vsOutput.texcoord2.xyz = ((dot((r1).xyz, (r10).xyz)).xxxx).xyz;
// calculate diffuse and specular lighting
	r0.x = ((dot((r10).xyz, (cLightData_parallelSpecular_0_direction).xyz)).xxxx).x;
	r0.y = ((dot((r10).xyz, (r1).xyz)).xxxx).y;
	r0.w = (cMaterial_specularPower).x;
	r0 = lit((r0).x, (r0).y, (r0).w);
	r0 = (dot((r10).xyz, (cLightData_parallelSpecular_0_direction).xyz)).xxxx;
	r7 = cExtLtData_parallelSpec_0_tangentColor + r7;
	r1 = max(r0, c0_0);
	r7 = cExtLtData_parallelSpec_0_tangentMinusDiffuse * -(r1.y) + r7;
	r1 = min(r0, c0_0);
	r7 = cExtLtData_parallelSpec_0_tangentMinusBack * r1.y + r7;
//mul r8, cLightData_parallelSpecular_0_specularColor, r0.z
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
// calculate the half angle (H = L + V / |L + V|)
	r2.xyz = (r0 + r11).xyz;
	r2.w = ((dot((r2).xyz, (r2).xyz)).xxxx).w;
	r2.w = ((rsqrt(abs(r2.w))).xxxx).x;
	r2.xyz = (r2 * r2.w).xyz;
// calculate diffuse and specular lighting
	r0.x = ((dot((r10).xyz, (r0).xyz)).xxxx).x;
	r0.y = ((dot((r10).xyz, (r2).xyz)).xxxx).y;
	r0.w = (cMaterial_specularPower).x;
	r0 = lit((r0).x, (r0).y, (r0).w);
	r0 = r0 * r1;
	r7 = cLightData_pointSpecular_0_diffuseColor * r0.y + r7;
//mad r8, cLightData_pointSpecular_0_specularColor, r0.z, r8
	r8 = cLightData_pointSpecular_0_specularColor * r0.z;
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
//mov oD0, r7
	vsOutput.color0 = r7 + cMaterial_emissiveColor;
	vsOutput.color1 = r8 * cMaterial_specularColor;
// ======================================================================
