#pragma once

#include "Thread/Thread.h"

namespace Influx
{
	enum class EThreads
	{
		EventThread,
		RenderThread,
		GameThread,
		LoggingThread
	};

	class ThreadManager final
	{
	public:
		static Ptr<ThreadManager> Create();

		template <class TThreadClass, EThreads TThreadRole, typename = std::enable_if<std::is_base_of<Thread, TThreadClass>::value>::type>
		TThreadClass* CreateAndLaunchThread();

		~ThreadManager();

	private:
		Vector<Thread*> mThreads;
		
		ThreadManager() = default;
	};

	template<class TThreadClass, EThreads TThreadRole, typename>
	inline TThreadClass* ThreadManager::CreateAndLaunchThread()
	{
		TThreadClass* newThread = new TThreadClass();
		mThreads.push_back(newThread);
		newThread->Run();
		return newThread;
	}
}


