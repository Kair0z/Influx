// renderer_structs.slang
implementing renderer;

public struct per_vertex_data
{
    float3 position : POSITION;
    float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

public struct vs_input
{
    per_vertex_data m_vertex;
};

public struct ps_input
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    nointerpolation float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    nointerpolation uint texid_albedo : TEXCOORD1;
    nointerpolation uint texid_normal : TEXCOORD2;
};

public struct ps_output
{
    gbuffer gbuffer_data;
};

public struct per_scene
{
	float4 m_time;
	float4 m_light_direction;
	float4 m_light_colour;
};

public struct per_view
{
	float4x4 m_viewprojection;
};

public struct per_material
{
	float4 m_colour;
};

public struct per_draw
{
	uint m_base_instance;
};

public struct per_instance
{
	float4x4    m_transform;
	float4      m_colour;
    uint4       m_texture_indices;

    [mutating]
	void set_albedo_index(uint index) { 
        m_texture_indices.x = index;
    }

	uint get_albedo_index() { 
        return m_texture_indices.x;
    }

    [mutating]
	void set_normal_index(uint index) { 
        m_texture_indices.y = index; 
    }

	uint get_normal_index() { 
        return m_texture_indices.y; 
    }
};

public struct per_vertex
{
	float3 m_position;
	float4 m_colour;
	float3 m_normal;
	float2 m_texcoord;
};

public struct per_pointlight
{
	float4 m_position;
	float4 m_colour;
	float4 m_attenuation;
};

public struct per_spotlight
{
	float4 m_position;
};

public struct per_dirlight
{
	float4 m_colour;
};