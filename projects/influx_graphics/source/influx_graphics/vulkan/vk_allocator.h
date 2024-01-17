#pragma once
#include "influx_graphics/commandallocator.h"
#include "vk_headers.h"

namespace influx::graphics
{
	class vk_command_allocator final : public command_allocator
	{
	public:
		vk_command_allocator(const vk::CommandPool& vkpool);

	private:
		vk::CommandPool m_vk_commandpool;
	};
}