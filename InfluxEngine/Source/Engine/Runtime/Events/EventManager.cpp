#include "pch.h"
#include "EventManager.h"

namespace Influx
{
	Ptr<EventManager> EventManager::Create()
	{
		return new EventManager();
	}

	void EventManager::FlushAllChannels()
	{
		for (uint32_t c = (uint32_t)EventCategory::Engine; c < (uint32_t)EventCategory::MAX; ++c)
			FlushChannel((EventCategory)c);
	}

	void EventManager::FlushChannel(EventCategory category)
	{
		EventGroup& group = mEventMap[category];

		/* TODO: don't destroy and reallocate... Re-use memory! */
		while (!group.Events.empty())
		{
			Event* e = group.Events.front();
			for (EventCallback listener : group.Listeners)
			{
				listener(e);
			}

			delete e;
			group.Events.pop();
		}
	}
}