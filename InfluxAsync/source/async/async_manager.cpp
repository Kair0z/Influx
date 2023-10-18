#include "async_pch.h"
#include "async_manager.h"

namespace influx::async
{
	void async_manager::initialize(const init_args& args)
	{
		m_is_initialized = true;
	}

	void async_manager::shutdown()
	{
		m_is_initialized = false;
	}

#pragma region frontend_api
	void initialize(const init_args& args)
	{
		return async_manager::get_instance().initialize(args);
	}

	void shutdown()
	{
		async_manager::get_instance().shutdown();
	}
#pragma endregion
}