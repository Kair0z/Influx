#pragma once

#ifndef _EVENT_MANAGER_H_
#define _EVENT_MANAGER_H_

#include "Core/Function/Function.h"
#include "Core/Container/Containers.h"
#include "Core/Memory/Reference.h"
#include "Core/Singleton/Locator.h"

#include "Event.h"

namespace Influx
{
	enum class EventCategory
	{
		Input,
		Engine,
		Window,
		UserChannel0,
		UserChannel1,
		UserChannel2,
		UserChannel3,
		MAX
	};

	class EventManager final
	{
	public:
		EventManager() = default;
		~EventManager() = default;

	public:
		//typedef void(*EventCallback)(Event*);
		using EventCallback = Function<void(Event*)>;

	private:
		struct EventGroup
		{
			List<EventCallback> Listeners{};
			Queue<Event*> Events{};
		};

		Unordered_Map<EventCategory, EventGroup> mEventMap{};

	public:
		static Ptr<EventManager> Create();

		/* Pops all deferred Events and executes subscribed callbacks */
		void FlushAllChannels();
		void FlushChannel(EventCategory channel);

		template <EventCategory C>
		inline void SubscribeToChannel(EventCallback callback)
		{
			mEventMap[C].Listeners.push_back(callback);
		}

		/* Pings all listeners directly (takes the thread places...) */
		template <EventCategory C, class EType, typename ...Args>
		inline void PingChannelImmediate(Args&&... args)
		{
			/* Creates a local event with the construction arguments, no memory allocation needed. */
			EType ev{ std::forward<Args&&>(args)... };
			for (EventCallback callback : mEventMap[C].Listeners)
			{
				callback(&ev);
			}
		}

		/* Allocates memory!, but the ping execution happens on FlushAllChannels... */
		template <EventCategory C, class EType, typename ...Args>
		inline void PingChannel(Args&&... args)
		{
			/* Creates an event to be stored in the event map with the construction arguments */
			/* Memory allocation necessary to keep it untill we flush the event pools. */
			mEventMap[C].Events.push(new EType(std::forward<Args&&>(args)...));
		}
	};

	using EventManagerLocator = Locator<EventManager>;
}

#endif


