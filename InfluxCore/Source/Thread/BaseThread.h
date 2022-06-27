#pragma once

namespace Influx
{
	class BaseThread
	{
		enum class EThreadPriority
		{
			Low, Normal, High
		};

	public:
		/* Run the thread */
		virtual int Run() = 0;

		/* Tell the Thread to Exit (Forcing is not recommended -> Leaks) */
		virtual void Kill(bool force = false) = 0;
		
		/* Halts the caller until this thread is finished. */
		virtual void WaitForComplete() = 0;

		/* Tell the Thread to pause/resume */
		virtual void Suspend(bool shouldPause = true) = 0;

	protected:
		BaseThread() = default;
		virtual ~BaseThread() = default;

		bool mIsRunning{ false };
	};
}


