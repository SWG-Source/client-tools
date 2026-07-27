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

//-- normalize position
	r0 = vPosition;
	r0.w = ((dot((r0).xyz, (r0).xyz)).xxxx).w;
	r0.w = ((rsqrt(abs(r0.w))).xxxx).x;
	r0.xyz = (r0.xyz * r0.w).xyz;
	r1.y = (r0.y).x;
	r0.y = (r0.y * c0_5).x;
	vsOutput.fog = c1_0;
//-- transform the vertex position into camera space
	vsOutput.position = float4(dot(r0, c[0]), dot(r0, c[1]), dot(r0, c[2]), dot(r0, c[3]));
//-- compute texture coordinates
	vsOutput.texcoord0.x = (cUserConstant0.x).x;
	r1.y = (c1_0 - r1.y).x;
	vsOutput.texcoord0.y = (r1.y).x;

	return vsOutput;
}
