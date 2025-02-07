
struct vs_input 
{
    float3 position : POSITION;
    float4 colour : COLOR;
};

struct ps_input 
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
};

struct per_view
{
    float4x4 mat_vp;
};

struct per_instance_data
{
    float3	    wp_start;
    float3      wp_end;
    float4      colour;
};

ConstantBuffer<per_view>                g_perview           : register(b0);
StructuredBuffer<per_instance_data>     g_instancebuffer    : register(t1);

[shader("vertex")]
ps_input main_vs(vs_input input, uint instanceID : SV_InstanceID)
{
    ps_input result;
    per_instance_data instance_data = g_instancebuffer[instanceID];

    float3 world_start = instance_data.wp_start.xyz;
    float3 world_end = instance_data.wp_end.xyz;
    float3 world_position = lerp(world_start, world_end, input.position.x);
    
    result.position = mul(g_perview.mat_vp, float4(world_position.xyz, 1.0f));
    result.colour = float4(1, 1, 1, 1);// instance_data.colour;

    return result;
}

[shader("pixel")]
float4 main_ps(ps_input input) : SV_TARGET
{
    return input.colour;
}