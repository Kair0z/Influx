#pragma once

// influx::renderer
#include "influx_renderer/common.h"
#include "influx_renderer/mesh.h"

// imgui
struct ImGuiContext;

namespace influx::renderer
{
	class scene;

	static constexpr object_id k_invalid_id = (uint32)-1;

	struct view_matrices final
	{
		matrix m_transform;	// transform of the camera
		matrix m_projection;
		matrix m_view;				// inv transform
		matrix m_viewprojection;
		matrix m_inv_viewprojection;
		matrix m_inv_projection;

		view_matrices() = default;
		explicit view_matrices(const matrix& transform, const camera& camera);
	};

	struct mesh_instance final
	{
		mesh_id				m_mesh_id = k_invalid_id;
		material_id			m_mat_id = k_invalid_id;
		transform_id		m_transform_id = k_invalid_id;
		colour				m_per_instance_colour = {};
	};

	struct line final
	{
		position3D m_points[2]{};
		colour m_colour;
		line(const position3D& start, const position3D& end, const colour& colour)
			: m_points{ start, end }
			, m_colour{ colour } {}
	};

	enum class e_scene_render_flags : uint8
	{
		none = 0,
		enable_debug = 1 << 0,
		enable_all = enable_debug
	};

	class world final
	{
	public:
		vector<matrix>				m_transforms{};
		vector<mesh_instance>		m_meshes = {};
		vector<light>				m_lights = {};
		vector<line>				m_lines{};

	private:
		template <typename _id>
		auto& get_container()
		{
			if constexpr (std::is_same_v<_id, transform_id>) {
				return m_transforms;
			}
			else if constexpr (std::is_same_v<_id, mesh_instance_id>) {
				return m_meshes;
			}
			else if constexpr (std::is_same_v<_id, light>) {
				return m_lights;
			}
			else if constexpr (std::is_same_v<_id, line>) {
				return m_lines;
			}
			else {
				static_assert(!std::is_same_v<_id, _id>, "Unsupported ID type passed to get_container");
			}
		}
		template <typename _id>
		bool in_range(const _id& id)
		{
			return id < get_container<_id>().size();
		}

	public:
		inline result<matrix> get_transform(const transform_id& id)
		{
			using result_type = result<matrix>;
			if (!in_range(id))
				return result_type::make_error("id is out of range!");
			return m_transforms[id];
		}
		
		world() = default;

	private:
		
	};

	class worldview final
	{
	public:
		camera					m_camera_settings;
		view_matrices			m_matrices;
		e_scene_render_flags	m_renderflags;

		worldview() = default;
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
		mesh_instance& get_mesh(const mesh_instance_id& id);
		
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
		void set_camera(const influx::camera& camera, const math::matrix4x4f& transform);
		
		INFLUX_RENDER_API
		void set_camera_settings(const influx::camera& camera);

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