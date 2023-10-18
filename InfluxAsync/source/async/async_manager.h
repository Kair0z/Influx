#pragma once
#include "influx_async.h"
#include "Core/singleton/Singleton.h"

namespace influx::async
{
	class async_manager final 
		: public singleton<async_manager>
	{
	public:
		void initialize(const init_args& args);
		void shutdown();

	private:
		bool m_is_initialized = false;
	};
}


