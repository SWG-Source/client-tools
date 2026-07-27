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
	float4 normal : NORMAL0;
	DECLARE_textureCoordinateSets
};

struct VsOutput
{
	float4 position : POSITION0;
	float4 texcoord0 : TEXCOORD0;
	float4 texcoord1 : TEXCOORD1;
};

// The assembly's symbolic input names.
#define vPosition vsInput.position
#define vNormal vsInput.normal

VsOutput main(VsInput vsInput)
{
	VsOutput vsOutput = (VsOutput)0;
	float4 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11;
	r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = r8 = r9 = r10 = r11 = 0.0f;

#include "vertex_program/modules/transform.inc"
//-- transform the normal into world space
	r0.xyz = (float3(dot((vNormal).xyz, (c[4]).xyz), dot((vNormal).xyz, (c[5]).xyz), dot((vNormal).xyz, (c[6]).xyz))).xyz;
//-- compute direction to eye
	r1.xyz = (float3(dot(vPosition, c[4]), dot(vPosition, c[5]), dot(vPosition, c[6]))).xyz;
	r2.xyz = (cCameraPosition - r1).xyz;
//-- normalize direction to eye
	r2.w = ((dot((r2).xyz, (r2).xyz)).xxxx).w;
	r2.w = ((rsqrt(abs(r2.w))).xxxx).x;
	r2.xyz = (r2 * r2.w).xyz;
//-- compute texture coordinates for low angle fade
	vsOutput.texcoord1.x = ((dot((r0).xyz, (r2).xyz)).xxxx).x;
	vsOutput.texcoord1.y = (c0_0).x;
	vsOutput.texcoord0 = asmTexcoord(vsInput.textureCoordinateSetMAIN);

	return vsOutput;
}
