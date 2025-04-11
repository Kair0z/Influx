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
		queue_desc m_desc{};

	public:
		INFLUX_GFX_API result<> submit(const vector<commandlist*>& commandlists);

		// queues a signal to the target fence
		INFLUX_GFX_API virtual result<> queue_signal(fence* fence, uint64 value) = 0;

	protected:
		queue(const queue_desc& desc);
		virtual ~queue() = default;
		result<> post_submit(const vector<commandlist*>& commandlists);
		friend class commandlist;
		friend class device;

		// impl
		virtual result<> submit_commandlists(const vector<commandlist*>& commandlists) = 0;
	};
}