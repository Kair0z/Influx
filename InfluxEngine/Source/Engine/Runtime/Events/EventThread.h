#pragma once

#include "Thread/Thread.h"

namespace Influx
{
	class EventManager;

	class EventThread final : public Thread
	{
	public:
		EventThread();

		virtual void OnStart() override final;
		virtual void OnTick() override final;
		virtual void OnQuit() override final;

		Ptr<EventManager> GetEventManager() const;

		virtual ~EventThread();

	private:
		
		Ptr<EventManager> mpEventManager;
	};
}


