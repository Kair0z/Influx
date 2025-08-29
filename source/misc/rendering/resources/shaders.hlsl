// ================================================================================================
// HLSL/C++ frontend
#ifndef SHADER_FRONTEND_H
#define SHADER_FRONTEND_H
#ifdef __cplusplus
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/math/matrix.h"
typedef influx::math::matrix4x4f    float4x4;
typedef influx::math::float4	    float4;
typedef influx::math::float3		float3;
typedef influx::math::float2		float2;
typedef influx::math::uint4			uint4;
typedef influx::math::uint3			uint3;
typedef influx::math::uint2			uint2;
typedef influx::uint32				uint;
typedef influx::math::int4          int4;
#endif // __cplusplus

#define THREAD_GROUP_SIZE 32

// bindless resource heap
static const uint k_instancebuffer_id   = 0;
static const uint k_albedo_id           = 1;
static const uint k_normals_id          = 2;
static const uint k_final_target_id     = 3;
static const uint k_dirlights_id        = 4;
static const uint k_gbuffer_id          = 5;
static const uint k_gbuffer_num         = 3;
// bindless sampler heap
static const uint k_sampler_id          = 0;

struct per_scene 
{
	float4 m_time;
	float4 m_light_direction;
	float4 m_light_colour;
};
struct per_view
{
	float4x4 m_viewprojection;
};
struct per_material 
{
	float4 m_colour;
};
struct per_draw
{
	uint m_base_instance;
};
struct per_vertex
{
    float3 m_position;
    float4 m_colour;
    float3 m_normal;
    float2 m_texcoord;
};
struct per_instance
{
	float4x4 m_transform;
	float4 m_colour;
    uint4 m_texture_indices;

	void set_albedo_index(uint index) { m_texture_indices[0] = index; }
	uint get_albedo_index() { return m_texture_indices[0]; }
	void set_normal_index(uint index) { m_texture_indices[1] = index; }
	uint get_normal_index() { return m_texture_indices[1]; }
};
struct per_pointlight
{
	float4 m_position;
	float4 m_colour;
	float4 m_attenuation;
};
struct per_spotlight
{
	float4 m_position;
};
struct per_dirlight
{
	float4 m_colour;
};
struct cs_shading_args // 56 dwords
{
    int4        m_texture_desc_indices;
    int4        m_buffer_desc_indices;
    int4        m_skybox_indices;
    float4      m_screen_size;
    float4      m_camera_position;
    float4x4    m_inv_viewprojection;
    float4x4    m_inv_projection;
    int4        m_num_lights;
};
#ifndef __cplusplus // -- end of frontend

// ================================================================================================
// gbuffer struct
// f3 -> uint (10b prec)
uint f3_uint(float3 f3)
{
    uint r = (uint) (f3.x * 1023.0); // 10-bit max value is 1024
    uint g = (uint) (f3.y * 1023.0); // 10-bit max value is 1024
    uint b = (uint) (f3.z * 1023.0); // 10-bit max value is 1024
    return (r) | (g << 10) | (b << 20); // Pack into a single uint
}
float3 uint_f3(uint packed)
{
    const float scale = 1.0f / 1023.0;
    uint r = (packed) & 0x000000003FF;
    uint g = (packed >> 10) & 0x000000003FF;
    uint b = (packed >> 20) & 0x000000003FF;
    return float3(r, g, b) * scale;
}
// 3 x f3 -> uint4 => 96/128bit
uint4 f3_u4(float3 f0, float3 f1, float3 f2, uint extra)
{
    return uint4(f3_uint(f0), f3_uint(f1), f3_uint(f2), extra);
}
void u4_f3s(uint4 value, out float3 f0, out float3 f1, out float3 f2, out float3 extra)
{
    f0 = uint_f3(value.x);
    f1 = uint_f3(value.y);
    f2 = uint_f3(value.z);
    extra = uint_f3(value.w);
}
float2 pack_normal(float3 normal)
{
    float2 result;
    result.xy = normalize(normal.xy) * sqrt(normal.z * 0.5f + 0.5f);
    return float2(normal.x, normal.y);
    return result;
}
float3 unpack_normal(float2 packed)
{
    float3 result;
    result.x = packed.x;
    result.y = packed.y;
    result.z = sqrt(1 - packed.x * packed.x - packed.y * packed.y);
    return result;
    result.z = (packed.x * packed.x + packed.y * packed.y) * 2.0f - 1.0f;
    result.xy = normalize(packed.xy) * sqrt(1.0 - result.z * result.z);
    return result;
}
uint f2_u(float2 value)
{
    uint x = (uint) (value.x * 65535.0) & 0xFFFF;
    uint y = (uint) (value.y * 65535.0) & 0xFFFF;
    return (y << 16) | x;
}
float2 u_f2(uint packed)
{
    uint x = (packed & 0xFFFF);
    uint y = ((packed >> 16) & 0xFFFF);
    return float2(x, y) / 65535.0;
}
struct gbuffer
{
    uint4 albedo_emmisive_pbr   : SV_TARGET0; // rgb8 x 4 = 96/128bit
    uint normal                 : SV_TARGET1; // rg8  x 1 = 16/32bit
    uint depth_stencil          : SV_TARGET2; // d24s8    = 32/32bit
    
