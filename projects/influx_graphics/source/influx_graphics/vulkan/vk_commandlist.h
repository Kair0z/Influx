#pragma once
#include "influx_graphics/commandlist.h"

#include "vk_headers.h"

namespace influx::graphics
{
	class vk_commandlist final : public command_list
	{
	public:
		vk_commandlist(const vk::CommandBuffer& vkbuffer);

		virtual void start(command_allocator* allocator, pipeline_state* init_state) override;

		virtual void clear_rtv(render_target_view* view, const math::vectorf4& clear_value) override;

		virtual void transition_resource(resource* resource, e_resource_state before, e_resource_state after) override;

		virtual void copy_resource(resource* source, resource* dest) override;

		virtual void end() override;

	private:
		vk::CommandBuffer m_vk_commandbuffer;
	};
}