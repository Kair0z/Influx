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

ConstantBuffer<constants> g_consts;
[shader("vertex")]
vs_output main_vs(vs_input input)
{
	vs_output result = (vs_output)0;
	result.m_position = mul(float4(input.m_position, 1.0f), g_consts.m_viewprojection);
	return result;
}

[shader("pixel")]
gbuffer main_ps(vs_output input)
{
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
