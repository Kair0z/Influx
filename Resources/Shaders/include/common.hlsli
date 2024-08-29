#ifndef _INFLUX_COMMON_HF
#define _INFLUX_COMMON_HF

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

struct vs_constants
{
    float4x4 mat_vp;
    float4x4 mat_mvp;
};

struct ps_constants
{
    float seconds;
    float delta_seconds;
};

struct per_instance_data
{
    float4x4	mat_transform;
    float4      colour;
};

// constant buffers
ConstantBuffer<vs_constants> g_perframe_vs : register(b0);
ConstantBuffer<ps_constants> g_perframe_ps : register(b0);

// structured buffers
StructuredBuffer<per_instance_data> g_instancebuffer;

// root descriptors
// ...

// root samplers
SamplerState g_sampler : register(s0);

// descriptor tables
Texture2D g_textures[4] : register(t0);


// global functions
float4 get_albedo(float2 texcoord)
{
    return g_textures[0].Sample(g_sampler, texcoord).rgba;
}

float3 get_normal(float2 texcoord)
{
    return g_textures[1].Sample(g_sampler, texcoord).rgb;
}

float3 hash(uint3 x)
{
    //https://www.shadertoy.com/view/4lXyWN

    //multiply large prime value first.
    uint k = 1103515245U;
    x *= k;
    //mix x, y, z values.
    //Without shift operator, x, y and z value become same value.
    x = ((x >> 2u) ^ (x.yzx >> 1u) ^ x.zxy) * k;

    return (float3(x) * (1.0 / float(0xffffffffU)));
}
#endif