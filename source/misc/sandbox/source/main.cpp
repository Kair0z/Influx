// influx::core
#include "core/time.h"
#include "core/threading/thread.h"
using namespace influx;

// STL
#include <thread>

uint32 g_global_sum[16u] = {};
void do_thing(uint32 value, uint32 index)
{
	index %= 16u;
	g_global_sum[index] += value;
}

template <uint32 _n>
class round_robin_executor final
{
	enum class e_state
	{
		init,
		running,
		paused
	};
	e_state m_state;
	thread m_threads[_n];

public:
	struct settings final
	{
		float m_max_time_per_process = 2.0f;
	};

	round_robin_executor(const settings& init_settings)
	{

	}

	bool is_running() const
	{
		return m_state == e_state::running;
	}

	void start()
	{
		if (is_running()) return;

		for (uint32 i = 0u; i < _n; ++i)
			m_threads[i] = thread([this, i]()
			{
				// check for state
				if (!is_running())
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(16));
					return;
				}
				else do_thing(4u, i);
			});

		m_state = e_state::running;
	}

	void stop()
	{
		if (!is_running()) return;

		m_state = e_state::paused;
	}
};

int main()
{
	round_robin_executor<8u>::settings settings{};
	round_robin_executor<8u> executor(settings);

	executor.start();

	executor.stop();
}