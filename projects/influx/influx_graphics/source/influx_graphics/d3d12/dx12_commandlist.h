#pragma once
#include "influx_graphics/commandlist.h"

struct ID3D12CommandList;
struct ID3D12GraphicsCommandList;

namespace influx::graphics
{
	class command_allocator;
	class pipeline;
	class render_target_view;

	class dx12_commandlist final : public command_list
	{
	public:
		dx12_commandlist(ID3D12GraphicsCommandList* commandlist);

		virtual void start(command_allocator* allocator, pipeline* init_state) override;

		virtual void draw_instanced(const draw_instanced_args& args) override;

		virtual void draw_indexed(const draw_indexed_args& args) override;

		virtual void set_indexbuffer(resource* index_buffer) override;

		virtual void set_vertexbuffer(resource* vertex_buffer) override;

		virtual void clear_rtv(render_target_view* view, const math::vectorf4& clear_value) override;

		virtual void transition_resource(resource* resource, e_resource_state before, e_resource_state after) override;

		virtual void copy_resource(resource* source, resource* dest) override;

		virtual void set(descriptor_heap* heap) override;

		virtual void set(render_target_view* rtv) override;

		virtual void set(rootsignature* rootsig) override;

		virtual void set(pipeline* pipeline) override;

		virtual void set(const viewport& viewport) override;

		virtual void set(const rect& rect) override;

		virtual void set(e_primitive_topology topo) override;

		virtual void end() override;

	private:
		ID3D12CommandList* mpdx_commandlist;
		ID3D12GraphicsCommandList* mpdx_graphics_commandlist;
	};
}