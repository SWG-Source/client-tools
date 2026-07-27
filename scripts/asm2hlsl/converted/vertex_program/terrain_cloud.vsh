//hlsl vs_2_0

// Converted from Direct3D 9 vertex assembly by asm2hlsl. The module structure of the
// original is preserved: each #include below is a block of statements inside main(),
// exactly as the assembly had it.

#include "vertex_program/include/asm_constants.inc"

struct VsInput
{
	float4 position : POSITION0;
};

struct VsOutput
{
	float4 position : POSITION0;
	float fog : FOG;
	float4 texcoord0 : TEXCOORD0;
};

// The assembly's symbolic input names.
#define vPosition vsInput.position

VsOutput main(VsInput vsInput)
{
	VsOutput vsOutput = (VsOutput)0;
	float4 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11;
	r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = r8 = r9 = r10 = r11 = 0.0f;

//-- transform the vertex position into camera space
	vsOutput.position = float4(dot(vPosition, c[0]), dot(vPosition, c[1]), dot(vPosition, c[2]), dot(vPosition, c[3]));
// -- transform vertex position into world space
	r9.xyz = (float3(dot(vPosition, c[4]), dot(vPosition, c[5]), dot(vPosition, c[6]))).xyz;
// -- calculate the direction to the viewer
	r11.xyz = (cCameraPosition - r9).xyz;
	r11.w = ((dot((r11).xyz, (r11).xyz)).xxxx).x;
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
//-- compute texture coordinates
	r1 = c1_0;
	r1 = r1 + r1;
	r1 = r1 * r1;
	r1 = r1 * r1;
	r1 = r1 * r1;
	r1 = ((1.0f / (r1.w))).xxxx;
	r1.x = (r1.x * vPosition.x).x;
	r1.z = (r1.z * vPosition.z).x;
	r2 = c0_5;
	r2 = r2 * r2;
	r2 = r2 * r2;
	r2 = r2 * r2;
	r2 = r2 * cCurrentTime;
	vsOutput.texcoord0.x = (r1.x + r2.x).x;
	vsOutput.texcoord0.y = (r1.z + -(r2.z)).x;

	return vsOutput;
}
