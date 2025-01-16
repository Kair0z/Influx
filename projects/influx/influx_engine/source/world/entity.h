#pragma once

// entt
#include "entt/entt.hpp"

// influx::engine
#include "world/world.h"

namespace influx::engine
{
	class entity final
	{
	public:
		entity() = default;
		entity(const entt::entity& handle);

		enum render_flag : uint32
		{
			render_invisible,
			render_debug
		};

		render_flag get_renderflags() const;

		void set_invisible(bool enabled);
		void set_debug_render(bool enabled);
		bool is_invisible() const;
		bool is_debug_render() const;
		entt::entity get_handle() const;

		template <typename _t>
		_t* get_component() const;

		bool operator==(const entity& b) const
		{
			return this->m_handle == b.m_handle;
		}
		bool operator!=(const entity& b) const
		{
			return !(*this == b);
		}

	private:
		render_flag m_renderflags;
		entt::entity m_handle;

	};

	template<typename _t>
	inline _t* entity::get_component() const
	{
		return get_engine()->get_world()->get_component<_t>(*this);
	}
}