    void set_normal(float3 norm)
    {
        float2 packed_normal = pack_normal(norm);
        normal = f2_u(packed_normal);
    }
    float3 get_normal()
    {
        return normalize(unpack_normal(u_f2(normal)));
    }
    void set_albedo(float3 albedo)
    {
        albedo_emmisive_pbr.x = f3_uint(albedo);
    }
    float3 get_albedo()
    {
        uint packed_albedo = albedo_emmisive_pbr.x;
        return uint_f3(packed_albedo);
    }
    void set_emmisive(float3 emmisive)
    {
        albedo_emmisive_pbr.y = f3_uint(emmisive);
    }
    float3 get_emmisive()
    {
        return uint_f3(albedo_emmisive_pbr.y);
    }
    void set_roughness(float roughness)
    {
    }
    float get_roughness()
    {
        return 0.0f;
    }
    void set_ao(float ao)
    {

    }
    float get_ao()
    {
        return 0.0f;
    }
    void set_metallic(float metallic)
    {

    }
    float get_metallic()
    {
        return 0.0f;
    }
    void set_depth(float depth)
    {
        depth_stencil = asuint(depth);
    }
    float get_depth()
    {
        return asfloat(depth_stencil);
    }
    void set_stencil(uint stencil)
    {

    }
    uint get_stencil()
    {
        return 0u;
    }
};

// ================================================================================================
// [vs/ps] shader I/O
struct vs_input
{
    float3 m_position : POSITION;
    float4 m_colour : COLOR;
    float3 m_normal : NORMAL;
    float2 m_texcoord : TEXCOORD;
};
struct ps_input
{
    float4 m_position : SV_POSITION;
    float3 m_worldpos : WORLD_POS;
    float3 m_normal : NORMAL;
    float2 m_texcoord : TEXCOORD;

    nointerpolation float4 m_colour : COLOR;
    nointerpolation uint m_texid_albedo : TEXCOORD1;
    nointerpolation uint m_texid_normal : TEXCOORD2;
};
struct ps_output
{
    gbuffer m_gbuffer;
};

ConstantBuffer<per_view>        g_perview       : register(b0);
ConstantBuffer<per_material>    g_permaterial   : register(b1);
ConstantBuffer<per_draw>        g_perdraw       : register(b2);

// ConstantBuffer<per_scene>       g_perscene      : register(b1);

Texture2D get_texture(int index)                        { return ResourceDescriptorHeap[1 + index]; }
Texture2D get_albedo()                                  { return ResourceDescriptorHeap[k_albedo_id]; }
Texture2D get_normals()                                 { return ResourceDescriptorHeap[k_normals_id]; }
SamplerState get_sampler(int index)                     { return SamplerDescriptorHeap[k_sampler_id]; }
StructuredBuffer<per_instance> get_instance_buffer()    { return ResourceDescriptorHeap[k_instancebuffer_id]; }

