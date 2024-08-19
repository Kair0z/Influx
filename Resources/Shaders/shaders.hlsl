
#include "common.hlsli"

[RootSignature(INFLUX_DEFAULT_ROOTSIGNATURE)]
[shader("vertex")]
ps_input VSMain ( vs_input input, uint vID : SV_VertexID )
{
    ps_input output = (ps_input)0;
    output.position = mul ( _perframe_vs.mat_mvp, float4 ( input.position, 1.0f ) );

    output.worldPos = input.position;
    output.texcoord = input.texcoord;
    output.colour = float4(hash(uint3(vID,vID+7,vID+41)), 1.0f);
    output.normal = input.normal;
    return output;
}

[RootSignature(INFLUX_DEFAULT_ROOTSIGNATURE)]
[shader("pixel")]
float4 PSMain ( ps_input input ) : SV_TARGET
{
    float3 lightDir = float3(-0.5,-0.5,-0.5);

    float4 albedo = g_textures[0].Sample(g_sampler, input.texcoord).rgba;
    float3 normal = g_textures[1].Sample(g_sampler, input.texcoord).rgb;

    float ambient = 0.2f;
    float diffuse = max(ambient,dot(normalize(normal), normalize(lightDir)));

    return diffuse * albedo;
}