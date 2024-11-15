#pragma once
#include "influx_graphics/base.h"
#include "core/container/vector.h"
#include "core/basetypes.h"

namespace influx::graphics
{
	class commandlist;
	class fence;

	enum class e_queue_type : uint8
	{
		graphics,
		copy,
		compute,
		count
	};

	enum class e_queue_flags : uint8
	{
		none
	};

	enum class e_queue_priority : uint8
	{
		normal,
		high,
		global_realtime,
		count
	};

	struct queue_desc final
	{
		static queue_desc default_graphics()
		{
			static queue_desc result{};
			result.m_flags = {};
			result.m_priority = e_queue_priority::normal;
			result.m_type = e_queue_type::graphics;
			return result;
		}

		e_queue_type m_type;
		e_queue_flags m_flags;
		e_queue_priority m_priority = e_queue_priority::normal;
	};

	class queue : public base
	{
	public:
		void submit(const vector<commandlist*>& commandlists);

		// queues a signal to the target fence
		virtual void queue_signal(fence* fence, uint64 value) = 0;

		virtual ~queue() = default;

	protected:
		friend class commandlist;
		queue(const queue_desc& desc);
		void post_submit(const vector<commandlist*>& commandlists);

	private:
		virtual void submit_commandlists(const vector<commandlist*>& commandlists) = 0;

	private:
		queue_desc m_desc{};
	};
}