#pragma once
#include "influx_graphics/base.h"

#include "core/wait.h"

namespace influx::graphics
{
	class queue;
	class fence : public base
	{
	public:
		// queues a signal command to the command queue
		virtual void queue_signal(uint64 value, queue* queue) = 0;

		virtual void signal(uint64 value) = 0;

		// wait for fence to reach value
		virtual void wait_for_value(uint64 value, wait_handle& handle) = 0;

		virtual uint64 query_value() const = 0;
	};
}