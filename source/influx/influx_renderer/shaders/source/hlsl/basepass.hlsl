#include "include/common.hlsli"
#include "include/renderer.hlsli"

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

[shader("vertex")]
ps_input main_vs(vs_input input, uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID)
{
    return basepass_vs(
        input,
        vertex_id,
        instance_id,
        g_perdraw,
        g_perview,
        g_permaterial,
        ResourceDescriptorHeap[0]);
}

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

[shader("pixel")]
ps_output main_ps(ps_input input)
{
    return basepass_ps(
        input,
        get_texture(input.texid_albedo),
        get_sampler(0));
}
