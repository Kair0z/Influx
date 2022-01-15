#pragma once

#include <functional>

namespace Influx
{
	template <typename TFunc>
	using Function = std::function<TFunc>;
}