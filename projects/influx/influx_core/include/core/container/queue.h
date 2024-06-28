#pragma once

#ifndef _CORE_QUEUE_H_
#define _CORE_QUEUE_H_

#include <queue>

namespace influx
{
	template <typename _t>
	using queue = std::queue<_t>;
}

#endif
