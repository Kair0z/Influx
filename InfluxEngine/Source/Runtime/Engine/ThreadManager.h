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
		WeakRef<TThreadClass> CreateAndLaunchThread();

		~ThreadManager();

	private:
		Vector<Ref<Thread>> mThreads;
		
		ThreadManager() = default;
	};

	template<class TThreadClass, EThreads TThreadRole, typename>
	inline WeakRef<TThreadClass> ThreadManager::CreateAndLaunchThread()
	{
		mThreads.emplace_back(std::make_shared<TThreadClass>());
		mThreads.back()->Run();
		return std::dynamic_pointer_cast<TThreadClass>(mThreads.back());
	}
}


