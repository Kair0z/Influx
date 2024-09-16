#pragma once

#include "commandlist.h"
#include "commandallocator.h"

namespace influx::graphics
{
	class commandlist;
	class command_allocator;
	class queue;
}

namespace influx::graphics
{
	namespace detail
	{
		class command_base
		{
		public:

		};
	}

	class command : public detail::command_base
	{
	public:

	private:

	};
	
	// a encapsulating layer that manages underlying command allocator and GPU state
	class commandbuffer
	{
	public:
		enum class e_state : uint8
		{
			idle,
			submitted,
			finished,
			count
		};

		virtual void submit() = 0;
		virtual void submit(queue* queue) = 0;

		virtual e_state get_state() const = 0;

		bool is_finished_gpu() const
		{
			return get_state() == e_state::finished;
		}

	private:
		graphics::commandlist* m_commandlist;
		graphics::command_allocator* m_allocator;

	protected:
		e_state m_state;
	};
}