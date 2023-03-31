#pragma once

#include "../Renderer/IFluxRenderer.h"

namespace Influx
{
	namespace Graphics
	{
		class D3D12Device;
		class RHICommandQueue;
		class RHICommandList;
		class RHISwapchain;

		class RHIGraphicsPipelineLayout;
		class RHIGraphicsPipeline;

		class RHIResource;
	}

	class RHIRenderer : public IFluxRenderer
	{
	public:
		RHIRenderer() = default;

	private:
		virtual void BuildRenderWork(Platform::WindowHandle windowHandle) override final;

		virtual void PresentToWindow(Platform::WindowHandle windowHandle) override final;

	private:
		Graphics::D3D12Device* mp_device;
		Graphics::RHICommandQueue* mp_commandQueue;
		Graphics::RHISwapchain* mp_swapchain;

		Graphics::RHICommandList* mp_commandList;

		Graphics::RHIGraphicsPipeline* mp_pipeline;
		Graphics::RHIGraphicsPipelineLayout* mp_pipelineLayout;

		Graphics::RHIResource* mp_indexBuffer;
		Graphics::RHIResource* mp_vertexBuffer;

		void InitializeDevice();
		void InitializeCommandQueue();
		void InitializeSwapchain(Platform::WindowHandle windowHandle);

		void InitializeRenderPipeline();

		void UpdateSceneBufferData();
	};
}


