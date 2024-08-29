#include "common.hlsli"

[shader("vertex")]
ps_input VSMain ( vs_input input, uint vID : SV_VertexID )
{
    ps_input output = (ps_input)0;
    output.position = mul ( g_perframe_vs.mat_mvp, float4 ( input.position, 1.0f ) );

    output.worldPos = input.position;
    output.texcoord = input.texcoord;
    output.colour = float4(hash(uint3(vID,vID+7,vID+41)), 1.0f);
    output.normal = input.normal;
    return output;
}

[shader("pixel")]
float4 PSMain ( ps_input input ) : SV_TARGET
{
    float3 lightDir = float3(-0.5,-0.5,-0.5);

    float4 albedo = get_albedo(input.texcoord).rgba;
    float3 normal = get_normal(input.texcoord).rgb;

    float ambient = 0.2f;
    float diffuse = max(ambient,dot(normalize(normal), normalize(lightDir)));

    return albedo;
}