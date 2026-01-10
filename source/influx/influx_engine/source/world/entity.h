#pragma once

// entt
#include "entt.hpp"

// influx::core
#include "core/enum.h"

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
}

ENABLE_ENUM_BIT_OPERATORS(influx::engine::entity::render_flag);