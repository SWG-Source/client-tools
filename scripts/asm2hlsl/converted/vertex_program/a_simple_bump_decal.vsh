//hlsl vs_2_0
#define textureCoordinateSetMAIN     textureCoordinateSet0
#define textureCoordinateSetNRML     textureCoordinateSet1
#define textureCoordinateSetDCAL     textureCoordinateSet2
#define textureCoordinateSetDOT3     textureCoordinateSet3
#define DECLARE_textureCoordinateSets	\
	float4 textureCoordinateSet0 : TEXCOORD0;\
	float4 textureCoordinateSet1 : TEXCOORD1;\
	float4 textureCoordinateSet2 : TEXCOORD2;\
	float4 textureCoordinateSet3 : TEXCOORD3;

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
	float4 color0 : COLOR0;
	float4 color1 : COLOR1;
	float fog : FOG;
	float4 texcoord0 : TEXCOORD0;
	float4 texcoord1 : TEXCOORD1;
	float4 texcoord2 : TEXCOORD2;
	float4 texcoord3 : TEXCOORD3;
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
#include "vertex_program/modules/lighting_fog_setup.inc"
#include "vertex_program/modules/fog.inc"
#include "vertex_program/modules/ambient.inc"
#include "vertex_program/modules/dot3_diffuse.inc"
#include "vertex_program/modules/dot3.inc"
// ----------------------------------------------------------------------
//  calculate diffuse/specular parallel light 1 so I can send two different lighting values into pixel shader
// calculate diffuse and specular lighting
	r0 = (dot((r10).xyz, (cLightData_parallelSpecular_0_direction).xyz)).xxxx;
	r0 = max(r0, c0_0);
	vsOutput.color1 = cLightData_parallelSpecular_0_diffuseColor * r0.y + r7;
	vsOutput.texcoord2 = asmTexcoord(vsInput.textureCoordinateSetMAIN);
	vsOutput.texcoord3 = asmTexcoord(vsInput.textureCoordinateSetDCAL);

	return vsOutput;
}
