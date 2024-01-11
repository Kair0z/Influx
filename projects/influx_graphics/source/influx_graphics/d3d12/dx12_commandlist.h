#pragma once
#include "influx_graphics/commandlist.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

#include "influx_graphics/pipelinestate.h"
#include "influx_graphics/commandallocator.h"

namespace influx::graphics
{
	class dx12_commandlist final : public command_list
	{
	public:
		inline dx12_commandlist(ID3D12GraphicsCommandList* commandlist)
		{
			mp_native = mpdx_graphics_commandlist = commandlist;
		}

		inline virtual void start(command_allocator* allocator, pipeline_state* init_state) override
		{
			ID3D12PipelineState* dxpipeline = (init_state ? init_state->get_native<ID3D12PipelineState>() : nullptr);
			ID3D12CommandAllocator* dxallocator = allocator->get_native<ID3D12CommandAllocator>();
			mpdx_graphics_commandlist->Reset(dxallocator, dxpipeline);
		}
		
		inline virtual void end() override
		{
			mpdx_graphics_commandlist->Close();
		}

	private:
		ID3D12GraphicsCommandList* mpdx_graphics_commandlist;
	};
}