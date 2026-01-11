#include "include/common.hlsli"
#include "include/renderer.hlsli"

ConstantBuffer<resolvepass_args> g_resolve_args : register(b0);

[shader("compute")]
[numthreads(8,8,1)]
void main_cs(uint3 thread_id : SV_DispatchThreadID)
{
    resolvepass_params params;
    params.m_thread_id = thread_id.xy;
    params.m_gbuffer_a = ResourceDescriptorHeap[g_resolve_args.texture_desc_indices.x];
    params.m_gbuffer_b = ResourceDescriptorHeap[g_resolve_args.texture_desc_indices.y];
    params.m_gbuffer_c = ResourceDescriptorHeap[g_resolve_args.texture_desc_indices.z];
    params.m_skycube = ResourceDescriptorHeap[g_resolve_args.skybox_indices[0]];
    params.m_skycube_sampler = SamplerDescriptorHeap[0];
    params.m_output = ResourceDescriptorHeap[g_resolve_args.texture_desc_indices.w];
    params.m_pointlights = ResourceDescriptorHeap[g_resolve_args.buffer_desc_indices[1]];
    params.m_spotlights = ResourceDescriptorHeap[g_resolve_args.buffer_desc_indices[2]];
    params.m_dirlights = ResourceDescriptorHeap[g_resolve_args.buffer_desc_indices[0]];
    params.m_args = g_resolve_args;
    resolvepass_cs(params);
}