// Generated from vertex_program/modules/registers.inc by asm2hlsl.
//
// The symbolic names are unchanged so the converted modules read like the assembly
// they came from. Constants resolve into a flat float4 c[96] at register(c0): fxc
// places that at $Globals offset 0, size 1536, so element i is at byte 16i, which is
// byte-identical to the register file the engine already uploads.

float4 c[96] : register(c0);

#define VERTEX_SHADER_VERSION 20

// A texture coordinate output is a whole register in assembly, but the input side is
// whatever width the runtime's DECLARE_textureCoordinateSets gave it: float2 for an
// ordinary set and float4 for a DOT3 one, decided per vertex buffer. These overloads
// let the generated code assign one to the other without knowing which it got.
float4 asmTexcoord(float2 v) { return float4(v, 0.0f, 0.0f); }
float4 asmTexcoord(float3 v) { return float4(v, 0.0f); }
float4 asmTexcoord(float4 v) { return v; }

#define cObjectWorldCameraProjectionMatrix             c[0]
#define cObjectWorldMatrix                             c[4]
#define cCameraPosition                                c[8]
#define cViewportData                                  c[9]
#define cFog                                           c[10]
#define cMaterial_diffuseColor                         c[11]
#define cMaterial_ambientColor                         c[12]
#define cMaterial_specularColor                        c[13]
#define cMaterial_emissiveColor                        c[14]
#define cMaterial_specularPower                        c[15].x
#define cLightData_ambient_ambientColor                c[16]
#define cLightData_parallelSpecular_0_direction        c[17]
#define cLightData_parallelSpecular_0_diffuseColor     c[18]
#define cLightData_parallelSpecular_0_specularColor    c[19]
#define cLightData_parallel_0_direction                c[20]
#define cLightData_parallel_0_diffuseColor             c[21]
#define cLightData_parallel_1_direction                c[22]
#define cLightData_parallel_1_diffuseColor             c[23]
#define cLightData_pointSpecular_0_position            c[24]
#define cLightData_pointSpecular_0_diffuseColor        c[25]
#define cLightData_pointSpecular_0_attenuation         c[26]
#define cLightData_pointSpecular_0_specularColor       c[27]
#define cLightData_point_0_position                    c[28]
#define cLightData_point_0_diffuseColor                c[29]
#define cLightData_point_0_attenuation                 c[30]
#define cLightData_point_1_position                    c[31]
#define cLightData_point_1_diffuseColor                c[32]
#define cLightData_point_1_attenuation                 c[33]
#define cLightData_point_2_position                    c[34]
#define cLightData_point_2_diffuseColor                c[35]
#define cLightData_point_2_attenuation                 c[36]
#define cLightData_point_3_position                    c[37]
#define cLightData_point_3_diffuseColor                c[38]
#define cLightData_point_3_attenuation                 c[39]
#define cLightData_dot3_0_cameraPosition               c[40]
#define cLightData_dot3_0_direction                    c[41]
#define cLightData_dot3_0_diffuseColor                 c[42]
#define cLightData_dot3_0_specularColor                c[43]
#define cTextureFactor                                 c[44]
#define cTextureFactor2                                c[45]
#define cTextureScroll                                 c[47]
#define cCurrentTime                                   c[48]
#define cUnitX                                         c[49]
#define cUnitY                                         c[50]
#define cUnitZ                                         c[51]
#define cUserConstant0                                 c[52]
#define cUserConstant1                                 c[53]
#define cUserConstant2                                 c[54]
#define cUserConstant3                                 c[55]
#define cUserConstant4                                 c[56]
#define cUserConstant5                                 c[57]
#define cUserConstant6                                 c[58]
#define cUserConstant7                                 c[59]
#define cExtLtData_parallelSpec_0_backColor            c[60]
#define cExtLtData_parallelSpec_0_tangentColor         c[61]
#define cExtLtData_parallelSpec_0_tangentMinusBack     c[62]
#define cExtLtData_parallelSpec_0_tangentMinusDiffuse  c[63]
#define cExtLtData_dot3_0_backColor                    c[64]
#define cExtLtData_dot3_0_tangentColor                 c[65]
#define cExtLtData_dot3_0_tangentMinusBack             c[66]
#define cExtLtData_dot3_0_tangentMinusDiffuse          c[67]
#define c0_0                                           0.0f
#define c0_5                                           0.5f
#define c1_0                                           1.0f
#define cLog2e                                         1.4426950408889634f

// The three symbols ILM_visuals.tre's diffuse.inc uses and never defines. They map
// onto the engine's extendedLightData block at c60..c63, whose HemisphericLightData
// fields are backColor, tangentColor, tangentMinusBackColor and
// tangentMinusDiffuseColor in that order (Direct3d9_LightManager.h).
#define cExtLtData_parallelSpec_0_backColor            c[60]
#define cExtLtData_parallelSpec_0_tangentColor         c[61]
#define cExtLtData_parallelSpec_0_tangentMinusBack     c[62]
#define cExtLtData_parallelSpec_0_tangentMinusDiffuse  c[63]

