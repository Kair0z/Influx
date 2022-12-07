#pragma once

#ifndef _CORE_FUNCTION_H_
#define _CORE_FUNCTION_H_

#include <functional>
#include <type_traits>

namespace Influx
{
	template <typename _F>
	using Function = std::function<_F>;
}

#endif