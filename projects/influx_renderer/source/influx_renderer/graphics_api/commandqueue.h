#pragma once
#include "graphics_api.h"
#include "core/basetypes.h"

namespace influx::renderer::api
{
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

	class command_queue final
		: public base
	{
	public:
		command_queue(const logical_device& device, const command_queue_desc& desc);
	};
}