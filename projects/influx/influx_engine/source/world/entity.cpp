#include "engine_pch.h"
#include "entity.h"

// influx::core
#include "core/flag.h"

namespace influx::engine
{
	entity::entity(const entt::entity& handle)
		: m_handle{handle}
		, m_renderflags{}
	{
	}

	entity::render_flag entity::get_renderflags() const
	{
		return m_renderflags;
	}

	void entity::set_invisible(bool enabled)
	{
		m_renderflags = set_flag(m_renderflags, render_flag::render_invisible, enabled);
	}

	void entity::set_debug_render(bool enabled)
	{
		m_renderflags = set_flag(m_renderflags, render_flag::render_debug, enabled);
	}

	bool entity::is_invisible() const
	{
		return is_flag_set(m_renderflags, render_flag::render_invisible);
	}

	bool entity::is_debug_render() const
	{
		return is_flag_set(m_renderflags, render_flag::render_debug);
	}
	entt::entity entity::get_handle() const
	{
		return m_handle;
	}
}