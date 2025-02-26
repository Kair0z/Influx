#pragma once

// influx::engine
#pragma region influx::engine
#include "component/component.h"
namespace influx::engine
{
	class scene;
}
#pragma endregion

// influx::files
#pragma region influx::files
#include "influx_file.h"
#pragma endregion

// influx::renderer
#pragma region influx::renderer
namespace influx::renderer
{
	struct scene;
	struct scene2D;
	struct scene_debug;
}
#pragma endregion

// entt
#include "entt/entt.hpp"

namespace influx::engine
{
	class world final
	{
		entt::registry m_registry;

	public:
		world();
		virtual ~world();

		void update();

		// rendering
		void build_renderscene(
			renderer::scene&, 
			renderer::scene2D&,
			renderer::scene_debug&) const;

		// entities / components
		entt::entity create_entity();

		void destroy_entity(entt::entity);

		template<typename _c, typename... _args>
		_c& create_component(entt::entity e, _args&&... args);

		template<typename _c>
		void destroy_component(entt::entity);

		template<typename _c>
		_c* get_component(entt::entity);

		template<typename _c>
		bool has_component(entt::entity) const;

		bool is_valid(entt::entity) const;

		void clear();

		// scene picking
		struct trace_result { entt::entity* m_entity = nullptr; };
		bool trace(const math::ray& ray, trace_result& out_result, e_collision_layer layer = e_collision_layer::all);

		// project file management
		void load_project(const influx::files::projectfile& proj);
		void save_project(influx::files::projectfile& proj);

		// gets the viewmatrix of the main camera
		math::matrix4x4f get_main_projection_matrix() const;
		math::matrix4x4f get_main_viewmatrix() const;
		math::float3 get_main_cameraposition() const;

	private:
		void update_transform_system();
		void update_input_system();
		void update_bounds_system();
		void update_stream_system();
		void update_rigidbody_system();
	};

	template<typename _c, typename... _args>
	inline _c& world::create_component(entt::entity e, _args&&... args)
	{
		return m_registry.emplace<_c>(e, std::forward(args)...);
	}

	template<typename _c>
	inline void world::destroy_component(entt::entity e)
	{
		m_registry.remove<_c>(e);
	}

	template<typename _c>
	inline _c* world::get_component(entt::entity e)
	{
		if (m_registry.valid(e))
		{
			return m_registry.try_get<_c>(e);
		}

		return nullptr;
	}

	template<typename _c>
	inline bool world::has_component(entt::entity e) const
	{
		if (m_registry.valid(e) && m_registry.try_get<_c>(e))
		{
			return true;
		}

		return false;
	}
}