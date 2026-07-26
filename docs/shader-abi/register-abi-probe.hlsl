// Mimics the shipped .inc pattern: constants annotated with explicit registers,
// only SOME of which the shader body reads, plus samplers with a gap.
float4x4 objectWorldCameraProjectionMatrix : register(c0);
float4x4 objectWorldMatrix                 : register(c4);
float4   cameraPosition                    : register(c8);
float4   viewportData                      : register(c9);
float4   fog                               : register(c10);
float4   textureFactor                     : register(c44);
float4   currentTime                       : register(c48);
float4   literalConstants                  : register(c95);

sampler mainSampler   : register(s0);
sampler skipMe        : register(s1);   // declared but never sampled
sampler detailSampler : register(s2);
sampler thirdSampler  : register(s3);

struct VsOut { float4 pos : POSITION0; float4 col : COLOR0; };

VsOut main(float4 p : POSITION, float2 uv : TEXCOORD0)
{
    VsOut o;
    o.pos = mul(p, objectWorldCameraProjectionMatrix);
    // read only a few, to test whether unreferenced ones keep their slots
    o.col = fog + literalConstants + tex2Dlod(mainSampler, float4(uv,0,0)) + tex2Dlod(detailSampler, float4(uv,0,0)) + tex2Dlod(thirdSampler, float4(uv,0,0));
    return o;
}
