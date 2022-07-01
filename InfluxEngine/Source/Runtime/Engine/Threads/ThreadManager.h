#pragma once

#include "Thread/Thread.h"

namespace Influx
{
	class ThreadManager final
	{
	public:
		static Ptr<ThreadManager> Create();

		template <class TThreadClass, typename = std::enable_if<std::is_base_of<Thread, TThreadClass>::value>::type>
		void CreateAndLaunchThread();

	private:
		Vector<Thread*> Threads;
		
		ThreadManager() = default;
	};

	template<class TThreadClass, typename>
	inline void ThreadManager::CreateAndLaunchThread()
	{
		Threads.push_back(new TThreadClass());

		Threads.back()->Run();
	}
}


