#include "include/common.hlsli"
#include "frontend.h"

struct vs_input 
{
    float3 position : POSITION;
    float4 colour : COLOR;
};

struct ps_input 
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
};

ConstantBuffer<per_view>                    g_perview           : register(b0);
StructuredBuffer<line_gpu_instance_data>    g_instancebuffer    : register(t1);

[shader("vertex")]
ps_input main_vs(vs_input input, uint instanceID : SV_InstanceID)
{
    ps_input result;
    line_gpu_instance_data instance_data = g_instancebuffer[instanceID];

    float3 world_start = instance_data.m_start_wp.xyz;
    float3 world_end = instance_data.m_end_wp.xyz;
    float3 world_position = lerp(world_start, world_end, input.position.x);
    
    result.position = mul(g_perview.m_viewprojection, float4(world_position.xyz, 1.0f));
    result.colour = instance_data.m_colour;

    return result;
}

[shader("pixel")]
float4 main_ps(ps_input input) : SV_TARGET
{
    return input.colour;
}