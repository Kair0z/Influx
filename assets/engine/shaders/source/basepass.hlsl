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

StructuredBuffer<per_instance> get_instance_buffer()
{
    return ResourceDescriptorHeap[0];
}

[shader("vertex")]
ps_input main_vs(vs_input input, uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID)
{
    ps_input output = (ps_input)0;

    // get instance data
    instance_id += g_perdraw.m_base_instance;
    per_instance instance_data = get_instance_buffer()[instance_id];
    
    // get vertex data
    per_vertex_data vertex_data = input.vertex_data;

    // positions
    float4x4 instance_transform = (float4x4)instance_data.m_transform;
    float4x4 mvp = mul((float4x4)g_perview.m_viewprojection, instance_transform);
    output.position = mul(mvp, float4 ( vertex_data.position, 1.0f ) );
    output.worldPos = mul(instance_transform, float4(vertex_data.position, 1.0f)).xyz;

    // uvs
    output.texcoord = vertex_data.texcoord;
    
    // normals
    output.normal = normalize(mul((float3x3)instance_transform, vertex_data.normal));

    // color
    float4 instance_color = instance_data.m_colour;
    output.colour.rgb = lerp(instance_color.rgb, g_permaterial.m_colour.rgb, 0.5f);
    
    // texids
    output.texid_albedo = instance_data.get_albedo_index();
    output.texid_normal = instance_data.get_normal_index();

    return output;
}

[shader("pixel")]
ps_output main_ps(ps_input input)
{
    float4 albedo = get_texture(input.texid_albedo).Sample(get_sampler(0), input.texcoord).rgba;
    // float3 normal = get_normal(input.texcoord).rgb;
    float3 normal = input.normal;

    normal = snap_normal(normal);
    normal = normalize(normal);

    ps_output output = (ps_output)0;
    output.gbuffer_data.set_albedo(lerp(albedo.rgb, input.colour.rgb, 0.5f));
    output.gbuffer_data.set_normal(normal.rgb);
    output.gbuffer_data.set_depth(input.position.z);
    return output;
}
