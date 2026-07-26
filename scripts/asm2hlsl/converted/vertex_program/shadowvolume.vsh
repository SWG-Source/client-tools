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
};

// The assembly's symbolic input names.
#define vPosition vsInput.position

VsOutput main(VsInput vsInput)
{
	VsOutput vsOutput = (VsOutput)0;
	float4 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11;
	r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = r8 = r9 = r10 = r11 = 0.0f;

	vsOutput.position = float4(dot(vPosition, c[0]), dot(vPosition, c[1]), dot(vPosition, c[2]), dot(vPosition, c[3]));

	return vsOutput;
}
