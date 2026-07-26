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
};

// The assembly's symbolic input names.
#define vPosition vsInput.position

VsOutput main(VsInput vsInput)
{
	VsOutput vsOutput = (VsOutput)0;
	float4 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11;
	r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = r8 = r9 = r10 = r11 = 0.0f;

#include "vertex_program/modules/transform.inc"
#include "vertex_program/modules/fog_setup.inc"
#include "vertex_program/modules/fog.inc"
	vsOutput.color0 = cMaterial_diffuseColor;
	vsOutput.texcoord0 = asmTexcoord(vsInput.textureCoordinateSetMAIN);

	return vsOutput;
}
