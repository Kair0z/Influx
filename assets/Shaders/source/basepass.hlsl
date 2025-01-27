#include "common.hlsli"

struct per_view
{
    float4x4 mat_vp;
};

struct per_scene
{
    float4 time;
    float4 light_direction;
    float4 light_colour;
};

struct per_material
{
    float4 colour;
};

struct per_draw
{
    uint start_instance;
    uint start_vertex;
};

struct per_vertex_data
{
    float3 position : POSITION;
    float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct per_instance_data
{
    float4x4	mat_transform;
    float4      colour;
    uint        base_vertex;
};

struct ps_input
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    nointerpolation float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

#define FLX_BINDLESS 1

/// SHADER INPUTS
// constant buffers:
ConstantBuffer<per_view>                g_perview           : register(b0);
ConstantBuffer<per_scene>               g_perscene          : register(b1);
ConstantBuffer<per_material>            g_permaterial       : register(b2);
ConstantBuffer<per_draw>                g_perdraw           : register(b3);

// samplers
SamplerState                            g_sampler           : register(s0);

// textures
StructuredBuffer<per_instance_data>     g_instancebuffer    : register(t1);
StructuredBuffer<per_vertex_data>       g_vertexbuffers     : register(t2);

Texture2D get_texture(int index)
{
    return ResourceDescriptorHeap[2];
}

SamplerState get_sampler(int index)
{
    return SamplerDescriptorHeap[index];
}

StructuredBuffer<per_instance_data> get_instance_buffer()
{
    return ResourceDescriptorHeap[0];
}

StructuredBuffer<per_vertex_data> get_vertex_buffer()
{
    return ResourceDescriptorHeap[1];
}

struct vs_input
{
    uint vertex_id : SV_VertexID;
    uint instance_id : SV_InstanceID;

    per_vertex_data get_vertex_data()
    {
        return g_vertexbuffers[vertex_id];
    }

    per_instance_data get_instance_data()
    {
        return g_instancebuffer[instance_id];
    }
};

[shader("vertex")]
ps_input main_vs(vs_input input)
{
    ps_input output = (ps_input)0;

    // get instance data
    per_instance_data instance_data = get_instance_buffer()[0];
    
    // get vertex data
    per_vertex_data vertex_data = get_vertex_buffer()[input.vertex_id];

    // positions
    float4x4 mvp = mul((float4x4)g_perview.mat_vp, (float4x4)instance_data.mat_transform);
    output.position = mul(mvp, float4 ( vertex_data.position, 1.0f ) );
    output.worldPos = mul(instance_data.mat_transform, float4(vertex_data.position, 1.0f)).xyz;

    // uvs
    output.texcoord = vertex_data.texcoord;
    
    // normals
    output.normal = normalize(mul((float3x3)instance_data.mat_transform, vertex_data.normal));

    // color
    output.colour.rgb = lerp(instance_data.colour.rgb, g_permaterial.colour.rgb, 0.5f);
    
    return output;
}




float4 get_albedo(float2 texcoord)
{
    return get_texture(0).Sample(get_sampler(0), texcoord).rgba;
}

float3 get_normal(float2 texcoord)
{
    return get_texture(0).Sample(get_sampler(0), texcoord).rgb;
}

[shader("pixel")]
float4 main_ps(ps_input input) : SV_TARGET
{
    float3 lightDir = g_perscene.light_direction.rgb;
    float3 lightCol = g_perscene.light_colour.rgb;

    float4 albedo = get_albedo(input.texcoord).rgba;
    // albedo.rgb = lerp(input.colour.rgb, albedo.rgb, 0.5f);

    float3 normal = get_normal(input.texcoord).rgb;
    normal = input.normal;

    float ambient = 0.2f;
    float diffuse = max(ambient, dot(normalize(normal), normalize(lightDir)));

    return float4(albedo.rgb, 1.0f);
}
