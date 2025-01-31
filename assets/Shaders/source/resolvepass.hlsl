#include "common.hlsli"

struct resolve_args
{
    int4 descriptor_indices;
    float4 screen_size;
    
    float4x4 inv_viewprojection;
};
ConstantBuffer<resolve_args> g_resolve_args : register(b0);

RWTexture2D<float4> get_output()
{
    return ResourceDescriptorHeap[g_resolve_args.descriptor_indices.w];
}

SamplerState get_sampler()
{
    return SamplerDescriptorHeap[0];
}

gbuffer get_gbuffer(uint2 thread_id)
{
    Texture2D<uint4> GbufferA = ResourceDescriptorHeap[g_resolve_args.descriptor_indices.x];
    Texture2D<uint> GBufferB = ResourceDescriptorHeap[g_resolve_args.descriptor_indices.y];
    Texture2D<uint> GBufferC = ResourceDescriptorHeap[g_resolve_args.descriptor_indices.z];

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
    depth = 2.0f * depth - 1.0f;
    float4 clippos = float4(ndc.xy, depth, 1.0f);
    float4 worldpos = mul(clippos, g_resolve_args.inv_viewprojection);
    return worldpos.xyz;
}

float3 pointlight(float3 light_position, float3 light_color, float3 worldpos, float3 normal)
{
    float3 to_light = light_position - worldpos;
    float distance = length(to_light);
    to_light = normalize(to_light);
    
    float attenuation = 1.0f / (2.0f * 2.0f);
    
    // Diffuse term (cosine of angle)
    float diffuse = max(dot(normal, to_light), 0.0f); 
    
    return diffuse * light_color * attenuation * 1000; // Diffuse lighting contribution
}

[shader("compute")]
[numthreads(8,8,1)]
void main_cs(uint3 thread_id : SV_DispatchThreadID)
{
    gbuffer _gbuffer = get_gbuffer(thread_id.xy);

    // albedo is encoded in 10b, but our output target is 8b
    float albedo_scale = 1.0f;

    float3 albedo = _gbuffer.get_albedo() * albedo_scale;
    float3 normal = _gbuffer.get_normal().rgb;
    float depth = _gbuffer.get_depth().r;
    
    float3 lightDir = float3(0.5, -0.5, -0.5);
    float3 lightCol = float3(1.0, 1.0, 1.0);

    float3 ambient = 0.2f;
    float3 diffuse = max(ambient, dot(normalize(normal), normalize(lightDir)));

    float3 lightpos = float3(0, 0, 0);
    float light_radius = 5.0f;
    
    float3 worldpos = get_worldpos(thread_id.xy * g_resolve_args.screen_size.zw, depth);
    // diffuse += pointlight(lightpos, lightCol, worldpos, normal).rgb;
    
    float3 scenecolor = albedo.rgb * diffuse.rgb;
    get_output()[thread_id.xy].rgba = float4(clamp(scenecolor.rgb, 0, 1), 1.0f);
}