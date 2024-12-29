#include "common.hlsli"

[shader("vertex")]
ps_input main_vs( vs_input input, uint vID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    ps_input output = (ps_input)0;

    // instance
    instanceID += g_perdraw.start_instance;
    per_instance_data instance_data = g_instancebuffer[instanceID];
    
    // position
    // calculate mvp
    float4x4 mvp = mul(
        (float4x4)g_perview.mat_vp,
        (float4x4)instance_data.mat_transform);
    output.position = mul(mvp, float4 ( input.position, 1.0f ) );
    output.worldPos = mul(instance_data.mat_transform, float4(input.position, 1.0f)).xyz;

    // uvs
    output.texcoord = input.texcoord;
    
    // normals
    output.normal = normalize(mul((float3x3)instance_data.mat_transform, input.normal));
    if (instance_data.invert_normals)
    {
        output.normal = -output.normal;
    }

    // color
    output.colour.rgb = lerp(instance_data.colour.rgb, g_permaterial.colour.rgb, 0.5f);
    
    return output;
}

[shader("pixel")]
float4 main_ps(ps_input input) : SV_TARGET
{
    float3 lightDir = g_perscene.light_direction.rgb;
    float3 lightCol = g_perscene.light_colour.rgb;

    float4 albedo = get_albedo(input.texcoord).rgba;
    // albedo.rgb = lerp(input.colour.rgb, albedo.rgb, 0.5f);

    float3 normal = get_normal(input.texcoord).rgb;
    normal = input.normal;

    float ambient = 0.2f;
    float diffuse = max(ambient, dot(normalize(normal), normalize(lightDir)));

    return float4(albedo.rgb, 1.0f);
}
