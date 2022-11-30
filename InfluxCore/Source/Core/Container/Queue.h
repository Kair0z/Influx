#pragma once

#ifndef _CORE_QUEUE_H_
#define _CORE_QUEUE_H_

#include <queue>

namespace Influx
{
	template <typename _T>
	using Queue = std::queue<_T>;
}

#endif
