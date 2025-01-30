#include "common.hlsli"

struct resolve_args
{
    int4 descriptor_indices;
    float4 screen_size;
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

gbuffer get_gbuffer(float2 texcoord)
{
    Texture2D GbufferA = ResourceDescriptorHeap[g_resolve_args.descriptor_indices.x];
    Texture2D GBufferB = ResourceDescriptorHeap[g_resolve_args.descriptor_indices.y];
    Texture2D GBufferC = ResourceDescriptorHeap[g_resolve_args.descriptor_indices.z];

    gbuffer buffers;
    buffers.albedo_emmisive_pbr.rgba    = GbufferA.Sample(get_sampler(), texcoord).rgba;
    buffers.normal.r                    = GBufferB.Sample(get_sampler(), texcoord).r;
    buffers.depth_stencil.r             = GBufferC.Sample(get_sampler(), texcoord).r;
    return buffers;
}

[shader("compute")]
[numthreads(8,8,1)]
void main_cs(uint3 thread_id : SV_DispatchThreadID)
{
    float2 texcoord = thread_id.xy * g_resolve_args.screen_size.zw;

    gbuffer _gbuffer = get_gbuffer(texcoord);

    float4 output = float4(_gbuffer.get_albedo().rgb, 1.0f);
    get_output()[thread_id.xy].rgba = output.rgba;
}