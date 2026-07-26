//hlsl vs_2_0
#define textureCoordinateSetMAIN     textureCoordinateSet0
#define textureCoordinateSetDETA     textureCoordinateSet1
#define textureCoordinateSetSPEC     textureCoordinateSet2
#define DECLARE_textureCoordinateSets	\
	float4 textureCoordinateSet0 : TEXCOORD0;\
	float4 textureCoordinateSet1 : TEXCOORD1;\
	float4 textureCoordinateSet2 : TEXCOORD2;

// Converted from Direct3D 9 vertex assembly by asm2hlsl. The module structure of the
// original is preserved: each #include below is a block of statements inside main(),
// exactly as the assembly had it.

#include "vertex_program/include/asm_constants.inc"

struct VsInput
{
	float4 position : POSITION0;
	float4 normal : NORMAL0;
	float4 color0 : COLOR0;
	DECLARE_textureCoordinateSets
};

struct VsOutput
{
	float4 position : POSITION0;
	float4 color0 : COLOR0;
	float4 color1 : COLOR1;
	float fog : FOG;
	float4 texcoord0 : TEXCOORD0;
	float4 texcoord1 : TEXCOORD1;
	float4 texcoord2 : TEXCOORD2;
};

// The assembly's symbolic input names.
#define vPosition vsInput.position
#define vNormal vsInput.normal
#define vColor0 vsInput.color0

VsOutput main(VsInput vsInput)
{
	VsOutput vsOutput = (VsOutput)0;
	float4 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11;
	r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = r8 = r9 = r10 = r11 = 0.0f;

#include "vertex_program/modules/transform.inc"
#include "vertex_program/modules/lighting_fog_setup.inc"
#include "vertex_program/modules/fog.inc"
#include "vertex_program/modules/c_ambient.inc"
#include "vertex_program/modules/diffuse_specular.inc"
	vsOutput.texcoord0 = asmTexcoord(vsInput.textureCoordinateSetMAIN);
	vsOutput.texcoord1 = asmTexcoord(vsInput.textureCoordinateSetDETA);
	vsOutput.texcoord2 = asmTexcoord(vsInput.textureCoordinateSetSPEC);

	return vsOutput;
}
