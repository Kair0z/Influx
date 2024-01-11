#pragma once
#include "influx_graphics/base.h"
#include "core/basetypes.h"
#include "core/container/vector.h"

namespace influx::graphics
{
	class command_list;

	enum class e_command_queue_type : uint8
	{
		graphics,
		copy,
		compute,
		count
	};

	enum class e_command_queue_flags : uint8
	{
		none
	};

	enum class e_command_queue_priority : uint8
	{
		low,
		medium,
		high,
		count
	};

	struct command_queue_desc final
	{
		e_command_queue_type m_type;
		e_command_queue_flags m_flags;
		float m_priority = 1.0f;
	};

	class command_queue : public base
	{
	public:
		virtual void submit_commandlists(const vector<command_list*>& commandlists) = 0;

	protected:
		command_queue(const command_queue_desc& desc);
	};
}