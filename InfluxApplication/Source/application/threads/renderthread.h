#pragma once
#include "threads.h"

namespace influx::application
{
	class renderthread final : public dedicated_thread
	{
	public:
		virtual void initialize() override;
		virtual void tick() override;
		virtual void cleanup() override;

		virtual e_dedicated_thread get_thread_type() const override
		{
			return e_dedicated_thread::renderthread;
		}
	};
}