#pragma once

#ifndef _CORE_QUEUE_H_
#define _CORE_QUEUE_H_

#include <queue>

namespace influx
{
	template <typename _t>
	class queue : public std::queue<_t>
	{
	public:
		// Expose the underlying container (range-based for)
		using std::queue<_t>::c;

		void clear()
		{
			c.clear();
		}

		template <typename _readfunc>
		void read(_readfunc&& func) const
		{
			for (const _t& val : c)
			{
				func(val);
			}
		}
	};
}

#endif
