#ifndef SHADER_FRONTEND_H
#define SHADER_FRONTEND_H

// some types
#ifdef __cplusplus
// c++ end
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/math/matrix.h"

typedef influx::math::matrix4x4f	float4x4;
typedef influx::math::float4		float4;
typedef influx::math::float3		float3;
typedef influx::math::float2		float2;
typedef influx::math::uint4			uint4;
typedef influx::math::uint3			uint3;
typedef influx::math::uint2			uint2;
typedef influx::uint32				uint;
#else

#endif

struct per_scene
{
	float4 m_time;
	float4 m_light_direction;
	float4 m_light_colour;
};

struct per_view
{
	float4x4 m_viewprojection;
};

struct per_material
{
	float4 m_colour;
};

struct per_draw
{
	uint m_base_instance;
};

struct per_instance
{
	float4x4 m_transform;
	float4 m_colour;

	void set_albedo_index(uint index) { m_texture_indices[0] = index; }
	uint get_albedo_index() { return m_texture_indices[0]; }

	void set_normal_index(uint index) { m_texture_indices[1] = index; }
	uint get_normal_index() { return m_texture_indices[1]; }

	uint4 m_texture_indices;
};

struct line_gpu_instance_data final
{
	float3 m_start_wp;
	float3 m_end_wp;
	float4 m_colour;
};
struct line_vertex final
{
	float3 m_position;
	float4 m_colour;
	uint m_id;
};

struct per_vertex
{
	float3 m_position;
	float4 m_colour;
	float3 m_normal;
	float2 m_texcoord;
};

struct per_pointlight
{
	float4 m_position;
	float4 m_colour;
	float4 m_attenuation;
};

struct per_spotlight
{
	float4 m_position;
};

struct per_dirlight
{
	float4 m_colour;
};

#endif // SHADER_FRONTEND_H