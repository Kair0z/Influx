
struct per_instance_data
{
    float3	    wp_start;
    float3      wp_end;
    float4      colour;
};

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

// SHADER INPUTS
// buffers
ConstantBuffer<per_view>                g_perview           : register(b0);
StructuredBuffer<per_instance_data>     g_instancebuffer    : register(t1);

ps_input main_vs(vs_input input, uint instanceID : SV_InstanceID)
{
    ps_input result;

    per_instance_data instance_data = g_instancebuffer[instanceID];

    float3 world_position = lerp(
        instance_data.wp_start, 
        instance_data.wp_end, 
        input.position.x);
    
    // to clip space
    result.position = mul(float4(world_position, 1.0f), g_perview.mat_vp);
    
    result.colour = instance_data.colour;

    return result;
}

float4 main_ps(ps_input input) : SV_TARGET
{
    return input.colour;
}