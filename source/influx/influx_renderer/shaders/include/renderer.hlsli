
struct per_vertex_data
{

};
struct vs_input 
{
    per_vertex_data m_vertex;
};
struct ps_input
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    nointerpolation float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    nointerpolation uint texid_albedo : TEXCOORD1;
    nointerpolation uint texid_normal : TEXCOORD2;
};
struct ps_output
{
    gbuffer gbuffer_data;
};

ps_input vs_basepass(
    vs_input input,
    uint vertex_id, 
    uint instance_id,
    ConstantBuffer<per_draw> drawbuffer,
    ConstantBuffer<per_view> viewbuffer,
    ConstantBuffer<per_material> materialbuffer,
    StructuredBuffer<per_instance> instancebuffer)
{
    // get instance data
    instance_id += drawbuffer.m_base_instance;
    per_instance instance_data = instancebuffer[instance_id];

    // get vertex data
    per_vertex_data vertex_data = input.vertex_data;

    ps_input output = (ps_input)0;
    
    // positions
    float4x4 instance_transform = (float4x4)instance_data.m_transform;
    float4x4 mvp = mul((float4x4)viewbuffer.m_viewprojection, instance_transform);
    output.position = mul(mvp, float4 ( vertex_data.position, 1.0f ) );
    output.worldPos = mul(instance_transform, float4(vertex_data.position, 1.0f)).xyz;

    // uvs
    output.texcoord = vertex_data.texcoord;

    // normals
    output.normal = normalize(mul((float3x3)instance_transform, vertex_data.normal));

    // color
    float4 instance_color = instance_data.m_colour;
    output.colour.rgb = lerp(instance_color.rgb, materialbuffer.m_colour.rgb, 0.5f);

    // texids
    output.texid_albedo = instance_data.get_albedo_index();
    output.texid_normal = instance_data.get_normal_index();
    return output;
}

