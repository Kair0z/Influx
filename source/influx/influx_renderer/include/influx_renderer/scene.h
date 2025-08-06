#pragma once

// influx::core
#include "core/math/vector.h"
#include "core/math/matrix.h"
#include "core/math/transform.h"
#include "core/string.h"
#include "core/container/vector.h"
#include "core/math/colour.h"
#include "core/math/rect.h"
#include "core/math/bounds.h"
#include "core/math/transform.h"
#include "core/material/material.h"
#include "core/scene/light.h"
#include "core/scene/camera.h"
#include "core/enum.h"

// influx::renderer
#include "influx_renderer/types.h"
#include "influx_renderer/mesh.h"

// imgui
struct ImGuiContext;

namespace influx::renderer
{
	class scene;

	static constexpr uint32 k_invalid_id = (uint32)-1;
	using object_id		= uint32;
	using material_id	= object_id;
	using camera_id		= object_id;
	using mesh_inst_id	= object_id;
	using mesh_id		= string;
	using light_id		= object_id;
	using transform_id	= object_id;

	struct light final
	{
		influx::light m_light;
		transform_id m_transform_id = k_invalid_id;
	};

	struct camera final
	{
		influx::camera m_camera;
		transform_id m_transform_id = k_invalid_id;
	};

	struct view_matrices final
	{
		view_matrices() = default;
		explicit view_matrices(const math::matrix4x4f& transform, const camera& camera);

		math::matrix4x4f m_transform;			// transform of the camera
		math::matrix4x4f m_projection;
		math::matrix4x4f m_view;				// inv transform
		math::matrix4x4f m_viewprojection;
		math::matrix4x4f m_inv_viewprojection;
		math::matrix4x4f m_inv_projection;
	};

	struct mesh_instance final
	{
		mesh_id	m_mesh_id			= "";
		material_id	m_mat_id		= k_invalid_id;
		transform_id m_transform_id	= k_invalid_id;
		math::vectorf4 m_per_instance_colour = {};
	};

	struct line final
	{
		line(const math::float3& start, const math::float3& end, const math::colour_rgba& colour)
			: m_points{ start, end }
			, m_colour{ colour } {}

		math::float3 m_points[2]{};
		math::colour_rgba m_colour;
	};

	enum class e_scene_render_flags : uint8
	{
		none = 0,
		enable_debug = 1 << 0,
		enable_all = enable_debug
	};

	class scene final
	{
	public:
		scene() = default;
		
		INFLUX_RENDER_API 
		bool is_empty() const;

		INFLUX_RENDER_API 
		transform_id add_transform(const math::matrix4x4f& matrix);
		
		INFLUX_RENDER_API 
		math::matrix4x4f& get_transform(const transform_id& id);
		
		INFLUX_RENDER_API 
		const math::matrix4x4f& get_transform(const transform_id& id) const;

		// adding meshes
		INFLUX_RENDER_API
		mesh_instance& add_mesh(const mesh_id& mesh_id, const math::matrix4x4f& transform = math::matrix4x4f::identity());
		
		INFLUX_RENDER_API
		mesh_instance& add_mesh(e_mesh mesh, const math::matrix4x4f& transform = math::matrix4x4f::identity());

		INFLUX_RENDER_API
		mesh_instance& get_mesh(const mesh_inst_id& id);
		
		INFLUX_RENDER_API
		mesh_instance& get_last_mesh();
		
		INFLUX_RENDER_API 
		uint32 get_num_meshes() const;
		
		INFLUX_RENDER_API 
		bool has_meshes() const;

		INFLUX_RENDER_API
		bool has_camera() const;

		INFLUX_RENDER_API 
		const vector<mesh_instance>& get_meshes() const;
		
		// adding lights
		INFLUX_RENDER_API
		light& add_light(const influx::light& light, const math::matrix4x4f& transform = math::matrix4x4f::identity());
		
		INFLUX_RENDER_API
		uint32 get_num_lights() const;
		
		INFLUX_RENDER_API
		uint32 get_num_lights(influx::e_light_type type) const;
		
		INFLUX_RENDER_API
		bool has_lights() const;
		
		INFLUX_RENDER_API
		const vector<light>& get_lights() const;

		INFLUX_RENDER_API
		const camera& get_camera() const;
		
		INFLUX_RENDER_API
		result<math::matrix4x4f> get_camera_transform() const;
		
		INFLUX_RENDER_API
		void set_camera(const camera& camera, const math::matrix4x4f& transform);
		
		INFLUX_RENDER_API
		void set_camera(const camera& camera);

		INFLUX_RENDER_API
		void set_camera_transform(const math::matrix4x4f& transform);

		INFLUX_RENDER_API
		const view_matrices& get_view_matrices() const;

		// debug lines
		INFLUX_RENDER_API
		void add_line_box(const math::boxf& box, const math::colour_rgba& colour);

		INFLUX_RENDER_API
		void add_line(const line& line);

		INFLUX_RENDER_API
		void add_line(const math::float3& start, const math::float3& end, const math::colour_rgba& colour);

		INFLUX_RENDER_API
		void add_point(const math::float3& point, const math::colour_rgba& colour);

		INFLUX_RENDER_API
		void add_gizmo_transform(const math::transform3D& transform);

		INFLUX_RENDER_API
		const vector<line>& get_lines() const;

		INFLUX_RENDER_API
		bool has_debug_primitives() const;
		
		INFLUX_RENDER_API
		void set_debug_render_enabled(bool enabled);

		INFLUX_RENDER_API
		bool is_debug_render_enabled() const;

		float m_delta_seconds;
		float m_seconds;

	private:
		vector<math::matrix4x4f>	m_transforms{};
		vector<mesh_instance>		m_meshes = {};
		vector<light>				m_lights = {};
		vector<line>				m_lines{};	
		camera						m_camera = {};
		view_matrices				m_viewmatrices{};
		e_scene_render_flags		m_renderflags;
	};

	struct sprite2D final
	{
		math::transform2D m_transform;
		math::rectf m_rectangle;
		string m_texture;

		bool m_scale_to_view;
	};

	struct scene2D final
	{
		inline bool is_empty() const
		{
			return m_sprites.size() == 0u;
		}

		vector<sprite2D> m_sprites = {};
	};

	struct scene_imgui final
	{
		inline bool is_empty() const
		{
			return m_imgui_stacks.size() == 0u;
		}

		vector<function<void(ImGuiContext&)>> m_imgui_stacks{};
	};
}
ENABLE_ENUM_BIT_OPERATORS(influx::renderer::e_scene_render_flags);