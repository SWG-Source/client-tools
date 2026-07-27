//hlsl vs_2_0

// Converted from Direct3D 9 vertex assembly by asm2hlsl. The module structure of the
// original is preserved: each #include below is a block of statements inside main(),
// exactly as the assembly had it.

#include "vertex_program/include/asm_constants.inc"

struct VsInput
{
	float4 position : POSITION0;
	float4 normal : NORMAL0;
	float4 textureCoordinateSet0 : TEXCOORD0;
	float4 textureCoordinateSet1 : TEXCOORD1;
};

struct VsOutput
{
	float4 position : POSITION0;
	float4 color0 : COLOR0;
	float fog : FOG;
	float4 texcoord0 : TEXCOORD0;
	float4 texcoord1 : TEXCOORD1;
	float4 texcoord2 : TEXCOORD2;
};

// The assembly's symbolic input names.
#define vPosition vsInput.position
#define vNormal vsInput.normal

VsOutput main(VsInput vsInput)
{
	VsOutput vsOutput = (VsOutput)0;
	float4 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11;
	r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = r8 = r9 = r10 = r11 = 0.0f;

// r7  = cummulative diffuse lighting
// r8  = cummulative specular lighting
// r9  = vertex in world space
// r10 = normal in world space
// r11 = vector to viewer
// -- transform vertex to projection space
	vsOutput.position = float4(dot(vPosition, c[0]), dot(vPosition, c[1]), dot(vPosition, c[2]), dot(vPosition, c[3]));
// -- transform vertex position into world space
	r9.xyz = (float3(dot(vPosition, c[4]), dot(vPosition, c[5]), dot(vPosition, c[6]))).xyz;
// -- rotate vertex normal into world space and renormalize
	r10.xyz = (float3(dot((vNormal).xyz, (c[4]).xyz), dot((vNormal).xyz, (c[5]).xyz), dot((vNormal).xyz, (c[6]).xyz))).xyz;
	r10.w = ((dot((r10).xyz, (r10).xyz)).xxxx).w;
	r10.w = ((rsqrt(abs(r10.w))).xxxx).x;
	r10.xyz = (r10 * r10.w).xyz;
// -- calculate the direction to the viewer
	r11.xyz = (cCameraPosition - r9).xyz;
	r11.w = ((dot((r11).xyz, (r11).xyz)).xxxx).w;
	r0.w = ((rsqrt(abs(r11.w))).xxxx).x;
	r11.xyz = (r11 * r0.w).xyz;
// -- calculate fog
// (distance)^2 * (density)^2 =
// (distance * density)^2
	r0.w = (r11.w * cFog.w).x;
// log2(e) * (distance * density)^2
	r0.w = (cLog2e * r0.w).x;
// 2^(log2(e) * (distance * density)^2) =
// (2^log2(e))^((distance * density)^2)
// e^((distance * density)^2)
	r0.w = ((exp2(r0.w)).xxxx).x;
// 1 / (e^((distance * density)^2))
	vsOutput.fog = ((1.0f / (r0.w))).xxxx;
// -- calculate ambient light
	r7 = cLightData_ambient_ambientColor;
// -- calculate diffuse & specular parallel light 1
	r0 = (dot((r10).xyz, (cLightData_parallelSpecular_0_direction).xyz)).xxxx;
	r0 = max(r0, c0_0);
	r7 = r0 * cLightData_parallelSpecular_0_diffuseColor + r7;
// -- calculate diffuse parallel light 1
	r0 = (dot((r10).xyz, (cLightData_parallel_0_direction).xyz)).xxxx;
	r0 = max(r0, c0_0);
	r7 = r0 * cLightData_parallel_0_diffuseColor + r7;
// -- calculate diffuse parallel light 2
	r0 = (dot((r10).xyz, (cLightData_parallel_1_direction).xyz)).xxxx;
	r0 = max(r0, c0_0);
	r7 = r0 * cLightData_parallel_1_diffuseColor + r7;
// -- store final colors
	vsOutput.color0 = r7;
// -- set up diffuse texture coordinates
	vsOutput.texcoord0.xy = (vsInput.textureCoordinateSet0.xy + cTextureScroll.xy).xy;
// -- set up normal map texture coordinates
	vsOutput.texcoord1.xy = (vsInput.textureCoordinateSet0.xy + cTextureScroll.xy).xy;
// -- transform the light direction into texture space, sign and bias
	r0.xyz = (vsInput.textureCoordinateSet1).xyz;
	r2.xyz = (vNormal).xyz;
	r1.xyz = (r2.zxy * r0.yzx).xyz;
	r1.xyz = (r2.yzx * r0.zxy + -(r1.xyz)).xyz;
	r1.xyz = (r1 * vsInput.textureCoordinateSet1.w).xyz;
	r0.xyz = (float3(dot((cLightData_dot3_0_direction).xyz, (r0).xyz), dot((cLightData_dot3_0_direction).xyz, (r1).xyz), dot((cLightData_dot3_0_direction).xyz, (r2).xyz))).xyz;
	vsOutput.texcoord2.xyz = (r0 * c0_5 + c0_5).xyz;

	return vsOutput;
}
