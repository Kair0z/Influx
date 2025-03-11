#pragma once
#include "influx_graphics/swapchain.h"
#include "vk_headers.h"

namespace influx::platform
{
	class window;
}

namespace influx::graphics
{
	class vk_swapchain final : public swapchain
	{
	public:
		struct dependencies final
		{
			dependencies(
				const vk::Device& device,
				const vk::PhysicalDevice& gpu,
				const vk::Instance& inst)
				: m_vk_device{ device }
				, m_vk_gpu{ gpu }
				, m_vk_instance{ inst } {}

			vk::Device m_vk_device{};
			vk::PhysicalDevice m_vk_gpu{};
			vk::Instance m_vk_instance{};
		};

	public:
		vk_swapchain(
			const platform::window& window,
			const swapchain_desc& desc,
			const swapchain_dependencies& swapchain_dependencies, 
			const dependencies& vk_dependencies);

		virtual void present(const present_args& args) override;

		// acquires the next available backbuffer (and returns the index)
		virtual uint8 acquire_backbuffer() override;

	private:
		vk::SwapchainKHR m_vk_swapchain;
		vector<vk::Image> m_vk_images;
		vk::Queue m_vk_present_queue;
		vk::Device m_vk_device;

		virtual vector<resource*> create_resources() override;
	};
}