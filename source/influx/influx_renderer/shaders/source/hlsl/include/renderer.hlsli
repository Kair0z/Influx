#include "../../frontend.h"

struct per_vertex_data
{
    float3 position : POSITION;
    float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
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

ps_input basepass_vs(
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
    per_vertex_data vertex_data = input.m_vertex;

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

ps_output basepass_ps(
    ps_input input,
    Texture2D tex_albedo,
    SamplerState samp_albedo)
{
    float4 albedo = tex_albedo.Sample(samp_albedo, input.texcoord).rgba;
    // float3 normal = get_normal(input.texcoord).rgb;
    float3 normal = input.normal;
    normal = normalize(normal);

    ps_output output = (ps_output)0;
    output.gbuffer_data.set_albedo(lerp(albedo.rgb, input.colour.rgb, 0.5f));
    output.gbuffer_data.set_normal(normal.rgb);
    output.gbuffer_data.set_depth(input.position.z);
    return output;
}

struct resolvepass_params
{
    uint2                               m_thread_id;
    Texture2D<uint4>                    m_gbuffer_a;
    Texture2D<uint>                     m_gbuffer_b;
    Texture2D<uint>                     m_gbuffer_c;
    TextureCube                         m_skycube;
    SamplerState                        m_skycube_sampler;
    RWTexture2D<float4>                 m_output;
    StructuredBuffer<per_pointlight>    m_pointlights;
    StructuredBuffer<per_spotlight>     m_spotlights;
    StructuredBuffer<per_dirlight>      m_dirlights;
    resolvepass_args                    m_args;
};

float3 pointlight(float3 light_position, float3 light_color, float const_att, float3 worldpos, float3 normal)
{
    float3 to_light = light_position - worldpos;
    float distance = length(to_light);
    to_light = normalize(to_light);
    
    float attenuation = 1 - (distance / (const_att * const_att));
    
    // Diffuse term (cosine of angle)
    float diffuse = max(dot(normal, to_light), 0.0f); 
    
    return diffuse * light_color * attenuation; // Diffuse lighting contribution
}

float3 get_worldpos(float2 screen_uv, float depth, float4x4 inv_viewprojection)
{
    screen_uv.y = 1.0f - screen_uv.y;
    float2 ndc = screen_uv * 2.0f - 1.0f;
    float4 clippos = float4(ndc.xy, depth, 1.0f);
    float4 worldpos = mul((float4x4)inv_viewprojection, clippos);
    worldpos.xyz /= worldpos.w;
    return worldpos.xyz;
}
float2 get_uv(uint2 thread_id, float2 inv_screensize)
{
    return float2(thread_id) * inv_screensize.xy;
}
float2 get_ndc(uint2 thread_id, float2 inv_screensize)
{
    float2 uv = get_uv(thread_id, inv_screensize);
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y *= -1.0;
    return ndc.xy;
}

void resolvepass_cs(resolvepass_params params)
{
    float4 output_color = float4(1, 0, 0, 1);
    uint2 thread_id = params.m_thread_id;

    // load the gbuffer
    gbuffer gbuffer;
    gbuffer.albedo_emmisive_pbr.rgba    = params.m_gbuffer_a.Load(uint3(thread_id.xy, 0));
    gbuffer.normal.r                    = params.m_gbuffer_b.Load(uint3(thread_id.xy, 0));
    gbuffer.depth_stencil.r             = params.m_gbuffer_c.Load(uint3(thread_id.xy, 0));

    // albedo is encoded in 10b, but our output target is 8b
    float albedo_scale = 1.0f;

    resolvepass_args resolve_args = params.m_args;
    StructuredBuffer<per_pointlight> pointlights = params.m_pointlights;
    float2 inv_screensize = resolve_args.screen_size.zw;

    float3 albedo = gbuffer.get_albedo() * albedo_scale;
    float3 normal = normalize(gbuffer.get_normal().rgb);
    float depth = gbuffer.get_depth().r;
    if (depth > 0.0f)
    {
        float3 lightDir = float3(0, -1, 0);
        float3 lightCol = float3(1.0, 1.0, 1.0);
        lightCol *= 2.0f;
        
        float3 ambient = 0.5f;
        float3 diffuse = max(ambient, lightCol.rgb * dot(normalize(normal), normalize(lightDir)));

        float3 lightpos = resolve_args.camera_position.xyz;
        float light_radius = 2.0f;

        float3 worldpos = get_worldpos(thread_id.xy * inv_screensize, depth, resolve_args.inv_viewprojection);
        for (uint i = 0; i < resolve_args.num_lights[1]; ++i)
        {

#define ENABLE_POINT_LIGHT 0
#if ENABLE_POINT_LIGHT
            diffuse += pointlight(
                pointlights[i].m_position.xyz,
                pointlights[i].m_colour.rgb, 
                pointlights[i].m_attenuation.r, 
                worldpos, 
                normal).rgb;
#endif
        }
        output_color.rgb = albedo;
    }
    else
    {
        float2 ndc = get_ndc(thread_id, inv_screensize);
        float4 clipSpacePos = float4(ndc.xy, 1.0, 1.0); // Homogeneous clip-space position at far plane
        float4 viewSpacePos = mul(clipSpacePos, resolve_args.inv_projection); // Transform to view space
        viewSpacePos /= viewSpacePos.w; // Perspective divide
        float3 view_direction = normalize(viewSpacePos.xyz);
        float3 sky_sample = params.m_skycube.Sample(params.m_skycube_sampler, view_direction).rgb;
        output_color.rgb = sky_sample;
    }

    output_color.rgb = float3(1,0,0);
    // write to output
    params.m_output[thread_id.xy].rgba = float4(clamp(output_color.rgb, 0, 1), 1.0f);
}
