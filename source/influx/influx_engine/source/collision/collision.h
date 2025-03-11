#pragma once

namespace influx::engine
{
	enum class e_collision_layer : uint8
	{
		none	= 0,

		one		= 1 << 0,
		two		= 1 << 1,
		three	= 1 << 2,

		all		= one | two | three
	};
}