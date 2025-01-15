#include "common.hlsli"

/// SHADER STRUCTS
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
};

struct per_instance_data
{
    float4x4	mat_transform;
    float4      colour;
    bool        invert_normals;
};

struct vs_input
{
    float3 position : POSITION;
    float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct ps_input
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    nointerpolation float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

/// SHADER INPUTS
// buffers
ConstantBuffer<per_view>                g_perview           : register(b0);
ConstantBuffer<per_scene>               g_perscene          : register(b1);
ConstantBuffer<per_material>            g_permaterial       : register(b2);
ConstantBuffer<per_draw>                g_perdraw           : register(b3);

// samplers
SamplerState                            g_sampler           : register(s0);

// textures
Texture2D                               g_textures[4]       : register(t0);
StructuredBuffer<per_instance_data>     g_instancebuffer    : register(t1);

/// SHADER FUNCTIONS
float4 get_albedo(float2 texcoord)
{
    return g_textures[0].Sample(g_sampler, texcoord).rgba;
}

float3 get_normal(float2 texcoord)
{
    return g_textures[1].Sample(g_sampler, texcoord).rgb;
}

[shader("vertex")]
ps_input main_vs( vs_input input, uint vID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    ps_input output = (ps_input)0;

    // instance
    instanceID += g_perdraw.start_instance;
    per_instance_data instance_data = g_instancebuffer[instanceID];
    
    // position
    // calculate mvp
    float4x4 mvp = mul(
        (float4x4)g_perview.mat_vp,
        (float4x4)instance_data.mat_transform);
    output.position = mul(mvp, float4 ( input.position, 1.0f ) );
    output.worldPos = mul(instance_data.mat_transform, float4(input.position, 1.0f)).xyz;

    // uvs
    output.texcoord = input.texcoord;
    
    // normals
    output.normal = normalize(mul((float3x3)instance_data.mat_transform, input.normal));
    if (instance_data.invert_normals)
    {
        output.normal = -output.normal;
    }

    // color
    output.colour.rgb = lerp(instance_data.colour.rgb, g_permaterial.colour.rgb, 0.5f);
    
    return output;
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
