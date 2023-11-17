#include "app_pch.h"
#include "threads.h"

namespace influx::application
{
	void gamethread::static_initialize()
	{
	}

	void gamethread::static_tick()
	{
	}

	void gamethread::static_cleanup()
	{
	}


	void renderthread::static_initialize()
	{
	}

	void renderthread::static_tick()
	{
	}

	void renderthread::static_cleanup()
	{
	}


	void mainthread::static_initialize()
	{
	}

	void mainthread::static_tick()
	{
	}

	void mainthread::static_cleanup()
	{
	}


	void dedicated_thread::spin()
	{
		m_thread_object = std::thread([this]()
		{
			call_initialize();
			while (true)
			{
				call_tick();
			}
			call_cleanup();
		});
	}
}

