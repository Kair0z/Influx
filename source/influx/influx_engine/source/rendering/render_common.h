#pragma once

// influx::core
#include "core/enum.h"

namespace influx::engine
{
	/* main views supported by this engine */
	enum class e_render_view : uint8
	{
		scene_editor,
		game,
		count
	};
	static constexpr uint8 k_num_render_views = static_cast<uint8>(e_render_view::count);
	constexpr static const char* k_render_view_names[k_num_render_views]
	{
		"scene",
		"game"
	};

	// a view contains a renderer::target to render to and data it wants rendered
	using render_view_id = string;

	enum class e_render_flags : uint8
	{
		none = 0,
		render_debug = 1 << 0,
		render_scene = 1 << 1,
		render_imgui = 1 << 2,
		all = render_debug | render_scene | render_imgui
	};

	enum class e_view_visibility_flags : uint8
	{
		none = 0,
		editor = 1 << 0,
		game = 1 << 1,
		all = game | editor
	};
}
ENABLE_ENUM_BIT_OPERATORS(influx::engine::e_render_flags);
ENABLE_ENUM_BIT_OPERATORS(influx::engine::e_view_visibility_flags);