#include "include/common.hlsli"
#include "frontend.h"

struct per_vertex_data
{
    float3 position : POSITION;
    float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct vs_input
{
    per_vertex_data vertex_data;
};

struct ps_input
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    nointerpolation float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;

    nointerpolation uint texid_albedo : TEXCOORD1;
    nointerpolation uint texid_normal : TEXCOORD2;
};

struct ps_output
{
    gbuffer gbuffer_data;
};

#define FLX_BINDLESS 1

/// SHADER INPUTS
// constant buffers:
ConstantBuffer<per_view>                g_perview           : register(b0);
ConstantBuffer<per_scene>               g_perscene          : register(b1);
ConstantBuffer<per_material>            g_permaterial       : register(b2);
ConstantBuffer<per_draw>                g_perdraw           : register(b3);

#if !FLX_BINDLESS
SamplerState                            g_sampler           : register(s0);
#endif

[shader("vertex")]
ps_input main_vs(vs_input input, uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID)
{
    return vs_basepass(
        input,
        vertex_id,
        instance_id,
        g_perdraw,
        g_perview,
        g_permaterial,
        ResourceDescriptorHeap[0]);
}

Texture2D get_texture(int index)
{
#if FLX_BINDLESS
    return ResourceDescriptorHeap[1 + index];
#endif
}

SamplerState get_sampler(int index)
{
#if FLX_BINDLESS
    return SamplerDescriptorHeap[index];
#else
    return g_sampler;
#endif
}

[shader("pixel")]
ps_output main_ps(ps_input input)
{
    float4 albedo = get_texture(input.texid_albedo).Sample(get_sampler(0), input.texcoord).rgba;
    // float3 normal = get_normal(input.texcoord).rgb;
    float3 normal = input.normal;
    normal = normalize(normal);

    ps_output output = (ps_output)0;
    output.gbuffer_data.set_albedo(lerp(albedo.rgb, input.colour.rgb, 0.5f));
    output.gbuffer_data.set_normal(normal.rgb);
    output.gbuffer_data.set_depth(input.position.z);
    return output;
}
