#pragma once

// influx::core
#include "core/container/pool.h"
#include "core/container/list.h"
#include "core/container/vector.h"
#include "core/pointer.h"
#include "core/result.h"
#include "core/flag.h"
#include "core/geometry/ray.h"

// influx::engine
#include "component/component.h"
#include "entity.h"

// influx::files
#include "influx_file.h"

namespace influx::engine
{
	class scene;
}

// influx::renderer
namespace influx::renderer
{
	struct scene;
	struct scene2D;
	struct scene_debug;
}

namespace influx::engine
{
	class world final
	{
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
		entity create_entity();

		void destroy_entity(entity);

		template<typename _ctype, typename... _args>
		_ctype& create_component(const entity e, _args&&... args);

		template<typename _ctype>
		_ctype* get_component(const entity& e);

		template<typename _ctype>
		bool has_component(const entity& e);

		void clear();

		// scene picking
		struct trace_result { entity* m_entity = nullptr; };
		bool trace(const math::ray& ray, trace_result& out_result, e_collision_layer layer = e_collision_layer::all);

		// project file management
		void load_project(const influx::files::projectfile& proj);
		void save_project(influx::files::projectfile& proj);

		// gets the viewmatrix of the main camera
		math::matrix4x4f get_main_projection_matrix() const;
		math::matrix4x4f get_main_viewmatrix() const;
		math::float3 get_main_cameraposition() const;

	private:
		entt::registry m_registry;
		list<entity> m_entities;
		
		// update
		void update_transform_system();
		void update_input_system();
		void update_bounds_system();
		void update_stream_system();
		void update_rigidbody_system();

		// deferred input: this is a bit ugly
		template <typename _t>
		struct lock_queue
		{
			void push(const _t& val)
			{
				m_lock.lock();
				m_data.push(val);
				m_lock.unlock();
			}

			template <typename _readfunc>
			void read(_readfunc&& func) const
			{
				m_data.read(func);
			}

			void clear()
			{
				m_lock.lock();
				m_data.clear();
				m_lock.unlock();
			}

			queue<_t> get_copy()
			{
				return m_data;
			}

			queue<_t> m_data{};
			std::mutex m_lock;
		};
		lock_queue<input::e_key> m_deferred_keydowns{};
		lock_queue<input::e_key> m_deferred_keyups{};
		lock_queue<char> m_deferred_ascii_downs{};
		lock_queue<char> m_deferred_ascii_ups{};
		lock_queue<input::mouse_position> m_deferred_mousemoves{};
		lock_queue<std::pair<input::e_mouse_button, input::mouse_position>> m_deferred_mousedowns{};
		lock_queue<std::pair<input::e_mouse_button, input::mouse_position>> m_deferred_mouseups{};
	};

	template<typename _ctype, typename... _args>
	inline _ctype& world::create_component(const entity e, _args&&... args)
	{
		return m_registry.emplace<_ctype>(e.get_handle(), std::forward<_args&&>(args)...);
	}

	template<typename _ctype>
	inline _ctype* world::get_component(const entity& e)
	{
		if (m_registry.valid(e.get_handle()))
		{
			return m_registry.try_get<_ctype>(e.get_handle());
		}

		return nullptr;
	}

	template<typename _ctype>
	inline bool world::has_component(const entity& e)
	{
		if (m_registry.valid(e.get_handle()) && m_registry.try_get<_ctype>(e.get_handle()))
		{
			return true;
		}

		return false;
	}
}