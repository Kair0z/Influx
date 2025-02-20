#include "engine_pch.h"
#include "entity.h"

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
		m_renderflags |= render_flag::render_invisible;
	}

	void entity::set_debug_render(bool enabled)
	{
		m_renderflags |= render_flag::render_debug;
	}

	bool entity::is_invisible() const
	{
		return has_flag(m_renderflags, render_flag::render_invisible);
	}

	bool entity::is_debug_render() const
	{
		return has_flag(m_renderflags, render_flag::render_debug);
	}
	entt::entity entity::get_handle() const
	{
		return m_handle;
	}
}