#ifdef __cplusplus
using float3 = math::float3;
using float4 = math::float4;
using float4x4 = math::mat44;
#endif

#define TGSIZE 32
struct constants
{
	float4x4 m_viewprojection;
};

#ifndef __cplusplus
struct vs_input
{
	float3 m_position : SV_POSITION;
};
struct vs_output
{
	float4 m_position : SV_POSITION;
	float4 m_colour : COLOR;
}; 
struct gbuffer
{
	float4 	m_albedo : SV_TARGET0;
	float 	m_depth : SV_TARGET1;
};

ConstantBuffer<constants>	g_consts		; // : register(b0);
Texture2D					g_textures[4]	; // : register(t0);
StructuredBuffer<float3>	g_structs		; // : register(t5);
SamplerState				g_samplers[4]	; // : register(s0);

[shader("vertex")]
vs_output main_vs(vs_input input)
{
	vs_output result = (vs_output)0;
	result.m_position = mul(float4(input.m_position, 1.0f), g_consts.m_viewprojection);
	result.m_colour.r = determinant(g_consts.m_viewprojection);
	result.m_colour.g = g_textures[0].SampleLevel(g_samplers[0], float2(0,0), 0).r;
	result.m_colour.b = g_structs[0][0].r;
	return result;
}

RWTexture2D<float3>			g_textures_rw[2]	; //
RWStructuredBuffer<float3>  g_structs_rw[2]		; //
Texture2DArray				g_texarray			; //: register(t2);

[shader("pixel")]
gbuffer main_ps(vs_output input)
{
	input.m_colour.a = g_texarray.Sample(g_samplers[1], float3(0,0,1)).r;

	g_structs_rw[0][0] = float3(1,0,0);
	g_textures_rw[0][uint2(0,0)] = float3(1,0,0);

	gbuffer result;
	result.m_albedo = input.m_colour;
	result.m_depth = input.m_position.z;
	return result;
}

Texture2D<float4> tex_gbcolor;
Texture2D<float> tex_gbdepth;
RWTexture2D<float4> tex_output;

[shader("compute")]
[numthreads(TGSIZE, TGSIZE, 1)]
void main_cs(uint3 tid : SV_DispatchThreadID)
{
	tex_output[tid.xy].rgba = tex_gbcolor.Load(0, 0).rgba;
}
#endif // __cplusplus
