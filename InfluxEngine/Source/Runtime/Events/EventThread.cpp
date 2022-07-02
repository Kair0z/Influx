#include "pch.h"
#include "EventThread.h"
#include "EventManager.h"

#include "Runtime/Application/WindowsApp.h"

namespace Influx
{
	EventThread::EventThread()
	{
		mpEventManager = EventManager::Create();
		EventManagerLocator::Provide(mpEventManager);
	}

	void EventThread::OnStart()
	{
		
	}

	void EventThread::OnTick()
	{
		// [Platform] Poll Application Events
		ApplicationLocator::Get()->PollEvents();

		// [TODO] put separate channel-flushing on separate threads?
		mpEventManager->FlushAllChannels();
	}

	void EventThread::OnEnd()
	{
	}

	Ptr<EventManager> EventThread::GetEventManager() const
	{
		return mpEventManager;
	}
}