#include "common.hlsli"

[shader("vertex")]
ps_input VSMain ( vs_input input, uint vID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    ps_input output = (ps_input)0;

    instanceID += g_perdraw_vs.start_instance;
    per_instance_data instance_data = g_instancebuffer[instanceID];

    float4x4 mvp = mul(
        (float4x4)g_perframe_vs.mat_vp,
        (float4x4)instance_data.mat_transform);

    output.position = mul( mvp, float4 ( input.position, 1.0f ) );

    output.worldPos = input.position;
    output.texcoord = input.texcoord;
    // output.colour = float4(hash(uint3(vID,vID+7,vID+41)), 1.0f);
    output.colour.r = (float)instanceID / 50u;
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

    return lerp(input.colour, albedo, sin(g_perframe_ps.seconds * 2.0f));
}