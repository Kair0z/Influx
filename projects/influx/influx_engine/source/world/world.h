#pragma once

// influx::core
#include "core/container/pool.h"
#include "core/container/list.h"
#include "core/container/vector.h"
#include "core/pointer.h"
#include "core/result.h"

// entt
#include "entt/entt.hpp"

// influx::engine
#include "component/component.h"

namespace influx::engine
{
	class scene;
}

// influx::renderer
namespace influx::renderer
{
	struct scene;
	struct scene2D;
}

namespace influx::engine
{
	using entity = entt::entity;

	class world final
	{
		template <uint32 _num>
		using tentity_pool = pool<entity, _num>;
		using entity_pool_8K = tentity_pool<8 * 1024u>;
		using entity_pool_4K = tentity_pool<4 * 1024u>;
		using entity_pool_1K = tentity_pool<1 * 1024u>;
		using entity_pool = entity_pool_4K;

	public:
		world();
		virtual ~world();

		void update();

		// -- engine-end
		void build_renderscene(renderer::scene&, renderer::scene2D&) const;

		entity create_entity();

		template<typename _ctype, typename... _args>
		_ctype& create_component(const entity e, _args&&... args);

		template<typename _ctype>
		_ctype* get_component(const entity& e);

		template<typename _ctype>
		bool has_component(const entity& e);

		// deletes unreferenced entities
		void flush();

	private:
		entt::registry m_registry;

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

		// this is ugly
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
		return m_registry.emplace<_ctype>(e, std::forward<_args&&>(args)...);
	}

	template<typename _ctype>
	inline _ctype* world::get_component(const entity& e)
	{
		if (m_registry.valid(e))
		{
			return m_registry.try_get<_ctype>(e);
		}

		return nullptr;
	}

	template<typename _ctype>
	inline bool world::has_component(const entity& e)
	{
		if (m_registry.valid(e) && m_registry.try_get<_ctype>(e))
		{
			return true;
		}

		return false;
	}
}