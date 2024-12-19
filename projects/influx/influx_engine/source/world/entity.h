#pragma once

// entt
#include "entt/entt.hpp"

namespace influx::engine
{
	class entity final
	{
	public:
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

	private:
		render_flag m_renderflags;
		entt::entity m_handle;
	};
}