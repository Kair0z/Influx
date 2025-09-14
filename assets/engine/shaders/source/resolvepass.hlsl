#include "include/common.hlsli"
#include "frontend.h"

// root constants
struct resolve_args
{
    int4 texture_desc_indices;
    int4 buffer_desc_indices;
    int4 skybox_indices;

    float4 screen_size;
    float4 camera_position;

    float4x4 inv_viewprojection;
    float4x4 inv_projection;
    int4 num_lights;
};
ConstantBuffer<resolve_args> g_resolve_args : register(b0);

// output UAV
RWTexture2D<float4> get_output()
{
    return ResourceDescriptorHeap[g_resolve_args.texture_desc_indices.w];
}

// light buffers
StructuredBuffer<per_pointlight> get_pointlights()
{
    return ResourceDescriptorHeap[g_resolve_args.buffer_desc_indices[1]];
}
StructuredBuffer<per_spotlight> get_spotlights()
{
    return ResourceDescriptorHeap[g_resolve_args.buffer_desc_indices[2]];
}
StructuredBuffer<per_dirlight> get_dirlights()
{
    return ResourceDescriptorHeap[g_resolve_args.buffer_desc_indices[0]];
}

// skycube
SamplerState get_skycube_sampler()
{
    return SamplerDescriptorHeap[0];
}
TextureCube get_skycube()
{
    return ResourceDescriptorHeap[g_resolve_args.skybox_indices[0]];
}

// skycube
float2 get_uv(uint2 thread_id)
{
    return float2(thread_id) * float2(g_resolve_args.screen_size.zw);
}
float2 get_ndc(uint2 thread_id)
{
    float2 uv = get_uv(thread_id);
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y *= -1.0;
    return ndc.xy;
}
float3 get_viewdirection(uint2 thread_id)
{
    float2 ndc = get_ndc(thread_id);
    
    // Step 3: Reconstruct View Direction
    float4 clipSpacePos = float4(ndc.xy, 1.0, 1.0); // Homogeneous clip-space position at far plane
    float4 viewSpacePos = mul(clipSpacePos, g_resolve_args.inv_projection); // Transform to view space
    viewSpacePos /= viewSpacePos.w; // Perspective divide

    return normalize(viewSpacePos.xyz);
}

float3 sample_sky(uint2 thread_id)
{
    return get_skycube().Sample(get_skycube_sampler(), get_viewdirection(thread_id)).rgb;
}

gbuffer get_gbuffer(uint2 thread_id)
{
    Texture2D<uint4> GbufferA = ResourceDescriptorHeap[g_resolve_args.texture_desc_indices.x];
    Texture2D<uint> GBufferB = ResourceDescriptorHeap[g_resolve_args.texture_desc_indices.y];
    Texture2D<uint> GBufferC = ResourceDescriptorHeap[g_resolve_args.texture_desc_indices.z];

    gbuffer buffers;
    buffers.albedo_emmisive_pbr.rgba    = GbufferA.Load(uint3(thread_id.xy, 0));
    buffers.normal.r                    = GBufferB.Load(uint3(thread_id.xy, 0));
    buffers.depth_stencil.r             = GBufferC.Load(uint3(thread_id.xy, 0));
    return buffers;
}

float3 get_worldpos(float2 screen_uv, float depth)
{
    screen_uv.y = 1.0f - screen_uv.y;
    float2 ndc = screen_uv * 2.0f - 1.0f;

    float4 clippos = float4(ndc.xy, depth, 1.0f);
    float4 worldpos = mul((float4x4)g_resolve_args.inv_viewprojection, clippos);
    worldpos.xyz /= worldpos.w;
    return worldpos.xyz;
}

float3 pointlight(float3 light_position, float3 light_color, float const_att, float3 worldpos, float3 normal)
{
    float3 to_light = light_position - worldpos;
    float distance = length(to_light);
    to_light = normalize(to_light);
    
    float attenuation = 1 - (distance / (const_att * const_att));
    
    // Diffuse term (cosine of angle)
    float diffuse = max(dot(normal, to_light), 0.0f); 
    
    return diffuse * light_color * attenuation; // Diffuse lighting contribution
}

void set_output(uint2 thread_id, float3 colour)
{
    get_output()[thread_id.xy].rgba = float4(clamp(colour.rgb, 0, 1), 1.0f);
}

[shader("compute")]
[numthreads(8,8,1)]
void main_cs(uint3 thread_id : SV_DispatchThreadID)
{
    float4 output_color = float4(0, 0, 0, 1);

    gbuffer _gbuffer = get_gbuffer(thread_id.xy);

    // albedo is encoded in 10b, but our output target is 8b
    float albedo_scale = 1.0f;

    float3 albedo = _gbuffer.get_albedo() * albedo_scale;
    float3 normal = normalize(_gbuffer.get_normal().rgb);
    float depth = _gbuffer.get_depth().r;

    if (depth > 0.0f)
    {
        float3 lightDir = float3(0, -1, 0);
        float3 lightCol = float3(1.0, 1.0, 1.0);
        lightCol *= 2.0f;
        
        float3 ambient = 0.5f;
        float3 diffuse = max(ambient, lightCol.rgb * dot(normalize(normal), normalize(lightDir)));

        float3 lightpos = g_resolve_args.camera_position.xyz;
        float light_radius = 2.0f;

        float3 worldpos = get_worldpos(thread_id.xy * g_resolve_args.screen_size.zw, depth);

        // get the pointlights
        StructuredBuffer<per_pointlight> pointlights = get_pointlights();
        for (uint i = 0; i < g_resolve_args.num_lights[1]; ++i)
        {
#define ENABLE_POINT_LIGHT 1
#if ENABLE_POINT_LIGHT
            diffuse += pointlight(
                pointlights[i].m_position.xyz,
                pointlights[i].m_colour.rgb, 
                pointlights[i].m_attenuation.r, 
                worldpos,
                normal).rgb;
#endif
        }

        output_color.rgb = albedo;
        set_output(thread_id.xy, output_color.rgb);
    }
}