#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/enum.h"

namespace influx::engine::editor
{
	enum class e_group_flags : uint8
	{
		none		= 0,

		project		= 1 << 0,	// project file editor
		mainmenu	= 1 << 1,	// main menu editor (top bar)
		gameplay	= 1 << 2,	// start / stop gameplay
		user		= 1 << 3,	// ...

		all = mainmenu | gameplay | user
	};
}
ENABLE_ENUM_BIT_OPERATORS(influx::engine::editor::e_group_flags);