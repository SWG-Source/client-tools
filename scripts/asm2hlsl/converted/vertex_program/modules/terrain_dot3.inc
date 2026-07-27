// Generated from vertex_program/modules/terrain_dot3.inc by asm2hlsl. Included inside main().

// -- take vertex color into account for diffuse lighting
	vsOutput.color0 = r7 * vColor0;
// -- store dot3 light modulated by vertex color
	vsOutput.color1 = cLightData_dot3_0_diffuseColor * vColor0;
// -- transform the light direction into texture space, sign and bias
//m3x3 r0.xyz, cLightData_dot3_0_direction, vTextureCoordinateSet0
//mad oT2.xyz, r0, c0_5, c0_5
// -- r10 = j (-z) = normal.cross (unitX)
	r10 = vNormal.yzxw * cUnitX.zxyw;
	r10 = -(cUnitX.yzxw) * vNormal.zxyw + r10;
// -- r9  = i (x) = j.cross (normal)
	r9 = r10.yzxw * vNormal.zxyw;
	r9 = -(vNormal.yzxw) * r10.zxyw + r9;
//m3x3 r5, v0, c[3]
	r0.x = ((dot((cLightData_dot3_0_direction).xyz, (r9).xyz)).xxxx).x;
	r0.y = ((dot((cLightData_dot3_0_direction).xyz, (r10).xyz)).xxxx).y;
	r0.z = ((dot((cLightData_dot3_0_direction).xyz, (vNormal).xyz)).xxxx).z;
	vsOutput.texcoord2.xyz = (r0 * c0_5 + c0_5).xyz;
