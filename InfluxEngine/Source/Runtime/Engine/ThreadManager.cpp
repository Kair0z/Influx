#include "pch.h"
#include "ThreadManager.h"

namespace Influx
{
	Ptr<ThreadManager> ThreadManager::Create()
	{
		Ptr<ThreadManager> newThreadManager = new ThreadManager();

		return newThreadManager;
	}

	ThreadManager::~ThreadManager()
	{
		for (Ref<Thread> thread : mThreads)
		{
			thread->SetQuit();
		}

		mThreads.clear();
	}
}