// ================================================================================================
// basepass-vs:
[shader("vertex")]
ps_input main_vs(vs_input input, uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID)
{
    ps_input output = (ps_input)0;

    // get instance data
    instance_id += g_perdraw.m_base_instance;
    per_instance instance_data = get_instance_buffer()[instance_id];

    // positions
    float4x4 instance_transform = (float4x4)instance_data.m_transform;
    float4x4 mvp = mul((float4x4)g_perview.m_viewprojection, (float4x4)instance_transform);
    output.m_position = mul(mvp, float4(input.m_position, 1.0f));
    output.m_worldpos = mul(instance_transform, float4(input.m_position, 1.0f)).xyz;
    // uvs
    output.m_texcoord = input.m_texcoord;
    // normals
    output.m_normal = normalize(mul((float3x3)instance_transform, input.m_normal));
    // color
    float4 instance_color = instance_data.m_colour;
    output.m_colour.rgb = lerp(instance_color.rgb, g_permaterial.m_colour.rgb, 0.5f);
    // texids ?
    output.m_texid_albedo = instance_data.get_albedo_index();
    output.m_texid_normal = instance_data.get_normal_index();
    return output;
}
// ================================================================================================
// basepass-ps: renders geometry data to the packed screen buffers
[shader("pixel")]
ps_output main_ps(ps_input input)
{
    float4 albedo = get_albedo().Sample(get_sampler(0), input.m_texcoord).rgba;
    // float3 normal = get_normal(input.texcoord).rgb; // normal mapping
    float3 normal = input.m_normal;
    normal = normalize(normal);

    ps_output output = (ps_output)0;
    output.m_gbuffer.set_albedo(lerp(albedo.rgb, input.m_colour.rgb, 0.5f));
    output.m_gbuffer.set_normal(normal.rgb);
    output.m_gbuffer.set_depth(input.m_position.z);
    return output;
}

// ================================================================================================
// shadepass-cs: shades each pixel according to info in packed screen buffers
ConstantBuffer<cs_shading_args> g_shadingargs : register(b4);

RWTexture2D<float4> get_output()                    { return ResourceDescriptorHeap[k_final_target_id]; }
StructuredBuffer<per_dirlight> get_dirlights()      { return ResourceDescriptorHeap[k_dirlights_id]; }
Texture2D<uint4> get_gbufferA()                     { return ResourceDescriptorHeap[k_gbuffer_id + 0]; }
Texture2D<uint> get_gbufferB()                      { return ResourceDescriptorHeap[k_gbuffer_id + 1]; }
Texture2D<uint> get_gbufferC()                      { return ResourceDescriptorHeap[k_gbuffer_id + 2]; }

[shader("compute")]
[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
void main_cs(uint3 thread_id : SV_DispatchThreadID)
{
    RWTexture2D<float4> output = get_output();

    // decode gbuffer
    Texture2D<uint4> GbufferA = get_gbufferA();
    Texture2D<uint> GBufferB = get_gbufferB();
    Texture2D<uint> GBufferC = get_gbufferC();
    gbuffer gbuffer;
    gbuffer.albedo_emmisive_pbr.rgba    = GbufferA.Load(uint3(thread_id.xy, 0));
    gbuffer.normal.r                    = GBufferB.Load(uint3(thread_id.xy, 0));
    gbuffer.depth_stencil.r             = GBufferC.Load(uint3(thread_id.xy, 0));

    // early out if depth fails
    float depth = gbuffer.get_depth().r;
    if (depth <= 0.0f)
    {
        output[thread_id.xy] = float4(0,0,0,1);
        return;
    }
    
    // float3 albedo = gbuffer.get_albedo();
    // float3 normal = normalize(gbuffer.get_normal().rgb);
    output[thread_id.xy] = float4(1, 1, 1, 1);
}
#endif // !__cplusplus
#endif // SHADER_FRONTEND_H