#pragma once

#include "../Renderer/IFluxRenderer.h"

#include "Vulkan/vulkan.hpp"

namespace influx
{
	class VulkanRenderer : public IFluxRenderer
	{
	public:
		VulkanRenderer() = default;

	private:
		virtual void RecordRenderCommands(platform::window_handle windowHandle) override final;

		virtual void PresentToWindow(platform::window_handle windowHandle) override final;

		void WaitForPreviousFrame();

	private:
		void Initialize();
		void InitializeDevice();
		void InitializeCommandQueue();
		void InitializeDescriptorPools();
		void InitializePipeline();
		void InitializeCommandList();
		void InitializeSynchronization();
		void InitializeRenderPass();
		void InitializeSwapchain(platform::window_handle windowHandle);

		vk::Instance		m_instance;
		vk::PhysicalDevice	m_physicalDevice;
		vk::Device			m_logicalDevice;

		vk::Queue			m_commandQueue;
		vk::SwapchainKHR	m_swapchain;
		vk::SurfaceKHR		m_windowSurface;

		uint32 m_currentSwapchainBuffer;
		vk::Image m_swapchainImages[GetNumSwapchainBuffers()];

		vk::DescriptorPool m_descriptorPool;

		vk::RenderPass m_renderPass;

		vk::ImageView m_depthImageView;
		vk::ImageView m_colorImageView;

		// Pool of buffers...
		vk::CommandPool m_commandPool;
		vk::CommandBuffer m_commandBuffers[GetNumSwapchainBuffers()];

		vk::Rect2D	 m_renderArea;
		vk::Extent2D m_surfaceSize;
		vk::Viewport m_viewport;

		vk::Semaphore m_presentCompleteSemaphore;
		vk::Semaphore m_renderCompleteSemaphore;

		vk::Fence m_waitFences[GetNumSwapchainBuffers()];
	};
}


