#pragma once
#include "influx_graphics/commandlist.h"

struct ID3D12CommandList;
struct ID3D12GraphicsCommandList;

namespace influx::graphics
{
	class command_allocator;
	class pipeline_state;
	class render_target_view;

	class dx12_commandlist final : public command_list
	{
	public:
		dx12_commandlist(ID3D12GraphicsCommandList* commandlist);

		virtual void start(command_allocator* allocator, pipeline_state* init_state) override;

		virtual void clear_rtv(render_target_view* view, const math::vectorf4& clear_value) override;

		virtual void transition_resource(resource* resource, e_resource_state before, e_resource_state after) override;

		virtual void copy_resource(resource* source, resource* dest) override;

		virtual void end() override;

	private:
		ID3D12CommandList* mpdx_commandlist;
		ID3D12GraphicsCommandList* mpdx_graphics_commandlist;
	};
}