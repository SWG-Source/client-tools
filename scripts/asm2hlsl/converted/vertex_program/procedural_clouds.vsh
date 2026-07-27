//hlsl vs_2_0
#define textureCoordinateSetMAIN     textureCoordinateSet0
#define DECLARE_textureCoordinateSets	\
	float4 textureCoordinateSet0 : TEXCOORD0;

// Converted from Direct3D 9 vertex assembly by asm2hlsl. The module structure of the
// original is preserved: each #include below is a block of statements inside main(),
// exactly as the assembly had it.

#include "vertex_program/include/asm_constants.inc"

struct VsInput
{
	float4 position : POSITION0;
	DECLARE_textureCoordinateSets
};

struct VsOutput
{
	float4 position : POSITION0;
	float4 color0 : COLOR0;
	float fog : FOG;
	float4 texcoord0 : TEXCOORD0;
	float4 texcoord1 : TEXCOORD1;
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
	r1 = c0_5;
	r1 = r1 * r1;
	r1 = r1 * r1;
	r2 = r1 * cCurrentTime;
	vsOutput.texcoord0.x = (vsInput.textureCoordinateSetMAIN.x + r2).x;
	vsOutput.texcoord0.y = (vsInput.textureCoordinateSetMAIN.y + r2).y;
	vsOutput.texcoord1.x = (vsInput.textureCoordinateSetMAIN.x + r2).x;
	vsOutput.texcoord1.y = (vsInput.textureCoordinateSetMAIN.y + r2).y;
//-- compute alpha
	vsOutput.color0 = c1_0;
	vsOutput.color0.w = (cUserConstant.x).x;

	return vsOutput;
}
