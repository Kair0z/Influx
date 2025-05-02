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
	class scene;
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
			renderer::scene2D&) const;

		void build_renderviews() const;

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

		template <typename _func>
		inline void visit_entities(_func&& visit_func) const
		{
			for (auto entity : m_registry.view<entt::entity>())
			{
				visit_func(entity);
			}
		}

		template <typename _c, typename _func>
		inline void visit_components(_func&& visit_func) const
		{
			for (auto [entity, comp] : m_registry.view<const _c>().each())
			{
				visit_func(comp);
			}
		}

		void clear();

		struct trace_result final
		{ 
			bool m_is_hit = false;
			cptr<entt::entity> m_entity = nullptr;
		};
		trace_result trace(const math::ray& ray, e_collision_layer layer = e_collision_layer::all) const;

		// project file management
		void load_project(const influx::files::projectfile& proj);
		void save_project(influx::files::projectfile& proj);

		// gets the viewmatrix of the first camera of the scene
		result<cptr<camera_component>> get_main_scene_camera() const;
		result<cptr<transform_component>> get_main_scene_camera_transform() const;
		result<trace_result> trace_main_scene(const math::float2& uv) const;
		result<math::ray> make_main_scene_viewray(const math::float2& uv) const;

		static math::ray make_viewray(
			const transform_component& transform,
			const camera_component& camera,
			const math::vectorf2& uv);

	private:
		void update_transform_system();
		void update_input_system();
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